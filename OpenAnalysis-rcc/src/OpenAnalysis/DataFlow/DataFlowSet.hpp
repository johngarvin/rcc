/*! \file
  
  \brief Abstract class for sets of dataflow facts.

  \authors Michelle Strout (April 2004)
           similar to John Mellor-Crummey's DataFlowSet.h

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DataFlowSet_h
#define DataFlowSet_h

#include <iostream>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {
  namespace DataFlow {

//! Set of values propogated from node to node during a data-flow analysis
class DataFlowSet : public Annotation {
  public:
    //************************************************************
    // Constructor / destructor / clone
    //************************************************************
    DataFlowSet() {}
    ~DataFlowSet() {}

    //! Create a copy of this set
    virtual OA_ptr<DataFlowSet> clone() const = 0;
  

    //************************************************************
    // Comparison Operators
    //************************************************************

    /*! Return true if all elements in the LHS are equal to a unique element
        in the RHS and the LHS and RHS have exactly the same number of
        elements. */
    virtual bool operator==(DataFlowSet &other) const = 0;

    //! Return true if the LHS and RHS do not equal each other
    virtual bool operator!=(DataFlowSet &other) const = 0;


    //************************************************************
    // Modifier Methods
    //************************************************************
    
    //! Set this set to the universal set
    virtual void setUniversal() = 0;

    //! Remove all elements from this set
    virtual void clear() = 0;


    //************************************************************
    // Information Methods
    //************************************************************
    
    //! Return the number of elements contained in this set
    virtual int size() const = 0;

    //! Return true if this is the universal set
    virtual bool isUniversalSet() const = 0;

    //! Return true if this set is empty
    virtual bool isEmpty() const = 0;


    //************************************************************
    // Output and Debugging Methods
    //************************************************************
    virtual void output(IRHandlesIRInterface& ir) const = 0;
    
    //! Output succinct description of set's contents
    virtual void dump(std::ostream &os, OA_ptr<IRHandlesIRInterface>) = 0;
};

  } // end of DataFlow namespace
} // end of OA namespace

#endif
