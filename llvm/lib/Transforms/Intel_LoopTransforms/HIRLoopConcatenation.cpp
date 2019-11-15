//===--- HIRLoopConcatenation.cpp - Implements Loop Concatenation class ---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file performs special kind of loop fusion/concatenation for loops. This
// is performed for 4 or 16 sibling loops where alternate loops write and then
// read from the same alloca. It is unwieldly to list all 16 loops so I am just
// pasting the first 4.
//
// <1247>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <3>             |   %13 = (%0)[sext.i32.i64(%1) * i1];
// <5>             |   %15 = (%2)[sext.i32.i64(%3) * i1];
// <9>             |   %19 = (%0)[sext.i32.i64(%1) * i1 + 4];
// <12>            |   %22 = (%2)[sext.i32.i64(%3) * i1 + 4];
// <18>            |   %28 = (%0)[sext.i32.i64(%1) * i1 + 1];
// <21>            |   %31 = (%2)[sext.i32.i64(%3) * i1 + 1];
// <25>            |   %35 = (%0)[sext.i32.i64(%1) * i1 + 5];
// <28>            |   %38 = (%2)[sext.i32.i64(%3) * i1 + 5];
// <34>            |   %44 = (%0)[sext.i32.i64(%1) * i1 + 2];
// <37>            |   %47 = (%2)[sext.i32.i64(%3) * i1 + 2];
// <41>            |   %51 = (%0)[sext.i32.i64(%1) * i1 + 6];
// <44>            |   %54 = (%2)[sext.i32.i64(%3) * i1 + 6];
// <50>            |   %60 = (%0)[sext.i32.i64(%1) * i1 + 3];
// <53>            |   %63 = (%2)[sext.i32.i64(%3) * i1 + 3];
// <57>            |   %67 = (%0)[sext.i32.i64(%1) * i1 + 7];
// <60>            |   %70 = (%2)[sext.i32.i64(%3) * i1 + 7];
// <71>            |   (%5)[0][i1][0] = zext.i8.i32(%60) + zext.i8.i32(%44) ...
// <74>            |   (%5)[0][i1][2] = -1 * zext.i8.i32(%60) + ...
// <77>            |   (%5)[0][i1][1] = -1 * zext.i8.i32(%60) + ...
// <80>            |   (%5)[0][i1][3] = zext.i8.i32(%60) + ...
// <1247>          + END LOOP
// <1247>
// <93>            %94 = 0;
// <1248>
// <1248>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <98>            |   %96 = (%5)[0][0][i1];
// <100>           |   %98 = (%5)[0][1][i1];
// <104>           |   %102 = (%5)[0][2][i1];
// <106>           |   %104 = (%5)[0][3][i1];
// <114>           |   %112 = (%96 + %98 + %102 + %104)/u32768  &&  65537;
// <117>           |   %115 = %96 + %98 + %102 + %104 + 65535 * %112
// ^  65535 * %112;
// <119>           |   %117 = (%96 + -1 * %98 + %102 + -1 * %104)/u32768
// &&  65537;
// <122>           |   %120 = %96 + -1 * %98 + %102 + -1 * %104 + 65535 * %117
// ^  65535 * %117;
// <124>           |   %122 = (%96 + %98 + -1 * (%102 + %104))/u32768
// &&  65537;
// <127>           |   %125 = %96 + %98 + 65535 * %122 + -1 * (%102 + %104)
// ^  65535 * %122;
// <129>           |   %127 = (%96 + -1 * %98 + -1 * %102 + %104)/u32768
// &&  65537;
// <132>           |   %130 = %96 + -1 * %98 + -1 * %102 + %104 + 65535 * %127
// ^  65535 * %127;
// <136>           |   %94 = %94 + %125 + %115 + %120  +  %130;
// <1248>          + END LOOP
// <1248>
// <145>           @llvm.lifetime.end.p0i8(64,  &((i8*)(%5)[0]));
// <152>           @llvm.lifetime.start.p0i8(64,  &((i8*)(%5)[0]));
//
// <1249>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <158>           |   %150 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1))];
// <160>           |   %152 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3))];
// <164>           |   %156 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 4];
// <167>           |   %159 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 4];
// <173>           |   %165 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 1];
// <176>           |   %168 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 1];
// <180>           |   %172 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 5];
// <183>           |   %175 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 5];
// <189>           |   %181 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 2];
// <192>           |   %184 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 2];
// <196>           |   %188 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 6];
// <199>           |   %191 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 6];
// <205>           |   %197 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 3];
// <208>           |   %200 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 3];
// <212>           |   %204 = (%0)[sext.i32.i64(%1) * i1 + sext.i32.i64((4 *
// %1)) + 7];
// <215>           |   %207 = (%2)[sext.i32.i64(%3) * i1 + sext.i32.i64((4 *
// %3)) + 7];
// <226>           |   (%5)[0][i1][0] = zext.i8.i32(%197) + ...
// <229>           |   (%5)[0][i1][2] = -1 * zext.i8.i32(%197) + ...
// <232>           |   (%5)[0][i1][1] = -1 * zext.i8.i32(%197) + ...
// <235>           |   (%5)[0][i1][3] = zext.i8.i32(%197) + ...
// <1249>          + END LOOP
// <1249>
// <250>           %233 = 0;
// <1250>
// <1250>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <255>           |   %235 = (%5)[0][0][i1];
// <257>           |   %237 = (%5)[0][1][i1];
// <261>           |   %241 = (%5)[0][2][i1];
// <263>           |   %243 = (%5)[0][3][i1];
// <271>           |   %251 = (%235 + %237 + %241 + %243)/u32768  &&  65537;
// <274>           |   %254 = %235 + %237 + %241 + %243  + 65535 * %251
// ^  65535 * %251;
// <276>           |   %256 = (%235 + -1 * %237 + %241 + -1 * %243)/u32768
// &&  65537;
// <279>           |   %259 = %235 + -1 * %237 + %241 + -1 * %243 + 65535 * %256
// ^  65535 * %256;
// <281>           |   %261 = (%235 + %237 + -1 * (%241 + %243))/u32768
// &&  65537;
// <284>           |   %264 = %235 + %237 + 65535 * %261 + -1 * (%241 + %243)
// ^  65535 * %261;
// <286>           |   %266 = (%235 + -1 * %237 + -1 * %241 + %243)/u32768
// &&  65537;
// <289>           |   %269 = %235 + -1 * %237 + -1 * %241 + %243 + 65535 * %266
// ^  65535 * %266;
// <293>           |   %233 = %233 + %264 + %254 + %259 +  %269;
// <1250>          + END LOOP
//
//
// The original alloca %5 which has the type [4 x [4 x i32]] is replaced by a
// new alloca with type [16 x [8 x i32] after the transformation. Reduction
// results are collected into allocas in the fused read loop summed up at the
// end. There are only four loops after the transformation. The first one
// represents all the loops which write to alloca. The second loop is used to
// initialize alloca created to hold reduction results. The third loop consists
// of 1st, 2nd, 5th and 6th (fused) read loops and the fourth loop contains the
// reductions.
//
// <1247>          + DO i1 = 0, 15, 1   <DO_LOOP>
// <3>             |   %13 = (%0)[sext.i32.i64(%1) * i1];
// <5>             |   %15 = (%2)[sext.i32.i64(%3) * i1];
// <9>             |   %19 = (%0)[sext.i32.i64(%1) * i1 + 4];
// <12>            |   %22 = (%2)[sext.i32.i64(%3) * i1 + 4];
// <18>            |   %28 = (%0)[sext.i32.i64(%1) * i1 + 1];
// <21>            |   %31 = (%2)[sext.i32.i64(%3) * i1 + 1];
// <25>            |   %35 = (%0)[sext.i32.i64(%1) * i1 + 5];
// <28>            |   %38 = (%2)[sext.i32.i64(%3) * i1 + 5];
// <34>            |   %44 = (%0)[sext.i32.i64(%1) * i1 + 2];
// <37>            |   %47 = (%2)[sext.i32.i64(%3) * i1 + 2];
// <41>            |   %51 = (%0)[sext.i32.i64(%1) * i1 + 6];
// <44>            |   %54 = (%2)[sext.i32.i64(%3) * i1 + 6];
// <50>            |   %60 = (%0)[sext.i32.i64(%1) * i1 + 3];
// <53>            |   %63 = (%2)[sext.i32.i64(%3) * i1 + 3];
// <57>            |   %67 = (%0)[sext.i32.i64(%1) * i1 + 7];
// <60>            |   %70 = (%2)[sext.i32.i64(%3) * i1 + 7];
// <71>            |   (%new)[0][i1][0] = zext.i8.i32(%60) + ...
// <74>            |   (%new)[0][i1][2] = -1 * zext.i8.i32(%60) + ...
// <77>            |   (%new)[0][i1][1] = -1 * zext.i8.i32(%60) + ...
// <80>            |   (%new)[0][i1][3] = zext.i8.i32(%60) + ...
// <158>           |   %150 = (%0)[sext.i32.i64(%1) * i1 + 8];
// <160>           |   %152 = (%2)[sext.i32.i64(%3) * i1 + 8];
// <164>           |   %156 = (%0)[sext.i32.i64(%1) * i1 + 12];
// <167>           |   %159 = (%2)[sext.i32.i64(%3) * i1 + 12];
// <173>           |   %165 = (%0)[sext.i32.i64(%1) * i1 + 9];
// <176>           |   %168 = (%2)[sext.i32.i64(%3) * i1 + 9];
// <180>           |   %172 = (%0)[sext.i32.i64(%1) * i1 + 13];
// <183>           |   %175 = (%2)[sext.i32.i64(%3) * i1 + 13];
// <189>           |   %181 = (%0)[sext.i32.i64(%1) * i1 + 10];
// <192>           |   %184 = (%2)[sext.i32.i64(%3) * i1 + 10];
// <196>           |   %188 = (%0)[sext.i32.i64(%1) * i1 + 14];
// <199>           |   %191 = (%2)[sext.i32.i64(%3) * i1 + 14];
// <205>           |   %197 = (%0)[sext.i32.i64(%1) * i1 + 11];
// <208>           |   %200 = (%2)[sext.i32.i64(%3) * i1 + 11];
// <212>           |   %204 = (%0)[sext.i32.i64(%1) * i1 + 15];
// <215>           |   %207 = (%2)[sext.i32.i64(%3) * i1 + 15];
// <226>           |   (%new)[0][i1][4] = zext.i8.i32(%197) + ...
// <229>           |   (%new)[0][i1][6] = -1 * zext.i8.i32(%197) + ...
// <232>           |   (%new)[0][i1][5] = -1 * zext.i8.i32(%197) + ...
// <235>           |   (%new)[0][i1][7] = zext.i8.i32(%197) + ...
// <1247>          + END LOOP
//
// <1263>          + DO i1 = 0, 7, 1   <DO_LOOP>
// <1264>          |   (%alloca30)[0][i1] = 0;
// <1265>          |   (%alloca31)[0][i1] = 0;
// <1266>          |   (%alloca32)[0][i1] = 0;
// <1267>          |   (%alloca33)[0][i1] = 0;
// <1263>          + END LOOP
// <1263>
//
// // All intermediate statements are moved here.
//
// <93>            %94 = 0;
// <145>           @llvm.lifetime.end.p0i8(64,  &((i8*)(%5)[0]));
// <152>           @llvm.lifetime.start.p0i8(64,  &((i8*)(%5)[0]));
// <250>           %233 = 0;
//
// <1250>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <98>            |   %96 = (%new)[0][0][i1];
// <100>           |   %98 = (%new)[0][1][i1];
// <104>           |   %102 = (%new)[0][2][i1];
// <106>           |   %104 = (%new)[0][3][i1];
// <114>           |   %112 = (%96 + %98 + %102 + %104)/u32768  &&  65537;
// <117>           |   %115 = %96 + %98 + %102 + %104 + 65535 * %112
// ^  65535 * %112;
// <119>           |   %117 = (%96 + -1 * %98 + %102 + -1 * %104)/u32768
// &&  65537;
// <122>           |   %120 = %96 + -1 * %98 + %102 + -1 * %104 +  65535 * %117
// ^  65535 * %117;
// <124>           |   %122 = (%96 + %98 + -1 * (%102 + %104))/u32768
// &&  65537;
// <127>           |   %125 = %96 + %98 + 65535 * %122 + -1 * (%102 + %104)
// ^  65535 * %122;
// <129>           |   %127 = (%96 + -1 * %98 + -1 * %102 + %104)/u32768
// &&  65537;
// <132>           |   %130 = %96 + -1 * %98 + -1 * %102 + %104 + 65535 * %127
// ^  65535 * %127;
//                 |   %t1 = (%alloca30)[0][i1];
// <136>           |   (%alloca30)[0][i1] = %t1 + %125 + %115 + %120  +  %130;
// <255>           |   %235 = (%new)[0][4][i1];
// <257>           |   %237 = (%new)[0][5][i1];
// <261>           |   %241 = (%new)[0][6][i1];
// <263>           |   %243 = (%new)[0][7][i1];
// <271>           |   %251 = (%235 + %237 + %241 + %243)/u32768  &&  65537;
// <274>           |   %254 = %235 + %237 + %241 + %243 + 65535 * %251
// ^  65535 * %251;
// <276>           |   %256 = (%235 + -1 * %237 + %241 + -1 * %243)/u32768
// &&  65537;
// <279>           |   %259 = %235 + -1 * %237 + %241 + -1 * %243 + 65535 * %256
// ^  65535 * %256;
// <281>           |   %261 = (%235 + %237 + -1 * (%241 + %243))/u32768
// &&  65537;
// <284>           |   %264 = %235 + %237 + 65535 * %261 + -1 * (%241 + %243)
// ^  65535 * %261;
// <286>           |   %266 = (%235 + -1 * %237 + -1 * %241 + %243)/u32768
// &&  65537;
// <289>           |   %269 = %235 + -1 * %237 + -1 * %241 + %243 + 65535 * %266
// ^  65535 * %266;
//                 |   %t1 = (%alloca31)[0][i1];
// <293>           |   (%alloca31)[0][i1] = %t1 + %264 + %254 + %259  +  %269;
// .....           >> Two more loop bodies concatenated.
// <1250>          + END LOOP
//
// <1272>          + DO i1 = 0, 3, 1   <DO_LOOP>
// <1273>          |   %94 = %94  +  (%alloca30)[0][i1];
// <1274>          |   %369 = %369  +  (%alloca30)[0][i1 + 4];
// <1275>          |   %233 = %233  +  (%alloca31)[0][i1];
// <1276>          |   %505 = %505  +  (%alloca31)[0][i1 + 4];
// <1277>          |   %645 = %645  +  (%alloca32)[0][i1];
// <1278>          |   %921 = %921  +  (%alloca32)[0][i1 + 4];
// <1279>          |   %785 = %785  +  (%alloca33)[0][i1];
// <1280>          |   %1057 = %1057  +  (%alloca33)[0][i1 + 4];
// <1272>          + END LOOP
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopConcatenation.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define DEBUG_TYPE "hir-loop-concatenation"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableConcatenation("disable-hir-loop-concatenation", cl::init(false),
                         cl::Hidden,
                         cl::desc("Disable HIR Loop Concatenation"));

namespace {

class HIRLoopConcatenation {
public:
  HIRLoopConcatenation(HIRFramework &HIRF, const TargetTransformInfo &TTI)
      : HIRF(HIRF), TTI(TTI), AllocaSymbase(0), Is16LoopMode(true) {
    llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
    Is64Bit = TargetTriple.isArch64Bit();
  }

  bool run();

private:
  /// Validates top level nodes in the region. Returns all the found loops in \p
  /// Loops. Also collects intermediate instructions.
  bool validTopLevelNodes(HLRegion *Reg, SmallVectorImpl<HLLoop *> &Loops);

  /// Separates out alloca read and write loops from \p Loops vector.
  void formReadWriteLoopSet(SmallVectorImpl<HLLoop *> &Loops);

  /// Returns true if \p Lp passes all the checks of a valid alloca write loop.
  bool isValidAllocaWriteLoop(HLLoop *Lp);

  /// Returns true if \p Lp passes all the checks of a valid alloca read loop.
  bool isValidAllocaReadLoop(HLLoop *Lp);

  /// Returns true if \p HInst is a valid alloca store instruction.
  bool isValidAllocaStore(HLInst *HInst, SmallSet<int64_t, 4> &FoundConstants);

  /// Returns true if \p HInst is a valid alloca load instruction.
  bool isValidAllocaLoad(HLInst *HInst,
                         SmallSet<int64_t, 4> &FoundConstants) const;

  /// Returns true if \p HInst is a valid function argument load instruction.
  bool isValidFunctionArgumentLoad(HLInst *HInst) const;

  /// Returns true if \p HInst is a valid binary instruction. If \p CheckAddRedn
  /// is set, checks that statement is a add reduction.
  bool isValidBinaryInst(HLInst *HInst, bool CheckAddRedn) const;

  /// Returns true if all the loops in alloca read loop set look valid.
  bool isValidReadLoopSet();

  /// Returns true if all the loops in alloca write loop set look valid.
  bool isValidWriteLoopSet();

  /// Returns true if Ref1 is identical to Ref2 once we replace all the temp
  /// blobs in \p TempBlobMap.
  bool areAnalogous(
      RegDDRef *Ref1, RegDDRef *Ref2,
      SmallVector<std::pair<unsigned, unsigned>, 16> &TempBlobMap) const;

  /// Returns true if \p Lp1 and \p Lp2 compute the same function.
  bool areAnalogousReadLoops(HLLoop *Lp1, HLLoop *Lp2) const;

  /// Returns true if \p Lp1 and \p Lp2 compute the same function. Some of the
  /// load refs different by (IV blob coefficient * \p Offset). If \p
  /// IsConstantOffset is set, the refs simply differ by the constant term.
  bool areAnalogousWriteLoops(HLLoop *Lp1, HLLoop *Lp2, int Offset,
                              bool IsConstantOffset = false) const;

  /// Creates and add a new blob to the CE in \p Ref based on the IV blob
  /// coefficient.
  void adjustRef(RegDDRef *Ref, int Offset, bool IsConstantOffset) const;

  /// Performs loop concatenation for the loops.
  void concatenateLoops(HLRegion *Reg);

  /// Performs loop concatenation for write loop set.
  void createConcatenatedWriteLoop(unsigned NewAllocaIndex) const;

  /// Creates a loop to initialize new allocas used for holding reduction sums.
  void createAllocaInitializationLoop();

  /// Replaces reduction temp at the end of the loop with alloca located at
  /// \p AllocaNum in RednTempToAllocaMap.
  void replaceReductionTempWithAlloca(HLLoop *Lp, unsigned AllocaNum);

  /// Performs loop concatenation for read loop set. Creates read loop from the
  /// set (1, 2, 5, 6) and returns the other set in \p UnConcatenatedLoops.
  void
  createConcatenatedReadLoops(unsigned NewAllocaIndex,
                              SmallVector<HLLoop *, 4> &UnConcatenatedLoops);

  /// Concatenates OtherLoops's bodies to \p FirstLp after making adjustments.
  void createConcatenatedReadLoop(unsigned NewAllocaIndex, HLLoop *FirstLp,
                                  SmallVector<HLLoop *, 3> &OtherLoops);

  /// Helper for createConcatenatedReadLoop() to adjust some refs for
  /// concatenation.
  void adjustAndAppend(HLLoop *FirstLp, HLLoop *Lp, unsigned NewAllocaIndex,
                       int64_t Offset);

  /// Adds a reduction of the form t = t + A[i] to end of \p Lp.
  void addReductionToLoop(HLLoop *Lp, RegDDRef *TempRef,
                          RegDDRef *AllocaRef) const;

  /// Created a reduction loop
  void createReductionLoop(SmallVector<HLLoop *, 4> &UnConcatenatedLoops) const;

private:
  HIRFramework &HIRF;
  const TargetTransformInfo &TTI;

  SmallVector<HLLoop *, 8> AllocaReadLoops;
  SmallVector<HLLoop *, 8> AllocaWriteLoops;
  SmallVector<unsigned, 4> AllocaLoadNodeOffset;
  SmallVector<unsigned, 4> AllocaStoreNodeOffset;
  SmallVector<HLInst *, 24> IntermediateInsts;
  SmallVector<std::pair<RegDDRef *, RegDDRef *>, 4> RednTempToAllocaMap;

  unsigned AllocaSymbase;
  bool Is64Bit;
  bool Is16LoopMode;
};
} // namespace

bool HIRLoopConcatenation::run() {
  if (DisableConcatenation) {
    LLVM_DEBUG(dbgs() << "HIR Loop Concatenation disabled \n");
    return false;
  }

  if (!TTI.isAdvancedOptEnabled(
          TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2)) {
    return false;
  }

  // Expect a function level region.
  if (HIRF.hir_begin() == HIRF.hir_end()) {
    return false;
  }

  auto Reg = cast<HLRegion>(HIRF.hir_begin());
  if (!Reg->isFunctionLevel()) {
    return false;
  }

  SmallVector<HLLoop *, 16> Loops;

  if (!validTopLevelNodes(Reg, Loops)) {
    return false;
  }

  Is16LoopMode = (Loops.size() == 16);

  formReadWriteLoopSet(Loops);

  if (!isValidWriteLoopSet()) {
    return false;
  }

  if (!isValidReadLoopSet()) {
    return false;
  }

  concatenateLoops(Reg);

  Reg->setGenCode();

  return true;
}

bool HIRLoopConcatenation::validTopLevelNodes(
    HLRegion *Reg, SmallVectorImpl<HLLoop *> &Loops) {
  auto Last = (--Reg->child_end());
  auto RetInst = dyn_cast<HLInst>(&*Last);

  if (!RetInst || !isa<ReturnInst>(RetInst->getLLVMInstruction())) {
    return false;
  }

  for (auto It = Reg->child_begin(); It != Last; ++It) {
    auto Child = &*It;

    if (auto Loop = dyn_cast<HLLoop>(Child)) {
      if (!Loop->isInnermost() || !Loop->isDo()) {
        return false;
      }

      if (Loop->hasPreheader() || Loop->hasPostexit()) {
        return false;
      }

      uint64_t TC;
      if (!Loop->isConstTripLoop(&TC) || (TC != 4)) {
        return false;
      }

      Loops.push_back(Loop);

    } else if (auto HInst = dyn_cast<HLInst>(Child)) {
      IntermediateInsts.push_back(HInst);

      if (HInst->isCopyInst()) {
        // Non-constant rval may cause data dependencies.

        auto RvalRef = HInst->getRvalDDRef();

        if (!RvalRef->isIntConstant()) {
          return false;
        }
        continue;
      }

      auto Intrinsic = dyn_cast<IntrinsicInst>(HInst->getLLVMInstruction());

      if (!Intrinsic ||
          ((Intrinsic->getIntrinsicID() != Intrinsic::lifetime_start) &&
           (Intrinsic->getIntrinsicID() != Intrinsic::lifetime_end))) {
        return false;
      }
    } else {
      return false;
    }
  }

  return (Loops.size() == 16 || Loops.size() == 4);
}

void HIRLoopConcatenation::formReadWriteLoopSet(
    SmallVectorImpl<HLLoop *> &Loops) {
  // Populate read and write loop set from alternating loops.

  bool IsEven = true;
  for (auto Lp : Loops) {
    if (IsEven) {
      AllocaWriteLoops.push_back(Lp);
    } else {
      AllocaReadLoops.push_back(Lp);
    }
    IsEven = !IsEven;
  }
}

bool HIRLoopConcatenation::isValidAllocaWriteLoop(HLLoop *Loop) {
  SmallSet<int64_t, 4> ConstantSubs;
  unsigned NodeCount = 0;

  // Checks that all children are either valid load or store HLInsts.
  for (auto It = Loop->child_begin(), End = Loop->child_end(); It != End;
       ++It, ++NodeCount) {
    auto HInst = dyn_cast<HLInst>(&*It);

    if (!HInst) {
      return false;
    }

    auto Inst = HInst->getLLVMInstruction();

    if (isa<LoadInst>(Inst)) {
      if (!isValidFunctionArgumentLoad(HInst)) {
        return false;
      }
    } else if (isa<StoreInst>(Inst)) {
      if (!isValidAllocaStore(HInst, ConstantSubs)) {
        return false;
      }

      AllocaStoreNodeOffset.push_back(NodeCount);

    } else {
      return false;
    }
  }

  // We should have found 4 alloca stores.
  return (ConstantSubs.size() == 4);
}

bool HIRLoopConcatenation::isValidFunctionArgumentLoad(HLInst *HInst) const {
  // Checks that RHS has single dimension, uses function argument as base
  // pointer and contains loop IV with a blob coefficient.

  auto RvalRef = HInst->getRvalDDRef();

  if (RvalRef->getNumDimensions() != 1) {
    return false;
  }

  if (!RvalRef->accessesFunctionArgument()) {
    return false;
  }

  auto CE = RvalRef->getSingleCanonExpr();

  if (CE->getDenominator() != 1) {
    return false;
  }

  unsigned Index;
  int64_t Coeff;

  CE->getIVCoeff(1, &Index, &Coeff);

  if ((Coeff != 1) || (Index == InvalidBlobIndex)) {
    return false;
  }

  auto &BU = CE->getBlobUtils();
  auto Blob = BU.getBlob(Index);

  if (!BU.isSignExtendBlob(Blob)) {
    return false;
  }

  return true;
}

bool HIRLoopConcatenation::isValidAllocaStore(
    HLInst *HInst, SmallSet<int64_t, 4> &FoundConstants) {
  // Checks-
  // store RHS is scalar without any IV.
  // store LHS is based on same alloca which has the type [4 x [4 x i32]]*. It
  // has 3 dimensions of the form [0][i1][constant]. Constant is expected to be
  // unique.

  auto RvalRef = HInst->getRvalDDRef();

  if (!RvalRef->isTerminalRef() || RvalRef->hasIV(1)) {
    return false;
  }

  auto LvalRef = HInst->getLvalDDRef();

  if (!LvalRef->accessesAlloca()) {
    return false;
  }

  if (LvalRef->getNumDimensions() != 3) {
    return false;
  }

  auto FirstCE = LvalRef->getDimensionIndex(1);

  int64_t Const;
  if (!FirstCE->isIntConstant(&Const) || FoundConstants.count(Const)) {
    return false;
  }

  FoundConstants.insert(Const);

  auto SecondCE = LvalRef->getDimensionIndex(2);
  if (!SecondCE->isStandAloneIV()) {
    return false;
  }

  auto ThirdCE = LvalRef->getDimensionIndex(3);

  if (!ThirdCE->isIntConstant(&Const) || (Const != 0)) {
    return false;
  }

  // Based on the structure, LvalRef has only one blob for the base.
  unsigned Symbase = (*LvalRef->blob_begin())->getSymbase();

  if (AllocaSymbase == 0) {
    // Check that base type is [4 x [4 x i32]]*.

    auto SrcTy = LvalRef->getSrcType();
    auto Int32Ty = Type::getInt32Ty(HInst->getHLNodeUtils().getContext());

    if (SrcTy != Int32Ty) {
      return false;
    }

    if ((LvalRef->getNumDimensionElements(1) != 4) ||
        (LvalRef->getNumDimensionElements(2) != 4)) {
      return false;
    }

    AllocaSymbase = Symbase;
  } else if (Symbase != AllocaSymbase) {
    return false;
  }

  return true;
}

bool HIRLoopConcatenation::isValidAllocaReadLoop(HLLoop *Loop) {
  SmallSet<int64_t, 4> ConstantSubs;
  unsigned NodeCount = 0;

  // Checks that all children are either valid alloca loads or binary HLInsts.
  for (auto It = Loop->child_begin(), End = Loop->child_end(); It != End;
       ++It, ++NodeCount) {
    auto HInst = dyn_cast<HLInst>(&*It);

    if (!HInst) {
      return false;
    }

    auto Inst = HInst->getLLVMInstruction();

    bool IsLastInst = (std::next(It) == End);

    if (!IsLastInst && isa<LoadInst>(Inst)) {
      if (!isValidAllocaLoad(HInst, ConstantSubs)) {
        return false;
      }

      AllocaLoadNodeOffset.push_back(NodeCount);

    } else if (isa<BinaryOperator>(Inst)) {
      if (!isValidBinaryInst(HInst, IsLastInst)) {
        return false;
      }
    } else {
      return false;
    }
  }

  return (ConstantSubs.size() == 4);
}

bool HIRLoopConcatenation::isValidAllocaLoad(
    HLInst *HInst, SmallSet<int64_t, 4> &FoundConstants) const {
  // Checks that the alloca look like (%t)[0][contant][i1].
  auto RvalRef = HInst->getRvalDDRef();

  if (!RvalRef->accessesAlloca()) {
    return false;
  }

  if (RvalRef->getNumDimensions() != 3) {
    return false;
  }

  auto FirstCE = RvalRef->getDimensionIndex(1);
  if (!FirstCE->isStandAloneIV()) {
    return false;
  }

  auto SecondCE = RvalRef->getDimensionIndex(2);
  int64_t Const;
  if (!SecondCE->isIntConstant(&Const) || FoundConstants.count(Const)) {
    return false;
  }
  FoundConstants.insert(Const);

  auto ThirdCE = RvalRef->getDimensionIndex(3);

  if (!ThirdCE->isIntConstant(&Const) || (Const != 0)) {
    return false;
  }

  unsigned Symbase = (*RvalRef->blob_begin())->getSymbase();

  if (Symbase != AllocaSymbase) {
    return false;
  }

  return true;
}

bool HIRLoopConcatenation::isValidBinaryInst(HLInst *HInst,
                                             bool CheckAddRedn) const {
  // Checks that Binary Op has all scalar operands
  auto LvalRef = HInst->getLvalDDRef();

  if (!LvalRef->isSelfBlob()) {
    return false;
  }

  if (CheckAddRedn &&
      (HInst->getLLVMInstruction()->getOpcode() != Instruction::Add)) {
    return false;
  }

  unsigned LvalBlobIndex = LvalRef->getSelfBlobIndex();

  auto OpRef1 = *HInst->rval_op_ddref_begin();

  if (!OpRef1->isTerminalRef() || OpRef1->hasIV(1)) {
    return false;
  }

  auto OpRef2 = *(HInst->rval_op_ddref_begin() + 1);

  if (!OpRef2->isTerminalRef() || OpRef2->hasIV(1)) {
    return false;
  }

  if (CheckAddRedn) {
    // We are looking for something like t1 = (t1 + t2 + ...)  +  t3;
    // Verifying the exact structure of the CE (like t1 should have a coeff of
    // 1) is not required for legality so we skip it.
    bool FoundUseInOp1 = OpRef1->usesTempBlob(LvalBlobIndex);
    bool FoundUseInOp2 = OpRef2->usesTempBlob(LvalBlobIndex);

    // Only allow use in one of the rval refs.
    if ((!FoundUseInOp1 && !FoundUseInOp2) ||
        (FoundUseInOp1 && FoundUseInOp2)) {
      return false;
    }
  }

  return true;
}

bool HIRLoopConcatenation::isValidReadLoopSet() {
  SmallSet<unsigned, 8> UniqueLivoutSet;

  auto RefLp = AllocaReadLoops[0];

  if (!isValidAllocaReadLoop(RefLp)) {
    return false;
  }

  auto &BU = RefLp->getBlobUtils();

  // Perform cheap checks first
  for (auto Lp : AllocaReadLoops) {

    // Checks that loop has only one instruction which is both livein and
    // liveout to the loop. This is unique to each loop which confirms that
    // there are no temp data dependencies between loops.
    if (Lp->getNumLiveOutTemps() != 1) {
      return false;
    }

    unsigned UniqueSB = *(Lp->live_out_begin());

    if (!Lp->isLiveIn(UniqueSB)) {
      return false;
    }

    for (unsigned SB : make_range(Lp->live_in_begin(), Lp->live_in_end())) {
      if (UniqueLivoutSet.count(SB)) {
        return false;
      }

      if ((SB == UniqueSB) || (SB == AllocaSymbase)) {
        continue;
      }

      if (BU.isInstBlob(BU.findTempBlobIndex(SB))) {
        return false;
      }
    }

    UniqueLivoutSet.insert(UniqueSB);
  }

  unsigned NumLoops = AllocaReadLoops.size();

  for (unsigned I = 1; I < NumLoops; ++I) {
    if (!areAnalogousReadLoops(RefLp, AllocaReadLoops[I])) {
      return false;
    }
  }

  return true;
}

bool HIRLoopConcatenation::areAnalogous(
    RegDDRef *Ref1, RegDDRef *Ref2,
    SmallVector<std::pair<unsigned, unsigned>, 16> &TempBlobMap) const {

  if (Ref1->isIntConstant() && !DDRefUtils::areEqual(Ref1, Ref2)) {
    return false;
  }

  std::unique_ptr<RegDDRef> CloneRef1(Ref1->clone());
  CloneRef1->replaceTempBlobs(TempBlobMap);

  return DDRefUtils::areEqual(CloneRef1.get(), Ref2);
}

bool HIRLoopConcatenation::areAnalogousReadLoops(HLLoop *Lp1,
                                                 HLLoop *Lp2) const {
  // Check that all instruction in Lp1 and Lp2 are analogous i.e. they are
  // computing the same function but just using different temps.
  SmallVector<std::pair<unsigned, unsigned>, 16> TempBlobMap;
  auto It2 = Lp2->child_begin();

  for (auto It1 = Lp1->child_begin(), End = Lp1->child_end(); It1 != End;
       ++It1, ++It2) {
    if (!isa<HLInst>(It2)) {
      return false;
    }

    auto HInst1 = cast<HLInst>(&*It1);
    auto HInst2 = cast<HLInst>(&*It2);

    if (isa<LoadInst>(HInst1->getLLVMInstruction())) {
      if (!isa<LoadInst>(HInst2->getLLVMInstruction())) {
        return false;
      }

      auto Ref1 = HInst1->getRvalDDRef();
      auto Ref2 = HInst2->getRvalDDRef();

      if (!DDRefUtils::areEqual(Ref1, Ref2)) {
        return false;
      }

      Ref1 = HInst1->getLvalDDRef();
      Ref2 = HInst2->getLvalDDRef();

      TempBlobMap.push_back(
          std::make_pair(Ref1->getSelfBlobIndex(), Ref2->getSelfBlobIndex()));

    } else {
      if (HInst1->getLLVMInstruction()->getValueID() !=
          HInst2->getLLVMInstruction()->getValueID()) {
        return false;
      }

      if (HInst1->getLvalDDRef()->getDestType() !=
          HInst2->getLvalDDRef()->getDestType()) {
        return false;
      }

      auto LvalRef1 = HInst1->getLvalDDRef();
      auto LvalRef2 = HInst2->getLvalDDRef();

      bool IsLastInst = false;

      // For the last instruction, add LHS temp in map before checking RHS due
      // to presence of reduction t1 = t1 + ...
      if (std::next(It1) == End) {
        IsLastInst = true;
        TempBlobMap.push_back(std::make_pair(LvalRef1->getSelfBlobIndex(),
                                             LvalRef2->getSelfBlobIndex()));
      }

      // We know these are binary operators, checks both rvals.
      auto Ref1 = *HInst1->rval_op_ddref_begin();
      auto Ref2 = *HInst2->rval_op_ddref_begin();

      if (!areAnalogous(Ref1, Ref2, TempBlobMap)) {
        return false;
      }

      Ref1 = *(HInst1->rval_op_ddref_begin() + 1);
      Ref2 = *(HInst2->rval_op_ddref_begin() + 1);

      if (!areAnalogous(Ref1, Ref2, TempBlobMap)) {
        return false;
      }

      if (!IsLastInst) {
        TempBlobMap.push_back(std::make_pair(LvalRef1->getSelfBlobIndex(),
                                             LvalRef2->getSelfBlobIndex()));
      }
    }
  }

  return (It2 == Lp2->child_end());
}

bool HIRLoopConcatenation::isValidWriteLoopSet() {

  auto RefLp = AllocaWriteLoops[0];

  if (!isValidAllocaWriteLoop(RefLp)) {
    return false;
  }

  auto &BU = RefLp->getBlobUtils();

  // Perform cheap checks first
  for (auto Lp : AllocaWriteLoops) {

    // Checks that loop has no instruction liveins and no liveouts. This
    // confirms that there are no temp data dependencies between loops.
    if (Lp->hasLiveOutTemps()) {
      return false;
    }

    for (unsigned SB : make_range(Lp->live_in_begin(), Lp->live_in_end())) {
      if (SB == AllocaSymbase) {
        continue;
      }

      if (BU.isInstBlob(BU.findTempBlobIndex(SB))) {
        return false;
      }
    }
  }

  // In 4 loop mode there are only 2 write loops.
  if (!Is16LoopMode) {
    return areAnalogousWriteLoops(RefLp, AllocaWriteLoops[1], 4);
  }

  // There are two different sets of write loops (1, 2, 5, 6) and (2, 3, 7, 8)
  // which are analogous in a slightly different way. 1) Inter-group: there is a
  // constant offset difference between the loads in the two groups. 2)
  // Intra-group: there is a IV blob coefficient offset difference inside the
  // group. We need to check both.

  // This inter-group check. Use the first loop of each group.
  auto SecondRefLp = AllocaWriteLoops[2];

  if (!areAnalogousWriteLoops(RefLp, SecondRefLp, 8, true)) {
    return false;
  }

  // This is intra-group check for 1st group.
  if (!areAnalogousWriteLoops(RefLp, AllocaWriteLoops[1], 4) ||
      !areAnalogousWriteLoops(RefLp, AllocaWriteLoops[4], 8) ||
      !areAnalogousWriteLoops(RefLp, AllocaWriteLoops[5], 12)) {
    return false;
  }

  // This is intra-group check for 2nd group.
  if (!areAnalogousWriteLoops(SecondRefLp, AllocaWriteLoops[3], 4) ||
      !areAnalogousWriteLoops(SecondRefLp, AllocaWriteLoops[6], 8) ||
      !areAnalogousWriteLoops(SecondRefLp, AllocaWriteLoops[7], 12)) {
    return false;
  }

  return true;
}

void HIRLoopConcatenation::adjustRef(RegDDRef *Ref, int Offset,
                                     bool IsCostantOffset) const {

  auto CE = Ref->getSingleCanonExpr();

  if (IsCostantOffset) {
    CE->addConstant(Offset, true);
    return;
  }

  // IV blob coeff looks something like sext.i8.i32(%0).
  // We want to create a new blob which looks like sext.i8.i32(Offset * %0) and
  // add to the CE as a blob.

  unsigned Index = CE->getIVBlobCoeff(1);
  auto &BU = CE->getBlobUtils();

  auto Blob = BU.getBlob(Index);
  BlobTy InnerBlob = nullptr;

  bool Res = BU.isSignExtendBlob(Blob, &InnerBlob);
  (void)Res;
  assert(Res && "Sign extended blob expected!");

  auto NewBlob = BU.createBlob(Offset, InnerBlob->getType(), false);
  NewBlob = BU.createMulBlob(NewBlob, InnerBlob, false);

  NewBlob = BU.createSignExtendBlob(NewBlob, Blob->getType(), true, &Index);

  CE->addBlob(Index, 1);
}

bool HIRLoopConcatenation::areAnalogousWriteLoops(HLLoop *Lp1, HLLoop *Lp2,
                                                  int Offset,
                                                  bool IsConstantOffset) const {
  // Check that all instruction in Lp1 and Lp2 are analogous i.e. they are
  // computing the same function but just using different temps and memory
  // references with a constant offset.
  SmallVector<std::pair<unsigned, unsigned>, 16> TempBlobMap;
  auto It2 = Lp2->child_begin();

  for (auto It1 = Lp1->child_begin(), End = Lp1->child_end(); It1 != End;
       ++It1, ++It2) {
    if (!isa<HLInst>(It2)) {
      return false;
    }

    auto HInst1 = cast<HLInst>(&*It1);
    auto HInst2 = cast<HLInst>(&*It2);

    if (isa<LoadInst>(HInst1->getLLVMInstruction())) {
      if (!isa<LoadInst>(HInst2->getLLVMInstruction())) {
        return false;
      }

      auto Ref1 = HInst1->getRvalDDRef();
      auto Ref2 = HInst2->getRvalDDRef();

      std::unique_ptr<RegDDRef> CloneRef1(Ref1->clone());

      adjustRef(CloneRef1.get(), Offset, IsConstantOffset);

      if (!DDRefUtils::areEqual(CloneRef1.get(), Ref2)) {
        return false;
      }

      Ref1 = HInst1->getLvalDDRef();
      Ref2 = HInst2->getLvalDDRef();

      TempBlobMap.push_back(
          std::make_pair(Ref1->getSelfBlobIndex(), Ref2->getSelfBlobIndex()));

    } else if (!isa<StoreInst>(HInst2->getLLVMInstruction())) {

      auto Ref1 = HInst1->getLvalDDRef();
      auto Ref2 = HInst2->getLvalDDRef();

      if (!DDRefUtils::areEqual(Ref1, Ref2)) {
        return false;
      }

      Ref1 = HInst1->getRvalDDRef();
      Ref2 = HInst2->getRvalDDRef();

      if (!areAnalogous(Ref1, Ref2, TempBlobMap)) {
        return false;
      }
    }
  }

  return (It2 == Lp2->child_end());
}

void HIRLoopConcatenation::concatenateLoops(HLRegion *Reg) {
  auto &HNU = AllocaReadLoops[0]->getHLNodeUtils();

  // Create new alloca with [16 x [8 x i32]]* type for 16 loop mode and [8 x [4
  // x i32]]* type for 4 loop mode.
  auto Int32Ty = Type::getInt32Ty(HNU.getContext());
  auto ArrTy = ArrayType::get(Int32Ty, Is16LoopMode ? 8 : 4);
  ArrTy = ArrayType::get(ArrTy, Is16LoopMode ? 16 : 8);

  unsigned NewAllocaIndex = HNU.createAlloca(ArrTy, Reg);

  createConcatenatedWriteLoop(NewAllocaIndex);

  if (Is16LoopMode) {
    createAllocaInitializationLoop();
  }

  SmallVector<HLLoop *, 4> UnConcatenatedLoops;
  createConcatenatedReadLoops(NewAllocaIndex, UnConcatenatedLoops);

  if (Is16LoopMode) {
    createReductionLoop(UnConcatenatedLoops);
  }
}

void HIRLoopConcatenation::createConcatenatedWriteLoop(
    unsigned NewAllocaIndex) const {
  // Replace old alloca stores in 1st write loop with new allocas.
  auto FirstLp = AllocaWriteLoops[0];
  auto FirstChildIt = FirstLp->child_begin();
  auto &DDRU = FirstLp->getDDRefUtils();

  for (unsigned I = 0; I < 4; I++) {
    auto NodeIt = FirstChildIt;
    std::advance(NodeIt, AllocaStoreNodeOffset[I]);

    auto Node = cast<HLDDNode>(&*NodeIt);
    auto OldRef = Node->getLvalDDRef();

    RegDDRef *NewRef = DDRU.createMemRef(NewAllocaIndex);
    NewRef->addDimension(OldRef->getDimensionIndex(3));
    NewRef->addDimension(OldRef->getDimensionIndex(2));
    NewRef->addDimension(OldRef->getDimensionIndex(1));

    Node->replaceOperandDDRef(OldRef, NewRef);
  }

  if (Is16LoopMode) {
    // Replace old alloca stores in 3th write loop with new allocas. This loops
    // body will be move to 1st loop.
    auto Lp = AllocaWriteLoops[2];
    auto ChildIt = Lp->child_begin();

    for (unsigned I = 0; I < 4; I++) {
      auto NodeIt = ChildIt;
      std::advance(NodeIt, AllocaStoreNodeOffset[I]);

      auto Node = cast<HLDDNode>(&*NodeIt);
      auto OldRef = Node->getLvalDDRef();

      RegDDRef *NewRef = DDRU.createMemRef(NewAllocaIndex);
      auto FirstCE = OldRef->getDimensionIndex(1);
      // First CE needs an adjustment.
      FirstCE->addConstant(4, true);
      NewRef->addDimension(OldRef->getDimensionIndex(3));
      NewRef->addDimension(OldRef->getDimensionIndex(2));
      NewRef->addDimension(FirstCE);

      Node->replaceOperandDDRef(OldRef, NewRef);
    }

    // Move 5th loop's body to 1st loop and adjust its upper canon.
    HLNodeUtils::moveAsLastChildren(FirstLp, Lp->child_begin(),
                                    Lp->child_end());

    auto &Context = FirstLp->getHLNodeUtils().getContext();
    Metadata *UnrollMD = MDString::get(Context, "llvm.loop.unroll.full");

    FirstLp->addLoopMetadata(MDNode::get(Context, UnrollMD));
  }

  FirstLp->getUpperCanonExpr()->setConstant(Is16LoopMode ? 15 : 7);

  // Remove all other write loops.
  for (unsigned I = 1, NumLoops = AllocaWriteLoops.size(); I < NumLoops; ++I) {
    HLNodeUtils::remove(AllocaWriteLoops[I]);
  }

  FirstLp->removeLiveInTemp(AllocaSymbase);
  auto NewSymbase = FirstLp->getBlobUtils().getTempBlobSymbase(NewAllocaIndex);
  FirstLp->addLiveInTemp(NewSymbase);

  HIRInvalidationUtils::invalidateBody(FirstLp);
  HIRInvalidationUtils::invalidateBounds(FirstLp);
}

void HIRLoopConcatenation::createAllocaInitializationLoop() {

  // Create a new loop with trip count 8 and insert it before first read loop.
  auto AllocaInitLp = AllocaReadLoops[0]->cloneEmpty();
  auto &HNU = AllocaInitLp->getHLNodeUtils();
  auto &DDRU = HNU.getDDRefUtils();
  auto &CEU = DDRU.getCanonExprUtils();
  auto &BU = CEU.getBlobUtils();

  AllocaInitLp->getUpperCanonExpr()->setConstant(7);

  HLNodeUtils::insertBefore(AllocaReadLoops[0], AllocaInitLp);

  // Create 4 new allocas and initialize them inside the loop.
  for (unsigned I = 0; I < 4; ++I) {
    // Create new alloca with [8 x i32]* type.
    auto Int32Ty = Type::getInt32Ty(HNU.getContext());
    auto Int64Ty = Type::getInt64Ty(HNU.getContext());
    auto ArrTy = ArrayType::get(Int32Ty, 8);

    unsigned NewAllocaIndex =
        HNU.createAlloca(ArrTy, AllocaInitLp->getParentRegion());

    // Create new ref of the form A[0][i1] based on the alloca.
    RegDDRef *AllocaRef = DDRU.createMemRef(NewAllocaIndex);
    auto Zero = CEU.createCanonExpr(Is64Bit ? Int64Ty : Int32Ty);
    auto IV = Zero->clone();
    IV->setIVCoeff(1, InvalidBlobIndex, 1);

    AllocaRef->addDimension(Zero);
    AllocaRef->addDimension(IV);

    auto ZeroRef = DDRU.createConstDDRef(Int32Ty, 0);

    auto Store = HNU.createStore(ZeroRef, "store", AllocaRef);
    HLNodeUtils::insertAsLastChild(AllocaInitLp, Store);
    RednTempToAllocaMap.push_back(std::make_pair(nullptr, AllocaRef));

    auto NewSymbase = BU.getTempBlobSymbase(NewAllocaIndex);
    AllocaInitLp->addLiveInTemp(NewSymbase);
  }
}

void HIRLoopConcatenation::replaceReductionTempWithAlloca(HLLoop *Lp,
                                                          unsigned AllocaNum) {
  auto &TempAllocaPair = RednTempToAllocaMap[AllocaNum];
  auto AllocaRef = TempAllocaPair.second;

  // Change-
  // t1 = t1 + ...
  //
  // To-
  // t2 = A[i]
  // A[i] = t2 + ...
  auto LastInst = cast<HLInst>(Lp->getLastChild());

  auto LoadInst = Lp->getHLNodeUtils().createLoad(AllocaRef->clone());
  HLNodeUtils::insertBefore(LastInst, LoadInst);

  auto LvalRef = LastInst->getLvalDDRef();
  unsigned LvalBlobIndex = LvalRef->getSelfBlobIndex();
  unsigned LoadLvalBlobIndex = LoadInst->getLvalDDRef()->getSelfBlobIndex();

  LastInst->replaceOperandDDRef(LvalRef, AllocaRef->clone());

  // Reduction temp may be in either of the two rval operands.
  auto *OpRef1 = *LastInst->rval_op_ddref_begin();

  OpRef1->replaceTempBlob(LvalBlobIndex, LoadLvalBlobIndex);

  auto *OpRef2 = *(LastInst->rval_op_ddref_begin() + 1);

  OpRef2->replaceTempBlob(LvalBlobIndex, LoadLvalBlobIndex);

  Lp->addLiveInTemp(AllocaRef->getBasePtrSymbase());
  Lp->removeLiveOutTemp(LvalRef->getSymbase());

  // Set temp ref for pair.
  TempAllocaPair.first = LvalRef;
}

void HIRLoopConcatenation::createConcatenatedReadLoops(
    unsigned NewAllocaIndex, SmallVector<HLLoop *, 4> &UnConcatenatedLoops) {

  auto FirstLp = AllocaReadLoops[0];

  if (Is16LoopMode) {
    replaceReductionTempWithAlloca(FirstLp, 0);
  }

  // Prepend all intermediate instructions to 1st read loop.
  for (auto &Inst : IntermediateInsts) {
    HLNodeUtils::moveBefore(FirstLp, Inst);
  }

  SmallVector<HLLoop *, 3> OtherLoops;

  OtherLoops.push_back(AllocaReadLoops[1]);
  if (Is16LoopMode) {
    OtherLoops.push_back(AllocaReadLoops[4]);
    OtherLoops.push_back(AllocaReadLoops[5]);
  }

  createConcatenatedReadLoop(NewAllocaIndex, FirstLp, OtherLoops);
  FirstLp->getUpperCanonExpr()->setConstant(Is16LoopMode ? 7 : 3);

  if (Is16LoopMode) {
    UnConcatenatedLoops.push_back(AllocaReadLoops[2]);
    UnConcatenatedLoops.push_back(AllocaReadLoops[3]);
    UnConcatenatedLoops.push_back(AllocaReadLoops[6]);
    UnConcatenatedLoops.push_back(AllocaReadLoops[7]);
  }
}

void HIRLoopConcatenation::createConcatenatedReadLoop(
    unsigned NewAllocaIndex, HLLoop *FirstLp,
    SmallVector<HLLoop *, 3> &OtherLoops) {

  // Replace old alloca loads in 1st read loop with new allocas.
  auto FirstChildIt = FirstLp->child_begin();
  auto &DDRU = FirstLp->getDDRefUtils();

  for (unsigned I = 0; I < 4; I++) {
    auto NodeIt = FirstChildIt;
    std::advance(NodeIt, AllocaLoadNodeOffset[I]);

    auto Node = cast<HLDDNode>(&*NodeIt);
    auto OldRef = Node->getRvalDDRef();

    RegDDRef *NewRef = DDRU.createMemRef(NewAllocaIndex);
    NewRef->addDimension(OldRef->getDimensionIndex(3));
    NewRef->addDimension(OldRef->getDimensionIndex(2));
    NewRef->addDimension(OldRef->getDimensionIndex(1));

    Node->replaceOperandDDRef(OldRef, NewRef);
  }

  // Adjust and append Otherloops' bodies to FirstLp.
  int64_t Offset = 4;
  for (unsigned I = 0, NumLoops = OtherLoops.size(); I < NumLoops;
       ++I, Offset += 4) {
    if (Is16LoopMode) {
      replaceReductionTempWithAlloca(OtherLoops[I], I + 1);
    }
    adjustAndAppend(FirstLp, OtherLoops[I], NewAllocaIndex, Offset);
    HLNodeUtils::remove(OtherLoops[I]);
  }

  FirstLp->removeLiveInTemp(AllocaSymbase);
  auto NewSymbase = FirstLp->getBlobUtils().getTempBlobSymbase(NewAllocaIndex);
  FirstLp->addLiveInTemp(NewSymbase);

  HIRInvalidationUtils::invalidateBody(FirstLp);
}

void HIRLoopConcatenation::adjustAndAppend(HLLoop *FirstLp, HLLoop *Lp,
                                           unsigned NewAllocaIndex,
                                           int64_t Offset) {

  auto FirstChildIt = Lp->child_begin();
  auto &DDRU = Lp->getDDRefUtils();

  for (unsigned I = 0; I < 4; I++) {
    auto NodeIt = FirstChildIt;
    std::advance(NodeIt, AllocaLoadNodeOffset[I]);

    auto Node = cast<HLDDNode>(&*NodeIt);

    auto OldRef = Node->getRvalDDRef();

    RegDDRef *NewRef = DDRU.createMemRef(NewAllocaIndex);
    auto SecondCE = OldRef->getDimensionIndex(2);
    // Second CE needs an adjustment.
    SecondCE->addConstant(Offset, true);

    NewRef->addDimension(OldRef->getDimensionIndex(3));
    NewRef->addDimension(SecondCE);
    NewRef->addDimension(OldRef->getDimensionIndex(1));

    Node->replaceOperandDDRef(OldRef, NewRef);
  }

  Lp->getHLNodeUtils().moveAsLastChildren(FirstLp, Lp->child_begin(),
                                          Lp->child_end());

  // Add Lp's liveins to FirstLp.
  for (auto It = Lp->live_in_begin(), E = Lp->live_in_end(); It != E; ++It) {
    FirstLp->addLiveInTemp(*It);
  }

  // Add Lp's liveouts to FirstLp.
  for (auto It = Lp->live_out_begin(), E = Lp->live_out_end(); It != E; ++It) {
    FirstLp->addLiveOutTemp(*It);
  }
}

void HIRLoopConcatenation::addReductionToLoop(HLLoop *Lp, RegDDRef *TempRef,
                                              RegDDRef *AllocaRef) const {
  auto &HNU = Lp->getHLNodeUtils();

  auto AddInst =
      HNU.createAdd(TempRef, AllocaRef->clone(), "redn", TempRef->clone());
  HLNodeUtils::insertAsLastChild(Lp, AddInst);

  Lp->addLiveInTemp(TempRef->getSymbase());
  Lp->addLiveInTemp(AllocaRef->getBasePtrSymbase());
  Lp->addLiveOutTemp(TempRef->getSymbase());
}

void HIRLoopConcatenation::createReductionLoop(
    SmallVector<HLLoop *, 4> &UnConcatenatedLoops) const {
  auto RednLp = UnConcatenatedLoops[0]->cloneEmpty();
  RegDDRef *AllocaRef = nullptr;

  HLNodeUtils::insertBefore(UnConcatenatedLoops[0], RednLp);

  for (unsigned I = 0; I < 4; ++I) {
    // Each alloca is used for collecting results of two temp reductions. First
    // temp is pushed onto the map. The second map is extracted from
    // UnConcatenatedLoops.
    auto TempRef = RednTempToAllocaMap[I].first;
    AllocaRef = RednTempToAllocaMap[I].second;

    addReductionToLoop(RednLp, TempRef, AllocaRef);

    auto LastInst = cast<HLInst>(UnConcatenatedLoops[I]->getLastChild());
    TempRef = LastInst->removeLvalDDRef();
    AllocaRef = AllocaRef->clone();

    // Adjust the ref by adding an offset of 4.
    AllocaRef->getDimensionIndex(1)->addConstant(4, true);

    addReductionToLoop(RednLp, TempRef, AllocaRef);

    HLNodeUtils::remove(UnConcatenatedLoops[I]);
  }

  unsigned AllocaSymbase = (*AllocaRef->blob_begin())->getSymbase();
  RednLp->addLiveInTemp(AllocaSymbase);
}

PreservedAnalyses
HIRLoopConcatenationPass::run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
  HIRLoopConcatenation(AM.getResult<HIRFrameworkAnalysis>(F),
                       AM.getResult<TargetIRAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRLoopConcatenationLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopConcatenationLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopConcatenationLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<TargetTransformInfoWrapperPass>();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }

  bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      LLVM_DEBUG(dbgs() << "HIR Loop Concatenation disabled \n");
      return false;
    }

    return HIRLoopConcatenation(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F))
        .run();
  }
};

char HIRLoopConcatenationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopConcatenationLegacyPass, "hir-loop-concatenation",
                      "HIR Loop Concatenation", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRLoopConcatenationLegacyPass, "hir-loop-concatenation",
                    "HIR Loop Concatenation", false, false)

FunctionPass *llvm::createHIRLoopConcatenationPass() {
  return new HIRLoopConcatenationLegacyPass();
}
