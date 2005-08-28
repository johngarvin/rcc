#ifndef OUTPUT_H
#define OUTPUT_H

#include <Visibility.h>

class Decls {
 public:
  Decls(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class Code {
 public:
  Code(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class GDecls {
 public:
  GDecls(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class GCode {
 public:
  GCode(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class Handle {
 public:
  Handle(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class DelText {
 public:
  DelText(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

typedef enum {DEP, CONST} dependence;

class Output {
 public:
  Output(Decls _d, Code _c, GDecls _gd, GCode _gc, 
	 Handle _h, DelText _dt, dependence _id, visibility _v)
    : decls(_d.get()), code(_c.get()), g_decls(_gd.get()), g_code(_gc.get()), 
    handle(_h.get()), del_text(_dt.get()), is_dep(_id), is_visible(_v)
    {};
  static const Output & bogus;
  static Output global(GDecls _gd, GCode _gc, Handle _h, visibility _v);
  static Output text_const(Handle _h);
  const std::string get_decls() { return decls; }
  const std::string get_code() { return code; }
  const std::string get_g_decls() { return g_decls; }
  const std::string get_g_code() { return g_code; }
  const std::string get_handle() { return handle; }
  const std::string get_del_text() { return del_text; }
  const dependence get_is_dep() { return is_dep; }
  const visibility get_is_visible() { return is_visible; }
 private:
  const std::string decls;
  const std::string code;
  const std::string g_decls;
  const std::string g_code;
  const std::string handle;
  const std::string del_text;
  const dependence is_dep;
  const visibility is_visible;
};

#endif
