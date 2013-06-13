#ifndef ANALYSISFW_DISPATCH
#define ANALYSISFW_DISPATCH

        template <typename E, typename... Params>
           E analyze_expr_dispatch (
                    const Expression* expr,
                    Params... p) {
                std::cout << "dispatch" << std::endl ;
                switch (expr->getExprType()) {
                    case Expression::ExprType::MATRIX:
                        return analyze_expr (static_cast<const MatrixExpr*>(expr),p...) ;
                    case Expression::ExprType::FN_HANDLE:
                        return analyze_expr (static_cast<const FnHandleExpr*>(expr),p...) ;
                    case Expression::ExprType::LAMBDA:
                        return analyze_expr (static_cast<const LambdaExpr*>(expr),p...) ;
                    case Expression::ExprType::CELLARRAY:
                        return analyze_expr (static_cast<const CellArrayExpr*>(expr),p...) ;
                    case Expression::ExprType::END:
                        return analyze_expr (static_cast<const EndExpr*>(expr),p...) ;
                    case Expression::ExprType::RANGE:
                        return analyze_expr (static_cast<const RangeExpr*>(expr),p...) ;
                    case Expression::ExprType::STR_CONST:
                        return analyze_expr (static_cast<const StrConstExpr*>(expr),p...) ;
                    case Expression::ExprType::FP_CONST:
                        return analyze_expr (static_cast<const FPConstExpr*>(expr),p...) ;
                    case Expression::ExprType::BINARY_OP:
                        return analyze_expr (static_cast<const BinaryOpExpr*>(expr),p...) ;
                    case Expression::ExprType::UNARY_OP:
                        return analyze_expr (static_cast<const UnaryOpExpr*>(expr),p...) ;
                    case Expression::ExprType::CELL_INDEX:
                        return analyze_expr (static_cast<const CellIndexExpr*>(expr),p...) ;
                    case Expression::ExprType::INT_CONST:
                        return analyze_expr (static_cast<const IntConstExpr*>(expr),p...) ;
                    case Expression::ExprType::PARAM:
                        return analyze_expr (static_cast<const ParamExpr*>(expr),p...) ;
                    case Expression::ExprType::DOT:
                        return analyze_expr (static_cast<const DotExpr*>(expr),p...) ;
                    case Expression::ExprType::SYMBOL:
                        return analyze_expr (static_cast<const SymbolExpr*>(expr),p...) ;
                }
                //return top<E>() ;
            }
#endif
