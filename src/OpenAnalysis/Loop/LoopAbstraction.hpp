/*! \file
  
  \brief Loop abstraction prototype. 

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/



#ifndef LoopAbstraction_hpp
#define LoopAbstraction_hpp

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/SetIterator.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include <OpenAnalysis/Utils/Iterator.hpp>
#include "LoopIndex.hpp"
#include <list>
using namespace std;

namespace OA {
  namespace Loop {

typedef SetIterator<StmtHandle> StmtIterator;

/*! OA abstraction class representing loops.  Loop objects contain information
    about the index variable they increment, the statements they contain, and
    the loop which is its parent (or NULL if an unnested).
    Trivial change to test doxygen.
*/
class LoopAbstraction : public virtual Annotation {
  public:
  //[Construction]
    /*! construct a new loop object, set the parent pointer to NULL for
        unnested loops. */
    LoopAbstraction(
        OA_ptr<LoopIndex> loopIndex,
        StmtHandle loopStmt,
        StmtHandle endStmt,
        OA_ptr<LoopAbstraction> parent);

  //[Accessor methods]
    /*! return the statement associated directly with this loop */
    StmtHandle getLoopStatement() { return mLoopStatement; }

    /*! return the statement marked as the last statement of the loop */
    StmtHandle getEndStatement() { return mEndStatement; }

    /*! return the index object for this loop abstraction. */
    OA_ptr<LoopIndex> getLoopIndex() { return mLoopIndex; }

    /*! return the loop which is this loop's direct parent, or NULL if
        this is an unnested loop. */
    OA_ptr<LoopAbstraction> getParent() { return mParent; }

  //[Ordering operators]
    //! an ordering for loop abstractions, needed for use within STL containers
    virtual bool operator<(LoopAbstraction &other);

    //! check if two loop abstractions
    virtual bool operator==(LoopAbstraction &other);
    
    //! check if two loop abstractions are not equal
    bool operator!=(LoopAbstraction& other) { return ! ((*this)==other); }

  //[Output methods] (derived from Annotation)
    virtual void output(IRHandlesIRInterface &ir);


  private:
  OUTPUT
    GENOUT  StmtHandle mLoopStatement;
    GENOUT  StmtHandle mEndStatement;
    GENOUT  OA_ptr<LoopIndex>  mLoopIndex;
    GENOUT  OA_ptr<LoopAbstraction> mParent;
    int mOrder;
    static int sAbstractionsBuilt;
};

} } // namespaces

#endif

