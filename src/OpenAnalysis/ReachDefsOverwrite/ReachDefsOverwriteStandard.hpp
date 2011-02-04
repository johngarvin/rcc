/*! \file
  
\brief Maps StmtHandles to set of StmtHandle defs that reach each StmtHandle and defs that overwrite this statements def

Copyright (c) 2002-2005, Rice University <br>
Copyright (c) 2004-2005, University of Chicago <br>
Copyright (c) 2006, Contributors <br>
All rights reserved. <br>
See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsOverwriteStandard_hpp
#define ReachDefsOverwriteStandard_hpp

#include <iostream>
#include <map>
#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ReachDefs/ReachDefsStandard.hpp>
#include <OpenAnalysis/ReachDefsOverwrite/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace ReachDefsOverwrite {

    class ReachDefsOverwriteStandard : public ReachDefsOverwrite::Interface,
				       public ReachDefs::ReachDefsStandard {
    public:
      ReachDefsOverwriteStandard(ProcHandle p) ;

      ~ReachDefsOverwriteStandard() {}

      /**
       * for a given s return the list of statements that overwritte
       * what is defined by s
       */
      OA_ptr<ReachDefsIterator> getOverwritingStmts(StmtHandle s);

      //! insert a relation: def is overwritten by ovw
      void insertOverwrittenBy(StmtHandle def, 
			       StmtHandle ovw);

      //! will use OutputBuilder to generate output 
      void output(IRHandlesIRInterface& pIR);

    private:
      // data members
      std::map<StmtHandle,OA_ptr<std::set<StmtHandle> > > mOverwrittenBy;

    };

  } 
} 

#endif

