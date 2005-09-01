// -*- Mode: C++ -*-
#ifndef OUTPUT_H
#define OUTPUT_H

#include <Visibility.h>

class SubexpBuffer;

//! Basic wrappers around strings. Helps a lot in getting types right
//! in the Output constructor.

class Decls {
 public:
  explicit Decls(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

class Code {
 public:
  explicit Code(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

class GDecls {
 public:
  explicit GDecls(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

class GCode {
 public:
  explicit GCode(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

class Handle {
 public:
  explicit Handle(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

class DelText {
 public:
  explicit DelText(std::string _str) : m_str(_str) {}
  const std::string get() const {return m_str;}
 private:
  const std::string m_str;
};

typedef enum {DEPENDENT, CONST} DependenceType;

class Output {
 public:
  Output(Decls _d, Code _c, GDecls _gd, GCode _gc, 
	 Handle _h, DelText _del, DependenceType _dt, VisibilityType _v);
  Output();
  static const Output & bogus;
  static const Output & nil;
  static Output global(GDecls _gd, GCode _gc, Handle _h, VisibilityType _v);
  static Output dependent(Decls _d, Code _c, Handle _h, DelText _dt, VisibilityType _v);
  static Output visible_const(Handle _h);
  static Output invisible_const(Handle _h);
  const std::string get_decls() const { return decls; }
  const std::string get_code() const { return code; }
  const std::string get_g_decls() const { return g_decls; }
  const std::string get_g_code() const { return g_code; }
  const std::string get_handle() const { return handle; }
  const std::string get_del_text() const { return del_text; }
  const DependenceType get_dependence() const { return dependence; }
  const VisibilityType get_visibility() const { return visibility; }
 private:
  std::string decls;
  std::string code;
  std::string g_decls;
  std::string g_code;
  std::string handle;
  std::string del_text;
  DependenceType dependence;
  VisibilityType visibility;
};

#endif
