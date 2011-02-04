/*! \file
  
  \brief Implementation of UDDUChains::UDDUChainsStandard

  \author Michelle Strout
  \version $Id: UDDUChainsStandard.cpp,v 1.10 2004/12/23 07:37:53 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "UDDUChainsStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace UDDUChains {
//! Return an iterator over all statements in this procedure
//! that may define a location used in the given statement
OA_ptr<Interface::ChainStmtIterator> 
UDDUChainsStandard::getUDChainStmtIterator(StmtHandle s)
{ 
  OA_ptr<ChainStmtIterator> retval;

  // need to create an empty StmtSet if we haven't seen this stmt before
  if ((*mUDChainForStmt)[s].ptrEqual(0)) {
    (*mUDChainForStmt)[s] = new StmtSet;
  }
  retval = new ChainStmtIterator((*mUDChainForStmt)[s]);
  return retval;
}

//! Return an iterator over all statements for which UDChain info
//! is available
OA_ptr<Interface::StmtsWithUDChainIterator> 
UDDUChainsStandard::getStmtsWithUDChainIterator()
{
    OA_ptr<StmtsWithUDChainIterator> retval;
    retval = new StmtsWithUDChainIterator(mUDChainForStmt);
    return retval;
}

//! Return an iterator over all statements in this procedure
//! that may use a location defined in the given statement
OA_ptr<Interface::ChainStmtIterator> 
UDDUChainsStandard::getDUChainStmtIterator(StmtHandle s)
{ 
  OA_ptr<ChainStmtIterator> retval;

  // need to create an empty StmtSet if we haven't seen this stmt before
  if ((*mDUChainForStmt)[s].ptrEqual(0)) {
    (*mDUChainForStmt)[s] = new StmtSet;
  }
  retval = new ChainStmtIterator((*mDUChainForStmt)[s]);
  return retval;
}

//! Return an iterator over all statements for which UDChain info
//! is available
OA_ptr<Interface::StmtsWithDUChainIterator> 
UDDUChainsStandard::getStmtsWithDUChainIterator()
{
    OA_ptr<StmtsWithDUChainIterator> retval;
    retval = new StmtsWithDUChainIterator(mDUChainForStmt);
    return retval;
}


//! Return an iterator over all statements in this procedure
//! that may define a location used by the given memory reference
OA_ptr<Interface::ChainStmtIterator> 
UDDUChainsStandard::getUDChainStmtIterator(MemRefHandle h)
{ 
  OA_ptr<ChainStmtIterator> retval;

  // need to create an empty StmtSet if we haven't seen this memref before
  if ((*mUDChainForMemRef)[h].ptrEqual(0)) {
    (*mUDChainForMemRef)[h] = new StmtSet;
  }
  retval = new ChainStmtIterator((*mUDChainForMemRef)[h]);
  return retval;
}

//! Return an iterator over all memory references for which UDChain info
//! is available
OA_ptr<Interface::MemRefsWithUDChainIterator> 
UDDUChainsStandard::getMemRefsWithUDChainIterator()
{
    OA_ptr<MemRefsWithUDChainIterator> retval;
    retval = new MemRefsWithUDChainIterator(mUDChainForMemRef);
    return retval;
}

//! Return an iterator over all statements in this procedure
//! that may use a location defined by the given memory reference
OA_ptr<Interface::ChainStmtIterator> 
UDDUChainsStandard::getDUChainStmtIterator(MemRefHandle h)
{ 
  OA_ptr<ChainStmtIterator> retval;

  // need to create an empty StmtSet if we haven't seen this memref before
  if ((*mDUChainForMemRef)[h].ptrEqual(0)) {
    (*mDUChainForMemRef)[h] = new StmtSet;
  }
  retval = new ChainStmtIterator((*mDUChainForMemRef)[h]);
  return retval;
}

//! Return an iterator over all memrefs for which UDChain info
//! is available
OA_ptr<Interface::MemRefsWithDUChainIterator> 
UDDUChainsStandard::getMemRefsWithDUChainIterator()
{
    OA_ptr<MemRefsWithDUChainIterator> retval;
    retval = new MemRefsWithDUChainIterator(mDUChainForMemRef);
    return retval;
}
//*****************************************************************
// Construction methods 
//*****************************************************************

//! insert a define use relationship
void UDDUChainsStandard::insertDefUse(StmtHandle def, StmtHandle use)
{ 
    if ((*mDUChainForStmt)[def].ptrEqual(0)) {
      (*mDUChainForStmt)[def] = new StmtSet;
    }
    (*mDUChainForStmt)[def]->insert(use); 

    if ((*mUDChainForStmt)[use].ptrEqual(0)) {
      (*mUDChainForStmt)[use] = new StmtSet;
    }
    (*mUDChainForStmt)[use]->insert(def); 
}

//! insert a memref, the memref may end up having an empty
//! chains so must make sure chain is initialized
void UDDUChainsStandard::insertMemRefDef(MemRefHandle ref)
{ 
    if ((*mDUChainForMemRef)[ref].ptrEqual(0)) {
      (*mDUChainForMemRef)[ref] = new StmtSet;
    }
}
void UDDUChainsStandard::insertMemRefUse(MemRefHandle ref)
{ 
    if ((*mUDChainForMemRef)[ref].ptrEqual(0)) {
      (*mUDChainForMemRef)[ref] = new StmtSet;
    }
}


//! insert a define use relationship with the specific def
//! memory reference
void UDDUChainsStandard::insertMemRefDefStmtUse(MemRefHandle def, StmtHandle use)
{ 
    if ((*mDUChainForMemRef)[def].ptrEqual(0)) {
      (*mDUChainForMemRef)[def] = new StmtSet;
    }
    (*mDUChainForMemRef)[def]->insert(use); 
}

//! insert a define use relationship with the specific use
//! memory reference
void UDDUChainsStandard::insertStmtDefMemRefUse(StmtHandle def, MemRefHandle use)
{ 
    if ((*mUDChainForMemRef)[use].ptrEqual(0)) {
      (*mUDChainForMemRef)[use] = new StmtSet;
    }
    (*mUDChainForMemRef)[use]->insert(def); 
}


//*****************************************************************
// Output
//*****************************************************************

//! incomplete output of info for debugging
void UDDUChainsStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) 
{
    os << "============= UDDUChains ============" << std::endl;
    // first iterate over all statements that have a UDChain associated
    // with them
    OA_ptr<Interface::StmtsWithUDChainIterator> useStmtIterPtr
        = getStmtsWithUDChainIterator(); 
    for ( ; useStmtIterPtr->isValid(); (*useStmtIterPtr)++ )
    {
        StmtHandle useStmt = useStmtIterPtr->current();

        os << "Use StmtHandle(" << useStmt.hval() << ") " << ir->toString(useStmt) 
           << std::endl << "\tDefine Statements: " << std::endl;
        // iterate over reaching defs for statement useStmt
        OA_ptr<Interface::ChainStmtIterator> udChainPtr
            = getUDChainStmtIterator(useStmt);
        for ( ; udChainPtr->isValid(); (*udChainPtr)++) {
            StmtHandle s = udChainPtr->current();
            if (s.hval()==0) {
              os << "\tStmtHandle(0)" << std::endl;
            } else {
              os << "\t" << ir->toString(s) << std::endl;
            }
        }

    }

    // now iterate over all statements that have a DUChain associated
    // with them
    OA_ptr<Interface::StmtsWithDUChainIterator> defStmtIterPtr
        = getStmtsWithDUChainIterator(); 
    for ( ; defStmtIterPtr->isValid(); (*defStmtIterPtr)++ )
    {
        StmtHandle defStmt = defStmtIterPtr->current();

        os << "Def StmtHandle(" << defStmt.hval() << ") " << ir->toString(defStmt) 
           << std::endl << "\tUse Statements: " << std::endl;
        // iterate over uses for statement defStmt
        OA_ptr<Interface::ChainStmtIterator> duChainPtr
            = getDUChainStmtIterator(defStmt);
        for ( ; duChainPtr->isValid(); (*duChainPtr)++) {
            os << "\t" << ir->toString(duChainPtr->current()) << std::endl;
        }

    }

    // use memrefs that have UDChains
    OA_ptr<Interface::MemRefsWithUDChainIterator> useIterPtr
        = getMemRefsWithUDChainIterator();
    for ( ; useIterPtr->isValid(); (*useIterPtr)++ ) {
        MemRefHandle use = useIterPtr->current();

        os << "Use MemRefHandle(" << use.hval() << ") " << ir->toString(use) 
           << std::endl << "\tDefine Statements: " << std::endl;
        // iterate over defs for memref use
        OA_ptr<Interface::ChainStmtIterator> udChainPtr
            = getUDChainStmtIterator(use);
        for ( ; udChainPtr->isValid(); (*udChainPtr)++) {
            StmtHandle s = udChainPtr->current();
            if (s.hval()==0) {
              os << "\tStmtHandle(0)" << std::endl;
            } else {
              os << "\t" << ir->toString(s) << std::endl;
            }
        }

    }

    // def memrefs that have DUChains
    OA_ptr<Interface::MemRefsWithUDChainIterator> defIterPtr
        = getMemRefsWithDUChainIterator();
    for ( ; defIterPtr->isValid(); (*defIterPtr)++ ) {
        MemRefHandle def = defIterPtr->current();

        os << "Def MemRefHandle(" << def.hval() << ") " << ir->toString(def) 
           << std::endl << "\tUse Statements: " << std::endl;
        // iterate over uses for memref def
        OA_ptr<Interface::ChainStmtIterator> duChainPtr
            = getDUChainStmtIterator(def);
        for ( ; duChainPtr->isValid(); (*duChainPtr)++) {
            os << "\t" << ir->toString(duChainPtr->current()) << std::endl;
        }

    }


}


void UDDUChainsStandard::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("UDDUChainsStandard");

    sOutBuild->mapStart("mUDChainForStmt", "StmtHandle", "OA::OA_ptr<StmtSet> ");
    std::map<StmtHandle, OA::OA_ptr<StmtSet> >::iterator reg_mUDChainForStmt_iterator;
    for(reg_mUDChainForStmt_iterator = mUDChainForStmt->begin();
        reg_mUDChainForStmt_iterator != mUDChainForStmt->end();
        reg_mUDChainForStmt_iterator++)
    {
        const StmtHandle &first = reg_mUDChainForStmt_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mUDChainForStmt_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Use StatementHandle ");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("UseStatementHandle");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();

        sOutBuild->fieldStart("Define Statements"); {
           sOutBuild->listStart(); {

            std::set<OA::StmtHandle>::iterator sIter;
 
            for( sIter = second->begin(); sIter != second->end() ; ++(sIter) ) {
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(*(sIter),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->fieldEnd("Define Statements");


        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();

  }
  sOutBuild->mapEnd("mUDChainForStmt");



  sOutBuild->mapStart("mDUChainForStmt", "StmtHandle", "OA::OA_ptr<StmtSet> ");
  std::map<StmtHandle, OA::OA_ptr<StmtSet> >::iterator reg_mDUChainForStmt_iterator;
  for(reg_mDUChainForStmt_iterator = mDUChainForStmt->begin();
      reg_mDUChainForStmt_iterator != mDUChainForStmt->end();
      reg_mDUChainForStmt_iterator++)
  {

        const StmtHandle &first = reg_mDUChainForStmt_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mDUChainForStmt_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Def StatementHandle ");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("DefStatementHandle");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();

        sOutBuild->fieldStart("Use Statements"); {
           sOutBuild->listStart(); {

            std::set<OA::StmtHandle>::iterator sIter;

            for( sIter = second->begin(); sIter != second->end() ; ++(sIter) ) {
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(*(sIter),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->fieldEnd("Use Statements");


        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();

  }
  sOutBuild->mapEnd("mDUChainForStmt");




    sOutBuild->mapStart("mUDChainForMemRef", "MemRefHandle", "OA::OA_ptr<StmtSet> ");
    std::map<MemRefHandle, OA::OA_ptr<StmtSet> >::const_iterator 
                                 reg_mUDChainForMemRef_iterator;

    for(reg_mUDChainForMemRef_iterator = mUDChainForMemRef->begin();
        reg_mUDChainForMemRef_iterator != mUDChainForMemRef->end();
        reg_mUDChainForMemRef_iterator++)
    {
        const MemRefHandle &first = reg_mUDChainForMemRef_iterator->first;
        const OA::OA_ptr<StmtSet> &second 
                       = reg_mUDChainForMemRef_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Use MemRefHandle");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("Use MemRefHandle");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        sOutBuild->fieldStart("Def Statements"); {
           sOutBuild->listStart(); {
            std::set<StmtHandle>::const_iterator stmtIterPtr;
            for(stmtIterPtr=second->begin(); stmtIterPtr!=second->end();
                ++stmtIterPtr) {
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(*(stmtIterPtr),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->fieldEnd("Def Statements");

        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mUDChainForMemRef");






    sOutBuild->mapStart("mDUChainForMemRef", "MemRefHandle", "OA::OA_ptr<StmtSet> ");
    std::map<MemRefHandle, OA::OA_ptr<StmtSet> >::const_iterator
                                 reg_mDUChainForMemRef_iterator;

    for(reg_mDUChainForMemRef_iterator = mDUChainForMemRef->begin();
        reg_mDUChainForMemRef_iterator != mDUChainForMemRef->end();
        reg_mDUChainForMemRef_iterator++)
    {
        const MemRefHandle &first = reg_mDUChainForMemRef_iterator->first;
        const OA::OA_ptr<StmtSet> &second
                       = reg_mDUChainForMemRef_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Def MemRefHandle");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("Def MemRefHandle");

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        sOutBuild->fieldStart("Use Statements"); {
           sOutBuild->listStart(); {
            std::set<StmtHandle>::const_iterator stmtIterPtr;
            for(stmtIterPtr=second->begin(); stmtIterPtr!=second->end();
                ++stmtIterPtr) {
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(*(stmtIterPtr),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->fieldEnd("Use Statements");

        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mDUChainForMemRef");

} 


  } // end of UDDUChains namespace
} // end of OA namespace
