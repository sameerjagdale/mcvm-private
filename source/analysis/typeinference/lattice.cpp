#include <analysis/typeinference/lattice.h>
#include <iostream>
#include "utility.h"


namespace mcvm { namespace analysis { namespace ti {
  
    std::string toString(std::vector<size_t> size) {
        std::string res ;
        res+= " { " ;
        for (auto& s : size) {
            res+= std::to_string(s) ;
            res+= " " ;
        }
        res+= "}" ;
        return res;
    }
      
  Lattice::Lattice(mclass m) : type_(m) {} 
  
  // Copy ctor
  Lattice::Lattice(const Lattice& l) {
      this->type_ = l.type_ ;
      this->lambda_ = l.lambda_ ;
      if (l.fnhandle_)
          this->fnhandle_ = std::unique_ptr<Lattice>(new Lattice(*l.fnhandle_)) ;
      for (auto& f : l.fields_)
          this->fields_ [ f.first ] = std::unique_ptr<Lattice>(new Lattice(*f.second)) ;
  }
  
  // Copy assignment operator
  Lattice& Lattice::operator= (const Lattice & l) {
      this->type_ = l.type_ ;
      this->lambda_ = l.lambda_ ;
      if (l.fnhandle_)
          this->fnhandle_ = std::unique_ptr<Lattice>(new Lattice(*l.fnhandle_)) ;
      for (auto& f : l.fields_)
          this->fields_ [ f.first ] = std::unique_ptr<Lattice>(new Lattice(*f.second)) ;
      return *this ;
  }

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
      case mclass::STRUCTARRAY:
	res += "struct of size " ;
        res += mcvm::analysis::ti::toString(size_) ; 
        res += " with fields : ";
        res += "\n" ;
        for (auto& f : fields_) {
            std::string s ;
            s += f.first ; 
            s += " : " ;
            s += f.second->toString() ; 
            res += indentText(s) ;
            res += "\n" ;
        }
        break;
      default:
	break;
    }
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
    
    std::vector<Lattice> div_op (const Lattice&, const Lattice&) {
        Lattice ret ;
        // the mclass "*" matrix (cf Anton thesis)
        ret.type_ = Lattice::mclass::DOUBLE;
        return {ret} ;
    }
        
    std::vector<Lattice> array_arithm_op (const Lattice&, const Lattice&) {
        Lattice ret ;
        // the mclass "*" matrix (cf Anton thesis)
        ret.type_ = Lattice::mclass::DOUBLE;
        return {ret} ;
    }
    
    std::vector<Lattice> power_op (const Lattice&, const Lattice&) {
        Lattice ret ;
        // the mclass "*" matrix (cf Anton thesis)
        ret.type_ = Lattice::mclass::DOUBLE;
        return {ret} ;
  }
}

}}}
