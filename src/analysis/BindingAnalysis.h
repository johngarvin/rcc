// -*- Mode: C++ -*-

#include <analysis/Annotation.h>

class BindingAnalysis {
public:
  BindingAnalysis(RAnnot::FuncInfo * root);
  void perform_analysis();
private:
  void find_free_mentions();
  void assign_scopes();
  const RAnnot::FuncInfo * m_root;
};
