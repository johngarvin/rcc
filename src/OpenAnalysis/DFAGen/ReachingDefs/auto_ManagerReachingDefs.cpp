// ManagerReachingDefs.cpp

#include "auto_ManagerReachingDefs.hpp"

using namespace std;
using namespace OA::DataFlow;
using namespace OA::CFG;
using namespace OA::Alias;
using namespace OA::SideEffect;

namespace OA {
  namespace ReachingDefs {

static bool debug = false;


ManagerReachingDefs::ManagerReachingDefs(
    OA_ptr<ReachingDefsIRInterface> _ir)
    :
    mIR(_ir)
{
    // Check OA_DEBUG environmental variable to see if we should turn debugging
    // mode on.
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerReachingDefs:ALL", debug);

    // Initialize member data
    mSolver = new CFGDFSolver(CFGDFSolver::Forward, *this);
}


OA_ptr<ReachingDefs> ManagerReachingDefs::performAnalysis(
    ProcHandle proc,
    OA_ptr<CFGInterface> cfg,
    OA_ptr<OA::Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> interSE)
{
    if (debug) {
        cout << "In ReachingDefs::ManagerReachingDefs::performAnalysis"
             << endl;
    }

    // create the object we'll store results in
    mReachingDefsMap = new ReachingDefs(proc, mIR);

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
    OA_ptr<ReachingDefsDFSet> exitSet =
        x.convert<ReachingDefsDFSet>();
    OA_ptr<DataFlow::DataFlowSetImplIterator<StmtHandle > > i;
    i = new DataFlow::DataFlowSetImplIterator<StmtHandle > (exitSet);
    for(; i->isValid(); (*i)++)
    {
        mReachingDefsMap->insertExit(i->current());
    }


    return mReachingDefsMap;
}



OA_ptr<DataFlowSet> ManagerReachingDefs::initializeTop()
{
    OA_ptr<ReachingDefsDFSet> retval;
    retval = new ReachingDefsDFSet();
    return retval;
}


OA_ptr<DataFlowSet> ManagerReachingDefs::initializeBottom()
{
    OA_ptr<ReachingDefsDFSet> retval;
    retval = new ReachingDefsDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerReachingDefs::initializeNodeIN(OA_ptr<NodeInterface> n)
{
    OA_ptr<ReachingDefsDFSet> retval;
    retval = new ReachingDefsDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerReachingDefs::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
    OA_ptr<ReachingDefsDFSet> retval;
    retval = new ReachingDefsDFSet();
    return retval;
}



void ManagerReachingDefs::dumpset(OA_ptr<ReachingDefsDFSet> inSet) {
    Alias::Interface& aliasResults = *mAlias;
    std::ostream& os = std::cout;

    OA_ptr<DataFlow::DataFlowSetImplIterator<StmtHandle > > iter;
    iter = new DataFlow::DataFlowSetImplIterator<StmtHandle > (
        inSet);
    for ( ; iter->isValid(); (*iter)++) {
        if(!iter->isFirst()) {
            cout << ",";
        }
        cout << mIR->toString(iter->current());
    }
    cout << endl;
}


OA_ptr<DataFlowSet>
ManagerReachingDefs::meet(
    OA_ptr<DataFlowSet> set1orig,
    OA_ptr<DataFlowSet> set2orig)
{
    // convert the incoming sets to sets of the specific flow value.
    OA_ptr<ReachingDefsDFSet> set1 = set1orig.convert<ReachingDefsDFSet>();
    OA_ptr<ReachingDefsDFSet> set2 = set2orig.convert<ReachingDefsDFSet>();

    if (debug) {
        cout << "ManagerReachingDefs::meet" << endl;
        cout << "\tset1 = ";
        dumpset(set1);
        cout << "\tset2 = ";
        dumpset(set2);
    }

    // perform the meet operator on the left hand set.  It's okay to modify the
    // set and return it as a result [since the solver passes a temporary set
    // in as set1.  See the CFGDFProblem header for more info.
    set1->unionEqu(*set2);

    if (debug) {
        cout << endl << "\tcalculated set = ";
        dumpset(set1);
    }

    return set1;
}

OA_ptr<DataFlowSet> ManagerReachingDefs::genSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<ReachingDefsDFSet> x;
    x = xdfset.convert<ReachingDefsDFSet>();

    OA_ptr<ReachingDefsDFSet> genSet;
    genSet =  new ReachingDefsDFSet();

    // Generate GEN code:
    OA_ptr<DataFlowSetImpl<StmtHandle > > set1;
    set1 = new DataFlowSetImpl<StmtHandle >();
    DataFlow::DataFlowSetImplIterator<StmtHandle >  iter1;
    StmtHandle iter1Val;

    {
        iter1Val = stmt;
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set2;
        set2 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MayDefMap[iter1Val]);
        bool cond1;
        cond1 = !set2->isEmpty();
        bool cond;
        cond = cond1;
        if(cond) {
            set1->insert(iter1Val);
        }
    }
    genSet = set1;

    return genSet;
}

OA_ptr<DataFlowSet> ManagerReachingDefs::killSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<ReachingDefsDFSet> x;
    x = xdfset.convert<ReachingDefsDFSet>();

    OA_ptr<ReachingDefsDFSet> killSet;
    killSet = new ReachingDefsDFSet();

    // Generate KILL code:
    OA_ptr<DataFlowSetImpl<StmtHandle > > set3;
    set3 = new DataFlowSetImpl<StmtHandle >();
    DataFlow::DataFlowSetImplIterator<StmtHandle >  iter2(x);
    StmtHandle iter2Val;

    for(; iter2.isValid(); iter2++) {
        iter2Val = iter2.current();
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set4;
        set4 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MayDefMap[iter2Val]);
        OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set5;
        set5 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MustDefMap[stmt]);
        bool cond2;
        cond2 = set4->isSubset(*set5);
        bool cond;
        cond = cond2;
        if(cond) {
            set3->insert(iter2Val);
        }
    }
    killSet = set3;

    return killSet;
}

// Currently the transfer function used by DFAGen is assumed to be:
// Y = gen[n] U (X - kill[n]).
OA_ptr<DataFlowSet>
ManagerReachingDefs::transfer(
    OA_ptr<DataFlowSet> xdfset,
    StmtHandle stmt)
{
    OA_ptr<ReachingDefsDFSet> xdfgenset;
    xdfgenset = xdfset.convert<ReachingDefsDFSet>();

    OA_ptr<ReachingDefsDFSet> x;
    x = new ReachingDefsDFSet(*xdfgenset);

    // if debug mode is on print information about transfer function
    if (debug) {
        cout << "In transfer: " << endl;
        cout << "\t Stmt  = " << mIR->toString(stmt) << endl;
        cout << "\t X     = ";
        dumpset(x);
    }

    OA_ptr<DataFlowSet> genBaseObj = genSet(stmt, x);
    OA_ptr<ReachingDefsDFSet> gen = genBaseObj.convert<ReachingDefsDFSet>();

    OA_ptr<DataFlowSet> killBaseObj = killSet(stmt, x);
    OA_ptr<ReachingDefsDFSet> kill = killBaseObj.convert<ReachingDefsDFSet>();


    // Add gen[n] to Y
    OA_ptr<DataFlow::DataFlowSetImplIterator<StmtHandle > > genIter;
    genIter = new DataFlow::DataFlowSetImplIterator<StmtHandle > (gen);
    for(; genIter->isValid(); ++(*genIter)) {
        StmtHandle  genVal;
        genVal = genIter->current();
        mReachingDefsMap->insert(stmt, genVal);
    }
    // Add X - kill[n] to Y
    x->minusEqu(*kill);
    OA_ptr<DataFlow::DataFlowSetImplIterator<StmtHandle > > xIter;
    xIter = new DataFlow::DataFlowSetImplIterator<StmtHandle > (x);
    for(; xIter->isValid(); ++(*xIter)) {
        StmtHandle  xVal;
        xVal = xIter->current();
        mReachingDefsMap->insert(stmt, xVal);
    }

    // if debug mode is on print information about transfer function
    if(debug) {
        cout << "\t gen  = ";
        dumpset(gen);
        cout << "\t kill = ";
        dumpset(kill);
        cout << "\t Y    = ";
        dumpset(mReachingDefsMap->getReachingDefsSet(stmt));
    }

    return mReachingDefsMap->getReachingDefsSet(stmt);
}

  } // end of namespace ReachingDefs
} // end of namespace OA
