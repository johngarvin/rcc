#ifndef SUBEXP_BUFFER_H
#define SUBEXP_BUFFER_H

#include <string>

#include <Output.h>

#include <include/R/R_RInternals.h>

#include <codegen/SubexpBuffer/Expression.h>


class SubexpBuffer {
public:
  SubexpBuffer(std::string pref = "v", bool is_c = false);
  virtual ~SubexpBuffer();
  SubexpBuffer &operator=(SubexpBuffer &sb);

  std::string decls;
  SubexpBuffer * encl_fn;
  bool has_i;
  const bool is_const;

  virtual void finalize();
  const std::string &output_decls();
  const std::string &output_defs();
  virtual void append_decls(std::string s);
  virtual void append_defs(std::string s);
  virtual std::string new_var();
  virtual std::string new_var_unp();
  virtual std::string new_var_unp_name(std::string name);
  int get_n_vars();
  int get_n_prot();
  std::string new_sexp();
  std::string new_sexp_unp();
  std::string new_sexp_unp_name(std::string name);
  std::string protect_str(std::string str);
  void del(Expression exp);
  SubexpBuffer new_sb(std::string pref);
  std::string output();
  void output_ip();

  void appl(std::string var, bool do_protect, std::string func, int argc, ...);
  std::string SubexpBuffer::appl1(std::string func, std::string arg);
  std::string SubexpBuffer::appl1_unp(std::string func, std::string arg);
  std::string SubexpBuffer::appl2(std::string func, std::string arg1, std::string arg2);
  std::string SubexpBuffer::appl2_unp(std::string func, std::string arg1, std::string arg2);
  std::string SubexpBuffer::appl3(std::string func, std::string arg1, std::string arg2, std::string arg3);
  std::string SubexpBuffer::appl3_unp(std::string func, std::string arg1, std::string arg2, std::string arg3);
  std::string SubexpBuffer::appl4(std::string func,
				  std::string arg1, 
				  std::string arg2, 
				  std::string arg3, 
				  std::string arg4);
  std::string SubexpBuffer::appl5(std::string func,
				  std::string arg1, 
				  std::string arg2, 
				  std::string arg3, 
				  std::string arg4,
				  std::string arg5);
  std::string SubexpBuffer::appl5_unp(std::string func, 
				      std::string arg1, 
				      std::string arg2, 
				      std::string arg3, 
				      std::string arg4,
				      std::string arg5);
  std::string SubexpBuffer::appl6(std::string func,
				  std::string arg1,
				  std::string arg2,
				  std::string arg3,
				  std::string arg4,
				  std::string arg5,
				  std::string arg6);

  Expression op_exp(SEXP cell, std::string rho, bool fullyEvaluatedResult = false);
  Expression op_primsxp(SEXP e, std::string rho);
  Expression op_symlist(SEXP e, std::string rho);
  Expression op_lang(SEXP e, std::string rho);
  Expression op_promise(SEXP e);
  Expression op_begin(SEXP exp, std::string rho);
  Expression op_if(SEXP e, std::string rho);
  Expression op_for(SEXP e, std::string rho);
  Expression op_while(SEXP e, std::string rho);
  Expression op_return(SEXP e, std::string rho);
  Expression op_break(SEXP e, std::string rho);
  Expression op_fundef(SEXP e, std::string rho, std::string opt_R_name = "");
  Expression op_special(SEXP e, SEXP op, std::string rho);
  Expression op_builtin(SEXP e, SEXP op, std::string rho);
  Expression op_set(SEXP e, SEXP op, std::string rho);
  Expression op_subscriptset(SEXP e, std::string rho);
  Expression op_clos_app(Expression op1, SEXP args, std::string rho);
  Expression op_arglist(SEXP e, std::string rho);
  Expression op_literal(SEXP e, std::string rho);
  Expression op_list_local(SEXP e, std::string rho, bool literal = TRUE, 
			   bool fullyEvaluatedResult = FALSE, std::string opt_l_car = "");
  Expression op_list(SEXP e, std::string rho, bool literal, bool fullyEvaluatedResult = FALSE);
  Expression op_list_help(SEXP e, std::string rho, 
			  SubexpBuffer & consts, 
			  std::string & out_const, bool literal);
  Expression op_string(SEXP s);
  Expression op_vector(SEXP e);

  //! Convert an Output into an Expression. Will go away as soon as
  //! everything uses Output instead of Expression.
  const Expression output_to_expression(const Output op);
  const std::string get_prefix() { return prefix; }

protected:
  const std::string prefix;
  static unsigned int n;
  unsigned int prot;
  std::string edefs;
  static unsigned int global_temps;
};

#endif
