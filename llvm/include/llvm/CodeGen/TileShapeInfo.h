//===- llvm/CodeGen/TileShapeInfo.h - ---------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file Shape utility for AMX.
/// AMX hardware requires to config the shape of tile data register before use.
/// The 2D shape includes row and column. In AMX intrinsics interface the shape
/// is passed as 1st and 2nd parameter and they are lowered as the 1st and 2nd
/// machine operand of AMX pseudo instructions. ShapeT class is to facilitate
/// tile config and register allocator. The row and column are machine operand
/// of AMX pseudo instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_TILESHAPEINFO_H
#define LLVM_CODEGEN_TILESHAPEINFO_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"

namespace llvm {

class ShapeT {
public:
  ShapeT(MachineOperand *Row, MachineOperand *Col,
         const MachineRegisterInfo *MRI = nullptr)
      : Row(Row), Col(Col) {
    if (MRI)
      deduceImm(MRI);
  }
#if INTEL_CUSTOMIZATION
  // When ShapeT has mult shapes, we only use Shapes (never use Row and Col)
  // and ImmShapes. Due to the most case is only one shape (just simply use
  // Shape.Row or Shape.Col), so here we don't merge Row and Col into vertor
  // Shapes to keep the speed and code simplicity.
  // TODO: The upper solution is a temporary way to minimize current tile
  // register allocation code changes. It can not handle both Reg shape and
  // Imm shape for different shapes (e.g. shape 1 is reg shape while shape 2
  // is imm shape). Refine me when we have more mult-tile shape instructions!
  ShapeT(ArrayRef<MachineOperand *> ShapesOperands,
         const MachineRegisterInfo *MRI = nullptr)
      : Row(nullptr), Col(nullptr), RowImm(InvalidImmShape),
        ColImm(InvalidImmShape) {
    assert(ShapesOperands.size() % 2 == 0 && "Miss row or col!");

    for(auto *Shape : ShapesOperands)
      Shapes.push_back(Shape);

    if (MRI)
      deduceImm(MRI);
  }
#endif // INTEL_CUSTOMIZATION
  ShapeT()
      : Row(nullptr), Col(nullptr), RowImm(InvalidImmShape),
        ColImm(InvalidImmShape) {}
#if INTEL_CUSTOMIZATION
  // TODO: We need to extern cmp operator for muti-shapes if
  // we have requirement in the future.
#endif // INTEL_CUSTOMIZATION
  bool operator==(const ShapeT &Shape) const {
    MachineOperand *R = Shape.Row;
    MachineOperand *C = Shape.Col;
    if (!R || !C)
      return false;
    if (!Row || !Col)
      return false;
    if (Row->getReg() == R->getReg() && Col->getReg() == C->getReg())
      return true;
    if ((RowImm != InvalidImmShape) && (ColImm != InvalidImmShape))
      return RowImm == Shape.getRowImm() && ColImm == Shape.getColImm();
    return false;
  }

  bool operator!=(const ShapeT &Shape) const { return !(*this == Shape); }

#if INTEL_CUSTOMIZATION
  MachineOperand *getRow(unsigned I = 0) const {
    if (Shapes.empty())
      return Row;
    assert(Shapes.size() / 2 >= I && "Get invalid row from id!");
    return Shapes[I * 2];
  }

  MachineOperand *getCol(unsigned I = 0) const {
    if (Shapes.empty())
      return Col;
    assert(Shapes.size() / 2 >= I && "Get invalid col from id!");
    return Shapes[I * 2 + 1];
  }

  int64_t getRowImm(unsigned I = 0) const {
    if (ImmShapes.empty())
      return RowImm;
    assert(ImmShapes.size() / 2 >= I && "Get invalid imm row from id!");
    return ImmShapes[I * 2];
  }

  int64_t getColImm(unsigned I = 0) const {
    if (ImmShapes.empty())
      return ColImm;
    assert(ImmShapes.size() / 2 >= I && "Get invalid imm col from id!");
    return ImmShapes[I * 2 + 1];
  }

  unsigned getShapeNum() {
    if (Shapes.empty())
      return isValid() ? 1 : 0;
    else
      return Shapes.size() / 2;
  }
#endif // INTEL_CUSTOMIZATION

  bool isValid() { return (Row != nullptr) && (Col != nullptr); }

  void deduceImm(const MachineRegisterInfo *MRI) {
    // All def must be the same value, otherwise it is invalid MIs.
    // Find the immediate.
    // TODO copy propagation.
    auto GetImm = [&](Register Reg) {
      int64_t Imm = InvalidImmShape;
      for (const MachineOperand &DefMO : MRI->def_operands(Reg)) {
        const auto *MI = DefMO.getParent();
        if (MI->isMoveImmediate()) {
          Imm = MI->getOperand(1).getImm();
          break;
        }
      }
      return Imm;
    };
#if INTEL_CUSTOMIZATION
    if (Shapes.empty()) { // Single Shape
      RowImm = GetImm(Row->getReg());
      ColImm = GetImm(Col->getReg());
    } else { // Multi Shapes
      for (auto *Shape : Shapes) {
        int64_t ImmShape = GetImm(Shape->getReg());
        ImmShapes.push_back(ImmShape);
      }
    }
#endif // INTEL_CUSTOMIZATION
  }

private:
  static constexpr int64_t InvalidImmShape = -1;
  MachineOperand *Row;
  MachineOperand *Col;
  int64_t RowImm = -1;
  int64_t ColImm = -1;
#if INTEL_CUSTOMIZATION
  // Multi Shapes
  SmallVector<MachineOperand *, 0> Shapes;
  SmallVector<int64_t, 0> ImmShapes;
#endif // INTEL_CUSTOMIZATION
};

} // namespace llvm

#endif
