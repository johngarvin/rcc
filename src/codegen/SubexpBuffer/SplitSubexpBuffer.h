#ifndef SPLIT_SUBEXP_BUFFER_H
#define SPLIT_SUBEXP_BUFFER_H

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

//! Huge functions are hard on compilers like gcc. To generate code
//! that goes down easy, we split up the constant initialization into
//! several functions.
class SplitSubexpBuffer : public SubexpBuffer {
public:
  SplitSubexpBuffer(std::string pref = "v", bool is_c = false, int thr = 300, std::string is = "init");

  static SplitSubexpBuffer global_constants;

  virtual void finalize();
  void virtual append_defs(std::string d);
  int virtual defs_location();
  void virtual insert_def(int loc, std::string d);
  unsigned int get_n_inits();
  std::string get_init_str();
  virtual std::string new_var();
  virtual std::string new_var_unp();
  virtual std::string new_var_unp_name(std::string name);

private:
  const unsigned int threshold;
  const std::string init_str;
  unsigned int init_fns;
  std::string split_defs;
  void flush_defs();
};

#endif
