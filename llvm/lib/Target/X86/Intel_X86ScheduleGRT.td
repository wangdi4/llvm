//=- X86ScheduleGRT.td - X86 Gracemont Scheduling ------------*- tablegen -*-=//
//
//      Copyright (c) 2021 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
//===----------------------------------------------------------------------===//
//
// This file defines the machine model for Intel Gracemont to support
// instruction scheduling and other instruction cost heuristics.
//
// Automatically generated file by smg tool.
//
//===----------------------------------------------------------------------===//

def GRTModel : SchedMachineModel {
  // Gracemont can allocate 5 uops per cycle.
  let IssueWidth = 5; // Based on allocator width.
  let MicroOpBufferSize = 256; // Based on the reorder buffer.
  let LoadLatency = 5;
  let MispredictPenalty = 15;

  // Latency for instructions without latency info.
  int MaxLatency = 50;

  // This flag is set to allow the scheduler to assign a default model to
  // unrecognized opcodes.
  let CompleteModel = 0;
}

let SchedModel = GRTModel in {

// Gracemont can issue micro-ops to 17 different ports in one cycle.
def GRTPort00 : ProcResource<1> { let BufferSize = 16; }
def GRTPort01 : ProcResource<1> { let BufferSize = 16; }
def GRTPort02 : ProcResource<1> { let BufferSize = 16; }
def GRTPort03 : ProcResource<1> { let BufferSize = 16; }
def GRTPort08 : ProcResource<1>;
def GRTPort09 : ProcResource<1>;
def GRTPort10 : ProcResource<1>;
def GRTPort11 : ProcResource<1>;
def GRTPort12 : ProcResource<1>;
def GRTPort13 : ProcResource<1>;
def GRTPort20 : ProcResource<1>;
def GRTPort21 : ProcResource<1>;
def GRTPort22 : ProcResource<1>;
def GRTPort28 : ProcResource<1>;
def GRTPort29 : ProcResource<1>;
def GRTPort30 : ProcResource<1>;
def GRTPort31 : ProcResource<1>;

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def GRTPortInvalid : ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
def GRTPort01_02       : ProcResGroup<[GRTPort01, GRTPort02]>;
def GRTPort00_01_02_03 : ProcResGroup<[GRTPort00, GRTPort01, GRTPort02, GRTPort03]>;
def GRTPort08_09       : ProcResGroup<[GRTPort08, GRTPort09]>;
def GRTPort10_11       : ProcResGroup<[GRTPort10, GRTPort11]>;
def GRTPort12_13       : ProcResGroup<[GRTPort12, GRTPort13]>;
def GRTPort20_21       : ProcResGroup<[GRTPort20, GRTPort21]>;
def GRTPort20_21_22    : ProcResGroup<[GRTPort20, GRTPort21, GRTPort22]> { let BufferSize = 36; }
def GRTPort28_29       : ProcResGroup<[GRTPort28, GRTPort29]> { let BufferSize = 18; }
def GRTPort30_31       : ProcResGroup<[GRTPort30, GRTPort31]>;

def GRTPort10_11_12_13 : ProcResGroup<[GRTPort10, GRTPort11, GRTPort12, GRTPort13]> {
  let BufferSize = 24;
}
def GRTPort08_09_30_31 : ProcResGroup<[GRTPort08, GRTPort09, GRTPort30, GRTPort31]> {
  let BufferSize = 44;
}

// Integer loads are 4 cycles, so ReadAfterLd registers needn't be available
// until 4 cycles after the memory operand.
def : ReadAdvance<ReadAfterLd, 4>;

// Vector loads are 5 cycles, so ReadAfterVec*Ld registers needn't be available
// until 5 cycles after the memory operand.
def : ReadAdvance<ReadAfterVecLd, 5>;
def : ReadAdvance<ReadAfterVecXLd, 5>;
def : ReadAdvance<ReadAfterVecYLd, 5>;

def : ReadAdvance<ReadInt2Fpu, 0>;

// Many SchedWrites are defined in pairs with and without a folded load.
// Instructions with folded loads are usually micro-fused, so they only appear
// as two micro-ops when queued in the reservation station.
// This multiclass defines the resource usage for variants with and without
// folded loads.
multiclass GRTWriteResPair<X86FoldableSchedWrite SchedRW,
                          list<ProcResourceKind> ExePorts,
                          int Lat, list<int> Res = [1], int UOps = 1,
                          int LoadLat = 4, int NumLD = 1> {
  // Register variant is using a single cycle on ExePort.
  def : WriteRes<SchedRW, ExePorts> {
    let Latency = Lat;
    let ReleaseAtCycles = Res;
    let NumMicroOps = UOps;
  }

  // Memory variant also uses a cycle on port 10/11 and adds LoadLat cycles to
  // the latency (default = 4).
  def : WriteRes<SchedRW.Folded, !listconcat([GRTPort10_11], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ReleaseAtCycles = !listconcat([NumLD], Res);
    let NumMicroOps = !add(UOps, NumLD);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [GRTPort00_01_02_03]> {
  let Latency = 2;
}
def : WriteRes<WriteADCLd, [GRTPort00_01_02_03]> {
  let Latency = 5;
}
defm : GRTWriteResPair<WriteAESDecEnc, [GRTPort20_21], 3, [2], 2, 5, 2>;
defm : GRTWriteResPair<WriteAESIMC, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteAESKeyGen, [GRTPort20_21], 3, [1], 1, 5>;
def : WriteRes<WriteALU, [GRTPort00_01_02_03]>;
def : WriteRes<WriteALULd, [GRTPort00_01_02_03]> {
  let Latency = 4;
}
defm : GRTWriteResPair<WriteBEXTR, [GRTPort01], 4, [1]>;
defm : GRTWriteResPair<WriteBLS, [GRTPort01], 3, [1]>;
defm : GRTWriteResPair<WriteBSF, [GRTPort01], 3, [1]>;
defm : GRTWriteResPair<WriteBSR, [GRTPort01], 3, [1]>;
def : WriteRes<WriteBSWAP32, [GRTPort00_01_02_03]>;
def : WriteRes<WriteBSWAP64, [GRTPort00_01_02_03]>;
defm : GRTWriteResPair<WriteBZHI, [GRTPort01], 3, [1]>;
def : WriteRes<WriteBitTest, [GRTPort00_01_02_03]>;
defm : X86WriteRes<WriteBitTestImmLd, [GRTPort00_01_02_03, GRTPort10_11], 6, [1, 1], 2>;
def : WriteRes<WriteBitTestRegLd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteBitTestSet, [GRTPort00_01_02_03]> {
  let Latency = 2;
}
def : WriteRes<WriteBitTestSetImmLd, [GRTPort00_01_02_03]> {
  let Latency = 5;
}
defm : GRTWriteResPair<WriteBlend, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteBlendY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : GRTWriteResPair<WriteCLMul, [GRTPort20], 4, [2], 2, 5, 2>;
defm : GRTWriteResPair<WriteCMOV, [GRTPort00_01_02_03], 1, [1]>;
defm : X86WriteRes<WriteCMPXCHG, [GRTPort00_01_02_03], 5, [5], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort12_13], 8, [1, 4, 1, 1, 1], 8>;
defm : GRTWriteResPair<WriteCRC32, [GRTPort01], 3, [1]>;
defm : GRTWriteResPair<WriteCvtI2PD, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteCvtI2PDY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteCvtI2PDZ>;
defm : GRTWriteResPair<WriteCvtI2PS, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteCvtI2PSY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteCvtI2PSZ>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtI2SD, [GRTPort20], 4, [1], 1, 0>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtI2SS, [GRTPort20], 4, [1], 1, 0>;
defm : GRTWriteResPair<WriteCvtPD2I, [GRTPort20], 3, [1], 1, 5>;
defm : X86WriteRes<WriteCvtPD2IY, [GRTPort20], 3, [2], 2>;
defm : X86WriteRes<WriteCvtPD2IYLd, [GRTPort10_11, GRTPort20], 8, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPD2IZ>;
defm : GRTWriteResPair<WriteCvtPD2PS, [GRTPort20], 3, [1], 1, 5>;
defm : X86WriteRes<WriteCvtPD2PSY, [GRTPort20], 3, [2], 2>;
defm : X86WriteRes<WriteCvtPD2PSYLd, [GRTPort10_11, GRTPort20], 8, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPD2PSZ>;
defm : GRTWriteResPair<WriteCvtPH2PS, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteCvtPH2PSY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteCvtPH2PSZ>;
defm : GRTWriteResPair<WriteCvtPS2I, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteCvtPS2IY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteCvtPS2IZ>;
defm : GRTWriteResPair<WriteCvtPS2PD, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteCvtPS2PDY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteCvtPS2PDZ>;
def : WriteRes<WriteCvtPS2PH, [GRTPort20]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteCvtPS2PHSt, [GRTPort12_13, GRTPort20, GRTPort28_29], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHY, [GRTPort20], 3, [2], 2>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [GRTPort12_13, GRTPort20, GRTPort28_29], 3, [2, 2, 2], 6>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZ>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZSt>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtSD2I, [GRTPort20], 4, [1], 1, 0>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtSD2SS, [GRTPort20], 4, [1], 1, 0>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtSS2I, [GRTPort20], 4, [1], 1, 0>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteCvtSS2SD, [GRTPort20], 4, [1], 1, 0>;
defm : GRTWriteResPair<WriteDPPD, [GRTPortInvalid, GRTPort20_21, GRTPort20_21_22], 9, [1, 2, 1], 4>;
defm : X86WriteRes<WriteDPPS, [GRTPortInvalid, GRTPort20_21, GRTPort20_21_22], 16, [1, 3, 4], 8>;
def : WriteRes<WriteDPPSLd, [GRTPortInvalid]> {
  let Latency = GRTModel.MaxLatency;
}
defm : X86WriteRes<WriteDPPSY, [GRTPortInvalid, GRTPort20_21, GRTPort20_21_22], 16, [2, 6, 8], 16>;
defm : X86WriteRes<WriteDPPSYLd, [GRTPortInvalid], GRTModel.MaxLatency, [2], 2>;
defm : X86WriteRes<WriteDiv16, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteDiv16Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteDiv32, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteDiv32Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteDiv64, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteDiv64Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : GRTWriteResPair<WriteDiv8, [GRTPort01_02], 1, [1]>;
def : WriteRes<WriteEMMS, [GRTPort00_01_02_03]>;
def : WriteRes<WriteFAdd, [GRTPort20_21]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFAddLd, [GRTPort10_11, GRTPort21], 9, [1, 1], 2>;
defm : GRTWriteResPair<WriteFAdd64, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFAdd64X, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFAdd64Y, [GRTPort20_21], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFAdd64Z>;
defm : GRTWriteResPair<WriteFAddX, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFAddY, [GRTPort20_21], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFAddZ>;
defm : GRTWriteResPair<WriteFBlend, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteFBlendY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteRes<WriteFCMOV, [GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort20, GRTPort20_21, GRTPort30_31], 8, [1, 1, 1, 1, 1, 1, 1], 7>;
defm : GRTWriteResPair<WriteFCmp, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFCmp64, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFCmp64X, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFCmp64Y, [GRTPort20_21], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFCmp64Z>;
defm : GRTWriteResPair<WriteFCmpX, [GRTPort20_21], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFCmpY, [GRTPort20_21], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFCmpZ>;
defm : GRTWriteResPair<WriteFCom, [GRTPort21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFComX, [GRTPort21], 4, [1], 1, 5>;
def : WriteRes<WriteFDiv, [GRTPort20]> {
  let Latency = 13;
}
defm : X86WriteRes<WriteFDivLd, [GRTPortInvalid, GRTPort10_11, GRTPort20], 12, [1, 1, 2], 4>;
defm : GRTWriteResPair<WriteFDiv64, [GRTPort20], 13, [1], 1, 5>;
defm : GRTWriteResPair<WriteFDiv64X, [GRTPort20], 21, [1], 1, 5>;
defm : GRTWriteResPair<WriteFDiv64Y, [GRTPort20], 21, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFDiv64Z>;
defm : GRTWriteResPair<WriteFDivX, [GRTPort20], 15, [1], 1, 5>;
defm : GRTWriteResPair<WriteFDivY, [GRTPort20], 15, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFDivZ>;
defm : GRTWriteResPair<WriteFHAdd, [GRTPort20_21, GRTPort20_21_22, GRTPort30_31], 7, [1, 2, 1], 4>;
defm : GRTWriteResPair<WriteFHAddY, [GRTPort20_21, GRTPort20_21_22, GRTPort30_31], 7, [2, 4, 2], 8, 4, 2>;
def : WriteRes<WriteFLD0, [GRTPort20_21]>;
def : WriteRes<WriteFLD1, [GRTPort20]> {
  let Latency = 4;
}
def : WriteRes<WriteFLDC, [GRTPort20]> {
  let Latency = 4;
}
def : WriteRes<WriteFLoad, [GRTPort10_11]> {
  let Latency = 4;
}
def : WriteRes<WriteFLoadX, [GRTPort10_11]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteFLoadY, [GRTPort10_11], 4, [2], 2>;
defm : GRTWriteResPair<WriteFLogic, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteFLogicY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFLogicZ>;
defm : GRTWriteResPair<WriteFMA, [GRTPort20_21], 6, [1], 1, 5>;
defm : GRTWriteResPair<WriteFMAX, [GRTPort20_21], 6, [1], 1, 5>;
defm : GRTWriteResPair<WriteFMAY, [GRTPort20_21], 6, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFMAZ>;
defm : X86WriteRes<WriteFMOVMSK, [GRTPort20], 5, [2], 2>;
defm : X86WriteRes<WriteFMaskedLoad, [GRTPort10_11, GRTPort20], 6, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [GRTPort10_11, GRTPort20], 6, [2, 2], 4>;
defm : X86WriteRes<WriteFMaskedStore32, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [2, 2, 2], 6>;
defm : X86WriteRes<WriteFMaskedStore64, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [2, 2, 2], 6>;
def : WriteRes<WriteFMoveX, [GRTPort20_21_22]>;
defm : X86WriteRes<WriteFMoveY, [GRTPort20_21_22], 1, [2], 2>;
defm : X86WriteResUnsupported<WriteFMoveZ>;
def : WriteRes<WriteFMul, [GRTPort20_21]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteFMulLd, [GRTPort10_11, GRTPort20], 10, [1, 1], 2>;
defm : GRTWriteResPair<WriteFMul64, [GRTPort20_21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFMul64X, [GRTPort20_21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFMul64Y, [GRTPort20_21], 4, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFMul64Z>;
defm : GRTWriteResPair<WriteFMulX, [GRTPort20_21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFMulY, [GRTPort20_21], 4, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFMulZ>;
defm : GRTWriteResPair<WriteFRcp, [GRTPort20], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFRcpX, [GRTPort20], 5, [1], 1, 5>;
defm : GRTWriteResPair<WriteFRcpY, [GRTPort20], 5, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFRcpZ>;
defm : GRTWriteResPair<WriteFRnd, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteFRndY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFRndZ>;
defm : GRTWriteResPair<WriteFRsqrt, [GRTPort20], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFRsqrtX, [GRTPort20], 5, [1], 1, 5>;
defm : GRTWriteResPair<WriteFRsqrtY, [GRTPort20], 5, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFRsqrtZ>;
defm : GRTWriteResPair<WriteFShuffle, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteFShuffle256, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : GRTWriteResPair<WriteFShuffleY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFShuffleZ>;
def : WriteRes<WriteFSign, [GRTPort20_21]>;
defm : GRTWriteResPair<WriteFSqrt, [GRTPort20], 11, [1], 1, 5>;
defm : GRTWriteResPair<WriteFSqrt64, [GRTPort20], 17, [1], 1, 5>;
defm : GRTWriteResPair<WriteFSqrt64X, [GRTPort20], 29, [1], 1, 5>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteFSqrt64Y, [GRTPort20], 48, [2], 2, -14, 2>;
defm : X86WriteResPairUnsupported<WriteFSqrt64Z>;
def : WriteRes<WriteFSqrt80, [GRTPort20]> {
  let Latency = 13;
}
defm : GRTWriteResPair<WriteFSqrtX, [GRTPort20], 17, [1], 1, 2>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteFSqrtY, [GRTPort20], 24, [2], 2, -5, 2>;
defm : X86WriteResPairUnsupported<WriteFSqrtZ>;
defm : X86WriteRes<WriteFStore, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [GRTPort08_09, GRTPort12_13], 0, [2, 2], 4>;
defm : X86WriteRes<WriteFStoreX, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [GRTPort08_09, GRTPort12_13], 0, [2, 2], 4>;
defm : GRTWriteResPair<WriteFTest, [GRTPort21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteFTestY, [GRTPort21], 4, [2], 2, 5, 2>;
defm : X86WriteRes<WriteFVarBlend, [GRTPort20_21, GRTPort20_21_22], 6, [1, 3], 4>;
defm : X86WriteRes<WriteFVarBlendLd, [GRTPort10_11, GRTPort20_21, GRTPort20_21_22], 9, [1, 1, 2], 4>;
defm : X86WriteRes<WriteFVarBlendY, [GRTPort20_21, GRTPort20_21_22], 6, [2, 6], 8>;
defm : X86WriteRes<WriteFVarBlendYLd, [GRTPort10_11, GRTPort20_21, GRTPort20_21_22], 9, [2, 2, 4], 8>;
defm : X86WriteResPairUnsupported<WriteFVarBlendZ>;
defm : GRTWriteResPair<WriteFVarShuffle, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteFVarShuffle256, [GRTPort20], 8, [2], 2, 10, 2>;
defm : GRTWriteResPair<WriteFVarShuffleY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteFVarShuffleZ>;
def : WriteRes<WriteFence, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
defm : X86WriteRes<WriteIDiv16, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteIDiv16Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteIDiv32, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteIDiv32Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteIDiv64, [GRTPort00_01_02_03, GRTPort01_02], 11, [2, 1], 3>;
defm : X86WriteRes<WriteIDiv64Ld, [GRTPort00_01_02_03, GRTPort01_02, GRTPort10_11, GRTPort30_31], 15, [1, 1, 1, 1], 4>;
defm : GRTWriteResPair<WriteIDiv8, [GRTPort01_02], 9, [1]>;
defm : GRTWriteResPair<WriteIMul16, [GRTPort00_01_02_03, GRTPort01_02], 8, [4, 1], 5>;
defm : X86WriteRes<WriteIMul16Imm, [GRTPort00_01_02_03, GRTPort01_02], 5, [2, 1], 3>;
defm : X86WriteRes<WriteIMul16ImmLd, [GRTPort00_01_02_03, GRTPort01_02], 5, [2, 1], 3>;
defm : GRTWriteResPair<WriteIMul16Reg, [GRTPort00_01_02_03, GRTPort01_02], 8, [4, 1], 5>;
defm : GRTWriteResPair<WriteIMul32, [GRTPort01_02], 3, [1]>;
def : WriteRes<WriteIMul32Imm, [GRTPort01_02]> {
  let Latency = 3;
}
def : WriteRes<WriteIMul32ImmLd, [GRTPort01_02]> {
  let Latency = 3;
}
defm : GRTWriteResPair<WriteIMul32Reg, [GRTPort01_02], 3, [1]>;
defm : GRTWriteResPair<WriteIMul64, [GRTPort01_02], 5, [1]>;
def : WriteRes<WriteIMul64Imm, [GRTPort01_02]> {
  let Latency = 5;
}
def : WriteRes<WriteIMul64ImmLd, [GRTPort01_02]> {
  let Latency = 5;
}
defm : GRTWriteResPair<WriteIMul64Reg, [GRTPort01_02], 5, [1]>;
defm : GRTWriteResPair<WriteIMul8, [GRTPort01_02], 3, [1]>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
// FIXME: Incompleted schedwrite.
defm : X86WriteResUnsupported<WriteIMulHLd>;
defm : X86WriteRes<WriteJump, [GRTPort08_09, GRTPort12_13, GRTPort30_31], 1, [1, 1, 1], 3>;
def : WriteRes<WriteJumpLd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteLAHFSAHF, [GRTPort00_01_02_03]>;
def : WriteRes<WriteLDMXCSR, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteLEA, [GRTPort00_01_02_03]>;
defm : GRTWriteResPair<WriteLZCNT, [GRTPort01], 3, [1]>;
def : WriteRes<WriteLoad, [GRTPort10_11]> {
  let Latency = 4;
}
def : WriteRes<WriteMMXMOVMSK, [GRTPort20]> {
  let Latency = 5;
}
defm : GRTWriteResPair<WriteMPSAD, [GRTPort20, GRTPort20_21_22], 6, [1, 2], 3>;
defm : GRTWriteResPair<WriteMPSADY, [GRTPort20, GRTPort20_21_22], 6, [2, 4], 6, 4, 2>;
defm : X86WriteRes<WriteMULX32, [GRTPort01_02], 3, [1], 0>;
defm : X86WriteRes<WriteMULX32Ld, [GRTPort01_02, GRTPort10_11], 7, [1, 1], 2>;
defm : X86WriteRes<WriteMULX64, [GRTPort01_02], 5, [1], 0>;
defm : X86WriteRes<WriteMULX64Ld, [GRTPort01_02, GRTPort10_11], 9, [1, 1], 2>;
def : WriteRes<WriteMicrocoded, [GRTPort30_31]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteMove, [GRTPort00_01_02_03]>;
def : WriteRes<WriteNop, [GRTPort20_21]>;
def : WriteRes<WritePCmpEStrI, [GRTPort20_21_22]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpEStrILd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpEStrM, [GRTPort20_21_22]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpEStrMLd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpIStrI, [GRTPort20_21_22]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpIStrILd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpIStrM, [GRTPort20_21_22]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WritePCmpIStrMLd, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
defm : GRTWriteResPair<WritePHAdd, [GRTPort00_01_02_03, GRTPort20_21_22], 6, [1, 5], 6>;
defm : GRTWriteResPair<WritePHAddX, [GRTPort20_21_22], 7, [7], 7>;
defm : GRTWriteResPair<WritePHAddY, [GRTPort20_21_22], 7, [14], 14, 4, 2>;
defm : GRTWriteResPair<WritePHMINPOS, [GRTPort20], 4, [1], 1, 5>;
defm : GRTWriteResPair<WritePMULLD, [GRTPort20], 4, [1], 1, 5>;
defm : GRTWriteResPair<WritePMULLDY, [GRTPort20], 4, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WritePMULLDZ>;
defm : GRTWriteResPair<WritePOPCNT, [GRTPort01], 3, [1]>;
defm : GRTWriteResPair<WritePSADBW, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WritePSADBWX, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WritePSADBWY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WritePSADBWZ>;
defm : X86WriteRes<WriteRMW, [GRTPort08_09, GRTPort10_11], 1, [1, 1], 2>;
def : WriteRes<WriteRotate, [GRTPort01]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteRotateLd, [GRTPort01]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteRotateCL, [GRTPort01]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteRotateCLLd, [GRTPort01]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteSETCC, [GRTPort00_01_02_03]>;
defm : X86WriteRes<WriteSETCCStore, [GRTPort00_01_02_03, GRTPort08_09, GRTPort12_13], 1, [1, 1, 1], 3>;
def : WriteRes<WriteSHDmrcl, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteSHDmri, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteSHDrrcl, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : WriteRes<WriteSHDrri, [GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
defm : X86WriteRes<WriteSTMXCSR, [GRTPortInvalid, GRTPort08_09, GRTPort12_13, GRTPort20], 4, [1, 1, 1, 1], 4>;
def : WriteRes<WriteShift, [GRTPort00_01_02_03]>;
def : WriteRes<WriteShiftLd, [GRTPort00_01_02_03]> {
  let Latency = 6;
}
def : WriteRes<WriteShiftCL, [GRTPort00_01_02_03]>;
def : WriteRes<WriteShiftCLLd, [GRTPort00_01_02_03]> {
  let Latency = 6;
}
defm : GRTWriteResPair<WriteShuffle, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteShuffle256, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : GRTWriteResPair<WriteShuffleX, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteShuffleY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteShuffleZ>;
defm : X86WriteRes<WriteStore, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteStoreNT, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
def : WriteRes<WriteSystem, [GRTPortInvalid]> {
  let Latency = GRTModel.MaxLatency;
}
defm : GRTWriteResPair<WriteTZCNT, [GRTPort01], 3, [1]>;
defm : GRTWriteResPair<WriteVPMOV256, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteRes<WriteVarBlend, [GRTPort20_21, GRTPort20_21_22], 7, [1, 3], 4>;
defm : X86WriteRes<WriteVarBlendLd, [GRTPort10_11, GRTPort20_21, GRTPort20_21_22], 10, [1, 1, 2], 4>;
defm : X86WriteRes<WriteVarBlendY, [GRTPort20_21, GRTPort20_21_22], 7, [2, 6], 8>;
defm : X86WriteRes<WriteVarBlendYLd, [GRTPort10_11, GRTPort20_21, GRTPort20_21_22], 10, [2, 2, 4], 8>;
defm : X86WriteResPairUnsupported<WriteVarBlendZ>;
defm : GRTWriteResPair<WriteVarShuffle, [GRTPort20_21_22], 1, [1], 1, 5>;
// Warning: zero or negtive load latency.
defm : GRTWriteResPair<WriteVarShuffle256, [GRTPort20], 6, [2], 2, 0, 2>;
defm : GRTWriteResPair<WriteVarShuffleX, [GRTPort21], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVarShuffleY, [GRTPort21], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVarShuffleZ>;
defm : GRTWriteResPair<WriteVarVecShift, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVarVecShiftY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVarVecShiftZ>;
defm : GRTWriteResPair<WriteVecALU, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecALUX, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecALUY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVecALUZ>;
def : WriteRes<WriteVecExtract, [GRTPort20]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecExtractSt, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [1, 1, 1], 3>;
defm : GRTWriteResPair<WriteVecIMul, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecIMulX, [GRTPort20], 3, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecIMulY, [GRTPort20], 3, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVecIMulZ>;
def : WriteRes<WriteVecInsert, [GRTPort20]>;
defm : X86WriteRes<WriteVecInsertLd, [GRTPort10_11, GRTPort20_21_22], 6, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [GRTPort10_11]> {
  let Latency = 4;
}
def : WriteRes<WriteVecLoadNT, [GRTPort10_11]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecLoadNTY, [GRTPort10_11], 4, [2], 2>;
def : WriteRes<WriteVecLoadX, [GRTPort10_11]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecLoadY, [GRTPort10_11], 4, [2], 2>;
defm : GRTWriteResPair<WriteVecLogic, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecLogicX, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecLogicY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVecLogicZ>;
def : WriteRes<WriteVecMOVMSK, [GRTPort20]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecMOVMSKY, [GRTPort20], 4, [2], 2>;
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [GRTPort10_11, GRTPort20], 6, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [GRTPort10_11, GRTPort20], 6, [2, 2], 4>;
defm : X86WriteRes<WriteVecMaskedStore32, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [2, 2, 2], 6>;
defm : X86WriteRes<WriteVecMaskedStore64, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [GRTPort12_13, GRTPort20, GRTPort28_29], 1, [2, 2, 2], 6>;
def : WriteRes<WriteVecMove, [GRTPort20_21_22]>;
def : WriteRes<WriteVecMoveFromGpr, [GRTPort20]>;
def : WriteRes<WriteVecMoveToGpr, [GRTPort20]> {
  let Latency = 4;
}
def : WriteRes<WriteVecMoveX, [GRTPort20_21_22]>;
defm : X86WriteRes<WriteVecMoveY, [GRTPort20_21_22], 1, [2], 2>;
defm : X86WriteResUnsupported<WriteVecMoveZ>;
defm : GRTWriteResPair<WriteVecShift, [GRTPort20_21_22], 1, [1], 1, 5>;
def : WriteRes<WriteVecShiftImm, [GRTPort20_21_22]>;
def : WriteRes<WriteVecShiftImmX, [GRTPort20_21_22]>;
defm : X86WriteResUnsupported<WriteVecShiftImmXLd>;
defm : X86WriteRes<WriteVecShiftImmY, [GRTPort20_21_22], 1, [2], 2>;
defm : X86WriteResUnsupported<WriteVecShiftImmYLd>;
defm : X86WriteResPairUnsupported<WriteVecShiftImmZ>;
defm : GRTWriteResPair<WriteVecShiftX, [GRTPort20_21_22], 1, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecShiftY, [GRTPort20_21_22], 1, [2], 2, 5, 2>;
defm : X86WriteResPairUnsupported<WriteVecShiftZ>;
defm : X86WriteRes<WriteVecStore, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [GRTPort08_09, GRTPort12_13], 0, [2, 2], 4>;
defm : X86WriteRes<WriteVecStoreX, [GRTPort08_09, GRTPort12_13], 0, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [GRTPort08_09, GRTPort12_13], 0, [2, 2], 4>;
defm : GRTWriteResPair<WriteVecTest, [GRTPort21], 4, [1], 1, 5>;
defm : GRTWriteResPair<WriteVecTestY, [GRTPort21], 4, [2], 2, 5, 2>;
defm : X86WriteRes<WriteXCHG, [GRTPort00_01_02_03], 4, [4], 4>;
def : WriteRes<WriteZero, []>;

// Dead schedwrites that nobody uses.
defm : X86WriteResUnsupported<WriteBitTestSetRegLd>;

// Infered SchedWriteRes and InstRW definition.
def : InstRW<[WriteRotateCLLd], (instregex "^RC(L|R)(8|16|32|64)mCL$")>;
def : InstRW<[WriteRotateLd], (instregex "^RC(L|R)(8|16|32|64)m(1|i)$")>;

def GRTWriteResGroup0 : SchedWriteRes<[GRTPort00_01_02_03]>;
def : InstRW<[GRTWriteResGroup0], (instregex "^ADC16(ri|i16|ri8)$",
                                             "^ADC(32|64)(i32|ri8)$",
                                             "^ADC(8|32)ri$",
                                             "^ADC64ri32$",
                                             "^ADC8i8$",
                                             "^BTS(16|32|64)r(r|i8)$",
                                             "^ENDBR(32|64)$",
                                             "^MOV64(cr|rc|rd)$",
                                             "^MOV64dr$",
                                             "^NOOP$",
                                             "^RDSSP(D|Q)$",
                                             "^RO(L|R)(8|16|32|64)r(1|i|CL)$",
                                             "^VZEROALL$",
                                             "^VZEROUPPER$",
                                             "^XADD(16|32|64)rr$")>;

def GRTWriteResGroup1 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup1], (instregex "^ADC(8|16|32)mi$",
                                             "^ADC(16|32|64)mi8$",
                                             "^ADC64mi32$")>;

def GRTWriteResGroup2 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort10_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup2, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^SARX(32|64)rm$")>;
def : InstRW<[GRTWriteResGroup2, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                        "^AD(C|O)X(32|64)rm$")>;

def GRTWriteResGroup3 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort10_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup3], (instregex "^MOV(8|16)ao(16|32|64)$",
                                             "^MOV(8|16)rm$",
                                             "^MOV8rm_NOREX$",
                                             "^MOVBE(16|32|64)rm$",
                                             "^MOVSX(16|32|64)rm(8|16|32)$",
                                             "^MOVSX32rm8_NOREX$",
                                             "^MOVZX16rm(8|16)$",
                                             "^RORX(32|64)mi$",
                                             "^TEST(8|16|32)mi$",
                                             "^TEST64mi32$")>;
def : InstRW<[GRTWriteResGroup3, ReadAfterLd], (instregex "^(OR|ADD|CMP|SUB|XOR)(8|16|32|64)rm$",
                                                          "^AND(8|16|32|64)rm$",
                                                          "^ANDN(32|64)rm$")>;
def : InstRW<[GRTWriteResGroup3, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^SH(L|R)X(32|64)rm$",
                                                                                                                           "^TEST(8|16|32|64)mr$")>;

def GRTWriteResGroup4 : SchedWriteRes<[GRTPortInvalid, GRTPort10_11, GRTPort20, GRTPort21]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup4], (instregex "^(ADD|SUB)_FI(16|32)m$",
                                             "^FICOM(P?)(16|32)m$",
                                             "^SUBR_FI(16|32)m$")>;

def GRTWriteResGroup5 : SchedWriteRes<[GRTPort21]> {
  let Latency = 4;
}
def : InstRW<[GRTWriteResGroup5], (instregex "^(ADD|SUB)_FrST0$",
                                             "^FCOMPP$",
                                             "^SHA(1|256)MSG(1|2)rr$",
                                             "^SHA1NEXTErr$",
                                             "^SUBR_FrST0$")>;

def GRTWriteResGroup6 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let Latency = GRTModel.MaxLatency;
}
def : InstRW<[GRTWriteResGroup6], (instregex "^BT(C|R|S)(16|32|64)mr$",
                                             "^(CL|ST)(I|AC)$",
                                             "^CPUID$",
                                             "^ENCL(S|U|V)$",
                                             "^FARCALL64m$",
                                             "^FARJMP64m$",
                                             "^FNCLEX$",
                                             "^FNINIT$",
                                             "^(INT|RSM)$",
                                             "^IN(T3|VD)$",
                                             "^PAUSE$",
                                             "^P(OP|USH)F(16|64)$",
                                             "^(RD|WR)PKRUr$",
                                             "^RD(PM|TS)C$",
                                             "^RD(RAN|SEE)D(16|32|64)r$",
                                             "^RDTSCP$",
                                             "^SEAMRET$",
                                             "^SERIALIZE$",
                                             "^TDCALL$",
                                             "^VMFUNC$",
                                             "^VMLAUNCH$",
                                             "^VMRESUME$",
                                             "^WBINVD$",
                                             "^X(G|S)ETBV$",
                                             "^XRSTOR((64)?)$",
                                             "^XSAVE((C|64|C64|OPT)?)$",
                                             "^XSAVEOPT64$")>;
def : InstRW<[GRTWriteResGroup6, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(D|Q)PDrm$",
                                                                            "^VGATHERDPSrm$",
                                                                            "^VGATHERQPSYrm$",
                                                                            "^VPGATHER(DD|QQ|QDY)rm$",
                                                                            "^VPGATHERDQrm$")>;

def GRTWriteResGroup7 : SchedWriteRes<[GRTPortInvalid, GRTPort08_09, GRTPort10_11, GRTPort12_13, GRTPort30_31]> {
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[GRTWriteResGroup7], (instregex "^CALL64m$")>;

def GRTWriteResGroup8 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let Latency = 2;
}
def : InstRW<[GRTWriteResGroup8], (instregex "^C(DQ|QO)$",
                                             "^SAR(8|16|32|64)rCL$",
                                             "^SARX(32|64)rr$")>;

def GRTWriteResGroup9 : SchedWriteRes<[GRTPort08_09, GRTPort12_13]> {
  let Latency = 0;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup9], (instregex "^CLDEMOTE$",
                                             "^CLFLUSH((OPT)?)$",
                                             "^CLWB$",
                                             "^MOV(8|16|32)o16a$",
                                             "^MOV(8|16|32|64)o(32|64)a$",
                                             "^SFENCE$")>;

def GRTWriteResGroup10 : SchedWriteRes<[GRTPort30_31]> {
  let Latency = GRTModel.MaxLatency;
}
def : InstRW<[GRTWriteResGroup10], (instregex "^CLTS$",
                                              "^GETSEC$",
                                              "^IRET(16|32|64)$",
                                              "^L(AR|SL)(16|32|64)rr$",
                                              "^L(F|G|S)S(32|64)rm$",
                                              "^LMSW16r$",
                                              "^MOV16sm$",
                                              "^PCONFIG$",
                                              "^POP(F|G)S(16|64)$",
                                              "^(RD|WR)MSR$",
                                              "^(V|SEA)MCALL$",
                                              "^SEAMOPS$",
                                              "^SYSENTER$",
                                              "^SYSEXIT$",
                                              "^VER(R|W)r$",
                                              "^VMREAD64(m|r)r$",
                                              "^VMWRITE64r(m|r)$",
                                              "^VMXOFF$")>;

def GRTWriteResGroup11 : SchedWriteRes<[GRTPort01, GRTPort10_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup11], (instregex "^CMP(8|16|32)mi$",
                                              "^CMP(16|32|64)mi8$",
                                              "^CMP64mi32$")>;
def : InstRW<[GRTWriteResGroup11, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^CMP(8|16|32|64)mr$")>;

def GRTWriteResGroup12 : SchedWriteRes<[GRTPortInvalid]> {
  let Latency = GRTModel.MaxLatency;
}
def : InstRW<[GRTWriteResGroup12], (instregex "^CMPXCHG(8|16)B$",
                                              "^IST(T?)_FP(16|32|64)m$",
                                              "^IST_F(16|32)m$",
                                              "^LCMPXCHG16B$")>;

def GRTWriteResGroup13 : SchedWriteRes<[GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort12_13]> {
  let ReleaseAtCycles = [1, 5, 1, 1, 1];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[GRTWriteResGroup13], (instregex "^CMPXCHG8rm$",
                                              "^LCMPXCHG8$")>;

def GRTWriteResGroup14 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let ReleaseAtCycles = [3];
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup14], (instregex "^CWD$")>;

def GRTWriteResGroup15 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup15], (instregex "^DIV(R?)_F32m$",
                                              "^DIV_F64m$")>;

def GRTWriteResGroup16 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup16], (instregex "^DIVR_F64m$",
                                              "^ILD_F(16|32|64)m$",
                                              "^VPDP(BU|WS)SD(S?)rm$")>;
def : InstRW<[GRTWriteResGroup16, ReadAfterVecXLd], (instregex "^SHA1RNDS4rmi$",
                                                               "^SHA256RNDS2rm$",
                                                               "^VGF2P8MULBrm$",
                                                               "^VPCLMULQDQrm$",
                                                               "^VPCMPGTQrm$")>;

def GRTWriteResGroup17 : SchedWriteRes<[GRTPort20_21]>;
def : InstRW<[GRTWriteResGroup17], (instregex "^F(DE|IN)CSTP$",
                                              "^FFREE$",
                                              "^(LD|ST)_Frr$",
                                              "^ST_FPrr$")>;

def GRTWriteResGroup18 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort20_21]> {
  let ReleaseAtCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup18], (instregex "^FFREEP$")>;

def GRTWriteResGroup19 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort20]> {
  let ReleaseAtCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup19], (instregex "^FNSTSW16r$")>;

def GRTWriteResGroup20 : SchedWriteRes<[GRTPort10_11, GRTPort30_31]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup20], (instregex "^JMP64m$",
                                              "^PTWRITE((64)?)m$")>;

def GRTWriteResGroup21 : SchedWriteRes<[GRTPort30_31]>;
def : InstRW<[GRTWriteResGroup21], (instregex "^JMP64r$",
                                              "^PTWRITE((64)?)r$")>;

def GRTWriteResGroup22 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort10_11]> {
  let ReleaseAtCycles = [3, 1];
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup22], (instregex "^LEAVE64$",
                                              "^POP16r$")>;

def GRTWriteResGroup23 : SchedWriteRes<[GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort12_13]> {
  let ReleaseAtCycles = [1, 2, 1, 1, 1];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[GRTWriteResGroup23], (instregex "^LXADD(8|16|32|64)$")>;

def GRTWriteResGroup24 : SchedWriteRes<[GRTPort08_09, GRTPort12_13]> {
  let Latency = GRTModel.MaxLatency;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup24], (instregex "^MMX_MASKMOVQ64$")>;

def GRTWriteResGroup25 : SchedWriteRes<[GRTPort20]> {
  let Latency = 5;
}
def : InstRW<[GRTWriteResGroup25], (instregex "^MMX_MOVD64to64rr$",
                                              "^MUL_FrST0$",
                                              "^VCVTSD2SIrr_Int$",
                                              "^VEXTRACTPSrr$",
                                              "^VMOVMSKP(D|S)rr$")>;
def : InstRW<[GRTWriteResGroup25, ReadDefault], (instregex "^VCVTSD2SIrr$")>;

def GRTWriteResGroup26 : SchedWriteRes<[GRTPort20_21_22]> {
  let Latency = 5;
}
def : InstRW<[GRTWriteResGroup26], (instregex "^MMX_MOVQ2DQrr$",
                                              "^MMX_MOVQ2FR64rr$")>;

def GRTWriteResGroup27 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort10_11, GRTPort20_21_22]> {
  let ReleaseAtCycles = [1, 1, 3];
  let Latency = 8;
  let NumMicroOps = 5;
}
def : InstRW<[GRTWriteResGroup27, ReadAfterVecLd], (instregex "^MMX_PH(ADD|SUB)Drm$")>;
def : InstRW<[GRTWriteResGroup27, ReadAfterVecXLd], (instregex "^VPH(ADD|SUB)Drm$")>;

def GRTWriteResGroup28 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort20_21_22]> {
  let ReleaseAtCycles = [1, 3];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup28], (instregex "^MMX_PH(ADD|SUB)Drr$",
                                              "^VPH(ADD|SUB)Drr$")>;

def GRTWriteResGroup29 : SchedWriteRes<[GRTPortInvalid, GRTPort01, GRTPort08_09, GRTPort12_13]> {
  let Latency = 3;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup29], (instregex "^MOV16ms$")>;

def GRTWriteResGroup30 : SchedWriteRes<[GRTPort10_11]> {
  let Latency = 4;
}
def : InstRW<[GRTWriteResGroup30], (instregex "^MOV32ao(16|32|64)$",
                                              "^MOV64ao(32|64)$",
                                              "^MOVZX(32|64)rm(8|16)$",
                                              "^MOVZX32rm8_NOREX$")>;

def GRTWriteResGroup31 : SchedWriteRes<[GRTPort01]> {
  let Latency = 3;
}
def : InstRW<[GRTWriteResGroup31], (instregex "^MOV64rs$",
                                              "^P(DEP|EXT)(32|64)rr$")>;

def GRTWriteResGroup32 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort08_09, GRTPort12_13]> {
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup32], (instregex "^MOVBE(16|32|64)mr$")>;

def GRTWriteResGroup33 : SchedWriteRes<[GRTPortInvalid, GRTPort10_11, GRTPort20]> {
  let ReleaseAtCycles = [1, 1, 2];
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup33], (instregex "^MUL_FI(16|32)m$")>;

def GRTWriteResGroup34 : SchedWriteRes<[GRTPort01, GRTPort10_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup34, ReadAfterLd], (instregex "^P(DEP|EXT)(32|64)rm$")>;

def GRTWriteResGroup35 : SchedWriteRes<[GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort12_13, GRTPort30_31]> {
  let ReleaseAtCycles = [1, 2, 1, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[GRTWriteResGroup35], (instregex "^POP16rmm$")>;

def GRTWriteResGroup36 : SchedWriteRes<[GRTPort08_09, GRTPort10_11]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup36], (instregex "^PREFETCHT0$",
                                              "^PUSH(16|64)rmm$",
                                              "^SH(L|R)D32mri8$")>;

def GRTWriteResGroup37 : SchedWriteRes<[GRTPort01, GRTPort08_09, GRTPort12_13, GRTPort30_31]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup37], (instregex "^PUSH(F|G)S(16|64)$")>;

def GRTWriteResGroup38 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let Latency = 6;
}
def : InstRW<[GRTWriteResGroup38, WriteRMW], (instregex "^RO(L|R)(8|16|32|64)m(1|i|CL)$")>;

def GRTWriteResGroup39 : SchedWriteRes<[GRTPort10_11, GRTPort21]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup39, ReadAfterVecXLd], (instregex "^SHA(1|256)MSG(1|2)rm$",
                                                               "^SHA1NEXTErm$")>;

def GRTWriteResGroup40 : SchedWriteRes<[GRTPort20]> {
  let Latency = 4;
}
def : InstRW<[GRTWriteResGroup40], (instregex "^SHA1RNDS4rri$",
                                              "^SHA256RNDS2rr$",
                                              "^VGF2P8MULBrr$",
                                              "^VPCLMULQDQrr$",
                                              "^VPCMPGTQrr$",
                                              "^VPDP(BU|WS)SD(S?)rr$")>;
def : InstRW<[GRTWriteResGroup40, ReadDefault, ReadInt2Fpu], (instregex "^VPINSRWrr$")>;

def GRTWriteResGroup41 : SchedWriteRes<[GRTPortInvalid, GRTPort00_01_02_03, GRTPort08_09, GRTPort10_11, GRTPort12_13]> {
  let ReleaseAtCycles = [1, 3, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[GRTWriteResGroup41], (instregex "^SH(L|R)D32mrCL$")>;

def GRTWriteResGroup42 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let ReleaseAtCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup42], (instregex "^SH(L|R)D32rrCL$")>;

def GRTWriteResGroup43 : SchedWriteRes<[GRTPortInvalid]> {
  let Latency = 0;
}
def : InstRW<[GRTWriteResGroup43], (instregex "^SH(L|R)D32rri8$")>;

def GRTWriteResGroup44 : SchedWriteRes<[GRTPort12_13, GRTPort20, GRTPort28_29]> {
  let NumMicroOps = 3;
}
def : InstRW<[GRTWriteResGroup44], (instregex "^ST_F(P?)(32|64)m$")>;

def GRTWriteResGroup45 : SchedWriteRes<[GRTPort01]> {
  let Latency = GRTModel.MaxLatency;
}
def : InstRW<[GRTWriteResGroup45], (instregex "^SWAPGS$")>;

def GRTWriteResGroup46 : SchedWriteRes<[GRTPort10_11, GRTPort20_21]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup46, ReadAfterVecLd], (instregex "^V(ADD|SUB)SSrm$",
                                                              "^V(ADD|SUB)SSrm_Int$")>;
def : InstRW<[GRTWriteResGroup46, ReadAfterVecXLd], (instregex "^VAES(DE|EN)CLASTrm$",
                                                               "^VAES(DE|EN)Crm$")>;

def GRTWriteResGroup47 : SchedWriteRes<[GRTPort20_21]> {
  let Latency = 3;
}
def : InstRW<[GRTWriteResGroup47], (instregex "^VAES(DE|EN)CLASTrr$",
                                              "^VAES(DE|EN)Crr$")>;

def GRTWriteResGroup48 : SchedWriteRes<[GRTPort10_11, GRTPort20_21_22]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup48], (instregex "^VBROADCAST(F|I)128$",
                                              "^VBROADCASTS(D|S)Yrm$",
                                              "^VPBROADCAST(B|D|Q|W)Yrm$")>;

def GRTWriteResGroup49 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let Latency = 15;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup49, ReadAfterVecLd], (instregex "^VDIVSSrm$",
                                                              "^VDIVSSrm_Int$")>;

def GRTWriteResGroup50 : SchedWriteRes<[GRTPort20]> {
  let Latency = 10;
}
def : InstRW<[GRTWriteResGroup50], (instregex "^VDIVSSrr$",
                                              "^VDIVSSrr_Int$")>;

def GRTWriteResGroup51 : SchedWriteRes<[GRTPort12_13, GRTPort20, GRTPort28_29]> {
  let ReleaseAtCycles = [2, 2, 2];
  let NumMicroOps = 6;
}
def : InstRW<[GRTWriteResGroup51], (instregex "^VEXTRACT(F|I)128mr$")>;

def GRTWriteResGroup52 : SchedWriteRes<[GRTPort20]> {
  let ReleaseAtCycles = [2];
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup52], (instregex "^VEXTRACT(F|I)128rr$")>;

def GRTWriteResGroup53 : SchedWriteRes<[GRTPort00_01_02_03]> {
  let ReleaseAtCycles = [2];
  let Latency = GRTModel.MaxLatency;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup53, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(D|Q)PDYrm$",
                                                                             "^VGATHERDPSYrm$",
                                                                             "^VPGATHER(DD|QQ)Yrm$",
                                                                             "^VPGATHERDQYrm$")>;

def GRTWriteResGroup54 : SchedWriteRes<[GRTPort10_11, GRTPort20_21]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup54, ReadAfterVecXLd], (instregex "^VGF2P8AFFINE((INV)?)QBYrmi$")>;

def GRTWriteResGroup55 : SchedWriteRes<[GRTPort20_21]> {
  let ReleaseAtCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup55], (instregex "^VGF2P8AFFINE((INV)?)QBYrri$")>;

def GRTWriteResGroup56 : SchedWriteRes<[GRTPort10_11, GRTPort20_21]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup56, ReadAfterVecXLd], (instregex "^VGF2P8AFFINE((INV)?)QBrmi$")>;
def : InstRW<[GRTWriteResGroup56, ReadAfterVecLd], (instregex "^VMULSSrm$",
                                                              "^VMULSSrm_Int$")>;

def GRTWriteResGroup57 : SchedWriteRes<[GRTPort20_21]> {
  let Latency = 4;
}
def : InstRW<[GRTWriteResGroup57], (instregex "^VGF2P8AFFINE((INV)?)QBrri$")>;

def GRTWriteResGroup58 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup58], (instregex "^VPDP(BU|WS)SD(S?)Yrm$")>;
def : InstRW<[GRTWriteResGroup58, ReadAfterVecXLd], (instregex "^VGF2P8MULBYrm$")>;
def : InstRW<[GRTWriteResGroup58, ReadAfterVecYLd], (instregex "^VPCMPGTQYrm$")>;

def GRTWriteResGroup59 : SchedWriteRes<[GRTPort20]> {
  let ReleaseAtCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup59], (instregex "^VGF2P8MULBYrr$",
                                              "^VPCMPGTQYrr$",
                                              "^VPDP(BU|WS)SD(S?)Yrr$")>;

def GRTWriteResGroup60 : SchedWriteRes<[GRTPort12_13, GRTPort20, GRTPort28_29, GRTPort30_31]> {
  let ReleaseAtCycles = [2, 2, 2, 1];
  let Latency = 3;
  let NumMicroOps = 7;
}
def : InstRW<[GRTWriteResGroup60], (instregex "^VMASKMOVDQU64$")>;

def GRTWriteResGroup61 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup61, ReadAfterVecYLd], (instregex "^VPERM2(F|I)128rm$")>;

def GRTWriteResGroup62 : SchedWriteRes<[GRTPort20]> {
  let ReleaseAtCycles = [2];
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup62], (instregex "^VPERM2(F|I)128rr$")>;

def GRTWriteResGroup63 : SchedWriteRes<[GRTPort10_11, GRTPort20]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup63, ReadAfterVecYLd], (instregex "^VPERM(Q|PD)Ymi$")>;

def GRTWriteResGroup64 : SchedWriteRes<[GRTPort20]> {
  let ReleaseAtCycles = [2];
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GRTWriteResGroup64], (instregex "^VPERM(Q|PD)Yri$")>;

def GRTWriteResGroup65 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort10_11, GRTPort20_21_22]> {
  let ReleaseAtCycles = [2, 2, 6];
  let Latency = 8;
  let NumMicroOps = 10;
}
def : InstRW<[GRTWriteResGroup65, ReadAfterVecYLd], (instregex "^VPH(ADD|SUB)DYrm$")>;

def GRTWriteResGroup66 : SchedWriteRes<[GRTPort00_01_02_03, GRTPort20_21_22]> {
  let ReleaseAtCycles = [2, 6];
  let Latency = 4;
  let NumMicroOps = 8;
}
def : InstRW<[GRTWriteResGroup66], (instregex "^VPH(ADD|SUB)DYrr$")>;

def GRTWriteResGroup67 : SchedWriteRes<[GRTPortInvalid, GRTPort00_01_02_03, GRTPort12_13]> {
  let ReleaseAtCycles = [1, 2, 1];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[GRTWriteResGroup67, WriteRMW], (instregex "^X(ADD|CHG)(8|16|32|64)rm$")>;

}
