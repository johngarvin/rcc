/*! \file
  
  \brief Linearity Pairs, holds the Datastructures for LinearityAnalysis.

  \authors Luis Ramos
  \version $Id: LinearityPair.cpp

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LinearityPair.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
using namespace OA;
//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {
  namespace Linearity {


//************************************************************
//SimpleAbstractVar Definitions
//************************************************************
void SimpleAbstractVar::operator= ( AbstractVar &other ) {
     var = other.getVar();
}

bool SimpleAbstractVar::operator==(AbstractVar &other) {
     //std::cout << "Comparing AbstractVar \t(" << var << ")  and  (" << other.getVar() << ")\n";
     return (var == other.getVar());
}

bool SimpleAbstractVar::operator<(AbstractVar &other) {
     if (var < other.getVar()) {
          return true;
     }
     return false;
}


//************************************************************
//LinearityClass Definitions
//************************************************************
LinearityClass::LClassLatticeVal LinearityClass::getLClass() 
{ 
     return mVal; 
}

void LinearityClass::putLClass(LClassLatticeVal init) 
{ 
     mVal = init; 
}


LinearityClass LinearityClass::meet(LinearityClass other) {
     LinearityClass result = mVal;
     //Anything w/Nonlinear = Nonlinear
     if (other.getLClass() == LCLASS_NONLINEAR) { result.putLClass(LCLASS_NONLINEAR); }  
     // Anything w/NoDep = Anything
     else if (mVal != LCLASS_NONLINEAR && mVal != LCLASS_LINEAR) { result.putLClass(other.getLClass());
     } 
     return result;
}

bool LinearityClass::operator==( LinearityClass other ) {
        	return (mVal == other.getLClass());
}

bool LinearityClass::operator!=( LinearityClass other ) {
		return (mVal != other.getLClass());
}

//************************************************************
//VarClassPair Definitions
//************************************************************
OA_ptr<Location> VarClassPair::getVar() 
{ 
     return v; 
}

LinearityClass VarClassPair::getLClass() 
{ 
     return lclass; 
}

bool VarClassPair::operator==( VarClassPair &other )
{
     return (*v == *(other.getVar()));
}

bool VarClassPair::operator<( VarClassPair &other )
{
     if (*v < *(other.getVar())) {
          return true;
     }
     return false;
}

void VarClassPair::output(IRHandlesIRInterface& ir) const
{
/*     if (v->isaNamed() && w->isaNamed()) {
       OA_ptr<NamedLoc> nv = v.convert<NamedLoc>();
       OA_ptr<NamedLoc> nw = w.convert<NamedLoc>();
       SymHandle symv = nv->getSymHandle();
       SymHandle symw = nw->getSymHandle();
*/
//       std::cout << "<<" << char(symv.hval()) << ","
//                 << char(symw.hval()) << ">,";
                    sOutBuild->objStart("VarClassPair");
                    sOutBuild->fieldStart("Location->V");
                    v->output(ir);
                    sOutBuild->fieldEnd("LocationV");
                    
                    sOutBuild->fieldStart("LClass");  
                    ostringstream os;
                    if (lclass.getLClass() == 1) {
	            os << "Linear";
                    } else {
	            os << "Nonlinear";
	            }
                    sOutBuild->outputString(os.str());
                    sOutBuild->fieldEnd("LClass");
                    sOutBuild->objEnd("VarClassPair");
     


}



//************************************************************
//LinearityPair Definitions
//************************************************************
OA_ptr<VarClassPair> LinearityPair::getVarClassPair() { 
     OA_ptr<VarClassPair> retval;
     retval = new VarClassPair(w,lclass);
     return retval;
}
	
LinearityClass LinearityPair::getLClass() 
{ 
     return lclass; 
}

OA_ptr<Location> LinearityPair::getVar1() 
{ 
     return v; 
}

OA_ptr<Location> LinearityPair::getVar2() 
{ 
     return w; 
}

void LinearityPair::output(IRHandlesIRInterface& ir) const
{
/*     if (v->isaNamed() && w->isaNamed()) {
       OA_ptr<NamedLoc> nv = v.convert<NamedLoc>();
       OA_ptr<NamedLoc> nw = w.convert<NamedLoc>();
       SymHandle symv = nv->getSymHandle();
       SymHandle symw = nw->getSymHandle();
*/
//       std::cout << "<<" << char(symv.hval()) << ","
//                 << char(symw.hval()) << ">,";
                    sOutBuild->objStart("LinearityPair");
                    sOutBuild->fieldStart("Location->V");
                    v->output(ir);
                    sOutBuild->fieldEnd("LocationV");
                    
                    sOutBuild->fieldStart("Location->W");
                    w->output(ir);
                    sOutBuild->fieldEnd("LocationW");
                    
                    sOutBuild->fieldStart("LClass");  
                    ostringstream os;
                    if (lclass.getLClass() == 1) {
	            os << "Linear";
                    } else {
	            os << "Nonlinear";
	            }
                    sOutBuild->outputString(os.str());
                    sOutBuild->fieldEnd("LClass");
                    sOutBuild->objEnd("LinearityPair");
     


}

void LinearityPair::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    os << "LP=";
    w->dump(std::cout,ir);
    os << ",";
    if (lclass.getLClass() == 1) {
        os << "LINEAR";
    } else {
        os << "NONLINEAR";
    }

}

bool LinearityPair::operator==( LinearityPair &other )
{ 
//std::cout << "Comparing LinearityPair! \t<" << v->getVar() << "," << w->getVar() << ">";
//std::cout << "  With  <" << (other.getVar1()->getVar()) << "," << (other.getVar2()->getVar()) << ">\n";
     return (*v == *(other.getVar1()) && *w == *(other.getVar2()));  
}

bool LinearityPair::operator!=( LinearityPair &other )
{
     return !(*v == *(other.getVar1()) && *w == *(other.getVar2()) && lclass == other.getLClass());
}

bool LinearityPair::operator<( LinearityPair &other ) {
     if (*v < *(other.getVar1())) {
          return true;
     } else if ((*v == *(other.getVar1())) && (*w < *(other.getVar2()))) {
          return true;
     }
     return false;
}

  } // end of namespace Linearity
} // end of namespace OA

