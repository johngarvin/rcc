/*! \file
  
  \brief The AnnotationManager that generates a CFGStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: ManagerCFGStandard.cpp,v 1.19.2.1 2006/01/19 05:30:48 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

// standard headers

#ifdef NO_STD_CHEADERS
# include <stdlib.h>
# include <string.h>
# include <assert.h>
#else
# include <cstdlib>
# include <cstring>
# include <cassert>
using namespace std; // For compatibility with non-std C headers
#endif

#include <iostream>
using std::ostream;
using std::endl;
using std::cout;
using std::cerr;
#include <list>
#include <set>
#include <map>

// Mint headers
#include "ManagerCFG.hpp"


namespace OA {
  namespace CFG {

static bool debug = false;

  ManagerCFGStandard::ManagerCFGStandard (OA_ptr<CFGIRInterface> _ir,
                                bool _build_stmt_level_cfg /* = false */ )
        : mIR(_ir), mBuildStmtLevelCFG(_build_stmt_level_cfg)
        {
 
       
             OA_DEBUG_CTRL_MACRO("DEBUG_ManagerCFG:ALL", debug);
             mFallThrough.clear();
        }


  OA_ptr<CFG::CFG> ManagerCFGStandard::performAnalysis(ProcHandle proc)
  {
    // get information from the ir interface
    bool return_statements_allowed = mIR->returnStatementsAllowed();
    // create a CFG that just has an entry and an exit node
    mCFG = new CFG();
    OA_ptr<NodesIteratorInterface> nodes_iter;
    // Create the unique entry node.
    OA_ptr<Node> entry; 
    entry = new Node();
    mCFG->setEntry(entry);
    mCFG->addNode(entry);

    // Create the actual start node where the first statements wil be added.
    OA_ptr<Node> r; r = new Node();
    mCFG->addNode(r);
    mCFG->connect (mCFG->getEntry(), r, FALLTHROUGH_EDGE);
    CFG::NodeLabelList exit_nodes;
    // some NodeLabelList's are OA_ptr's instead of passed around by reference
    // because some of the logic looks to see if the list is NULL
    OA_ptr<CFG::NodeLabelList> return_nodes;
    return_nodes = new CFG::NodeLabelList;
    OA_ptr<IRRegionStmtIterator> stmt_iterptr = mIR->procBody(proc);
    OA_ptr<CFG::NodeLabelList> nullList; nullList = NULL;

    if (return_statements_allowed) {
      build_block(r, stmt_iterptr, exit_nodes, nullList, return_nodes, nullList);
    } else {
      build_block(r, stmt_iterptr, exit_nodes, nullList, nullList, nullList);
    }

    // create unique exit node
    OA_ptr<Node> final; final = new Node();
    mCFG->addNode(final);
    mCFG->setExit(final);
    // connect the exit nodes to the final node
    mCFG->connect(exit_nodes, final);
    // return nodes also lead to the exit node of the sub-program
    mCFG->connect(*return_nodes, final);
    // finalize control flow for unstructured constructs
    HandleDelayedBranches();
    return mCFG;

  }


  //--------------------------------------------------------------------
  IRStmtType
  ManagerCFGStandard::build_block (OA_ptr<Node> prev_node,
                                OA_ptr<IRRegionStmtIterator> si_ptr,
                                std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > continue_nodes)
  {
      std::list<CFG::NodeLabel> x_nodes;

      IRStmtType prev_statement = SIMPLE;
      while (si_ptr->isValid()) {
        StmtHandle stmt = si_ptr->current();
        if (debug) { std::cout << mIR->toString(stmt) << std::endl; }
       
        if (mIR->getLabel(stmt)) {
            StmtLabel lab = mIR->getLabel(stmt);
            OA_ptr<Node> new_node;
         
            if (mCFG->isLabelMappedToNode(lab)) {
                new_node = mCFG->node_from_label(lab);
            } else if (! prev_node->empty()) {
                new_node = new Node();
                mCFG->addNode(new_node);
                if(debug) {
                    cout << "\nCFG Node";
                    new_node->dump(cout , mIR);
                    cout << "\n";
                }
                mCFG->mapLabelToNode(lab,new_node);

            } else {
                mCFG->mapLabelToNode(lab,prev_node);
                new_node = prev_node;
            }

            if (prev_statement != SIMPLE && !new_node.ptrEqual(prev_node)) {
                mCFG->connect(x_nodes, new_node);
            } else if (!prev_node.ptrEqual(NULL)
                     && !new_node.ptrEqual(prev_node))
            {
                mCFG->connect(prev_node, new_node, FALLTHROUGH_EDGE);
            }
            x_nodes.clear();
            prev_node = new_node;
      
                  
         } else if (mBuildStmtLevelCFG) {
            if (!prev_node->empty()) {
                OA_ptr<Node> new_node;
                new_node = new Node();
                mCFG->addNode(new_node);
                if(debug) {
                    cout << "\n CFG Node";
                    new_node->dump(cout , mIR);
                    cout << "\n";
                }

               
                if (prev_statement == SIMPLE) {
                    mCFG->connect(prev_node, new_node, FALLTHROUGH_EDGE);
                } else {
                   mCFG->connect(x_nodes, new_node);
                }
                prev_node = new_node;
            }  
            x_nodes.clear();

         } else if (prev_statement != SIMPLE) {
            // now we need to create a new CFG node
            prev_node = new Node();
            mCFG->addNode(prev_node);
            if(debug) {
                 cout << "\n CFG Node";
                 prev_node->dump(cout , mIR);
                 cout << "\n";
            }


            mCFG->connect(x_nodes, prev_node);
            x_nodes.clear();
         }
         prev_statement = build_stmt(prev_node, stmt, x_nodes, break_nodes,
                                                      return_nodes, continue_nodes);
         ++(*si_ptr);
      }  

       
      if (prev_statement == SIMPLE) {
        exit_nodes.push_back(CFG::NodeLabel(prev_node,
                         FALLTHROUGH_EDGE));
      } else {
        CFG::NodeLabelListIterator x_nodes_iter(x_nodes);
        while (x_nodes_iter.isValid()) {
          exit_nodes.push_back(x_nodes_iter.current());
          ++x_nodes_iter;
        }
      }
      
      return NONE;
  }


  //--------------------------------------------------------------------
/*!
   Build a CFG for the IR rooted at the given node.  The CFG builder
   depends upon the IR providing a minimal interface.  In particular, the CFG
   needs for the IR nodes to be able to categorize themselves as simple
   statements, statement lists, loops, two-way branches, and multi-way
   branches.

   The routine uses the given node prev_node as the entry node and updates the
   exit_nodes for the CFG that it builds.  Notice that for every CFG component
   built by this call, there is exactly one entry node, but there may be
   multiple exit nodes.
*/
IRStmtType
ManagerCFGStandard::build_stmt (OA_ptr<Node> prev_node,
    OA::StmtHandle stmt,
    std::list<CFG::NodeLabel>& exit_nodes,
    OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
    OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
    OA_ptr<std::list<CFG::NodeLabel> > continue_nodes)
{
    switch (mIR->getCFGStmtType(stmt)) {
       case SIMPLE:
         {
            prev_node->add(stmt);
            if (debug) { std::cout << mIR->toString(stmt) << std::endl; }
            return SIMPLE;
         }
       case COMPOUND:
         {
            build_block(prev_node, mIR->getFirstInCompound(stmt), exit_nodes,
            break_nodes, return_nodes, continue_nodes);
            return COMPOUND;
         }
       case BREAK:
         {
            prev_node->add(stmt);
            if (break_nodes.ptrEqual(0)) {
               //  throw Unexpected_Break();
            } else {
               break_nodes->push_back(CFG::NodeLabel(prev_node,
                               BREAK_EDGE));
            }
            return BREAK;
         }
       case RETURN:
         {
            prev_node->add(stmt);
            if (return_nodes.ptrEqual(0)) {
               //throw Unexpected_Return();
            } else {
              return_nodes->push_back(CFG::NodeLabel(prev_node,
                                RETURN_EDGE));
            }
           return RETURN;
         }
       case LOOP:
       {
           return build_CFG_loop (prev_node, stmt, exit_nodes, return_nodes);
       }
 
       case STRUCT_TWOWAY_CONDITIONAL:
       {
           return build_CFG_twoway_branch (prev_node, stmt, exit_nodes, break_nodes,
                                      return_nodes, continue_nodes);
       }

       case STRUCT_MULTIWAY_CONDITIONAL:
       {
            if (mIR->isBreakImplied(stmt)) {
                return build_CFG_multiway_branch (prev_node, stmt, exit_nodes,
                                          break_nodes, return_nodes,
                                          continue_nodes);
            }else {
                return build_CFG_multiway_branch_with_fallthrough (prev_node, stmt,                                           exit_nodes, return_nodes, continue_nodes);
            }
            
       } 

       case END_TESTED_LOOP:
       {
            return build_CFG_end_tested_loop (prev_node, stmt, exit_nodes, return_nodes);
       }
      
       case LOOP_CONTINUE:
       {
            prev_node->add(stmt);
            if (continue_nodes.ptrEqual(0)) {
      //         throw Unexpected_Continue();
            } else {
                 continue_nodes->push_back(CFG::NodeLabel(prev_node,
                                  CONTINUE_EDGE));
            }
            return LOOP_CONTINUE;
       }
       //
       // These unstructured constructs have the possibility of being
       // delayed branches (depending on the underlying machine). Those
       // without delay slots are handled now.  Those with delay slots
       // are just added as simple statements.  A post-processing step
       // will finalize control flow for them.
       //
       case UNCONDITIONAL_JUMP:
       {
            if (mIR->numberOfDelaySlots(stmt) == 0) {
                return build_CFG_unconditional_jump (prev_node, stmt);
            } else {
                prev_node->add(stmt);
                return SIMPLE;
            }
       }
       case USTRUCT_TWOWAY_CONDITIONAL_T:
       case USTRUCT_TWOWAY_CONDITIONAL_F:
       {
             if (mIR->numberOfDelaySlots(stmt) == 0) {
                return build_CFG_ustruct_twoway_branch (prev_node, stmt, exit_nodes);
             } else {
                prev_node->add(stmt);
                return SIMPLE;
             }
       }
      case UNCONDITIONAL_JUMP_I:
       {
            if (mIR->numberOfDelaySlots(stmt) == 0) {
                return build_CFG_unconditional_jump_i (prev_node, stmt);
            } else {
                prev_node->add(stmt);
                return SIMPLE;
            }
       }
      case USTRUCT_MULTIWAY_CONDITIONAL:
       {    
           // Currently assume multiways don't have delay slots.
           assert(mIR->numberOfDelaySlots(stmt) == 0);
           return build_CFG_ustruct_multiway_branch (prev_node, stmt);
       }
           // FIXME: Unimplemented.
      case ALTERNATE_PROC_ENTRY:
       {
           cout << "FIXME: ALTERNATE_PROC_ENTRY not yet implemented" << endl;
           assert (0);
           return ALTERNATE_PROC_ENTRY;
       }
    default:
       {
           assert (0);
       }

 
    }
}


IRStmtType
ManagerCFGStandard::build_CFG_loop(OA_ptr<Node> prev_node,
                                StmtHandle th,
                                std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes)
{
    // If there is a header, (as in a "for" loop), it is assimilated in the
  // previous (prev_node) CFG node.  The condition needs a separate CFG node.
  // So, unless prev_node is empty (in which case that can become the
  // condition node) a new CFG node must be allocated for the loop condition.
  OA_ptr<Node> c;

  StmtHandle header = mIR->loopHeader(th);
  if (header) {
    prev_node->add(header);
  }

  // To handle C and Fortran loops in the simplest way we take the following
  // measures:
  //
  // If the number of loop iterations is defined
  // at loop entry (i.e. Fortran semantics), we add the loop statement
  // representative to the header node so that definitions from inside
  // the loop don't reach the condition and increment specifications
  // in the loop statement.
  //
  // On the other hand, if the number of iterations is not defined at
  // entry (i.e. C semantics), we add the loop statement to a node that
  // is inside the loop in the CFG so definitions inside the loop will
  // reach uses in the conditional test. for C style semantics, the
  // increment itself may be a separate statement. if so, it will appear
  // explicitly at the bottom of the loop.

  bool definedAtEntry =  mIR->loopIterationsDefinedAtEntry(th);
  if (definedAtEntry) {
    prev_node->add(th);
  }
  if (prev_node->empty()) {
    prev_node->add(th);
    c = prev_node;
  }
  else {
    c = new Node(th);
    mCFG->addNode(c);
    mCFG->connect(prev_node, c, FALLTHROUGH_EDGE);
  }
  exit_nodes.push_back(CFG::NodeLabel(c, FALSE_EDGE));

  // allocate a new CFG node for the loop body and recur
  std::list<CFG::NodeLabel> x_nodes;
  OA_ptr<IRRegionStmtIterator> body = mIR->loopBody(th);
  if (body->isValid() && body->current()) {
    OA_ptr<Node> b; 
    b = new Node();
    mCFG->addNode(b);

    if(debug)
    {
                 cout << "\n CFG Node";
                 b->dump(cout , mIR);
                 cout << "\n";
    }

 

    
    OA_ptr<std::list<CFG::NodeLabel> > break_statements, continue_statements;
    break_statements = new std::list<CFG::NodeLabel>;
    continue_statements = new std::list<CFG::NodeLabel>;
    if (build_block(b, body, x_nodes, break_statements, return_nodes,
                    continue_statements) == SIMPLE)
    {
      x_nodes.push_back(CFG::NodeLabel(b, FALLTHROUGH_EDGE));
    }

    mCFG->connect(c, b, TRUE_EDGE);
    CFG::NodeLabelListIterator break_iter(*break_statements);
    while (break_iter.isValid()) {
      exit_nodes.push_back(break_iter.current());
      ++break_iter;
    }

    CFG::NodeLabelListIterator continue_iter(*continue_statements);
    while (continue_iter.isValid()) {
      x_nodes.push_back(continue_iter.current());
      ++continue_iter;
    }
  }  else {// empty body
    x_nodes.push_back(CFG::NodeLabel(c, TRUE_EDGE));
  }

  // allocate a new CFG node for the increment part, if needed
  StmtHandle incr = mIR->getLoopIncrement(th);
  if (incr) {
    OA_ptr<Node> li; 
    li = new Node(incr);
    mCFG->addNode(li);
    mCFG->connect(x_nodes, li);
    mCFG->connect(li, c, FALLTHROUGH_EDGE);
  }
  else {
    mCFG->connect(x_nodes, c);
  }

  return LOOP;

 
}


//--------------------------------------------------------------------
/*!
   A two-way branch consists of a condition along with a true branch
   and a false branch.  Note that "elseif" statements should be
   translated to this form while parsing, since, they are semantically
   equivalent to a chain of nested "else if" statements.  Indeed,
   "elseif" is just a short form for "else if".  The general structure
   of the CFG for a two-way branch is as follows:

    <PRE>
        |
        V
    condition
      |      \
      V       V
   true_body false_body
      |          |
       \________/
           |
           V
    </PRE>
*/

IRStmtType
ManagerCFGStandard::build_CFG_twoway_branch (OA_ptr<Node> prev_node,
        StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > continue_nodes)
{

  if(debug)
  {
      std::cout << "Inside CFG::build_CFG_twoway_branch" << endl;
  }
 // add the if statement itself to the previous block. it represents
  // the conditional branch terminating the block.
  prev_node->add(th);

  // construct the CFG for the true block
  OA_ptr<Node> b;
  OA_ptr<IRRegionStmtIterator> true_body = mIR->trueBody(th);
  if (true_body->isValid() && true_body->current()) {
    b = new Node();

    if(debug)
    {
        std::cout << "True branch" << std::endl;
    }

    if(debug)
   {
       std::cout << "In CFG addNode" << std::endl;
       b->dump(cout , mIR);
   }

   mCFG->addNode(b);

   if(debug)
   {
       std::cout << "CFG addNode ends" << std::endl;
   }

    
    if (build_block(b, true_body, exit_nodes, break_nodes, return_nodes, continue_nodes) == SIMPLE)
      exit_nodes.push_back(CFG::NodeLabel(b, FALLTHROUGH_EDGE));

    mCFG->connect(prev_node, b, TRUE_EDGE);
  }
  else {
    // the body is empty
    b = 0;
    exit_nodes.push_back(CFG::NodeLabel(prev_node, TRUE_EDGE));
  }

 // handle the false block
  OA_ptr<IRRegionStmtIterator> false_body = mIR->elseBody(th);
  if (false_body->isValid() && false_body->current()) {
    OA_ptr<Node> c_body; c_body = new Node();
    if(debug)
    {
        std::cout << "false branch" << std::endl;
    }

    if(debug)
    {
       std::cout << "In CFG addNode" << std::endl;
       c_body->dump(cout , mIR);
    }

    mCFG->addNode(c_body);

    if(debug)
    {
      std::cout << "CFG addNode ends" << std::endl;
    }

    
    if (build_block(c_body, false_body, exit_nodes, break_nodes, return_nodes, continue_nodes) == SIMPLE)
      exit_nodes.push_back(CFG::NodeLabel(c_body, FALLTHROUGH_EDGE));

    mCFG->connect(prev_node, c_body, FALSE_EDGE);
  }
  else if (!b.ptrEqual(0)) // if b is 0 then the "condition" node has already been added to exit_nodes
    exit_nodes.push_back(CFG::NodeLabel(prev_node, FALSE_EDGE));

  return STRUCT_TWOWAY_CONDITIONAL;

  if(debug)
  {
      std::cout << "*****  CFG::build_CFG_twoway_branch *****" << endl;
  }
  
}


//--------------------------------------------------------------------
/*!
   A multi-way branch consists of a multiway condition expression that
   branches off into a number of cases, and an optional catchall
   (default) body.  The structure of a multi-way branch is as follows:

    <PRE>
                |
                |
                V
         ___condition______
        /  /  |  \ ...\    \
       /  /   |   \    \    \
      V  V    V    V    V    V
     B1  B2   B3  B4    Bk  catchall
      |  |    |    |    |    |
      \__|____|____|____|___/
                |
                |
                V
    </PRE>
*/
IRStmtType
ManagerCFGStandard::build_CFG_multiway_branch (OA_ptr<Node> prev_node,
        StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
        OA_ptr<std::list<CFG::NodeLabel> > continue_nodes)
{
   // add the switch statement to the previous block. the switch statement
  // represents the multi-way conditional branch terminating the block.
  prev_node->add(th);

  bool direct_drop = false;    // should there be a direct edge from the condition to exit?
  bool default_cond = false;   // is there a default case?

  // iterate over the multi-branches
  for (int bridx = 0; bridx < mIR->numMultiCases(th); bridx++) {
    OA_ptr<IRRegionStmtIterator> body = mIR->multiBody(th, bridx);

    // if there is an empty body then just make a direct drop, this is correct since
    // break is always implied
    if (body->isValid() && body->current()) {
      OA_ptr<Node> c; c = new Node();
      mCFG->addNode(c);

 if(debug)
                {
                 cout << "\n build_CFG_multiway_branch CFG Node";
                 c->dump(cout , mIR);
                 cout << "\n";
                }


      if (build_block(c, body, exit_nodes, break_nodes, return_nodes, continue_nodes) == SIMPLE)
      {
        exit_nodes.push_back(CFG::NodeLabel(c, FALLTHROUGH_EDGE));
      }

       // check if this is the catch all case
       if (mIR->isCatchAll(th,bridx)) {
           default_cond = true;
           mCFG->connect(prev_node, c, MULTIWAY_EDGE);
       } else {
           ExprHandle multiCond = mIR->getSMultiCondition(th, bridx);
           mCFG->connect(prev_node, c, MULTIWAY_EDGE, multiCond);
       }
       
       // there is an empty body with an implied break
     } else {
           direct_drop = true;
     }
  
   } /* for */

   // if we didn't find a catchall then there should be a direct drop
   if (default_cond == false) {
       direct_drop = true;
   }
   
   // direct drop is true iff there is not catchall or at least one multi-branch body is null
   if (direct_drop) {
       exit_nodes.push_back(CFG::NodeLabel(prev_node, FALLTHROUGH_EDGE));
   }
   
   return STRUCT_MULTIWAY_CONDITIONAL;

}


//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*!
   This is similar to build_CFG_multiway_branch, but it handles switch
   statements that have fall-through semantics for each case (that is,
   there are no implied breaks).  Each case can fall-through to the
   following case if an explicit control flow statement (e.g., break
   in C) is not present.  Also, unlike build_CFG_multiway_branch, the
   catchall/default case is not necessarily the last.  The functions
   are different enough that is seems cleaner to keep them separate.

   FIXME: The default case is still assumed to be last. All of the
   underlying infrastructure of mint assumes this and has to be fixed.
   MMS 12/03: this function no longer assumes that the default case is
   last, not sure what other code might still make this assumption
*/
                                 


IRStmtType
ManagerCFGStandard::build_CFG_multiway_branch_with_fallthrough (
       OA_ptr<Node> prev_node,
       StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
       OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
       OA_ptr<std::list<CFG::NodeLabel> > continue_nodes)
{
     // add the switch statement to the previous block. the switch statement
     // represents the multi-way conditional branch terminating the block.
     prev_node->add(th);
     
     //bool direct_drop = true;   // should there be a direct edge from the condition to exit?
     bool empty_cond = false;     // is there an empty case?
     bool default_cond = false;   // is there a default case
    
    // We must create a new list of break statements since any breaks seen
    // in subtrees of this statement will exit this switch, not some other
    // enclosing construct.
    OA_ptr<std::list<CFG::NodeLabel> > break_switch_statements;
    break_switch_statements = new std::list<CFG::NodeLabel>;

   // Track the last case body processed so that fall-through
   // connections can be made This will be non-null whenever the
   // previous case falls through (i.e., did not end in a break
   // statement).  the case_exit_nodes list will contain all the exits
   // from the previous case block
   OA_ptr<Node> prev_case_node;
   prev_case_node = 0;
   std::list<CFG::NodeLabel> case_exit_nodes;

   // iterate over the multi-branches
   for (int bridx = 0; bridx < mIR->numMultiCases(th); bridx++) {

    OA_ptr<IRRegionStmtIterator> body = mIR->multiBody(th, bridx);
    //if (body->isValid() && body->current()) {
    OA_ptr<Node> c; c = new Node();
    mCFG->addNode(c);

 if(debug)
                {
                 cout << "\n Add CFG multiway branch with_Fallthrough CFG Node";
                 c->dump(cout , mIR);
                 cout << "\n";
                }


    
    break_switch_statements->clear();
    // If a previous case falls-through, connect all of its exit nodes
    // to the current case node.
    if (!prev_case_node.ptrEqual(NULL)) {
      mCFG->connect(case_exit_nodes, c);
      case_exit_nodes.clear();
      prev_case_node = 0;
    }

   // if there are statements in the case body then look for break
    if (body->isValid() && body->current()) {
      // A placeholder for exit nodes.  We don't want the case body to
      // be an exit from the switch construct unless there is a break.
      // Either the body will have a break statement that collects
      // exits or the exits will all be connected the next case node
      IRStmtType prev_case_statement = build_block(c, body, case_exit_nodes,
                                                   break_switch_statements,
                                                   return_nodes, continue_nodes);

      // if there wasn't a break statement then update case_exit_nodes
      if ( break_switch_statements->size() == 0 ) {
        prev_case_node = c;
        if (prev_case_statement == SIMPLE) {
          case_exit_nodes.push_back(CFG::NodeLabel(prev_case_node, FALLTHROUGH_EDGE));
        }

      // if we did see a break then this case node will not fall through to next case node
      } else {
        prev_case_node = 0;

        // There should only be one break, but iterate anyway.
        CFG::NodeLabelListIterator
            break_stmt_iter(*break_switch_statements);
        while (break_stmt_iter.isValid()) {
          exit_nodes.push_back(break_stmt_iter.current());
          ++break_stmt_iter;
        }
        //IMPROVEME: if the body only has a break than we can simplify
        //the CFG by adding a direct_drop edge and removing this CFG
        //node, if figure out how to do this just need to set
        //empty_cond to true

      }

    // if have an empty block, still have a CFG node because break is not implied
    } else {
      prev_case_node = c;
      case_exit_nodes.push_back(CFG::NodeLabel(prev_case_node, FALLTHROUGH_EDGE));
    }

    // check if this is the catch all case
    if (mIR->isCatchAll(th,bridx)) {
      default_cond = true;
      mCFG->connect(prev_node, c, MULTIWAY_EDGE);
    } else {
      ExprHandle multiCond = mIR->getSMultiCondition(th, bridx);
      mCFG->connect(prev_node, c, MULTIWAY_EDGE, multiCond);
    }

  } /* for */

  // if last condition didn't have a break then it won't be on
  // the exit_nodes stack and should be added
  if (!prev_case_node.ptrEqual(NULL)) {
    exit_nodes.push_back(CFG::NodeLabel(prev_case_node, FALLTHROUGH_EDGE));
  }

  // direct drop is true iff there is not catchall or at least one multi-branch body is null
  if ( !default_cond || empty_cond ) {
    exit_nodes.push_back(CFG::NodeLabel(prev_node, FALLTHROUGH_EDGE));
  }

  return STRUCT_MULTIWAY_CONDITIONAL;
           
}


//--------------------------------------------------------------------
/*!
   The general structure of an end-tested loop (e.g., do-while in C)
   CFG is as below:
    <PRE>
              |
              |   ______
              |  /      \
              V V        |
             body        |
              |          |
              |          |
              V          |
          condition      |
              |\________/
              |
              |
              |
              V

    </PRE>
    The statement representing the do-while loop will be used to
    represent the condition block in the CFG.
*/

IRStmtType
ManagerCFGStandard::build_CFG_end_tested_loop (OA_ptr<Node> prev_node,
                StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
                        OA_ptr<std::list<CFG::NodeLabel> > return_nodes)
{
  // FIXME: New construct, partially tested.
  // New CFG nodes must be allocated for both the body and loop condition (we can re-use
  // prev_node for the body if it is empty).
  OA_ptr<Node> b; b = NULL;
  OA_ptr<Node> c; c = NULL;

 // Process the loop body.
  std::list<CFG::NodeLabel> x_nodes;
  OA_ptr<IRRegionStmtIterator> body = mIR->loopBody(th);
  if (body->isValid() && body->current()) {
    if (prev_node->empty()) {
      b = prev_node;
    } else {
      b = new Node();
      mCFG->addNode(b);
      mCFG->connect(prev_node, b, FALLTHROUGH_EDGE);
    }
    OA_ptr<std::list<CFG::NodeLabel> > break_statements, continue_statements;
    break_statements = new std::list<CFG::NodeLabel>;
    continue_statements = new std::list<CFG::NodeLabel>;
    if (build_block(b, body, x_nodes, break_statements, return_nodes, continue_statements) == SIMPLE) {
      x_nodes.push_back(CFG::NodeLabel(b, FALLTHROUGH_EDGE));
    }

    CFG::NodeLabelListIterator break_iter(*break_statements);
    while (break_iter.isValid()) {
      exit_nodes.push_back(break_iter.current());
      ++break_iter;
    }

    CFG::NodeLabelListIterator continue_iter(*continue_statements);
    while (continue_iter.isValid()) {
      x_nodes.push_back(continue_iter.current());
      ++continue_iter;
    }
  }
  else { // empty body
    // FIXME: c is NULL at this point...
    x_nodes.push_back(CFG::NodeLabel(c, TRUE_EDGE));
  }

  // Allocate a node for the condition and make the proper connections.
  c = new Node(th);
  mCFG->addNode(c);
  exit_nodes.push_back(CFG::NodeLabel(c, FALSE_EDGE));
  mCFG->connect(c, b, TRUE_EDGE);
  mCFG->connect(x_nodes, c);

  return END_TESTED_LOOP;

}

IRStmtType
ManagerCFGStandard::build_CFG_unconditional_jump (
        OA_ptr<Node> prev_node,
        StmtHandle stmt)
{
  // FIXME: New construct, partially tested.
  StmtLabel lab = mIR->getTargetLabel(stmt, 0);
  prev_node->add(stmt);
  mCFG->connect(prev_node, mCFG->node_from_label(lab), FALLTHROUGH_EDGE);
  return UNCONDITIONAL_JUMP;
}


//--------------------------------------------------------------------
/*!
    Handle unstructured twoway branches.
    The structure of a branch-on-true is as follows:

    <PRE>
        |
        V
    condition_t
      |        \ (true edge)
      V          \
    fall-through   \
     block          V
                  true block
    </PRE>

    A similar subgraph is created for branch-on-false, except that the
    fall-through is on true and the branching edge is a false edge.
*/

IRStmtType
ManagerCFGStandard::build_CFG_ustruct_twoway_branch (OA_ptr<Node>
        prev_node, StmtHandle stmt,  std::list<CFG::NodeLabel>& exit_nodes)
{
  // FIXME: New construct, partially untested.
  StmtLabel lab = mIR->getTargetLabel(stmt, 0);
  bool branch_on_true = (mIR->getCFGStmtType(stmt) == USTRUCT_TWOWAY_CONDITIONAL_T);
  prev_node->add(stmt);
  mCFG->connect(prev_node, mCFG->node_from_label(lab), (branch_on_true ? TRUE_EDGE : FALSE_EDGE));

  // This is so that the branch block will get connected to the fall-through block.
  exit_nodes.push_back(CFG::NodeLabel(prev_node, FALLTHROUGH_EDGE));  /// ??? Use FALSE_EDGE?

  return mIR->getCFGStmtType(stmt);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*!
   Handle unconditional indirect jumps.

    Create:
    <PRE>
      goto statement
       |   ...   |
       |         |
       V         V
    potential   potential
    target 0    target n
    </PRE>

    Currently, any label in the program unit is considered to
    be a potential target of the indirect branch. This could be very
    conservative for some constructs, such as Fortran computed gotos
    (since we could more intelligently examine only those labels that
    are used in the Fortran ASSIGN statement -- i.e., labels that have
    their address taken and stored). It may be worthwhile to revisit
    this if unacceptably large graphs result from a combination of many
    labels and many indirect branches.
*/
IRStmtType
ManagerCFGStandard::build_CFG_unconditional_jump_i (
        OA_ptr<Node> prev_node,
        StmtHandle stmt)
{
  // Every label must be seen before all the edges can be added.
  // Just add this to the list of indirect jumps, which will have their
  // edges added later.
  cout << "FIXME: UNCONDITIONAL_JUMP_I not yet implemented" << endl;
  assert(0);
  return UNCONDITIONAL_JUMP_I;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Handle unstructured multiway branches.

    An unstructured multiway is essentially any non-structured branch
    with multiple targets. An optional default/catchall target is also
    allowed. Examples of this construct include Fortran's computed goto
    statement or a simple jump/dispatch table in a low-level intermediate
    representation. A vanilla jump table will not have a default target,
    but a Fortran computed goto (for example) may if it isn't known that
    one of the targets is always taken.
    <PRE>
                |
                |
                V
         ____branch________
        /  /  |  \ ...\    \
       /  /   |   \    \    \
      V  V    V    V    V    V
     B1  B2   B3  B4    Bk  catchall (optional)

    </PRE>

    Each edge will have an associated condition. For case-like statements,
    this is the case value for the particular target. For jump/dispatch
    tables or computed gotos, which have no conditions, the index of the
    target is used (i.e., 0..number_of_targets).
*/

IRStmtType
ManagerCFGStandard::build_CFG_ustruct_multiway_branch (OA_ptr<Node>
        prev_node, StmtHandle stmt)
{
  // FIXME: New construct, partially tested.
  // Add the multi-way statement to the previous block. It represents
  // the multi-way branch terminating the block (which holds the selector
  // expression).
  prev_node->add(stmt);

  // Connect the branch to each of its targets.
  for (int br = 0; br < mIR->numUMultiTargets(stmt); br++) {
    StmtLabel target_lab = mIR->getUMultiTargetLabel(stmt, br);
    ExprHandle cond_expr = mIR->getUMultiCondition(stmt, br);
    mCFG->connect(prev_node, mCFG->node_from_label(target_lab),
                  MULTIWAY_EDGE, cond_expr );
  }

  // If there is a default target, make the proper connections.
  StmtLabel default_lab = mIR->getUMultiCatchallLabel(stmt);
  if (default_lab) {
    mCFG->connect(prev_node, mCFG->node_from_label(default_lab),
                  MULTIWAY_EDGE /* , multiCond */);
  }

  return USTRUCT_MULTIWAY_CONDITIONAL;
}


//*****************************************************************************


//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
// Refine the CFG to split blocks as necessary to handle delay slots, along
// with branches and labels in delay slots
//*****************************************************************************
void
ManagerCFGStandard::HandleDelayedBranches ()
{
  //
  // Preconditions:
  //
  // On input to this routine, all nodes and edges resulting from
  // structured control flow are already present.  In addition, all
  // nodes and edges induced by labels are present. To be finalized
  // here are the unstructured control flow contructs.
  //

  //
  // Create the "basic" CFG.  This adds control flow for the first
  // branch in a block (after its 'k' delay slots).
  //
  createBasicCFG();

#if 0 // FIXME: Disabled until its fixed.
  //
  // Finally, perform Waterman's worklist algorithm to finalize
  // control flow for the cases where branches are contained in
  // the delay slots of other branches.
  //

  // Place the start block of the procedure on the worklist
  Counterlist* emptyCounterList = new Counterlist;
  the_worklist->copyCountersToWorklist(Entry(), emptyCounterList);

  while (! the_worklist->isEmpty()) {
    for (i:NodesIterator nodes_iter(*this);
         (bool)nodes_iter && !the_worklist->isEmpty(); ++nodes_iter) {
      CFG::Node *node = dynamic_cast<CFG::Node*>((DGraph::Node*)nodes_iter);
      processBlock(node);
    }
  }
#endif
}


void
ManagerCFGStandard::createBasicCFG()
{
  // Create and populate the block_list.
  // PLM I have to modify this
  std::set<OA_ptr<Node> > block_list;
 
  OA_ptr<DGraph::NodesIteratorInterface> nodes_iter = mCFG->getNodesIterator();
  for( ; nodes_iter->isValid(); ++(*nodes_iter))
  {
    OA_ptr<DGraph::NodeInterface> dnode = nodes_iter->current();  
    OA_ptr<Node> node = dnode.convert<Node>();   

    /*! commented out by PLM 08/15/06
    OA_ptr<CFG::Node> node = nodes_iter.currentCFGNode();
    */

    block_list.insert(node);
  }
  
/*  for (NodesIterator nodes_iter(*mCFG); nodes_iter.isValid();
       ++nodes_iter)
  {
    OA_ptr<Node> node = nodes_iter.current();
    block_list.insert(node);
  }
*/

 // Process the blocks.
  while (block_list.size() > 0) {
    bool branch_found = false;
    int countdown = 0;
    StmtHandle stmt = 0;
    OA_ptr<Node> node = *block_list.begin();
    int numRemoved = block_list.erase(node);
    assert(numRemoved > 0);

    // Find first unstructured branch statement in this block.
   NodeStatementsIterator si(*node);
    for ( ; si.isValid(); ++si) {
      stmt = si.current();
      IRStmtType ty = mIR->getCFGStmtType(stmt);
      assert(ty != UNCONDITIONAL_JUMP_I);  // FIXME: Add support.
      if (ty == USTRUCT_TWOWAY_CONDITIONAL_T
          || ty == USTRUCT_TWOWAY_CONDITIONAL_F
          || ty == UNCONDITIONAL_JUMP) {
        branch_found = true;
        countdown = mIR->numberOfDelaySlots(stmt);
        break;
      } // if ty == branch
    } // For statements.



  // Only do this if countdown > 0, non-delayed branches are already
    // done in the main build routine.
    if (branch_found == true && countdown > 0) {
      // Move to end of this branch's delay slots.
      for ( ; si.isValid() && countdown > 0; ++si) {
        // FIXME: Need to check ParallelWithSuccessor, etc.
        --countdown;
      }


 if (countdown == 0) {
        //
        // Split the block just past its final delay slot.
        // Add the new block (the second piece of the original block) to
        // the block_list.
        // Add edges from the current block to the target(s) of the branch.
        //
        // FIXME: But before trying to do the split, check if the split point is
        // already at the end of the block.  This could happen if there
        // is a label just after the branch's final delay slot.
        //
        ++si;
        if (1 /* (bool)si == true */) {
          StmtHandle val = (si.isValid()) ? si.current() : (StmtHandle)0;
          OA_ptr<Node> newnode = mCFG->splitBlock (node, val);
          mFallThrough[node] = newnode;
          block_list.insert(newnode);
          IRStmtType ty = mIR->getCFGStmtType(stmt);
          if (ty == USTRUCT_TWOWAY_CONDITIONAL_T
              || ty == USTRUCT_TWOWAY_CONDITIONAL_F) {
            mCFG->connect(node, mCFG->getLabelBlock(mIR->getTargetLabel(stmt, 0)),
                    ty == USTRUCT_TWOWAY_CONDITIONAL_T ? TRUE_EDGE : FALSE_EDGE);
            mCFG->connect(node, newnode, FALLTHROUGH_EDGE);
          } else if (ty == UNCONDITIONAL_JUMP) {
            StmtLabel label = mIR->getTargetLabel(stmt, 0);
            OA_ptr<Node> nodeTmp = mCFG->getLabelBlock(label);
            mCFG->connect(node, nodeTmp, FALLTHROUGH_EDGE);
          } else {
            assert(0);
          }
        } // if (si)
      } // if (countdown == 0)


    } // if branch_found.

 if (branch_found == false || countdown > 0) {
      // ??? Add fall-thru edge from b (fixme, should already be there?).
      // Note, countdown > 0 can only happen if there was a label
      // in a delay slot.
    }

    
   } // While block_list.
}
  
  
  } // end of namespace CFG
} //end of namespace OA
