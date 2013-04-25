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
                res += "bottom";
                break;
            case mclass::DOUBLE:
                res += "double";
                break;
            case mclass::CHARARRAY:
                res += "string";
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

    }}}
