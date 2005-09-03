#ifndef RCC_EXPRESSION_H
#define RCC_EXPRESSION_H

#include <string>

#include <Visibility.h>

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
  Expression(std::string v, bool d, VisibilityType vis, std::string dt)
    : var(v), is_dep(d), is_visible(vis), is_alloc(false), del_text(dt)
  { }
  Expression() {}
  std::string var;
  bool is_dep;
  VisibilityType is_visible;
  bool is_alloc;
  std::string del_text;
  static const Expression bogus_exp;
  static const Expression nil_exp;
};

#endif
