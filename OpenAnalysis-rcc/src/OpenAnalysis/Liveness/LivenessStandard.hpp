// LivenessStandard.hpp

#ifndef LivenessStandard_hpp
#define LivenessStandard_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Liveness/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>
#include <OpenAnalysis/Utils/Util.hpp>

namespace OA {
  namespace Liveness {

class LivenessStandard : public Interface, public Annotation
{
  public:
    LivenessStandard(ProcHandle p){} 
    ~LivenessStandard() {}

    virtual OA_ptr<LivenessIterator> getLivenessIterator(StmtHandle s) const;
    virtual OA_ptr<LivenessIterator> getExitLivenessIterator() const;

    void insertLive(StmtHandle s, Alias::AliasTag live)
    { 
      if(mLiveness[s].ptrEqual(0)) {
        mLiveness[s] = new std::set<Alias::AliasTag>();
      }
      mLiveness[s]->insert(live);    
    }

    void removeLive(StmtHandle s, Alias::AliasTag live)
    {
      if(mLiveness[s].ptrEqual(0)) {
        mLiveness[s] = new std::set<Alias::AliasTag>();
      }
      mLiveness[s]->erase(live);
    }

    void insertExitLive(Alias::AliasTag live)
    { 
      mExitLiveness->insert(live); 
    }

    void output(IRHandlesIRInterface& pIR) const;

    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    std::map<StmtHandle, OA_ptr<std::set<Alias::AliasTag> > > mLiveness;
    OA_ptr<std::set<Alias::AliasTag> > mExitLiveness;
}; 

class LivenessStandardIterator : public LivenessIterator
{
  public:
    LivenessStandardIterator(OA_ptr<std::set<Alias::AliasTag> > pSet) {
        mpSet = pSet;
        mIter = pSet->begin();
    }
    ~LivenessStandardIterator() {}

    void operator++() { mIter++; }
    bool isValid() const 
      { return mIter != mpSet->end(); }
    Alias::AliasTag current() const 
      { return *mIter; }
    void reset() { mIter = mpSet->begin(); }

  private:
    OA_ptr<std::set<Alias::AliasTag> > mpSet;
    std::set<Alias::AliasTag>::iterator mIter;
};

  } // end of Liveness namespace
} // end of OA namespace

#endif

