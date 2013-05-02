#ifndef ANALYSISFW_H
#define ANALYSISFW_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
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

namespace mcvm { namespace analysis {

    template <typename FlowInfo>
        FlowInfo operator+ (const FlowInfo& left,const FlowInfo& right) ;
    template <typename FlowInfo>
        FlowInfo operator- (const FlowInfo& left,const FlowInfo& right) ;

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

    template <typename FlowInfo,typename ExprInfo>
        struct Analyzer {

            std::function<FlowInfo(FlowInfo&,FlowInfo&)> merge_ ;

            template <typename Expression>
                using ExpressionFn = std::function<ExprInfo(
                        const Expression*,
                        const Analyzer<FlowInfo,ExprInfo>&,
                        AnalyzerContext<FlowInfo>&,
                        const FlowInfo& //in set
                        )> ;

            ExpressionFn<Expression> expression_ ;
            ExpressionFn<CellIndexExpr> cellindex_;
            ExpressionFn<IntConstExpr> intconst_;
            ExpressionFn<FnHandleExpr> fnhandle_;
            ExpressionFn<LambdaExpr> lambda_;
            ExpressionFn<CellArrayExpr> cellarray_;
            ExpressionFn<FPConstExpr> fpconst_;
            ExpressionFn<ParamExpr> paramexpr_;
            ExpressionFn<BinaryOpExpr> binop_;
            ExpressionFn<UnaryOpExpr> unaryop_;
            ExpressionFn<MatrixExpr> matrix_;
            ExpressionFn<StrConstExpr> str_;
            ExpressionFn<SymbolExpr> symbol_function_;
            ExpressionFn<EndExpr> end_;
            ExpressionFn<RangeExpr> range_;

            template <typename Stmt>
                using TransferFn = std::function<FlowInfo(
                        const Stmt*,
                        const Analyzer& ,
                        AnalyzerContext<FlowInfo>&,
                        const FlowInfo&
                        )> ;

            TransferFn<AssignStmt> assignstmt_ ;
            TransferFn<StmtSequence> sequencestmt_ ;
            TransferFn<ForStmt> forstmt_ ;
            TransferFn<LoopStmt> loopstmt_ ;
        };

    template <typename FlowInfo>
        FlowInfo merge_list(
                const std::vector<FlowInfo>& infos,
                const std::function<FlowInfo(FlowInfo&,FlowInfo&)>& merge
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

                
    template <typename FlowInfo, typename ExprInfo, typename Info>
        AnalyzerContext<FlowInfo> analyze(
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                ProgFunction* function,
                const std::vector<Info>& params
                )
        {
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
            return analyze(analyzer,context,function,i) ;
        }

    template <typename FlowInfo, typename ExprInfo>
        AnalyzerContext<FlowInfo> analyze(
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                ProgFunction* function,
                const FlowInfo& entry
                )
        {
            auto body = function->getCurrentBody() ;
            context.local_env_ = ProgFunction::getLocalEnv(function);
            context.recursive_.insert(function) ;
            context.recursion_ = false ;
            auto end_info = analyzer.sequencestmt_ (body,analyzer,context,entry) ;
            context.return_points_.push_back(end_info) ;
            std::cout << "return size" << context.return_points_.size() << std::endl ;
            
            // Compute the return flowinfo from returns point
            context.returns_ = 
                merge_list(
                        context.return_points_,
                        analyzer.merge_);
            
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
                new_end_info = analyzer.sequencestmt_ (body,analyzer,context,entry) ;
            } while (context.returns_ != new_context.returns_ ) ;
            return new_context ;
        }


    template <typename FlowInfo, typename ExprInfo>
        FlowInfo sequencestmt(
                const StmtSequence* seq,
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in
                ){
            auto current = in ;
            for (auto st: seq->getStatements() ) {
                std::cout << st->toString() << std::endl ;
                switch (st->getStmtType()) {
                    case Statement::ASSIGN:
                        current = analyzer.assignstmt_(
                                static_cast<AssignStmt*>(st),
                                analyzer,
                                context,
                                current);
                        break;

                    case Statement::IF_ELSE:
                        {
                            auto stmt = static_cast<IfElseStmt*>(st) ;
                            auto stmt_if = stmt->getIfBlock() ;
                            auto stmt_else = stmt->getElseBlock() ;
                            auto info_if =
                                analyzer.sequencestmt_(
                                        static_cast<StmtSequence*>(stmt_if),
                                        analyzer,
                                        context,
                                        current) ;
                            auto info_else =
                                analyzer.sequencestmt_(
                                        static_cast<StmtSequence*>(stmt_else),
                                        analyzer,
                                        context,
                                        current) ;

                            current = analyzer.merge_(info_if,info_else) ;
                        }
                        break;
                    case Statement::LOOP:
                        current = analyzer.loopstmt_(
                                static_cast<LoopStmt*>(st),
                                analyzer,
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
                        current = analyzer.forstmt_(
                                static_cast<ForStmt*>(st),
                                analyzer,
                                context,
                                current
                                );
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

    template <typename FlowInfo, typename ExprInfo>
        ExprInfo expression(
                const Expression* expr,
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in)
        {
            switch (expr->getExprType()) {
                case Expression::ExprType::MATRIX:
                    return analyzer.matrix_((MatrixExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::FN_HANDLE:
                    return analyzer.fnhandle_((FnHandleExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::LAMBDA:
                    return analyzer.lambda_((LambdaExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::CELLARRAY:
                    return analyzer.cellarray_((CellArrayExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::END:
                    return analyzer.end_((EndExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::RANGE:
                    return analyzer.range_((RangeExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::STR_CONST:
                    return analyzer.str_((StrConstExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::FP_CONST:
                    return analyzer.fpconst_((FPConstExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::BINARY_OP:
                    return analyzer.binop_((BinaryOpExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::UNARY_OP:
                    return analyzer.unaryop_((UnaryOpExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::CELL_INDEX:
                    return analyzer.cellindex_((CellIndexExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::INT_CONST:
                    return analyzer.intconst_((IntConstExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::PARAM:
                    return analyzer.paramexpr_( (ParamExpr*)expr, analyzer, context, in) ;
                case Expression::ExprType::SYMBOL:
                    {
                        auto s = (SymbolExpr*)expr;
                        auto itr = in.find(s) ;
                        if (itr != std::end(in)) {
                            return {itr->second};
                        }
                        return analyzer.symbol_function_(s,analyzer,context,in) ;
                    }
            }
            return {};
        }

}}

#endif
