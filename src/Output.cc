#include <Output.h>

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

Output Output::text_const(Handle _h) {
  return Output::Output(Decls(""), Code(""), GDecls(""), GCode(""),
			_h, DelText(""), CONST, VISIBLE);
}
