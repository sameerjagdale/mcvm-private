#ifndef ANALYSIS_TYPEINFERENCE_LATTICE_H
#define ANALYSIS_TYPEINFERENCE_LATTICE_H

// Header files
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

class Function ;
class LambdaExpr ;
class SymbolExpr ;

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
        LAMBDA,
        FNHANDLE
    };
   
    mclass type_ ;
    std::vector<size_t> size_ ;
    std::vector<std::unique_ptr<Lattice>> cells_ ;
    std::unordered_map<std::string,std::unique_ptr<Lattice>> fields_ ;
    const Function* function_ = nullptr ;
    bool integer_only_ = false ;
    

    const LambdaExpr* lambda_ = nullptr ;
    std::unique_ptr<Lattice> fnhandle_ = nullptr ;
    std::unordered_map<SymbolExpr*,const Lattice*> env_ ;
    
    Lattice(mclass) ;
    Lattice() = default ;
    Lattice(const Lattice&) ;
    Lattice& operator= (const Lattice& other) ;
    Lattice(Lattice&&) = default ;
    
    std::string toString() const;
    bool operator==(const Lattice&) const;

};

namespace typemap {
  std::vector<Lattice> logical_op (const Lattice&, const Lattice&) ;
  std::vector<Lattice> mult_op (const Lattice&, const Lattice&) ;
  std::vector<Lattice> div_op (const Lattice&, const Lattice&) ;
  std::vector<Lattice> array_arithm_op (const Lattice&, const Lattice&) ;
  std::vector<Lattice> power_op (const Lattice&, const Lattice&) ;
}

}}}

#endif
