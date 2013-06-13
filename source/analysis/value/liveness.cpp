#include <unordered_set>
#include <unordered_map>
#include <analysis/analysisfw.h>
#include <analysis/value/liveness.h>

#include <dotexpr.h>

namespace mcvm { namespace analysis {

    using L = LivenessInfo ;
    
    template <typename Expr>
        LivenessInfo analyze_expr (
                const Expr*,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            std::cout << "generic liveness" << std::endl;
            return LivenessInfo{} ;
        }

#include "analysis/analysisfw_dispatch.h"

    template <>
        L top() {
            return L{} ;
        }

    template <>
        L operator+ (const L& left,const L& right) {
            L ret = left ;
           for (auto& r : right) {
               ret.insert(r) ;
           }
           return ret;
        }
    
    template <>
        L operator- (const L& left,const L& right) {
            L ret = left ;
            for (auto& r : right) {
                ret.erase(r) ;
            }
            return ret;
        }

    template <>
        FlowMap<L> analyze(ProgFunction* function) {
            AnalyzerContext<LivenessInfo> context ;
            LivenessInfo entry ;
            return analyze_function<LivenessInfo,Direction::Backward> 
                (context,function,entry) ;
        }
    
    template <>
        LivenessInfo merge (
                const LivenessInfo& a,
                const LivenessInfo& b) {
            return a + b ;
        }
    
    template <>
        LivenessInfo analyze_expr (
                const SymbolExpr* symbol,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            std::cout << "specialize" << std::endl ;
            auto out = in ;
            out.insert (symbol) ;
            return out;
        }

    template <> 
        LivenessInfo analyze_assignstmt (
                const AssignStmt* assign,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            auto rhs = analyze_expr_dispatch<LivenessInfo>(
                 assign->getRightExpr(),
                 context,
                 in);
            //auto lhs = assign->getLeftExprs() ;
            auto defs = assign->getSymbolDefs() ;
            LivenessInfo kill ( defs.begin() , defs.end() ) ;
            LivenessInfo lhs ;
            auto gen = lhs + rhs ;
            return (in - kill) + gen;
        }

    template <>
        LivenessInfo analyze_expr (
                const DotExpr* dot,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            return analyze_expr_dispatch <LivenessInfo>(
                    dot->getExpr(),
                    context,
                    in);
        }

    template <>
        LivenessInfo analyze_expr (
                const ParamExpr* param,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            LivenessInfo gen ;
            auto left = param->getExpr() ;
            if (left->getExprType() == Expression::SYMBOL) {
                auto symbol = static_cast<SymbolExpr*>(left);
                std::set<std::string> function_to_watch { "ones", "zeros" , "eye" } ;
                auto itr = function_to_watch.find(symbol->getSymName()) ;
                if (itr != std::end(function_to_watch)) {
                    for (auto& p : param->getArguments()){
                        gen.insert((SymbolExpr*)p) ;
                    }
                }
            }
            return (in + gen) ;
        }
}}

std::ostream& operator<<(
        std::ostream &strm,
        const mcvm::analysis::LivenessInfo& liveset) {
    for (auto& symbol : liveset) {
        strm << symbol->toString() << std::endl ;
    }
    return strm ;
}
