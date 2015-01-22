//===- CanonExpr.cpp - Implements the CanonExpr class -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CanonExpr class.
//
//===----------------------------------------------------------------------===//


#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

using namespace llvm;
using namespace loopopt;

std::set< CanonExpr* >CanonExpr::Objs;

CanonExpr::CanonExpr(Type* Typ, bool Gen, int Level, int64_t Cons, int64_t Denom)
  : Ty(Typ), Generable(Gen), DefinedAtLevel(Level), Const(Cons)
  , Denominator(Denom) {

  Objs.insert(this);
}

void CanonExpr::destroy() {
  Objs.erase(this);
  delete this;
}

void CanonExpr::destroyAll() {

  for (auto &I : Objs) {
      delete I;
  }

  Objs.clear();
}

CanonExpr* CanonExpr::clone() const {
  // TODO: placeholder, implement later
  return nullptr;
}

void CanonExpr::dump() const {
  // TODO: placeholder, implement later
}

void CanonExpr::print() const {
  // TODO: placeholder, implement later
}

