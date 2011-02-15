/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief Declarations for the SymAliasSets which maps formal and global
         symbols to sets.  If two symbols map to the same set then they
         may alias each other.

  \authors Michelle Strout
  \version $Id: SymAliasSets.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SymAliasSets_H
#define SymAliasSets_H

#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <map>

namespace OA {
  namespace Alias {

/*!
   This class can be used to store information about the mapping of symbols
   to invisible symbols.  It can be used as a DataFlowSet.
   If a symbol is not indicated as mapping to the same invisible as another
   symbol then the default mapping is that the symbol maps to itself.
   
   This class also stores information about which calls contribute to
   the mapping.

*/
class SymAliasSets : public virtual DataFlow::DataFlowSet {
  private:
    class SymSetIterator;
    friend class SymSetIterator;

  public:
    SymAliasSets() : mNextSetId(0) {}
    SymAliasSets(const SymAliasSets& other);
    ~SymAliasSets() {}

    static const int SET_ID_NONE = -1; 

    //! Returns iterator over all formal ref params and globals that 
    //! are in the same set as the given SymHandle
    OA_ptr<SymHandleIterator> getSymIterator(SymHandle sym);

    //*****************************************************************
    // DataFlowSet interface
    //*****************************************************************
    bool operator ==(DataFlow::DataFlowSet &other) const;
    bool operator !=(DataFlow::DataFlowSet &other) const;

    OA_ptr<DataFlow::DataFlowSet> clone();

    void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface>);
    void dump(std::ostream &os);

    //*****************************************************************
    // Meet routine
    //*****************************************************************

    //! If two symbols map to the same invisible symbol in even one 
    //! of the SymAliasSets then they map to the same symbol in the result
    //! Does a union of calls that contribute to the two maps.
    OA_ptr<SymAliasSets> meet(SymAliasSets& other);
    
    //*****************************************************************
    // Construction methods 
    //*****************************************************************
    
    /*
    //! indicate the two formal params and/or globals may alias each
    //! other at a particular call therefore should map to same invisible
    void mapSymsToSameInvSym(ExprHandle call, SymHandle sym1, SymHandle sym2);
    */

    //! indicate the two formal params and/or globals may alias each
    //! other and therefore should map to same set
    void mergeSyms(SymHandle sym1, SymHandle sym2);

  private:
    class SymSet;
    std::map<int,OA_ptr<SymSet> >  mIdToSymSetMap;
    std::map<SymHandle,int> mSymToIdMap;
    int mNextSetId;
    //std::set<ExprHandle> mCallSet;

    //! insert set into datastructure and map all symbols it contains 
    //! to the set
    void insertSet(OA_ptr<SymSet> pSet);

    //! replace the set with the given set id with the given set
    void replaceSet(int oldSetId, OA_ptr<SymSet> pSet);

    //! Get unique set id for set that the given symbol maps to 
    int getAliasSetId(SymHandle sym);

  class SymSet : public DataFlow::IRHandleDataFlowSet<SymHandle> {
  public:
    SymSet() : DataFlow::IRHandleDataFlowSet<SymHandle>() {}
    SymSet(const SymSet& other) 
        : DataFlow::IRHandleDataFlowSet<SymHandle>(other) {}
    ~SymSet() {}

  };

  class SymSetIterator : public virtual SymHandleIterator,
                         public DataFlow::IRHandleIterator<SymHandle>
  {
  public:
    SymSetIterator(OA_ptr<SymSet> pSet) : DataFlow::IRHandleIterator<SymHandle>(pSet) {}
    ~SymSetIterator() {}

    void operator++() { DataFlow::IRHandleIterator<SymHandle>::operator++(); }
    void operator++(int i) { DataFlow::IRHandleIterator<SymHandle>::operator++(i); }
    bool isValid() const 
      { return DataFlow::IRHandleIterator<SymHandle>::isValid(); }
    SymHandle current() const 
      { return DataFlow::IRHandleIterator<SymHandle>::current(); }
    void reset() { DataFlow::IRHandleIterator<SymHandle>::current(); }
  };


};

  } // end of Alias namespace
} // end of OA namespace

#endif

