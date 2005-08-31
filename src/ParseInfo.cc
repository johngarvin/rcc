// -*- Mode: C++ -*-
// Copyright (c) 2005 Rice University
//
// August 29, 2005
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
///

#include "ParseInfo.h"

// initialize statics
std::map<std::string, std::string> ParseInfo::func_map;
std::map<std::string, std::string> ParseInfo::symbol_map;
std::map<std::string, std::string> ParseInfo::string_map;
std::map<double, std::string> ParseInfo::sc_real_map;
std::map<int, std::string> ParseInfo::sc_logical_map;
std::map<int, std::string> ParseInfo::sc_integer_map;
std::map<int, std::string> ParseInfo::primsxp_map;
std::list<std::string> ParseInfo::direct_funcs;
SubexpBuffer * ParseInfo::global_fundefs;
SplitSubexpBuffer * ParseInfo::global_constants;
SubexpBuffer * ParseInfo::global_labels;
