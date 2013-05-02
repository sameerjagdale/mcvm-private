#ifndef ANALYSIS_TYPEINFERENCE_LATTICE_H
#define ANALYSIS_TYPEINFERENCE_LATTICE_H

// Header files
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

class Function;

namespace mcvm { namespace analysis { namespace ti {
    
struct Lattice
{
    
    enum class mclass {
        BOTTOM,
        TOP,
        INT8,
        INT32,
        DOUBLE,
        STRUCTARRAY,
        CELLARRAY,
        CHARARRAY,
	LOGICALARRAY,
        PROGFUNCTION,
        LIBFUNCTION,
        FNHANDLE
    };
   
    mclass type_ ;
    std::vector<size_t> size_ ;
    //std::vector<std::unique_ptr<Lattice>> cells_ ;
    //std::unordered_map<std::string,std::unique_ptr<Lattice>> fields_ ;
    Function* function_ ;
    bool integer_only_ ;
    
    Lattice(mclass) ;
    Lattice() = default ;
    Lattice(const Lattice&) = default ;
    
    std::string toString() const;
    bool operator==(const Lattice&) const;

};

namespace typemap {
  std::vector<Lattice> logical_op (const Lattice&, const Lattice&) ;
}

}}}

#endif
