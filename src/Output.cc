#include "Output.h"

const Output & Output::bogus = Output(Decls(""), Code(""), GDecls(""), GCode(""),
				    Handle(""), DelText(""), CONST, INVISIBLE);

Output Output::global(GDecls _gd, GCode _gc, Handle _h, visibility _v) {
  return Output::Output(Decls(""), Code(""), _gd, _gc, _h, DelText(""), CONST, _v);
}

Output Output::text_const(Handle _h) {
  return Output::Output(Decls(""), Code(""), GDecls(""), GCode(""),
			_h, DelText(""), CONST, VISIBLE);
}
