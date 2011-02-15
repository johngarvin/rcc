/*! \file
  
  \brief Represents a context for a call (minimally a k-call path)

  \authors Barbara Kreaseck
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CallContext_H
#define CallContext_H

#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include "Interface.hpp"

#include <list>

namespace OA {
  namespace Alias {

//! Represents a context for a call (minimally a k-call path)
//! Intended usage is for context-sensitive alias analysis. 
class CallContext: public Annotation {
public:

  //! static class method to set maxDegree of call context.
  //! should be called/set once at beginning of analysis.
  static void setMaxDegree(int m);

  // ************************************************************
  //  Constructors (& destructor)
  // ************************************************************
  //! copies first MaxDegree CallHandles during construction
  CallContext(std::list<CallHandle> l);

  //! copies first MaxDegree CallHandles during construction
  CallContext(OA_ptr<CallContext> cc);

  ~CallContext();

  // ************************************************************
  //  Comparison
  // ************************************************************
  //! compares first MaxDegree CallHandles
  bool operator ==(const CallContext& other) const;


  // ************************************************************
  //  Construction
  // ************************************************************
  
  //!Expand context by given CallHandle (i.e., add it to the end of the list)
  //! *this calls 'call'
  //! before A->B
  //! after  A->B->call
  //!*** will only expand when list contains less than MaxDegree CallHandles
  void append(CallHandle call);

  //!Expand context by given CallHandle (i.e., add it to the front of the list)
  //! 'call' calls *this
  //! before A->B
  //! after call->A->B
  //!*** will only expand when list contains less than MaxDegree CallHandles
  void prepend(CallHandle call);

  //!Return a clone of first MaxDegree CallHandles
  CallContext& clone() const;

  //!assignment
  CallContext& operator=(const CallContext& other);


  // ************************************************************
  //  Output
  // ************************************************************
  virtual void output(OA::IRHandlesIRInterface& ir) const;

  virtual void output(OA::IRHandlesIRInterface& ir,
		      const Alias::Interface& aliasResults) const;

  void dump(
	    std::ostream& os,
	    IRHandlesIRInterface& ir,
	    Interface& aliasResults);

  //  std::string toString(OA::IRHandlesIRInterface& ir) const;
  //  std::string toString(OA::IRHandlesIRInterface& ir,
  //                       const Interface& aliasResults) const;

  //friend CallContextIterator;


private:
  static int mMaxDegree;

  std::list<CallHandle> mList;
  
}; // end of CallContext


  }
}

#endif
