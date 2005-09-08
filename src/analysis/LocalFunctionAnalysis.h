// -*- Mode:C++ -*-
#ifndef LOCAL_FUNCTION_ANALYSIS_H
#define LOCAL_FUNCTION_ANALYSIS_H

#include <include/R/R_RInternals.h>

//! LocalFunctionAnalysis
//!
//! Discovers local information about the given function
//! definition. Adds annotations to the global AnalysisResults.
class LocalFunctionAnalysis {
public:
  static void perform_analysis(const SEXP fundef);
};

#endif
