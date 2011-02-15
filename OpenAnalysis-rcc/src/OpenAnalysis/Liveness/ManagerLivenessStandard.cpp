
// ManagerLivenessStandard.cpp


#include "ManagerLivenessStandard.hpp"


namespace OA {
  namespace Liveness {

using namespace Alias;
using namespace std;
using namespace DataFlow;

static bool debug = false;


ManagerLivenessStandard::ManagerLivenessStandard(OA_ptr<LivenessIRInterface> _ir) 
    : mIR(_ir)
{
   OA_DEBUG_CTRL_MACRO("DEBUG_ManagerLivenessStandard:ALL", debug);
   mSolver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Backward,*this);
}

OA_ptr<DataFlow::DataFlowSet> ManagerLivenessStandard::initializeTop()
{
    OA_ptr<TagsDFSet>  retval;
    retval = new TagsDFSet();
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> ManagerLivenessStandard::initializeBottom()
{
    OA_ptr<TagsDFSet> retval;
    retval = new TagsDFSet();
    return retval;
}

OA_ptr<LivenessStandard> ManagerLivenessStandard::performAnalysis(
    ProcHandle proc, 
    OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias,
    OA_ptr<SideEffect::InterSideEffectInterface> interSE,
    DataFlow::DFPImplement algorithm)
{
  if (debug) {
    std::cout << "In Liveness::ManagerLivenessStandard::performAnalysis"
              << std::endl;
  }
  mLiveMap = new LivenessStandard(proc);

  // store Alias information for use within the transfer function
  mAlias = alias;

  // get mapping of statements to locations they may and must define
  OA_ptr<OA::IRStmtIterator> sIt = mIR->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++) {
    OA::StmtHandle stmt = sIt->current();
    if (debug) {
      std::cout<< "\tstmt (" << stmt.hval() << ") = ";
      mIR->dump(stmt,std::cout);
    } 
    
    // initialize each stmt to define an empty set of locations
    mStmtMustDefMap[stmt] = new TagsDFSet();
    mStmtMayUseMap[stmt]  = new TagsDFSet();

   // locations that each statement may or must def
    OA_ptr<MemRefHandleIterator> defIterPtr = mIR->getDefMemRefs(stmt);
    for (; defIterPtr->isValid(); (*defIterPtr)++) {
        MemRefHandle ref = defIterPtr->current();
        if (debug) {
          std::cout << "\t\tdef ref (" << ref.hval() << ") = ";
          mIR->dump(ref,std::cout);
        }

        OA_ptr<AliasTagSet> tagSet = alias->getAliasTags(ref);
        OA_ptr<AliasTagIterator> tagIter;
        tagIter = tagSet->getIterator();
        for (; tagIter->isValid(); ++(*tagIter)) {
            if(tagSet->isMust()) {
                mStmtMustDefMap[stmt]->insert(tagIter->current());
            }
        }
    }

    /*
    // must or may defines from procedure calls
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
      CallHandle expr = callsiteItPtr->current();

      OA_ptr<LocIterator> locIterPtr;
      
      // DEF
      locIterPtr = interSE->getDEFIterator(expr);
      for ( ; locIterPtr->isValid(); (*locIterPtr)++) {
          OA_ptr<Location> defLocPtr = locIterPtr->current();
          mStmtMustDefMap[stmt].insert(defLocPtr);
      }
    }
*/

    OA_ptr<MemRefHandleIterator> useIterPtr = mIR->getUseMemRefs(stmt);
    for (; useIterPtr->isValid(); (*useIterPtr)++) {
        MemRefHandle useref = useIterPtr->current();
        if (debug) {
          std::cout << "\t\tuse ref (" << useref.hval() << ") = ";
          mIR->dump(useref,std::cout);
        }

        OA_ptr<AliasTagSet> tagSet = alias->getAliasTags(useref);
        OA_ptr<AliasTagIterator> tagIter;
        tagIter = tagSet->getIterator();
        for (; tagIter->isValid(); ++(*tagIter)) {
            mStmtMayUseMap[stmt]->insert(tagIter->current());
        }
    }

  }//loop over statements

  // use the dataflow solver to get the In and Out sets for the BBs
  //DataFlow::CFGDFProblem::solve(cfg);
  //

  mSolver->solve(cfg,algorithm);  
  
  // get exit node for CFG and determine what definitions reach that node
  OA_ptr<CFG::NodeInterface> node;
  
  node = cfg->getExit();

  OA_ptr<DataFlow::DataFlowSet> x = mSolver->getOutSet(node);

  OA_ptr<TagsDFSet> inSet
    = x.convert<TagsDFSet>();

   
  return mLiveMap;

}

OA_ptr<DataFlow::DataFlowSet>
ManagerLivenessStandard::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<TagsDFSet>  retval;
     retval = new TagsDFSet();
     return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerLivenessStandard::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<TagsDFSet>  retval;
     retval = new TagsDFSet();
     return retval;
}



OA_ptr<DataFlow::DataFlowSet> 
ManagerLivenessStandard::meet (OA_ptr<DataFlow::DataFlowSet> set1orig, 
                       OA_ptr<DataFlow::DataFlowSet> set2orig)
{
    OA_ptr<TagsDFSet> set1
        = set1orig.convert<TagsDFSet>();
    if (debug) {
        std::cout << "ManagerLivenessStandard::meet" << std::endl;
        std::cout << "\tset1 = ";
        set1->dump(std::cout,mIR);
        std::cout << ", set2 = ";
        set2orig->dump(std::cout,mIR);
    }
    
    OA_ptr<DataFlowSet> retvalOrig;
    retvalOrig = set1->clone();
    OA_ptr<TagsDFSet> retval;
    retval = retvalOrig.convert<TagsDFSet>();
    retval->unionEqu(*set2orig);
    if (debug) {
        std::cout << std::endl << "\tretval set = ";
        retval->dump(std::cout,mIR);
        std::cout << std::endl;
    }
       
    return retval;
}


OA_ptr<DataFlow::DataFlowSet> 
ManagerLivenessStandard::transfer(
    OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt) 
{
    OA_ptr<TagsDFSet> inRecast 
        = in.convert<TagsDFSet>();

    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
    }


    if(mStmtMustDefMap[stmt].ptrEqual(NULL)) {
        mStmtMustDefMap[stmt] = new TagsDFSet();
    }
    if(mStmtMayUseMap[stmt].ptrEqual(NULL)) {
        mStmtMayUseMap[stmt] = new TagsDFSet();
    }

    OA_ptr<TagsDFSetIter> inIter;
    inIter = new TagsDFSetIter(inRecast);
    for(; inIter->isValid(); ++(*inIter)) {
        AliasTag livevar = inIter->current();
        if (debug) {
          std::cout << "\tlivevar in Inset = ";
          //livevar.dump(std::cout,mIR);
        }
   
       mLiveMap->insertLive(stmt, livevar);
    }

    OA_ptr<TagsDFSetIter> setIter;
    setIter = new TagsDFSetIter(mStmtMustDefMap[stmt]);
    for(; setIter->isValid(); ++(*setIter))
    {
        AliasTag loc = setIter->current();
        inRecast->remove(loc);
        mLiveMap->removeLive(stmt,loc);
    }

    OA_ptr<TagsDFSetIter> gensetIter;
    gensetIter = new TagsDFSetIter(mStmtMayUseMap[stmt]);
    for(; gensetIter->isValid(); ++(*gensetIter))
    {
        AliasTag loc = gensetIter->current();
        inRecast->insert(loc);
        mLiveMap->insertLive(stmt,loc);
    }

    return inRecast;
 }

  } // end of namespace Liveness
} // end of namespace OA
