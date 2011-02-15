/*! \file
  
  \brief Linearity Matrix which holds Dataflow Sets for the Analysis.

  \authors Luis Ramos
  \version $Id: LinearityMatrix.cpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#include "LinearityMatrixStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
using namespace OA;
//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

static bool debug = false;

//************************************************************
//LinearityMatrix Definitions
//************************************************************
// When calculating DEPS, we need to iterate over
// all the pairs where a particular variable is
// the first var in the pair.
//      ie. <<v,w>, class> in IN(b) for a particular v
OA_ptr<LinearityPairIterator> LinearityMatrix::getPairIteratorForVar( OA_ptr<Location> v )
{
     //Return the Iterator to all the LinearityPairs for Abstract *v
     mIter = mLMmap->find(v);
     OA_ptr<LinearityPairIterator> retval;
     if (mIter != mLMmap->end()) {
          retval = new LinearityPairIterator(mIter->second); 
          return retval; 
     } else {
          retval = new LinearityPairIterator();
	  return retval; 
     }
}

void LinearityMatrix::putLPSet(OA_ptr<Location> v, OA_ptr<std::set<OA_ptr<LinearityPair> > >lpSet) {
     //mLMmap->insert(make_pair(v,lpSet));
     (*mLMmap)[v] = (lpSet);
}

void LinearityMatrix::putLPair(OA_ptr<LinearityPair> lp) {
     //std::cout << "Find: ";
     //Locate the Key v
     mIter = mLMmap->find(lp->getVar1());
     if (mIter != mLMmap->end()) {
          //std::cout << "Yes\n";

          //Key v exist, delete current lp and insert new lp to current set
          mIter->second->erase(lp);
          mIter->second->insert(lp);
          //((*mLMmap)[cv])->erase(clp);
          //((*mLMmap)[cv])->insert(clp);
			
	  //std::cout << "  Inserted New Pair! <" << (lp->getVar1()->getVar()) 
	  //          << "," << (lp->getVar2()->getVar()) << ">," 
	  //          << lp->getLClass().getLClass() << "\n";
      } else {
           //std::cout << "No\n";

           //create emptly set
           OA_ptr<std::set<OA_ptr<LinearityPair> > > lpSet;
           lpSet = new std::set<OA_ptr<LinearityPair> >;
			
           //mLMmap->insert(std::map<OA_ptr<Location>,OA_ptr<std::set<OA_ptr<LinearityPair> > > >::value_type(cv,lpSet));
           //insert lp into new set, then insert v and set into map
           lpSet->insert(lp);
           mLMmap->insert(make_pair(lp->getVar1(),lpSet));
			
           //std::cout << "Created New One! <" << lp->getVar1()->getVar() << "> -> <"
	   //          << (lp->getVar1()->getVar()) << ","
	   //          << (lp->getVar2()->getVar()) << ">\n";
      } 
}

OA_ptr<DataFlow::DataFlowSet> LinearityMatrix::clone()
{ 
  OA_ptr<LinearityMatrix> retval;
  retval = new LinearityMatrix(*this); 
  return retval; 
}

// meet operation between two LinearityMatrix objects
// For now go ahead and create a new LinearityMatrix
// for the result, the result represent the meet between
// the dataflow sets.
OA_ptr<LinearityMatrix> LinearityMatrix::meet( OA_ptr<LinearityMatrix> other, IRHandlesIRInterface& pIR )
{
     //create a result matrix
     OA_ptr<LinearityMatrix> result; 
     //start by putting the Other Map into result, then you can
     //overwrite specific LinearityPairs to that set if nessesary
     if (other.ptrEqual(0)) {
       assert(0);
     }
     result = new LinearityMatrix(*other);
     //result = other;
     //Iterate through the Map
     for(mIter = mLMmap->begin();mIter != mLMmap->end(); mIter++)
     {
          //create set iterator for this map 
          OA_ptr<LinearityPairIterator> thisIter; 
          thisIter = new LinearityPairIterator(mIter->second);
          //create set iterator for other map
          OA_ptr<LinearityPairIterator> lpIter; 
          lpIter = other->getPairIteratorForVar(mIter->first);
          //iterate throught this map's set
          for ( ; thisIter->isValid(); (*thisIter)++)
	  {
               OA_ptr<LinearityPair> lpthis;
               lpthis = thisIter->current();
               //look for this LinearityPair match in the other set  
               lpIter->findIter(lpthis);
	        		
               //check for a match in LinearityPair
               if (lpIter->isValid())
               {
                    OA_ptr<LinearityPair> lp;
                    lp = lpIter->current();

                    if (debug) {
                      std::cout << "VALID MATCH:\nLinearity Pair OTHER:\n ";
                      lp->output(pIR);
                      std::cout << "Linearity Pair THIS:\n";
                      lpthis->output(pIR);
                    }
                    //put LinearityPair in a new LM result after meeting the LClasses;
                    LinearityClass lc = lpthis->getLClass().meet(lp->getLClass());
		    if (debug) {
	              std::cout << "Meet Class: ";
		      lc.output();
		    }

        	    if (lc == lpthis->getLClass()) { //std::cout << "Meet First ";lc.output();
                         result->putLPair(lpthis); }
        	    else { //std::cout << "Meet Second ";lc.output();
		         result->putLPair(lp); }
        				
               } else {
	       //there was no match, just insert LinearityPair
               result->putLPair(lpthis);	
               }//end if match
	  }//end for set
     }//end for map
     return result;
}

OA_ptr<LinearityMatrix> LinearityMatrix::meet( DataFlowSet &other, IRHandlesIRInterface& pIR )
{
     //create a result matrix
     OA_ptr<LinearityMatrix> result; 
     result = new LinearityMatrix;

     LinearityMatrix &recastOther = dynamic_cast<LinearityMatrix&>(other);
     
     //start by putting the Other Map into result, then you can
     //overwrite specific LinearityPairs to that set if nessesary
     OA_ptr<DataFlow::DataFlowSet> temp = recastOther.clone();
     result = temp.convert<LinearityMatrix>();

     //Iterate through the Map
     for(mIter = mLMmap->begin();mIter != mLMmap->end(); mIter++)
     {
          //create set iterator for this map 
          OA_ptr<LinearityPairIterator> thisIter; 
          thisIter = new LinearityPairIterator(mIter->second);
          //create set iterator for other map
          OA_ptr<LinearityPairIterator> lpIter; 
          lpIter = recastOther.getPairIteratorForVar(mIter->first);
          //iterate throught this map's set
          for ( ; thisIter->isValid(); (*thisIter)++)
	  {
               OA_ptr<LinearityPair> lpthis;
               lpthis = thisIter->current();
               //look for this LinearityPair match in the other set  
               lpIter->findIter(lpthis);
	        		
               //check for a match in LinearityPair
               if (lpIter->isValid())
               {
                    OA_ptr<LinearityPair> lp;
                    lp = lpIter->current();
                    //debug output
                    //std::cout << "\t<<" << lpthis->getVar1()->getVar() << "," \
	            //     << lpthis->getVar2()->getVar() << ">,";
	            //     if (lpthis->getLClass().getLClass() == 1) {
		    //     std::cout << "Linear" << ">  -AND-";
		    //     } else {
		    //     std::cout << "Nonlinear" << ">  -AND-";
		    //     }
	            //std::cout << "  <<" << lp->getVar1()->getVar() << "," \
	            //               << lp->getVar2()->getVar() << ">,";
		    //     if (lp->getLClass().getLClass() == 1) {
		    //     std::cout << "Linear" << ">\n";
		    //     } else {
		    //     std::cout << "Nonlinear" << ">\n";
                    //     }//end debug output
				
                    //put LinearityPair in a new LM result after meeting the LClasses;
                    LinearityClass lc = lpthis->getLClass().meet(lp->getLClass());
        	    if (lc == lpthis->getLClass()) { //std::cout << "Meet First ";lc.output();
                         result->putLPair(lpthis); }
        	    else { //std::cout << "Meet Second ";lc.output();
		         result->putLPair(lp); }
        				
               } else {
	       //there was no match, just insert LinearityPair
               result->putLPair(lpthis);	
               }//end if match
	  }//end for set
     }//end for map
     return result;
}


//putDepsSet
//Uses the AbstractVar v and uses the instance of deps
//to call deps->convert function in order to store the
//linearity pairs back to the Map for a specific v. 
void LinearityMatrix::putDepsSet(OA_ptr<Location> v, OA_ptr<LinearityDepsSet> deps)
{
     OA_ptr<std::set<OA_ptr<LinearityPair> > > lpdeps;
     OA_ptr<std::set<OA_ptr<LinearityPair> > > lpSet;

     lpdeps = deps->convertToLinearityPair(v);
     lpSet = new std::set<OA_ptr<LinearityPair> >(*(lpdeps));

     (*mLMmap)[v] = (lpSet);
}

//getDepSet
//Uses AbstractVar v to locate all the Sets of LinearityPairs
//that Map to v. It then iterates over all the pairs where
//the particular variable v is the first var in the pair and
//creates a VarClassPair to insert into the DEPS set. 
//NOTE: Use this when you Union what you had.
//      ie. {<v,linear> } U {<w,class> | <<v,w>,class> E IN(b)
OA_ptr<LinearityDepsSet> LinearityMatrix::getDepsSet(OA_ptr<Location> v)
{
     OA_ptr<LinearityDepsSet> lds; 
     lds = new LinearityDepsSet();
        	
     mIter = mLMmap->find(v);
     if (mIter != mLMmap->end()) {
          OA_ptr<LinearityPairIterator> lpIter; 
          lpIter = new LinearityPairIterator(mIter->second);
          for( ;lpIter->isValid();(*lpIter)++) {
               OA_ptr<VarClassPair> vcp;
        			
               OA_ptr<LinearityPair> lp;
	       lp = lpIter->current();
               vcp = lp->getVarClassPair();
               lds->insert(vcp);
	  }
     }
     return lds;
}


//getMap
OA_ptr<std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > > > LinearityMatrix::getMap() 
{
     return mLMmap;
}


void LinearityMatrix::output() {
     for(mIter = mLMmap->begin();mIter != mLMmap->end(); mIter++)
     {
          //std::cout << "<" << mIter->first->getVar() << ">" << " ->";
          OA_ptr<LinearityPairIterator> thisIter; 
          thisIter = new LinearityPairIterator(mIter->second);

          for ( ; thisIter->isValid(); (*thisIter)++)
          {
               OA_ptr<LinearityPair> lpthis;
               lpthis = thisIter->current();
               //std::cout << "\t<<" << lpthis->getVar1()->getVar() << "," \
     	       //     << lpthis->getVar2()->getVar() << ">,";
	       //     if (lpthis->getLClass().getLClass() == 1) {
	       //     std::cout << "Linear" << ">\n";
	       //     } else {
	       //     std::cout << "Nonlinear" << ">\n";
	       //     }
          }
          std::cout << std::endl;
     }
}
        
void LinearityMatrix::output(IRHandlesIRInterface& ir) 
{
    sOutBuild->objStart("LinearityStandard");

    sOutBuild->mapStart("mLMmap", "OA_ptr<Location>", ",OA_ptr<std::set<OA_ptr<LinearityPair> > >\n");
    for (mIter = mLMmap->begin(); mIter != mLMmap->end(); mIter++) {
//        OA_ptr<Location> s;
//        SymHandle sym;
//        if (mIter->first->isaNamed()) {
//          OA_ptr<NamedLoc> ns = mIter->first.convert<NamedLoc>();
//          s = new NamedLoc(*ns);
//          sym = ns->getSymHandle();
//         }
        
        OA_ptr<LinearityPairIterator> lpIter; 
        lpIter = new LinearityPairIterator(mIter->second);

        if ( !(lpIter->isValid()) ) continue;

        sOutBuild->mapEntryStart();
          sOutBuild->mapKeyStart();
            //std::cout << char(sym.hval());
            (mIter->first)->output(ir);
          sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();

        sOutBuild->listStart();
        for ( ; lpIter->isValid(); (*lpIter)++) {
            sOutBuild->listItemStart();
              OA_ptr<LinearityPair> lp;
              lp = lpIter->current();
              lp->output(ir);
            sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();

        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mLMmap");
    sOutBuild->objEnd("LinearityStandard");

}

void LinearityMatrix::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    for (mIter = mLMmap->begin(); mIter != mLMmap->end(); mIter++) {
        OA_ptr<Location> v = mIter->first;

        //os << "Location(" << v.hval() << ") " << ir->toString(v) 
        //   << std::endl << "\tLinearityPair: " << std::endl;
        os << "Location(";
        v->dump(std::cout,ir);
        os <<")\n";
        // iterate over LinearityPairs
        OA_ptr<LinearityPairIterator> lpIter; 
        lpIter = new LinearityPairIterator(mIter->second);
        
        if (!(lpIter->isValid()))
            os << "\tLP=NULL\n";
        for ( ; lpIter->isValid(); (*lpIter)++) {
            os << "\t";
            lpIter->current()->dump(std::cout,ir);
            os << std::endl;
        }

    }

}


LinearityMatrix& LinearityMatrix::operator=(const LinearityMatrix& other)
{
    mLMmap = other.mLMmap;
    return *this;
}

bool LinearityMatrix::operator ==(DataFlow::DataFlowSet &other) const
{
    LinearityMatrix& recastOther = dynamic_cast<LinearityMatrix&>(other);
    if (mLMmap->size() != recastOther.mLMmap->size()) {
        return false;
    }
    std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >::iterator mapIter;
    for(mapIter = mLMmap->begin();mapIter != mLMmap->end(); mapIter++)
     {
          //create set iterator for this map 
          OA_ptr<LinearityPairIterator> thisIter; 
          thisIter = new LinearityPairIterator(mapIter->second);
          //create set iterator for other map
          OA_ptr<LinearityPairIterator> otherIter; 
          otherIter = recastOther.getPairIteratorForVar(mapIter->first);
          //iterate throught this map's set and compare
          for ( ; thisIter->isValid() && otherIter->isValid(); (*thisIter)++,(*otherIter)++)
	  {
              //get this linearity pair
              OA_ptr<LinearityPair> lpthis;
              lpthis = thisIter->current();
              //get other linearity pair
              OA_ptr<LinearityPair> lpother;
              lpother = otherIter->current();

              //compare and return if not equal
              if ((*lpthis) != (*lpother))
                  return false;

          }
              
     } 

    
/*    LinearityMatrix& recastOther = dynamic_cast<LinearityMatrix&>(other);

    if (mLMmap->size() != recastOther.mLMmap->size()) {
        return false;
    }
*/
    return true;
}

bool LinearityMatrix::operator !=(DataFlow::DataFlowSet &other) const
{
    LinearityMatrix& recastOther = dynamic_cast<LinearityMatrix&>(other);
    if (mLMmap->size() != recastOther.mLMmap->size()) {
        return true;
    }
    std::map<OA_ptr<Location> ,OA_ptr<std::set<OA_ptr<LinearityPair> > > >::iterator mapIter;
    for(mapIter = mLMmap->begin();mapIter != mLMmap->end(); mapIter++)
     {
          //create set iterator for this map 
          OA_ptr<LinearityPairIterator> thisIter; 
          thisIter = new LinearityPairIterator(mapIter->second);
          //create set iterator for other map
          OA_ptr<LinearityPairIterator> otherIter; 
          otherIter = recastOther.getPairIteratorForVar(mapIter->first);
          //iterate throught this map's set and compare
          for ( ; thisIter->isValid() && otherIter->isValid(); (*thisIter)++,(*otherIter)++)
	  {
              //get this linearity pair
              OA_ptr<LinearityPair> lpthis;
              lpthis = thisIter->current();
              //get other linearity pair
              OA_ptr<LinearityPair> lpother;
              lpother = otherIter->current();

              //compare and return if not equal
              if ((*lpthis) != (*lpother))
                  return true;

          }
              
     } 

    //LinearityMatrix& recastOther = dynamic_cast<LinearityMatrix&>(other);
    //if (mLMmap->size() != recastOther.mLMmap->size()) {
    //    return true;
    //} 
    return false;
    //return (!(*this==other)); 
}

bool LinearityMatrix::operator ==(const LinearityMatrix& other) const
{
    return LinearityMatrix::operator==(const_cast<LinearityMatrix&>(other));
}

  } // end of namespace Linearity
} // end of namespace OA

