// Copyright (c) 2003-2005 John Garvin 
//
//  July 11, 2003 
//
//  Parses R code, turns into C code that uses internal R functions.
//  Attempts to output some code in regular C rather than using R
//  functions.
//  
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
//

#include <support/StringUtils.h>

std::string make_type(int t) {
  switch(t) {
  case NILSXP:     return  "0 /* NILSXP */";
  case SYMSXP:     return  "1 /* SYMSXP */";
  case LISTSXP:    return  "2 /* LISTSXP */";
  case CLOSXP:     return  "3 /* CLOSXP */";
  case ENVSXP:     return  "4 /* ENVSXP */";
  case PROMSXP:    return  "5 /* PROMSXP */";
  case LANGSXP:    return  "6 /* LANGSXP */";
  case SPECIALSXP: return  "7 /* SPECIALSXP */";
  case BUILTINSXP: return  "8 /* BUILTINSXP */";
  case CHARSXP:    return  "9 /* CHARSXP */";
  case LGLSXP:     return "10 /* LGLSXP */";
  case INTSXP:     return "13 /* INTSXP */";
  case REALSXP:    return "14 /* REALSXP */";
  case CPLXSXP:    return "15 /* CPLXSXP */";
  case STRSXP:     return "16 /* STRSXP */";
  case DOTSXP:     return "17 /* DOTSXP */";
  case ANYSXP:     return "18 /* ANYSXP */";
  case VECSXP:     return "19 /* VECSXP */";
  case EXPRSXP:    return "20 /* EXPRSXP */";
  case BCODESXP:   return "21 /* BCODESXP */";
  case EXTPTRSXP:  return "22 /* EXTPTRSXP */";
  case WEAKREFSXP: return "23 /* WEAKREFSXP */";
  case FUNSXP:     return "99 /* FUNSXP */";
  default: err("make_type: invalid type"); return "BOGUS";
  }
}

std::string indent(std::string str) {
  const std::string IND_STR = "  ";
  if (str.empty()) return "";
  std::string newstr = IND_STR;   // Add indentation to beginning
  std::string::iterator it;

  // Indent after every newline (unless there's one at the end)
  for(it = str.begin(); it != str.end(); it++) {
    if (*it == '\n' && it != str.end() - 1) {
      newstr += '\n' + IND_STR;
    } else {
      newstr += *it;
    }
  }
  return newstr;
}

// Rrrrrgh. C++: the language that makes the hard things hard and the
// easy things hard.
//
std::string i_to_s(const int i) {
  if (i == (int)0x80000000) {
    return "0x80000000";      // Doesn't parse as a decimal constant
  } else {
    std::ostringstream ss;
    ss << i;
    return ss.str();
  }
}

std::string d_to_s(double d) {
  if (d == HUGE_VAL) {
    return "HUGE_VAL";         // special R value
  } else {
    std::ostringstream ss;
    ss << d;
    return ss.str();
  }
}

std::string c_to_s(Rcomplex c) {
  return "mk_complex(" + d_to_s(c.r) + "," + d_to_s(c.i) + ")";
}

// Escape "'s, \'s and \n's to turn a string into its representation
// in C code.
//
std::string escape(std::string str) {
  unsigned int i;
  std::string out = "";
  for(i=0; i<str.size(); i++) {
    if (str[i] == '\n') {
      out += "\\n";
    } else if (str[i] == '"') {
      out += "\\\"";
    } else if (str[i] == '\\') {
      out += "\\\\";
    } else {
      out += str[i];
    }
  }
  return out;
}

// Make a std::string suitable for use as a C identifier. Used for both R
// identifiers and arbitrary strings for syntactic sugar in
// variable names. It's important that no two R identifiers can map to
// the same C name, but for arbitrary strings it doesn't matter.
//
std::string make_c_id(std::string str) {
  std::string out;
  unsigned int i;
  if (!isalpha(str[0])) {
    out += 'a';
  }
  for(i=0; i<str.size(); i++) {
    if (!isalnum(str[i])) {
      out += '_';
    } else {
      out += str[i];
    }
  }
  return out;
}

// Simple function to add quotation marks around a string
std::string quote(std::string str) {
  return "\"" + str + "\"";
}

std::string unp(std::string str) {
  return "UNPROTECT_PTR(" + str + ");\n";
}

std::string strip_suffix(std::string name) {
  std::string::size_type pos = name.rfind(".", name.size());
  if (pos == std::string::npos) {
    return name;
  } else {
    return name.erase(pos, name.size() - pos);
  }
}

int filename_pos(std::string str) {
  std::string::size_type pos = str.rfind("/", str.size());
  if (pos == std::string::npos) {
    return 0;
  } else {
    return pos + 1;
  }
}

void err(std::string message) {
  std::cerr << "Error: " << message;
  exit(1);
}
