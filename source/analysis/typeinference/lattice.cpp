#include <analysis/typeinference/lattice.h>

namespace mcvm { namespace analysis { namespace ti {
  
  Lattice::Lattice(mclass m) : type_(m) {} 
  
  std::string Lattice::toString() const {
    std::string res ;
    switch (type_) {
      case mclass::TOP:
	res += "TOP";
	break;
      case mclass::BOTTOM:
	res += "BOTTOM";
	break;
      case mclass::DOUBLE:
	res += "double";
	break;
      case mclass::CHARARRAY:
	res += "string";
	break;
      case mclass::LOGICALARRAY:
	res += "logical";
	break;
      case mclass::FNHANDLE:
	res += "@fnhandle -> ";
        res += fnhandle_->toString() ;
	break;
      case mclass::LAMBDA:
	res += "lambda";
	break;
      default:
	break;
    }
    res+= " { " ;
    for (auto& s : size_) {
      res+= std::to_string(s) ;
      res+= " " ;
    }
    res+= "}" ;
    return res ;
  }
  
  
  bool Lattice::operator == (const Lattice& a) const {
    return this->type_ == a.type_ ;
  }
  
  namespace typemap {
    std::vector<Lattice> logical_op (const Lattice&, const Lattice&) {
      Lattice ret ;
      ret.type_ = Lattice::mclass::LOGICALARRAY ;
      return {ret} ;
    }
    
    std::vector<Lattice> arithm_op (const Lattice&, const Lattice&) {
      Lattice ret ;
      ret.type_ = Lattice::mclass::DOUBLE;
      return {ret} ;
    }
    
    std::vector<Lattice> mult_op (const Lattice&, const Lattice&) {
        Lattice ret ;
        // the mclass "*" matrix (cf Anton thesis)
        ret.type_ = Lattice::mclass::DOUBLE;
        return {ret} ;
    }
}

}}}
