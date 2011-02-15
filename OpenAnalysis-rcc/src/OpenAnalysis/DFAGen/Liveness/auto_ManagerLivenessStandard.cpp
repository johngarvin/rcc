
// ManagerLivenessStandard.cpp

#include "auto_ManagerLivenessStandard.hpp"

using namespace std;
using namespace OA::DataFlow;
using namespace OA::CFG;
using namespace OA::Alias;
using namespace OA::SideEffect;

namespace OA {
  namespace Liveness {

static bool debug = false;


ManagerLivenessStandard::ManagerLivenessStandard(
    OA_ptr<LivenessIRInterface> _ir)
    :
    mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerLivenessStandard:ALL", debug);
    mSolver = new CFGDFSolver(CFGDFSolver::Backward, *this);
}


OA_ptr<DataFlowSet> ManagerLivenessStandard::initializeTop()
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet> ManagerLivenessStandard::initializeBottom()
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerLivenessStandard::initializeNodeIN(OA_ptr<NodeInterface> n)
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerLivenessStandard::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<LivenessStandard> ManagerLivenessStandard::performAnalysis(
    ProcHandle proc,
    OA_ptr<CFGInterface> cfg,
    OA_ptr<OA::Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> interSE)
{
    if (debug) {
        cout << "In Liveness::ManagerLivenessStandard::performAnalysis"
             << endl;
    }

    // create the object we'll store results in
    mLivenessMap = new LivenessStandard(proc, mIR);

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

                OA_ptr<LocIterator> locIter;
                locIter = alias->getMayLocs(ref);
                for (; locIter->isValid(); ++(*locIter)) {
                    mStmt2MayDefMap[stmt].insert(locIter->current());
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
                OA_ptr<LocIterator> locIter;
                locIter = alias->getMayLocs(ref);
                for (; locIter->isValid(); ++(*locIter)) {
                    mStmt2MustDefMap[stmt].insert(locIter->current());
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

            OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
            for (; useIterPtr->isValid(); (*useIterPtr)++) {
                MemRefHandle ref = useIterPtr->current();
                OA_ptr<LocIterator> locIterPtr = alias->getMayLocs(ref);
                for (; locIterPtr->isValid(); ++(*locIterPtr)) {
                    mStmt2MayUseMap[stmt].insert(locIterPtr->current());
                }
            }
        }
    } {

        // mustcode for uses[s]
        OA_ptr<IRStmtIterator> stmtIter;
        stmtIter = mIR->getStmtIterator(proc);
        for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

            OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
            for (; useIterPtr->isValid(); (*useIterPtr)++) {
                MemRefHandle ref = useIterPtr->current();
                OA_ptr<LocIterator> locIterPtr = alias->getMustLocs(ref);
                for (; locIterPtr->isValid(); ++(*locIterPtr)) {
                    mStmt2MustUseMap[stmt].insert(locIterPtr->current());
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
    OA_ptr<LivenessDFSet> exitSet =
        x.convert<LivenessDFSet>();
    for(LivenessDFSet::iterator i = exitSet->begin();
        i != exitSet->end(); i++)
    {
        mLivenessMap->insertExit(*i);
    }


    return mLivenessMap;
}

void ManagerLivenessStandard::dumpset(OA_ptr<LivenessDFSet> inSet) {
    OA_ptr<set<OA_ptr<Location>  > > setRecast;
    setRecast = inSet.convert<set<OA_ptr<Location>  > >();
    set<OA_ptr<Location>  >::iterator iter;
    iter = setRecast->begin();
    for(; iter != setRecast->end(); iter++) {
        cout << (*iter)->toString(mIR);
    }
    cout << endl;
}


OA_ptr<DataFlowSet>
ManagerLivenessStandard::meet(
    OA_ptr<DataFlowSet> set1orig,
    OA_ptr<DataFlowSet> set2orig)
{
    // convert the incoming sets to sets of the specific flow value.
    OA_ptr<LivenessDFSet> set1 = set1orig.convert<LivenessDFSet>();
    OA_ptr<LivenessDFSet> set2 = set2orig.convert<LivenessDFSet>();

    if (debug) {
        cout << "ManagerLivenessStandard::meet" << endl;
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

OA_ptr<DataFlowSet> ManagerLivenessStandard::genSet(StmtHandle stmt)
{
    OA_ptr<LivenessDFSet> genSet;
    genSet =  new LivenessDFSet();

    // Generate GEN code:
    OA_ptr<LivenessDFSet> set1;
    set1 = new LivenessDFSet(mStmt2MayUseMap[stmt]);
    genSet = set1;

    return genSet;
}

OA_ptr<DataFlowSet> ManagerLivenessStandard::killSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<LivenessDFSet> x;
    x = xdfset.convert<LivenessDFSet>();

    OA_ptr<LivenessDFSet> killSet;
    killSet = new LivenessDFSet();

    // Generate KILL code:
    OA_ptr<LivenessDFSet> set2;
    set2 = new LivenessDFSet(mStmt2MustDefMap[stmt]);
    killSet = set2;

    return killSet;
}

// Currently the transfer function used by DFAGen is assumed to be:
// Y = gen[n] U (X - kill[n]).
OA_ptr<DataFlowSet>
ManagerLivenessStandard::transfer(
    OA_ptr<DataFlowSet> xdfset,
    StmtHandle stmt)
{
    OA_ptr<set<OA_ptr<Location> > > xdfgenset;
    xdfgenset = xdfset.convert<set<OA_ptr<Location> > >();

    OA_ptr<LivenessDFSet> x;
    x = new LivenessDFSet(*xdfgenset);

    // if debug mode is on print information about transfer function
    if (debug) {
        cout << "In transfer: " << endl;
        cout << "\t Stmt  = " << mIR->toString(stmt) << endl;
        cout << "\t X     = ";
        dumpset(x);
    }

    OA_ptr<DataFlowSet> genBaseObj = genSet(stmt);
    OA_ptr<LivenessDFSet> gen = genBaseObj.convert<LivenessDFSet>();

    OA_ptr<DataFlowSet> killBaseObj = killSet(stmt, x);
    OA_ptr<LivenessDFSet> kill = killBaseObj.convert<LivenessDFSet>();


    // Add gen[n] to Y
    LivenessDFSet::iterator genIter = gen->begin();
    for(; genIter != gen->end(); ++genIter) {
        OA_ptr<Location>  genVal;
        genVal = *genIter;
        mLivenessMap->insert(stmt, genVal);
    }
    // Add X - kill[n] to Y
    x->minusEqu(*kill);
    LivenessDFSet::iterator xIter = x->begin();
    for(; xIter != x->end(); ++xIter) {
        OA_ptr<Location>  xVal;
        xVal = *xIter;
        mLivenessMap->insert(stmt, xVal);
    }

    // if debug mode is on print information about transfer function
    if(debug) {
        cout << "\t gen  = ";
        dumpset(gen);
        cout << "\t kill = ";
        dumpset(kill);
        cout << "\t Y    = ";
        dumpset(mLivenessMap->getLivenessSet(stmt));
    }

    return mLivenessMap->getLivenessSet(stmt);
}

  } // end of namespace Liveness
} // end of namespace OA
