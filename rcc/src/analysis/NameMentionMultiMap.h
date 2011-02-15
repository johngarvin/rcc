// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
//
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

// File: NameMentionMultiMap.h
//
// Defines a mapping from names (SEXPs of SYMSXP type) to a set of
// mentions (SEXPs of LANGSXP or LISTSXP type that contain the name as
// the CAR).
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef NAME_MENTION_MULTI_MAP_H
#define NAME_MENTION_MULTI_MAP_H

#include <map>

#include <include/R/R_RInternals.h>

typedef SEXP Name;
typedef std::multimap<Name, SEXP> NameMentionMultiMap;

#endif 
