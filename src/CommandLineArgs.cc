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

// File: CommandLineArgs.cc
//
// Represents the command line arguments to the compiler.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>
#include <iostream>

#include <analysis/Settings.h>

#include <support/RccError.h>
#include <support/StringUtils.h>

#include "CommandLineArgs.h"

static void arg_err();

CommandLineArgs::CommandLineArgs(int argc, char * argv[]) 
  : m_output_main_program(true),
    m_analysis_debug(false),
    m_out_file_exists(false),
    m_out_filename(""),
    m_in_file_exists(false),
    m_fullname("")
{
  int c;
  extern char * optarg;
  extern int optind;

    // get options
  while(1) {
    c = getopt(argc, argv, "df:mo:");
    if (c == -1) {
      break;
    }
    switch(c) {
    case 'd':
      // print debugging information
      m_analysis_debug = true;
      break;
    case 'f':
      // option flag specified
      add_f_option(std::string(optarg));
      break;
    case 'm':
      // don't output a main program
      m_output_main_program = false;
      break;
    case 'o':
      // specify output file
      m_out_file_exists = true;
      m_out_filename = std::string(optarg);
      break;
    case '?':
      arg_err();
      break;
    case ':':
      arg_err();
      break;
    default:
      rcc_error("Unknown error: getopt() returned " + i_to_s(c));
      break;
    }
  }

  // get filename, if it exists
  if (optind < argc) {
    m_in_file_exists = true;
    m_fullname = std::string(argv[optind++]);
    if (optind < argc) {
      std::cerr << "Warning: ignoring extra arguments: ";
      while (optind < argc) {
	std::cerr << argv[optind++] << " ";
      }
      std::cerr << std::endl;
    }
  } else {  // no filename specified
    m_in_file_exists = false;
  }

}

bool CommandLineArgs::get_output_main_program() { return m_output_main_program; }
bool CommandLineArgs::get_analysis_debug() { return m_analysis_debug; }
bool CommandLineArgs::get_out_file_exists() { return m_out_file_exists; }
std::string CommandLineArgs::get_out_filename() { return m_out_filename; }
bool CommandLineArgs::get_in_file_exists() { return m_in_file_exists; }
std::string CommandLineArgs::get_fullname() { return m_fullname; }

void CommandLineArgs::add_f_option(std::string option) {
  Settings * settings = Settings::instance();
  bool flag;
  size_t prefix_size = 3; // size of "no-" modifier

  // -ffoo    means make foo true
  // -fno-foo means make foo false
  if (option.substr(0, prefix_size) == "no-") {
    flag = false;
    option.erase(0, prefix_size);
  } else {
    flag = true;
  }

  if (option == "for-loop-range-deforestation") {
    settings->set_for_loop_range_deforestation(flag);
  } else if (option == "subscript-assignment") {
    settings->set_subscript_assignment(flag);
  } else if (option == "strictness") {
    settings->set_strictness(flag);
  } else if (option == "special-case-arithmetic") {
    settings->set_special_case_arithmetic(flag);
  } else if (option == "call-graph") {
    settings->set_call_graph(flag);
  } else if (option == "lookup-elimination") {
    settings->set_lookup_elimination(flag);
  } else if (option == "stack_alloc_obj") {
    settings->set_stack_alloc_obj(flag);
  } else if (option == "stack_debug") {
    settings->set_stack_debug(flag);
  } else if (option == "assume-correct-program") {
    settings->set_assume_correct_program(flag);
  } else if (option == "aggressive-CBV") {
    settings->set_aggressive_cbv(flag);
  } else if (option == "resolve-arguments") {
    settings->set_resolve_arguments(flag);
  } else {
    arg_err();
  }
}

static void arg_err() {
  std::cerr << "Usage: rcc [input-file] [-a] [-c] [-d] [-f option...] [-l] [-m] [-o output-file]\n";
  exit(1);
}
