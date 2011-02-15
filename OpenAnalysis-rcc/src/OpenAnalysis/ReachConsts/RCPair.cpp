/*! \file
  
  \brief Implementation of ReachConsts::RCPair

  \author Michelle Strout, Barbara Kreaseck
  \version $Id: ReachConstsStandard.cpp,v 1.7 2005/03/18 18:14:16 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#include "RCPair.hpp"
#include <OpenAnalysis/Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace ReachConsts {

static bool debug = false;


//! ===============================================================
//! RCPair methods
//! ===============================================================


RCPair::RCPair(Alias::AliasTag tag, 
               OA_ptr<ConstValBasicInterface> cvbP) 
{
  OA_DEBUG_CTRL_MACRO("DEBUG_RCPair:ALL", debug);
  mTag = tag;
  mConstPtr = cvbP; 
}


RCPair::RCPair(const RCPair& other) 
{
  mTag = other.mTag;
  mConstPtr = other.mConstPtr;	
}



RCPair& RCPair::operator=(const RCPair& other) {

  mTag = other.getTag();
  mConstPtr = other.getConstPtr();
  return *this;

}



RCPair::~RCPair() { }




Alias::AliasTag RCPair::getTag() const { 
  return mTag; 
}



OA_ptr<ConstValBasicInterface> RCPair::getConstPtr() const { 
  return mConstPtr; 
}



bool RCPair::operator== (const RCPair &other) const {
  bool retval=false;
  
  if (mTag==other.getTag()) {
    
    if (mConstPtr.ptrEqual(0)
        &&other.getConstPtr().ptrEqual(0))
      {
        retval = true; 
      }
    else if (!mConstPtr.ptrEqual(0)&&
             !other.getConstPtr().ptrEqual(0)) 
      {
        if (mConstPtr==other.getConstPtr()) 
          { 
            retval = true; 
          }
      }
  }

  return retval;
}




bool RCPair::operator< (const RCPair &other) const 
{ 
  bool retval = false;

  if (mTag < other.mTag) { 
    // (5,*) < (7,*)
    retval = true; 
  } else if (mTag == other.mTag) {
    // if we assume that any non-null const ptr < a NULL const ptr,
    // we only have these possibilities when the tags are equal:
    //   (5,6) <? (5,4)       false
    //   (5,6) <? (5,6)       false
    //   (5,6) <? (5,7)       true
    //   (5,6) <? (5,NULL)    true
    //   (5,NULL) <? (5,4)    false
    //   (5,NULL) <? (5,NULL) false
    if (!mConstPtr.ptrEqual(0)) {
      if (!other.getConstPtr().ptrEqual(0)) {
        if (mConstPtr<other.getConstPtr()) 
          { 
            // (5,6) < (5,7)
            retval = true; 
          }
      } else {
        // (5,6) < (5,NULL)
        retval = true;
      }
    }
  }
  
  return retval;
}



bool RCPair::operator!= (const RCPair &other) const 
{ return !(*this==other); }



 


//! =========================================================================
//! RCPair output methods
//! =========================================================================


//! Return a string that contains a character representation of a RCPair.
std::string RCPair::toString(IRHandlesIRInterface& pIR) 
{
  std::ostringstream oss;
  oss << "<";
  oss << mTag;
  oss << ",VALUE=" << (*mConstPtr).toString() << ">"; 
  return oss.str();
}




void RCPair::output(IRHandlesIRInterface& ir) const {
  /*
    // since AliasTag::output(IRHandlesIRInterface& pIR) does not
    // have this assert, seems like we can use that output without
    // sending in mAlias as we do in RCPair::output(ir,mAlias) below

       std::cout << "Please call output with alias::Interface"
                 << std::endl;
       assert(0);
  */

  sOutBuild->objStart("RCpair");
      
  sOutBuild->fieldStart("mTag");
    mTag.output(ir);
  sOutBuild->fieldEnd("mTag");
      
  sOutBuild->fieldStart("mConstPtr");
    std::ostringstream oss;
    oss << ",VALUE=" << (*mConstPtr).toString() << ">";
  sOutBuild->fieldEnd("mConstPtr");
  
  sOutBuild->outputString( oss.str());
  sOutBuild->objEnd("RCPair");


}





void RCPair::output(OA::IRHandlesIRInterface &ir,
                    Alias::Interface &alias) const
{
  sOutBuild->objStart("RCpair");
      
  sOutBuild->fieldStart("mTag");
    mTag.output(ir,alias);
  sOutBuild->fieldEnd("mTag");
      
  sOutBuild->fieldStart("mConstPtr");
    std::ostringstream oss;
    oss << ",VALUE=" << (*mConstPtr).toString() << ">";
  sOutBuild->fieldEnd("mConstPtr");
  
  sOutBuild->outputString( oss.str());
  sOutBuild->objEnd("RCPair");
}
    



void RCPair::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR) {
    os << toString(*pIR) << std::endl;
}


/*
void RCPair::output() {
  std::cout << "RCPair: " << *this;
}
*/


std::ostream &operator<<(std::ostream &os, const RCPair &rc)
{
  os << "<";
  os << rc.getTag();
  os << ",VALUE=" << (*(rc.getConstPtr())).toString() << ">"; 

  return os;
}



  } // end of ReachConst namespace
} // end of OA namespace

