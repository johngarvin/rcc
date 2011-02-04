/*! \file
  
  \brief Linearity Deps Set which holds VarClassPairs for each Statement.

  \authors Luis Ramos
  \version $Id: LinearityDepsSet.cpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#include <OpenAnalysis/Utils/OutputBuilder.hpp>

#include "LinearityDepsSet.hpp"
using namespace OA;
//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {

//************************************************************
//LinearityDepsSet Definitions
//************************************************************
//Create a New Empty LinearityDepsSet
LinearityDepsSet::LinearityDepsSet() 
{
     vcSet= new std::set<OA_ptr<VarClassPair> >; 
}

/* k -> { <v,T> | v E V } */
//Expression is a Constant, 
//  No difference in Linearity(NODEP), just create new Set
LinearityDepsSet::LinearityDepsSet(double) 
{
     vcSet= new std::set<OA_ptr<VarClassPair> >;
}

/* v -> { <v,linear> } U { < w,class> | <<v,w>,class> E IN(b) } */
//Expression is a Variable,
//  v becomes linear, then union v,linear with the rest of VarClassPairs
//  containing v in thier dependance chain.
LinearityDepsSet::LinearityDepsSet(OA_ptr<Location> v, OA_ptr<LinearityDepsSet> deps) 
{
     //Create New Set
     vcSet= new std::set<OA_ptr<VarClassPair> >;
     //Create LinearityClass with Linear
     LinearityClass lc(LinearityClass::LCLASS_LINEAR);
     //Create new VarClassPair with v and LinearityClass
     OA_ptr<VarClassPair> vcp;
     vcp = new VarClassPair(v,lc);

     //Inset VarClassPair with the Rest (v,linear Union deps)
     deps->insert(vcp);
     vcSet = deps->getSet();
}

/* e1(* /)e2 -> {<v,nonlinear> | <v,class> E DEPS(e1) OR <v,class> E DEPS(e2) */
//Expression1 * Expresion2, or Expresion1 / Expresion2,
//  Expresion v becomes nonlinear for both expressions
//  such that there is a v1 in deps(expression1) or
//  there is a v2 in deps(expression2). All the v's in
//  both expression dependance chain become nonlinear
OA_ptr<LinearityDepsSet> LinearityDepsSet::multdiv(OA_ptr<LinearityDepsSet> deps) 
{
     //V is Nonlinear if it exist in either DEPS
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc(LinearityClass::LCLASS_NONLINEAR);

     //Get the Iterator for this VarClassPair Set
     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);
     //Get the Iterator for deps VarClassPair Set
     OA_ptr<VarClassPairIterator> otherIter;
     otherIter = deps->getVarClassPairIterator();

     OA_ptr<VarClassPair> vcp;

     for ( ; thisIter->isValid(); (*thisIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      vcp = new VarClassPair(thisIter->current()->getVar(),lc);
      retval->insert(vcp);
     }
     for ( ; otherIter->isValid(); (*otherIter)++){
      //get varclasspair and create a new varclass
	  //pair using its var and linearityclass nonlinear
      vcp = new VarClassPair(otherIter->current()->getVar(),lc);
      retval->insert(vcp);
     }


     return retval;
}

/* e1(+ -)e2 -> {<v1,(class1 meet class2)> */
//Expression1 + Expression2, or Expression1 - Expression2,
//  Expression v becomes the meet of its classes for all its dependance chain
//  if Expression1 v1 and Expression2 v2 are both equal,
//  otherwise Expression v stays the same
OA_ptr<LinearityDepsSet> LinearityDepsSet::addsub(OA_ptr<LinearityDepsSet> deps) 
{
     //Meet V with other and this lclass if both V's are
     //equal where V1 is in this Deps and V2 is in other
     //Deps, otherwise lclass stays the same.
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;
     
     retval = deps;

     //Get the Iterator for this VarClassPair Set
     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);
     //Get the Iterator for deps VarClassPair Set
     OA_ptr<VarClassPairIterator> otherIter;
     otherIter = deps->getVarClassPairIterator();

     OA_ptr<VarClassPair> vcp;

     for ( ; thisIter->isValid(); (*thisIter)++){
      //Compare VarClassPairs
      OA_ptr<VarClassPair> vcthis;
      vcthis = thisIter->current();
      //look for this VarClassPair match in the other set
      otherIter->findIter(vcthis);

      //check for a match in VarClassPair
      if (otherIter->isValid()){
           OA_ptr<VarClassPair> vcother;
           vcother = otherIter->current();
	       
           //There was a Match between both Deps
           //VarClass, Do A meet between LClasses
           LinearityClass lc = vcthis->getLClass().meet(vcother->getLClass());
           vcp = new VarClassPair(vcother->getVar(),lc);
           retval->erase(vcp);
           retval->insert(vcp);
      } else {
           retval->insert(vcthis);
      }
}



     return retval;
}

/* function(v) -> {<v,nonlinear>} */
//Expression v is a variable in a function,
//  v becomes nonlinear for all its dependace chain
//  that is contain in deps(expression)
OA_ptr<LinearityDepsSet> LinearityDepsSet::func(OA_ptr<LinearityDepsSet> deps) 
{
     //V is Nonlinear if it exist in either DEPS
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc(LinearityClass::LCLASS_NONLINEAR);

//     //Get the Iterator for this VarClassPair Set
//     OA_ptr<VarClassPairIterator> thisIter; 
//     thisIter = new VarClassPairIterator(vcSet);
     //Get the Iterator for deps VarClassPair Set
     OA_ptr<VarClassPairIterator> otherIter;
     otherIter = deps->getVarClassPairIterator();

     OA_ptr<VarClassPair> vcp;

//     for ( ; thisIter->isValid(); (*thisIter)++){
//          //get varclasspair and create a new varclass
//	  //pair using its var and linearityclass nonlinear
//	  vcp = new VarClassPair(thisIter->current()->getVar(),lc);
//	  retval->insert(vcp);
//    }
      for ( ; otherIter->isValid(); (*otherIter)++){
      //get varclasspair and create a new varclass
	  //pair using its var and linearityclass nonlinear
	  vcp = new VarClassPair(otherIter->current()->getVar(),lc);
	  retval->insert(vcp);
     }


     return retval;
}

/* e1^1 -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
//Expression1 to the Power of 1,
//  Expression v becomes the meet between linear and v.class
//  if Expression1 v.class is NODEP then v.cass becomes linear
//  for all its dependance chain
OA_ptr<LinearityDepsSet> LinearityDepsSet::power1(OA_ptr<LinearityDepsSet> deps) 
{
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc1(LinearityClass::LCLASS_LINEAR);

     //Get the Iterator for deps VarClassPair Set
     OA_ptr<VarClassPairIterator> otherIter;
     otherIter = deps->getVarClassPairIterator();

     OA_ptr<VarClassPair> vcp;

     for ( ; otherIter->isValid(); (*otherIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      LinearityClass lc = lc1.meet(otherIter->current()->getLClass());
      vcp = new VarClassPair(otherIter->current()->getVar(),lc);
      retval->insert(vcp);
     }

     return retval;
}

/* e1^k -> {<v1,(nonlinear meet class)> | <v,class> E DEPS(e1) */
//Expression1 to the Power of k(constant),
//  Expression v becomes the meet between nonlinear and v.class
//  if Expression1 v.class is NODEP then v.class becomes nonlinear
//  for all its dependance chain
OA_ptr<LinearityDepsSet> LinearityDepsSet::powerk(OA_ptr<LinearityDepsSet> deps) 
{
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc1(LinearityClass::LCLASS_NONLINEAR);

     //Get the Iterator for deps VarClassPair Set
     OA_ptr<VarClassPairIterator> otherIter;
     otherIter = deps->getVarClassPairIterator();

     OA_ptr<VarClassPair> vcp;

     for ( ; otherIter->isValid(); (*otherIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      LinearityClass lc = lc1.meet(otherIter->current()->getLClass());
      vcp = new VarClassPair(otherIter->current()->getVar(),lc);
      retval->insert(vcp);
     }

     return retval;
}

/* e1 anyop k -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
//Expression1 using * / - + with a constant,
//  Expression v becomes the meet between linear and v.class
//  if Expression1 v.class is NODEP then v.cass becomes linear
//  for all its dependance chain
OA_ptr<LinearityDepsSet> LinearityDepsSet::anyopk() 
{
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc1(LinearityClass::LCLASS_LINEAR);

     //Get the Iterator for this VarClassPair Set
     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);

     OA_ptr<VarClassPair> vcp;

     for ( ; thisIter->isValid(); (*thisIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      LinearityClass lc = lc1.meet(thisIter->current()->getLClass());
      vcp = new VarClassPair(thisIter->current()->getVar(),lc);
      retval->insert(vcp);
     }

     return retval;
}

/* General LINEAR -> {<v1,(linear meet class)> | <v,class> E DEPS(e1) */
//A General Linear Single Varaible Definition,
//  Expression v becomes the meet between linear and v.class
//  if Expression1 v.class is NODEP then v.cass becomes linear
//  for all its dependance chain
OA_ptr<LinearityDepsSet> LinearityDepsSet::linear() 
{
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc1(LinearityClass::LCLASS_LINEAR);

     //Get the Iterator for this VarClassPair Set
     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);

     OA_ptr<VarClassPair> vcp;

     for ( ; thisIter->isValid(); (*thisIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      LinearityClass lc = lc1.meet(thisIter->current()->getLClass());
      vcp = new VarClassPair(thisIter->current()->getVar(),lc);
      retval->insert(vcp);
     }

     return retval;
}

/* General NONLINEAR -> {<v1,(nonlinear meet class)> | <v,class> E DEPS(e1) */
//A General NonLinear Single Varaible Definition,
//  Expression v becomes the meet between nonlinear and v.class
//  if Expression1 v.class is NODEP then v.cass becomes nonlinear
//  for all its dependance chain
OA_ptr<LinearityDepsSet> LinearityDepsSet::nonlinear() 
{
     //Create Return Value for LinearityDepsSet
     OA_ptr<LinearityDepsSet> retval;
     retval = new LinearityDepsSet;

     //Create LinearityClass with Nonlinear
     LinearityClass lc1(LinearityClass::LCLASS_NONLINEAR);

     //Get the Iterator for this VarClassPair Set
     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);

     OA_ptr<VarClassPair> vcp;

     for ( ; thisIter->isValid(); (*thisIter)++){
      //get varclasspair and create a new varclass
      //pair using its var and linearityclass nonlinear
      LinearityClass lc = lc1.meet(thisIter->current()->getLClass());
      vcp = new VarClassPair(thisIter->current()->getVar(),lc);
      retval->insert(vcp);
     }

     return retval;
}


// meet operation between two LinearityDepsSet objects
OA_ptr<LinearityDepsSet> LinearityDepsSet::meet( OA_ptr<LinearityDepsSet> other )
{
     //create a result depsset
     OA_ptr<LinearityDepsSet> result;
     result = new LinearityDepsSet();
     //start by copying the other depsset into result, then overwrite
     //other specific varclasspairs if nessesary
     if (other.ptrEqual(0)) {
       assert(0);
     }
     result = other;
     
     //create set iterator for this set
     OA_ptr<VarClassPairIterator> thisIter;
     thisIter = new VarClassPairIterator(vcSet);
     //create set iterator for other set
     OA_ptr<VarClassPairIterator> vcIter;
     vcIter = other->getVarClassPairIterator();

     //Iterate throught the set
     for( ; thisIter->isValid(); (*thisIter)++)
     {
         OA_ptr<VarClassPair> vcthis;
         vcthis = thisIter->current();
         //look for this VarClassPair match in otherset
         vcIter->findIter(vcthis);

         //check for a match
         if (vcIter->isValid())
         {
             OA_ptr<VarClassPair> vcp;
             vcp = vcIter->current();

             //put VarClassPair in result after meeting its LClasses
             LinearityClass lc = vcthis->getLClass().meet(vcp->getLClass());
             if (lc == vcthis->getLClass()) { result->insert(vcthis); }
             else { result->insert(vcp); }
         } else {
             //there was no match
             result->insert(vcthis);
         }//end if match
     }//end for set
     return result;
}

void LinearityDepsSet::insert(OA_ptr<VarClassPair> vcp) 
{
     vcSet->insert(vcp);
}

void LinearityDepsSet::erase(OA_ptr<VarClassPair> vcp) 
{
     vcSet->erase(vcp);
}

// all vars and their classes that we have info on
OA_ptr<VarClassPairIterator> LinearityDepsSet::getVarClassPairIterator( )
{
     //Return the Iterator to the list of all VarClassPair
     OA_ptr<VarClassPairIterator> retval;
     retval = new VarClassPairIterator(vcSet);
     return retval;
}

//Converts all Current VarClassPairs in this instance into 
//LinearityPairs with *v being its first argument 
OA_ptr<std::set<OA_ptr<LinearityPair> > > LinearityDepsSet::convertToLinearityPair(OA_ptr<Location> v)
{
     OA_ptr<std::set<OA_ptr<LinearityPair> > > result; 
     result = new std::set<OA_ptr<LinearityPair> >;
     OA_ptr<VarClassPairIterator> ldsIter; 
     ldsIter = new VarClassPairIterator(vcSet);
			
     for ( ;ldsIter->isValid();(*ldsIter)++) {
          OA_ptr<VarClassPair> vcPair = ldsIter->current();
				
	  OA_ptr<LinearityPair> lp;
	  lp = new LinearityPair(v,vcPair->getVar(),vcPair->getLClass());
		
	  result->insert(lp);	
     }
     return result;
}

void LinearityDepsSet::output(IRHandlesIRInterface& ir) const {
     
    sOutBuild->objStart("LinearityDepsSet");
     sOutBuild->listStart();

     OA_ptr<VarClassPairIterator> thisIter; 
     thisIter = new VarClassPairIterator(vcSet);
     for ( ; thisIter->isValid(); (*thisIter)++)
     {
         OA_ptr<VarClassPair> vcthis;
         vcthis = thisIter->current();
         sOutBuild->listItemStart();
            vcthis->output(ir);
         sOutBuild->listItemEnd();

         //std::cout << "\t<" << vcthis->getVar()->getVar() << ",";
         //     if (vcthis->getLClass().getLClass() == 1) {
         //     std::cout << "Linear" << ">\n";
         //     } else {
         //     std::cout << "Nonlinear" << ">\n"; }
     }
     sOutBuild->listEnd();
    sOutBuild->objEnd("LinearityDepsSet");
}

OA_ptr<std::set<OA_ptr<VarClassPair> > > LinearityDepsSet::getSet()
{
     return vcSet;
}

LinearityDepsSet& LinearityDepsSet::operator= (const LinearityDepsSet& other)
{
    vcSet = other.vcSet;
    return *this;
}

OA_ptr<LinearityDepsSet> LinearityDepsSet::operator=( OA_ptr<LinearityDepsSet> other )
{
    std::cout << "\n\nCALLING DEPS OPERATOR=\n-=-=-=-=-=-=-=-=-=-==-=-=-=-=-\n\n";
    (*vcSet) = *(other->getSet());     
}

  } // end of namespace Linearity
} // end of namespace OA

