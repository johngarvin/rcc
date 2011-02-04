/*! \file
  
  \brief Contains some default implementation for the CFGIRInterface.

  \authors Michelle Strout
  \version $Id: CFGIRInterfaceDefault.hpp,v 1.5 2005/06/14 03:52:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CFGIRInterfaceDefault_h
#define CFGIRInterfaceDefault_h

#include <iostream>
#include "CFGIRInterface.hpp"

namespace OA {
  namespace CFG {

/*! 
 * Implements defaults for some of the methods required by the CFGIRInterface.
 */
class CFGIRInterfaceDefault : public virtual CFGIRInterface {
 public:
  CFGIRInterfaceDefault() { }
  virtual ~CFGIRInterfaceDefault() {} 

  //! Given a statement, return true if it issues in parallel with its
  //! successor.  This would be used, for example, when the underlying IR
  //! is a low-level/assembly-level language for a VLIW or superscalar
  //! instruction set. The default implementation (which is appropriate
  //! for most IR's) is to return false.
  virtual bool parallelWithSuccessor(StmtHandle h) { return false; } 

  //! Given an unstructured branch/jump statement, return the number
  //! of delay slots. Again, this would be used when the underlying IR
  //! is a low-level/assembly-level language for a VLIW or superscalar
  //! instruction set. The default implementation (which is appropriate
  //! for most IR's) is to return 0.
  virtual int numberOfDelaySlots(StmtHandle h) { return 0; } 

  //--------------------------------------------------------
  // Debugging
  //--------------------------------------------------------  

  //! Given a LeafHandle, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void printLeaf(LeafHandle vh, std::ostream& os) { } 

  //! Given a statement, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void dump(StmtHandle stmt, std::ostream& os) 
    { os << "Override CFGIRInterfaceDefault::dump(StmtHandle,os)"; }
  
  //! Given a memory reference, pretty-print it to the output stream os.
  //! The default implementation does nothing.
  virtual void dump(MemRefHandle stmt, std::ostream& os) { }
};



  } // end of namespace CFG
} // end of namespace OA

#endif // CFGIRInterfaceDefault_h
