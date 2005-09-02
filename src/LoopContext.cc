#include <LoopContext.h>
#include <support/StringUtils.h>

using namespace std;

unsigned int LoopContext::mContextId = 0;
LoopContext* LoopContext::top = NULL;

LoopContext *LoopContext::Top()
{
  return top;
}

LoopContext::LoopContext()
{
  mContextName = "loop_" + i_to_s(mContextId++);

  // link with chain of enclosing contexts 
  enclosing = top;
  top = this;
}

LoopContext::~LoopContext()
{
  top = enclosing; 
}

string LoopContext::breakLabel()
{
  return "break_" + mContextName; 
}
  
string LoopContext::doBreak()
{
  return "goto " + breakLabel();
}
