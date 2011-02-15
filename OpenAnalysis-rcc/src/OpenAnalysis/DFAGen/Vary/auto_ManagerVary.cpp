// ManagerVary.cpp

#include "auto_ManagerVary.hpp"

using namespace std;
using namespace OA::DataFlow;
using namespace OA::CFG;
using namespace OA::Alias;
using namespace OA::SideEffect;

namespace OA {
  namespace Vary {

static bool debug = false;


ManagerVary::ManagerVary(
    OA_ptr<VaryIRInterface> _ir)
    :
    mIR(_ir)
{
    // Check OA_DEBUG environmental variable to see if we should turn debugging
    // mode on.
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerVary:ALL", debug);

    // Initialize member data
    mSolver = new CFGDFSolver(CFGDFSolver::Forward, *this);
}


OA_ptr<Vary> ManagerVary::performAnalysis(
    ProcHandle proc,
    OA_ptr<CFGInterface> cfg,
    OA_ptr<OA::Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> interSE)
{
    if (debug) {
        cout << "In Vary::ManagerVary::performAnalysis"
             << endl;
    }

    // create the object we'll store results in
    mVaryMap = new Vary(proc, mIR);

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
    OA_ptr<VaryDFSet> exitSet =
        x.convert<VaryDFSet>();
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > i;
    i = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (exitSet);
    for(; i->isValid(); (*i)++)
    {
        mVaryMap->insertExit(i->current());
    }


    return mVaryMap;
}



OA_ptr<DataFlowSet> ManagerVary::initializeTop()
{
    OA_ptr<VaryDFSet> retval;
    retval = new VaryDFSet();
    return retval;
}


OA_ptr<DataFlowSet> ManagerVary::initializeBottom()
{
    OA_ptr<VaryDFSet> retval;
    retval = new VaryDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerVary::initializeNodeIN(OA_ptr<NodeInterface> n)
{
    OA_ptr<VaryDFSet> retval;
    retval = new VaryDFSet();
    return retval;
}


OA_ptr<DataFlowSet>
ManagerVary::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
    OA_ptr<VaryDFSet> retval;
    retval = new VaryDFSet();
    return retval;
}



void ManagerVary::dumpset(OA_ptr<VaryDFSet> inSet) {
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
ManagerVary::meet(
    OA_ptr<DataFlowSet> set1orig,
    OA_ptr<DataFlowSet> set2orig)
{
    // convert the incoming sets to sets of the specific flow value.
    OA_ptr<VaryDFSet> set1 = set1orig.convert<VaryDFSet>();
    OA_ptr<VaryDFSet> set2 = set2orig.convert<VaryDFSet>();

    if (debug) {
        cout << "ManagerVary::meet" << endl;
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

OA_ptr<DataFlowSet> ManagerVary::genSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<VaryDFSet> x;
    x = xdfset.convert<VaryDFSet>();

    OA_ptr<VaryDFSet> genSet;
    genSet =  new VaryDFSet();

    // Generate GEN code:
    OA_ptr<DataFlowSetImpl<Alias::AliasTag > > set1;
    set1 = new DataFlowSetImpl<Alias::AliasTag >();
    OA_ptr<VaryDFSet> set2;
    set2 = new DataFlowSetImpl<Alias::AliasTag >(mStmt2MayDefMap[stmt]);
    DataFlow::DataFlowSetImplIterator<Alias::AliasTag >  iter1(set2);
    Alias::AliasTag iter1Val;

    for(; iter1.isValid(); iter1++) {
        iter1Val = iter1.current();
        OA_ptr<VaryDFSet> set3;
        set3 = new VaryDFSet(mStmt2MayDefMap[stmt]);
        bool cond1;
        cond1 = set3->contains(iter1Val);
        OA_ptr<VaryDFSet> set4;
        set4 = new VaryDFSet(mStmt2MustUseMap[stmt]);
        OA_ptr<VaryDFSet> set5;
        set5 = x;
        set5->intersectEqu(*set4);
        bool cond2;
        cond2 = !set5->isEmpty();
        bool cond3;
        cond3 = cond1 && cond2;
        bool cond;
        cond = cond3;
        if(cond) {
            set1->insert(iter1Val);
        }
    }
    genSet = set1;

    return genSet;
}

OA_ptr<DataFlowSet> ManagerVary::killSet(
    StmtHandle stmt, OA_ptr<DataFlowSet> xdfset)
{
    OA_ptr<VaryDFSet> x;
    x = xdfset.convert<VaryDFSet>();

    OA_ptr<VaryDFSet> killSet;
    killSet = new VaryDFSet();


    return killSet;
}

// Currently the transfer function used by DFAGen is assumed to be:
// Y = gen[n] U (X - kill[n]).
OA_ptr<DataFlowSet>
ManagerVary::transfer(
    OA_ptr<DataFlowSet> xdfset,
    StmtHandle stmt)
{
    OA_ptr<VaryDFSet> xdfgenset;
    xdfgenset = xdfset.convert<VaryDFSet>();

    OA_ptr<VaryDFSet> x;
    x = new VaryDFSet(*xdfgenset);

    // if debug mode is on print information about transfer function
    if (debug) {
        cout << "In transfer: " << endl;
        cout << "\t Stmt  = " << mIR->toString(stmt) << endl;
        cout << "\t X     = ";
        dumpset(x);
    }

    OA_ptr<DataFlowSet> genBaseObj = genSet(stmt, x);
    OA_ptr<VaryDFSet> gen = genBaseObj.convert<VaryDFSet>();

    OA_ptr<DataFlowSet> killBaseObj = killSet(stmt, x);
    OA_ptr<VaryDFSet> kill = killBaseObj.convert<VaryDFSet>();


    // Add gen[n] to Y
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > genIter;
    genIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (gen);
    for(; genIter->isValid(); ++(*genIter)) {
        Alias::AliasTag  genVal;
        genVal = genIter->current();
        mVaryMap->insert(stmt, genVal);
    }
    // Add X - kill[n] to Y
    x->minusEqu(*kill);
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag > > xIter;
    xIter = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag > (x);
    for(; xIter->isValid(); ++(*xIter)) {
        Alias::AliasTag  xVal;
        xVal = xIter->current();
        mVaryMap->insert(stmt, xVal);
    }

    // if debug mode is on print information about transfer function
    if(debug) {
        cout << "\t gen  = ";
        dumpset(gen);
        cout << "\t kill = ";
        dumpset(kill);
        cout << "\t Y    = ";
        dumpset(mVaryMap->getVarySet(stmt));
    }

    return mVaryMap->getVarySet(stmt);
}

  } // end of namespace Vary
} // end of namespace OA
