
/* Copyright (c) 2003 John Garvin 
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <string>

using namespace std;

extern "C" {

#include <stdarg.h>
#include <stdlib.h>
#include <sys/file.h>
#include <IOStuff.h>
#include <Parse.h>
#include "get_name.h"
#include "replacements.h"

// Prevent conflict with basicstring::{append, length} on Alpha
#undef append
#undef length

extern int Rf_initialize_R(int argc, char **argv);
extern void setup_Rmainloop(void);

}

string make_symbol(SEXP e);
string make_type(int t);
string make_fundef(string func_name, SEXP args, SEXP code);
string make_fundef_argslist(string func_name, SEXP args, SEXP code);
string indent(string str);
string i_to_s(const int i);
string d_to_s(double d);
string c_to_s(Rcomplex c);
string escape(string str);
string quote(string str);
string unp(string str);
string strip_suffix(string str);
int filename_pos(string str);
int parse_R(list<SEXP> & e, char *inFile);
void err(string message);
void printstr(string str);

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

/* Expression is a struct returned by the op_ functions representing a
 * subexpression in the output.
 *
 * var = the name of the variable storing the expression
 * is_prot = whether the expression has been PROTECTed (and thus must
 *           be UNPROTECTed).
 * is_dep = whether the expression depends on the current
 *          environment. If false, the expression can be hoisted out
 *          of an f-function.
 * is_visible = whether the expression should be printed if it
 *              apppears at top level in R.
 */
struct Expression {
  string var;
  bool is_dep;
  bool is_visible;
  bool is_alloc;
  string del_text;
  Expression() {}
  Expression(string v, bool d, bool vis, string dt) {
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
  unsigned int n;
  unsigned int prot;
  AllocList alloc_list;
public:
  bool has_i;
  string decls;
  string defs;
  virtual string new_var();
  virtual string new_var_unp();
  int get_n_vars();
  int get_n_prot();
  string new_sexp();
  string new_sexp_unp();
  string appl1(string func, string arg);
  string appl1_unp(string func, string arg);
  string appl2(string func, string arg1, string arg2);
  string appl2_unp(string func, string arg1, string arg2);
  string appl3(string func, string arg1, string arg2, string arg3);
  string appl3_unp(string func, string arg1, string arg2, string arg3);
  string appl4(string func,
	       string arg1,
	       string arg2,
	       string arg3,
	       string arg4);
  string appl5(string func,
	       string arg1,
	       string arg2,
	       string arg3,
	       string arg4,
	       string arg5);
  string appl5_unp(string func,
		   string arg1,
		   string arg2,
		   string arg3,
		   string arg4,
		   string arg5);
  string appl6(string func,
	       string appl1,
	       string appl2,
	       string appl3,
	       string appl4,
	       string appl5,
	       string appl6);
  void del(Expression exp);
  Expression op_exp(SEXP e, string rho);
  Expression op_primsxp(SEXP e, string rho);
  Expression op_symlist(SEXP e, string rho);
  Expression op_lang(SEXP e, string rho);
  Expression op_begin(SEXP exp, string rho);
  Expression op_if(SEXP e, string rho);
  Expression op_for(SEXP e, string rho);
  Expression op_while(SEXP e, string rho);
  Expression op_fundef(SEXP e, string rho);
  Expression op_special(SEXP e, SEXP op, string rho);
  Expression op_set(SEXP e, SEXP op, string rho);
  Expression op_clos_app(Expression op1, SEXP args, string rho);
  Expression op_arglist(SEXP e, string rho);
  Expression op_literal(SEXP e, string rho);
  Expression op_list_array(SEXP e, string rho, bool literal = TRUE);
  Expression op_list(SEXP e, string rho, bool literal = TRUE);
  Expression op_list_help(SEXP e, string rho, 
			  SubexpBuffer & consts, 
			  string & out_const, bool literal);
  Expression op_string(SEXP s);
  Expression op_vector(SEXP e);
  string output();
  void output_ip();
  SubexpBuffer(string pref = "v") : prefix(pref) {
    has_i = FALSE;
    n = prot = 0;
    decls = defs = "";
  }
  virtual ~SubexpBuffer() {};
};

/* Huge functions are hard on compilers like gcc. To generate code
 * that goes down easy, we split up the constant initialization into
 * several functions.
 */
class SplitSubexpBuffer : public SubexpBuffer {
private:
  const unsigned int threshold;
  const string init_str;
  unsigned int init_fns;
public:
  static SplitSubexpBuffer global_constants;
  unsigned int get_n_inits();
  string get_init_str();
  virtual string new_var();
  virtual string new_var_unp();
  SplitSubexpBuffer(string pref = "v", int thr = 500, string is = "init") 
    : SubexpBuffer(pref), threshold(thr), init_str(is) {
    init_fns = 0;
  }
};

string make_symbol(SEXP e);
string make_type(int t);
string make_fundef(string func_name, SEXP args, SEXP code);
string make_fundef_argslist(string func_name, SEXP args, SEXP code);
string indent(string str);
string i_to_s(int i);
string d_to_s(double d);
string c_to_s(Rcomplex c);
