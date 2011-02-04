/*! \file
  
  \brief Implementation of ReachConsts::ReachConstsStandard

  \author Michelle Strout, Barbara Kreaseck
  \version $Id: ReachConstsStandard.cpp,v 1.7 2005/03/18 18:14:16 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#include "ReachConsts.hpp"
#include <Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace ReachConsts {

static bool debug = false;


//! ===================================================================
//! ReachConstsStandard methods
//! ===================================================================

ReachConstsStandard::ReachConstsStandard(ProcHandle p) 
{
  OA_DEBUG_CTRL_MACRO("DEBUG_ReachConstsStandard:ALL", debug);
}





//! Return the ConstValBasicInterface of the reaching constant
//! for the given MemRefExpr-AliasTagSet and StmtHandle
//! Returns NULL if no constant value exists for this MemRefExpr-AliasTagSet
//! at this StmtHandle
OA_ptr<ConstValBasicInterface> 
ReachConstsStandard::getReachConst(StmtHandle stmt,OA_ptr<Alias::AliasTagSet> tset)
{
   OA_ptr<ConstValBasicInterface> retval; retval = NULL;

   if (tset.ptrEqual(0)) {return retval;}

   OA_ptr<RCPairSet> rcSet = mReachConsts[stmt];
   assert(!rcSet.ptrEqual(0));
   OA_ptr<RCPairSetIterator> rcIter;
   rcIter = new RCPairSetIterator(*rcSet);
   bool first = true;
   bool conflict = false;
   OA_ptr<ConstValBasicInterface> tempPtr;
   
   OA_ptr<Alias::AliasTagIterator> tIter;
   tIter = tset->getIterator();
   for (; tIter->isValid()&&!conflict; ++(*tIter)) {
     Alias::AliasTag tag = tIter->current();
     for(rcIter->reset(); rcIter->isValid(); ++(*rcIter)) {
       RCPair rc = rcIter->current();
       if (rc.getTag() == tag) {
         if (first) {
           retval = rc.getConstPtr();
           assert(!retval.ptrEqual(0));
         } else {
           tempPtr = rc.getConstPtr();
           if (tempPtr != retval) {
             conflict = true;
           }
         }
       }
     }
   }

   if (conflict) {
     retval = NULL;
   }

   return retval;
}

//! Return the ConstValBasicInterface of the reaching constant
//! for the given MemRefExpr-AliasTagSet and CallHandle
//! Returns NULL if no constant value exists for this MemRefExpr-AliasTagSet
//! at this CallHandle
OA_ptr<ConstValBasicInterface> 
ReachConstsStandard::getReachConst(CallHandle call,OA_ptr<Alias::AliasTagSet> tset)
{
   OA_ptr<ConstValBasicInterface> retval; retval = NULL;

   if (tset.ptrEqual(0)) {return retval;}

   OA_ptr<RCPairSet> rcSet = mReachConstsForCallStmt[call];
   assert(!rcSet.ptrEqual(0));
   OA_ptr<RCPairSetIterator> rcIter;
   rcIter = new RCPairSetIterator(*rcSet);
   bool first = true;
   bool conflict = false;
   OA_ptr<ConstValBasicInterface> tempPtr;
   
   OA_ptr<Alias::AliasTagIterator> tIter;
   tIter = tset->getIterator();
   for (; tIter->isValid()&&!conflict; ++(*tIter)) {
     Alias::AliasTag tag = tIter->current();
     for(rcIter->reset(); rcIter->isValid(); ++(*rcIter)) {
       RCPair rc = rcIter->current();
       if (rc.getTag() == tag) {
         if (first) {
           retval = rc.getConstPtr();
           assert(!retval.ptrEqual(0));
         } else {
           tempPtr = rc.getConstPtr();
           if (tempPtr != retval) {
             conflict = true;
           }
         }
       }
     }
   }

   if (conflict) {
     retval = NULL;
   }

   return retval;
}




//! reset a statement's RCPairSet by sending in a NULL OA_ptr
//! otherwise, stores an RCPairSet that reaches this StmtHandle
void ReachConstsStandard::storeRCPairSet(StmtHandle s, 
                                         OA_ptr<RCPairSet> rcSet)
{
  if (rcSet.ptrEqual(0)) {
    mReachConsts[s] = new RCPairSet();
  } else {
    mReachConsts[s] = (rcSet->clone()).convert<RCPairSet>();
  }
}


//! Return the set of reaching constants for a given stmt
OA_ptr<RCPairSetIterator>
ReachConstsStandard::getRCPairIterator(StmtHandle s)
{
   if (mReachConsts[s].ptrEqual(NULL)) {
        mReachConsts[s] = new RCPairSet();
   }

   OA_ptr<RCPairSetIterator> retval;
   retval = new RCPairSetIterator(*(mReachConsts[s]));
   return retval;
}




//! reset a call's RCPairSet by sending in a NULL OA_ptr
//! otherwise, stores an RCPairSet that reaches this CallHandle
void ReachConstsStandard::storeRCPairSet(CallHandle s, 
                                         OA_ptr<RCPairSet> rcSet)
{
  if (rcSet.ptrEqual(0)) {
    mReachConstsForCallStmt[s] = new RCPairSet();
  } else {
    mReachConstsForCallStmt[s] = (rcSet->clone()).convert<RCPairSet>();
  }
}


//! Return the set of reaching constants for a given call handle
OA_ptr<RCPairSetIterator>
ReachConstsStandard::getRCPairIterator(CallHandle s)
{
   if (mReachConstsForCallStmt[s].ptrEqual(NULL)) {
     mReachConstsForCallStmt[s] = new RCPairSet();
   }

   OA_ptr<RCPairSetIterator> retval;
   retval = new RCPairSetIterator(*(mReachConstsForCallStmt[s]));
   return retval;
}





//! =============== Output/Debugging =====================
void ReachConstsStandard::output(IRHandlesIRInterface& ir) const
{
    std::cout << "Please call output with alias::Interface"
              << std::endl;
    assert(0);
}






void ReachConstsStandard::output(OA::IRHandlesIRInterface &ir,
                                 Alias::Interface& alias) const
{
    sOutBuild->objStart("ReachConstsStandard");

    sOutBuild->mapStart("mReachConsts", "StmtHandle", "OA::OA_ptr<RCPairSet> ");
    std::map<StmtHandle, OA::OA_ptr<RCPairSet> >::const_iterator
        reg_mReachConsts_iterator;
    for(reg_mReachConsts_iterator = mReachConsts.begin();
        reg_mReachConsts_iterator != mReachConsts.end();
        reg_mReachConsts_iterator++)
    {
        const StmtHandle &first = reg_mReachConsts_iterator->first;
        OA::OA_ptr<RCPairSet> second = reg_mReachConsts_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Statement");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("Statement");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        second->output(ir,alias);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mReachConsts");


    sOutBuild->mapStart("mReachConstsForCallStmt", "CallHandle", "OA::OA_ptr<RCPairSet> ");
    std::map<CallHandle, OA::OA_ptr<RCPairSet> >::const_iterator
        reg_mReachConstsForCallStmt_iterator;
    for(reg_mReachConstsForCallStmt_iterator = mReachConstsForCallStmt.begin();
        reg_mReachConstsForCallStmt_iterator != mReachConstsForCallStmt.end();
        reg_mReachConstsForCallStmt_iterator++)
    {
        const CallHandle &first = reg_mReachConstsForCallStmt_iterator->first;
        OA::OA_ptr<RCPairSet> second = reg_mReachConstsForCallStmt_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("CallHandle");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("CallHandle");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        second->output(ir,alias);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mReachConstsForCallStmt");


    sOutBuild->objEnd("ReachConstsStandard");
}







//! incomplete output of info for debugging, just lists stmts
//! and associated set of RCPair that are reaching const defs for the given
void ReachConstsStandard::dump(std::ostream& os,
                             OA_ptr<OA::IRHandlesIRInterface> ir)
{
    std::map<StmtHandle, OA_ptr<RCPairSet> >::iterator mapIter;
    for (mapIter = mReachConsts.begin(); mapIter != mReachConsts.end(); mapIter++) {
        StmtHandle s = mapIter->first;

        os << "StmtHandle(" << s.hval() << ") " << ir->toString(s)
           << std::endl << "\treachConsts: " << std::endl;
        // iterate over reaching consts for statement s

        OA_ptr<RCPairSet> rcSet = mapIter->second;
        rcSet->dump(os,ir);

        os << std::endl;
    }

    std::map<CallHandle, OA_ptr<RCPairSet> >::iterator cIter;
    for (cIter = mReachConstsForCallStmt.begin();
         cIter != mReachConstsForCallStmt.end();
         cIter++) {

        CallHandle c = cIter->first;
        OA_ptr<RCPairSet> rcSet = cIter->second;
        
        os << "CallHandle ( " << ir->toString(c) << " )\n"
           << "\treachConsts:\n";
        rcSet->dump(os,ir);
        os << std::endl;
    }
}






  } // end of ReachConsts namespace
} // end of OA namespace
