#include <unordered_set>
#include <unordered_map>

class SymbolExpr ;

namespace mcvm { namespace analysis {

    using LivenessInfo = std::unordered_set<const SymbolExpr*> ;

}}

std::ostream& operator<<(
        std::ostream &,
        const mcvm::analysis::LivenessInfo& );

