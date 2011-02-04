// ManagerAvailExpr.cpp

#include "auto_ManagerAvailExpr.hpp"

using namespace std;
using namespace OA::DataFlow;
using namespace OA::CFG;
using namespace OA::Alias;
using namespace OA::SideEffect;

namespace OA {
  namespace AvailExpr {

static bool debug = false;


ManagerAvailExpr::ManagerAvailExpr(
    OA_ptr<AvailExprIRInterface> _ir)
    :
    mIR(_ir)
{
    // Check OA_DEBUG environmental variable to see if we should turn debugging
    // mode on.
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerAvailExpr:ALL", debug);

    // Initialize member data
    mSolver = new CFGDFSolver(CFGDFSolver::Forward, *this);
}


OA_ptr<AvailExpr> ManagerAvailExpr::performAnalysis(
    ProcHandle proc,
    OA_ptr<CFGInterface> cfg,
    OA_ptr<OA::Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> interSE)
{
    if (debug) {
        cout << "In AvailExpr::ManagerAvailExpr::performAnalysis"
             << endl;
    }

    // create the object we'll store results in
    mAvailExprMap = new AvailExpr(proc, mIR);

    // store Alias information for use within the transfer function
    mAlias = alias;

    {

        // maycode for defs[s]
        OA_ptr<IRStmtIterator> stmtIter;
        stmtIter = mIR->getStmtIterator(proc);
        for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

            OA_ptr<MemRefHandleIterator> defsIter;
            defsIter = mIR->getDefMemRefs(stmt);
            for (; defsIter->isValid(); (*defsIter)++) {
                MemRefHandle ref = defsIter->current();

                OA_ptr<AliasTagSet> tagSet;
                tagSet = alias->getAliasTags(ref);
                OA_ptr<AliasTagIterator> tagIter;
                tagIter = tagSet->getIterator();
                for(; tagIter->isValid(); ++(*tagIter)) {
                    mStmt2MayDefMap[stmt].insert(tagIter->current());
                }
            }
        }
    } {

        // mustcode for defs[s]
        OA_ptr<IRStmtIterator> stmtIter;
        stmtIter = mIR->getStmtIterator(proc);
        for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

            OA_ptr<MemRefHandleIterator> defsIter;
            defsIter = mIR->getDefMemRefs(stmt);
            for (; defsIter->isValid(); (*defsIter)++) {
                MemRefHandle ref = defsIter->current();

                OA_ptr<AliasTagSet> tagSet;
                tagSet = alias->getAliasTags(ref);
                if(tagSet->isMust()) {
                    OA_ptr<AliasTagIterator> tagIter;
                    tagIter = tagSet->getIterator();
                    for(; tagIter->isValid(); ++(*tagIter)) {
                        mStmt2MustDefMap[stmt].insert(tagIter->current());
                    }
                }
            }
        }
    }
    {

      OA_ptr<IRStmtIterator> stmtIter;
      OA_ptr<ExprHandleIterator> iterHdlExpr;
      OA_ptr<NewExprTree> eTree;

      // iterate through all statements in the procedure
      stmtIter = mIR->getStmtIterator(proc);
      for(; stmtIter->isValid(); ++(*stmtIter)) {
          StmtHandle stmt = stmtIter->current();

          // iterate through all expressions in the statement
          iterHdlExpr = mIR->getExprHandleIterator(stmt);
          for(; iterHdlExpr->isValid(); ++(*iterHdlExpr)) {
              ExprHandle hExp = iterHdlExpr->current();
              
              eTree = mIR->getNewExprTree(hExp);
              mStmt2MayExprMap[stmt].insert(eTree);
          }
      }
    } {

      OA_ptr<IRStmtIterator> stmtIter;
      OA_ptr<ExprHandleIterator> iterHdlExpr;
      OA_ptr<NewExprTree> eTree;

      // iterate through all statements in the procedure
      stmtIter = mIR->getStmtIterator(proc);
      for(; stmtIter->isValid(); ++(*stmtIter)) {
          StmtHandle stmt = stmtIter->current();

          // iterate through all expressions in the statement
          iterHdlExpr = mIR->getExprHandleIterator(stmt);
          for(; iterHdlExpr->isValid(); ++(*iterHdlExpr)) {
              ExprHandle hExp = iterHdlExpr->current();
              
              eTree = mIR->getNewExprTree(hExp);
              mStmt2MustExprMap[stmt].insert(eTree);
          }
      }
    }
    {

        // maycode for uses[s]
        OA_ptr<IRStmtIterator> stmtIter;
        stmtIter = mIR->getStmtIterator(proc);
        for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

            OA_ptr<MemRefHandleIterator> usesIter;
            usesIter= mIR->getUseMemRefs(stmt);
            for (; usesIter->isValid(); (*usesIter)++) {
                MemRefHandle ref = usesIter->current();
                
                OA_ptr<AliasTagSet> tagSet;
                tagSet = alias->getAliasTags(ref);
                OA_ptr<AliasTagIterator> tagIter;
                tagIter = tagSet->getIterator();
                for(; tagIter->isValid(); ++(*tagIter)) {
                    mStmt2MayUseMap[stmt].insert(tagIter->current());
                }
            }
        }
    } {

        // maycode for uses[s]
        OA_ptr<IRStmtIterator> stmtIter;
        stmtIter = mIR->getStmtIterator(proc);
        for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

            OA_ptr<MemRefHandleIterator> usesIter;
            usesIter= mIR->getUseMemRefs(stmt);
            for (; usesIter->isValid(); (*usesIter)++) {
                MemRefHandle ref = usesIter->current();
                
                OA_ptr<AliasTagSet> tagSet;
                tagSet = alias->getAliasTags(ref);
                if(tagSet->isMust()) {
                    OA_ptr<AliasTagIterator> tagIter;
                    tagIter = tagSet->getIterator();
                    for(; tagIter->isValid(); ++(*tagIter)) {
                        mStmt2MustUseMap[stmt].insert(tagIter->current());
                    }
                }
            }
        }
    }

    // use the dataflow solver to get the In and Out sets for the
    // basic blocks (BBs).
    mSolver->solve(cfg, ITERATIVE);

    // get exit node for CFG and determine what data flow values exit it.
    // store these data flow values in the result's exit information.
    OA_ptr<NodeInterface> node;
    node = cfg->getExit();
    OA_ptr<DataFlowSet> x = mSolver->getOutSet(node);
    OA_ptr<AvailExprDFSet> exitSet =
        x.convert<AvailExprDFSet>();
    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > i;
    i = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (exitSet);
    for(; i->isValid(); (*i)++)
    {
        mAvailExprMap->insertExit(i->current());
    }


    return mAvailExprMap;
}



OA_ptr<DataFlowSet> ManagerAvailExpr::initializeTop()
{
    OA_ptr<AvailExprDFSet> retval;
    retval = new AvailExprDFSet();
    return retval;
}


OA_ptr<DataFlowSet> ManagerAvailExpr::initializeBottom()
{
    OA_ptr<AvailExprDFSet> retval;
    retval = new AvailExprDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerAvailExpr::initializeNodeIN(OA_ptr<NodeInterface> n)
{
    OA_ptr<AvailExprDFSet> retval;
    retval = new AvailExprDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerAvailExpr::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
    OA_ptr<AvailExprDFSet> retval;
    retval = new AvailExprDFSet();
    return retval;
}



void ManagerAvailExpr::dumpset(OA_ptr<AvailExprDFSet> inSet) {
    Alias::Interface& aliasResults = *mAlias;
    std::ostream& os = std::cout;

    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > iter;
    iter = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (
        inSet);
    for ( ; iter->isValid(); (*iter)++) {
        if(!iter->isFirst()) {
            cout << ",";
        }
        iter->current()->dump(cout, mIR);
    }
    cout << endl;
}


OA_ptr<DataFlowSet>
ManagerAvailExpr::meet(
    OA_ptr<DataFlowSet> set1orig,
    OA_ptr<DataFlowSet> set2orig)
{
    // convert the incoming sets to sets of the specific flow value.
    OA_ptr<AvailExprDFSet> set1 = set1orig.convert<AvailExprDFSet>();
    OA_ptr<AvailExprDFSet> set2 = set2orig.convert<AvailExprDFSet>();

    if (debug) {
        cout << "ManagerAvailExpr::meet" << endl;
        cout << "\tset1 = ";
        dumpset(set1);
        cout << "\tset2 = ";
        dumpset(set2);
    }

    // perform the meet operator on the left hand set.  It's okay to modify the
    // set and return it as a result [since the solver passes a temporary set
    // in as set1.  See the CFGDFProblem header for more info.
    set1->intersectEqu(*set2);

    if (debug) {
        cout << endl << "\tcalculated set = ";
        dumpset(set1);
    }

    return set1;
}

OA_ptr<DataFlowSet> ManagerAvailExpr::genSet(StmtHandle stmt)
{
    OA_ptr<AvailExprDFSet> genSet;
    genSet =  new AvailExprDFSet();

    // Generate GEN code:
    OA_ptr<AvailExprDFSet> set1;
    set1 = new AvailExprDFSet(mStmt2MustExprMap[stmt]);
    genSet = set1;

    return genSet;
}

OA_ptr<DataFlowSet> ManagerAvailExpr::killSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<AvailExprDFSet> x;
    x = xdfset.convert<AvailExprDFSet>();

    OA_ptr<AvailExprDFSet> killSet;
    killSet = new AvailExprDFSet();

    // Generate KILL code:
    OA_ptr<DataFlowSetImpl<OA_ptr<ExprTree> > > set2;
    set2 = new DataFlowSetImpl<OA_ptr<ExprTree> >();
    DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> >  iter1(x);
    OA_ptr<ExprTree> iter1Val;

    for(; iter1.isValid(); iter1++) {
        iter1Val = iter1.current();
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set3;
        set3 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MayUseMap[iter1Val]);
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set4;
        set4 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MayDefMap[stmt]);
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set5;
        set5 = set3->intersect(set4);
        bool cond1;
        cond1 = !set5->isEmpty();
        bool cond;
        cond = cond1;
        if(cond) {
            set2->insert(iter1Val);
        }
    }
    killSet = set2;

    return killSet;
}

// Currently the transfer function used by DFAGen is assumed to be:
// Y = gen[n] U (X - kill[n]).
OA_ptr<DataFlowSet>
ManagerAvailExpr::transfer(
    OA_ptr<DataFlowSet> xdfset,
    StmtHandle stmt)
{
    OA_ptr<AvailExprDFSet> xdfgenset;
    xdfgenset = xdfset.convert<AvailExprDFSet>();

    OA_ptr<AvailExprDFSet> x;
    x = new AvailExprDFSet(*xdfgenset);

    // if debug mode is on print information about transfer function
    if (debug) {
        cout << "In transfer: " << endl;
        cout << "\t Stmt  = " << mIR->toString(stmt) << endl;
        cout << "\t X     = ";
        dumpset(x);
    }

    OA_ptr<DataFlowSet> genBaseObj = genSet(stmt);
    OA_ptr<AvailExprDFSet> gen = genBaseObj.convert<AvailExprDFSet>();

    OA_ptr<DataFlowSet> killBaseObj = killSet(stmt, x);
    OA_ptr<AvailExprDFSet> kill = killBaseObj.convert<AvailExprDFSet>();


    // Add gen[n] to Y
    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > genIter;
    genIter = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (gen);
    for(; genIter->isValid(); ++(*genIter)) {
        OA_ptr<ExprTree>  genVal;
        genVal = genIter->current();
        mAvailExprMap->insert(stmt, genVal);
    }
    // Add X - kill[n] to Y
    x->minusEqu(*kill);
    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > xIter;
    xIter = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (x);
    for(; xIter->isValid(); ++(*xIter)) {
        OA_ptr<ExprTree>  xVal;
        xVal = xIter->current();
        mAvailExprMap->insert(stmt, xVal);
    }

    // if debug mode is on print information about transfer function
    if(debug) {
        cout << "\t gen  = ";
        dumpset(gen);
        cout << "\t kill = ";
        dumpset(kill);
        cout << "\t Y    = ";
        dumpset(mAvailExprMap->getAvailExprSet(stmt));
    }

    return mAvailExprMap->getAvailExprSet(stmt);
}

  } // end of namespace AvailExpr
} // end of namespace OA
