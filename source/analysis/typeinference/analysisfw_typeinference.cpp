#include <analysis/typeinference/analysisfw_typeinference.h>

#include "dotexpr.h"

namespace mcvm { namespace analysis { namespace ti {

    Info merge(
            const Info& a,
            const Info& b) {
        Info ret ;
        for (auto& var : a) {
            auto itr = b.find(var.first) ;
            if (itr != std::end(b)) {
                if ( var.second == itr->second || 
                        var.second.type_ == Lattice::mclass::BOTTOM ||
                        itr->second.type_ == Lattice::mclass::BOTTOM ) {
                    ret.insert(var) ;
                }
            }
        }
        return ret ;
    }

    ExprInfo lambda(
            const LambdaExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice l (Lattice::mclass::FNHANDLE) ;
        
        l.fnhandle_ = std::unique_ptr<Lattice>
            (new Lattice(Lattice::mclass::LAMBDA));
        
        l.fnhandle_->lambda_ = expr ;
        
        for (auto& var:in) {
            l.fnhandle_->env_ [var.first ] = & var.second;
        }

        return {l} ;
    }
    
    ExprInfo fpconst(
            const FPConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice lattice (Lattice::mclass::DOUBLE) ;
        lattice.size_ = {1,1} ;
        return {lattice} ;
    }

    ExprInfo intconst(
            const IntConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice lattice (Lattice::mclass::DOUBLE) ;
        lattice.integer_only_ = true ;
        lattice.size_ = {1,1} ;
        return {lattice} ;
    }

    ExprInfo dotexpr(
            const DotExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        auto left_lattice = analyzer.expression_ (
                expr->getExpr(),
                analyzer,
                context,
                in) ;
       
        if (left_lattice.size() != 1) 
            return {} ;
        
        auto lattice = left_lattice.front() ;
        if (lattice.type_ != Lattice::mclass::STRUCTARRAY)
            return {} ;
        
        auto& f = lattice.fields_ ;
        auto itr = f.find (expr->getField()) ;
        
        if (itr == std::end(f))
            return {} ;

        auto ret = *itr->second ;
        return {ret} ;
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

        if (v.type_ == Lattice::mclass::FNHANDLE) {
            v = * v.fnhandle_ ;
        }

        switch (v.type_) {
            case Lattice::mclass::LIBFUNCTION:
                std::cout << "lib" << std::endl ;
                assert(false) ;
            case Lattice::mclass::PROGFUNCTION:
                {
                    auto function = v.function_ ;
                    auto progfunction = (ProgFunction*)function ;
                    auto itr = context.recursive_.find(progfunction) ;
                    if (itr != std::end(context.recursive_)) {
                        std::cout << "RECURSION" << std::endl ;
                        if (!context.recursion_) {
                            // first time we see the recursion
                            context.recursion_ = true ;
                            return {Lattice(Lattice::mclass::BOTTOM) };
                        } else {
                            // returns a vector
                            auto ret = 
                                compute_returned_vector<Info,ExprInfo,Lattice>(
                                        context.return_points_,
                                        analyzer.merge_,
                                        progfunction);
                            return ret ;
                        }
                    } else { // Not recursive, analyze it
                        
                        // Construct the param args
                        auto input_env = construct_function_environment(
                                analyzer,
                                context,
                                in,
                                expr->getArguments(),
                                progfunction->getInParams());
                        
                        //Analyze it
                        AnalyzerContext<FlowInfo> c ;
                        c = analyze(analyzer,c,progfunction,input_env) ; 

                        //Get the output types
                        auto ret = 
                            compute_returned_vector<Info,ExprInfo,Lattice>(
                                    c.return_points_,
                                    analyzer.merge_,
                                    progfunction);
                        return ret ;
                    }
                }
                break;

            case Lattice::mclass::LAMBDA:
                {
                    // Create the input flowinfo
                    // from lambda environment and params
                   
                    auto input_env = construct_function_environment(
                            analyzer,
                            context,
                            in,
                            expr->getArguments(),
                            v.lambda_->getInParams());
                    
                    // Add the lambda environment
                    for (auto& var:v.env_) {
                        auto itr = input_env.find (var.first) ;
                        if ( itr == std::end(input_env)) {
                            input_env [var.first] = * var.second ;
                        }
                    }

                            
                    // Analyze the body
                    return analyzer.expression_(
                            v.lambda_->getBodyExpr() ,
                            analyzer,
                            context,
                            input_env);
                }
                
            default:
                assert(false) ;
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
            assert (context.local_env_ != nullptr) ;
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
        Lattice l ;
        if (function->isProgFunction() ) {
            l = Lattice(Lattice::mclass::PROGFUNCTION);
            l.function_ = function ;
        } else {
            l = Lattice(Lattice::mclass::LIBFUNCTION) ;
        }
        l.function_ = function ;
        return {l} ;
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
     auto left = analyzer.expression_(
             expr->getLeftExpr(),
             analyzer,
             context,
             in) ;

     auto right = analyzer.expression_(
             expr->getRightExpr(),
             analyzer,
             context,
             in) ;
     
    if (left.empty() || right.empty() ) {
         return {} ;
     }

     auto l = left.front() ;
     auto r = right.front() ;
     
     switch (expr->getOperator())
	{
		// Array arithmetic operation (int preserving)
		case BinaryOpExpr::PLUS:
		case BinaryOpExpr::MINUS:
		case BinaryOpExpr::ARRAY_MULT:
		case BinaryOpExpr::ARRAY_POWER:
			return typemap::array_arithm_op(l,r);
		case BinaryOpExpr::ARRAY_DIV:
		case BinaryOpExpr::ARRAY_LEFT_DIV:
			return {} ; //arrayArithOpTypeMapping<false>(argTypes);
		case BinaryOpExpr::MULT:
			return typemap::mult_op(l,r);
		case BinaryOpExpr::DIV:
			return typemap::div_op(l,r);
		case BinaryOpExpr::LEFT_DIV:
			return {} ; //leftDivOpTypeMapping(argTypes);
		case BinaryOpExpr::POWER:
			return typemap::power_op(l,r) ;
		// Logical arithmetic operation
		case BinaryOpExpr::EQUAL:
		case BinaryOpExpr::NOT_EQUAL:
		case BinaryOpExpr::LESS_THAN:
		case BinaryOpExpr::LESS_THAN_EQ:
		case BinaryOpExpr::GREATER_THAN:
		case BinaryOpExpr::GREATER_THAN_EQ:
		case BinaryOpExpr::ARRAY_OR:
		case BinaryOpExpr::ARRAY_AND:
			return typemap::logical_op(l,r) ; //arrayLogicOpTypeMapping(argTypes);
		case BinaryOpExpr::OR:
		case BinaryOpExpr::AND:
		{
			// Return the type info for the logical value
			return {Lattice(
				Lattice::mclass::LOGICALARRAY)} ;
		}
		break;
	}
        return {} ;
    }

    ExprInfo str(
            const StrConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice res (Lattice::mclass::CHARARRAY) ;
        res.size_.push_back(1);
        res.size_.push_back(expr->getValue().length());
        return {res} ;
    }
    
    Lattice recursive_assign (const Expression* expr, const Lattice& l) {
        switch (expr->getExprType()) {
            case Expression::SYMBOL:
                return l ;
            case Expression::PARAM:
                {
                auto param = (ParamExpr*)expr ;
                auto lattice = l ;
                lattice.size_.push_back(45) ;
                return lattice ;
                }
            case Expression::DOT:
                {
                auto dot = (DotExpr*)expr ;
                Lattice lattice (Lattice::mclass::STRUCTARRAY);
                lattice.fields_ [ dot->getField() ] = 
                    std::unique_ptr<Lattice>( new Lattice(l)) ;
                lattice.size_.push_back(1) ;
                lattice.size_.push_back(1) ;
                return recursive_assign(dot->getExpr(), lattice) ;
                }
            default:
                return {};
        }
    }
    
    Info assignstmt(
            const AssignStmt* assign,
            const Analyzer<FlowInfo,ExprInfo>& analyzer,
            AnalyzerContext<FlowInfo>& context,
            const Info& in
            )
    {
            auto out = in ;

            auto rights = analyzer.expression_ (assign->getRightExpr(),analyzer,context,in) ;
            auto lefts = assign->getLeftExprs() ;

            if (rights.size() < lefts.size() ) {
                std::cout << "Not enough returned values" << rights.size() << std::endl << lefts.size() << std::endl ;
                return in ;
                //throw std::exception() ;
            }

            auto it1 =  rights.begin() ;
            auto it2 = lefts.begin() ;

            /* We iterate only over lefts expression because there can be less left
             * than right
             * ex : [a,b] = <std::vector>.size 3 is valid
             */
            for (;
                    it2 != lefts.end() ;
                    ++it1 , ++it2 ) {
                auto info = recursive_assign (*it2, *it1) ;
                auto root = getRootSymbol (*it2) ;
                
                if ( (*it2)->getExprType() != Expression::SYMBOL) {
                   auto itr = in.find (root) ;
                   if (itr != std::end(in)) {
                       info.merge(itr->second) ;
                   }
                }
                out [root] = info ;
            }

            return out;
        }
        
    FlowInfo construct_function_environment(
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const FlowInfo& in,
            const std::vector<Expression*>& caller,
            const std::vector<SymbolExpr*>& callee
            )
    {
        auto it1 = caller.begin() ;
        auto it2 = callee.begin() ;

        FlowInfo res ;

        for (;
                it1 != std::end(caller) ;
                ++it1 , ++it2 ) {
            
            const Expression* expr = *it1 ;

            auto vec = analyzer.expression_(
                    expr,
                    analyzer,
                    context,
                    in);
            
            assert (vec.size() == 1) ;
            auto front = vec.front() ;

            res [*it2] = front ;
        }

        return res;
    }

    ExprInfo cellindex(
            const CellIndexExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in){
        return {} ;
    }
    
    ExprInfo cellarray(
            const CellArrayExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in) {
        auto rows = expr->getRows() ;
        Lattice l (Lattice::mclass::CELLARRAY) ;
        /*
        for (auto& row : rows) {
            auto r = row - rows.begin() ;
            std::vector<std::unique_ptr<Lattice>> vect ;
            for (auto& cell : row) {
                auto c = cell - row.begin() ;
                auto exprvect = analyzer.expression_(
                        cell,
                        analyzer,
                        context,
                        in) ;
                auto l = exprvect.front() ;
                auto ptr = std::unique_ptr<Lattice>(new Lattice(l)) ;
                vect.push_back(ptr) ;
            }
            l.cells_.push_back(vect) ;
        }
        */
        return {l} ;
    }
    
    Analyzer<Info,ExprInfo> get_analyzer() {
        Analyzer<Info,ExprInfo> ret ;
        ret.sequencestmt_ = &sequencestmt<Info,ExprInfo> ;
        ret.loopstmt_ = &loopstmt<Info,ExprInfo> ;
        ret.expression_ = &expression<Info,ExprInfo> ;
        
        ret.fpconst_ = &ti::fpconst ;
        ret.assignstmt_ = &ti::assignstmt ;
        ret.lambda_ = &ti::lambda ;
        ret.intconst_ = &ti::intconst ;
        ret.paramexpr_ = &ti::paramexpr ;
        ret.dotexpr_ = &ti::dotexpr ;
        ret.cellindex_ = &ti::cellindex ;
        ret.cellarray_ = &ti::cellarray ;
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
            strm << pp.first->toString() << " : " << pp.second.toString() << std::endl ;
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
