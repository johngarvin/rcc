/*! \file
  
  \brief Definition for Location Set class 

  \authors Michelle Strout, Andy Stone
  \version $Id: Location.cpp,v 1.25.6.3 2005/12/30 05:26:28 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LocSet.hpp"
#include "Location.hpp"

namespace OA {

static bool debug = false;




void dumpLocSet(LocSet& set, std::ostream& os, 
                OA_ptr<IRHandlesIRInterface> pIR)
{
    os << "LocSet = { ";
    LocSet::iterator setIter;
    for (setIter=set.begin(); setIter!=set.end(); setIter++) {
        OA_ptr<Location> loc = *setIter;
        loc->dump(os,pIR);
    }
    os << " }" << std::endl;   
}


OA_ptr<LocSet > 
intersectLocSets(LocSet& set1,
                 LocSet& set2)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_LocSet:ALL", debug);

    OA_ptr<LocSet> temp; temp = new LocSet;
    std::set_intersection(set1.begin(), set1.end(), 
                          set2.begin(), set2.end(),
                          std::inserter(*temp,temp->end()));
    return temp;
}


OA_ptr<LocSet > 
unionLocSets(LocSet& set1, LocSet& set2)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_LocSet:ALL", debug);

  OA_ptr<LocSet> temp; temp = new LocSet;

  LocSet::iterator setIter;
  int count1=0;
  for (setIter=set1.begin(); setIter!=set1.end(); setIter++) {
      OA_ptr<Location> loc = *setIter;
      count1++;
  }

  int count2=0;
  for (setIter=set2.begin(); setIter!=set2.end(); setIter++) {
      OA_ptr<Location> loc = *setIter;
      count2++;
  }
  
  std::set_union(set1.begin(), set1.end(), 
                 set2.begin(), set2.end(),
                 std::inserter(*temp,temp->end()));
  return temp;
}


bool mayOverlapLocSets(LocSet& set1,
                       LocSet& set2)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_LocSet:ALL", debug);

    if (debug) {
        std::cout << "mayOverlapLocSets: " << std::endl;
    }
    LocSet::iterator setIter1;
    LocSet::iterator setIter2;
    for (setIter1=set1.begin(); setIter1!=set1.end(); setIter1++) {
        for (setIter2=set2.begin(); setIter2!=set2.end(); setIter2++) {
            OA_ptr<Location> loc1 = *setIter1, loc2 = *setIter2;
            loc1 = *setIter1;
            loc2 = *setIter2;
            if (debug) {
                std::cout << "loc1 = ";
                loc1->dump(std::cout);
                std::cout << "loc2 = ";
                loc2->dump(std::cout);
            }
            if (loc1->mayOverlap(*loc2)) {
                return true;
            }
        }
    }

    return false;
}


bool subSetOf(LocSet& set1, LocSet& set2)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_LocSet:ALL", debug);

    bool retval = true;

    LocSet::iterator setIter1;
    LocSet::iterator setIter2;
    for (setIter1=set1.begin(); setIter1!=set1.end(); setIter1++) {
        bool found = false;
        for (setIter2=set2.begin(); setIter2!=set2.end(); setIter2++) {
            OA_ptr<Location> loc1, loc2;
            loc1 = *setIter1;
            loc2 = *setIter2;
            if (loc1 == loc2) {
                found = true;  
                break;
            }
        }
        if (!found) {
            retval = false;
            break;
        }
    }

    return retval;
}

} // end namespace
