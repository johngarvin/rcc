/*! \file
  
  \brief Implementation of ParamBindings

  \authors Michelle Strout 
  \version $Id: ParamBindings.cpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $


  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ParamBindings.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DataFlow {

//*****************************************************************
// Construction methods 
//*****************************************************************

void ParamBindings::mapFormalToActualForCall(CallHandle call, 
                                             OA_ptr<MemRefExpr> formal, 
                                             OA_ptr<ExprTree> actual)
{
  mCallToFormalToActual[call].push_back(
     std::pair<OA_ptr<MemRefExpr>,OA_ptr<ExprTree> >(formal, actual) );
}


OA_ptr<ParamBindIterator> 
ParamBindings::getParamBindIterator(CallHandle call)
{
  std::list<std::pair<OA_ptr<MemRefExpr>, OA_ptr<ExprTree> > > formalToactual;
  formalToactual = mCallToFormalToActual[call];
  OA_ptr<ParamBindIterator> retval;
  retval = new OA::DataFlow::ParamBindIterator(formalToactual);
  return retval;
}



void ParamBindings::output(IRHandlesIRInterface& ir) const
{
  std::map<CallHandle, std::list<std::pair<OA_ptr<OA::MemRefExpr>,
           OA_ptr<OA::ExprTree> > > >::const_iterator mIter;

  sOutBuild->objStart("ParamBindings");
  
  for(mIter = mCallToFormalToActual.begin();
      mIter != mCallToFormalToActual.end();
      ++mIter)
  {
      const CallHandle &first = mIter->first;
      std::list<std::pair<OA_ptr<OA::MemRefExpr>,
      OA_ptr<OA::ExprTree> > > second = mIter->second;



      sOutBuild->mapEntryStart();
      sOutBuild->mapKeyStart();
      sOutBuild->outputIRHandle(first, ir);
      sOutBuild->mapKeyEnd();
      sOutBuild->mapValueStart();
     
 

      std::list<std::pair<OA_ptr<MemRefExpr>,
      OA_ptr<ExprTree> > >::const_iterator pairIter;

      for (pairIter = mCallToFormalToActual.find(first)->second.begin();
           pairIter!= mCallToFormalToActual.find(first)->second.end();
           ++pairIter)
      {
           OA_ptr<MemRefExpr> lhs = pairIter->first;
           OA_ptr<ExprTree> exprTree = pairIter->second;

           sOutBuild->mapEntryStart();
                 sOutBuild->mapKeyStart();
                       lhs->output(ir);
                 sOutBuild->mapKeyEnd();
                 sOutBuild->mapValueStart();
                       exprTree->output(ir);
                 sOutBuild->mapValueEnd();
           sOutBuild->mapEntryEnd();

      }


      sOutBuild->mapEnd("second");
      sOutBuild->mapValueEnd();
      sOutBuild->mapEntryEnd();

      
  }

  sOutBuild->objEnd("ParamBindings");
  
}



  } // end of DataFlow namespace
} // end of OA namespace

