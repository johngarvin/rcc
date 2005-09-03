#include <support/StringUtils.h>

#include "SplitSubexpBuffer.h"

//--------------------------------------------------------------------
// SplitSubexpBuffer methods
//--------------------------------------------------------------------

SplitSubexpBuffer::SplitSubexpBuffer(std::string pref /* = "v" */, 
				     bool is_c        /* = FALSE */,
				     int thr          /* = 300 */,
				     std::string is   /* = "init" */)
  : SubexpBuffer(pref, is_c), threshold(thr), init_str(is) {
  init_fns = 0;
}

void SplitSubexpBuffer::finalize() {
  flush_defs();
}

void SplitSubexpBuffer::append_defs(std::string d) {
  split_defs += d;
}

int SplitSubexpBuffer::defs_location() {
  flush_defs();
  return edefs.length();
}

void SplitSubexpBuffer::insert_def(int loc, std::string d) { 
  flush_defs();
  edefs.insert(loc, d); 
}

unsigned int SplitSubexpBuffer::get_n_inits() {
  return init_fns;
}

std::string SplitSubexpBuffer::get_init_str() {
  return init_str;
}

std::string SplitSubexpBuffer::new_var() {
  prot++;
  return new_var_unp();
}

std::string SplitSubexpBuffer::new_var_unp() {
  if ((SubexpBuffer::n % threshold) == 0) {
    flush_defs();
  }
  return prefix + i_to_s(SubexpBuffer::n++);
}

std::string SplitSubexpBuffer::new_var_unp_name(std::string name) {
  return new_var_unp() + "_" + make_c_id(name);
}

void SplitSubexpBuffer::flush_defs() { 
  if (split_defs.length() > 0) {
    edefs += "\n";
    if (is_const) {
      decls += "static ";
      edefs += "static ";
    }
    decls += "void " + init_str + i_to_s(init_fns) + "();\n";
    edefs += "void " + init_str + i_to_s(init_fns) + "() {\n";
    edefs += indent(split_defs);
    edefs += "}\n";
    split_defs = "";
    init_fns++;
  }
}
