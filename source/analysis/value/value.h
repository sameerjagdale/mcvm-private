#include <unordered_map>
#include <objects.h>

class IIRNode;
class SymbolExpr;

namespace mcvm { namespace analysis {

    using ValueInfo = std::unordered_map<const SymbolExpr*, const DataObject* > ;
    using ValueMap = std::unordered_map<const IIRNode*, ValueInfo> ;


}}

std::ostream& operator<<(
        std::ostream &,
        const mcvm::analysis::ValueInfo& );

