#include <analysis/typeinference/analysisfw_typeinference.h>

namespace mcvm { namespace analysis { namespace ti {

    Info merge(
            const Info& a,
            const Info& b) {
        Info ret ;
        for (auto& var : a) {
            auto itr = b.find(var.first) ;
            if (itr != std::end(b)) {
                if ( *var.second == *itr->second || 
                        var.second->type_ == Lattice::mclass::BOTTOM ||
                        itr->second->type_ == Lattice::mclass::BOTTOM ) {
                    ret.insert(var) ;
                }
            }
        }
        return ret ;
    }

    ExprInfo intconst(
            const IntConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        auto lattice = new Lattice(Lattice::mclass::DOUBLE) ;
        lattice->integer_only_ = true ;
        lattice->size_ = {1,1} ;
        return {lattice} ;
    }

    ExprInfo paramexpr(
            const ParamExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        auto leftexpr = expr->getSymExpr() ;
        auto left = analyzer.expression_(leftexpr,analyzer,context,in);
        auto v = * left.begin() ;

        switch (v->type_) {
            case Lattice::mclass::PROGFUNCTION:
                auto function = v->function_ ;
                if (function->isProgFunction()) {
                    auto progfunction = (ProgFunction*)function ;
                    auto itr = context.recursive_.find(progfunction) ;
                    if (itr != std::end(context.recursive_)) {
                        std::cout << "RECURSION" << std::endl ;
                        if (!context.recursion_) {
                            // first time we see the recursion
                            context.recursion_ = true ;
                            return {new Lattice(Lattice::mclass::BOTTOM) };
                        } else {
                            // returns a vector
                            auto ret = context.returns_ ;
                            return ret ;
                        }


                        
                    }
                    // Pass the parameters values

                }
                break;
        }
    }

    ExprInfo symbol_function(
            const SymbolExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        DataObject* pObject;
        try
        {
            assert (local_env_ != nullptr) ;
            pObject = Interpreter::evalSymbol(expr,context.local_env_);
        }
        catch (RunError e)
        {
            pObject = nullptr;
        }
        if (pObject == nullptr || pObject->getType() != DataObject::Type::FUNCTION)
        {
            throw TypeError() ;
        }
        Function* function = (Function*)pObject;
        TypeInfoPtr function_typeinfo = 
            new TypeInfo(Lattice::mclass::PROGFUNCTION) ;
        function_typeinfo->function_ = function ;
        return {function_typeinfo} ;
    }
    
    ExprInfo matrixexpr(
            const MatrixExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        return {} ;
    }

    ExprInfo binop(
            const BinaryOpExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        return {} ;
    }

    ExprInfo str(
            const StrConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        auto res = new Lattice(Lattice::mclass::CHARARRAY) ;
        res->size_.push_back(1);
        res->size_.push_back(expr->getValue().length());
        return {res} ;
    }
    
    Analyzer<Info,ExprInfo> get_analyzer() {
        Analyzer<Info,ExprInfo> ret ;
        ret.sequencestmt_ = &sequencestmt<Info,ExprInfo> ;
        ret.assignstmt_ = &assignstmt<Info,ExprInfo> ;
        ret.expression_ = &expression<Info,ExprInfo> ;
        ret.intconst_ = &ti::intconst ;
        ret.paramexpr_ = &ti::paramexpr ;
        ret.symbol_function_ = &ti::symbol_function ;
        ret.matrix_ = &ti::matrixexpr ;
        ret.str_= &ti::str;
        ret.binop_ =  &ti::binop ;
        ret.merge_ = &ti::merge ;
        return ret ;
    }

}}}

std::ostream& operator<<(
        std::ostream &strm,
        const mcvm::analysis::ti::Info& symbolmap)
{
        for (auto& pp : symbolmap) {
            strm << pp.first->toString() << " : " << pp.second->toString() << std::endl ;
        }
        return strm ;
}

std::ostream& operator<<(
        std::ostream &strm,
        const std::unordered_map<IIRNode*,mcvm::analysis::ti::Info>& stmtmap)
{
    for (auto& p : stmtmap) {
        strm << "--------------------------" ;
        strm << "\n Statement \n" << p.first->toString() << std::endl ;
        strm << p.second ;
    }
    return strm ;
}
