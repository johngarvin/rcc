/*! \file
  
  \brief Results interface for loop detection analysis.

  \authors Andy Stone (aistone@gmail.com)

  Copyright (c) 2007, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LoopInterface_H
#define LoopInterface_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Utils/ListIterator.hpp>
#include "LoopAbstraction.hpp"
#include <list>
using namespace std;


namespace OA {
  namespace Loop {

/*! Defines an iterator type for iterating across loop objects in a list. */
typedef ListIterator<OA_ptr<LoopAbstraction> > LoopIterator;

/*! Interface for classes that store loop detection analysis results. */
class Interface {
  public:
    Interface() {}
    virtual ~Interface() {}

    //[Result builder]
    /*! Add a loop to the results */
    virtual void addLoop(OA_ptr<LoopAbstraction> l) = 0;


    //[Results queries]
    /*! Given a statement handle to a loop return the abstraction that's been
        built to represent it.  If no such abstraction has been built return
        a null pointer */
    virtual OA_ptr<LoopAbstraction> getLoop(StmtHandle stmt) = 0;


    /*! Get an ordered list of variables representing the index variables for
        the specified loop and its parents.  The first element of the list
        corresponds to the outermost (top-level) loop.  This is to correspond
        with the order typically used in distance and direction vectors. */
    virtual OA_ptr<list<OA_ptr<LoopIndex> > >
        getIndexVars(OA_ptr<LoopAbstraction> loop) = 0;
};

} } // namespaces

#endif

