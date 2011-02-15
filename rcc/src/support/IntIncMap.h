// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: IntIncMap.h
//
// Maps ints to ints that can only be incremented and read.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

class IntIncMap {
public:
  explicit IntIncMap();
  void inc(int key);
  int get(int key) const;
  int total() const;
  int max_key() const;
private:
  mutable std::map<int,int> m_map;
  int m_max_key;
};
