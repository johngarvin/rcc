/*! \file
  
  \brief Declarations of the AnnotationManager that generates a CFGStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout, Priyadarshini Malusare
  \version $Id: ManagerCFGStandard.hpp,v 1.6.2.1 2006/01/19 05:30:48 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef MANAGERCFGSTANDARD_H
#define MANAGERCFGSTANDARD_H

//--------------------------------------------------------------------
// standard headers
#ifdef NO_STD_CHEADERS
# include <string.h>
#else
# include <cstring>
#endif

// STL headers
#include <list>
#include <set>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/IRInterface/CFGIRInterface.hpp>

namespace OA {
  namespace CFG {


//--------------------------------------------------------------------
class Worklist;


//--------------------------------------------------------------------
/*! 
   The AnnotationManager for a CFGStandard Annotation.  Knows how to
   build a CFG, read one in from a file, and write one out to a file.
*/
class ManagerCFGStandard { //??? eventually public OA::AnnotationManager
public:
  ManagerCFGStandard(OA_ptr<CFGIRInterface> _ir, bool _build_stmt_level_cfg = false);
  virtual ~ManagerCFGStandard () { }

  //??? don't think this guy need AQM, but will eventually have
  //to have one so is standard with other AnnotationManagers
  virtual OA_ptr<CFG::CFG> performAnalysis(ProcHandle);



  //------------------------------------------------------------------
  // Exceptions
  //------------------------------------------------------------------
/*  class Unexpected_Break : public Exception {
  public:
    void report (std::ostream& os) const
    { os << "E!  Unexpected break statement." << std::endl; }
  };
  //------------------------------------------------------------------
  class Unexpected_Return : public Exception {
  public:
    void report (std::ostream& os) const
    { os << "E!  Unexpected return statement." << std::endl; }
  };
  //------------------------------------------------------------------
  class Unexpected_Continue : public Exception {
  public:
    void report (std::ostream& os) const
    { os << "E!  Unexpected continue statement." << std::endl; }
  };
  //------------------------------------------------------------------

*/

private:

  //------------------------------------------------------------------
  // Methods that recursively build CFG
  //------------------------------------------------------------------
    IRStmtType build_block (OA_ptr<Node> prev_node,
                            OA_ptr<IRRegionStmtIterator> si,
                            std::list<CFG::NodeLabel>& exit_nodes,
                            OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
                            OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                            OA_ptr<std::list<CFG::NodeLabel> > continue_nodes);

    IRStmtType build_stmt (OA_ptr<Node> prev_node, OA::StmtHandle,
                           std::list<CFG::NodeLabel>& exit_nodes,
                           OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
                           OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                           OA_ptr<std::list<CFG::NodeLabel> > continue_nodes);


    IRStmtType build_CFG_loop (OA_ptr<Node> prev_node,
                               OA::StmtHandle th,
                               std::list<CFG::NodeLabel>& exit_nodes,
                               OA_ptr<std::list<CFG::NodeLabel> > return_nodes);

    IRStmtType build_CFG_twoway_branch (OA_ptr<Node> prev_node,
                                OA::StmtHandle th,
                                std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > continue_nodes);

     IRStmtType build_CFG_multiway_branch (OA_ptr<Node> prev_node,
                                StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > break_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > continue_nodes);
                                
      
     IRStmtType build_CFG_multiway_branch_with_fallthrough ( OA_ptr<Node> prev_node,
                                StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > continue_nodes);
          
    IRStmtType build_CFG_end_tested_loop (OA_ptr<Node> prev_node,
                                StmtHandle th, std::list<CFG::NodeLabel>& exit_nodes,
                                OA_ptr<std::list<CFG::NodeLabel> > return_nodes);
 
    IRStmtType build_CFG_unconditional_jump ( OA_ptr<Node> prev_node,
                                StmtHandle stmt);

    IRStmtType build_CFG_ustruct_twoway_branch (OA_ptr<Node>
        prev_node, StmtHandle stmt,  std::list<CFG::NodeLabel>& exit_nodes);



    IRStmtType build_CFG_unconditional_jump_i ( OA_ptr<Node> prev_node,
                                 StmtHandle stmt);


    IRStmtType build_CFG_ustruct_multiway_branch (OA_ptr<Node>
                                 prev_node, StmtHandle stmt);

  //******************************************
  // Support for building cfgs with delay slots.
  //******************************************
  void HandleDelayedBranches();
//  bool isInternalBranch(StmtHandle);
//  void processBlock(OA_ptr<CFGStandard::Node> );
  void createBasicCFG();
//  void processBlock();

 


private:
  OA_ptr<CFGIRInterface> mIR;
  OA_ptr<CFG> mCFG;


  //------------------------------------------
  // data structures for handling delay slots
  //------------------------------------------
  std::map<OA_ptr<Node>, OA_ptr<Node> > mFallThrough;

  //---------------------------------------------------------------
  // build a CFG for individual statements rather than basic blocks
  //---------------------------------------------------------------
  const bool mBuildStmtLevelCFG;
   };

  } // end of CFG namespace
} // end of OA namespace

#endif
