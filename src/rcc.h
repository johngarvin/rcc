/* -*-C++-*-
 * Copyright (c) 2003-2005 John Garvin 
 *
 * July 11, 2003 
 *
 * Parses R code, turns into C code that uses internal R functions.
 * Attempts to output some code in regular C rather than using R
 * functions.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef RCC_H
#define RCC_H

#define __USE_STD_IOSTREAM

#include <fstream>
#include <list>
#include <string>

extern "C" {

#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>

} //extern "C"

#include <rinternals.h>
#include <util.h>
#include <parser.h>
#include <R_Analyst.h>
#include <R_Utils.hpp>
#include "get_name.h"
#include "replacements.h"
#include "visibility.h"
#include "Macro.hpp"

using namespace std;

bool is_special(string func);

// VarRef: reference to an allocated variable inside a string: its
// name, location, and length of the list it represents.
//
struct VarRef {
  string name;
  int location;
  int size;
  VarRef(string nm, int loc, int sz) {
    name = nm;
    location = loc;
    size = sz;
  }
};

// AllocList keeps track of locally-allocated lists (LALs). When more
// than one LAL is live at the same time, they must be in different
// places in memory, but if they have different live ranges, they can
// use the same memory. Each memory location must be as big as the
// biggest list allocated there. 
//
struct AllocListElem {
  int max;
  bool occupied;
  list<VarRef> vars;
  AllocListElem(int m, bool o, list<VarRef> vs) {
    max = m;
    occupied = o;
    vars = vs;
  }
};

class AllocList {
 private:
  list<AllocListElem> ls;
 public:
  void add(string name, int loc, int sz) {
    list<AllocListElem>::iterator i;
    for(i=ls.begin(); ; i++) {
      if (i == ls.end()) {
	list<VarRef> vs;
	vs.push_back(VarRef(name, loc, sz));
        ls.push_back(AllocListElem(sz, TRUE, vs));
	break;
      } else if (!i->occupied) {
	i->max = max(sz, i->max);
	i->occupied = TRUE;
	i->vars.push_back(VarRef(name, loc, sz));
	break;
      }
    }
  }
  
  void remove(string name) {
    list<AllocListElem>::iterator i;
    list<VarRef>::iterator j;
    for(i=ls.begin(); i != ls.end(); i++) {
      for(j = i->vars.begin(); j != i->vars.end(); j++) {
	if (j->name == name) {
	  i->occupied = FALSE;
	  return;
	}
      }
    }
    err("AllocList::remove: item not found");
  }

  list<AllocListElem> get() {
    return ls;
  }
};


#if 0
// Sorted sequence of the lengths of lists currently allocated
class SortedIntList {
private:
  list<int> ls;
public:

  void add(int n) {
    list<int>::iterator i;
    for(i = ls.begin(); i != ls.end() && *i < n; i++)
      ;
    ls.insert(i,n);
  }

  void remove(int n) {
    if (ls.empty()) {
      err("SortedIntList::remove: list empty");
    }
    list<int>::iterator i;
    for(i = ls.begin(); *i != n; i++) {
      if (i == ls.end()) {
	err("SortedIntList::remove: item not found");
      }
    }
    ls.erase(i);
  }
  
  list<int> get() {
    return ls;
  }
  
  void set_max(list<int> new_ls) {
    list<int>::iterator p, q;
    for(p = ls.begin(), q = new_ls.begin();
	p != ls.end();
	p++, q++) {
      if (q == new_ls.end()) {
	new_ls.insert(q, *p);
      } else {
	*q = max(*q,*p);
      }
    }
  }
  
  void print() {
    list<int>::iterator i;    
    for(i = ls.begin(); i != ls.end(); i++) {
      cout << *i << " ";
    }
    cout << "\n";
  }
  
  SortedIntList() {
  }
};

#endif

//!  Expression is a struct returned by the op_ functions representing a
//!  subexpression in the output.
//!  var = the name of the variable storing the expression
//!  is_dep = whether the expression depends on the current
//!           environment. If false, the expression can be hoisted out
//!           of an f-function.
//!  is_visible = whether the expression should be printed if it
//!               appears at top level in R.
//!  is_alloc = whether the expression is locally allocated.
//!  del_text = code to clean up after the final use of the
//!             expression. Most commonly a call to UNPROTECT_PTR.
struct Expression {
  string var;
  bool is_dep;
  visibility is_visible;
  bool is_alloc;
  string del_text;
  Expression() {}
  Expression(string v, bool d, visibility vis, string dt) {
    var = v;
    is_dep = d;
    is_visible = vis;
    is_alloc = FALSE;
    del_text = dt;
  }
};

class SubexpBuffer {
protected:
  const string prefix;
  static unsigned int n;  // see also definition immediately
			  // following class definition
  unsigned int prot;
  AllocList alloc_list;
  string edefs;
public:
  virtual void finalize() { };
  const string &output_decls() { return decls; }
  const string &output_defs() { return edefs; }
  string decls;
  SubexpBuffer * encl_fn;
  bool has_i;
  const bool is_const;
  virtual void append_defs(string s) {
    edefs += s;
  }
  virtual string new_var() {
    prot++;
    return new_var_unp();
  }
  virtual string new_var_unp() {
    return prefix + i_to_s(SubexpBuffer::n++);
  }
  virtual string new_var_unp_name(string name) {
    return prefix + i_to_s(SubexpBuffer::n++) + "_" + make_c_id(name);
  }
  int get_n_vars() { return n; }
  int get_n_prot() { return prot; }
  string new_sexp() {
    string str = new_var();
    if (is_const) {
      decls += "static SEXP " + str + ";\n";
    } else {
      decls += "SEXP " + str + ";\n";
    }
    return str;
  }
  string new_sexp_unp() {
    string str = new_var_unp();
    if (is_const) {
      decls += "static SEXP " + str + ";\n";
    } else {
      decls += "SEXP " + str + ";\n";
    }
    return str;
  }
  string new_sexp_unp_name(string name) {
    string str = new_var_unp_name(name);
    if (is_const) {
      decls += "static SEXP " + str + ";\n";
    } else {
      decls += "SEXP " + str + ";\n";
    }
    return str;
  }

  string protect_str (string str) {
    prot++;
    return "PROTECT(" + str + ")";
  }

  void appl(string var, bool do_protect, string func, int argc, ...) {
    va_list param_pt;
    string stmt;
    
    stmt = var + " = " + func + "(";
    va_start (param_pt, argc);
    for (int i = 0; i < argc; i++) {
      if (i > 0) stmt += ", ";
      stmt += *va_arg(param_pt, string *);
    }
    stmt += ")";
    string defs;
    if (do_protect) {
      defs += protect_str(stmt) + ";\n";
    }
    else
      defs += stmt + ";\n";
    append_defs(defs);
  }

  /* Convenient macro-like things for outputting function applications */
  
  string appl1(string func, string arg) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 1, &arg);
    return var;
  }
  
  string appl1_unp(string func, string arg) {
    string var = new_sexp_unp();
    appl (var, FALSE, func, 1, &arg);
    return var;
  }
  
  string appl2(string func, string arg1, string arg2) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 2, &arg1, &arg2);
    return var;
  }
  
  string appl2_unp(string func, string arg1, string arg2) {
    string var = new_sexp_unp();
    appl (var, FALSE, func, 2, &arg1, &arg2);
    return var;
  }
  
  string appl3(string func, string arg1, string arg2, string arg3) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 3, &arg1, &arg2, &arg3);
    return var;
  }

  string appl3_unp(string func, string arg1, string arg2, string arg3) {
    string var = new_sexp_unp();
    appl (var, FALSE, func, 3, &arg1, &arg2, &arg3);
    return var;
  }

  string appl4(string func,
               string arg1, 
	       string arg2, 
	       string arg3, 
	       string arg4) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 4, &arg1, &arg2, &arg3, &arg4);
    return var;
  }
  
  string appl5(string func,
               string arg1, 
	       string arg2, 
	       string arg3, 
	       string arg4,
	       string arg5) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 5, &arg1, &arg2, &arg3, &arg4, &arg5);
    return var;
  }

  string appl5_unp(string func, 
		   string arg1, 
		   string arg2, 
		   string arg3, 
		   string arg4,
		   string arg5) {
    string var = new_sexp_unp();
    appl (var, FALSE, func, 5, &arg1, &arg2, &arg3, &arg4, &arg5);
    return var;
  }
  
  string appl6(string func,
	       string arg1,
	       string arg2,
	       string arg3,
	       string arg4,
	       string arg5,
	       string arg6) {
    string var = new_sexp_unp();
    appl (var, TRUE, func, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
    return var;
  }

  void del(Expression exp) {
    append_defs(exp.del_text);
    if (exp.is_alloc) {
      alloc_list.remove(exp.var);
    }
  }

  Expression op_exp(SEXP e, string rho, bool primFuncArg = FALSE);
  Expression op_primsxp(SEXP e, string rho);
  Expression op_symlist(SEXP e, string rho);
  Expression op_lang(SEXP e, string rho);
  Expression op_promise(SEXP e);
  Expression op_begin(SEXP exp, string rho);
  Expression op_if(SEXP e, string rho);
  Expression op_for(SEXP e, string rho);
  Expression op_while(SEXP e, string rho);
  Expression op_c_return(SEXP e, string rho);
  Expression op_fundef(SEXP e, string rho, string opt_R_name = "");
  Expression op_special(SEXP e, SEXP op, string rho);
  Expression op_builtin(SEXP e, SEXP op, string rho);
  Expression op_set(SEXP e, SEXP op, string rho);
  Expression op_subscriptset(SEXP e, string rho);
  Expression op_clos_app(Expression op1, SEXP args, string rho);
  Expression op_arglist(SEXP e, string rho);
  Expression op_arglist_local(SEXP e, string rho);
  Expression op_literal(SEXP e, string rho);
  Expression op_list_local(SEXP e, string rho, bool literal = TRUE, 
			   bool primFuncArgList = FALSE, string opt_l_car = "");
  Expression op_list(SEXP e, string rho, bool literal, bool primFuncArgList = FALSE);
  Expression op_list_help(SEXP e, string rho, 
			  SubexpBuffer & consts, 
			  string & out_const, bool literal);
  Expression op_string(SEXP s);
  Expression op_vector(SEXP e);
  string output();
  void output_ip();
  SubexpBuffer new_sb() {
    SubexpBuffer new_sb;
    new_sb.encl_fn = encl_fn;
    return new_sb;
  }
  SubexpBuffer new_sb(string pref) {
    SubexpBuffer new_sb(pref);
    new_sb.encl_fn = encl_fn;
    return new_sb;
  }
  SubexpBuffer(string pref = "v", bool is_c = FALSE)
    : prefix(pref), is_const(is_c) {
    has_i = FALSE;
    prot = 0;
    decls = edefs = "";
    encl_fn = this;
  }
  virtual ~SubexpBuffer() {};
  SubexpBuffer &operator=(SubexpBuffer &sb) { return sb; }
};

unsigned int SubexpBuffer::n;

//! Huge functions are hard on compilers like gcc. To generate code
//! that goes down easy, we split up the constant initialization into
//! several functions.
class SplitSubexpBuffer : public SubexpBuffer {
private:
  const unsigned int threshold;
  const string init_str;
  unsigned int init_fns;
  string split_defs;
  void flush_defs() { 
    if (split_defs.length() > 0) {
      edefs += "\n";
      if (is_const) { decls += "static "; edefs += "static "; }
      decls += "void " + init_str + i_to_s(init_fns) + "();\n";
      edefs += "void " + init_str + i_to_s(init_fns) + "() {\n";
      edefs += indent(split_defs);
      edefs += "}\n";
      split_defs = "";
      init_fns++;
    }
  }
public:
  virtual void finalize() { flush_defs(); };

  void virtual append_defs(string d) { split_defs += d; }
  int virtual defs_location() { flush_defs(); return edefs.length(); }
  void virtual insert_def(int loc, string d) { 
    flush_defs(); edefs.insert(loc, d); 
  }
  static SplitSubexpBuffer global_constants;
  unsigned int get_n_inits() { return init_fns; }
  string get_init_str() { return init_str; }
  virtual string new_var() { prot++; return new_var_unp(); }
  virtual string new_var_unp() {
    if ((SubexpBuffer::n % threshold) == 0) flush_defs();
    return prefix + i_to_s(SubexpBuffer::n++);
  }
  virtual string new_var_unp_name(string name) {
    return new_var_unp() + "_" + make_c_id(name);
  }
  SplitSubexpBuffer(string pref = "v", bool is_c = FALSE, int thr = 300, string is = "init")
    : SubexpBuffer(pref, is_c), threshold(thr), init_str(is) {
    init_fns = 0;
  }
};

static void arg_err();
static void set_funcs(int argc, char *argv[]);
string make_fundef(SubexpBuffer * this_buf, string func_name, SEXP args, SEXP code);
string make_fundef_c(SubexpBuffer * this_buf, string func_name, SEXP args, SEXP code);
string make_symbol(SEXP e);

#endif // defined RCC_H
