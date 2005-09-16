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
  const std::string decls() const { return m_decls; }
  const std::string code() const { return m_code; }
  const std::string g_decls() const { return m_g_decls; }
  const std::string g_code() const { return m_g_code; }
  const std::string handle() const { return m_handle; }
  const std::string del_text() const { return m_del_text; }
  const DependenceType dependence() const { return m_dependence; }
  const VisibilityType visibility() const { return m_visibility; }
 private:
  std::string m_decls;
  std::string m_code;
  std::string m_g_decls;
  std::string m_g_code;
  std::string m_handle;
  std::string m_del_text;
  DependenceType m_dependence;
  VisibilityType m_visibility;
};

#endif
