/* $Id: AnalysisResults.cc,v 1.2 2005/09/02 14:01:01 johnmc Exp $ */
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

StackTmpl<RAnnot::FuncInfo*> lexicalContext;
