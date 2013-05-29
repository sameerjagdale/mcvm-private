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
      this->function_ = l.function_ ;
      this->integer_ = l.integer_ ;
      this->size_ = l.size_ ;
      if (l.fnhandle_)
          this->fnhandle_ = std::unique_ptr<Lattice>(new Lattice(*l.fnhandle_)) ;
      for (auto& f : l.fields_)
          this->fields_ [ f.first ] = std::unique_ptr<Lattice>(new Lattice(*f.second)) ;
  }
  
  // Copy assignment operator
  Lattice& Lattice::operator= (const Lattice & l) {
      this->type_ = l.type_ ;
      this->lambda_ = l.lambda_ ;
      this->function_ = l.function_ ;
      this->integer_ = l.integer_ ;
      this->size_ = l.size_ ;
      if (l.fnhandle_)
          this->fnhandle_ = std::unique_ptr<Lattice>(new Lattice(*l.fnhandle_)) ;
      this->fields_.clear() ;
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
        if (integer_)
            res += " (integer)" ;
        res += mcvm::analysis::ti::toString(size_) ; 
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
      case mclass::PROGFUNCTION:
	res += "progfunction";
	break;
      case mclass::LIBFUNCTION:
	res += "libfunction";
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
  
void Lattice::merge (const Lattice& old) {
    /*
    PRDEBUG("Merging :"
            "\n%s\n"
            "Into:"
            "\n%s\n",
            typeinfo.toString().c_str(),
            this->toString().c_str());
            */


    // If either of them is not a structarray, don't do anything
    if ( type_ != Lattice::mclass::STRUCTARRAY ||
            old.type_ != Lattice::mclass::STRUCTARRAY ) {
        return ;
    }


    // Merge the size first
    if (!old.size_.empty()) {
        auto typeinfo_size = old.size_ ;
        int current_dim_number = size_.size() ;

        int id = 0;
        for (auto it = std::begin(typeinfo_size) ,
                end = std::end(typeinfo_size) ;
                it != end ;
                ++it,++id) 
        {
            if (id > current_dim_number) {
                // Take the merged value
                size_.at(id) = typeinfo_size.at(id);
            } else {
                // Take the max
            size_.at(id) = 
                std::max(
                        size_.at(id),
                        typeinfo_size.at(id)
                        );
            }
        }
    } else {
        size_.clear();
    }

    // Then merge the fields
    auto& fields = old.fields_ ;
    for (auto it = std::begin(fields),
            end = std::end(fields) ;
            it != end ;
            ++it)
    {
        auto field_exist = fields_.find (it->first) ;

        if (field_exist == std::end(fields_)) {
            auto insert_field = std::unique_ptr<Lattice>(
                    new Lattice(*it->second)) ;
            fields_ [it->first] = std::move(insert_field) ;
        } else {
            // Recursive merge of this field
            auto& recursive_field = field_exist->second ;
            const std::unique_ptr<Lattice>& recursive_merge = it->second ;
            Lattice lat = *recursive_merge ;
            recursive_field->merge(lat);
        }
    }
}
  
  bool Lattice::operator == (const Lattice& a) const {
    return 
        this->type_ == a.type_ &&
        this->integer_ == a.integer_ && 
        this->size_ == a.size_ ;
  }
  
bool is_composite(const Lattice& l) {
    switch (l.type_) {
        case Lattice::mclass::STRUCTARRAY:
        case Lattice::mclass::LOGICALARRAY:
        case Lattice::mclass::DOUBLE:
        return true;
        default:
            return false;
    }
}

  namespace typemap {
    std::vector<Lattice> logical_op (const Lattice&, const Lattice&) {
      Lattice ret ;
      ret.type_ = Lattice::mclass::LOGICALARRAY ;
      return {ret} ;
    }
    
    std::vector<Lattice> arithm_op (const Lattice& a , const Lattice& b ) {
        return {a} ;
      Lattice ret ;
      ret.type_ = Lattice::mclass::DOUBLE;
      
      if (a.integer_ && b.integer_)
          ret.integer_ = true ;

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
