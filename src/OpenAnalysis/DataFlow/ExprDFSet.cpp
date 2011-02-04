//ExprDFSet.cpp

#include "ExprDFSet.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace DataFlow {

static bool debug = false;

ExprDFSet::ExprDFSet() 
{ 
  mSetPtr = new ExprTreeSet;
  //std::cout << "ExprDFSet constructor called\n";
    OA_DEBUG_CTRL_MACRO("DEBUG_ExprDFSet:ALL", debug);
}

ExprDFSet::ExprDFSet(const ExprDFSet &other)
{
//std::cout << "ExprDFSet copy constr called\n";
   mSetPtr = new ExprTreeSet;
  *mSetPtr= *(other.mSetPtr);
//std::cout << "ExprDFSet end copy constr called\n";
  //  mSetPtr = other.mSetPtr;
}

OA_ptr<DataFlowSet> ExprDFSet::clone()
{ 
//std::cout << "ExprDFSet clone called\n";
  OA_ptr<ExprDFSet> retval;
  retval = new ExprDFSet(*this); 
  return retval; 
}
  
// param for these can't be const because will have to 
// dynamic cast to specific subclass
bool ExprDFSet::operator ==(DataFlowSet &other) const
{ 
//std::cout << "ExprDFSet == called\n";
   ExprDFSet& recastOther 
        = dynamic_cast<ExprDFSet&>(other);
    return *mSetPtr == *(recastOther.mSetPtr); 
}

bool ExprDFSet::operator !=(DataFlowSet &other) const
{ 
//std::cout << "ExprDFSet != called\n";
    ExprDFSet& recastOther 
        = dynamic_cast<ExprDFSet&>(other);
    return *mSetPtr != *(recastOther.mSetPtr); 
}

bool ExprDFSet::operator <(DataFlowSet &other) const
{
//std::cout << "ExprDFSet < called\n";
   ExprDFSet& recastOther
        = dynamic_cast<ExprDFSet&>(other);
    return *mSetPtr < *(recastOther.mSetPtr);
}

bool ExprDFSet::operator =(DataFlowSet &other) const
{
//std::cout << "ExprDFSet = called\n";
}


ExprDFSet& ExprDFSet::setUnion(DataFlowSet &other)
{ 
   std::cout << "ExprDFSet set union called\n";
    ExprDFSet& recastOther 
        = dynamic_cast<ExprDFSet&>(other);
    OA_ptr<ExprTreeSet> temp;
    std::cout << "Printing 2 df sets\n";
    std::set<OA_ptr<ExprTree> >::iterator iter;
    for (iter=mSetPtr->begin(); iter!=mSetPtr->end(); iter++) 
    {
      std::cout << "Print this Exprset "<< ", " << (*iter)<< "\n";
    }
    for (iter=recastOther.mSetPtr->begin(); iter!=recastOther.mSetPtr->end(); iter++) 
    {
      std::cout << "Print other Exprset "<< ", " << (*iter)<< "\n";
    }
    std::cout << "End exprtree print\n";
    temp = new ExprTreeSet; 
    std::set_union(mSetPtr->begin(), mSetPtr->end(), 
                   recastOther.mSetPtr->begin(), recastOther.mSetPtr->end(),
                   std::inserter(*temp,temp->end()));
for (iter=temp->begin(); iter!=temp->end(); iter++) 
    {
      std::cout << "Print result Exprset "<< ", " << (*iter)<< "\n";
    }

    *mSetPtr = *temp;
    return *this; 
}

ExprDFSet& ExprDFSet::setIntersect(DataFlowSet &other)
  {
//std::cout << "ExprDFSet intersect called\n";
    ExprDFSet& recastOther
        = dynamic_cast<ExprDFSet&>(other);
    OA_ptr<ExprTreeSet> temp;
    std::cout << "Printing 2 df sets\n";
    std::set<OA_ptr<ExprTree> >::iterator iter;
    for (iter=mSetPtr->begin(); iter!=mSetPtr->end(); iter++) 
    {
      std::cout << "Print this Exprset "<< ", " << (*iter)<< "\n";
    }
    for (iter=recastOther.mSetPtr->begin(); iter!=recastOther.mSetPtr->end(); iter++) 
    {
      std::cout << "Print other Exprset "<< ", " << (*iter)<< "\n";
    }
    std::cout << "End exprtree print\n";
    temp = new ExprTreeSet;
    std::set_intersection(mSetPtr->begin(), mSetPtr->end(),
                          recastOther.mSetPtr->begin(), recastOther.mSetPtr->end(),
                          std::inserter(*temp,temp->end()));
for (iter=temp->begin(); iter!=temp->end(); iter++) 
    {
      std::cout << "Print result Exprset "<< ", " << (*iter)<< "\n";
    }

    *mSetPtr = *temp;
    return *this;
  }

ExprDFSet& ExprDFSet::setDifference(ExprDFSet &other)
{
}

void ExprDFSet::dump(std::ostream &os)
  {
     /* os << "ExprDFSet: mSet = ";
      // iterate over IRHandle's and print out hvals
      std::set<OA_ptr<ExprTree> >::iterator iter;
      for (iter=mSetPtr->begin(); iter!=mSetPtr->end(); iter++) {
          os << (*iter).hval() << ", ";
      }
      os << std::endl;*/
  }


void ExprDFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> ir)
  {
      os << "ExprDFSet: mSet = ";
      // iterate over IRHandle's and have the IR print them out
      std::set<OA_ptr<ExprTree> >::iterator iter;
      for (iter=mSetPtr->begin(); iter!=mSetPtr->end(); iter++) {
	os << "Print Expr "<< ", "; 
	  (*iter)->output(*ir);
        os << "\n";
      }
      os << std::endl;
  }


  void ExprDFSet::insert(OA_ptr<ExprTree> h) { 

    std::cout<<"find"<< hasExprTree(h)<<"\n";  
    std::cout << "Inside Insert" << h <<"\n";
    mSetPtr->insert(h);
          std::set<OA_ptr<ExprTree> >::iterator iter;
      for (iter=mSetPtr->begin(); iter!=mSetPtr->end(); iter++) {
	std::cout << "Print ExprDFSet after insert "<< ", ";
      }

 }
  void ExprDFSet::remove(OA_ptr<ExprTree> h) { mSetPtr->erase(h); }

  bool ExprDFSet::hasExprTree(OA_ptr<ExprTree> h) { return (mSetPtr->find(h)!=mSetPtr->end()); }

  bool ExprDFSet::empty() { return mSetPtr->empty(); }

  } // end of DataFlow namespace
} // end of OA namespace

