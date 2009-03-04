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

// File: CommandLineArgs.h
//
// Represents the command line arguments to the compiler.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef COMMAND_LINE_ARGS_H
#define COMMAND_LINE_ARGS_H

#include <string>

class CommandLineArgs {
public:
  CommandLineArgs(int argc, char * argv[]);
  
  bool get_output_main_program();
  bool get_output_default_args();
  bool get_analysis_debug();
  bool get_out_file_exists();
  std::string get_out_filename();
  bool get_in_file_exists();
  std::string get_fullname();

private:
  bool m_output_main_program;
  bool m_output_default_args;
  bool m_analysis_debug;
  bool m_out_file_exists;
  std::string m_out_filename;
  bool m_in_file_exists;
  std::string m_fullname;
};

#endif
