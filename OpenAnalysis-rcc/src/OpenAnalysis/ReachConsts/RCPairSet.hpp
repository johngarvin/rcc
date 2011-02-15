/*! \file
  
  \brief Holds a set of RCPairs

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ReachConstsStandard.hpp,v 1.9 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef RCPairSet_hpp
#define RCPairSet_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>
#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

#include "RCPair.hpp"

namespace OA {
  namespace ReachConsts {


class RCPairSet;
class RCPairSetIterator;

/*!Holds a set of RCPair.  This is the underlying result type 
 * for ReachConsts::ReachConstsStandard
 */

class RCPairSet  : public virtual DataFlow::DataFlowSet
{

public:


    //! ====================================================
    //! DataFlow Methods 
    //! ====================================================


    //! ===== construction =====
    OA_ptr<DataFlow::DataFlowSet> clone() const;


    //! ===== comparision =====
    bool operator==(DataFlowSet &other) const;
    bool operator!=(DataFlowSet &other) const;


    //! ===== access =====
    void setUniversal() ;
    void clear() ;
    int  size() const ;
    bool isUniversalSet() const ;
    bool isEmpty() const ;


    //! ===== output =====
    void output(IRHandlesIRInterface& ir) const;
    void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR);




    //! ==========================================================
    //! RCPairSet Methods 
    //! ==========================================================


    //! ===== construction =====

    //! non-universal empty set
    RCPairSet();

    //! copy constructor
    RCPairSet(const RCPairSet& other);

    //! partial-universal constructors
    RCPairSet(const OA_ptr<Alias::AliasTagSet> tset);
    RCPairSet(const OA_ptr<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> > iset);

    //! assignment operator
    RCPairSet& operator= (const RCPairSet& other);


    //! ===== destruction =====
    ~RCPairSet();

    //! ===== other access for RCPairSets (non-DataFlowSet access) =====
    bool isPartialSet() const;
    int tagsetSize() const;
    OA_ptr<OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > getTagSetIterator() const;
    bool containsPartialTag(Alias::AliasTag) const;
    bool containsPairTag(Alias::AliasTag) const;

    //! ===== Modification routines by RCPair ====
    void insert(RCPair);
    void remove(RCPair);

    //! ===== Needed Set Operations =====
  
    //! intersects the argument with this set, store results in this set
    RCPairSet& intersectEqu(RCPairSet& other);
    //! unions the argument with this set, store results in this set
    RCPairSet& unionEqu(RCPairSet& other);
    //! subtracts the argument from this set, storing results in this set
    RCPairSet& minusEqu(RCPairSet& other);

    //! =============== relationship ====================
    bool operator==(const RCPairSet& other) const;
    bool operator!=(const RCPairSet& other) const;

    //! ================ Output & Debugging =========================
    void output(IRHandlesIRInterface& ir, Alias::Interface &alias) const;
    std::string toString(OA_ptr<IRHandlesIRInterface> pIR);
    std::string toString();



protected:

    //! ===== Members =====  
    OA_ptr<OA::DataFlow::DataFlowSetImpl<RCPair> > mSet;

    //! for partial universal sets
    OA_ptr<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> > mTagSet;

    //! RCPairSet owns two DataFlowSetImpl and cannot rely upon the universality
    //! of either one ... so maintaining own mUniversal member.
    bool mUniversal;


    //! ===== Friends =====
    friend class RCPairSetIterator;
};
    

 





 
//! ==============================================
//! RCPairSet Iterator
//! ==============================================

class RCPairSetIterator {

public:

  //! ===== Constructor/Destructor =====
  RCPairSetIterator (RCPairSet& RCSet);
  ~RCPairSetIterator ();


  //! ===== Access =====
  void operator ++ ();
  bool isValid();
  RCPair current();
  void reset();

  //! ===== UniversalSet Discriminators =====
  bool isUniversalSetIter();
  bool isPartialSetIter();


private:
  OA_ptr<OA::DataFlow::DataFlowSetImpl<RCPair> > mSet;
  OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > mRCIter;
  bool mUniversal;
  bool mPartial;
};



  } // end of ReachConst namespace
} // end of OA namespace




#endif // RCPairSet_hpp
