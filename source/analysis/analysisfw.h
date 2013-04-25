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
            FlowInfo returns_ ; std::stack<FlowInfo> recursive_returns_ ;
            
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
            ExpressionFn<IntConstExpr> intconst_;
            ExpressionFn<ParamExpr> paramexpr_;
            ExpressionFn<BinaryOpExpr> binop_;
            ExpressionFn<MatrixExpr> matrix_;
            ExpressionFn<StrConstExpr> str_;
            ExpressionFn<SymbolExpr> symbol_function_;

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

    template <typename FlowInfo, typename ExprInfo>
        AnalyzerContext<FlowInfo> analyze(
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                ProgFunction* function
                )
        {
            AnalyzerContext<FlowInfo> context ;
            return analyze(analyzer,context,function) ;
        }
            
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
    

    template <typename FlowInfo, typename ExprInfo>
        AnalyzerContext<FlowInfo> analyze(
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                ProgFunction* function
                )
        {
            auto body = function->getCurrentBody() ;
            context.local_env_ = ProgFunction::getLocalEnv(function);
            context.recursive_.insert(function) ;
            context.recursion_ = false ;
            FlowInfo entry ;
            auto end_info = analyzer.sequencestmt_ (body,analyzer,context,entry) ;
            context.return_points_.push_back(end_info) ;
            std::cout << "return size" << context.return_points_.size() << std::endl ;
            
            // Compute the return flowinfo from returns point
            auto params = function->getOutParams() ;
            //DISCLAIMER: we suppose that everything is a symbolexpr
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

    template <typename FlowInfo,typename ExprInfo>
        FlowInfo assignstmt(
                const AssignStmt* assign,
                const Analyzer<FlowInfo,ExprInfo>& analyzer,
                AnalyzerContext<FlowInfo>& context,
                const FlowInfo& in
                )
        {
            auto out = in ;

            auto rights = analyzer.expression_ (assign->getRightExpr(),analyzer,context,in) ;
            auto lefts = assign->getLeftExprs() ;

            if (rights.size() < lefts.size() ) {
                std::cout << "Not enough returned values" << rights.size() << std::endl << lefts.size() << std::endl ;
                return in ;
                //throw std::exception() ;
            }

            auto it1 =  rights.begin() ;
            auto it2 = lefts.begin() ;

            /* We iterate only over lefts expression because there can be less left
             * than right
             * ex : [a,b] = <std::vector>.size 3 is valid
             */
            for (;
                    it2 != lefts.end() ;
                    ++it1 , ++it2 ) {
                out[ (SymbolExpr*) *it2] = *it1 ;
            }

            return out;
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
                case Expression::ExprType::LAMBDA:
                case Expression::ExprType::CELLARRAY:
                case Expression::ExprType::END:
                case Expression::ExprType::RANGE:
                case Expression::ExprType::STR_CONST:
                    return analyzer.str_((StrConstExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::FP_CONST:
                case Expression::ExprType::BINARY_OP:
                    return analyzer.binop_((BinaryOpExpr*)expr,analyzer,context,in ) ;
                case Expression::ExprType::UNARY_OP:
                case Expression::ExprType::CELL_INDEX:
                case Expression::ExprType::INT_CONST:
                    return analyzer.intconst_((IntConstExpr*)expr,analyzer,context,in ) ;

                case Expression::ExprType::SYMBOL:
                    {
                        auto s = (SymbolExpr*)expr;
                        auto itr = in.find(s) ;
                        if (itr != std::end(in)) {
                            return {itr->second};
                        }
                        return analyzer.symbol_function_(s,analyzer,context,in) ;
                    }
                    
                case Expression::ExprType::PARAM:
                    return analyzer.paramexpr_( (ParamExpr*)expr, analyzer, context, in) ;
                    
            }
            return {};
        }

}}

#endif
