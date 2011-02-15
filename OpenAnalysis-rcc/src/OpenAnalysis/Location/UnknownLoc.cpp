/*! \file
  
  \brief Definition for unknown location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "UnknownLoc.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

static bool debug = false;

void UnknownLoc::acceptVisitor(LocationVisitor& pVisitor)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_UnknownLoc:ALL", debug);
    pVisitor.visitUnknownLoc(*this);
}

OA_ptr<Location> UnknownLoc::getBaseLoc() 
{ 
    OA_ptr<Location> retval; 
    retval = new UnknownLoc();
    return retval;
}


/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.
*/
bool UnknownLoc::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    return false;
}  


bool UnknownLoc::operator==(Location& other) 
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    if (other.isaUnknown()) {
        return true;
    } else {
        return false;
    }
}

void UnknownLoc::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("UnknownLoc");
    sOutBuild->objEnd("UnknownLoc");
}


void UnknownLoc::dump(std::ostream& os)
{
    os << "UnknownLoc" << std::endl;
}


void UnknownLoc::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os);
}

std::string UnknownLoc::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
  std::ostringstream oss;
  oss << "UnknownLoc";
  return oss.str();
}

} // end namespace
