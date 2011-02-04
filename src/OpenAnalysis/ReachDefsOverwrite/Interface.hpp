/*! \file
  
  \brief Declarations for the abstract ReachDefsOverwrite interface.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsOverwriteInterface_hpp
#define ReachDefsOverwriteInterface_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ReachDefs/Interface.hpp>

namespace OA {
  namespace ReachDefsOverwrite {

    class Interface : public virtual ReachDefs::Interface { 
    public:
      Interface() {}
      virtual ~Interface() {}
      
      //! Return an iterator over overwriting definitions 
      virtual OA_ptr<ReachDefsIterator> getOverwritingStmts(StmtHandle s) = 0;
      
    };

  } 
} 

#endif

