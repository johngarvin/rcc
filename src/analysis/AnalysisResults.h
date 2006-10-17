// -*- Mode: C++ -*-
/* $Id: AnalysisResults.h,v 1.8 2006/10/17 22:09:49 garvin Exp $ */
//****************************************************************************/
//                 Copyright (c) 1990-2005 Rice University
//****************************************************************************/

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

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
