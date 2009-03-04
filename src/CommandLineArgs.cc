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

#include <support/RccError.h>
#include <support/StringUtils.h>

#include "CommandLineArgs.h"

static void arg_err();

CommandLineArgs::CommandLineArgs(int argc, char * argv[]) 
  : m_output_main_program(true),
    m_output_default_args(true),
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
    c = getopt(argc, argv, "admo:");
    if (c == -1) {
      break;
    }
    switch(c) {
    case 'a':
      m_output_default_args = false;
      break;
    case 'd':
      // print debugging information
      m_analysis_debug = true;
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
bool CommandLineArgs::get_output_default_args() { return m_output_default_args; }
bool CommandLineArgs::get_analysis_debug() { return m_analysis_debug; }
bool CommandLineArgs::get_out_file_exists() { return m_out_file_exists; }
std::string CommandLineArgs::get_out_filename() { return m_out_filename; }
bool CommandLineArgs::get_in_file_exists() { return m_in_file_exists; }
std::string CommandLineArgs::get_fullname() { return m_fullname; }

static void arg_err() {
  std::cerr << "Usage: rcc [input-file] [-a] [-c] [-d] [-l] [-m] [-o output-file]\n";
  exit(1);
}
