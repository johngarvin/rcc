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

#include "VaryStandard.hpp"

namespace OA {
  namespace Activity {

VaryStandard::VaryStandard(ProcHandle p) 
{ 
    mIndepTagSet = new DataFlow::DataFlowSetImpl<int>(); 
}

OA_ptr<DataFlow::DataFlowSetImplIterator<int> > 
VaryStandard::getIndepSetIterator() 
{ 
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
  retval = new DataFlow::DataFlowSetImplIterator<int>(mIndepTagSet);
  return retval;
}

OA_ptr<DataFlow::DataFlowSetImplIterator<int> > 
VaryStandard::getOutVaryIterator(StmtHandle s)
{ 
    OA_ptr<DataFlow::DataFlowSetImplIterator<int> > retval;
    if (mOutVary[s].ptrEqual(0)) {
        mOutVary[s] = new DataFlow::DataFlowSetImpl<int>();
    }
    retval = new DataFlow::DataFlowSetImplIterator<int>(mOutVary[s]);
    return retval;
}

OA_ptr<DataFlow::DataFlowSetImpl<int> > VaryStandard::getInVarySet(ExprHandle call)
{
    if (mInVary.find(call)!=mInVary.end()) {
        return mInVary[call];
    } else {
        OA_ptr<DataFlow::DataFlowSetImpl<int> > emptySet; 
        emptySet = new DataFlow::DataFlowSetImpl<int>();
        return emptySet;
    }
}

//*****************************************************************
// Construction methods 
//*****************************************************************

void VaryStandard::insertIndepTag(int tag) 
{ 
    mIndepTagSet->insert(tag); 
}

void VaryStandard::copyIntoOutVary(StmtHandle s, 
                                  OA_ptr<DataFlow::DataFlowSetImpl<int> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mOutVary[s] = cloneDFset.convert<DataFlow::DataFlowSetImpl<int> >();
}

void VaryStandard::copyIntoInVary(ExprHandle call, 
                                  OA_ptr<DataFlow::DataFlowSetImpl<int> > dfset)
{
    OA_ptr<DataFlow::DataFlowSet> cloneDFset = dfset->clone();
    mInVary[call] = cloneDFset.convert<DataFlow::DataFlowSetImpl<int> >();
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
             OA_ptr<DataFlow::DataFlowSetImpl<int> > >::iterator mapIter;
    for (mapIter = mOutVary.begin(); mapIter != mOutVary.end(); mapIter++) {
        StmtHandle s = mapIter->first;

        os << "StmtHandle(" << s.hval() << ") " << ir->toString(s) 
           << std::endl << "\tOutVary: " << std::endl;
        mapIter->second->dump(os,ir);
    }
}



  } // end of Activity namespace
} // end of OA namespace
