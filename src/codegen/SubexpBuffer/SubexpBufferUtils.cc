#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/StringUtils.h>

void SubexpBuffer::finalize() {}

const std::string &SubexpBuffer::output_decls() {
  return decls;
}

const std::string &SubexpBuffer::output_defs() {
  return edefs;
}

void SubexpBuffer::append_decls(std::string s) {
  decls += s;
}

void SubexpBuffer::append_defs(std::string s) {
  edefs += s;
}

std::string SubexpBuffer::new_var() {
  prot++;
  return new_var_unp();
}

std::string SubexpBuffer::new_var_unp() {
  return prefix + i_to_s(SubexpBuffer::n++);
}

std::string SubexpBuffer::new_var_unp_name(std::string name) {
  return prefix + i_to_s(SubexpBuffer::n++) + "_" + make_c_id(name);
}

int SubexpBuffer::get_n_vars() {
  return n;
}

int SubexpBuffer::get_n_prot() {
  return prot;
}

std::string SubexpBuffer::new_sexp() {
  std::string str = new_var();
  if (is_const) {
    decls += "static SEXP " + str + ";\n";
  } else {
    decls += "SEXP " + str + ";\n";
  }
  return str;
}

std::string SubexpBuffer::new_sexp_unp() {
  std::string str = new_var_unp();
  if (is_const) {
    decls += "static SEXP " + str + ";\n";
  } else {
    decls += "SEXP " + str + ";\n";
  }
  return str;
}

std::string SubexpBuffer::new_sexp_unp_name(std::string name) {
  std::string str = new_var_unp_name(name);
  if (is_const) {
    decls += "static SEXP " + str + ";\n";
  } else {
    decls += "SEXP " + str + ";\n";
  }
  return str;
}

std::string SubexpBuffer::protect_str(std::string str) {
  prot++;
  return "PROTECT(" + str + ")";
}

void SubexpBuffer::del(Expression exp) {
  append_defs(exp.del_text);
#if 0
  if (exp.is_alloc) {
    alloc_list.remove(exp.var);
  }
#endif
}

SubexpBuffer SubexpBuffer::new_sb(std::string pref) {
  SubexpBuffer new_sb(pref + "_" + i_to_s(global_temps++) + "_");
  new_sb.encl_fn = encl_fn;
  return new_sb;
}

std::string SubexpBuffer::output() {
  std::string out;
  output_ip();
  finalize();
  out += output_decls();
  out += output_defs();
  return out;
}

void SubexpBuffer::output_ip() {
#if 0
  int i;
  static int id;
  list<AllocListElem> ls = alloc_list.get();
  list<AllocListElem>::iterator p;
  list<VarRef>::iterator q;
  int offset = 0;
  for(p = ls.begin(), i=0; p != ls.end(); p++, i++) {
    std::string var = "mem" + i_to_s(i) + "_" + i_to_s(id++);
    encl_fn->decls += "SEXP " + var + ";\n";
    std::string alloc = var + "= alloca(" + i_to_s(p->max) + "*sizeof(SEXPREC));\n";
    alloc += "my_init_memory(" + var + ", " + i_to_s(p->max) + ");\n";
    encl_fn->defs.insert(0, alloc);
    if (encl_fn == this) offset += alloc.length();
    for(q = p->vars.begin(); q != p->vars.end(); q++) {
      std::string ref = q->name + " = " + var + "+" + i_to_s(q->size - 1) + ";\n";
      defs.insert(q->location + offset, ref);
      offset += ref.length();
    }
  }
#endif
}

// Outputting function applications
  
void SubexpBuffer::appl(std::string var, Protection do_protect, std::string func, int argc, ...) {
  va_list param_pt;
  std::string stmt;
    
  stmt = var + " = " + func + "(";
  va_start (param_pt, argc);
  for (int i = 0; i < argc; i++) {
    if (i > 0) stmt += ", ";
    stmt += *va_arg(param_pt, std::string *);
  }
  stmt += ")";
  std::string defs;
  if (do_protect == Protected) {
    defs += protect_str(stmt) + ";\n";
  }
  else
    defs += stmt + ";\n";
  append_defs(defs);
}

std::string SubexpBuffer::appl1(std::string func, 
				std::string arg,
				Protection resultProtection)
{
  std::string var = new_sexp_unp();
  appl (var, resultProtection, func, 1, &arg);
  return var;
}
  
std::string SubexpBuffer::appl2(std::string func, 
				std::string arg1, 
				std::string arg2,
				Protection resultProtection)
{
  std::string var = new_sexp_unp();
  appl (var, resultProtection, func, 2, &arg1, &arg2);
  return var;
}
  
std::string SubexpBuffer::appl3(std::string func, 
				std::string arg1, 
				std::string arg2, 
				std::string arg3,
				Protection resultProtection) {
  std::string var = new_sexp_unp();
  appl (var, resultProtection, func, 3, &arg1, &arg2, &arg3);
  return var;
}

std::string SubexpBuffer::appl4(std::string func,
				std::string arg1, 
				std::string arg2, 
				std::string arg3, 
				std::string arg4,
				Protection resultProtection) 
{
  std::string var = new_sexp_unp();
  appl (var, resultProtection, func, 4, &arg1, &arg2, &arg3, &arg4);
  return var;
}
  
std::string SubexpBuffer::appl5(std::string func,
				std::string arg1, 
				std::string arg2, 
				std::string arg3, 
				std::string arg4,
				std::string arg5,
				Protection resultProtection) 
{
  std::string var = new_sexp_unp();
  appl (var, resultProtection, func, 5, &arg1, &arg2, &arg3, &arg4, &arg5);
  return var;
}
