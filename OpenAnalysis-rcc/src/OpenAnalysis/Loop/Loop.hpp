/*! \file
  
  \brief Description

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef Loop_H
#define Loop_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/SetIterator.hpp>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
#include "LoopIndex.hpp"
#include <list>
using namespace std;

namespace OA {

typedef SetIterator<StmtHandle> StmtIterator;

class Loop : public virtual Annotation {
  public:
    Loop(OA_ptr<LoopIndex> loopIndex);

    /*! returns an iterator over all statements in the loop */
    StmtIterator getStmtIterator();

    /*! Uses a return code to specify the order between two statements.
        return code:    description:
        -1              stmt1 occurs before stmt2
        0               the ordering is unknown
        1               stmt1 occurs after stmt2
    */
    int getStmtOrder(StmtHandle stm1, StmtHandle stm2);

    //- [Accessor methods] -
    OA_ptr<LoopIndex> getLoopIndex() { return mLoopIndex; }

    // - [output methods] - (derived from Annotation)
    virtual void output(IRHandlesIRInterface &ir);

  private:
  OUTPUT
    GENOUT  set<StmtHandle>   mStatements;
    GENOUT  OA_ptr<LoopIndex> mLoopIndex;
};


} // namespaces

#endif

