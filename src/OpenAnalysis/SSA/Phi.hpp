/*! \file
  
  \brief Declaration for Phi.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: Phi.hpp,v 1.2 2005/03/11 18:13:22 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef OA_PHI_H
#define OA_PHI_H

//-----------------------------------------------------------------------------

#include <iostream>

// STL headers
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/Utils/Iterator.hpp>
#include <OpenAnalysis/Utils/Exception.hpp>

//-----------------------------------------------------------------------------

namespace OA {
  namespace SSA {

//-----------------------------------------------------------------------------
class Phi {

public:
  class ArgIterator;
  friend class ArgIterator;

  //--------------------------------------------------------------------
  class ArgIterator : public Iterator {
  public:
    ArgIterator(const Phi& p) 
      : center(const_cast<Phi&>(p)), iter(center.args.begin()) { }
    ~ArgIterator() { }
    
    OA_ptr<CFG::NodeInterface> currentSrc() const { return (*iter).first; }
    LeafHandle                   currentTarg() const { return (*iter).second; }
    bool isValid() const { return (iter != center.args.end()); }
    
    void operator++() { ++iter; }
    void reset() { iter = center.args.begin(); }
    
  private:
    Phi& center;
    std::map<OA_ptr<CFG::NodeInterface>, LeafHandle>::iterator iter;
  };
  //--------------------------------------------------------------------

public:
  Phi(const SymHandle& var_name, OA_ptr<CFG::CFGInterface> cfg_)
    : sym(var_name), cfg(cfg_) { }
  ~Phi() { }
  
  void add_arg(OA_ptr<CFG::NodeInterface> c_n, LeafHandle a_n) 
    { args[c_n] = a_n; }
  LeafHandle arg(OA_ptr<CFG::NodeInterface> n) { return args[n]; }
  int num_args() { return args.size(); }

  bool operator==(Phi& other) { return &other == this; }
  bool operator<(Phi& other) { return this < &other; }
  
  void dump(std::ostream&);
  
  //-------------------------------------
  // Iterators
  //-------------------------------------
  OA_ptr<ArgIterator> getArgIterator() const {
    OA_ptr<ArgIterator> it; it = new ArgIterator(*this);
    return it;
  }

private:
  SymHandle sym;
  OA_ptr<CFG::CFGInterface> cfg;
  
  std::map<OA_ptr<CFG::NodeInterface>, LeafHandle> args;
};
//-----------------------------------------------------------------------------

  } // end of SSA namespace
} // end of OA namespace

#endif
