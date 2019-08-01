//===--- ParsePragma.h - Intel Pragmas --------------------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#if INTEL_CUSTOMIZATION
// #pragma inline
class PragmaInlineHandler: public PragmaHandler {
  public:
    explicit PragmaInlineHandler(const char *name) : PragmaHandler(name) {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducer Introducer, Token &FirstTok);
};

// #pragma block_loop
class PragmaBlockLoopHandler : public PragmaHandler {
  public:
    PragmaBlockLoopHandler(const char *name) : PragmaHandler(name) {}
    virtual void HandlePragma(Preprocessor &PP, PragmaIntroducer Introducer,
                              Token &Tok);
};
#endif // INTEL_CUSTOMIZATION
