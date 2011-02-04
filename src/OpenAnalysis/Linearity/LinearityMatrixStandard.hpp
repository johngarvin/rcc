/*! \file
  
  \brief Linearity Matrix which holds Dataflow Sets for the Linearity Analysis.

  \authors Luis Ramos
  \version $Id: LinearityMatrix.hpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef _LINEARITYMATRIX_H
#define _LINEARITYMATRIX_H

//--------------------------------------------------------------------
// STL headers
#include <iostream>
#include <map>
#include <set>
#include <list>

// Local headers
#include "LinearityPair.hpp"
#include "LinearityDepsSet.hpp"

//OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/Linearity/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

// Keep in mind that the default should be LCLASS_NODEP.
// You probably don't want to store that for all var pairs.
// Just store info for the ones that aren't that.
class LinearityMatrix : public virtual DataFlow::DataFlowSet,
                        public virtual Linearity::Interface,
                        public virtual Annotation {
    public:
        LinearityMatrix() {
    	   mLMmap = new std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >;
        }

        // num is the number of variables to maintain linearity
        // relationships amongst
        LinearityMatrix(int num) {
    	   mLMmap = new std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >;
        }

  //LinearityMatrix(const LinearityMatrix &other) : mLMmap(other.mLMmap) {
	  // go create an empty mLMmap, and populate it with the cloned contents
	  // of other.mLMmap.  (make sure that the clones of the contents are
	  // really making clones)
        LinearityMatrix(const LinearityMatrix &other) {
	  mLMmap = new std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >;
	  // iterate through other.mLMmap and 
	  std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >::iterator mapIter;
	  mapIter = other.mLMmap->begin();
	  for (; mapIter!=other.mLMmap->end(); mapIter++) {
	    OA_ptr<Location> loc = mapIter->first;
	    OA_ptr<std::set<OA_ptr<LinearityPair> > > lpSet = mapIter->second;
	    OA_ptr<std::set<OA_ptr<LinearityPair> > > cplpSet;
	    cplpSet = new std::set<OA_ptr<LinearityPair> >;
	    std::set<OA_ptr<LinearityPair> >::iterator setIter;
	    setIter = lpSet->begin();
	    for (; setIter!=lpSet->end(); setIter++) {
	      OA_ptr<LinearityPair> lp = *setIter;
	      OA_ptr<LinearityPair> cplp;
	      cplp = new LinearityPair(lp->getVar1(),lp->getVar2(),lp->getLClass());
	      cplpSet->insert(cplp);
	    }
	    (*mLMmap)[loc]=cplpSet;
	  }
	    

        }

        // When calculating DEPS, we need to iterate over
        // all the pairs where a particular variable is
        // the first var in the pair.
        //      ie. <<v,w>, class> in IN(b) for a particular v
        OA_ptr<LinearityPairIterator> getPairIteratorForVar( OA_ptr<Location> v );

        void putLPSet(OA_ptr<Location> v, OA_ptr<std::set<OA_ptr<LinearityPair> > >lpSet); 
		
        void putLPair(OA_ptr<LinearityPair> lp);

        OA_ptr<DataFlow::DataFlowSet> clone();
            
        // meet operation between two LinearityMatrix objects
        // For now go ahead and create a new LinearityMatrix
        // for the result, the result represent the meet between
        // the dataflow sets.
        OA_ptr<LinearityMatrix> meet( OA_ptr<LinearityMatrix> other, IRHandlesIRInterface& pIR );
        OA_ptr<LinearityMatrix> meet( DataFlowSet &other, IRHandlesIRInterface& pIR );
      
        //putDepsSet
        //Uses the AbstractVar v and uses the instance of deps
        //to call deps->convert function in order to store the
        //linearity pairs back to the Map for a specific v. 
        void putDepsSet(OA_ptr<Location> v, OA_ptr<LinearityDepsSet> deps);
        
        //getDepSet
        //Uses AbstractVar v to locate all the Sets of LinearityPairs
        //that Map to v. It then iterates over all the pairs where
        //the particular variable v is the first var in the pair and
        //creates a VarClassPair to insert into the DEPS set. 
        //NOTE: Use this when you Union what you had.
        //      ie. {<v,linear> } U {<w,class> | <<v,w>,class> E IN(b)
        OA_ptr<LinearityDepsSet> getDepsSet(OA_ptr<Location> v);
        
        //getMap
        OA_ptr<std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > > > getMap(); 
        
        void output() const;
        
        void output(IRHandlesIRInterface& pIR) const;

        void dump(std::ostream &os) {}

        void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

        //OA_ptr<LinearityMatrix> operator=( OA_ptr<LinearityMatrix> other );
        LinearityMatrix& operator=(const LinearityMatrix& other);

        bool operator ==(DataFlow::DataFlowSet &other) const;

        bool operator !=(DataFlow::DataFlowSet &other) const;

        bool operator ==(const LinearityMatrix& other) const;
        
    private:
    //Map an AbstractVar <v> with a set of VarClassPair <w,class>
    OA_ptr<std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > > > mLMmap;
    std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >::iterator mIter;

};     

  } // end of namespace Linearity
} // end of namespace OA


#endif

