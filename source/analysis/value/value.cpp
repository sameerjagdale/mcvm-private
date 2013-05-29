#include "value.h"
#include "liveness.h"
#include <analysis/analysisfw.h>

namespace mcvm { namespace analysis {

    using I = ValueInfo ;

    template <>
        FlowMap<I> analyze(ProgFunction* function) {
            AnalyzerContext<I> context ;
            I entry ;
            auto l = analyze<LivenessInfo>(function) ;
            return analyze_function<I,Direction::Forward,decltype(l)> 
                (context,function,entry,l) ;
        }

    template <>
        I merge (
                const I& a,
                const I& b) {
            return I{} ;
        }

    template <>
        I analyze_assignstmt (
                const AssignStmt* assign,
                AnalyzerContext<I>& context,
                const I& in,
                const FlowMap<LivenessInfo>& live) {
            return in ;
        }

}}

std::ostream& operator<<(
        std::ostream &,
        const mcvm::analysis::ValueInfo& ) {
assert(false) ;
} 
