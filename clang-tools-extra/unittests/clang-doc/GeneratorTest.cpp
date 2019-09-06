//===-- clang-doc/GeneratorTest.cpp ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ClangDocTest.h"
#include "Generators.h"
#include "Representation.h"
#include "Serialize.h"
#include "gtest/gtest.h"

namespace clang {
namespace doc {

TEST(GeneratorTest, emitIndex) {
  Index Idx;
  auto InfoA = llvm::make_unique<Info>();
  InfoA->Name = "A";
  InfoA->USR = serialize::hashUSR("1");
  Generator::addInfoToIndex(Idx, InfoA.get());
  auto InfoC = llvm::make_unique<Info>();
  InfoC->Name = "C";
  InfoC->USR = serialize::hashUSR("3");
  Reference RefB = Reference("B");
  RefB.USR = serialize::hashUSR("2");
  InfoC->Namespace = {std::move(RefB)};
  Generator::addInfoToIndex(Idx, InfoC.get());
  auto InfoD = llvm::make_unique<Info>();
  InfoD->Name = "D";
  InfoD->USR = serialize::hashUSR("4");
  auto InfoF = llvm::make_unique<Info>();
  InfoF->Name = "F";
  InfoF->USR = serialize::hashUSR("6");
  Reference RefD = Reference("D");
  RefD.USR = serialize::hashUSR("4");
  Reference RefE = Reference("E");
  RefE.USR = serialize::hashUSR("5");
  InfoF->Namespace = {std::move(RefE), std::move(RefD)};
  Generator::addInfoToIndex(Idx, InfoF.get());
  auto InfoG = llvm::make_unique<Info>(InfoType::IT_namespace);
  Generator::addInfoToIndex(Idx, InfoG.get());

  Index ExpectedIdx;
  Index IndexA;
  IndexA.Name = "A";
  ExpectedIdx.Children.emplace_back(std::move(IndexA));
  Index IndexB;
  IndexB.Name = "B";
  Index IndexC;
  IndexC.Name = "C";
  IndexB.Children.emplace_back(std::move(IndexC));
  ExpectedIdx.Children.emplace_back(std::move(IndexB));
  Index IndexD;
  IndexD.Name = "D";
  Index IndexE;
  IndexE.Name = "E";
  Index IndexF;
  IndexF.Name = "F";
  IndexE.Children.emplace_back(std::move(IndexF));
  IndexD.Children.emplace_back(std::move(IndexE));
  ExpectedIdx.Children.emplace_back(std::move(IndexD));
  Index IndexG;
  IndexG.Name = "GlobalNamespace";
  IndexG.RefType = InfoType::IT_namespace;
  ExpectedIdx.Children.emplace_back(std::move(IndexG));

  CheckIndex(ExpectedIdx, Idx);
}

} // namespace doc
} // namespace clang
