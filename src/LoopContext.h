#ifndef LoopContext_h
#define LoopContext_h

#include <string>

class LoopContext {
public:
  static LoopContext *Top();
public:
  LoopContext();
  ~LoopContext();
  std::string breakLabel();
  std::string doBreak();

private:
  std::string mContextName;
  LoopContext *enclosing;

private:
  static unsigned int mContextId;
  static LoopContext *top; 
};

#endif
