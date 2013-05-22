#ifndef ANALYSISFW_H
#define ANALYSISFW_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include "stmtsequence.h"
#include "assignstmt.h"
#include "ifelsestmt.h"
#include "loopstmts.h"
#include "exprstmt.h"
#include "constexprs.h"
#include "functions.h"
#include "iir.h"
#include "interpreter.h"

class ParamExpr;
class DotExpr;

namespace mcvm { namespace analysis {

    /************************************
     * Type declaration 
     ************************************/

    enum class Direction { Forward, Backward } ;
    
    template <typename FlowInfo>
        struct AnalyzerContext {
            std::vector<FlowInfo> break_points_ ;
            std::vector<FlowInfo> continue_points_ ;

            // Returned info
            std::vector<FlowInfo> return_points_ ;
            FlowInfo returns_ ; 
            std::stack<FlowInfo> recursive_returns_ ;

            std::unordered_map<IIRNode*,FlowInfo> data_ ;
            std::unordered_set<ProgFunction*> recursive_ ;
            bool recursion_ ;
            Environment* local_env_ ;
        };
    
    /************************************
     *  Prototypes
     ************************************/
    template <typename FlowInfo>
        FlowInfo operator+ (const FlowInfo& left,const FlowInfo& right) ;
    
    template <typename FlowInfo>
        FlowInfo operator- (const FlowInfo& left,const FlowInfo& right) ;
   
    template <typename FlowInfo>
    FlowInfo merge(
            const FlowInfo&,
            const FlowInfo&) ;
    
    template <typename Expr, typename FlowInfo, typename ExprInfo>
        ExprInfo analyze_expr (
                const Expr* expr,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in) { 
            return ExprInfo{} ;
        }

    template <typename FlowInfo, typename ExprInfo, Direction d>
        FlowInfo analyze_sequencestmt (
                const StmtSequence* stmt,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in);
    
    template <typename FlowInfo>
        FlowInfo analyze_assignstmt (
                const AssignStmt* seq,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in);
    
    template <typename FlowInfo, typename ExprInfo, Direction d>
        AnalyzerContext<FlowInfo> analyze_function (
                AnalyzerContext<FlowInfo>& context,
                ProgFunction* function,
                const FlowInfo& entry);
        
    template <typename FlowInfo>
        AnalyzerContext<FlowInfo> analyze(ProgFunction* function);
    
    /************************************
     * Template Implementation 
     ************************************/

    template <typename FlowInfo>
        FlowInfo merge_list(
                const std::vector<FlowInfo>& infos
                ) 
        {
            auto merged = * infos.begin() ;
            for (auto info : infos) {
                merged = merge(info,merged) ;
            }
            return merged ;
        }
    
    //DISCLAIMER: we suppose that everything is a symbolexpr
    template <typename FlowInfo, typename ExprInfo, typename Info>
        ExprInfo compute_returned_vector (
                const std::vector<FlowInfo>& infos,
                const std::function<FlowInfo(FlowInfo&,FlowInfo&)>& merge,
                const ProgFunction* function ) {
            auto params = function->getOutParams() ;
            ExprInfo ret ;
            auto l = merge_list(infos,merge) ;
            for (auto param: params) {
                auto itr = l.find (param) ;
                if (itr != std::end(l)) {
                    ret.push_back(itr->second) ;
                }
            }
            return ret ;
        }

    template <typename FlowInfo, typename ExprInfo, Direction d>
        FlowInfo analyze_loopstmt (
                const LoopStmt* loop,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in
                ){
            
            auto current = in ;
            for (;;) {
                // Handle the init seq
                auto initseq = loop->getInitSeq() ;
                auto after_init = analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                        initseq,
                        context,
                        in) ;


                // Test sequence
                auto testseq = loop->getTestSeq() ;
                auto after_test = analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                        testseq,
                        context,
                        after_init) ;

                // Run on the body loop
                auto body = loop->getBodySeq() ;
                auto after_body = analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                        body,
                        context,
                        after_test) ;

                // Incrementation seq
                auto incrseq = loop->getIncrSeq() ;

                auto after_incr = analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                        incrseq,
                        context,
                        after_body) ;

                if (current == after_incr) {
                    return after_incr ;
                } else {
                    current = after_incr ;
                }
            }
        }
    
            
    
    template <typename FlowInfo, typename ExprInfo, Direction d>
        FlowInfo analyze_sequencestmt (
                const StmtSequence* seq,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in
                ){
            
            auto current = in ;
            
            auto stmtsequence = seq->getStatements() ;

            if (d == Direction::Backward) 
                std::reverse (std::begin(stmtsequence) , std::end(stmtsequence)) ;

            for (auto st: stmtsequence) {
                std::cout << st->toString() << std::endl ;
                switch (st->getStmtType()) {
                    case Statement::ASSIGN:
                        current = analyze_assignstmt<FlowInfo>(
                                static_cast<AssignStmt*>(st),
                                context,
                                current);
                        break;

                    case Statement::IF_ELSE:
                        {
                            auto stmt = static_cast<IfElseStmt*>(st) ;
                            auto stmt_if = stmt->getIfBlock() ;
                            auto stmt_else = stmt->getElseBlock() ;
                            auto info_if =
                                analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                                        static_cast<StmtSequence*>(stmt_if),
                                        context,
                                        current) ;
                            auto info_else =
                                analyze_sequencestmt<FlowInfo,ExprInfo,d> (
                                        static_cast<StmtSequence*>(stmt_else),
                                        context,
                                        current) ;

                            current = merge(info_if,info_else) ;
                        }
                        break;
                    case Statement::LOOP:
                        current = analyze_loopstmt<FlowInfo,ExprInfo,d>(
                                static_cast<LoopStmt*>(st),
                                context,
                                current) ;
                        break;

                    case Statement::BREAK:
                        context.break_points_.push_back(current) ;
                        break;

                    case Statement::CONTINUE:
                        context.continue_points_.push_back(current) ;
                        break;

                    case Statement::RETURN:
                        context.return_points_.push_back(current) ;
                        break;

                    case Statement::EXPR:
                        //current = exprstmt(current,static_cast<ExprStmt*>(st) ) ;
                        break;

                    case Statement::FOR:
                        break;

                    case Statement::SWITCH:
                    case Statement::COMPOUND_END:
                    case Statement::WHILE:
                        break;
                }
                std::cout << current << std::endl ;
                context.data_[(IIRNode*)st] = current ;
            }
            return current ;
        }

    template <typename FlowInfo, typename ExprInfo, typename Info>
        AnalyzerContext<FlowInfo> analyze(
                ProgFunction* function,
                const std::vector<Info>& params ) {
            
            AnalyzerContext<FlowInfo> context ;
            // Construct the correct input flowinfo
            auto fun_params = function->getInParams() ;
            
            auto it_caller =  params.begin() ;
            auto it_callee = fun_params.begin() ;

            /* We iterate only over lefts expression because there can be less left
             * than right
             *
             * call function(1344) 
             * ex : function (a,b,c) 
             * 
             */
            FlowInfo i ;
            for (;
                    it_caller != std::end(params);
                    ++it_caller, ++it_callee) {
                i [ (SymbolExpr*) *it_callee] = *it_caller ;
            }
            return analyze(context,function,i) ;
        }


    template <typename FlowInfo, typename ExprInfo, Direction d>
        AnalyzerContext<FlowInfo> analyze_function(
                AnalyzerContext<FlowInfo>& context,
                ProgFunction* function,
                const FlowInfo& entry
                )
        {
            auto body = function->getCurrentBody() ;
            context.local_env_ = ProgFunction::getLocalEnv(function);
            context.recursive_.insert(function) ;
            context.recursion_ = false ;
            auto end_info = analyze_sequencestmt<FlowInfo,ExprInfo,d>(body,context,entry) ;
            context.return_points_.push_back(end_info) ;
            std::cout << "return size" << context.return_points_.size() << std::endl ;
            
            // Compute the return flowinfo from returns point
            context.returns_ = 
                merge_list(
                        context.return_points_);
            
            std::cout << "FIN " << context.returns_ << std::endl ;

            if (!context.recursion_) {
                std::cout << "BYE" << std::endl ;
                // no recursion, analyze is finished 
                return context ;
            }

            // The analysis is not over, we need to find a fixpoint
            // on the overall function
            
            AnalyzerContext<FlowInfo> new_context ;
            new_context.returns_ = context.returns_ ;
            new_context.recursive_.insert(function) ;
            FlowInfo new_entry ;
            FlowInfo new_end_info ;
            do {
                new_end_info = analyze_sequencestmt<FlowInfo,ExprInfo,d>(body,context,entry) ;
            } while (context.returns_ != new_context.returns_ ) ;
            return new_context ;
        }


    template <typename FlowInfo, typename ExprInfo>
        ExprInfo analyze_expr(
                const Expression* expr,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in)
        {
            std::cout << "dispatch" << std::endl ;
            
            switch (expr->getExprType()) {
                case Expression::ExprType::MATRIX:
                    return analyze_expr<MatrixExpr,FlowInfo,ExprInfo> ((MatrixExpr*)expr,context,in ) ;
                case Expression::ExprType::FN_HANDLE:
                    return analyze_expr<FnHandleExpr,FlowInfo,ExprInfo> ((FnHandleExpr*)expr,context,in ) ;
                case Expression::ExprType::LAMBDA:
                    return analyze_expr<LambdaExpr,FlowInfo,ExprInfo> ((LambdaExpr*)expr,context,in ) ;
                case Expression::ExprType::CELLARRAY:
                    return analyze_expr<CellArrayExpr,FlowInfo,ExprInfo> ((CellArrayExpr*)expr,context,in ) ;
                case Expression::ExprType::END:
                    return analyze_expr<EndExpr,FlowInfo,ExprInfo> ((EndExpr*)expr,context,in ) ;
                case Expression::ExprType::RANGE:
                    return analyze_expr<RangeExpr,FlowInfo,ExprInfo> ((RangeExpr*)expr,context,in ) ;
                case Expression::ExprType::STR_CONST:
                    return analyze_expr<StrConstExpr,FlowInfo,ExprInfo> ((StrConstExpr*)expr,context,in ) ;
                case Expression::ExprType::FP_CONST:
                    return analyze_expr<FPConstExpr,FlowInfo,ExprInfo> ((FPConstExpr*)expr,context,in ) ;
                case Expression::ExprType::BINARY_OP:
                    return analyze_expr<BinaryOpExpr,FlowInfo,ExprInfo> ((BinaryOpExpr*)expr,context,in ) ;
                case Expression::ExprType::UNARY_OP:
                    return analyze_expr<UnaryOpExpr,FlowInfo,ExprInfo> ((UnaryOpExpr*)expr,context,in ) ;
                case Expression::ExprType::CELL_INDEX:
                    return analyze_expr<CellIndexExpr,FlowInfo,ExprInfo> ((CellIndexExpr*)expr,context,in ) ;
                case Expression::ExprType::INT_CONST:
                    return analyze_expr<IntConstExpr,FlowInfo,ExprInfo> ((IntConstExpr*)expr,context,in ) ;
                case Expression::ExprType::PARAM:
                    return analyze_expr<ParamExpr,FlowInfo,ExprInfo> ((ParamExpr*)expr,context,in ) ;
                case Expression::ExprType::DOT:
                    return analyze_expr<DotExpr,FlowInfo,ExprInfo> ((DotExpr*)expr,context,in ) ;
                case Expression::ExprType::SYMBOL:
                      return analyze_expr<SymbolExpr,FlowInfo,ExprInfo> ((SymbolExpr*)expr,context,in ) ;
            }
        }

}}

#endif
