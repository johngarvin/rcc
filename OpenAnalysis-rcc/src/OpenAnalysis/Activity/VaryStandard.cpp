/*! \file
  
  \brief Implementation of Activity::VaryStandard

  \author Michelle Strout
  \version $Id: VaryStandard.cpp,v 1.5 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/Activity/VaryStandard.hpp>

namespace OA {
  namespace Activity {

VaryStandard::VaryStandard(ProcHandle p) 
{ 
    mIndepTagSet = new DataFlow::DataFlowSetImpl<Alias::AliasTag>(); 
}

OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > 
VaryStandard::getIndepSetIterator() 
{ 
  OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > retval;
  retval = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mIndepTagSet);
  return retval;
}

OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > 
VaryStandard::getOutVaryIterator(StmtHandle s)
{ 
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > retval;
    if (mOutVary[s].ptrEqual(0)) {
        mOutVary[s] = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();
    }
    retval = new DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mOutVary[s]);
    return retval;
}

OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > VaryStandard::getInVarySet(ExprHandle call)
{
    if (mInVary.find(call)!=mInVary.end()) {
        return mInVary[call];
    } else {
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > emptySet; 
        emptySet = new DataFlow::DataFlowSetImpl<Alias::AliasTag>();
        return emptySet;
    }
}

//*****************************************************************
// Construction methods 
//*****************************************************************

void VaryStandard::insertIndepTag(Alias::AliasTag tag) 
{ 
    mIndepTagSet->insert(tag); 
}

void VaryStandard::copyIntoOutVary(StmtHandle s, 
                                  OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mOutVary[s] = cloneDFset.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
}

void VaryStandard::copyIntoInVary(ExprHandle call, 
                                  OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mInVary[call] = cloneDFset.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
}

   
//*****************************************************************
// Output
//*****************************************************************

//! incomplete output of info for debugging
void VaryStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "============================ VaryStandard" << std::endl;
    os << "IndepSet = " << std::endl;
    mIndepTagSet->dump(os,ir);

    // iterate over all stmts we know about
    std::map<StmtHandle, 
             OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > >::iterator mapIter;
    for (mapIter = mOutVary.begin(); mapIter != mOutVary.end(); mapIter++) {
        StmtHandle s = mapIter->first;

        os << "StmtHandle(" << s.hval() << ") " << ir->toString(s) 
           << std::endl << "\tOutVary: " << std::endl;
        mapIter->second->dump(os,ir);
    }
}



  } // end of Activity namespace
} // end of OA namespace
