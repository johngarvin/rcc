/*! \file
  
\brief The AnnotationManager that generates InterActive 
INTERprocedurally.

\authors Jaewook Shin
\version $Id: ManagerDUActive.cpp,v 1.0 2005/04/10

Copyright (c) 2002-2004, Rice University <br>
Copyright (c) 2004, University of Chicago <br>  
All rights reserved. <br>
See ../../../Copyright.txt for details. <br>

*/

#include <iostream>
#include "ManagerDUActive.hpp"



namespace OA {
  namespace Activity {

#if defined(DEBUG_ALL) || defined(DEBUG_ManagerDUActive)
static bool debug = true;
#else
static bool debug = false;
#endif

ManagerDUActive::ManagerDUActive(
    OA_ptr<Activity::ActivityIRInterface> _ir,
    OA_ptr<OA::DUG::DUGStandard> _dug)
    : mIR(_ir), mDUG(_dug)
{    
}

/*!
 */
void
ManagerDUActive::markVaried(bool activeWithVariedOnly)
{
    std::list<CallHandle> callStack;
    std::set<OA_ptr<DUG::EdgeInterface> > visited;
    std::set<std::pair<unsigned,unsigned> > onPath;

#ifdef DEBUG_DUAA
    std::cout << "ManagerDUActive::markVaried: ---" << std::endl;
#endif
    // iterate over independent variables
    std::list<std::pair<SymHandle, ProcHandle> >& indepSyms = mDUG->getIndepSyms();
    std::list<std::pair<SymHandle, ProcHandle> >::iterator indepIter = indepSyms.begin();
    for (; indepIter != indepSyms.end(); indepIter++) {
	callStack.empty();
	std::pair<SymHandle, ProcHandle> indepPair = *indepIter;
	SymHandle sym = indepPair.first;
	ProcHandle proc = indepPair.second;
#ifdef DEBUG_DUAA
	std::cout << "ManagerDUActive::markVaried: Independent(" << mIR->toString(sym) 
		  << "), isNode(" << mDUG->isNode(sym, proc) << ")" << std::endl;
#endif
    
	if (mDUG->isNode(sym, proc)){
	    OA_ptr<DUG::NodeInterface> node = mDUG->getNode(sym, proc);
	    std::pair<unsigned, unsigned> pathNode(1, node->getId());
	    onPath.insert(pathNode);
	    node->markVaried(callStack, mIR, visited, onPath, node->getProc(), 
			     (unsigned)-1, OA_ptr<DUG::EdgeInterface>(),
			     activeWithVariedOnly);
	    onPath.erase(pathNode);
	}
    }
  
}

/*!
 */
void
ManagerDUActive::markUseful()
{
    std::list<CallHandle> callStack;
    std::set<OA_ptr<DUG::EdgeInterface> > visited;
    std::set<std::pair<unsigned,unsigned> > onPath;


#ifdef DEBUG_DUAA
    std::cout << "ManagerDUActive::markUseful: ---" << std::endl;
#endif
    // iterate over dependent variables
    std::list<std::pair<SymHandle, ProcHandle> >& depSyms = mDUG->getDepSyms();
    std::list<std::pair<SymHandle, ProcHandle> >::iterator depIter = depSyms.begin();
    for (; depIter != depSyms.end(); depIter++) {
	callStack.empty();
	std::pair<SymHandle, ProcHandle> depPair = *depIter;
	SymHandle sym = depPair.first;
	ProcHandle proc = depPair.second;
#ifdef DEBUG_DUAA
	std::cout << "ManagerDUActive::markUseful: Dependent(" << mIR->toString(sym) 
		  << "), isNode(" << mDUG->isNode(sym, proc) << ")" << std::endl;
#endif

	if (mDUG->isNode(sym, proc)){
	    OA_ptr<DUG::NodeInterface> node = mDUG->getNode(sym, proc);
	    std::pair<unsigned, unsigned> pathNode(1, node->getId());
	    onPath.insert(pathNode);


      
	    node->markUseful(callStack, mIR, visited, onPath, node->getProc(), 
			     (unsigned)-1, OA_ptr<DUG::EdgeInterface>());
	    onPath.erase(pathNode);
	}
    }
}

/*!
 */
OA_ptr<InterActiveFortran> 
ManagerDUActive::performAnalysis(OA_ptr<DataFlow::ParamBindings> paramBind,
				 bool activeWithVariedOnly)
{
#ifdef DEBUG_DUAA
  std::cout << "ManagerDUActive::performAnalysis: with activeWithVariedOnly = " << activeWithVariedOnly << std::endl;
#endif
    OA_ptr<InterActiveFortran> retval;
    retval = new InterActiveFortran;

    // Def-Use Activity Analysis
    markVaried(activeWithVariedOnly);
    if (!activeWithVariedOnly)
      markUseful();
  
    //-------------------------------------------------------------
    // copy active SymHandles from mDUG to 'retval'
    OA_ptr<std::set<SymHandle> > activeSymSet = mDUG->getActiveSymSet();
  
    std::set<SymHandle>::iterator ssIter = activeSymSet->begin();
    for (; ssIter != activeSymSet->end(); ssIter++){
	retval->setActive(*ssIter);
    }
  
    // store results that will be needed in callbacks
    mParamBind  = paramBind;

    // create an empty InterActive
    mInterActiveFortran = new InterActiveFortran();

    mActiveMap = mDUG->getActiveMap();

    // then iterate over each statement to find active use memrefs

    // assign activity results for each procedure into InterActiveFortran
    std::map<ProcHandle,OA_ptr<ActiveStandard> >::iterator mapIter;
    for (mapIter=mActiveMap.begin(); mapIter!=mActiveMap.end(); mapIter++) {
	retval->mapProcToActive(mapIter->first,mapIter->second);
    }

    // iterate over all symbols to determine the size
    OA_ptr<SymHandleIterator> symIter = retval->getActiveSymIterator();
    int bytes = 0;
    int numActiveSyms = 0;
    for ( ; symIter->isValid(); (*symIter)++ ) {
	SymHandle sym = symIter->current();

	bytes += mIR->getSizeInBytes(sym);
	numActiveSyms++;

#ifdef DEBUG_DUAA
	std::cout << "ManagerICFGActive: sym = " << mIR->toString(sym)
		  << ", size = " << mIR->getSizeInBytes(sym) << ", bytes = "
		  << bytes << std::endl;
#endif
    }
    retval->setActiveSizeInBytes(bytes);

#ifdef DEBUG_DUAA
    std::cout << "CSFI - numActiveSyms is " << numActiveSyms << std::endl;
    std::cout << "CSFI - total active variable size is " << bytes << " bytes" << std::endl;
#endif

    return retval;
}

  } // end of namespace Activity
} // end of namespace OA
