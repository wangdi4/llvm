//=- X86ScheduleGLC.td - X86 Goldencove Scheduling -----------*- tablegen -*-=//
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
// This file defines the machine model for Intel Goldencove to support
// instruction scheduling and other instruction cost heuristics.
//
// Automatically generated file by smg tool.
//
//===----------------------------------------------------------------------===//

def GLCModel : SchedMachineModel {
  // Gracemont can allocate 6 uops per cycle.
  let IssueWidth = 6; // Based on allocator width.
  let MicroOpBufferSize = 512; // Based on the reorder buffer.
  let LoadLatency = 5;
  let MispredictPenalty = 14;

  // Latency for microcoded instructions or instructions without latency info.
  int MaxLatency = 100;

  // Based on the LSD (loop-stream detector) queue size (ST).
  let LoopMicroOpBufferSize = 72;

  // This flag is set to allow the scheduler to assign a default model to
  // unrecognized opcodes.
  let CompleteModel = 0;
}

let SchedModel = GLCModel in {

// Gracemont can issue micro-ops to 17 different ports in one cycle.
def GLCPort00 : ProcResource<1>;
def GLCPort01 : ProcResource<1>;
def GLCPort02 : ProcResource<1>;
def GLCPort03 : ProcResource<1>;
def GLCPort04 : ProcResource<1>;
def GLCPort05 : ProcResource<1>;
def GLCPort06 : ProcResource<1>;
def GLCPort07 : ProcResource<1>;
def GLCPort08 : ProcResource<1>;
def GLCPort09 : ProcResource<1>;
def GLCPort10 : ProcResource<1>;
def GLCPort11 : ProcResource<1>;

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def GLCPortInvalid :ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
def GLCPort00_01          : ProcResGroup<[GLCPort00, GLCPort01]>;
def GLCPort00_05          : ProcResGroup<[GLCPort00, GLCPort05]>;
def GLCPort00_06          : ProcResGroup<[GLCPort00, GLCPort06]>;
def GLCPort00_01_05       : ProcResGroup<[GLCPort00, GLCPort01, GLCPort05]>;
def GLCPort00_01_05_06    : ProcResGroup<[GLCPort00, GLCPort01, GLCPort05]>;
def GLCPort00_05_06_10    : ProcResGroup<[GLCPort00, GLCPort05, GLCPort06, GLCPort10]>;
def GLCPort01_05          : ProcResGroup<[GLCPort01, GLCPort05]>;
def GLCPort01_05_10       : ProcResGroup<[GLCPort01, GLCPort05, GLCPort10]>;
def GLCPort02_03_11       : ProcResGroup<[GLCPort02, GLCPort03, GLCPort11]>;
def GLCPort07_08          : ProcResGroup<[GLCPort07, GLCPort08]>;

// EU has 112 reservation stations.
def GLCPort00_01_05_06_10 : ProcResGroup<[GLCPort00, GLCPort01, GLCPort05,
                                          GLCPort06, GLCPort10]> {
  let BufferSize = 112;
}

// STD has 48 reservation stations.
def GLCPort04_09          : ProcResGroup<[GLCPort04, GLCPort09]> {
  let BufferSize = 48;
}

// MEM has 72 reservation stations.
def GLCPort02_03_07_08_11 : ProcResGroup<[GLCPort02, GLCPort03, GLCPort07,
                                          GLCPort08, GLCPort11]> {
  let BufferSize = 72;
}

// Integer loads are 5 cycles, so ReadAfterLd registers needn't be available
// until 5 cycles after the memory operand.
def : ReadAdvance<ReadAfterLd, 5>;

// Vector loads are 6 cycles, so ReadAfterVec*Ld registers needn't be available
// until 6 cycles after the memory operand.
def : ReadAdvance<ReadAfterVecLd, 6>;
def : ReadAdvance<ReadAfterVecXLd, 6>;
def : ReadAdvance<ReadAfterVecYLd, 6>;

def : ReadAdvance<ReadInt2Fpu, 0>;

// Many SchedWrites are defined in pairs with and without a folded load.
// Instructions with folded loads are usually micro-fused, so they only appear
// as two micro-ops when queued in the reservation station.
// This multiclass defines the resource usage for variants with and without
// folded loads.
multiclass GLCWriteResPair<X86FoldableSchedWrite SchedRW,
                          list<ProcResourceKind> ExePorts,
                          int Lat, list<int> Res = [1], int UOps = 1,
                          int LoadLat = 5> {
  // Register variant is using a single cycle on ExePort.
  def : WriteRes<SchedRW, ExePorts> {
    let Latency = Lat;
    let ResourceCycles = Res;
    let NumMicroOps = UOps;
  }

  // Memory variant also uses a cycle on port 2/3/10 and adds LoadLat cycles to
  // the latency (default = 4).
  def : WriteRes<SchedRW.Folded, !listconcat([GLCPort02_03_11], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ResourceCycles = !listconcat([1], Res);
    let NumMicroOps = !add(UOps, 1);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [GLCPort00_06]>;
defm : X86WriteRes<WriteADCLd, [GLCPort00_01_05_06_10, GLCPort00_06], 7, [1, 1], 2>;
defm : GLCWriteResPair<WriteAESDecEnc, [GLCPort00_01], 3, [1], 1, 8>;
defm : GLCWriteResPair<WriteAESIMC, [GLCPort00_01], 6, [2], 2, 8>;
defm : X86WriteRes<WriteAESKeyGen, [GLCPort00, GLCPort00_01_05, GLCPort05], 20, [3, 2, 6], 11>;
defm : X86WriteRes<WriteAESKeyGenLd, [GLCPort00, GLCPort00_01_05, GLCPort02_03_11, GLCPort05], 25, [3, 1, 1, 6], 11>;
def : WriteRes<WriteALU, [GLCPort00_01_05_06_10]>;
def : WriteRes<WriteALULd, [GLCPort00_01_05_06_10]> {
  let Latency = 6;
}
defm : GLCWriteResPair<WriteBEXTR, [GLCPort00_06, GLCPort01], 2, [1, 1], 2>;
defm : GLCWriteResPair<WriteBLS, [GLCPort01_05_10], 1, [1]>;
defm : GLCWriteResPair<WriteBSF, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteBSR, [GLCPort01], 3, [1]>;
def : WriteRes<WriteBSWAP32, [GLCPort01_05]>;
defm : X86WriteRes<WriteBSWAP64, [GLCPort00_06, GLCPort01], 2, [1, 1], 2>;
defm : GLCWriteResPair<WriteBZHI, [GLCPort01], 1, [1]>;
def : WriteRes<WriteBitTest, [GLCPort01]>;
defm : X86WriteRes<WriteBitTestImmLd, [GLCPort01, GLCPort02_03_11], 6, [1, 1], 2>;
defm : X86WriteRes<WriteBitTestRegLd, [GLCPort00_01_05_06, GLCPort02_03_11], 6, [1, 1], 2>;
def : WriteRes<WriteBitTestSet, [GLCPort01]>;
def : WriteRes<WriteBitTestSetImmLd, [GLCPort01]> {
  let Latency = 5;
}
def : WriteRes<WriteBitTestSetRegLd, [GLCPort00_01_05_06]> {
  let Latency = 5;
}
defm : GLCWriteResPair<WriteBlend, [GLCPort00_01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteBlendY, [GLCPort00_01_05], 1, [1], 1, 7>;
defm : GLCWriteResPair<WriteCLMul, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteCMOV, [GLCPort00_06], 1, [1]>;
defm : X86WriteRes<WriteCMPXCHG, [GLCPort00_01_05_06, GLCPort00_06], 5, [3, 2], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08], 8, [1, 2, 1, 1, 1], 6>;
defm : GLCWriteResPair<WriteCRC32, [GLCPort01], 3, [1]>;
defm : X86WriteRes<WriteCvtI2PD, [GLCPort00_01, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDLd, [GLCPort00_01, GLCPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDY, [GLCPort00_01, GLCPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDYLd, [GLCPort00_01, GLCPort02_03_11], 13, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtI2PDZ>;
defm : X86WriteRes<WriteCvtI2PS, [GLCPort00, GLCPort00_01], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PSLd, [GLCPort00_01, GLCPort02_03_11], 10, [1, 1], 2>;
defm : GLCWriteResPair<WriteCvtI2PSY, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteCvtI2PSZ>;
defm : X86WriteRes<WriteCvtI2SD, [GLCPort00_01_05, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SDLd, [GLCPort01, GLCPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SS, [GLCPort00_01, GLCPort05], 6, [1, 2], 3>;
defm : X86WriteRes<WriteCvtI2SSLd, [GLCPort01, GLCPort02_03_11], 9, [1, 1], 2>;
defm : GLCWriteResPair<WriteCvtPD2I, [GLCPort00_01, GLCPort05], 5, [1, 1], 2, 6>;
defm : GLCWriteResPair<WriteCvtPD2IY, [GLCPort00_01, GLCPort05], 7, [1, 1], 2, 7>;
defm : X86WriteResPairUnsupported<WriteCvtPD2IZ>;
defm : GLCWriteResPair<WriteCvtPD2PS, [GLCPort00_01, GLCPort05], 5, [1, 1], 2, 6>;
defm : GLCWriteResPair<WriteCvtPD2PSY, [GLCPort00_01, GLCPort05], 7, [1, 1], 2, 1>;
defm : X86WriteResPairUnsupported<WriteCvtPD2PSZ>;
defm : X86WriteRes<WriteCvtPH2PS, [GLCPort00_01, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSLd, [GLCPort00_01, GLCPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSY, [GLCPort00_01, GLCPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSYLd, [GLCPort00_01, GLCPort02_03_11], 13, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPH2PSZ>;
defm : X86WriteRes<WriteCvtPS2I, [GLCPort00, GLCPort00_01_05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2ILd, [GLCPort00, GLCPort02_03_11], 9, [1, 1], 2>;
defm : GLCWriteResPair<WriteCvtPS2IY, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteCvtPS2IZ>;
defm : X86WriteRes<WriteCvtPS2PD, [GLCPort00_01, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDLd, [GLCPort00_01, GLCPort02_03_11], 10, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDY, [GLCPort00_01, GLCPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDYLd, [GLCPort00_01, GLCPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPS2PDZ>;
defm : X86WriteRes<WriteCvtPS2PH, [GLCPort00_01, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHSt, [GLCPort00_01, GLCPort04_09, GLCPort07_08], 5, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHY, [GLCPort00_01, GLCPort05], 3, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [GLCPort00_01, GLCPort04_09, GLCPort07_08], 5, [1, 1, 1], 3>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZ>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZSt>;
defm : GLCWriteResPair<WriteCvtSD2I, [GLCPort00, GLCPort00_01], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSD2SS, [GLCPort00_01_05, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSD2SSLd, [GLCPort00_01, GLCPort02_03_11, GLCPort05], 10, [1, 1, 1], 3>;
defm : GLCWriteResPair<WriteCvtSS2I, [GLCPort00, GLCPort00_01], 7, [1, 1], 2, 4>;
defm : X86WriteRes<WriteCvtSS2SD, [GLCPort00_01_05, GLCPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SDLd, [GLCPort00_01, GLCPort02_03_11], 10, [1, 1], 2>;
defm : GLCWriteResPair<WriteDPPD, [GLCPort00_01, GLCPort01_05], 9, [2, 1], 3, 6>;
defm : GLCWriteResPair<WriteDPPS, [GLCPort00_01, GLCPort05], 13, [3, 1], 4, 6>;
defm : GLCWriteResPair<WriteDPPSY, [GLCPort00_01, GLCPort05], 13, [3, 1], 4, 7>;
defm : X86WriteRes<WriteDiv16, [GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort01, GLCPort05, GLCPort06], 76, [7, 11, 1, 2, 8, 3], 32>;
defm : X86WriteRes<WriteDiv16Ld, [GLCPort00, GLCPort02_03_11], 29, [1, 1], 2>;
defm : X86WriteRes<WriteDiv32, [GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort01, GLCPort05, GLCPort06], 76, [7, 11, 1, 2, 8, 3], 32>;
defm : X86WriteRes<WriteDiv32Ld, [GLCPort00, GLCPort02_03_11], 29, [1, 1], 2>;
defm : X86WriteRes<WriteDiv64, [GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort01, GLCPort05, GLCPort06], 76, [7, 11, 1, 2, 8, 3], 32>;
defm : X86WriteRes<WriteDiv64Ld, [GLCPort00, GLCPort02_03_11], 29, [1, 1], 2>;
defm : GLCWriteResPair<WriteDiv8, [GLCPort00], 25, [1], 1, 4>;
defm : X86WriteRes<WriteEMMS, [GLCPort00_01_05_06, GLCPort00_05], 10, [1, 9], 10>;
def : WriteRes<WriteFAdd, [GLCPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFAddLd, [GLCPort00_01, GLCPort02_03_11], 9, [1, 1], 2>;
defm : GLCWriteResPair<WriteFAdd64, [GLCPort00_01], 4, [1]>;
defm : GLCWriteResPair<WriteFAdd64X, [GLCPort00_01], 2, [1], 1, 7>;
defm : GLCWriteResPair<WriteFAdd64Y, [GLCPort00_01], 2, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAdd64Z>;
defm : GLCWriteResPair<WriteFAddX, [GLCPort00_01], 2, [1], 1, 7>;
defm : GLCWriteResPair<WriteFAddY, [GLCPort00_01], 2, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAddZ>;
defm : GLCWriteResPair<WriteFBlend, [GLCPort00_01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteFBlendY, [GLCPort00_01_05], 1, [1], 1, 7>;
def : WriteRes<WriteFCMOV, [GLCPort01]> {
  let Latency = 3;
}
defm : GLCWriteResPair<WriteFCmp, [GLCPort00_01], 4, [1]>;
defm : GLCWriteResPair<WriteFCmp64, [GLCPort00_01], 4, [1]>;
defm : GLCWriteResPair<WriteFCmp64X, [GLCPort00_01], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFCmp64Y, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFCmp64Z>;
defm : GLCWriteResPair<WriteFCmpX, [GLCPort00_01], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFCmpY, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFCmpZ>;
def : WriteRes<WriteFCom, [GLCPort00]> {
  let Latency = 2;
}
defm : X86WriteRes<WriteFComLd, [GLCPort02_03_11, GLCPort05], 8, [1, 1], 2>;
defm : GLCWriteResPair<WriteFComX, [GLCPort00], 3, [1], 1, 4>;
defm : GLCWriteResPair<WriteFDiv, [GLCPort00], 15, [1], 1, 1>;
defm : GLCWriteResPair<WriteFDiv64, [GLCPort00], 14, [1]>;
defm : GLCWriteResPair<WriteFDiv64X, [GLCPort00], 14, [1], 1, 6>;
defm : GLCWriteResPair<WriteFDiv64Y, [GLCPort00], 14, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFDiv64Z>;
defm : GLCWriteResPair<WriteFDivX, [GLCPort00], 11, [1], 1, 6>;
defm : GLCWriteResPair<WriteFDivY, [GLCPort00], 11, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFDivZ>;
defm : GLCWriteResPair<WriteFHAdd, [GLCPort00_01, GLCPort05], 5, [1, 2], 3, 6>;
defm : GLCWriteResPair<WriteFHAddY, [GLCPort00_01, GLCPort05], 5, [1, 2], 3, 7>;
def : WriteRes<WriteFLD0, [GLCPort00_05]>;
defm : X86WriteRes<WriteFLD1, [GLCPort00_05], 1, [2], 2>;
defm : X86WriteRes<WriteFLDC, [GLCPort00_05], 1, [2], 2>;
def : WriteRes<WriteFLoad, [GLCPort02_03_11]> {
  let Latency = 6;
}
def : WriteRes<WriteFLoadX, [GLCPort02_03_11]> {
  let Latency = 6;
}
def : WriteRes<WriteFLoadY, [GLCPort02_03_11]> {
  let Latency = 7;
}
defm : GLCWriteResPair<WriteFLogic, [GLCPort00_01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteFLogicY, [GLCPort00_01_05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFLogicZ>;
defm : GLCWriteResPair<WriteFMA, [GLCPort00_01], 4, [1]>;
defm : GLCWriteResPair<WriteFMAX, [GLCPort00_01], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFMAY, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFMAZ>;
def : WriteRes<WriteFMOVMSK, [GLCPort00]>;
defm : X86WriteRes<WriteFMaskedLoad, [GLCPort00_01_05, GLCPort02_03_11], 7, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [GLCPort00_01_05, GLCPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedStore32, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
def : WriteRes<WriteFMoveX, [GLCPort00_01_05]> {
  let Latency = 0;
}
def : WriteRes<WriteFMoveY, [GLCPort00_01_05]> {
  let Latency = 0;
}
def : WriteRes<WriteFMul, [GLCPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteFMulLd, [GLCPort00_01, GLCPort02_03_11], 9, [1, 1], 2>;
defm : GLCWriteResPair<WriteFMul64, [GLCPort00_01], 4, [1]>;
defm : GLCWriteResPair<WriteFMul64X, [GLCPort00_01], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFMul64Y, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFMul64Z>;
defm : GLCWriteResPair<WriteFMulX, [GLCPort00_01], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFMulY, [GLCPort00_01], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFMulZ>;
defm : GLCWriteResPair<WriteFRcp, [GLCPort00], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFRcpX, [GLCPort00], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFRcpY, [GLCPort00], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFRcpZ>;
defm : GLCWriteResPair<WriteFRnd, [GLCPort00_01], 8, [2], 2, 6>;
defm : GLCWriteResPair<WriteFRndY, [GLCPort00_01], 8, [2], 2, 7>;
defm : X86WriteResPairUnsupported<WriteFRndZ>;
defm : GLCWriteResPair<WriteFRsqrt, [GLCPort00], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFRsqrtX, [GLCPort00], 4, [1], 1, 6>;
defm : GLCWriteResPair<WriteFRsqrtY, [GLCPort00], 4, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFRsqrtZ>;
defm : GLCWriteResPair<WriteFShuffle, [GLCPort05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteFShuffle256, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteFShuffleY, [GLCPort05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFShuffleZ>;
def : WriteRes<WriteFSign, [GLCPort00]>;
defm : GLCWriteResPair<WriteFSqrt, [GLCPort00], 12, [1], 1, 6>;
defm : GLCWriteResPair<WriteFSqrt64, [GLCPort00], 18, [1], 1, 6>;
defm : GLCWriteResPair<WriteFSqrt64X, [GLCPort00], 18, [1], 1, 6>;
defm : GLCWriteResPair<WriteFSqrt64Y, [GLCPort00], 18, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFSqrt64Z>;
def : WriteRes<WriteFSqrt80, [GLCPort00]> {
  let Latency = 21;
}
defm : GLCWriteResPair<WriteFSqrtX, [GLCPort00], 12, [1], 1, 6>;
defm : GLCWriteResPair<WriteFSqrtY, [GLCPort00], 12, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFSqrtZ>;
defm : X86WriteRes<WriteFStore, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreX, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : GLCWriteResPair<WriteFTest, [GLCPort00], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteFTestY, [GLCPort00], 5, [1], 1, 6>;
defm : GLCWriteResPair<WriteFVarBlend, [GLCPort00_01_05], 3, [3], 3>;
defm : GLCWriteResPair<WriteFVarBlendY, [GLCPort00_01_05], 3, [3], 3, 6>;
defm : X86WriteResPairUnsupported<WriteFVarBlendZ>;
defm : GLCWriteResPair<WriteFVarShuffle, [GLCPort05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteFVarShuffle256, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteFVarShuffleY, [GLCPort05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFVarShuffleZ>;
defm : X86WriteRes<WriteFence, [GLCPort00_01_05_06], 2, [2], 2>;
defm : X86WriteRes<WriteIDiv16, [GLCPort00, GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort05, GLCPort06], 102, [4, 34, 14, 2, 4, 8], 66>;
defm : X86WriteRes<WriteIDiv16Ld, [GLCPort00, GLCPort00_01_05_06, GLCPort02_03_11, GLCPort05], 28, [2, 1, 1, 4], 8>;
defm : X86WriteRes<WriteIDiv32, [GLCPort00, GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort05, GLCPort06], 102, [4, 34, 14, 2, 4, 8], 66>;
defm : X86WriteRes<WriteIDiv32Ld, [GLCPort00, GLCPort00_01_05_06, GLCPort02_03_11, GLCPort05], 28, [2, 1, 1, 4], 8>;
defm : X86WriteRes<WriteIDiv64, [GLCPort00, GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort05, GLCPort06], 102, [4, 34, 14, 2, 4, 8], 66>;
defm : X86WriteRes<WriteIDiv64Ld, [GLCPort00, GLCPort00_01_05_06, GLCPort02_03_11, GLCPort05], 28, [2, 1, 1, 4], 8>;
def : WriteRes<WriteIDiv8, [GLCPort00]> {
  let Latency = 25;
}
defm : X86WriteRes<WriteIDiv8Ld, [GLCPort00, GLCPort00_01_05_06, GLCPort02_03_11, GLCPort05], 28, [2, 1, 1, 4], 8>;
defm : GLCWriteResPair<WriteIMul16, [GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01], 4, [2, 1, 1], 4>;
defm : GLCWriteResPair<WriteIMul16Imm, [GLCPort00_01_05_06_10, GLCPort01], 4, [1, 1], 2>;
defm : GLCWriteResPair<WriteIMul16Reg, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteIMul32, [GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01], 4, [1, 1, 1], 3>;
defm : GLCWriteResPair<WriteIMul32Imm, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteIMul32Reg, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteIMul64, [GLCPort01, GLCPort05], 4, [1, 1], 2>;
defm : GLCWriteResPair<WriteIMul64Imm, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteIMul64Reg, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteIMul8, [GLCPort01], 3, [1]>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
def : WriteRes<WriteIMulHLd, []> {
  let Latency = 3;
}
defm : GLCWriteResPair<WriteJump, [GLCPort00_06], 1, [1]>;
def : WriteRes<WriteLAHFSAHF, [GLCPort00_06]>;
defm : X86WriteRes<WriteLDMXCSR, [GLCPort00, GLCPort00_01_05, GLCPort02_03_11, GLCPort06], 7, [1, 1, 1, 1], 4>;
def : WriteRes<WriteLEA, [GLCPort00_01_05_06_10]>;
defm : GLCWriteResPair<WriteLZCNT, [GLCPort01], 3, [1]>;
def : WriteRes<WriteLoad, [GLCPort02_03_11]> {
  let Latency = 5;
}
def : WriteRes<WriteMMXMOVMSK, [GLCPort00]>;
defm : GLCWriteResPair<WriteMPSAD, [GLCPort01_05, GLCPort05], 4, [1, 1], 2, 6>;
defm : GLCWriteResPair<WriteMPSADY, [GLCPort01_05, GLCPort05], 4, [1, 1], 2, 7>;
defm : GLCWriteResPair<WriteMULX32, [GLCPort00_01_05_06, GLCPort00_06, GLCPort01], 3, [1, 1, 1], 3>;
defm : GLCWriteResPair<WriteMULX64, [GLCPort01, GLCPort05], 3, [1, 1], 2>;
def : WriteRes<WriteMicrocoded, [GLCPort00_01_05_06]> {
  let Latency = GLCModel.MaxLatency;
}
def : WriteRes<WriteMove, [GLCPort00_01_05_06]>;
defm : X86WriteRes<WriteNop, [], 1, [], 0>;
defm : GLCWriteResPair<WritePCmpEStrI, [GLCPort00, GLCPort00_01_05_06, GLCPort05], 18, [4, 1, 3], 8, 6>;
defm : GLCWriteResPair<WritePCmpEStrM, [GLCPort00, GLCPort00_01_05, GLCPort00_01_05_06, GLCPort05], 19, [4, 1, 1, 3], 9, 6>;
defm : GLCWriteResPair<WritePCmpIStrI, [GLCPort00], 11, [3], 3>;
defm : GLCWriteResPair<WritePCmpIStrM, [GLCPort00], 10, [3], 3, 6>;
defm : GLCWriteResPair<WritePHAdd, [GLCPort00_05, GLCPort05], 3, [1, 2], 3>;
defm : GLCWriteResPair<WritePHAddX, [GLCPort00_01_05, GLCPort01_05], 2, [1, 2], 3, 6>;
defm : GLCWriteResPair<WritePHAddY, [GLCPort00_01_05, GLCPort01_05], 2, [1, 2], 3, 7>;
defm : GLCWriteResPair<WritePHMINPOS, [GLCPort00], 4, [1], 1, 6>;
defm : GLCWriteResPair<WritePMULLD, [GLCPort00_01], 10, [2], 2, 6>;
defm : GLCWriteResPair<WritePMULLDY, [GLCPort00_01], 10, [2], 2, 7>;
defm : X86WriteResPairUnsupported<WritePMULLDZ>;
defm : GLCWriteResPair<WritePOPCNT, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WritePSADBW, [GLCPort05], 3, [1], 1, 6>;
defm : GLCWriteResPair<WritePSADBWX, [GLCPort05], 3, [1], 1, 6>;
defm : GLCWriteResPair<WritePSADBWY, [GLCPort05], 3, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WritePSADBWZ>;
defm : X86WriteRes<WriteRMW, [GLCPort02_03_11, GLCPort04_09, GLCPort07_08], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteRotate, [GLCPort00_01_05_06, GLCPort00_06], 3, [2, 1], 3>;
defm : X86WriteRes<WriteRotateLd, [GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort07_08], 8, [2, 1, 1, 1], 5>;
defm : X86WriteRes<WriteRotateCL, [GLCPort00_06], 3, [2], 2>;
defm : X86WriteRes<WriteRotateCLLd, [GLCPort00_06], 8, [2], 2>;
def : WriteRes<WriteSETCC, [GLCPort00_06]>;
defm : X86WriteRes<WriteSETCCStore, [GLCPort00_06, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteSHDmrcl, [GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01, GLCPort02_03_11, GLCPort04_09, GLCPort07_08], 11, [1, 1, 1, 1, 1, 1], 6>;
defm : X86WriteRes<WriteSHDmri, [GLCPort00_01_05_06_10, GLCPort01, GLCPort02_03_11, GLCPort04_09, GLCPort07_08], 9, [1, 1, 1, 1, 1], 5>;
defm : X86WriteRes<WriteSHDrrcl, [GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01], 4, [1, 1, 1], 3>;
def : WriteRes<WriteSHDrri, [GLCPort01]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteSTMXCSR, [GLCPort00, GLCPort04_09, GLCPort06, GLCPort07_08], 2, [1, 1, 1, 1], 4>;
def : WriteRes<WriteShift, [GLCPort00_06]>;
def : WriteRes<WriteShiftLd, [GLCPort00_06]> {
  let Latency = 6;
}
defm : X86WriteRes<WriteShiftCL, [GLCPort00_06], 3, [2], 2>;
defm : X86WriteRes<WriteShiftCLLd, [GLCPort00_06], 8, [2], 2>;
defm : GLCWriteResPair<WriteShuffle, [GLCPort05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteShuffle256, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteShuffleX, [GLCPort01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteShuffleY, [GLCPort01_05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteShuffleZ>;
defm : X86WriteRes<WriteStore, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteStoreNT, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
def : WriteRes<WriteSystem, [GLCPort00_01_05_06]> {
  let Latency = GLCModel.MaxLatency;
}
defm : GLCWriteResPair<WriteTZCNT, [GLCPort01], 3, [1]>;
defm : GLCWriteResPair<WriteVPMOV256, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteVarBlend, [GLCPort00_01_05], 3, [3], 3>;
defm : GLCWriteResPair<WriteVarBlendY, [GLCPort00_01_05], 3, [3], 3, 6>;
defm : X86WriteResPairUnsupported<WriteVarBlendZ>;
defm : GLCWriteResPair<WriteVarShuffle, [GLCPort00, GLCPort05], 1, [1, 1], 2>;
defm : GLCWriteResPair<WriteVarShuffle256, [GLCPort05], 3, [1], 1, 7>;
defm : GLCWriteResPair<WriteVarShuffleX, [GLCPort01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVarShuffleY, [GLCPort01_05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteVarShuffleZ>;
defm : GLCWriteResPair<WriteVarVecShift, [GLCPort00_01], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVarVecShiftY, [GLCPort00_01], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteVarVecShiftZ>;
defm : GLCWriteResPair<WriteVecALU, [GLCPort00], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVecALUX, [GLCPort00_01], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVecALUY, [GLCPort00_01], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteVecALUZ>;
defm : X86WriteRes<WriteVecExtract, [GLCPort00, GLCPort01_05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecExtractSt, [GLCPort01_05, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : GLCWriteResPair<WriteVecIMul, [GLCPort00], 5, [1], 1, 4>;
defm : GLCWriteResPair<WriteVecIMulX, [GLCPort00_01], 5, [1], 1, 6>;
defm : GLCWriteResPair<WriteVecIMulY, [GLCPort00_01], 5, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteVecIMulZ>;
defm : X86WriteRes<WriteVecInsert, [GLCPort01_05, GLCPort05], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecInsertLd, [GLCPort01_05, GLCPort02_03_11], 7, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [GLCPort02_03_11]> {
  let Latency = 5;
}
def : WriteRes<WriteVecLoadNT, [GLCPort02_03_11]> {
  let Latency = 6;
}
def : WriteRes<WriteVecLoadNTY, [GLCPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadX, [GLCPort02_03_11]> {
  let Latency = 6;
}
def : WriteRes<WriteVecLoadY, [GLCPort02_03_11]> {
  let Latency = 7;
}
defm : GLCWriteResPair<WriteVecLogic, [GLCPort00_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVecLogicX, [GLCPort00_01_05], 1, [1], 1, 6>;
defm : GLCWriteResPair<WriteVecLogicY, [GLCPort00_01_05], 1, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteVecLogicZ>;
def : WriteRes<WriteVecMOVMSK, [GLCPort00]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMOVMSKY, [GLCPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [GLCPort00_01_05, GLCPort02_03_11], 7, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [GLCPort00_01_05, GLCPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedStore32, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [GLCPort00, GLCPort04_09, GLCPort07_08], 2, [1, 1, 1], 3>;
def : WriteRes<WriteVecMove, [GLCPort00_05]>;
def : WriteRes<WriteVecMoveFromGpr, [GLCPort05]>;
def : WriteRes<WriteVecMoveToGpr, [GLCPort00]>;
def : WriteRes<WriteVecMoveX, [GLCPort00_01_05]>;
def : WriteRes<WriteVecMoveY, [GLCPort00_01_05]> {
  let Latency = 0;
}
defm : GLCWriteResPair<WriteVecShift, [GLCPort00], 1, [1], 1, 6>;
def : WriteRes<WriteVecShiftImm, [GLCPort00]>;
def : WriteRes<WriteVecShiftImmX, [GLCPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmXLd>;
def : WriteRes<WriteVecShiftImmY, [GLCPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmYLd>;
defm : X86WriteResPairUnsupported<WriteVecShiftImmZ>;
defm : X86WriteRes<WriteVecShiftX, [GLCPort00_01, GLCPort01_05], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftXLd, [GLCPort00_01, GLCPort02_03_11], 7, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftY, [GLCPort00_01, GLCPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftYLd, [GLCPort00_01, GLCPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteVecShiftZ>;
defm : X86WriteRes<WriteVecStore, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreX, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [GLCPort04_09, GLCPort07_08], 1, [1, 1], 2>;
defm : GLCWriteResPair<WriteVecTest, [GLCPort00, GLCPort05], 4, [1, 1], 2>;
defm : GLCWriteResPair<WriteVecTestY, [GLCPort00, GLCPort05], 3, [1, 1], 2, 7>;
defm : X86WriteRes<WriteXCHG, [GLCPort00_01_05_06_10], 2, [3], 3>;
defm : X86WriteResUnsupported<WriteZero>;

// Dead schedwrites that nobody uses.

// Infered SchedWriteRes and InstRW definition.
def : InstRW<[WriteRotateLd], (instregex "^RC(L|R)(8|16|32|64)m(1|i)$")>;

def GLCWriteResGroup0 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup0], (instregex "^(ADC|SBB)16i16$",
                                             "^(ADC|SBB)(32|64)i32$",
                                             "^J(E|R)CXZ$")>;

def GLCWriteResGroup1 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 6;
}
def : InstRW<[GLCWriteResGroup1, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)mr$")>;

def GLCWriteResGroup2 : SchedWriteRes<[GLCPort00_06, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup2], (instregex "^RORX(32|64)mi$")>;
def : InstRW<[GLCWriteResGroup2, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^S(A|H)RX(32|64)rm$",
                                                                                                                           "^SHLX(32|64)rm$")>;
def : InstRW<[GLCWriteResGroup2, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                        "^AD(C|O)X(32|64)rm$")>;

def GLCWriteResGroup3 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 6;
}
def : InstRW<[GLCWriteResGroup3], (instregex "^(ADC|SBB)64mi32$")>;

def GLCWriteResGroup4 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup4], (instregex "^CMP(8|16|32)mi$",
                                             "^CMP(16|32|64)mi8$",
                                             "^MOV(8|16)rm$",
                                             "^MOVZX16rm8$",
                                             "^POP64r$",
                                             "^TEST(8|16|32)mi$")>;
def : InstRW<[GLCWriteResGroup4, ReadAfterLd], (instregex "^(OR|ADD|CMP|SUB|XOR)(8|16|32|64)rm$",
                                                          "^AND(8|16|32|64)rm$")>;
def : InstRW<[GLCWriteResGroup4, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^CMP(8|16|32|64)mr$",
                                                                                                                           "^TEST(8|16|32|64)mr$")>;

def GLCWriteResGroup5 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup5], (instregex "^(ADD|SUB)_F(32|64)m$",
                                             "^ILD_F(16|32|64)m$",
                                             "^SUBR_F(32|64)m$")>;
def : InstRW<[GLCWriteResGroup5, ReadAfterVecYLd], (instregex "^VPACK(S|U)S(DW|WB)Yrm$",
                                                              "^VPCMPGTQYrm$")>;

def GLCWriteResGroup6 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup6], (instregex "^(ADD|SUB)_FI(16|32)m$",
                                             "^SUBR_FI(16|32)m$")>;

def GLCWriteResGroup7 : SchedWriteRes<[GLCPort00_01_05_06]>;
def : InstRW<[GLCWriteResGroup7], (instregex "^ADJCALLSTACKDOWN64$",
                                             "^ADJCALLSTACKUP64$",
                                             "^CMP16i16$",
                                             "^CMP(8|16|32|64)rr_REV$",
                                             "^CMP(32|64)i32$",
                                             "^CMP64ri32$",
                                             "^MOV(8|16|32)ao16$",
                                             "^MOV(8|16|32|64)ao(32|64)$",
                                             "^MOV(8|16|32)o16a$",
                                             "^MOV(8|16|32|64)o(32|64)a$",
                                             "^MOV(32|64)ImmSExti8$",
                                             "^MOVSX16rr(16|32)$",
                                             "^MOVSX(32|64)rr32$",
                                             "^MOV(S|Z)X32rr8_NOREX$",
                                             "^MOVZX16rr16$",
                                             "^S(G|I)DT64m$",
                                             "^SMSW16m$",
                                             "^STRm$",
                                             "^SYSCALL$",
                                             "^TEST16i16$",
                                             "^TEST(32|64)i32$",
                                             "^TEST64ri32$")>;

def GLCWriteResGroup8 : SchedWriteRes<[GLCPort01_05_10, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup8, ReadAfterLd], (instregex "^ANDN(32|64)rm$")>;

def GLCWriteResGroup9 : SchedWriteRes<[GLCPort01_05_10]>;
def : InstRW<[GLCWriteResGroup9], (instregex "^ANDN(32|64)rr$",
                                             "^C(BW|DQE|WDE)$",
                                             "^MOVSX(16|32|64)rr8$",
                                             "^MOVSX(32|64)rr16$")>;

def GLCWriteResGroup10 : SchedWriteRes<[]> {
  let NumMicroOps = 0;
}
def : InstRW<[GLCWriteResGroup10], (instregex "^AVX2_SETALLONES$",
                                              "^(V|AVX)_SET0$",
                                              "^CLC$",
                                              "^FsFLD0F128$",
                                              "^FsFLD0S(D|S)$",
                                              "^LD_Fp(0|1)80$",
                                              "^MMX_SET0$",
                                              "^MOV32r0$",
                                              "^V_SETALLONES$")>;

def GLCWriteResGroup11 : SchedWriteRes<[GLCPort01]>;
def : InstRW<[GLCWriteResGroup11], (instregex "^BSWAP32r$")>;

def GLCWriteResGroup12 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup12], (instregex "^((FAR)?)CALL64m$",
                                              "^CALL64m_NT$",
                                              "^CALL64m_RVMARKER$")>;

def GLCWriteResGroup13 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort04_09, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup13], (instregex "^CALL64pcrel32$")>;

def GLCWriteResGroup14 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup14], (instregex "^CALL64r$")>;

def GLCWriteResGroup15 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup15], (instregex "^CALL64r_NT$",
                                              "^CALL64r_RVMARKER$")>;

def GLCWriteResGroup16 : SchedWriteRes<[GLCPort00_06]>;
def : InstRW<[GLCWriteResGroup16], (instregex "^C(DQ|QO|LAC)$",
                                              "^RO(L8|R16)ri$",
                                              "^STAC$")>;

def GLCWriteResGroup17 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort06]> {
  let ResourceCycles = [2, 1];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup17], (instregex "^CLD$")>;

def GLCWriteResGroup18 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort04_09, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup18], (instregex "^CLDEMOTE$",
                                              "^PUSH(16|64)r$",
                                              "^PUSH64i(8|32)$")>;

def GLCWriteResGroup19 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup19], (instregex "^CLFLUSH$")>;

def GLCWriteResGroup20 : SchedWriteRes<[GLCPort00_05_06_10, GLCPort04_09, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup20], (instregex "^CLFLUSHOPT$")>;

def GLCWriteResGroup21 : SchedWriteRes<[GLCPort01, GLCPort06]> {
  let ResourceCycles = [1, 2];
  let Latency = GLCModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup21], (instregex "^CLI$")>;

def GLCWriteResGroup22 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup22], (instregex "^CMP64mi32$",
                                              "^POP(16|64)rmr$",
                                              "^TEST64mi32$")>;

def GLCWriteResGroup23 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort05, GLCPort07_08]> {
  let ResourceCycles = [2, 6, 4, 1, 1, 4, 1];
  let Latency = 23;
  let NumMicroOps = 19;
}
def : InstRW<[GLCWriteResGroup23], (instregex "^CMPXCHG16B$")>;

def GLCWriteResGroup24 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 6;
}
def : InstRW<[GLCWriteResGroup24], (instregex "^CMPXCHG(8|16|32|64)rm$")>;

def GLCWriteResGroup25 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01_05, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [5, 4, 2, 1, 1, 1];
  let Latency = 16;
  let NumMicroOps = 14;
}
def : InstRW<[GLCWriteResGroup25], (instregex "^CMPXCHG8B$")>;

def GLCWriteResGroup26 : SchedWriteRes<[GLCPort05]>;
def : InstRW<[GLCWriteResGroup26], (instregex "^COM(P?)_FST0r$",
                                              "^UCOM_F(P?)r$",
                                              "^VPALIGNR(Y?)rri$",
                                              "^VPBROADCAST(B|D|Q|W)rr$")>;

def GLCWriteResGroup27 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort05, GLCPort06]> {
  let ResourceCycles = [5, 1, 1, 1];
  let Latency = 18;
  let NumMicroOps = 8;
}
def : InstRW<[GLCWriteResGroup27], (instregex "^CPUID$",
                                              "^RDTSC$")>;

def GLCWriteResGroup28 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup28], (instregex "^CWD$")>;

def GLCWriteResGroup29 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 27;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup29], (instregex "^DIVR_F(32|64)m$")>;

def GLCWriteResGroup30 : SchedWriteRes<[GLCPort00, GLCPort02_03_11, GLCPort05]> {
  let Latency = 30;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup30], (instregex "^DIVR_FI(16|32)m$")>;

def GLCWriteResGroup31 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 22;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup31], (instregex "^DIV_F(32|64)m$")>;

def GLCWriteResGroup32 : SchedWriteRes<[GLCPort00, GLCPort02_03_11, GLCPort05]> {
  let Latency = 25;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup32], (instregex "^DIV_FI(16|32)m$")>;

def GLCWriteResGroup33 : SchedWriteRes<[GLCPort00]> {
  let Latency = 20;
}
def : InstRW<[GLCWriteResGroup33], (instregex "^DIV_F(P?)rST0$",
                                              "^DIV_FST0r$")>;

def GLCWriteResGroup34 : SchedWriteRes<[GLCPort02_03_11, GLCPort06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup34], (instregex "^((FAR)?)JMP64m$",
                                              "^JMP64m_(NT|REX)$")>;

def GLCWriteResGroup35 : SchedWriteRes<[GLCPort04_09, GLCPort07_08]> {
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup35], (instregex "^FBSTPm$",
                                              "^(RD|WR)FLAGS64$",
                                              "^SFENCE$",
                                              "^VMPTRSTm$")>;

def GLCWriteResGroup36 : SchedWriteRes<[GLCPort00_05]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup36], (instregex "^FDECSTP$")>;

def GLCWriteResGroup37 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup37], (instregex "^FICOM(P?)(16|32)m$")>;

def GLCWriteResGroup38 : SchedWriteRes<[GLCPort00_05]>;
def : InstRW<[GLCWriteResGroup38], (instregex "^FINCSTP$",
                                              "^FNOP$",
                                              "^MMX_P(ADD|SUB)(B|D|Q|W)rr$")>;

def GLCWriteResGroup39 : SchedWriteRes<[GLCPort00, GLCPort00_05, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup39], (instregex "^FLDCW16m$")>;

def GLCWriteResGroup40 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort00_06, GLCPort02_03_11]> {
  let ResourceCycles = [2, 39, 5, 10, 8];
  let Latency = 62;
  let NumMicroOps = 64;
}
def : InstRW<[GLCWriteResGroup40], (instregex "^FLDENVm$")>;

def GLCWriteResGroup41 : SchedWriteRes<[GLCPort00_01_05_06]> {
  let ResourceCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup41], (instregex "^FNCLEX$")>;

def GLCWriteResGroup42 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_05, GLCPort05]> {
  let ResourceCycles = [6, 3, 6];
  let Latency = 75;
  let NumMicroOps = 15;
}
def : InstRW<[GLCWriteResGroup42], (instregex "^FNINIT$")>;

def GLCWriteResGroup43 : SchedWriteRes<[GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup43], (instregex "^FNSTCW16m$")>;

def GLCWriteResGroup44 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup44], (instregex "^FNSTSW16r$")>;

def GLCWriteResGroup45 : SchedWriteRes<[GLCPort00, GLCPort04_09, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup45], (instregex "^FNSTSWm$")>;

def GLCWriteResGroup46 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort04_09, GLCPort05, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [9, 30, 21, 1, 11, 16, 1, 11];
  let Latency = 106;
  let NumMicroOps = 100;
}
def : InstRW<[GLCWriteResGroup46], (instregex "^FSTENVm$")>;

def GLCWriteResGroup47 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort00_06, GLCPort01_05, GLCPort02_03_11, GLCPort06]> {
  let ResourceCycles = [4, 47, 1, 2, 1, 33, 2];
  let Latency = 63;
  let NumMicroOps = 90;
}
def : InstRW<[GLCWriteResGroup47], (instregex "^FXRSTOR$")>;

def GLCWriteResGroup48 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort00_06, GLCPort01_05, GLCPort02_03_11, GLCPort06]> {
  let ResourceCycles = [4, 45, 1, 2, 1, 31, 4];
  let Latency = 63;
  let NumMicroOps = 88;
}
def : InstRW<[GLCWriteResGroup48], (instregex "^FXRSTOR64$")>;

def GLCWriteResGroup49 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort05, GLCPort06]> {
  let ResourceCycles = [10, 4, 3, 1, 5];
  let Latency = 35;
  let NumMicroOps = 23;
}
def : InstRW<[GLCWriteResGroup49], (instregex "^IN(8|16|32)r(i|r)$")>;

def GLCWriteResGroup50 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort04_09, GLCPort05, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1, 1, 1];
  let Latency = 20;
  let NumMicroOps = 8;
}
def : InstRW<[GLCWriteResGroup50], (instregex "^INS(B|L|W)$")>;

def GLCWriteResGroup51 : SchedWriteRes<[GLCPort04_09, GLCPort05, GLCPort07_08]> {
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup51], (instregex "^IST(T?)_FP(16|32|64)m$",
                                              "^IST_F(16|32)m$")>;

def GLCWriteResGroup52 : SchedWriteRes<[GLCPort06]>;
def : InstRW<[GLCWriteResGroup52], (instregex "^JMP64r((_NT)?)$",
                                              "^JMP64r_REX$")>;

def GLCWriteResGroup53 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort01, GLCPort02_03_11, GLCPort06]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 9;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup53], (instregex "^L(AR|SL)(16|32|64)rm$")>;

def GLCWriteResGroup54 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort01, GLCPort06]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup54], (instregex "^LAR(16|32|64)rr$")>;

def GLCWriteResGroup55 : SchedWriteRes<[GLCPort02_03_11]> {
  let Latency = 7;
}
def : InstRW<[GLCWriteResGroup55], (instregex "^LD_F(32|64|80)m$",
                                              "^VBROADCAST(F|I)128$",
                                              "^VBROADCASTS(D|S)Yrm$",
                                              "^VMOV(D|SH|SL)DUPYrm$",
                                              "^VPBROADCAST(D|Q)Yrm$")>;

def GLCWriteResGroup56 : SchedWriteRes<[GLCPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup56], (instregex "^LEA16r$")>;

def GLCWriteResGroup57 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup57], (instregex "^LEAVE64$",
                                              "^POP16r$",
                                              "^XLAT$")>;

def GLCWriteResGroup58 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup58], (instregex "^LOCK_(OR|ADD|SUB|XOR)(8|16|32)mi$",
                                              "^LOCK_(OR|ADD|SUB|XOR)(16|32|64)mi8$",
                                              "^LOCK_(OR|ADD|SUB|XOR)(8|16|32|64)mr$",
                                              "^LOCK_(OR|ADD|SUB|XOR)64mi32$",
                                              "^LOCK_AND(8|16|32)mi$",
                                              "^LOCK_AND(16|32|64)mi8$",
                                              "^LOCK_AND(8|16|32|64)mr$",
                                              "^LOCK_AND64mi32$",
                                              "^LOCK_(DE|IN)C(8|16|32|64)m$",
                                              "^LXADD(8|16|32|64)$")>;

def GLCWriteResGroup59 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01_05, GLCPort06]> {
  let ResourceCycles = [2, 3, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[GLCWriteResGroup59], (instregex "^LOOP$")>;

def GLCWriteResGroup60 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06]> {
  let ResourceCycles = [9, 2];
  let Latency = 11;
  let NumMicroOps = 11;
}
def : InstRW<[GLCWriteResGroup60], (instregex "^LOOP(N?)E$")>;

def GLCWriteResGroup61 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort06]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup61], (instregex "^(L?)RET64$")>;

def GLCWriteResGroup62 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort07_08]> {
  let ResourceCycles = [2, 1];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup62], (instregex "^MFENCE$")>;

def GLCWriteResGroup63 : SchedWriteRes<[GLCPort00, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup63], (instregex "^MMX_MASKMOVQ64$")>;

def GLCWriteResGroup64 : SchedWriteRes<[GLCPort00_01_05, GLCPort00_05]> {
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup64], (instregex "^MMX_MOVDQ2Qrr$")>;

def GLCWriteResGroup65 : SchedWriteRes<[GLCPort00, GLCPort00_01_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup65], (instregex "^MMX_MOVQ2DQrr$")>;

def GLCWriteResGroup66 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup66, ReadAfterVecLd], (instregex "^MMX_PACKSS(DW|WB)rm$",
                                                              "^MMX_PACKUSWBrm$")>;

def GLCWriteResGroup67 : SchedWriteRes<[GLCPort05]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup67], (instregex "^MMX_PACKSS(DW|WB)rr$",
                                              "^MMX_PACKUSWBrr$")>;

def GLCWriteResGroup68 : SchedWriteRes<[GLCPort00_05, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup68, ReadAfterVecLd], (instregex "^MMX_P(ADD|SUB)(B|D|Q|W)rm$")>;

def GLCWriteResGroup69 : SchedWriteRes<[GLCPort00, GLCPort05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup69], (instregex "^MMX_PEXTRWrr$",
                                              "^VEXTRACTPSrr$",
                                              "^VPEXTRWrr_REV$")>;

def GLCWriteResGroup70 : SchedWriteRes<[GLCPort00, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup70, ReadAfterVecLd], (instregex "^MMX_PH(ADD|SUB)SWrm$")>;

def GLCWriteResGroup71 : SchedWriteRes<[GLCPort00, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup71], (instregex "^MMX_PH(ADD|SUB)SWrr$")>;

def GLCWriteResGroup72 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup72], (instregex "^VPBROADCAST(B|W)rm$")>;
def : InstRW<[GLCWriteResGroup72, ReadAfterLd], (instregex "^MMX_PINSRWrm$")>;
def : InstRW<[GLCWriteResGroup72, ReadAfterVecXLd], (instregex "^VPALIGNRrmi$")>;

def GLCWriteResGroup73 : SchedWriteRes<[GLCPort05]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup73, ReadDefault, ReadInt2Fpu], (instregex "^MMX_PINSRWrr$")>;

def GLCWriteResGroup74 : SchedWriteRes<[GLCPort01, GLCPort04_09, GLCPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup74], (instregex "^MOV16ms$")>;

def GLCWriteResGroup75 : SchedWriteRes<[GLCPort00_01_05_06_10]>;
def : InstRW<[GLCWriteResGroup75], (instregex "^MOV(8|16|32|64)ri$",
                                              "^MOV(8|16)rr$")>;

def GLCWriteResGroup76 : SchedWriteRes<[GLCPort00_01_05_06_10]> {
  let Latency = 0;
}
def : InstRW<[GLCWriteResGroup76], (instregex "^MOV(32|64)rr$",
                                              "^MOVZX(32|64)rr8$")>;

def GLCWriteResGroup77 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort01]> {
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup77], (instregex "^MOV(32|64)rs$")>;

def GLCWriteResGroup78 : SchedWriteRes<[GLCPort00_06, GLCPort04_09, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup78], (instregex "^MOVBE16mr$")>;

def GLCWriteResGroup79 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup79], (instregex "^MOVBE16rm$")>;

def GLCWriteResGroup80 : SchedWriteRes<[GLCPort01, GLCPort04_09, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup80], (instregex "^MOVBE32mr$")>;

def GLCWriteResGroup81 : SchedWriteRes<[GLCPort01, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup81], (instregex "^MOVBE32rm$")>;

def GLCWriteResGroup82 : SchedWriteRes<[GLCPort00_06, GLCPort01, GLCPort04_09, GLCPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup82], (instregex "^MOVBE64mr$")>;

def GLCWriteResGroup83 : SchedWriteRes<[GLCPort00_06, GLCPort01, GLCPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup83], (instregex "^MOVBE64rm$")>;

def GLCWriteResGroup84 : SchedWriteRes<[GLCPort02_03_11]> {
  let Latency = 5;
}
def : InstRW<[GLCWriteResGroup84], (instregex "^MOVSX16rm(16|32)$",
                                              "^MOVSX32rm32$",
                                              "^MOV(S|Z)X32rm8_NOREX$",
                                              "^MOVZX(16|32|64)rm16$",
                                              "^MOVZX(32|64)rm8$",
                                              "^VMOV(D|SH|SL)DUPrm$",
                                              "^VMOVS(D|S)rm_alt$")>;

def GLCWriteResGroup85 : SchedWriteRes<[GLCPort01_05_10, GLCPort02_03_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup85], (instregex "^MOVSX16rm8$")>;

def GLCWriteResGroup86 : SchedWriteRes<[GLCPort02_03_11]> {
  let Latency = 6;
}
def : InstRW<[GLCWriteResGroup86], (instregex "^MOVSX(32|64)rm(8|16)$",
                                              "^MOVSX64rm32$",
                                              "^VBROADCASTSSrm$",
                                              "^VMOVDI2PDIrm$",
                                              "^VMOVQI2PQIrm$",
                                              "^VPBROADCAST(D|Q)rm$")>;

def GLCWriteResGroup87 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01, GLCPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup87, WriteIMulHLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^MULX32rm$")>;

def GLCWriteResGroup88 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup88, WriteIMulH], (instregex "^MULX32rr$")>;

def GLCWriteResGroup89 : SchedWriteRes<[GLCPort01, GLCPort02_03_11, GLCPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup89, WriteIMulHLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^MULX64rm$")>;

def GLCWriteResGroup90 : SchedWriteRes<[GLCPort01, GLCPort05]> {
  let Latency = 4;
}
def : InstRW<[GLCWriteResGroup90, WriteIMulH], (instregex "^MULX64rr$")>;

def GLCWriteResGroup91 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup91], (instregex "^MUL_F(32|64)m$")>;

def GLCWriteResGroup92 : SchedWriteRes<[GLCPort00, GLCPort02_03_11, GLCPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup92], (instregex "^MUL_FI(16|32)m$")>;

def GLCWriteResGroup93 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort05, GLCPort06]> {
  let ResourceCycles = [7, 1, 2];
  let Latency = 20;
  let NumMicroOps = 10;
}
def : InstRW<[GLCWriteResGroup93], (instregex "^MWAITrr$")>;

def GLCWriteResGroup94 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort02_03_11, GLCPort05, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [10, 4, 2, 1, 5, 1];
  let Latency = 35;
  let NumMicroOps = 23;
}
def : InstRW<[GLCWriteResGroup94], (instregex "^OUT(8|16|32)(i|r)r$")>;

def GLCWriteResGroup95 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort06]> {
  let ResourceCycles = [3, 1];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup95], (instregex "^PAUSE$")>;

def GLCWriteResGroup96 : SchedWriteRes<[GLCPort01, GLCPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup96, ReadAfterLd], (instregex "^P(DEP|EXT)(32|64)rm$")>;

def GLCWriteResGroup97 : SchedWriteRes<[GLCPort01]> {
  let Latency = 3;
}
def : InstRW<[GLCWriteResGroup97], (instregex "^P(DEP|EXT)(32|64)rr$")>;

def GLCWriteResGroup98 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup98], (instregex "^P(OP|USH)16rmm$",
                                              "^PUSH64rmm$")>;

def GLCWriteResGroup99 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort07_08]> {
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup99], (instregex "^POP64rmm$")>;

def GLCWriteResGroup100 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort02_03_11, GLCPort06]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = GLCModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup100], (instregex "^PTWRITE((64)?)m$")>;

def GLCWriteResGroup101 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort06]> {
  let ResourceCycles = [1, 2];
  let Latency = GLCModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup101], (instregex "^PTWRITE64r$")>;

def GLCWriteResGroup102 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort06]> {
  let ResourceCycles = [2, 2];
  let Latency = GLCModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup102], (instregex "^PTWRITEr$")>;

def GLCWriteResGroup103 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort04_09, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup103], (instregex "^PUSH(16|64)rmr$",
                                               "^STOS(B|L|Q|W)$")>;

def GLCWriteResGroup104 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort04_09, GLCPort07_08]> {
  let ResourceCycles = [4, 1, 1];
  let Latency = 5;
  let NumMicroOps = 6;
}
def : InstRW<[GLCWriteResGroup104], (instregex "^PUSHF16$")>;

def GLCWriteResGroup105 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01, GLCPort04_09, GLCPort07_08]> {
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup105], (instregex "^PUSHF64$")>;

def GLCWriteResGroup106 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort01, GLCPort04_09, GLCPort07_08]> {
  let Latency = GLCModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup106], (instregex "^PUSH(F|G)S64$")>;

def GLCWriteResGroup107 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort01_05, GLCPort02_03_11, GLCPort07_08]> {
  let ResourceCycles = [1, 5, 1, 1, 1, 1];
  let Latency = 15;
  let NumMicroOps = 10;
}
def : InstRW<[GLCWriteResGroup107], (instregex "^RCL(8|16|32|64)mCL$")>;

def GLCWriteResGroup108 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01]> {
  let ResourceCycles = [2, 3, 2];
  let Latency = 11;
  let NumMicroOps = 7;
}
def : InstRW<[GLCWriteResGroup108], (instregex "^RC(L|R)(16|32|64)rCL$")>;

def GLCWriteResGroup109 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06]> {
  let ResourceCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup109], (instregex "^RC(L|R)(8|16|32|64)ri$")>;

def GLCWriteResGroup110 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort01_05]> {
  let ResourceCycles = [2, 5, 1, 1];
  let Latency = 11;
  let NumMicroOps = 9;
}
def : InstRW<[GLCWriteResGroup110], (instregex "^RCL8rCL$")>;

def GLCWriteResGroup111 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort01_05, GLCPort02_03_11, GLCPort07_08]> {
  let ResourceCycles = [2, 4, 2, 1, 1, 1];
  let Latency = 18;
  let NumMicroOps = 11;
}
def : InstRW<[GLCWriteResGroup111], (instregex "^RCR(8|16|32|64)mCL$")>;

def GLCWriteResGroup112 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort01_05]> {
  let ResourceCycles = [3, 4, 2, 1];
  let Latency = 14;
  let NumMicroOps = 10;
}
def : InstRW<[GLCWriteResGroup112], (instregex "^RCR8rCL$")>;

def GLCWriteResGroup113 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort05]> {
  let ResourceCycles = [20, 2];
  let Latency = 42;
  let NumMicroOps = 22;
}
def : InstRW<[GLCWriteResGroup113], (instregex "^RDTSCP$")>;

def GLCWriteResGroup114 : SchedWriteRes<[GLCPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup114, WriteRMW], (instregex "^RO(L|R)(8|16|32|64)m1$")>;

def GLCWriteResGroup115 : SchedWriteRes<[GLCPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup115, WriteRMW], (instregex "^RO(L|R)(8|16|32|64)mi$")>;

def GLCWriteResGroup116 : SchedWriteRes<[GLCPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup116], (instregex "^RO(L|R)(8|16|32|64)r1$")>;

def GLCWriteResGroup117 : SchedWriteRes<[GLCPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup117], (instregex "^ROL(16|32|64)ri$",
                                               "^ROR(32|64)ri$",
                                               "^SAHF$")>;

def GLCWriteResGroup118 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06]> {
  let ResourceCycles = [2, 1];
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup118], (instregex "^ROR8ri$")>;

def GLCWriteResGroup119 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup119], (instregex "^SCAS(B|L|Q|W)$")>;

def GLCWriteResGroup120 : SchedWriteRes<[GLCPort00_01_05, GLCPort02_03_11, GLCPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup120, ReadAfterVecXLd], (instregex "^SHA1MSG1rm$")>;

def GLCWriteResGroup121 : SchedWriteRes<[GLCPort00_01_05, GLCPort05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup121], (instregex "^SHA1MSG1rr$")>;

def GLCWriteResGroup122 : SchedWriteRes<[GLCPort00_01, GLCPort00_01_05, GLCPort01_05, GLCPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup122, ReadAfterVecXLd], (instregex "^SHA1NEXTErm$")>;

def GLCWriteResGroup123 : SchedWriteRes<[GLCPort00_01, GLCPort00_01_05, GLCPort01_05]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup123], (instregex "^SHA1NEXTErr$")>;

def GLCWriteResGroup124 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup124, ReadAfterVecXLd], (instregex "^SHA1RNDS4rmi$",
                                                                "^SHA256RNDS2rm$")>;

def GLCWriteResGroup125 : SchedWriteRes<[GLCPort05]> {
  let Latency = 4;
}
def : InstRW<[GLCWriteResGroup125], (instregex "^SHA1RNDS4rri$",
                                               "^SHA256RNDS2rr$")>;

def GLCWriteResGroup126 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup126, ReadAfterVecXLd], (instregex "^SHA256MSG2rm$")>;

def GLCWriteResGroup127 : SchedWriteRes<[GLCPort05]> {
  let ResourceCycles = [2];
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup127], (instregex "^SHA256MSG2rr$")>;

def GLCWriteResGroup128 : SchedWriteRes<[GLCPort00_01_05_06_10, GLCPort00_06, GLCPort01]> {
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup128], (instregex "^SHRD(16|32|64)rrCL$")>;

def GLCWriteResGroup129 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort01, GLCPort06]> {
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup129], (instregex "^SLDT(16|32|64)r$")>;

def GLCWriteResGroup130 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort06]> {
  let ResourceCycles = [5, 1];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[GLCWriteResGroup130], (instregex "^STD$")>;

def GLCWriteResGroup131 : SchedWriteRes<[GLCPort00_06, GLCPort01, GLCPort06]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup131], (instregex "^STR(16|32|64)r$")>;

def GLCWriteResGroup132 : SchedWriteRes<[GLCPort00_01]> {
  let Latency = 2;
}
def : InstRW<[GLCWriteResGroup132], (instregex "^V(ADD|SUB)S(D|S)rr_Int$")>;

def GLCWriteResGroup133 : SchedWriteRes<[GLCPort00_01]> {
  let Latency = 4;
}
def : InstRW<[GLCWriteResGroup133], (instregex "^V(ADD|MUL|SUB)SSrr$",
                                               "^VCVTDQ2PSrr$",
                                               "^VCVT(T?)PS2DQrr$",
                                               "^VMULSSrr_Int$")>;

def GLCWriteResGroup134 : SchedWriteRes<[GLCPort00_01, GLCPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup134], (instregex "^VCVT(T?)PS2DQrm$")>;
def : InstRW<[GLCWriteResGroup134, ReadAfterVecLd], (instregex "^V(CMP|MAX|MIN|MUL)S(D|S)rm_Int$",
                                                               "^VCVTSI((64)?)2SDrm_Int$",
                                                               "^VCVTSI2SSrm_Int$")>;
def : InstRW<[GLCWriteResGroup134, ReadAfterVecLd, ReadAfterVecLd], (instregex "^VF(N?)M(ADD|SUB)(132|213|231)S(D|S)m_Int$")>;

def GLCWriteResGroup135 : SchedWriteRes<[GLCPort00]> {
  let Latency = 2;
}
def : InstRW<[GLCWriteResGroup135], (instregex "^V(U?)COMIS(D|S)rr_Int$",
                                               "^VMOVSDto64rr$",
                                               "^VMOVSS2DIrr$",
                                               "^XAM_F((p80)?)$")>;

def GLCWriteResGroup136 : SchedWriteRes<[GLCPort00, GLCPort00_01]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup136], (instregex "^VCVTSD2SI((64)?)rr_Int$")>;

def GLCWriteResGroup137 : SchedWriteRes<[GLCPort00_01, GLCPort02_03_11, GLCPort05]> {
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup137, ReadAfterVecLd], (instregex "^VCVTS(D|I64)2SSrm_Int$")>;

def GLCWriteResGroup138 : SchedWriteRes<[GLCPort00_01, GLCPort05]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup138], (instregex "^VCVTSD2SSrr_Int$",
                                               "^VCVTSS2SDrr_Int$")>;
def : InstRW<[GLCWriteResGroup138, ReadDefault, ReadInt2Fpu], (instregex "^VCVTSI((64)?)2SDrr_Int$",
                                                                         "^VCVTSI2SSrr_Int$")>;

def GLCWriteResGroup139 : SchedWriteRes<[GLCPort00_01_05, GLCPort05]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup139, ReadDefault, ReadInt2Fpu], (instregex "^VCVTSI2SSrr$")>;

def GLCWriteResGroup140 : SchedWriteRes<[GLCPort00, GLCPort00_01]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup140, ReadDefault], (instregex "^VCVTSS2SI((64)?)rr$")>;

def GLCWriteResGroup141 : SchedWriteRes<[GLCPort00, GLCPort00_01, GLCPort05]> {
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup141], (instregex "^VCVT(T?)SS2SI64rr_Int$")>;

def GLCWriteResGroup142 : SchedWriteRes<[GLCPort00, GLCPort00_01, GLCPort05]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup142, ReadDefault], (instregex "^VCVTTSS2SI((64)?)rr$")>;

def GLCWriteResGroup143 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 20;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup143, ReadAfterVecLd], (instregex "^VDIVSDrm_Int$")>;

def GLCWriteResGroup144 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 17;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup144, ReadAfterVecLd], (instregex "^VDIVSSrm_Int$",
                                                               "^VSQRTSSm$")>;

def GLCWriteResGroup145 : SchedWriteRes<[GLCPort00]> {
  let Latency = 11;
}
def : InstRW<[GLCWriteResGroup145], (instregex "^VDIVSSrr$",
                                               "^VDIVSSrr_Int$")>;

def GLCWriteResGroup146 : SchedWriteRes<[GLCPort04_09, GLCPort05, GLCPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup146], (instregex "^VEXTRACTPSmr$")>;

def GLCWriteResGroup147 : SchedWriteRes<[GLCPort00, GLCPort00_01_05, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 4, 1];
  let Latency = 20;
  let NumMicroOps = 7;
}
def : InstRW<[GLCWriteResGroup147, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Yrm$",
                                                                              "^VGATHERDPSrm$",
                                                                              "^VGATHERQPDYrm$",
                                                                              "^VPGATHER(DD|QDY|QQY)rm$",
                                                                              "^VPGATHERDQYrm$")>;

def GLCWriteResGroup148 : SchedWriteRes<[GLCPort00, GLCPort00_01_05, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 2, 1];
  let Latency = 18;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup148, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)rm$",
                                                                              "^VGATHERQPDrm$",
                                                                              "^VPGATHER(D|Q)Qrm$",
                                                                              "^VPGATHERQDrm$")>;

def GLCWriteResGroup149 : SchedWriteRes<[GLCPort00, GLCPort00_01_05, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 8, 1];
  let Latency = 22;
  let NumMicroOps = 11;
}
def : InstRW<[GLCWriteResGroup149, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSYrm$",
                                                                              "^VPGATHERDDYrm$")>;

def GLCWriteResGroup150 : SchedWriteRes<[GLCPort00_01, GLCPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup150, ReadAfterVecXLd], (instregex "^VGF2P8AFFINE((INV)?)QB(Y?)rmi$",
                                                                "^VGF2P8MULB(Y?)rm$")>;

def GLCWriteResGroup151 : SchedWriteRes<[GLCPort00_01]> {
  let Latency = 3;
}
def : InstRW<[GLCWriteResGroup151], (instregex "^VGF2P8AFFINE((INV)?)QBYrri$",
                                               "^VGF2P8AFFINEINVQBrri$",
                                               "^VGF2P8MULBrr$")>;

def GLCWriteResGroup152 : SchedWriteRes<[GLCPort00_01, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup152, ReadAfterVecYLd], (instregex "^VH(ADD|SUB)PSYrm$")>;

def GLCWriteResGroup153 : SchedWriteRes<[GLCPort00_01, GLCPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup153], (instregex "^VH(ADD|SUB)PS(Y?)rr$")>;

def GLCWriteResGroup154 : SchedWriteRes<[GLCPort00_01, GLCPort02_03_11, GLCPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup154, ReadAfterVecXLd], (instregex "^VH(ADD|SUB)PSrm$")>;

def GLCWriteResGroup155 : SchedWriteRes<[GLCPort00_01_05, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup155, ReadAfterVecYLd], (instregex "^VINSERT(F|I)128rm$")>;
def : InstRW<[GLCWriteResGroup155, ReadAfterVecXLd], (instregex "^VP(ADD|SUB)(B|D|Q|W)rm$")>;

def GLCWriteResGroup156 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup156], (instregex "^VLDMXCSR$")>;

def GLCWriteResGroup157 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort01, GLCPort01_05, GLCPort02_03_11, GLCPort04_09, GLCPort05, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [8, 1, 1, 1, 1, 2, 3, 1];
  let Latency = 40;
  let NumMicroOps = 18;
}
def : InstRW<[GLCWriteResGroup157], (instregex "^VMCLEARm$")>;

def GLCWriteResGroup158 : SchedWriteRes<[GLCPort00_01_05]>;
def : InstRW<[GLCWriteResGroup158], (instregex "^VMOV(APD|DQA|DQU|UPS)Yrr_REV$",
                                               "^VMOV(APD|UPS)rr_REV$",
                                               "^VMOV(APS|UPD)(Y?)rr_REV$",
                                               "^VMOVS(D|S)rr$",
                                               "^VP(ADD|SUB)(D|Q|W|BY)rr$",
                                               "^VP(ADD|SUB)(B|DY|QY|WY)rr$")>;

def GLCWriteResGroup159 : SchedWriteRes<[GLCPort00_01_05]> {
  let Latency = 0;
}
def : InstRW<[GLCWriteResGroup159], (instregex "^VMOVDQ(A|U)rr$")>;

def GLCWriteResGroup160 : SchedWriteRes<[GLCPort01_05, GLCPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup160, ReadAfterVecXLd], (instregex "^VMOVLP(D|S)rm$",
                                                                "^VPBLENDWrmi$",
                                                                "^VSHUFP(D|S)rmi$")>;

def GLCWriteResGroup161 : SchedWriteRes<[GLCPort00]> {
  let Latency = 3;
}
def : InstRW<[GLCWriteResGroup161], (instregex "^VMOVMSKP(D|S)rr$",
                                               "^VMOVPDI2DIrr$",
                                               "^VMOVPQIto64rr$")>;

def GLCWriteResGroup162 : SchedWriteRes<[GLCPort01_05]>;
def : InstRW<[GLCWriteResGroup162], (instregex "^VMOVS(H|L)DUP(Y?)rr$",
                                               "^VPBLENDW(Y?)rri$",
                                               "^VSHUFP(S|DY)rri$",
                                               "^VSHUFP(D|SY)rri$")>;

def GLCWriteResGroup163 : SchedWriteRes<[GLCPort05]> {
  let Latency = 3;
}
def : InstRW<[GLCWriteResGroup163], (instregex "^VPACK(S|U)S(WB|DWY)rr$",
                                               "^VPACK(S|U)S(DW|WBY)rr$",
                                               "^VPCMPGTQ(Y?)rr$")>;

def GLCWriteResGroup164 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup164, ReadAfterVecXLd], (instregex "^VPACK(S|U)S(DW|WB)rm$",
                                                                "^VPCLMULQDQrm$",
                                                                "^VPCMPGTQrm$")>;

def GLCWriteResGroup165 : SchedWriteRes<[GLCPort00_01_05, GLCPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup165, ReadAfterVecYLd], (instregex "^VP(ADD|SUB)(B|D|Q|W)Yrm$")>;

def GLCWriteResGroup166 : SchedWriteRes<[GLCPort02_03_11, GLCPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup166], (instregex "^VPBROADCAST(B|W)Yrm$")>;
def : InstRW<[GLCWriteResGroup166, ReadAfterVecYLd], (instregex "^VPALIGNRYrmi$")>;

def GLCWriteResGroup167 : SchedWriteRes<[GLCPort01_05, GLCPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup167, ReadAfterVecYLd], (instregex "^VPBLENDWYrmi$",
                                                                "^VSHUFP(D|S)Yrmi$")>;

def GLCWriteResGroup168 : SchedWriteRes<[GLCPort00_01, GLCPort01_05, GLCPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup168, ReadAfterVecYLd], (instregex "^VPHADDSWYrm$")>;

def GLCWriteResGroup169 : SchedWriteRes<[GLCPort00_01, GLCPort01_05]> {
  let ResourceCycles = [1, 2];
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[GLCWriteResGroup169], (instregex "^VPH(ADD|SUB)SW(Y?)rr$")>;

def GLCWriteResGroup170 : SchedWriteRes<[GLCPort00_01, GLCPort01_05, GLCPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup170, ReadAfterVecXLd], (instregex "^VPH(ADD|SUB)SWrm$")>;

def GLCWriteResGroup171 : SchedWriteRes<[GLCPort00_01, GLCPort01_05, GLCPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[GLCWriteResGroup171, ReadAfterVecYLd], (instregex "^VPHSUBSWYrm$")>;

def GLCWriteResGroup172 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup172, ReadAfterVecLd], (instregex "^VRCPSSm$",
                                                               "^VRSQRT14SDZm_alt$",
                                                               "^VRSQRTSSm$")>;

def GLCWriteResGroup173 : SchedWriteRes<[GLCPort00, GLCPort02_03_11]> {
  let Latency = 23;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup173, ReadAfterVecLd], (instregex "^VSQRTSDm$")>;

def GLCWriteResGroup174 : SchedWriteRes<[GLCPort00_01_05_06]> {
  let ResourceCycles = [16];
  let Latency = 16;
  let NumMicroOps = 16;
}
def : InstRW<[GLCWriteResGroup174], (instregex "^VZEROALL$")>;

def GLCWriteResGroup175 : SchedWriteRes<[]> {
  let Latency = 0;
  let NumMicroOps = 0;
}
def : InstRW<[GLCWriteResGroup175], (instregex "^VZEROUPPER$")>;

def GLCWriteResGroup176 : SchedWriteRes<[GLCPort00_01_05_06]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup176], (instregex "^WAIT$",
                                               "^XGETBV$")>;

def GLCWriteResGroup177 : SchedWriteRes<[GLCPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[GLCWriteResGroup177, WriteRMW], (instregex "^XADD(8|16|32|64)rm$")>;

def GLCWriteResGroup178 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06, GLCPort06]> {
  let ResourceCycles = [3, 1, 1];
  let Latency = 10;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup178, WriteRMW], (instregex "^XCHG(8|16|32|64)rm$")>;

def GLCWriteResGroup179 : SchedWriteRes<[GLCPort00, GLCPort00_01_05_06, GLCPort00_05, GLCPort01, GLCPort05, GLCPort06]> {
  let ResourceCycles = [2, 4, 2, 1, 2, 4];
  let Latency = 17;
  let NumMicroOps = 15;
}
def : InstRW<[GLCWriteResGroup179], (instregex "^XCH_F$")>;

def GLCWriteResGroup180 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort01, GLCPort02_03_11, GLCPort06]> {
  let ResourceCycles = [21, 1, 1, 8];
  let Latency = 37;
  let NumMicroOps = 31;
}
def : InstRW<[GLCWriteResGroup180], (instregex "^XRSTOR((S|64|S64)?)$")>;

def GLCWriteResGroup181 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [26, 1, 1, 11, 1];
  let Latency = 42;
  let NumMicroOps = 40;
}
def : InstRW<[GLCWriteResGroup181], (instregex "^XSAVE((C|S|C64|S64)?)$")>;

def GLCWriteResGroup182 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [26, 1, 1, 10, 1];
  let Latency = 41;
  let NumMicroOps = 39;
}
def : InstRW<[GLCWriteResGroup182], (instregex "^XSAVE64$")>;

def GLCWriteResGroup183 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort02_03_11, GLCPort04_09, GLCPort06, GLCPort07_08]> {
  let ResourceCycles = [30, 1, 1, 11, 1];
  let Latency = 46;
  let NumMicroOps = 44;
}
def : InstRW<[GLCWriteResGroup183], (instregex "^XSAVEOPT((64)?)$")>;

def GLCWriteResGroup184 : SchedWriteRes<[GLCPort00_01_05_06, GLCPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[GLCWriteResGroup184], (instregex "^XSETBV$")>;

}
