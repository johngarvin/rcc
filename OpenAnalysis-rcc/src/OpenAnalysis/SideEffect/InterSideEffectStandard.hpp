/*! \file
  
  \brief Definition of SideEffectStandard 

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: InterSideEffectStandard.hpp,v 1.7 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterSideEffectStandard_hpp
#define InterSideEffectStandard_hpp

#include <set>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/SideEffect/SideEffectStandard.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>


namespace OA {
  namespace SideEffect {

/*! Stores results of standard interprocedural side effects analysis. */
class InterSideEffectStandard : public InterSideEffectInterface,
			        public virtual Annotation
{
  public:
    InterSideEffectStandard(OA_ptr<Alias::InterAliasInterface> alias);
    ~InterSideEffectStandard() {}

    //*****************************************************************
    // Interface Implementation
    //*****************************************************************
    OA_ptr<ProcIterator> getKnownProcIterator();
    OA_ptr<Alias::AliasTagIterator> getLMODIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getMODIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getLDEFIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getDEFIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getLUSEIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getUSEIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getLREFIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getREFIterator(CallHandle call);
    OA_ptr<Alias::AliasTagIterator> getLMODIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getMODIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getLDEFIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getDEFIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getLUSEIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getUSEIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getLREFIterator(ProcHandle p);
    OA_ptr<Alias::AliasTagIterator> getREFIterator(ProcHandle p);

    //*****************************************************************
    // Construction methods
    //*****************************************************************
    
    //! Associate the given procedure with the given intraprocedural
    //! SideEffect information
    void mapProcToSideEffect(
        ProcHandle proc, 
         OA_ptr<OA::SideEffect::SideEffectStandard> sideEffect);
    
    //! Get the SideEffect information associated with the given procedure
    OA_ptr<OA::SideEffect::SideEffectStandard> getSideEffectResults(
        ProcHandle proc);

    //! Associate the given call with the given intraprocedural
    //! SideEffect information
    void mapCallToSideEffect(
        CallHandle call, OA_ptr<OA::SideEffect::SideEffectStandard> sideEffect);
    
    
    //! Return count of tags in all sets for given procedure
    int getTagCount(ProcHandle proc);
    
    //! Insert a tag into the LMOD set for the given call
    void insertLMOD(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the MOD set for the given call
    void insertMOD(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the LDEF set for the given call
    void insertLDEF(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the DEF set for the given call
    void insertDEF(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the LUSE set for the given call
    void insertLUSE(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the USE set for the given call
    void insertUSE(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the LREF set for the given call
    void insertLREF(CallHandle call, Alias::AliasTag tag);
    
    //! Insert a tag into the REF set for the given call
    void insertREF(CallHandle call, Alias::AliasTag tag);

    //! Initialize the information for a particular call to have all
    //! empty sets
    void initCallSideEffect(CallHandle call);

   
    //*****************************************************************
    // Output
    //*****************************************************************
  public:
    //! will use OutputBuilder to generate output
    void output(IRHandlesIRInterface& ir) const { }
   
    void output(IRHandlesIRInterface& ir, Alias::Interface &alias) const;
  
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    OA_ptr<Alias::InterAliasInterface>  mAlias;

    OUTPUT 
    
    GENOUT typedef std::map<ProcHandle,OA_ptr<OA::SideEffect::SideEffectStandard> >  ProcToSideEffectMap;
    GENOUT typedef std::map<CallHandle,OA_ptr<OA::SideEffect::SideEffectStandard> >  CallToSideEffectMap;

    // mappings of procedures to various sets
    GENOUT ProcToSideEffectMap mProcToSideEffectMap;

    // mapping function calls to various sets
    GENOUT CallToSideEffectMap mCallToSideEffectMap;

    // default SideEffect results
    OA_ptr<OA::SideEffect::Interface> mDefaultSideEffect;

    //! Iterator over procedures in the mProcToSideEffectMap
    class InterSideEffectProcIter : public virtual ProcIterator,
                                    public IRHandleSetIterator<ProcHandle>
    {
      public:
        InterSideEffectProcIter(OA_ptr<std::set<ProcHandle> > pSet )
            : IRHandleSetIterator<ProcHandle>(pSet) {}
        ~InterSideEffectProcIter() { }

        ProcHandle current() const
          { return IRHandleSetIterator<ProcHandle>::current(); }
        bool isValid() const 
          { return IRHandleSetIterator<ProcHandle>::isValid(); }
        void operator++() { IRHandleSetIterator<ProcHandle>::operator++(); }
        void reset() { IRHandleSetIterator<ProcHandle>::reset(); }
    };
};


  } // end of SideEffect namespace
} // end of OA namespace

#endif

