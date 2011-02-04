/*! \file
  
  \brief The AnnotationManager that generates a SSAStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: ManagerSSAStandard.cpp,v 1.2 2005/04/07 17:33:26 eraxxon Exp $

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
#include "ManagerSSAStandard.hpp"


namespace OA {
  namespace SSA {

static bool debug = false;
    

OA_ptr<SSAStandard> 
ManagerStandard::performAnalysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg)
{
  SymHandle procName = cfg->getName();
  mSSA = new SSAStandard(procName, mIR, cfg);
  build(cfg);
  return mSSA;
}


void 
ManagerStandard::build(OA_ptr<CFG::CFGInterface> cfg)
{
  // FIXME: move SSAStandard::SSAStandard to here
}
 

  } // end of SSA namespace
} // end of OA namespace
