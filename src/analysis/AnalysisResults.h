/* $Id: AnalysisResults.h,v 1.7 2006/05/20 14:49:59 garvin Exp $ */
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
#include <analysis/FuncInfo.h>

#include <support/stacks/StackTmpl.h>

typedef std::map<char *,RAnnot::FuncInfo*> func_attribute_map;
extern func_attribute_map  func_unique_defintion;

//**************************************************************************/
// macros 
//**************************************************************************/

//--------------------------------------------------------------------------
// property access
//--------------------------------------------------------------------------
#define getProperty(name, sexp) \
  dynamic_cast<name *>(analysisResults.lookup(name::handle(), sexp))

#define putProperty(name, sexp, prop, own) \
  analysisResults.insert(name::handle(), sexp, prop, own)

void clearProperties();

//**************************************************************************/
// extern declarations for global variables
//**************************************************************************/
extern RProp::PropertySet analysisResults;
extern StackTmpl<RAnnot::FuncInfo*> lexicalContext;

#endif
