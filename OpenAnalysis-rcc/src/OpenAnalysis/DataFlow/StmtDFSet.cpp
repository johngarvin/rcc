//StmtDFSet.cpp

/*#include "StmtDFSet.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;

StmtDFSet::StmtDFSet() 
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_StmtDFSet:ALL", debug);
}

StmtDFSet::StmtDFSet(const StmtDFSet &other): mSet(other.mSet){} 


OA_ptr<DataFlowSet> StmtDFSet::clone()
{ 
  OA_ptr<StmtDFSet> retval;
  retval = new StmtDFSet(*this); 
  return retval; 
}
  
// param for these can't be const because will have to 
// dynamic cast to specific subclass
bool StmtDFSet::operator ==(DataFlowSet &other) const
{ 
    StmtDFSet& recastOther 
        = dynamic_cast<StmtDFSet&>(other);
    return mSet == recastOther.mSet; 
}

bool StmtDFSet::operator !=(DataFlowSet &other) const
{ 
    StmtDFSet& recastOther 
        = dynamic_cast<StmtDFSet&>(other);
    return mSet != recastOther.mSet; 
}

bool StmtDFSet::operator <(DataFlowSet &other) const
{
  
}

StmtDFSet& StmtDFSet::setUnion(DataFlowSet &other)
{ 
    StmtDFSet& recastOther 
        = dynamic_cast<StmtDFSet&>(other);
    std::set<StmtHandle> temp; 
    std::set_union(mSet.begin(), mSet.end(), 
                   recastOther.mSet.begin(), recastOther.mSet.end(),
                   std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
}

StmtDFSet& StmtDFSet::setIntersect(StmtDFSet &other)
  {
    std::set<StmtHandle> temp;
    std::set_intersection(mSet.begin(), mSet.end(),
                          other.mSet.begin(), other.mSet.end(),
                          std::inserter(temp,temp.end()));
    mSet = temp;
    return *this;
  }

StmtDFSet& StmtDFSet::setDifference(DataFlowSet &other)
{ 
    OA_ptr<LocSet> temp; temp = new LocSet;
    std::set_difference(mSetPtr->begin(), mSetPtr->end(), 
                        other.mSetPtr->begin(), other.mSetPtr->end(),
                        std::inserter(*temp,temp->end()));
    *mSetPtr = *temp;
    mBaseLocToSetMapValid = false;
    return *this;
}

void StmtDFSet::dump(std::ostream &os)
  {
      os << "StmtDFSet: mSet = ";
      // iterate over IRHandle's and print out hvals
      std::set<StmtHandle>::iterator iter;
      for (iter=mSet.begin(); iter!=mSet.end(); iter++) {
          os << (*iter).hval() << ", ";
      }
      os << std::endl;
  }


void StmtDFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir)
  {
      os << "StmtDFSet: mSet = ";
      // iterate over IRHandle's and have the IR print them out
      std::set<StmtHandle>::iterator iter;
      for (iter=mSet.begin(); iter!=mSet.end(); iter++) {
          os << ir->toString(*iter) << ", ";
      }
      os << std::endl;
  }


  void StmtDFSet::insert(StmtHandle h) { mSet.insert(h); }
  void StmtDFSet::remove(StmtHandle h) { mSet.erase(h); }

  bool StmtDFSet::handleInSet(StmtHandle h) { return (mSet.find(h)!=mSet.end()); }

  bool StmtDFSet::empty() { return mSet.empty(); }

  } // end of DataFlow namespace
} // end of OA namespace
*/
