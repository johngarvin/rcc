/*! \file
  
  \brief Concrete ExprTreeVisitor that collects all Memory Reference
         Expressions in the ExprTree.
  
  \authors Michelle Strout, Priyadarshini Malusare
  \version $Id: Modified EvalToMemRefVisitor.hpp,v 1.3 2005/03/17 21:47:45 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CollectMREVisitor_H
#define CollectMREVisitor_H

#include <OpenAnalysis/ExprTree/ExprTreeVisitor.hpp>

namespace OA {

//--------------------------------------------------------------------
/*! 
*/
class CollectMREVisitor : public ExprTreeVisitor {
public:
  CollectMREVisitor() { 
  }
  
  ~CollectMREVisitor() {}

  void visitExprTreeBefore(const ExprTree&) { }
  void visitExprTreeAfter(const ExprTree&) { }

  //! ===============================================================
  //! Method for each ExprTree::Node subclass
  //! Collect set of Memory Reference Expressions for each MemRefNode 
  //! ===============================================================
  
  void visitNode(const ExprTree::Node&) { }

  void visitOpNode(const ExprTree::OpNode& n) {
       OA_ptr<ExprTree::Node> cetNodePtr;
       ExprTree::ChildNodesIterator cNodesIter(n);

       for(;cNodesIter.isValid(); ++cNodesIter) {
           cetNodePtr = cNodesIter.current();
           cetNodePtr->acceptVisitor(*this);
       }
  }
  void visitCallNode(const ExprTree::CallNode& n) { }

  //! If MemRefNode found, store MREs into the Map
  void visitMemRefNode(const ExprTree::MemRefNode& n)  
  {
    mSet.insert(n.getMRE()); 
  }
  
  void visitConstSymNode(const ExprTree::ConstSymNode& n)  { }

  void visitConstValNode(const ExprTree::ConstValNode& n)  { }

  //! ===============================================================
  //! An Interface to get set of Memory Reference Expression
  //! ===============================================================
  std::set<OA_ptr<MemRefExpr> > getMemRef() { 
       return mSet;
  }

private:
  //! Store set of Memory Reference Expressions.
  std::set<OA_ptr<MemRefExpr> > mSet;
};


} // end of OA namespace

#endif
