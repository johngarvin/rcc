/*! \file
  
  \brief Linearity Deps Set which holds VarClassPairs for each Statement.

  \authors Luis Ramos
  \version $Id: LinearityDepsSet.hpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//--------------------------------------------------------------------
//--------------------------------------------------------------------

#ifndef _LINEARITYDEP_H
#define _LINEARITYDEP_H

//--------------------------------------------------------------------
// STL headers
#include <iostream>
#include <map>
#include <set>
#include <list>

// Local headers
#include "LinearityPair.hpp"

//OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Location/Locations.hpp>

//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

//***********************************************
// Sets Starts
//***********************************************

class LinearityDepsSet : public virtual Annotation{
    private:
        //<v,class>
        //A list of the VarClassPair <v,class> that we have.
        OA_ptr<std::set<OA_ptr<VarClassPair> > > vcSet;
        std::set<OA_ptr<VarClassPair> >::iterator mIter;
    public:
        LinearityDepsSet();
        /* k -> { <v,T> | v E V } */
        LinearityDepsSet(double);

        /* v -> { <v,linear> } U { < w,class> | <<v,w>,class> E IN(b) */
        LinearityDepsSet(OA_ptr<Location> v, OA_ptr<LinearityDepsSet> deps);

        /* e1(* /)e2 -> {<v,nonlinear> | <v,class> E DEPS(e1) OR <v,class> E DEPS(e2) */
        OA_ptr<LinearityDepsSet> multdiv(OA_ptr<LinearityDepsSet> deps); 

        /* e1(+ -)e2 -> {<v1,(class1 meet class2)> */
        OA_ptr<LinearityDepsSet> addsub(OA_ptr<LinearityDepsSet> deps); 

        /* function(v) -> {<v,nonlinear>} */
        OA_ptr<LinearityDepsSet> func(OA_ptr<LinearityDepsSet> deps);
    	
        /* e1^1 -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
        OA_ptr<LinearityDepsSet> power1(OA_ptr<LinearityDepsSet> deps);
        
        /* e1^k -> {<v1,(nonlinear meet class)> | <v,class> E DEPS(e1) */
        OA_ptr<LinearityDepsSet> powerk(OA_ptr<LinearityDepsSet> deps);
        
        /* e1 anyop k -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
        OA_ptr<LinearityDepsSet> anyopk(); 

        /* general linear definition -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
        OA_ptr<LinearityDepsSet> linear();

        /* general nonlinear definition -> 
         * {<v1,(nonlinear meet class)> | <v,class> E DEPS(e1) */
        OA_ptr<LinearityDepsSet> nonlinear();

        // meet operation between two LinearityDepsSet objects
        OA_ptr<LinearityDepsSet> meet( OA_ptr<LinearityDepsSet> other );
        
        void insert(OA_ptr<VarClassPair> vcp);
        void erase(OA_ptr<VarClassPair> vcp);
    	
        // all vars and their classes that we have info on
        OA_ptr<VarClassPairIterator> getVarClassPairIterator();
        
        OA_ptr<std::set<OA_ptr<VarClassPair> > > getSet();

        void output(IRHandlesIRInterface& ir) const;
        
        //Converts all Current VarClassPairs in this instance into 
        //LinearityPairs with *v being its first argument 
        OA_ptr<std::set<OA_ptr<LinearityPair> > >convertToLinearityPair(OA_ptr<Location> v);

        OA_ptr<LinearityDepsSet> operator=( OA_ptr<LinearityDepsSet> other );
        LinearityDepsSet& operator=(const LinearityDepsSet& other);
};

  } // end of namespace Linearity
} // end of namespace OA

#endif

