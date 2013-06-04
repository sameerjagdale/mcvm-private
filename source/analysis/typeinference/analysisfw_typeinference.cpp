#include <analysis/typeinference/analysisfw_typeinference.h>

#include "dotexpr.h"

namespace mcvm { namespace analysis { 

    using Info = TypeFlowInfo ;
    using Lattice = ti::Lattice ;
    using ExprInfo = TypeExprInfo;
    using E = TypeExprInfo ;
    using F = TypeFlowInfo ;

    template <>
        TypeExprInfo top() {
            return {} ;
        }

    template <>
        FlowMap<F> analyze(ProgFunction* function) {
            AnalyzerContext<F> context ;
            F entry ;
            return analyze_function<F,Direction::Forward> (context,function,entry) ;
        }
    
    TypeFlowInfo analyze_function_without_flowmap(
            AnalyzerContext<TypeFlowInfo>&,
            const ProgFunction* progfunction,
            const TypeFlowInfo& input_env) {
        return input_env;
    }

    template <>
    TypeFlowInfo merge(
            const TypeFlowInfo& a,
            const TypeFlowInfo& b) {
        Info ret ;
        
        //std::cout << "Debut merge with " << a << "fdfsdf" ;
        //std::cout << "Debut merge with " << b << "fdfsdf" ;
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
        //std::cout << "fin de merge" << ret << "fdfsdf" ;
        return ret ;
    }

    template <>
    ExprInfo analyze_expr(
            const RangeExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        return {} ;
    }

    template <>
    ExprInfo analyze_expr(
            const LambdaExpr* expr,
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
    
    template <>
    ExprInfo analyze_expr (
            const FPConstExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice lattice (Lattice::mclass::DOUBLE) ;
        lattice.size_ = {1,1} ;
        return {lattice} ;
    }

    template <>
    ExprInfo analyze_expr (
            const IntConstExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        Lattice lattice (Lattice::mclass::DOUBLE) ;
        lattice.integer_ = true ;
        lattice.size_ = {1,1} ;
        return {lattice} ;
    }

    template <>
    ExprInfo analyze_expr (
            const DotExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        auto left_lattice = analyze_expr<Info,ExprInfo> (
                expr->getExpr(),
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
    
    ExprInfo libfunction(
            const LibFunction* func) {
        return {} ;
    }
    
    template <>
    ExprInfo analyze_expr (
            const ParamExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in) {
        
        auto leftexpr = expr->getSymExpr() ;
        auto left = analyze_expr<TypeFlowInfo,TypeExprInfo> 
            (leftexpr,context,in);
        
        if (left.size() != 1)
            return {};

        auto v = left.front() ;

        if (v.type_ == Lattice::mclass::FNHANDLE) {
            v = * v.fnhandle_ ;
        }

        switch (v.type_) {
            case Lattice::mclass::LIBFUNCTION:
                return {};
            case Lattice::mclass::PROGFUNCTION:
                {
                    auto function = v.function_ ;
                    auto progfunction = (ProgFunction*)function ;
                    auto rec = context.recursive_ ;
                    auto itr = rec.find(progfunction) ;

                    if (itr != std::end(rec)) {
                        context.recursion_ = true ;

                        auto returned = merge_list (context.return_points_) ;
                        
                        if (returned.empty()) {
                            std::cout << "return bottom" << std::endl ;
                            return {Lattice(Lattice::mclass::BOTTOM) };
                        } else {
                            auto ret = compute_returned_vector<TypeFlowInfo,TypeExprInfo>(
                                    returned,
                                    progfunction) ;
                            return ret ;
                        }
                        
                    } else { // Not recursive, analyze it
                        
                        // Construct the param args
                        auto input_env = construct_function_environment(
                                context,
                                in,
                                expr->getArguments(),
                                progfunction->getInParams());
                        
                        auto c = analyze_function_without_flowmap 
                            (context,progfunction,input_env) ; 
                        
                        return {} ;

                        //Get the output types
                        auto ret = 
                            compute_returned_vector<Info,TypeExprInfo>(
                                    c,
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
                    return analyze_expr <TypeFlowInfo, TypeExprInfo> (
                            v.lambda_->getBodyExpr() ,
                            context,
                            input_env);
                }
                
            default:
                //std::cout << "Unsupported param type" << v.toString() << std::endl ;
                return {} ;
        }

        return {} ;
    }

    template <>
    ExprInfo analyze_expr (
            const SymbolExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in)
    {
        
        auto s = (SymbolExpr*)expr;
        auto itr = in.find(s) ;
        if (itr != std::end(in)) {
            return {itr->second};
        }
        
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
            return {} ;
        
        Function* function = (Function*)pObject;
        Lattice l ;
        if (function->isProgFunction() ) {
            l = Lattice(Lattice::mclass::PROGFUNCTION) ;
        } else {
            l = Lattice(Lattice::mclass::LIBFUNCTION) ;
        }
            l.function_ = function ;
        return {l} ;
    }
    
    template <>
    ExprInfo analyze_expr (
            const MatrixExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in) {
        
        Lattice ret;
	auto& rows = expr->getRows();
	if (rows.empty() || rows[0].empty())
	{
                ret.size_.push_back (0) ;
                ret.integer_ = true ;
                return {ret} ;
	}
        
        size_t nb_col = rows.begin()->size() ;
        size_t nb_row = rows.size() ;
        
        ret.size_ = {nb_row,nb_col} ;
        ret.integer_ = true ;
        
	for (auto rowItr = rows.begin(); rowItr != rows.end(); ++rowItr) {
		auto& row = *rowItr;
               
                //size_t colsize ;
                for (auto colItr = row.begin(); colItr != row.end(); ++colItr) {
			auto pExpr = *colItr;
			auto typevec = analyze_expr <TypeFlowInfo,TypeExprInfo> (
                                pExpr,
                                context,
                                in) ;
                        if (typevec.size() != 1)
                            return {} ;
                        
                        
                        auto type = typevec.front() ;
                        if (rowItr == rows.begin() 
                                && colItr == row.begin()) {
                            ret.type_ = type.type_ ;
                        } else {
                            if (ret.type_ != type.type_)
                                return {} ;
                            
                            if (!type.integer_)
                                ret.integer_ = false ;
                            
                        }
                }
                /*
                if (rowItr == rows.begin()) 
                    colsize = row.size() ;
                else
                    if (colsize != row.size()) 
                        ret.size_ = {} ;
                        */
                
        }
        return {ret} ;
    }

    template <>
    ExprInfo analyze_expr (
            const UnaryOpExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in) {
        
	auto type = analyze_expr <Info,ExprInfo> (
		expr->getOperand(),
                context,
                in);
	
	switch (expr->getOperator())
	{
		case UnaryOpExpr::PLUS:
		case UnaryOpExpr::MINUS:
		case UnaryOpExpr::NOT:
			return type;
		case UnaryOpExpr::TRANSP:
		case UnaryOpExpr::ARRAY_TRANSP:
			return {};
	}
    }

    template <>
    ExprInfo analyze_expr (
            const BinaryOpExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in) {
        
     auto left = analyze_expr <F,E> (
             expr->getLeftExpr(),
             context,
             in) ;

     auto right = analyze_expr <F,E> (
             expr->getRightExpr(),
             context,
             in) ;
     
    if (left.empty() || right.empty() ) {
         return {} ;
     }

     auto l = left.front() ;
     auto r = right.front() ;
     
    if (l.type_ == Lattice::mclass::BOTTOM ||
        r.type_ == Lattice::mclass::BOTTOM )
        return {Lattice::mclass::BOTTOM} ;
            
     switch (expr->getOperator())
	{
		// Array arithmetic operation (int preserving)
		case BinaryOpExpr::PLUS:
		case BinaryOpExpr::MINUS:
		case BinaryOpExpr::ARRAY_MULT:
		case BinaryOpExpr::ARRAY_POWER:
			return ti::typemap::array_arithm_op(l,r);
		case BinaryOpExpr::ARRAY_DIV:
		case BinaryOpExpr::ARRAY_LEFT_DIV:
			return {} ; //arrayArithOpTypeMapping<false>(argTypes);
		case BinaryOpExpr::MULT:
			return ti::typemap::mult_op(l,r);
		case BinaryOpExpr::DIV:
			return ti::typemap::div_op(l,r);
		case BinaryOpExpr::LEFT_DIV:
			return {} ; //leftDivOpTypeMapping(argTypes);
		case BinaryOpExpr::POWER:
			return ti::typemap::power_op(l,r) ;
		// Logical arithmetic operation
		case BinaryOpExpr::EQUAL:
		case BinaryOpExpr::NOT_EQUAL:
		case BinaryOpExpr::LESS_THAN:
		case BinaryOpExpr::LESS_THAN_EQ:
		case BinaryOpExpr::GREATER_THAN:
		case BinaryOpExpr::GREATER_THAN_EQ:
		case BinaryOpExpr::ARRAY_OR:
		case BinaryOpExpr::ARRAY_AND:
			return ti::typemap::logical_op(l,r) ; //arrayLogicOpTypeMapping(argTypes);
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

    template <>
    ExprInfo analyze_expr (
            const StrConstExpr* expr,
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
                return recursive_assign(param->getExpr(),lattice) ;
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
    
    template <>
    TypeFlowInfo analyze_assignstmt (
            const AssignStmt* assign,
            AnalyzerContext<TypeFlowInfo>& context,
            const TypeFlowInfo& in
            ) {
        
            auto out = in ;

            auto rights = analyze_expr <F,E>
                (assign->getRightExpr(),context,in) ;
            
            auto lefts = assign->getLeftExprs() ;

            if (rights.size() < lefts.size() ) {
                std::cout << "Not enough returned values" << rights.size() << std::endl << lefts.size() << std::endl ;
                for (auto& l : lefts) {
                    out [ getRootSymbol(l) ] = Lattice (Lattice::mclass::TOP) ;
                }
                return out ;
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

                Lattice info ;
                
                // Apply when its a function call without parenthesis
                /*
                if ( assign->getRightExpr()->getExprType() == Expression::SYMBOL ) {
                        if (it1->type_ == Lattice::mclass::PROGFUNCTION) {
                        } else if (it1->type_ == Lattice::mclass::LIBFUNCTION ) {
                            info = analyzer.libfunction_((LibFunction*)it1->function_) ;
                        }
                }
                */
            
                info = recursive_assign (*it2, *it1) ;
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
        
    TypeFlowInfo construct_function_environment(
            AnalyzerContext<Info>& context,
            const TypeFlowInfo& in,
            const std::vector<Expression*>& caller,
            const std::vector<SymbolExpr*>& callee
            )
    {
        auto it1 = caller.begin() ;
        auto it2 = callee.begin() ;

        TypeFlowInfo res ;

        for (;
                it1 != std::end(caller) ;
                ++it1 , ++it2 ) {
            
            const Expression* expr = *it1 ;

            auto vec = analyze_expr <F,E> (
                    expr,
                    context,
                    in);
            
            assert (vec.size() == 1) ;
            auto front = vec.front() ;

            res [*it2] = front ;
        }

        return res;
    }

    template <>
    ExprInfo analyze_expr (
            const CellIndexExpr* expr,
            AnalyzerContext<Info>& context,
            const Info& in){
        return {} ;
    }
    
    template <>
    ExprInfo analyze_expr (
            const CellArrayExpr* expr,
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
    
}}

std::ostream& operator<<(
        std::ostream &strm,
        const mcvm::analysis::TypeFlowInfo& symbolmap)
{
        for (auto& pp : symbolmap) {
            strm << pp.first->toString() << " : " << pp.second.toString() << std::endl ;
        }
        return strm ;
}

std::ostream& operator<<(
        std::ostream &strm,
        const std::unordered_map<IIRNode*,mcvm::analysis::TypeFlowInfo>& stmtmap)
{
    for (auto& p : stmtmap) {
        strm << "--------------------------" ;
        strm << "\n Statement \n" << p.first->toString() << std::endl ;
        strm << p.second ;
    }
    return strm ;
}
