/*! \file
  
  \brief Implementation of OutputExprTreeVisitor class.
  
  \authors Luis Ramos
  \version $Id: OutputExprTreeVisitor.cpp,v 1.5 2005/03/17 21:47:45 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/ExprTree/OutputExprTreeVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

static bool debug = false;

namespace OA {


OutputExprTreeVisitor::OutputExprTreeVisitor(IRHandlesIRInterface& ir)
	:mIR(ir)
{
  mId = 0;
}

//! if we don't know how to handle a node then the default action
//! is to change the eval result to NULL
void OutputExprTreeVisitor::visitNode(const ExprTree::Node&)
{
    std::cout << "visitNode\n";
}

void OutputExprTreeVisitor::visitOpNode(const ExprTree::OpNode& n)
{
  int Id = mId;
  ostringstream label;
  label << "ET OpNode ( " << n.getOpInterface()->toString() << " )";
  sOutBuild->graphNodeStart(mId);
  sOutBuild->graphNodeLabel(label.str());
  mId++;


  if(n.num_children() >= 1)
  {
    // if so then visit each child
    OA_ptr<ExprTree::Node> cetNodePtr;
    ExprTree::ChildNodesIterator cNodesIter(n);

    for(;cNodesIter.isValid(); ++cNodesIter, ++mId) {
      cetNodePtr = cNodesIter.current();
 
      sOutBuild->graphEdgeStart();
        sOutBuild->graphEdgeLabelStart();
        sOutBuild->graphEdgeSourceNode(Id);
          cetNodePtr->acceptVisitor(*this);
        sOutBuild->graphEdgeLabelEnd();
    }
    
  } else {
      assert(0);
  }

  mId--;
}

void OutputExprTreeVisitor::visitCallNode(const ExprTree::CallNode& n)
{
    CallHandle call = n.getHandle();
    ostringstream label;
    label << "ET CallNode ( " << mIR.toString(call) << " )";

    sOutBuild->graphNodeStart(mId);
    sOutBuild->graphNodeLabel(label.str());
}

void OutputExprTreeVisitor::visitMemRefNode(const ExprTree::MemRefNode& n)
{
    ostringstream label;
    OA_ptr<MemRefExpr> mre;
    mre = n.getMRE();
    label << "ET MemRefNode ( ";
    mre->output(mIR);
    label << " )";

    sOutBuild->graphNodeStart(mId);
    sOutBuild->graphNodeLabel(label.str());
}

void OutputExprTreeVisitor::visitConstSymNode(const ExprTree::ConstSymNode& n)
{
    ConstSymHandle simnode = n.getHandle();
    ostringstream label;
    label << "ET ConstSymNode ( " << mIR.toString(simnode) << " )";

    sOutBuild->graphNodeStart(mId);
    sOutBuild->graphNodeLabel(label.str());
}

void OutputExprTreeVisitor::visitConstValNode(const ExprTree::ConstValNode& n)
{
    ostringstream label;
    label << "ET ConstValNode ( " 
          << n.getConstValInterface()->toString() 
          << " )";

    sOutBuild->graphNodeStart(mId);
    sOutBuild->graphNodeLabel(label.str());
}

void OutputExprTreeVisitor::output(IRHandlesIRInterface&) const
{
}



} // end of OA namespace

