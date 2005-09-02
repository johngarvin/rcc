/* $Id: AnalysisResults.h,v 1.3 2005/09/02 14:01:02 johnmc Exp $ */
//****************************************************************************/
//                 Copyright (c) 1990-2005 Rice University
//                          All Rights Reserved
//****************************************************************************/

//***************************************************************************
// AnalysisResults.h
//
//   repository for analysis results
//
// Author: John Mellor-Crummey                                
//
// Creation Date: 31 August 2005
//
// Modification History:
//  31 August 2005 -- John Mellor-Crummey
//
//**************************************************************************/
  
#ifndef AnalysisResults_h
#define AnalysisResults_h

#include <analysis/PropertySet.h>
#include <analysis/Annotation.h>

#include <support/stacks/StackTmpl.h>


//**************************************************************************/
// macros 
//**************************************************************************/

//--------------------------------------------------------------------------
// property access
//--------------------------------------------------------------------------
#define getProperty(name, sexp) \
  dynamic_cast<name *>(analysisResults.lookup(name::name ## Property, sexp))

#define putProperty(name, sexp, prop, own) \
  analysisResults.insert(name::name ## Property, sexp, prop, own)



//**************************************************************************/
// extern declarations for global variables
//**************************************************************************/
extern RProp::PropertySet analysisResults;
extern StackTmpl<RAnnot::FuncInfo*> lexicalContext;

#endif
