/* $Id: AnalysisResults.cc,v 1.4 2006/01/26 23:44:03 garvin Exp $ */
//****************************************************************************/
//                 Copyright (c) 1990-2005 Rice University
//                          All Rights Reserved
//****************************************************************************/

//***************************************************************************
// AnalysisResults.cc
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
  
#include <analysis/AnalysisResults.h>

RProp::PropertySet analysisResults;

void clearProperties() {
  analysisResults.clear();
}

StackTmpl<RAnnot::FuncInfo*> lexicalContext;

func_attribute_map  func_unique_defintion;

