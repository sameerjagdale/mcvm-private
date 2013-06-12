#ifndef ANALYSISFW_TYPEINFERENCE_H
#define ANALYSISFW_TYPEINFERENCE_H

#include <analysis/analysisfw.h>
#include <analysis/typeinference/lattice.h>
#include <paramexpr.h>


class SymbolExpr;
class DotExpr;
class ParamExpr;
class MatrixExpr;

namespace mcvm { namespace analysis {  

    using TypeLattice = ti::Lattice ;
    using TypeFlowInfo = std::unordered_map<SymbolExpr*,TypeLattice> ;
    using TypeExprInfo = std::vector<TypeLattice> ;

//    class TypeError : public std::exception {};
    
    TypeLattice recursive_assign (const Expression*, const TypeLattice&) ;
    
    TypeFlowInfo construct_function_environment(
            AnalyzerContext<TypeFlowInfo>& context,
            const TypeFlowInfo& in,
            const std::vector<Expression*>& caller,
            const std::vector<SymbolExpr*>& callee
            );

    TypeFlowInfo analyze_function_without_flowmap(
            AnalyzerContext<TypeFlowInfo>&,
            const ProgFunction* progfunction,
            const TypeFlowInfo& input_env) ; 
    
}}

std::ostream& operator<<(
        std::ostream &strm,
        const mcvm::analysis::TypeFlowInfo& symbolmap);
std::ostream& operator<<(
        std::ostream &strm,
        const std::unordered_map<IIRNode*,mcvm::analysis::TypeFlowInfo>& stmtmap);

#endif
