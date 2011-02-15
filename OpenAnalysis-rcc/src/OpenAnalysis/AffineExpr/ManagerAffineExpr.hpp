#ifndef ManagerAffineExpr_H
#define ManagerAffineExpr_H

#include <OpenAnalysis/ExprTree/ExprTreeVisitor.hpp>
#include <OpenAnalysis/IRInterface/AffineExprIRInterface.hpp>
#include <OpenAnalysis/AffineExpr/AffineExprAbstraction.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <stack>
using namespace std;

namespace OA {
namespace AffineExpr {

/*! Used by the ManagerAffineExpr to signify an error state when something
    unexpected occurs in the expression tree. */
enum AffineAnlState {
    INVALID_OPERATOR = 0,       // ex:  using '/', '%', or '-'
    NON_LINEAR_TERM,            // ex:  i * i or i * j or a function call
    INVALID_VAR,                // ex:  3 * j when j is not a global param.
    INDIRECT_REF,               // ex:  A[i]
    VALID_AFFINE_EXP            // ex:  3 * i + 4
};

// used by the affine expression visitor class when attempting to constructing
// an affine expression.  Values and variables are pushed onto a stack and
// popped off when operators are read in.
struct StackObj {
    bool isVar;

    // these two can't be put into a union because the OA_ptr object
    // requires construction
    int val;
    OA_ptr<NamedLoc> var;
};


/*! Used by ManagerAffineExpr to build affine expressions.  This class
    traverses the nodes in an expression tree parsing out the needed
    information to make an affine expression.  Performs a prefix traversal. */
class AffineExprExprTreeVisitor : public ExprTreeVisitor {
  public:
    AffineExprExprTreeVisitor(
        OA_ptr<AffineExprAbstraction> afExp,
        OA_ptr<AffineExprIRInterface> ir,
        ProcHandle hProc,
        const set<OA_ptr<NamedLoc> > & indexVars,
        const set<OA_ptr<NamedLoc> > & nonConstVars,
        OA_ptr<Alias::Interface> aliasResults)
          :
        mAfExp(afExp),
        mIR(ir),
        mhProc(hProc),
        mIndexVars(indexVars),
        mNonConstVars(nonConstVars),
        mAliasResults(aliasResults),
        mState(VALID_AFFINE_EXP)
    { }

    virtual ~AffineExprExprTreeVisitor() {}

    virtual void visitExprTreeBefore(ExprTree&) { }
    virtual void visitExprTreeAfter(ExprTree&) { }
    virtual void visitNode(ExprTree::Node& n) { }

    virtual void visitOpNode(ExprTree::OpNode& n);
    virtual void visitCallNode(ExprTree::CallNode& n);
    virtual void visitMemRefNode(ExprTree::MemRefNode& n);
    virtual void visitConstSymNode(ExprTree::ConstSymNode& n);
    virtual void visitConstValNode(ExprTree::ConstValNode& n);

    AffineAnlState getState() { return mState; }

  private:
    void pushVar(OA_ptr<NamedLoc> var);
    void pushScaler(int val);
    void factor();
    void term();

    stack<StackObj> mStack;

    OA_ptr<AffineExprAbstraction> mAfExp;
    OA_ptr<AffineExprIRInterface> mIR;
    ProcHandle mhProc;
    const set<OA_ptr<NamedLoc> > & mIndexVars;
    const set<OA_ptr<NamedLoc> > & mNonConstVars;
    OA_ptr<Alias::Interface> mAliasResults;
    AffineAnlState mState;
};



/*! The affine expression manager is responsible for constructing affine
    expression objects. */
class ManagerAffineExpr {
  public:
    //! construct an affine expression analysis manager
    ManagerAffineExpr(
        OA_ptr<AffineExprIRInterface> _ir);
        
    /*! given an expression construct an affine expression object.
        Analogous to performAnalysis() in most managers. */
    OA_ptr<AffineExprAbstraction> analyzeMemRefExp(
        ExprHandle hExp,
        ProcHandle hProc,
        const set<OA_ptr<NamedLoc> > & indexVars,
        const set<OA_ptr<NamedLoc> > & nonConstVars,
        OA_ptr<Alias::Interface> aliasResults,
        AffineAnlState *state);

    static string toString(AffineAnlState state) {
        switch(state) {
            case INVALID_OPERATOR: return "INVALID_OPERATOR"; break;
            case NON_LINEAR_TERM:  return "NON_LINEAR_TERM";  break;
            case INVALID_VAR:      return "INVALID_VAR";      break;
            case INDIRECT_REF:     return "INDIRECT_REF";     break;
            case VALID_AFFINE_EXP: return "VALID_AFFINE_EXP"; break;
        };
        return "INVALID AffineAnlState!";
    }

  private:
    OA_ptr<AffineExprIRInterface> mIR;
};

} } // end namespaces

#endif
