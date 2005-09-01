#include <Output.h>

// big constructor that sets everything
Output::Output(Decls _d,
	       Code _c,
	       GDecls _gd,
	       GCode _gc, 
	       Handle _h,
	       DelText _del,
	       DependenceType _dt,
	       VisibilityType _v)
    : decls(_d.get()),
      code(_c.get()),
      g_decls(_gd.get()),
      g_code(_gc.get()), 
      handle(_h.get()),
      del_text(_del.get()),
      dependence(_dt),
      visibility(_v)
  {}

Output::Output()
  : decls(""),
    code(""),
    g_decls(""),
    g_code(""),
    handle(""),
    del_text(""),
    dependence(CONST),
    visibility(INVISIBLE)
  {}

const Output & Output::bogus = Output(Decls(""), Code(""),
				      GDecls(""), GCode(""),
				      Handle(""), DelText(""),
				      CONST, INVISIBLE);

const Output & Output::nil = Output(Decls(""), Code(""),
				    GDecls(""), GCode(""),
				    Handle("R_NilValue"), DelText(""),
				    CONST, INVISIBLE);

Output Output::global(GDecls _gd, GCode _gc, Handle _h, VisibilityType _v) {
  return Output::Output(Decls(""), Code(""), _gd, _gc, _h, DelText(""), CONST, _v);
}

Output Output::dependent(Decls _d, Code _c, Handle _h, DelText _dt, VisibilityType _v) {
  return Output::Output(_d, _c, GDecls(""), GCode(""), _h, _dt, DEPENDENT, _v);
}

Output Output::visible_const(Handle _h) {
  return Output::Output(Decls(""), Code(""), GDecls(""), GCode(""),
			_h, DelText(""), CONST, VISIBLE);
}

Output Output::invisible_const(Handle _h) {
  return Output::Output(Decls(""), Code(""), GDecls(""), GCode(""),
			_h, DelText(""), CONST, INVISIBLE);
}

