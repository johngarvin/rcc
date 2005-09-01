/* $Id: AnalysisResults.h,v 1.2 2005/09/01 17:43:06 johnmc Exp $ */
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

extern RProp::PropertySet analysisResults;

#define getProperty(name, sexp) dynamic_cast<name *>(analysisResults.lookup(name::name ## Property, sexp))
#define putProperty(name, sexp, prop, own) analysisResults.insert(name::name ## Property, sexp, prop, own)

#endif
