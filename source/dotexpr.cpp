#include "dotexpr.h"

DotExpr* DotExpr::copy() const
{
	return new DotExpr(expr_->copy(),field_);
}	

std::string DotExpr::toString() const
{
	std::string output;
	output = expr_->toString() + "." + field_ ;
	return output;
}

Expression::ExprVector DotExpr::getSubExprs() const
{
	ExprVector list;
	list.push_back(expr_);
	return list;
}

void DotExpr::replaceSubExpr(size_t index, Expression* pNewExpr)
{
	if (index == 0)
	{
		expr_ = pNewExpr;
	} else {
		assert(false) ;
	}
	return;
}
