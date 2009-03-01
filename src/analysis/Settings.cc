// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: Settings.cc
//
// Settings for analyses and optimizations. Default is to turn all optimizations on.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "Settings.h"

Settings * Settings::s_instance = 0;

Settings * Settings::get_instance() {
  if (s_instance == 0) {
    s_instance = new Settings();
  }
  return s_instance;
}
