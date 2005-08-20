#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <string>

typedef enum {INVISIBLE, VISIBLE, CHECK_VISIBLE} visibility;

class Visibility {
public:
  static std::string emit_set(visibility vis);
  static std::string emit_set_if_visible(visibility vis);
  static std::string emit_check_expn();
private:
  static const std::string R_VISIBILITY_GLOBAL_VAR;
};

#endif
