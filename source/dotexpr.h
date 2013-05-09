#ifndef DOTEXPR_H_
#define DOTEXPR_H_

#include <string>
#include "expressions.h"
#include "symbolexpr.h"

class DotExpr : public Expression
{
public:
	
	DotExpr(Expression* e, std::string f)
	: expr_(e), field_(f)
	{ m_exprType = Expression::ExprType::DOT ; }
	
	// Method to recursively copy this node
	DotExpr* copy() const;
	
	ExprVector getSubExprs() const;
	
        SymbolExpr* getRootSymbol() const ;
        
	void replaceSubExpr(size_t index, Expression* pNewExpr);
	
	virtual std::string toString() const;
	
	std::string getField() const { return field_; }
	
	// Accessor to get the arguments
	Expression* getExpr() const { return expr_; }

protected:
	Expression* expr_ ;
	std::string field_ ;
};

#endif // #ifndef DOTEXPR_H_
