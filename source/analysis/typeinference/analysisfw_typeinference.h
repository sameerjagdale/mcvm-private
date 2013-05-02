#ifndef ANALYSISFW_TYPEINFERENCE_H
#define ANALYSISFW_TYPEINFERENCE_H

#include <analysis/analysisfw.h>
#include <analysis/typeinference/lattice.h>
#include <paramexpr.h>


class SymbolExpr;
class DotExpr;
class ParamExpr;
class MatrixExpr;

namespace mcvm { namespace analysis { namespace ti {

    using FlowInfo = std::unordered_map<SymbolExpr*,Lattice> ;
    using Info = FlowInfo ;
    using ExprInfo = std::vector<Lattice> ;

    class TypeError : public std::exception {};

    Info merge(
            const Info& a,
            const Info& b);

    ExprInfo intconst(
            const IntConstExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in);

    ExprInfo paramexpr(
            const ParamExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in);

    ExprInfo symbol_function(
            const SymbolExpr* expr,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in);

    Info assignstmt(
            const AssignStmt* assign,
            const Analyzer<Info,ExprInfo>& analyzer,
            AnalyzerContext<Info>& context,
            const Info& in
            );

    Analyzer<Info,ExprInfo> get_analyzer();

}}}

std::ostream& operator<<(
        std::ostream &strm,
        const mcvm::analysis::ti::Info& symbolmap);
std::ostream& operator<<(
        std::ostream &strm,
        const std::unordered_map<IIRNode*,mcvm::analysis::ti::Info>& stmtmap);

#endif
