#include "visibility.h"
#include "codegen.h"

using namespace std;

const string Visibility::R_VISIBILITY_GLOBAL_VAR = "R_Visible";

string Visibility::emit_check_expn()
{
  string vstring = R_VISIBILITY_GLOBAL_VAR;
  return vstring;
}

string Visibility::emit_set_if_visible(visibility vis)
{
  string vstring;
  if (vis == VISIBLE) vstring = emit_set(vis);
  return vstring;
}

string Visibility::emit_set(visibility vis)
{
  string vstring;
  if (vis != CHECK_VISIBLE) {
    vstring += emit_assign(R_VISIBILITY_GLOBAL_VAR, 
			   (vis == INVISIBLE ? "0" : "1"));
  }
  return vstring;
}
