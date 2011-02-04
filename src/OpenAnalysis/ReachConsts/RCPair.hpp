/* \file

  \brief Pairs AliasTag with a ConstValBasicInterface for ReachConsts

  \authors Michelle Stroug, Barbara Kreaseck
  
  \version $Id: RCPair.hpp, mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef RCPair_hpp
#define RCPair_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
  namespace ReachConsts {

//! =================================================================================
//! Reaching constant is represented by RCPair i.e. 
//! 2-tuple: RCPair(AliasTag, constValBasic pointer)
//!  
//!          constValBasic pointer : is not NULL if we have a
//!                                  constant value assoc. with AliasTag
//!
//! ===============================================================================

class RCPair  : public virtual Annotation{

public:

  //! === constructors ===

  RCPair(Alias::AliasTag, OA_ptr<ConstValBasicInterface>);
  RCPair(const RCPair& other);
  RCPair& operator= (const RCPair& other);


  ~RCPair();


  //! ===== Access Methods =====
  OA::Alias::AliasTag getTag() const;
  OA::OA_ptr<OA::ConstValBasicInterface> getConstPtr() const;


  //! ===== Full Comparison =====
  bool operator< (const RCPair &other) const;
  bool operator== (const RCPair &other) const;
  bool operator!= (const RCPair &other) const;


  //! =================== Output and debugging ========================

  //! ===== Annotation Output =====
  void output(IRHandlesIRInterface& pIR) const;
  void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR);


  //! ===== RCPair Output =====
  void output(IRHandlesIRInterface& pIR, Alias::Interface &alias) const;
  std::string toString(IRHandlesIRInterface& pIR);
  // void output(); 


protected:

  // BK: do we use this?
  void setConstPtr(OA::OA_ptr<OA::ConstValBasicInterface>);

private:

  //! ===== Members =====
  OA::Alias::AliasTag mTag;
  OA::OA_ptr<OA::ConstValBasicInterface> mConstPtr;

};

std::ostream &operator<<(std::ostream &os, const RCPair &rc);



  } // end of ReachConst namespace
} // end of OA namespace



#endif // RCPair_hpp
