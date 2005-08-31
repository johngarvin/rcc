#ifndef OUTPUT_H
#define OUTPUT_H

#include <Visibility.h>

class SubexpBuffer;

//! Basic wrappers around strings. Helps a lot in getting types right
//! in the Output constructor.

class Decls {
 public:
  explicit Decls(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class Code {
 public:
  explicit Code(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class GDecls {
 public:
  explicit GDecls(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class GCode {
 public:
  explicit GCode(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class Handle {
 public:
  explicit Handle(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

class DelText {
 public:
  explicit DelText(std::string _str) : m_str(_str) {}
  std::string get() {return m_str;}
 private:
  std::string m_str;
};

typedef enum {DEPENDENT, CONST} DependenceType;

class Output {
 public:
  Output(Decls _d, Code _c, GDecls _gd, GCode _gc, 
	 Handle _h, DelText _del, DependenceType _dt, VisibilityType _v)
    : decls(_d.get()), code(_c.get()), g_decls(_gd.get()), g_code(_gc.get()), 
    handle(_h.get()), del_text(_del.get()), dependence(_dt), visibility(_v)
    {};
  static const Output & bogus;
  static const Output & nil;
  static Output global(GDecls _gd, GCode _gc, Handle _h, VisibilityType _v);
  static Output dependent(Decls _d, Code _c, Handle _h, DelText _dt, VisibilityType _v);
  static Output text_const(Handle _h);
  const std::string get_decls() { return decls; }
  const std::string get_code() { return code; }
  const std::string get_g_decls() { return g_decls; }
  const std::string get_g_code() { return g_code; }
  const std::string get_handle() { return handle; }
  const std::string get_del_text() { return del_text; }
  const DependenceType get_dependence() { return dependence; }
  const VisibilityType get_visibility() { return visibility; }
 private:
  const std::string decls;
  const std::string code;
  const std::string g_decls;
  const std::string g_code;
  const std::string handle;
  const std::string del_text;
  const DependenceType dependence;
  const VisibilityType visibility;
};

#endif
