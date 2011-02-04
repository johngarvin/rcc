// ManagerLiveness.cpp

#include "auto_ManagerLiveness.hpp"

using namespace std;
using namespace OA::DataFlow;
using namespace OA::CFG;
using namespace OA::Alias;
using namespace OA::SideEffect;

namespace OA {
  namespace Liveness {

static bool debug = false;


ManagerLiveness::ManagerLiveness(
    OA_ptr<LivenessIRInterface> _ir)
    :
    mIR(_ir)
{
    // Check OA_DEBUG environmental variable to see if we should turn debugging
    // mode on.
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerLiveness:ALL", debug);

    // Initialize member data
    mSolver = new CFGDFSolver(CFGDFSolver::Backward, *this);
}


OA_ptr<Liveness> ManagerLiveness::performAnalysis(
    ProcHandle proc,
    OA_ptr<CFGInterface> cfg,
    OA_ptr<OA::Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> interSE)
{
    if (debug) {
        cout << "In Liveness::ManagerLiveness::performAnalysis"
             << endl;
    }

    // create the object we'll store results in
    mLivenessMap = new Liveness(proc, mIR);

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
    OA_ptr<LivenessDFSet> exitSet =
        x.convert<LivenessDFSet>();
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > i;
    i = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (exitSet);
    for(; i->isValid(); (*i)++)
    {
        mLivenessMap->insertExit(i->current());
    }


    return mLivenessMap;
}



OA_ptr<DataFlowSet> ManagerLiveness::initializeTop()
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet> ManagerLiveness::initializeBottom()
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerLiveness::initializeNodeIN(OA_ptr<NodeInterface> n)
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerLiveness::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
    OA_ptr<LivenessDFSet> retval;
    retval = new LivenessDFSet();
    return retval;
}



void ManagerLiveness::dumpset(OA_ptr<LivenessDFSet> inSet) {
    Alias::Interface& aliasResults = *mAlias;
    std::ostream& os = std::cout;

    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > iter;
    iter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (
        inSet);
    for ( ; iter->isValid(); (*iter)++) {
        if(!iter->isFirst()) {
            cout << ",";
        }
        iter->current().dump(os, *mIR, aliasResults);
    }
    cout << endl;
}


OA_ptr<DataFlowSet>
ManagerLiveness::meet(
    OA_ptr<DataFlowSet> set1orig,
    OA_ptr<DataFlowSet> set2orig)
{
    // convert the incoming sets to sets of the specific flow value.
    OA_ptr<LivenessDFSet> set1 = set1orig.convert<LivenessDFSet>();
    OA_ptr<LivenessDFSet> set2 = set2orig.convert<LivenessDFSet>();

    if (debug) {
        cout << "ManagerLiveness::meet" << endl;
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

OA_ptr<DataFlowSet> ManagerLiveness::genSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<LivenessDFSet> x;
    x = xdfset.convert<LivenessDFSet>();

    OA_ptr<LivenessDFSet> genSet;
    genSet =  new LivenessDFSet();

    // Generate GEN code:
    OA_ptr<LivenessDFSet> set1;
    set1 = new LivenessDFSet(mStmt2MayUseMap[stmt]);
    genSet = set1;

    return genSet;
}

OA_ptr<DataFlowSet> ManagerLiveness::killSet(
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
ManagerLiveness::transfer(
    OA_ptr<DataFlowSet> xdfset,
    StmtHandle stmt)
{
    OA_ptr<LivenessDFSet> xdfgenset;
    xdfgenset = xdfset.convert<LivenessDFSet>();

    OA_ptr<LivenessDFSet> x;
    x = new LivenessDFSet(*xdfgenset);

    // if debug mode is on print information about transfer function
    if (debug) {
        cout << "In transfer: " << endl;
        cout << "\t Stmt  = " << mIR->toString(stmt) << endl;
        cout << "\t X     = ";
        dumpset(x);
    }

    OA_ptr<DataFlowSet> genBaseObj = genSet(stmt, x);
    OA_ptr<LivenessDFSet> gen = genBaseObj.convert<LivenessDFSet>();

    OA_ptr<DataFlowSet> killBaseObj = killSet(stmt, x);
    OA_ptr<LivenessDFSet> kill = killBaseObj.convert<LivenessDFSet>();


    // Add gen[n] to Y
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > genIter;
    genIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (gen);
    for(; genIter->isValid(); ++(*genIter)) {
        Alias::AliasTag  genVal;
        genVal = genIter->current();
        mLivenessMap->insert(stmt, genVal);
    }
    // Add X - kill[n] to Y
    x->minusEqu(*kill);
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > xIter;
    xIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (x);
    for(; xIter->isValid(); ++(*xIter)) {
        Alias::AliasTag  xVal;
        xVal = xIter->current();
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
