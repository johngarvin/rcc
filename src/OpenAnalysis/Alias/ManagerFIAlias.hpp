/*! \file
  
  \brief Declarations of the AnnotationManager that implements FIAlias.

  \authors Michelle Strout, Brian White

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerFIAlias_H
#define ManagerFIAlias_H

//--------------------------------------------------------------------
#include <cassert>
#include <map>
#include <set>

// OpenAnalysis headers
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/Utils/UnionFindUniverse.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
  namespace Alias {

class AnalyzedProcIterator : public IRProcIterator { 
public: 
  AnalyzedProcIterator(std::set<ProcHandle> analyzedProcs) :
    mAnalyzedProcs(analyzedProcs), mIt(analyzedProcs.begin())
  { } 
  ~AnalyzedProcIterator() { }; 
 
  virtual ProcHandle current() const; // Returns the current item. 
  virtual bool isValid() const;        // False when all items are exhausted. 
         
  virtual void operator++();     // prefix 
  void operator++(int) { ++*this; }  // postfix 

  virtual void reset(); 
private:
  std::set<ProcHandle>           mAnalyzedProcs;
  std::set<ProcHandle>::iterator mIt;

}; 

class RecordMREsVisitor;

// FIAlias may analyze all procedures defined in the program or  
// only those (iteratively) determined to be reachable.  
// The latter is our modification to Ryder's algorithm.
enum FIAliasImplement { ALL_PROCS = 0, REACHABLE_PROCS };

/*! 
    Creates a union-find universe holding sets of aliased MREs.
    Uses the FIAlias algorithm described in the Ryder2001 journal paper.
    Note that ManagerFIAlias no longer satisfies the Alias::Interface.hpp
    interface.  Returning a concrete representation of alias
    relations (via performAnalysis) is now left to subclasses 
    of ManagerFIAlias, such as ManagerFIAliasEquivSets and 
    ManagerFIAliasAliasMap.
*/
class ManagerFIAlias 
{ 
public:
  ManagerFIAlias(OA_ptr<AliasIRInterface> _ir);
  ~ManagerFIAlias () {}

  OA_ptr<UnionFindUniverse>
  performFIAlias( OA_ptr<IRProcIterator> procIter,
                  FIAliasImplement implement = ALL_PROCS );

  OA_ptr<IRProcIterator> getAnalyzedProcIter();

  // MMS, 1/17/08, old, should be removed
  //void mergeSubSetRefs(OA_ptr<UnionFindUniverse> ufset);

protected:
  OA_ptr<AliasIRInterface> mIR;

  //===== helper datastructures and routines
  // mapping of MemRefExpr to unique ids
  std::map<OA_ptr<MemRefExpr>,int> mMREToID;
  // use a counter to assign unique ids to each MemRefExpr
  int mCount;

  // this will be the default value for this
  // for each partition
  //    map[part][memref] = 0; 
  std::map<int,std::map<OA_ptr<MemRefExpr>,int> > mMap;

  // mapping of MemRefExpr to set of MemRefHandles that can
  // be expressed by said MemRefExpr
  std::map<OA_ptr<MemRefExpr>,std::set<MemRefHandle> > mMREToMemRefHandles;

  // mapping of MemRefExpr to procedures it shows up in
  std::map<OA_ptr<MemRefExpr>,std::set<ProcHandle> > mMREToProcs;

  // mapping of MemRefHandles to procedures, each MemRefHandle is only in
  // one procedure
  std::map<MemRefHandle,ProcHandle> mMemRefHandleToProc;

  //! set of formal symbols associated with each procedure
  std::map<ProcHandle,std::set<SymHandle> > mProcToFormalSet;

  //! maps mre to the given proc and memref, also assigns mre a unique ID
  void recordMRE( OA_ptr<MemRefExpr> mre, ProcHandle proc, MemRefHandle memref);
  void recordMRE( OA_ptr<MemRefExpr> mre, ProcHandle proc );

  OA_ptr<MemRefExpr> createDeref( OA_ptr<MemRefExpr> mre );

  std::set<OA_ptr<MemRefExpr> > 
    allMemRefExprsInSameSet( OA_ptr<MemRefExpr> pMRE, 
                             OA_ptr<UnionFindUniverse> ufset);

  //! records all memory reference expressions in a given procedure
  void initMemRefExprs( ProcHandle proc );

  //! records all memory reference expressions in all procedures
  void initMemRefExprs( OA_ptr<IRProcIterator> procIter );

  //! increment versions of FIAlias use a worklist.  this does not, so this is a noop.
  void addProcToWorkList(ProcHandle proc);

  //! perform Ryder phase 1 on stmt.
  void doPhase1Iteration(StmtHandle stmt, ProcHandle currProc, OA_ptr<UnionFindUniverse> ufset);

  //! perform Ryder phase 2
  bool doPhase2Iteration(OA_ptr<UnionFindUniverse> ufset, std::map<int,std::map<OA_ptr<MemRefExpr>,int> > & map);

  //! perform Ryder phase 3 on the callsite call invoked from caller currProc.
  bool doPhase3Iteration(CallHandle call, ProcHandle currProc,OA_ptr<UnionFindUniverse> ufset, std::map<int,std::map<OA_ptr<MemRefExpr>,int> > & map);

  //! A set of reachable procs that have been analyzed.
  std::set<ProcHandle> mAnalyzedProcs;

  //! Perform FIAlias using Ryder's original algorithm that analyzes all procs.
  OA_ptr<UnionFindUniverse>
  performFIAliasAllProcs( OA_ptr<IRProcIterator> procIter );

  //! Perform FIAlias using a modification that only visits reachable procs.
  OA_ptr<UnionFindUniverse>
  performFIAliasReachableProcs( OA_ptr<IRProcIterator> procIter );

  friend class RecordMREsVisitor;
  friend class AnalyzedProcIterator;

private: // helper functions
  void outputMREsInSet(int setID, 
        OA_ptr<UnionFindUniverse> ufset, 
        std::map<int,std::map<OA_ptr<MemRefExpr>,int> > & map  );

  //! merger routine as defined in Ryder paper
  void merge(int part1, int part2, OA_ptr<UnionFindUniverse> ufset, 
           std::map<int,std::map<OA_ptr<MemRefExpr>,int> > & map  );

  //! The list of procedures to be analyzed.
  std::set<ProcHandle> mWorklist;

  //! The flavor of FIAlias--examine all procs or only reachable procs.
  FIAliasImplement mImplement;
};


  } // end of Alias namespace
} // end of OA namespace

#endif
