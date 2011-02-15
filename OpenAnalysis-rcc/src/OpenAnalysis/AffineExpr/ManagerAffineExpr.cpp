#include "ManagerAffineExpr.hpp"

namespace OA {
namespace AffineExpr {

ManagerAffineExpr::ManagerAffineExpr(OA_ptr<AffineExprIRInterface> _ir)
    :
    mIR(_ir)
{
}


OA_ptr<AffineExprAbstraction> ManagerAffineExpr::analyzeMemRefExp(
        ExprHandle hExp,
        ProcHandle hProc,
        const set<OA_ptr<NamedLoc> > & indexVars,
        const set<OA_ptr<NamedLoc> > & nonConstVars,
        OA_ptr<Alias::Interface> aliasResults,
        AffineAnlState *state)
{
    OA_ptr<AffineExprAbstraction> afnExp;
    afnExp = new AffineExprAbstraction();

    // convert the expression into an expression tree
    OA_ptr<ExprTree> expTree = mIR->getExprTree(hExp);

    // an affine expression abstraction is constructed by visiting each
    // of the nodes in an expression tree and determining the expressions
    // terms and factors.  The AffineExprExprTreeVisitor does this.
    AffineExprExprTreeVisitor visitor(
        afnExp, mIR, hProc, indexVars, nonConstVars, aliasResults);
    expTree->acceptVisitor(visitor);

    // the success or failure of converting the expression tree into an
    // affine expression abstraction is stored in a state variable passed
    // as an argument.
    *state = visitor.getState();

    return afnExp;
}


// Visitor class:
// ----------------------------------

void AffineExprExprTreeVisitor::visitOpNode(ExprTree::OpNode& n) {
    // don't continue when in an error state
    if(mState != VALID_AFFINE_EXP) { return; }

    // iterate over children in order to do a post-order traversal
    for(int i = 0; i < n.num_children(); i++) {
        ExprTree::ChildNodesIterator iterChildren(n);

        for(; iterChildren.isValid(); ++iterChildren) {
            OA_ptr<ExprTree::Node> child;
            child = iterChildren.current();
            child->acceptVisitor(*this);
        }
    }

    // determine what to do based on the type of operator
    OpHandle hOp = n.getHandle();
    switch(mIR->getOpType(hOp)) {
        // if it's a plus sign analyze for a term
        case OP_ADD:
        case OP_SUBTRACT:
            term();
            break;

        // if it's a multiply sign analyze for a term
        case OP_MULTIPLY:
            term();
            break;

        // in all other cases change the error state
        case OP_DIVIDE:
        case OP_MODULO:
        case OP_SHIFT_LEFT:
        case OP_SHIFT_RIGHT:
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_OTHER:
            mState = INVALID_OPERATOR;
            break;
    }
}

void AffineExprExprTreeVisitor::visitCallNode(ExprTree::CallNode& n) {
    // don't continue when in an error state
    if(mState != VALID_AFFINE_EXP) { return; }

    // any time a function is called in an affine expression it is
    // assumed to be non-linear
    mState = NON_LINEAR_TERM;
}

void AffineExprExprTreeVisitor::visitMemRefNode(ExprTree::MemRefNode& n) {
    // don't continue when in an error state
    if(mState != VALID_AFFINE_EXP) { return; }

    OA_ptr<NamedLoc> var;
    // get a named location for the memory reference:

    // If we have alias information:
    // ....
    /*
        // convert the MRE into a MemRefHandle
        
        // use aliasing information to get a named location
        OA_ptr<LocIterator> locs;
        locs = mAliasResults->getMayLocs(hMemRef);
        OA_ptr<Location> loc = locs->current();*
        OA_ptr<NamedLoc> var = loc.convert<NamedLoc>();
        var->output(*mIR);
    */

    // IF we don't have alias information:
    // there should be one MRE asscociated with the MemRefNode, if this
    // is not the case than the variable is considered indirect.
    MemRefHandle hMemRef = n.getHandle();
    OA_ptr<MemRefExprIterator> iterMRE = mIR->getMemRefExprIterator(hMemRef);
    OA_ptr<MemRefExpr> mre = iterMRE->current();
    (*iterMRE)++;
    if(iterMRE->isValid()) { mState = INDIRECT_REF; return; }

    // convert the MRE to a NamedRef, use information from the
    // NamedRef class to get the location asscociated with it
    if(mre->isaNamed()) {
        OA_ptr<Location> loc;
        OA_ptr<NamedRef> namedRef;
        namedRef = mre.convert<NamedRef>();
        SymHandle hSym = namedRef->getSymHandle();
        loc = mIR->getLocation(mhProc, hSym);
        var = loc.convert<NamedLoc>();
    } else { assert(false); }

    // Now that we've got a location check to see if it's for a valid variable.
    // It's not valid if the variable has been reassigned or its not
    if(mNonConstVars.find(var) == mNonConstVars.end()) {
        mState = INVALID_VAR;
        return;
    }

    // if the variable was legal, recognize it
    pushVar(var);
}

void AffineExprExprTreeVisitor::visitConstSymNode(ExprTree::ConstSymNode& n) {
    assert(false);
}

void AffineExprExprTreeVisitor::visitConstValNode(ExprTree::ConstValNode& n) {
    int val = mIR->constValIntVal(n.getHandle());
    pushScaler(val);
}

void AffineExprExprTreeVisitor::pushVar(OA_ptr<NamedLoc> var) {
    // convert the passed var into an object encoded for the stack, then
    // push it onto the stack
    StackObj oVar;
    oVar.var = var;
    oVar.isVar = true;
    mStack.push(oVar);
}

void AffineExprExprTreeVisitor::pushScaler(int val) {
    // convert the passed int into an object encoded for the stack, then
    // push it onto the stack
    StackObj oVal;
    oVal.val = val;
    oVal.isVar = false;
    mStack.push(oVal);
}

void AffineExprExprTreeVisitor::term() {
    // don't continue when in an error state
    if(mState != VALID_AFFINE_EXP) { return; }

    // pop off the last two items added to the stack
    StackObj item1 = mStack.top();  mStack.pop();
    StackObj item2 = mStack.top();  mStack.pop();

    // if one item is a variable and the other is a scalar set it so
    // that item 1 is the scalar and item 2 is the variable
    if(item1.isVar && !item2.isVar) {
        StackObj tmp = item1;
        item1 = item2;
        item2 = tmp;
    }

    // if any variable term 

    if(item1.isVar && item2.isVar) {
        mState = NON_LINEAR_TERM;
    } else {
        if(item1.isVar) {
            mAfExp->addTerm(item1.var, item2.val);
        } else {
            mAfExp->addTerm(item2.var, item1.val);
        }
    }
}

/*
void AffineExprExprTreeVisitor::term() {
    // don't continue when in an error state
    if(mState != VALID_AFFINE_EXP) { return; }

    // check to make sure the last thing added to the stack was a val, add
    // it as the offset in the affine expression abstraction
    StackObj item = mStack.top(); mStack.pop();
    assert(item.isVar == false);
    mAfExp->setOffset(item.val);
}
*/

} } // end namespaces

