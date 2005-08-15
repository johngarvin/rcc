/* Copyright (c) 2005 John Garvin 
 *
 * May 27, 2005
 *
 * RCC uses macros to output common patterns.
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

class MacroFactory {
public:
  static MacroFactory * Instance();
protected:
  MacroFactory();
  virtual ~MacroFactory() {}
private:
  static MacroFactory _instance;
  const Macro const &getMacro(const string name);
  map<string,Macro> macro_map;
};

class Macro {
public:
  Macro(const string b, const list<string> f, const int n) 
    : body(b),
      formals(f),
      nargs(n)
  {
    // empty
  };
  virtual ~Macro();
  string call(const int nargs, const list<string> args);
private:
  const string body;
  const list<string> formals;
  const int nargs;
};

// Example:
// MacroFactory mf(MACRO_FILE);
// prim_mac = mf.getMacro("primsxp");
// list_mac = mf.getMacro("list");

// defs += prim_mac.call(gensym(), "foo+1", "0", "name");

// List example:
// list_mac.call(make_list(foo));

// BEGIN MACRO primsxp $ret$ $val$ $builtin$ $name$
// PROTECT($ret$ = mkPRIMSXP($val$,$builtin$)); /* $name$ */
// END MACRO

//    defs += "PROTECT(" + var
//      + " = mkPRIMSXP(" + i_to_s(PRIMOFFSET(e)) 
//      + "," + i_to_s(is_builtin) + "));"
//      + " /* " + string(PRIMNAME(e)) + " */\n";


//   {
//     SEXP tmp_ls_0_7;
//     SEXP tmp_ls_0_8;
//     SEXP tmp_ls_0_9;
//     SEXP tmp_ls_0_10;
//     SEXP tmp_ls_0_11;
//     SEXP tmp_ls_0_12;
//     tmp_ls_0_7 = cons(c6, R_NilValue);
//     tmp_ls_0_8 = cons(c5, tmp_ls_0_7);
//     tmp_ls_0_9 = cons(c4, tmp_ls_0_8);
//     tmp_ls_0_10 = cons(c3, tmp_ls_0_9);
//     tmp_ls_0_11 = cons(c2, tmp_ls_0_10);
//     tmp_ls_0_12 = cons(c1, tmp_ls_0_11);
//     PROTECT(c13 = tmp_ls_0_12);
//   }

BEGIN LISTMACRO list $tmp$ $data$
$tmp$0$ = cons($data$n-1$, R_NilValue);
FOR i = 1 to n-1
$tmp$i$ = cons($data$n-i$, $tmp$i-1$);
END FOR
PROTECT($v$ = $tmp$n-1$);
END LISTMACRO

BEGIN MACRO cons $v$ $car$ $cdr$
$v$ = cons($car$, $cdr$);
END MACRO
