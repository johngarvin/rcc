/*! \file
  
  \brief Default implementation for some of the methods in the
         abstract IR interface for Alias analysis.  
  
  \authors Michelle Strout
  \version $Id: AliasIRInterfaceDefault.hpp,v 1.10.6.1 2005/11/23 05:01:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasIRInterfaceDefault_h
#define AliasIRInterfaceDefault_h

#include <iostream>
#include <list>
#include "AliasIRInterface.hpp"

namespace OA {
  namespace Alias {

/*! Enumerates no statements for default implementation of dynamically
    allocated unnamed locations.  Optimistic assumption that might be
    true for F90 but isn't for C or C++.
 */
class EmptyIRStmtIterator  : public IRStmtIterator {
public:
  EmptyIRStmtIterator() { }
  ~EmptyIRStmtIterator() { };

  StmtHandle current() const { return StmtHandle(0); }
  bool isValid()  const { return false; }
        
  void operator++() {}

  void reset() {}
};



class AliasIRInterfaceDefault : public virtual AliasIRInterface {
 public:
  AliasIRInterfaceDefault() { }
  virtual ~AliasIRInterfaceDefault() { }
 
  //! Given a subprogram return an IRStmtIterator* for all
  //! statements that involve an unnamed dynamic memory allocation
  //! in the subprogram
  //! The user must free the iterator's memory via delete.
  OA_ptr<IRStmtIterator> getUnnamedDynAllocStmtIterator(ProcHandle h)
      { OA_ptr<IRStmtIterator> retval;
        retval = new EmptyIRStmtIterator; 
        return retval;
      }

  //! Given a statement, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void dump(StmtHandle stmt, std::ostream& os) { }

  //! Given a memory reference, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void dump(MemRefHandle stmt, std::ostream& os) { }

};  

  } // end of namespace Alias
} // end of namespace OA

#endif 
