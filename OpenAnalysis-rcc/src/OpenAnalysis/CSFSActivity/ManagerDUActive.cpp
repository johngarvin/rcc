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
ManagerDUActive::markVaried()
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
		  << ")" << std::endl;
#endif
	std::set<OA_ptr<DUG::Node> >& nodeSet = mDUG->getInDepSymNodeSet(sym);
	std::set<OA_ptr<DUG::Node> >::iterator setIter = nodeSet.begin();
	for (; setIter != nodeSet.end(); setIter++){
	    OA_ptr<DUG::NodeInterface> node = *setIter;
	    std::pair<unsigned, unsigned> pathNode(1, node->getId());
	    onPath.insert(pathNode);
	    node->markVaried(callStack, mIR, visited, onPath, node->getProc(), 
			     (unsigned)-1, OA_ptr<DUG::EdgeInterface>());
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
		  << ")" << std::endl;
#endif
	std::set<OA_ptr<DUG::Node> >& nodeSet = mDUG->getInDepSymNodeSet(sym);
	std::set<OA_ptr<DUG::Node> >::iterator setIter = nodeSet.begin();
	for (; setIter != nodeSet.end(); setIter++){
	    OA_ptr<DUG::NodeInterface> node = *setIter;
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
ManagerDUActive::performAnalysis(
    OA_ptr<DataFlow::ParamBindings>              paramBind)
{
    OA_ptr<InterActiveFortran> retval;
    retval = new InterActiveFortran;

#ifdef DEBUG_DUAA
    // Def-Use Activity Analysis
    std::cout << "==================================================" << std::endl;
    std::cout << "\tManagerDUActive::performAnalysis" << std::endl;
    std::cout << "==================================================" << std::endl;
#endif
    markVaried();
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
    std::cout << "CSFS - numActiveSyms is " << numActiveSyms << std::endl;
    std::cout << "CSFS - total active variable size is " << bytes << " bytes" << std::endl;
#endif

    return retval;
}

  } // end of namespace Activity
} // end of namespace OA
