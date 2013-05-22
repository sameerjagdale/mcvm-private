#include <unordered_set>
#include <unordered_map>
#include <analysis/analysisfw.h>
#include <analysis/value/liveness.h>

namespace mcvm { namespace analysis {

    template <>
        AnalyzerContext<LivenessInfo> analyze(ProgFunction* function) {
            AnalyzerContext<LivenessInfo> context ;
            LivenessInfo entry ;
            return analyze_function<LivenessInfo,LivenessInfo,Direction::Backward> (context,function,entry) ;
        }
    
    template <>
        LivenessInfo merge (
                const LivenessInfo& a,
                const LivenessInfo& b) {
            return LivenessInfo{} ;
        }
    
    template <>
        LivenessInfo analyze_assignstmt (
                const AssignStmt* assign,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            auto rhs = analyze_expr<LivenessInfo,LivenessInfo> (
                    assign->getRightExpr(),
                    context,
                    in);
            return rhs ;
        }

    template <>
        LivenessInfo analyze_expr (
                const SymbolExpr* symbol,
                AnalyzerContext<LivenessInfo>& context,
                const LivenessInfo& in) {
            auto out = in ;
            out.insert (symbol) ;
            return out;
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
