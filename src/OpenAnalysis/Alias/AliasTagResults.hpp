/*! \file
  
  \brief Class that maps MREs and MemRefHandles to sets of 
         AliasTags.  Essentially, it stores the results of alias
         analysis.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasTagResults_H
#define AliasTagResults_H

#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <map>
#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace Alias {


class AliasTagResults : public virtual Interface,
                        public virtual Annotation
{
  public:
    AliasTagResults();
    ~AliasTagResults() {};

    //***********************************************************
    // 
    //***********************************************************

    AliasResultType alias(MemRefHandle ref1, MemRefHandle ref2);
       
    OA_ptr<AliasTagSet> getAliasTags( OA_ptr<MemRefExpr> mre );

    OA_ptr<AliasTagSet> getAliasTags( MemRefHandle memref );

    OA_ptr<MemRefExprIterator>
        getMemRefExprIterator(AliasTag tag) const;

    AliasTag getMaxAliasTag();

    //***********************************************************
    // Construction methods 
    //***********************************************************
   
    //! Associate an MRE with given alias tag set
    void mapMemRefToAliasTagSet( OA_ptr<MemRefExpr> mre,
                              OA_ptr<AliasTagSet> alias_tag_set);

    //! Associate a MemRefHandle with given alias tag
    void mapMemRefToAliasTagSet( MemRefHandle ref,
                              OA_ptr<AliasTagSet> alias_tag_set);

    // FIXME: 4/22/08, MMS, would liked to deprecate these
    //! Associate an MRE with given alias tag
//    void mapMemRefToAliasTag( OA_ptr<MemRefExpr> mre,
//                              AliasTag alias_tag,
//                              bool isMust );

    //! Associate a MemRefHandle with given alias tag
//    void mapMemRefToAliasTag( MemRefHandle ref,
//                              AliasTag alias_tag,
//                              bool isMust );
 
    //***********************************************************
    // Output Functionalities
    //***********************************************************
    void output(OA::IRHandlesIRInterface& ir) const;
    
    //***********************************************************
    // Member variables
    //***********************************************************
  private:

    OUTPUT

    // data members
    //GENOUT std::map<OA_ptr<MemRefExpr>, OA_ptr<std::set<AliasTag> > >
    //    mMREToTags;
    GENOUT std::map<OA_ptr<MemRefExpr>, OA_ptr<AliasTagSet> >
        mMREToTags;
    GENOUT std::map<AliasTag, OA_ptr<std::set<OA_ptr<MemRefExpr> > > >
        mTagToMREs; 

    //GENOUT std::map<MemRefHandle, OA_ptr<std::set<AliasTag> > > mMemRefToTags;
    GENOUT std::map<MemRefHandle, OA_ptr<AliasTagSet> > mMemRefToTags;
    GENOUT std::map<AliasTag, OA_ptr<std::set<MemRefHandle> > > mTagToMemRefs;

    GENOUT std::map<OA_ptr<MemRefExpr>,bool> mMREToMustFlag;
    GENOUT std::map<MemRefHandle,bool > mMemRefToMustFlag;
};


class InterAliasResults : public OA::Alias::InterAliasInterface {
  public:
    InterAliasResults(OA::OA_ptr<OA::Alias::Interface> results) 
        : mAliasResults(results)
    { }

    virtual OA::OA_ptr<OA::Alias::Interface> getAliasResults(OA::ProcHandle) {
        return mAliasResults;
    }

  private:
    OA::OA_ptr<OA::Alias::Interface> mAliasResults;
};



  } // end of Alias namespace
} // end of OA namespace

#endif

