#include <analysis/typeinference/analysisfw_typeinference.h>

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
        
        auto u = (new Lattice (Lattice::mclass::LAMBDA)) ;
        u->lambda_ = expr ;
        
        for (auto& var:in) {
            u->env_ [var.first ] = & var.second;
        }

        l.fnhandle_ = u ;

        return {l} ;
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
        Lattice function_typeinfo (Lattice::mclass::PROGFUNCTION) ;
        function_typeinfo.function_ = function ;
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
			return {} ; //arrayArithOpTypeMapping<true>(argTypes);
		case BinaryOpExpr::ARRAY_DIV:
		case BinaryOpExpr::ARRAY_LEFT_DIV:
			return {} ; //arrayArithOpTypeMapping<false>(argTypes);
		case BinaryOpExpr::MULT:
			return typemap::mult_op(l,r);
		case BinaryOpExpr::DIV:
			return {} ; //divOpTypeMapping(argTypes);
		case BinaryOpExpr::LEFT_DIV:
		{
			// Perform type inference for the division operation
			return {} ; //leftDivOpTypeMapping(argTypes);
		}
		break;	
			
		// Exponentiation operation	
		case BinaryOpExpr::POWER:
		{
			// Perform type inference for the power operation
			return {} ; //powerOpTypeMapping(argTypes);
		}
		break;

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
                out[ (SymbolExpr*) *it2] = *it1 ;
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

    
    Analyzer<Info,ExprInfo> get_analyzer() {
        Analyzer<Info,ExprInfo> ret ;
        ret.sequencestmt_ = &sequencestmt<Info,ExprInfo> ;
        ret.expression_ = &expression<Info,ExprInfo> ;
        
        ret.assignstmt_ = &ti::assignstmt ;
        ret.lambda_ = &ti::lambda ;
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
