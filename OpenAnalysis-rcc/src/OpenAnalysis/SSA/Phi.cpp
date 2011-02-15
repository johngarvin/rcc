/*! \file
  
  \brief Declaration for Phi.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: Phi.cpp,v 1.2 2005/03/11 18:13:21 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

//-----------------------------------------------------------------------------
// OpenAnalysis headers
#include "Phi.hpp"
//-----------------------------------------------------------------------------

namespace OA {
  namespace SSA {

//-----------------------------------------------------------------------------
void
Phi::dump(std::ostream& os)
{
#if 0 // FIXME
  os << cfg->GetIRInterface().GetSymNameFromSymHandle(sym) << " = phi (";
  ArgIterator arg_iter(*this);
  CFG::Node* n;
  LeafHandle l;
  if ((bool)arg_iter) {
    n = arg_iter;
    l = arg_iter;
    n->dump(os);
    os << ":";
    cfg->GetIRInterface().PrintLeaf(l, os);
    ++arg_iter;
    while ((bool)arg_iter) {
      os << ", ";
      n = arg_iter;
      l = arg_iter;
      n->dump(os);
      os << ":";
      cfg->GetIRInterface().PrintLeaf(l, os);
      ++arg_iter;
    }
  }
  os << ")";
#endif
}
//-----------------------------------------------------------------------------

  } // end of SSA namespace
} // end of OA namespace
