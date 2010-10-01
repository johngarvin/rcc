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

// File: DefaultAnnotationMap.h
//
// Common implementation details for most annotation maps.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef DEFAULT_ANNOTATION_MAP_H
#define DEFAULT_ANNOTATION_MAP_H

#include <map>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class DefaultAnnotationMap : public AnnotationMap {
public:
  explicit DefaultAnnotationMap();
  virtual ~DefaultAnnotationMap();

  virtual void put(const MyKeyT & k, MyMappedT value);
  virtual MyMappedT get(const MyKeyT & k);

  virtual bool is_valid(const MyKeyT & k);
  virtual bool is_computed() const;
  virtual bool computation_in_progress() const;
  virtual void reset();

  // iterators
  virtual iterator begin();
  virtual const_iterator begin() const;
  virtual iterator end();
  virtual const_iterator end() const;
  
protected:
  std::map<MyKeyT, MyMappedT> & get_map();
  void delete_map_values();
  void compute_if_necessary();
  virtual void compute() = 0;              // Template Method pattern

private:
  bool m_computed;
  bool m_computation_in_progress;
  std::map<MyKeyT, MyMappedT> m_map;
};

} // end namespace RAnnot

#endif
