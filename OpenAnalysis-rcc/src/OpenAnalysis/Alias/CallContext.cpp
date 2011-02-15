/*! \file
  
  \brief Implementation of CallContext

  \authors Barbara Kreaseck
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/
#include "CallContext.hpp"


namespace OA {
  namespace Alias {

// initialization of static class member
int CallContext::mMaxDegree = 1;

void CallContext::setMaxDegree(int m)
{
  static int numSetMaxDegreeCalls = 0;
  assert(m>=0);
  mMaxDegree = m;
  numSetMaxDegreeCalls++;
  assert(numSetMaxDegreeCalls < 2);
}

CallContext::CallContext(std::list<CallHandle> l)
{
  int cnt;
  std::list<CallHandle>::iterator iter;
  for (iter = l.begin(), cnt = 0; 
       iter!=l.end() && cnt < mMaxDegree; 
       iter++,cnt++) {
    mList.push_back(*iter);
  }
}

CallContext::CallContext(OA_ptr<CallContext> cc)
{
  int cnt;
  if (!(cc.ptrEqual(0))) {
    std::list<CallHandle>::iterator iter;
    for (iter = cc->mList.begin(), cnt = 0; 
	 iter!=cc->mList.end() && cnt < mMaxDegree; 
	 iter++,cnt++) {
      mList.push_back(*iter);
    }
  }
}



CallContext::~CallContext()
{

}


bool CallContext::operator ==(const CallContext& other) const
{
  bool result = true;
  std::list<CallHandle>::iterator iter1;
  std::list<CallHandle>::iterator iter2;
  int cnt;

  if (mList.size() != other.mList.size()) {
    return false;
  }

  iter1 = mList.begin();
  iter2 = other.mList.begin();
  for (cnt = 0;
       iter1 != mList.end(),  iter2 != other.mList.end(), cnt < mMaxDegree;
       iter1++, iter2++, cnt++)
    {
      if (*iter1 != *iter2) {
	return false;
      }
    }

  if ((mList.size() > mMaxDegree)) {
    // all we know is that they agree for the first mMaxDegree CallHandles
    // but not if they agree beyond that ...
    //   Although, not sure how the lists got longer than mMaxDegree ...
    assert(0);
  }
  
  return true;
}

//!Expand context by given CallHandle (i.e., add it to the end of the list)
//! *this calls 'call'
//! before A->B
//! after  A->B->call
void CallContext::append(CallHandle call)
{
  if (mList.size() >= mMaxDegree) {
    // cannot append ... do we drop ?? from which end ??
    assert (0 && "CallContext::append() exceeds mMaxDegree");
  } else {
    assert(call != CallHandle(0));
    mList.push_back(call);
  }
}


//!Expand context by given CallHandle (i.e., add it to the front of the list)
//! 'call' calls *this
//! before A->B
//! after call->A->B
void CallContext::prepend(CallHandle call)
{
  if (mList.size() >= mMaxDegree) {
    // cannot append ... do we drop ?? from which end ??
    assert (0 && "CallContext::append() exceeds mMaxDegree");
  } else {
    assert(call!=CallHandle(0));
    mList.push_front(call);
  }
}


//!Return a clone
CallContext& CallContext::clone() const
{
  CallContext *result = new CallContext(mList);
  return *result;
}

//!assignment
CallContext& CallContext::operator=(const CallContext& other)
{
  int cnt;
  std::list<CallHandle>::iterator iter;
  for (iter = other.mList.begin(), cnt = 0; 
       iter!= other.mList.end() && cnt < mMaxDegree; 
       iter++,cnt++) {
    mList.push_back(*iter);
  }
  return *this;
}


void CallContext::output(OA::IRHandlesIRInterface& ir) const
{
    sOutBuild->objStart("CallContext");
    sOutBuild->field("mMaxDegree",int2string(mMaxDegree));
    sOutBuild->listStart();
    std::list<CallHandle>::iterator iter;
    for (iter = mList.begin(); iter != mList.end(); iter++) {
      sOutBuild->listItem(ir.toString(*iter));
    }
    sOutBuild->listEnd();
    sOutBuild->objEnd("CallContext");
}

void CallContext::output(
    OA::IRHandlesIRInterface& ir,
    const Interface& aliasResults) const
{
  output(ir);
}

void CallContext::dump(
    std::ostream& os,
    IRHandlesIRInterface& ir,
    Interface& aliasResults)
{
    bool first = true;

    os << "<mMaxDegree: " << int2string(mMaxDegree) << ", CallContext: \n";

    // iterate through each CallHandle
    std::list<CallHandle>::iterator iter;
    for (iter = mList.begin(); iter != mList.end(); iter++) {

        if(!first) {
            os << ", \n";
        } else {
            first = false;
        }
	os << "\t";
        os << ir.toString(*iter);
    }

    os << "\n>";
}

    /*
std::string AliasTag::toString(OA::IRHandlesIRInterface& ir) const
{
  std::ostringstream os;
  os << "tag(" << mTagID << ")";
  return os.str();
}


std::string AliasTag::toString(OA::IRHandlesIRInterface& ir,
                       const Interface& aliasResults) const
{
  std::ostringstream os;
  os << "tag<" << mTagID;

  //bool first = true;
  //
  //  // iterate through each mre
  //  OA_ptr<MemRefExprIterator> mreIter;
  //  mreIter = aliasResults.getMemRefExprIterator(mTagID);
  //  for(; mreIter->isValid(); ++(*mreIter)) {
  //      if(!first) {
  //          os << ", ";
  //      } else {
  //          first = false;
  //      }
  //      mreIter->current()->dump(os, ir);
  //  }

  os << ">";


  return os.str();

}

    */

	}
}
