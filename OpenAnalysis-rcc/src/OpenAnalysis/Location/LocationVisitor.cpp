/*! \file
  
  \brief Visitor for locations
  
  \authors Michelle Strout
  \version $Id: LocationVisitor.hpp,v 1.2.6.1 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LocationVisitor.hpp"
#include "Locations.hpp"

namespace OA {

void LocationVisitor::visitLocIdxSubSet(LocIdxSubSet& loc) {
    visitLocSubSet(loc);
}

void LocationVisitor::visitLocFieldSubSet(LocFieldSubSet& loc) {
    visitLocSubSet(loc);
}

} // end namespace
