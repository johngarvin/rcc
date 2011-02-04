/*! \file
  
  \brief Definition of SideEffectStandard 

  \authors Michelle Strout, Andy Stone (convert to alias tags)
  \version $Id: SideEffectStandard.hpp,v 1.9 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SideEffectStandard_hpp
#define SideEffectStandard_hpp

#include <set>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/SideEffect/SideEffectInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
  namespace SideEffect {

/* 
   Maps procedures to USE, DEF, MOD, and REF sets and their local versions.
*/
class SideEffectStandard : public Interface,
                           public virtual Annotation
{
  public:
    SideEffectStandard(OA_ptr<Alias::Interface> alias);
    ~SideEffectStandard() {}


    //*****************************************************************
    // Iterators (from the interface)
    //*****************************************************************
    OA_ptr<Alias::AliasTagIterator> getLMODIterator();
    OA_ptr<Alias::AliasTagIterator> getMODIterator();
    OA_ptr<Alias::AliasTagIterator> getLDEFIterator();
    OA_ptr<Alias::AliasTagIterator> getDEFIterator();
    OA_ptr<Alias::AliasTagIterator> getLUSEIterator();
    OA_ptr<Alias::AliasTagIterator> getUSEIterator();
    OA_ptr<Alias::AliasTagIterator> getLREFIterator();
    OA_ptr<Alias::AliasTagIterator> getREFIterator();

    OA_ptr<MemRefExprIterator> getLMODMREIterator();
    OA_ptr<MemRefExprIterator> getMODMREIterator();
    OA_ptr<MemRefExprIterator> getLDEFMREIterator();
    OA_ptr<MemRefExprIterator> getDEFMREIterator();
    OA_ptr<MemRefExprIterator> getLUSEMREIterator();
    OA_ptr<MemRefExprIterator> getUSEMREIterator();
    OA_ptr<MemRefExprIterator> getLREFMREIterator();
    OA_ptr<MemRefExprIterator> getREFMREIterator();

    //*****************************************************************
    // Other informational methods
    //*****************************************************************
    //! Return true if the given tag is in the LMOD set 
    bool inLMOD(Alias::AliasTag tag);

    //! Return true if the given tag is in the MOD set 
    bool inMOD(Alias::AliasTag tag);

    //! Return true if the given tag is in the LDEF set 
    bool inLDEF(Alias::AliasTag tag);

    //! Return true if the given tag is in the DEF set 
    bool inDEF(Alias::AliasTag tag);

    //! Return true if the given tag is in the LUSE set
    bool inLUSE(Alias::AliasTag tag);

    //! Return true if the given tag is in the USE set 
    bool inUSE(Alias::AliasTag tag);

    //! Return true if the given tag is in the LREF set
    bool inLREF(Alias::AliasTag tag);

    //! Return true if the given tag is in the REF set
    bool inREF(Alias::AliasTag tag);

    //! Return the number of elements in the LMOD set
    int countLMOD();

    //! Return the number of elements in the MOD set
    int countMOD();

    //! Return the number of elements in the LDEF set
    int countLDEF();

    //! Return the number of elements in the DEF set
    int countDEF();

    //! Return the number of elements in the LUSE set
    int countLUSE();

    //! Return the number of elements in the USE set
    int countUSE();

    //! Return the number of elements in the LREF set
    int countLREF();

    //! Return the number of elements in the REF set
    int countREF();

    //*****************************************************************
    // Construction methods
    //*****************************************************************
    //! Insert a tag into the LMOD set
    void insertLMOD(Alias::AliasTag tag);
    
    //! Insert a tag into the MOD set
    void insertMOD(Alias::AliasTag tag);
    
    //! Insert a tag into the LDEF set
    void insertLDEF(Alias::AliasTag tag);
    
    //! Insert a tag into the DEF set
    void insertDEF(Alias::AliasTag tag);
    
    //! Insert a tag into the LUSE set
    void insertLUSE(Alias::AliasTag tag);
    
    //! Insert a tag into the USE set
    void insertUSE(Alias::AliasTag tag);
    
    //! Insert a tag into the LREF set
    void insertLREF(Alias::AliasTag tag);
    
    //! Insert a tag into the REF set
    void insertREF(Alias::AliasTag tag);

    //*************************************************************
    
    //! Make the LMOD set empty
    void emptyLMOD();
    
    //! Make the MOD set empty
    void emptyMOD();
    
    //! Make the LDEF set empty
    void emptyLDEF();
    
    //! Make the DEF set empty
    void emptyDEF();
    
    //! Make the LUSE set empty
    void emptyLUSE();
    
    //! Make the USE set empty
    void emptyUSE();
    
    //! Make the LREF set empty
    void emptyLREF();
    
    //! Make the REF set empty
    void emptyREF();
   

    //*****************************************************************
    // Output
    //*****************************************************************

    //! will use OutputBuilder to generate output
    void output(IRHandlesIRInterface& ir) const {}

    void output(IRHandlesIRInterface& ir, Alias::Interface &alias) const;
    
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    OA_ptr<Alias::Interface> mAlias;

    OUTPUT

    // various sets
    GENOUT OA_ptr<Alias::AliasTagSet> mLMODSet;
    GENOUT OA_ptr<Alias::AliasTagSet> mMODSet;
    GENOUT OA_ptr<Alias::AliasTagSet> mLDEFSet;
    GENOUT OA_ptr<Alias::AliasTagSet> mDEFSet;
    GENOUT OA_ptr<Alias::AliasTagSet> mLUSESet;
    GENOUT OA_ptr<Alias::AliasTagSet> mUSESet;
    GENOUT OA_ptr<Alias::AliasTagSet> mLREFSet;
    GENOUT OA_ptr<Alias::AliasTagSet> mREFSet;
};

  } // end of SideEffect namespace
} // end of OA namespace

#endif
