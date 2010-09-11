// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: SideEffectAnnotationMap.h
//
// Maps expression SEXPs to side effect information.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SIDE_EFFECT_ANNOTATION_MAP_H
#define SIDE_EFFECT_ANNOTATION_MAP_H

#include <map>
#include <vector>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>
#include <analysis/SideEffect.h>

namespace RAnnot {

class FuncInfo;

class SideEffectLibMap {
public:
  explicit SideEffectLibMap();
  void insert(std::string key, bool side_effects, bool expensive, bool error);
  bool get(std::string key, int which);
private:
  std::map<std::string, std::vector<bool> > m_map;
};

class SideEffectAnnotationMap : public DefaultAnnotationMap {
public:
  // destructor
  virtual ~SideEffectAnnotationMap();

  // singleton
  static SideEffectAnnotationMap * instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  // private constructor for singleton pattern
  explicit SideEffectAnnotationMap();

  void compute();
  void compute_oa_side_effect();
  void make_side_effect(const FuncInfo * const, const SEXP e);

  void init_lib_data();

  bool expression_is_trivial(const SEXP e);
  bool expression_is_cheap(const SEXP e);
  bool call_may_have_action(const SEXP e);
  bool call_may_be_expensive(const SEXP e);
  bool call_may_throw_error(const SEXP e);

  void add_all_names_used(SideEffect * annot,
			  OA::OA_ptr<OA::Alias::Interface> alias,
			  OA::OA_ptr<OA::Alias::AliasTagIterator> tag_iter);
  void add_all_names_defined(SideEffect * annot,
			     OA::OA_ptr<OA::Alias::Interface> alias,
			     OA::OA_ptr<OA::Alias::AliasTagIterator> tag_iter);

  static SideEffectAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();

  OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> m_side_effect;
  OA::OA_ptr<OA::Alias::InterAliasInterface> m_alias;
  SideEffectLibMap m_non_action_libs;
};

}  // end namespace RAnnot

#endif
