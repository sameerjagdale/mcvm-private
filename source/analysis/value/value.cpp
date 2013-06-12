#include "value.h"
#include "liveness.h"
#include <analysis/analysisfw.h>

namespace mcvm { namespace analysis {

    using I = ValueInfo ;
    using E = DataObject* ;

    template<>
        I top() {
            return I{};
        }

    template <>
        E top() {
        return nullptr ;
    }

    template <>
    E analyze_expr (
            const IntConstExpr* expr,
            AnalyzerContext<I>& context,
            const I& in)
    {
        return (DataObject*)expr->copy() ;
    }

    template <>
        FlowMap<I> analyze(ProgFunction* function) {
            AnalyzerContext<I> context ;
            I entry ;
            auto l = analyze<LivenessInfo>(function) ;
            return analyze_function<I,Direction::Forward,decltype(l)> 
                (context,function,entry,l) ;
        }

    template <>
        I merge (
                const I& a,
                const I& b) {
            I ret ;
            for (auto& val:a) {
                auto itr = b.find (val.first) ;
                if (itr != std::end (b))
                    if (val.second == itr->second)
                        ret.insert (val) ;
            }
            return ret ;
        }
        

    template <>
        I analyze_assignstmt (
                const AssignStmt* assign,
                AnalyzerContext<I>& context,
                const I& in,
                const FlowMap<LivenessInfo>& live) {
            auto out = in ;

            //Find the current liveness 
            auto litr = live.find (assign) ;
            auto l = litr->second.in ;
            //std::cout << "assign " << assign->toString() <<
                //"live" << l ;
            auto assigned = assign->getLeftExprs() ;
            if (assigned.size() != 1) {
                return top<I>();
            }
            auto var = assigned.front() ;
            if (std::find(std::begin(l), std::end(l), var)!=std::end(l)) {
                auto gen = analyze_expr <E>(
                        assign->getRightExpr(),
                        context,
                        in
                        ) ;
                out [(SymbolExpr*)var] = gen ;
            }
            return out ;
        }

}}

std::ostream& operator<<(
        std::ostream& strm,
        const mcvm::analysis::ValueInfo& v) {
    for (auto& itr:v)
        strm << itr.first->toString() << itr.second->toString() << std::endl ;
    return strm;

    
} 
