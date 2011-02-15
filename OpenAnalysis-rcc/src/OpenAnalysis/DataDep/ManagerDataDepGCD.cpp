#include "ManagerDataDepGCD.hpp"

using namespace OA::AffineExpr;
using namespace OA::Loop;

namespace OA {
namespace DataDep {

ManagerDataDepGCD::ManagerDataDepGCD(
    OA_ptr<DataDepIRInterface> ir,
    OA_ptr<Alias::Interface> aliasResults)
    :
    ManagerDataDep(ir),
    mAliasResults(aliasResults)
{
}

OA_ptr<DataDepResults> ManagerDataDepGCD::performAnalysis(
    ProcHandle p, OA_ptr<LoopAbstraction> loop)
{
#if 0
    AffineExpr::AffineAnlState errState;
    OA_ptr<ManagerAffineExpr> affineExprManager;
    affineExprManager = new ManagerAffineExpr(mIR, mAliasResults);

    // iterate across all IdxExprAccess objects, for now we assume they're
    // in the given loop
    OA_ptr<IdxExprAccessIterator> iter;
    iter = mIR->getIdxExprAccessIter(p);
cout << "Start Iteration\n";
    for(; iter->isValid(); (*iter)++) {
        // Use an affine expression manager to generate the affine expression
        // for the IdxExprAccess object we're currently iterating over.
        OA_ptr<IdxExprAccess> expr = iter->current();
        if(expr->getExpr() == MemRefHandle(0)) {
            continue;
        } else { }

        OA_ptr<ExprTree> eTree;
        eTree = mIR->getExprTree(iter->current()->getExpr());
        OA_ptr<AffineExprAbstraction> affineExp;
        affineExp = affineExprManager->exprTreeToAffineExpr(*eTree, &errState);
        // TODO, finish implementation
        assert(false);

        // Output the affine expression for now
        affineExp->output(*mIR);
    }
cout << "End Iteration\n";
#endif
}

} } // end namespaces

