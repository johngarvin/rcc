/*! \file
  
  \brief Abstract interface that the CFG AnnotationManagers require.

  \authors Michelle Strout
  \version $Id: CFGIRInterface.hpp,v 1.12.10.1 2005/12/01 04:48:29 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  A CFG AnnotationManager generates a CFG representation of the program.
  The analysis engine within the CFG AnnotationManager requires that
  the CFGIRInterface be implemented so that queries can be made to
  the Source IR relevant to constructing control flow graphs.  Some of
  the methods in this interface return an IRRegionStmtIterator.
*/

#ifndef CFGIRInterface_h
#define CFGIRInterface_h

//-----------------------------------------------------------------------------
// This file contains the abstract base classes for the IR interface.
//
// See the top level README for a description of the IRInterface and
// how to use it.
//-----------------------------------------------------------------------------

#include <iostream>
#include "IRHandles.hpp"
#include <OpenAnalysis/Utils/OA_ptr.hpp>

namespace OA {
  namespace CFG {

//! Statements are classified into one of the following types for CFG:
enum IRStmtType {
  SIMPLE,                       // Anything not covered below.
  COMPOUND,                     // A block of statements. 
  LOOP,                         // Any type of top-tested, structured loop.
  END_TESTED_LOOP,              // Any type of end-tested, structured loop.
  STRUCT_TWOWAY_CONDITIONAL,    // Structured if-then-else.
  STRUCT_MULTIWAY_CONDITIONAL,  // Structured switch statement.
  USTRUCT_TWOWAY_CONDITIONAL_T, // Unstructured branch (on true).
  USTRUCT_TWOWAY_CONDITIONAL_F, // Unstructured branch (on false).
  USTRUCT_MULTIWAY_CONDITIONAL, // Unstructured multiway branch
                                //  (e.g., computed goto in Fortran or 
                                //  jump tables in low-level/assembly
                                //  languages).
  RETURN,                       // Return statement.
  BREAK,                        // Break statement.
  LOOP_CONTINUE,                // Loop continuation statement.
  ALTERNATE_PROC_ENTRY,         // Alternate entry point (e.g., Fortran)
  UNCONDITIONAL_JUMP,           // GOTO in HLL, or unconditional direct
                                //   jump in low-level/assembly languages.
  UNCONDITIONAL_JUMP_I,         // Assigned GOTO in HLL, or unconditional
                                //   indirect jump in low-level/assembly
                                //   languages.
  NONE
};


//class EdgeInfo;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

/*! 
 * The CFGIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class CFGIRInterface : public virtual IRHandlesIRInterface {
 public:
  CFGIRInterface() { }
  //virtual ~CFGIRInterface() = 0 ;
  virtual ~CFGIRInterface() {} // changed to get rid of in-charge with whirl, 4/12

  //! Given a ProcHandle, return an IRRegionStmtIterator* for the
  //! procedure. 
  virtual OA_ptr<IRRegionStmtIterator> procBody(ProcHandle h) = 0;
  
  //--------------------------------------------------------
  // Statements: General
  //--------------------------------------------------------

  //! Are return statements allowed
  virtual bool returnStatementsAllowed() = 0;

  //! Given a statement, return its CFG::IRStmtType
  virtual IRStmtType getCFGStmtType(StmtHandle h) = 0; 

  //! Given a statement, return a label (or StmtHandle(0) if
  //! there is no label associated with the statement).
  virtual StmtLabel getLabel(StmtHandle h) = 0; 

  //! Given a compound statement, return an IRRegionStmtIterator for the
  //! statements.  
  //! A compound is a list of statements.
  virtual OA_ptr<IRRegionStmtIterator> getFirstInCompound(StmtHandle h) = 0;  
  
  //--------------------------------------------------------
  // Loops
  //--------------------------------------------------------

  //! Given a loop statement, return an IRRegionStmtIterator for the
  //! loop body.  
  virtual OA_ptr<IRRegionStmtIterator> loopBody(StmtHandle h) = 0; 

  //! Given a loop statement, return the loop header statement.  This 
  //! would be the initialization statement in a C 'for' loop, for example.
  virtual StmtHandle loopHeader(StmtHandle h) = 0; 

  //! Given a loop statement, return the increment statement. 
  virtual StmtHandle getLoopIncrement(StmtHandle h) = 0; 

  //! Given a loop statement, return:
  //! 
  //! True: If the number of loop iterations is defined
  //! at loop entry (i.e. Fortran semantics).  This causes the CFG builder 
  //! to add the loop statement representative to the header node so that
  //! definitions from inside the loop don't reach the condition and increment
  //! specifications in the loop statement.
  //!
  //! False: If the number of iterations is not defined at
  //! entry (i.e. C semantics), we add the loop statement to a node that
  //! is inside the loop in the CFG so definitions inside the loop will 
  //! reach uses in the conditional test. For C style semantics, the 
  //! increment itself may be a separate statement. if so, it will appear
  //! explicitly at the bottom of the loop. 
  virtual bool loopIterationsDefinedAtEntry(StmtHandle h) = 0; 

  // deprecated
  //OA::ExprHandle getLoopCondition(OA::StmtHandle h); 


  //--------------------------------------------------------
  // Invariant: a two-way conditional or a multi-way conditional MUST provide
  // provide either a target, or a target label
  //--------------------------------------------------------

  //--------------------------------------------------------
  // Structured two-way conditionals
  //
  // Note: An important pre-condition for structured conditionals is
  // that chains of else-ifs must be represented as nested elses.  For
  // example, this Matlab statement:
  //   if (c1)
  //     s1;
  //   elseif (c2)
  //     s2;
  //   else
  //     s3;
  //   end;
  //
  // would need be represented by the underlying IR as:
  //   if (c1)
  //     s1;
  //   else
  //     if (c2)
  //       s2;
  //     else
  //       s3;
  //     end;
  //   end; 
  //--------------------------------------------------------

  //! Given a structured two-way conditional statement, return an
  //! IRRegionStmtIterator for the "true" part (i.e., the statements under
  //! the "if" clause). 
  virtual OA_ptr<IRRegionStmtIterator> trueBody(StmtHandle h) = 0; 

  //! Given a structured two-way conditional statement, return an
  //! IRRegionStmtIterator for the "else" part (i.e., the statements under
  //! the "else" clause). 
  virtual OA_ptr<IRRegionStmtIterator> elseBody(StmtHandle h) = 0; 
  
  //--------------------------------------------------------
  // Structured multiway conditionals
  //--------------------------------------------------------

  //! Given a structured multi-way branch, return the number of cases.
  //! The count does not include the default/catchall case.
  virtual int numMultiCases(StmtHandle h) = 0; 

  //! Given a structured multi-way branch, return an IRRegionStmtIterator for
  //! the body corresponding to target 'bodyIndex'. The n targets are 
  //! indexed [0..n-1].  
  virtual OA_ptr<IRRegionStmtIterator> 
      multiBody(StmtHandle h, int bodyIndex) = 0; 

  //! Given a structured multi-way branch, return true if the cases have
  //! implied break semantics.  For example, this method would return false
  //! for C since one case will fall-through to the next if there is no
  //! explicit break statement.  Matlab, on the other hand, implicitly exits
  //! the switch statement once a particular case has executed, so this
  //! method would return true.
  virtual bool isBreakImplied(StmtHandle multicond) = 0; 

  //! Given a structured multi-way branch, return true if the body 
  //! corresponding to target 'bodyIndex' is the default/catchall/ case.
  virtual bool isCatchAll(StmtHandle h, int bodyIndex) = 0;

  //! Given a structured multi-way branch, return an IRRegionStmtIterator
  //! for the body corresponding to default/catchall case.  
  virtual OA_ptr<IRRegionStmtIterator> getMultiCatchall (StmtHandle h) = 0;
  
  //! Given a structured multi-way branch, return the condition
  //! expression corresponding to target 'bodyIndex'. The n targets are
  //! indexed [0..n-1].
  virtual ExprHandle getSMultiCondition(StmtHandle h, int bodyIndex) = 0; 

  //--------------------------------------------------------
  // Unstructured two-way conditionals: 
  //--------------------------------------------------------

  //! Given an unstructured two-way branch, return the label of the
  //! target statement.  The second parameter is currently unused.
  virtual StmtLabel  getTargetLabel(StmtHandle h, int n) = 0; 

  //--------------------------------------------------------
  // Unstructured multi-way conditionals
  // FIXME: Review all of the multi-way stuff.
  //--------------------------------------------------------

  //! Given an unstructured multi-way branch, return the number of targets.
  //! The count does not include the optional default/catchall case.
  virtual int numUMultiTargets(StmtHandle h) = 0; 

  //! Given an unstructured multi-way branch, return the label of the target
  //! statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
  virtual StmtLabel getUMultiTargetLabel(StmtHandle h, int targetIndex) = 0; 

  //! Given an unstructured multi-way branch, return label of the target
  //! corresponding to the optional default/catchall case.  Return 0
  //! if there is no default target.
  virtual StmtLabel getUMultiCatchallLabel(StmtHandle h) = 0; 

  // Given an unstructured multi-way branch, return the condition
  // expression corresponding to target 'targetIndex'. The n targets
  // are indexed [0..n-1].
  // multiway target condition 
  virtual ExprHandle getUMultiCondition(StmtHandle h, int targetIndex) = 0; 

  //--------------------------------------------------------
  // Special, for assembly-language level instructions only.
  // These are necessary because there are some intricacies involved
  // in building a CFG for an instruction set which has delayed branches,
  // and in particular, allows branches within branch delay slots. 
  //--------------------------------------------------------

  //! Given a statement, return true if it issues in parallel with its
  //! successor.  This would be used, for example, when the underlying IR
  //! is a low-level/assembly-level language for a VLIW or superscalar
  //! instruction set. The default implementation (which is appropriate
  //! for most IR's) is to return false.
  virtual bool parallelWithSuccessor(StmtHandle h) = 0;

  //! Given an unstructured branch/jump statement, return the number
  //! of delay slots. Again, this would be used when the underlying IR
  //! is a low-level/assembly-level language for a VLIW or superscalar
  //! instruction set. The default implementation (which is appropriate
  //! for most IR's) is to return 0.
  virtual int numberOfDelaySlots(StmtHandle h) = 0;


  //--------------------------------------------------------
  // Symbol Handles
  //--------------------------------------------------------

  // Currently never used. This might be deprecated in the future.
  // virtual SymHandle getProcSymHandle(ProcHandle h) = 0;
  
};


  } // end of namespace CFG
} // end of namespace OA

#endif // CFGIRInterface_h
