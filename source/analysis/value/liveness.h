#include <unordered_set>
#include <unordered_map>

class IIRNode ;
class SymbolExpr ;

namespace mcvm { namespace analysis {
    
        using LivenessInfo = std::unordered_set<const SymbolExpr*> ;
        using LivenessMap = std::unordered_map<const IIRNode*, LivenessInfo> ;
        
}}

std::ostream& operator<<(
        std::ostream &,
        const mcvm::analysis::LivenessInfo& );

