//===- MCAsmLexer.cpp - Abstract Asm Lexer Interface ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/Support/SMLoc.h"

using namespace llvm;

<<<<<<< HEAD
MCAsmLexer::MCAsmLexer() : AltMacroMode(false) {// INTEL
=======
MCAsmLexer::MCAsmLexer() : AltMacroMode(false) {
>>>>>>> 079b067df74b010544d2187e0385b396950169c4
  CurTok.emplace_back(AsmToken::Space, StringRef());
}

MCAsmLexer::~MCAsmLexer() = default;

SMLoc MCAsmLexer::getLoc() const {
  return SMLoc::getFromPointer(TokStart);
}

SMLoc AsmToken::getLoc() const {
  return SMLoc::getFromPointer(Str.data());
}

SMLoc AsmToken::getEndLoc() const {
  return SMLoc::getFromPointer(Str.data() + Str.size());
}

SMRange AsmToken::getLocRange() const {
  return SMRange(getLoc(), getEndLoc());
}
