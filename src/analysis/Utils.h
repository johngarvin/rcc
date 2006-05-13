#ifndef UTILS_H
#define UTILS_H

#include <string>

#include <include/R/R_Defn.h>

bool is_assign_prim(const SEXP e);
bool is_local_assign_prim(const SEXP e);
bool is_free_assign_prim(const SEXP e);
bool is_local_assign(const SEXP e);
bool is_free_assign(const SEXP e);
bool is_assign(const SEXP e);
bool is_simple_assign(const SEXP e);
SEXP assign_lhs_c(const SEXP e);
SEXP assign_rhs_c(const SEXP e);

bool is_fundef(const SEXP e);
SEXP fundef_args_c(const SEXP e);
SEXP fundef_body_c(const SEXP e);

bool is_struct_field(const SEXP e);
SEXP struct_field_lhs_c(const SEXP e);
SEXP struct_field_rhs_c(const SEXP e);
bool is_simple_subscript(const SEXP e);
bool is_subscript(const SEXP e);
SEXP subscript_lhs_c(const SEXP e);
SEXP subscript_rhs_c(const SEXP e);
bool is_const(const SEXP e);
bool is_var(const SEXP e);
std::string var_name(const SEXP e);
bool is_cons(const SEXP e);
bool is_string(const SEXP e);
bool is_call(const SEXP e);
SEXP call_lhs(const SEXP e);
SEXP call_args(const SEXP e);

bool is_if(const SEXP e);
SEXP if_cond_c(const SEXP e);
SEXP if_truebody_c(const SEXP e);
SEXP if_falsebody_c(const SEXP e);
bool is_return(const SEXP e);
bool is_break(const SEXP e);
bool is_stop(const SEXP e);
bool is_next(const SEXP e);

bool is_loop(const SEXP e);
bool is_loop_header(const SEXP e);
bool is_loop_increment(const SEXP e);
SEXP loop_body_c(const SEXP e);
bool is_for(const SEXP e);
SEXP for_iv_c(const SEXP e);
SEXP for_range_c(const SEXP e);
SEXP for_body_c(const SEXP e);
bool is_while(const SEXP e);
SEXP while_cond_c(const SEXP e);
SEXP while_body_c(const SEXP e);
bool is_repeat(const SEXP e);
SEXP repeat_body_c(const SEXP e);

bool is_paren_exp(const SEXP e);
SEXP paren_body_c(const SEXP e);
bool is_curly_list(const SEXP e);
SEXP curly_body(const SEXP e);

bool is_rcc_assertion(const SEXP e);
bool is_rcc_assert(const SEXP e);
bool is_rcc_assert_sym(const SEXP e);
bool is_rcc_assert_exp(const SEXP e);
bool is_value_assert(const SEXP e);

#endif
