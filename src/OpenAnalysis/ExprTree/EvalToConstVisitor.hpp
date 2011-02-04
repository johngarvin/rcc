/*! \file
  
  \brief Concrete ExprTreeVisitor that will use reaching constant information
         if available and an IRInterface to attempt to evaluate 
         an expression tree to a ConstValBasicInterface.
  
  \authors Barbara Kreaseck, Michelle Strout
  \version $Id: EvalToConstVisitor.hpp,v 1.6 2005/03/17 21:47:45 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef EvalToConstVisitor_H
#define EvalToConstVisitor_H

#include <OpenAnalysis/ExprTree/ExprTreeVisitor.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/EvalToConstVisitorIRInterface.hpp>
#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/IRInterface/OpBasicInterface.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {

//--------------------------------------------------------------------
/*! 
*/
class EvalToConstVisitor : public ExprTreeVisitor {
public:
  EvalToConstVisitor(OA_ptr<EvalToConstVisitorIRInterface> ir,
                     OA_ptr<ReachConsts::Interface> rc,
                     OA_ptr<Alias::Interface> alias,
                     StmtHandle stmt);
  EvalToConstVisitor(OA_ptr<EvalToConstVisitorIRInterface> ir,
                     OA_ptr<ReachConsts::Interface> rc,
                     OA_ptr<Alias::Interface> alias,
                     CallHandle call);
  ~EvalToConstVisitor() {}

  void visitExprTreeBefore(const ExprTree&) { }
  void visitExprTreeAfter(const ExprTree&) { }

  //---------------------------------------
  // method for each ExprTree::Node subclass
  //---------------------------------------
  // default base class so that visitors can handle unknown
  // node sub-classes in a generic fashion
  void visitNode(const ExprTree::Node&);

  void visitOpNode(const ExprTree::OpNode& n);
  void visitCallNode(const ExprTree::CallNode& n);
  void visitMemRefNode(const ExprTree::MemRefNode& n);
  void visitConstSymNode(const ExprTree::ConstSymNode& n);
  void visitConstValNode(const ExprTree::ConstValNode& n);

  //! interface for results of visiting the expression tree
  OA_ptr<ConstValBasicInterface> getConstVal() { return mEvalResult; }

private:
  OA_ptr<ConstValBasicInterface> mEvalResult;
  OA_ptr<EvalToConstVisitorIRInterface> mIR;
  OA_ptr<ReachConsts::Interface> mReachConsts;
  OA_ptr<Alias::Interface> mAlias;

  bool mHaveStmt;
  StmtHandle mStmt;
  CallHandle mCall;


};


} // end of OA namespace

#endif
