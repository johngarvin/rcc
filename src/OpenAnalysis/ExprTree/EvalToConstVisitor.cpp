/*! \file
  
  \brief Implementation of EvalToConstVisitor class.
  
  \authors Barbara Kreaseck, Michelle Strout
  \version $Id: EvalToConstVisitor.cpp,v 1.5 2005/03/17 21:47:45 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/ExprTree/EvalToConstVisitor.hpp>

static bool debug = false;

namespace OA {


EvalToConstVisitor::EvalToConstVisitor(OA_ptr<EvalToConstVisitorIRInterface> ir,
                                       OA_ptr<ReachConsts::Interface> rc,
                                       OA_ptr<Alias::Interface> alias,
                                       StmtHandle stmt)
  : mIR(ir), mReachConsts(rc), mAlias(alias), mStmt(stmt)
{
  mCall = CallHandle(0);
  mHaveStmt = true;

  mEvalResult = NULL;

}

EvalToConstVisitor::EvalToConstVisitor(OA_ptr<EvalToConstVisitorIRInterface> ir,
                                       OA_ptr<ReachConsts::Interface> rc,
                                       OA_ptr<Alias::Interface> alias,
                                       CallHandle call)
  : mIR(ir), mReachConsts(rc), mAlias(alias), mCall(call)
{
  mStmt = StmtHandle(0);
  mHaveStmt = false;

  mEvalResult = NULL;
}

//! if we don't know how to handle a node then the default action
//! is to change the eval result to NULL
void EvalToConstVisitor::visitNode(const ExprTree::Node&)
{
    mEvalResult = NULL;
}

void EvalToConstVisitor::visitOpNode(const ExprTree::OpNode& n)
{
  //OpHandle opH = n.getHandle();
  OA_ptr<ConstValBasicInterface> ch1; ch1 = NULL;
  OA_ptr<ConstValBasicInterface> ch2; ch2 = NULL;

  if (debug){
    /*
      std::cout << "In EvalToConstVisitor::visitOpNode---> OP=";
      n.dump(std::cout,mIR);
      std::cout << std::endl;
      
                << mIR->returnOpEnumValInt(opH) 
                << " (" << n.num_children() << " children), ";
      */
  }

  // check that there are at most 2 children
  if ( n.num_children() <= 2 ) {

    // if so then visit each child
    OA_ptr<ExprTree::Node> cetNodePtr;
    ExprTree::ChildNodesIterator cNodesIter(n);

    // child1
    if (cNodesIter.isValid()) {
      cetNodePtr = cNodesIter.current();

      cetNodePtr->acceptVisitor(*this);
      ch1 = getConstVal();

    }
    ++cNodesIter;

    // child2
    if (cNodesIter.isValid()) {
      cetNodePtr = cNodesIter.current();

      cetNodePtr->acceptVisitor(*this);
      ch2 = getConstVal();
    }
  
    if (debug){
      std::cout << " with children { (";
      if (ch1.ptrEqual(NULL)) {
        std::cout << "NULL";
      } else {
        std::cout << ch1->toString();
      }
      std::cout << ") (";
      if (ch2.ptrEqual(NULL)) {
        std::cout << "NULL";
      } else {
        std::cout << ch2->toString();
      }
      std::cout << ")" << std::endl;
      std::cout.flush();
    }

    OA_ptr<OpBasicInterface> opH = n.getOpInterface();

    mEvalResult = mIR->evalOp(opH,ch1,ch2);

  // there are more than 2 children
  } else {
    mEvalResult = NULL;
  }

  if (debug) {
      if (mEvalResult.ptrEqual(NULL)) {
        std::cout << " } evalResult (NULL)" << std::endl;
      } else {
        std::cout << " } evalResult (" << mEvalResult->toString() 
                  << ")" << std::endl;
      }
  }

}

void EvalToConstVisitor::visitCallNode(const ExprTree::CallNode& n)
{
  mEvalResult = NULL;
}

void EvalToConstVisitor::visitMemRefNode(const ExprTree::MemRefNode& n)
{
  mEvalResult = NULL;
  
  if (mReachConsts.ptrEqual(NULL)) {
    return;
  }

  OA_ptr<MemRefExpr> mre = n.getMRE();

  if(mre->isaAddressOf()) {
    return; // addressOf do not have const values
  }
  
  OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(mre);
  if (mHaveStmt) {
    mEvalResult = mReachConsts->getReachConst(mStmt,aSet);
  } else {
    mEvalResult = mReachConsts->getReachConst(mCall,aSet);
  }

}

void EvalToConstVisitor::visitConstSymNode(const ExprTree::ConstSymNode& n)
{
  //mEvalResult = mIR->getConstValBasic(n.getHandle());
  //-------------
  // not sure why the above no longer works, but cannot just comment it
  // out and not set mEvalResult to NULL.  Maybe should just assert??

  mEvalResult = NULL;
}

void EvalToConstVisitor::visitConstValNode(const ExprTree::ConstValNode& n)
{
  mEvalResult = n.getConstValInterface();
}


} // end of OA namespace

