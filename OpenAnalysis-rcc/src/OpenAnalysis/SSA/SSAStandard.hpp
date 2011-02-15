/*! \file
  
  \brief Declaration for standard SSAStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: SSAStandard.hpp,v 1.4 2005/08/18 14:06:56 johnmc Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SSAStandard_H
#define SSAStandard_H

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
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>
#include <OpenAnalysis/SSA/Phi.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/Utils/DomTree.hpp>
#include <OpenAnalysis/CFG/CFG.hpp>

namespace OA {
  namespace SSA {


//--------------------------------------------------------------------
/*! CFGStandard implements the CFG::Interface using the DGraph class.
    FIXME -- we should have an SSAStandard, etc.
*/ 
//--------------------------------------------------------------------

class SSAStandard /* : public virtual Interface */ {

public:
  class Use;
  class PhiNodesIterator;
  class NonLocalsIterator;
  class DefBlocksIterator;
  friend class PhiNodesIterator;
  friend class NonLocalsIterator;
  friend class DefBlocksIterator;
  
  //--------------------------------------------------------------------
  /*! 
  */
  //--------------------------------------------------------------------
  class Def {
  public:
    virtual ~Def() {}
    virtual void dump(std::ostream&) = 0;
    virtual std::list<Use*>* uses_list () = 0;
  };
  
  class Use {
  public:
    virtual ~Use() {}
    virtual void dump(std::ostream&) = 0;
    virtual Def* def() = 0;
  };
 
  //--------------------------------------------------------------------
  /*! 
  */
  //--------------------------------------------------------------------
  class LeafDef : public Def {
  public:
    LeafDef (LeafHandle l) : Def() { leaf = l; }
    
    void dump (std::ostream&);
    std::list<Use*>* uses_list () { return &uses; }
  private:
    LeafHandle leaf;
    std::list<Use*> uses;
  };

  class PhiDef : public Def {
  public:
    PhiDef(SSA::Phi* p) : Def() { phi = p; }
    ~PhiDef() { }

    void dump (std::ostream&);
    std::list<Use*>* uses_list() { return &uses; }
    
  private:
    SSA::Phi* phi;
    std::list<Use*> uses;
  };

  class LeafUse : public Use {
  public:
    LeafUse(LeafHandle l) : Use() { leaf = l; }
    void dump(std::ostream&);
    Def* def() { return definition; }
  private:
    LeafHandle leaf;
    Def* definition;
  };
  
  class PhiUse : public Use {
  public:
    PhiUse(SSA::Phi* p) : Use() { phi = p; }
    void dump(std::ostream&);
    Def* def() { return definition; }
  private:
    SSA::Phi* phi;
    Def* definition;
  };
  
  //--------------------------------------------------------------------
  /*! 
  */
  //--------------------------------------------------------------------
  class PhiNodesIterator : public Iterator {
  public:
    PhiNodesIterator(SSAStandard& s, OA_ptr<CFG::NodeInterface> n) 
      { phi_set = &s.phi_node_sets[n]; iter = phi_set->begin(); }
    virtual ~PhiNodesIterator() { }

    OA_ptr<SSA::Phi> current() const { return *iter; }
    bool isValid() const { return (iter != phi_set->end()); }
    
    void operator++ () { ++iter; }
    void reset() { iter = phi_set->begin(); }
    
  private:
    std::set<OA_ptr<SSA::Phi> >* phi_set;
    std::set<OA_ptr<SSA::Phi> >::iterator iter;
  };

  
  class NonLocalsIterator : public Iterator {
  public:
    NonLocalsIterator(SSAStandard& s) : ssa(s)
      { iter = ssa.non_locals.begin(); }
    virtual ~NonLocalsIterator() { }
    
    LeafHandle current() const { return *iter; }
    bool isValid() const { return (iter != ssa.non_locals.end()); }
    
    void operator++ () { ++iter; }
    void reset() { iter = ssa.non_locals.begin(); }

  private:
    SSAStandard& ssa;
    std::set<LeafHandle>::iterator iter;
  };


  class DefBlocksIterator : public Iterator {
  public:
    DefBlocksIterator(SSAStandard& s, SymHandle name) 
      { blk_set = &(s.def_blocks_set[name]); iter = blk_set->begin(); }
    
    OA_ptr<CFG::NodeInterface> current() const { return *iter; }
    bool isValid() const { return (iter != blk_set->end()); }
    
    void operator++() { ++iter; }
    void reset() { iter = blk_set->begin(); }
    
  private:
    std::set<OA_ptr<CFG::NodeInterface> >* blk_set;
    std::set<OA_ptr<CFG::NodeInterface> >::iterator iter;
  };

  
//-------------------------------------------------------------------
// SSAStandard methods
//-------------------------------------------------------------------
public:
  SSAStandard(const SymHandle name, 
	      OA_ptr<SSAIRInterface> ir, OA_ptr<CFG::CFGInterface> cfg);
  virtual ~SSAStandard();

  void dump (std::ostream&);

  //-------------------------------------
  // Information access
  //-------------------------------------

  //-------------------------------------
  // Iterators
  //-------------------------------------
  OA_ptr<PhiNodesIterator> getPhiNodesIterator(OA_ptr<CFG::NodeInterface> n) {
    OA_ptr<PhiNodesIterator> it; it = new PhiNodesIterator(*this, n);
    return it;
  }

  OA_ptr<NonLocalsIterator> getNonLocalsIterator() {
    OA_ptr<NonLocalsIterator> it; it = new NonLocalsIterator(*this);
    return it;
  }

  OA_ptr<DefBlocksIterator> getDefBlocksIterator(SymHandle nm) {
    OA_ptr<DefBlocksIterator> it; it = new DefBlocksIterator(*this, nm);
    return it;
  }


private:
  void compute_uses_sets();

private:
  SymHandle name;
  OA_ptr<CFG::CFGInterface> cfg;
  std::map<OA_ptr<CFG::NodeInterface>, 
           std::set<OA_ptr<SSA::Phi> > > phi_node_sets;
  
  
  // FIXME: move to ManagerSSA
  OA_ptr<SSAIRInterface> mIR;
  std::set<LeafHandle> non_locals;
  std::map<SymHandle, std::set<OA_ptr<CFG::NodeInterface> > > def_blocks_set;
  
};

  } // end of SSA namespace
} // end of OA namespace

#endif
