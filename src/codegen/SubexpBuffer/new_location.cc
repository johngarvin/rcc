#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

/// Return a new location (pointer to a slot in the environment).
string SubexpBuffer::new_location() {
  string location = ParseInfo::global_constants->new_var_unp();
  string decl = "static R_varloc_t " + location + ";\n";
  ParseInfo::global_constants->append_decls(decl);
  return location;
}
