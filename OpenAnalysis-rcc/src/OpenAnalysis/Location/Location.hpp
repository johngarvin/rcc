/*! \file
  
  \brief Declarations for the Location abstraction.

  \authors Michelle Strout, Andy Stone
  \version $Id: Location.hpp,v 1.26.6.1 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef Location_H
#define Location_H

#include <iostream>
#include <cassert>
#include <list>
#include <set>
#include <algorithm>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>

using namespace std;
namespace OA {

class Location;

// forward-decl to avoid circular reference in header files
class LocationVisitor;



// Location set abstraction
typedef std::set<OA_ptr<Location> > LocSet;

void dumpLocSet(std::set<OA_ptr<Location> >& set, std::ostream& os, 
                OA_ptr<IRHandlesIRInterface> pIR, bool succinct=false);

/*!  Function that intersects sets of Location */
OA_ptr<std::set<OA_ptr<Location> > > 
intersectLocSets(std::set<OA_ptr<Location> >& set1,
                 std::set<OA_ptr<Location> >& set2);
                 
/*!  Function the unions sets of Location */
OA_ptr<std::set<OA_ptr<Location> > > 
    unionLocSets(
        std::set<OA_ptr<Location> >& set1,
        std::set<OA_ptr<Location> >& set2);

/*! Returns true if any location in set1 mayOverlap with any location
    in set2.  Otherwise returns false
*/
bool mayOverlapLocSets(std::set<OA_ptr<Location> >& set1,
                       std::set<OA_ptr<Location> >& set2);

/*!  Function that determines if set1 of Locations* is a subset of set2 */
bool subSetOf(std::set<OA_ptr<Location> >& set1, 
              std::set<OA_ptr<Location> >& set2);


/*! abstract base class that has default implementations for the
    methods that all Locations must provide
*/
class Location : public Annotation {
  public:
    Location();
    Location(Location &loc) {}
    virtual ~Location() {}

    virtual void acceptVisitor(LocationVisitor& pVisitor) = 0;

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    virtual bool isaNamed() { return false; }
    virtual bool isaUnnamed() { return false; }
    virtual bool isaUnknown() { return false; }
    virtual bool isaInvisible() { return false; }
    virtual bool isaSubSet() { return false; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    
    //! Return base location for this location if this location
    //! contains another location.  If not just return this location.
    virtual OA_ptr<Location> getBaseLoc() = 0;
    
    //! true if the location block represents a local variable that is 
    //! only visible within the current procedure or an unnamed variable 
    //! that is allocated in the current procedure. If the current procedure
    //! is nested and has access to local variables of a parent procedure, 
    //! those variables are not considered local for the current procedure.
    virtual bool isLocal() { return false; }

    //! be because there could be multiple allocations at the same 
    //! statement and they maybe lumped together depending on how much
    //! context is maintained. A LocBlock for an unnamed allocated memory 
    //! block could be Unique if there is a different LocBlock for each 
    //! possible context of the alloc.
    //! A local variable is always unique, allocated memory blocks may
    virtual bool isUnique() { return false; }
    
    //! does the Location abstraction accurately represent a 
    //! specific memory location, for example, if a range in an
    //! array then false, if one index of an array then true
    
    /* PLM 1/23/07 deprecated hasFullAccuracy
    virtual bool hasFullAccuracy() { return true; }
    */

    //*****************************************************************
    // Relationship methods 
    //*****************************************************************

    //! an ordering for locations, needed for use within STL containers
    virtual bool operator<(Location & other) = 0;

    //! indicate whether this location is semantically equivalent
    virtual bool operator==(Location & other) = 0;

    //! indicate whether this location is semantically equivalent
    virtual bool operator!=(Location & other) { return ! operator==(other); }

    //! Indicate whether this location may overlap with the given
    //! location either fully or partially.
    virtual bool mayOverlap(Location & other) { return true; }

    //! Indicate whether this location must overlap fully with the given
    //! location.
    virtual bool mustOverlap(Location & other) { return false; }

    //! Indicate whether this location is a subset of the given location
    virtual bool subSetOf(Location & other) { return true; }

    //*****************************************************************
    // Debugging
    //*****************************************************************
    virtual void dump(ostream& os, OA_ptr<IRHandlesIRInterface> pIR) = 0;
    virtual void dump(ostream& os) = 0;

    virtual string toString(OA_ptr<IRHandlesIRInterface> pIR) = 0;

    virtual int getOrder() { return sOrder; }

  private:
    static const int sOrder = -100;
};


} // end of OA namespace
#endif
