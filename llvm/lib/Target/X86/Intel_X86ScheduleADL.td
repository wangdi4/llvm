//=- Intel_X86ScheduleADL.td - X86 Alderlake Scheduling ------*- tablegen -*-=//
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
// This file defines the machine model for Intel Alderlake to support
// instruction scheduling and other instruction cost heuristics.
//
// Automatically generated file by smg tool.
//
//===----------------------------------------------------------------------===//

def ADLModel : SchedMachineModel {
  // Alderlake can allocate 6 uops per cycle.
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

let SchedModel = ADLModel in {

// Alderlake can issue micro-ops to 17 different ports in one cycle.
def ADLPort00 : ProcResource<1>;
def ADLPort01 : ProcResource<1>;
def ADLPort02 : ProcResource<1>;
def ADLPort03 : ProcResource<1>;
def ADLPort04 : ProcResource<1>;
def ADLPort05 : ProcResource<1>;
def ADLPort06 : ProcResource<1>;
def ADLPort07 : ProcResource<1>;
def ADLPort08 : ProcResource<1>;
def ADLPort09 : ProcResource<1>;
def ADLPort10 : ProcResource<1>;
def ADLPort11 : ProcResource<1>;

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def ADLPortInvalid :ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
def ADLPort00_01          : ProcResGroup<[ADLPort00, ADLPort01]>;
def ADLPort00_01_05       : ProcResGroup<[ADLPort00, ADLPort01, ADLPort05]>;
def ADLPort00_01_05_06    : ProcResGroup<[ADLPort00, ADLPort01, ADLPort05, ADLPort06]>;
def ADLPort00_05          : ProcResGroup<[ADLPort00, ADLPort05]>;
def ADLPort00_05_06       : ProcResGroup<[ADLPort00, ADLPort05, ADLPort06]>;
def ADLPort00_06          : ProcResGroup<[ADLPort00, ADLPort06]>;
def ADLPort01_05          : ProcResGroup<[ADLPort01, ADLPort05]>;
def ADLPort01_05_10       : ProcResGroup<[ADLPort01, ADLPort05, ADLPort10]>;
def ADLPort02_03          : ProcResGroup<[ADLPort02, ADLPort03]>;
def ADLPort02_03_07       : ProcResGroup<[ADLPort02, ADLPort03, ADLPort07]>;
def ADLPort02_03_11       : ProcResGroup<[ADLPort02, ADLPort03, ADLPort11]>;
def ADLPort07_08          : ProcResGroup<[ADLPort07, ADLPort08]>;

// EU has 112 reservation stations.
def ADLPort00_01_05_06_10 : ProcResGroup<[ADLPort00, ADLPort01, ADLPort05,
                                          ADLPort06, ADLPort10]> {
  let BufferSize = 112;
}

// STD has 48 reservation stations.
def ADLPort04_09          : ProcResGroup<[ADLPort04, ADLPort09]> {
  let BufferSize = 48;
}

// MEM has 72 reservation stations.
def ADLPort02_03_07_08_11 : ProcResGroup<[ADLPort02, ADLPort03, ADLPort07,
                                          ADLPort08, ADLPort11]> {
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
multiclass ADLWriteResPair<X86FoldableSchedWrite SchedRW,
                          list<ProcResourceKind> ExePorts,
                          int Lat, list<int> Res = [1], int UOps = 1,
                          int LoadLat = 5> {
  // Register variant is using a single cycle on ExePort.
  def : WriteRes<SchedRW, ExePorts> {
    let Latency = Lat;
    let ResourceCycles = Res;
    let NumMicroOps = UOps;
  }

  // Memory variant also uses a cycle on port 2/3/11 and adds LoadLat cycles to
  // the latency (default = 4).
  def : WriteRes<SchedRW.Folded, !listconcat([ADLPort02_03_11], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ResourceCycles = !listconcat([1], Res);
    let NumMicroOps = !add(UOps, 1);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [ADLPort00_06]>;
defm : X86WriteRes<WriteADCLd, [ADLPort00_01_05_06_10, ADLPort00_06], 11, [1, 1], 2>;
defm : ADLWriteResPair<WriteAESDecEnc, [ADLPort00_01], 5, [1], 1, 7>;
defm : ADLWriteResPair<WriteAESIMC, [ADLPort00_01], 8, [2], 2, 7>;
defm : X86WriteRes<WriteAESKeyGen, [ADLPort00, ADLPort00_01, ADLPort00_01_05, ADLPort00_06, ADLPort01_05, ADLPort05], 7, [4, 1, 1, 2, 3, 3], 14>;
defm : X86WriteRes<WriteAESKeyGenLd, [ADLPort00, ADLPort00_01, ADLPort00_06, ADLPort01_05, ADLPort02_03_11, ADLPort05], 12, [4, 1, 2, 3, 1, 3], 14>;
def : WriteRes<WriteALU, [ADLPort00_01_05_06_10]>;
def : WriteRes<WriteALULd, [ADLPort00_01_05_06_10]> {
  let Latency = 11;
}
defm : ADLWriteResPair<WriteBEXTR, [ADLPort00_06, ADLPort01], 6, [1, 1], 2>;
defm : ADLWriteResPair<WriteBLS, [ADLPort01_05_10], 2, [1]>;
defm : ADLWriteResPair<WriteBSF, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteBSR, [ADLPort01], 3, [1]>;
def : WriteRes<WriteBSWAP32, [ADLPort01]>;
defm : X86WriteRes<WriteBSWAP64, [ADLPort00_06, ADLPort01], 2, [1, 1], 2>;
defm : ADLWriteResPair<WriteBZHI, [ADLPort01], 3, [1]>;
def : WriteRes<WriteBitTest, [ADLPort01]>;
defm : X86WriteRes<WriteBitTestImmLd, [ADLPort01, ADLPort02_03_11], 6, [1, 1], 2>;
defm : X86WriteRes<WriteBitTestRegLd, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11], 11, [4, 2, 1, 2, 1], 10>;
def : WriteRes<WriteBitTestSet, [ADLPort01]>;
def : WriteRes<WriteBitTestSetImmLd, [ADLPort01]> {
  let Latency = 11;
}
defm : X86WriteRes<WriteBitTestSetRegLd, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10], 17, [3, 2, 1, 2], 8>;
defm : ADLWriteResPair<WriteBlend, [ADLPort01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteBlendY, [ADLPort00_01_05], 1, [1], 1, 8>;
defm : ADLWriteResPair<WriteCLMul, [ADLPort05], 3, [1], 1, 7>;
defm : ADLWriteResPair<WriteCMOV, [ADLPort00_06], 1, [1], 1, 6>;
defm : X86WriteRes<WriteCMPXCHG, [ADLPort00_01_05_06_10, ADLPort00_06], 3, [3, 2], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08], 12, [1, 2, 1, 1, 1], 6>;
defm : ADLWriteResPair<WriteCRC32, [ADLPort01], 3, [1]>;
defm : X86WriteRes<WriteCvtI2PD, [ADLPort00_01, ADLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDY, [ADLPort00_01, ADLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDYLd, [ADLPort00_01, ADLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtI2PDZ>;
defm : ADLWriteResPair<WriteCvtI2PS, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteCvtI2PSY, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteCvtI2PSZ>;
defm : X86WriteRes<WriteCvtI2SD, [ADLPort00_01, ADLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SDLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SS, [ADLPort00_01, ADLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SSLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : ADLWriteResPair<WriteCvtPD2I, [ADLPort00_01, ADLPort05], 5, [1, 1], 2, 7>;
defm : ADLWriteResPair<WriteCvtPD2IY, [ADLPort00_01, ADLPort05], 7, [1, 1], 2, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPD2IZ>;
defm : ADLWriteResPair<WriteCvtPD2PS, [ADLPort00_01, ADLPort05], 5, [1, 1], 2, 7>;
defm : ADLWriteResPair<WriteCvtPD2PSY, [ADLPort00_01, ADLPort05], 7, [1, 1], 2, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPD2PSZ>;
defm : X86WriteRes<WriteCvtPH2PS, [ADLPort00_01, ADLPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSLd, [ADLPort00_01, ADLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSY, [ADLPort00_01, ADLPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSYLd, [ADLPort00_01, ADLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPH2PSZ>;
defm : ADLWriteResPair<WriteCvtPS2I, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteCvtPS2IY, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPS2IZ>;
defm : X86WriteRes<WriteCvtPS2PD, [ADLPort00_01, ADLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDY, [ADLPort00_01, ADLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDYLd, [ADLPort00_01, ADLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPS2PDZ>;
defm : X86WriteRes<WriteCvtPS2PH, [ADLPort00_01, ADLPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHSt, [ADLPort00_01, ADLPort04_09, ADLPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHY, [ADLPort00_01, ADLPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [ADLPort00_01, ADLPort04_09, ADLPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZ>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZSt>;
defm : ADLWriteResPair<WriteCvtSD2I, [ADLPort00, ADLPort00_01], 7, [1, 1], 2>;
defm : ADLWriteResPair<WriteCvtSD2SS, [ADLPort00_01, ADLPort05], 5, [1, 1], 2, 7>;
defm : ADLWriteResPair<WriteCvtSS2I, [ADLPort00, ADLPort00_01], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SD, [ADLPort00_01, ADLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SDLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : ADLWriteResPair<WriteDPPD, [ADLPort00_01, ADLPort01_05], 9, [2, 1], 3, 7>;
defm : ADLWriteResPair<WriteDPPS, [ADLPort00_01, ADLPort00_06, ADLPort01_05, ADLPort05], 14, [2, 1, 2, 1], 6, 7>;
defm : ADLWriteResPair<WriteDPPSY, [ADLPort00_01, ADLPort00_06, ADLPort01_05, ADLPort05], 14, [2, 1, 2, 1], 6, 8>;
defm : ADLWriteResPair<WriteDiv16, [ADLPort00_01_05_06_10, ADLPort01], 16, [1, 3], 4, 4>;
defm : ADLWriteResPair<WriteDiv32, [ADLPort00_01_05_06_10, ADLPort01], 15, [1, 3], 4, 4>;
defm : ADLWriteResPair<WriteDiv64, [ADLPort01], 18, [3], 3>;
defm : X86WriteRes<WriteDiv8, [ADLPort01], 17, [3], 3>;
defm : X86WriteRes<WriteDiv8Ld, [ADLPort01], 22, [3], 3>;
defm : X86WriteRes<WriteEMMS, [ADLPort00, ADLPort00_05, ADLPort00_06], 10, [1, 8, 1], 10>;
def : WriteRes<WriteFAdd, [ADLPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFAddLd, [ADLPort02_03, ADLPort05], 10, [1, 1], 2>;
defm : ADLWriteResPair<WriteFAdd64, [ADLPort01_05], 3, [1], 1, 7>;
defm : ADLWriteResPair<WriteFAdd64X, [ADLPort01_05], 3, [1], 1, 7>;
defm : ADLWriteResPair<WriteFAdd64Y, [ADLPort01_05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAdd64Z>;
defm : ADLWriteResPair<WriteFAddX, [ADLPort01_05], 3, [1], 1, 7>;
defm : ADLWriteResPair<WriteFAddY, [ADLPort01_05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAddZ>;
defm : ADLWriteResPair<WriteFBlend, [ADLPort00_01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteFBlendY, [ADLPort00_01_05], 1, [1], 1, 8>;
def : WriteRes<WriteFCMOV, [ADLPort01]> {
  let Latency = 3;
}
defm : ADLWriteResPair<WriteFCmp, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFCmp64, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFCmp64X, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFCmp64Y, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFCmp64Z>;
defm : ADLWriteResPair<WriteFCmpX, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFCmpY, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFCmpZ>;
def : WriteRes<WriteFCom, [ADLPort00]> {
  let Latency = 2;
}
defm : X86WriteRes<WriteFComLd, [ADLPort02_03, ADLPort05], 8, [1, 1], 2>;
defm : ADLWriteResPair<WriteFComX, [ADLPort00], 3, [1]>;
def : WriteRes<WriteFDiv, [ADLPort00]> {
  let Latency = 15;
}
defm : X86WriteRes<WriteFDivLd, [ADLPort00, ADLPort02_03], 27, [1, 1], 2>;
defm : ADLWriteResPair<WriteFDiv64, [ADLPort00], 14, [1], 1, 6>;
defm : ADLWriteResPair<WriteFDiv64X, [ADLPort00], 14, [1], 1, 6>;
defm : ADLWriteResPair<WriteFDiv64Y, [ADLPort00], 14, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFDiv64Z>;
defm : ADLWriteResPair<WriteFDivX, [ADLPort00], 11, [1], 1, 7>;
defm : ADLWriteResPair<WriteFDivY, [ADLPort00], 11, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFDivZ>;
defm : ADLWriteResPair<WriteFHAdd, [ADLPort01_05, ADLPort05], 6, [1, 2], 3, 6>;
defm : ADLWriteResPair<WriteFHAddY, [ADLPort01_05, ADLPort05], 5, [1, 2], 3, 8>;
def : WriteRes<WriteFLD0, [ADLPort00_05]>;
defm : X86WriteRes<WriteFLD1, [ADLPort00_05], 1, [2], 2>;
defm : X86WriteRes<WriteFLDC, [ADLPort00_05], 1, [2], 2>;
def : WriteRes<WriteFLoad, [ADLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadX, [ADLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadY, [ADLPort02_03_11]> {
  let Latency = 8;
}
defm : ADLWriteResPair<WriteFLogic, [ADLPort00_01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteFLogicY, [ADLPort00_01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFLogicZ>;
defm : ADLWriteResPair<WriteFMA, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFMAX, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFMAY, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMAZ>;
def : WriteRes<WriteFMOVMSK, [ADLPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFMaskedLoad, [ADLPort00_01_05, ADLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [ADLPort00_01_05, ADLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedStore32, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMoveX, [], 1, [], 0>;
defm : X86WriteRes<WriteFMoveY, [], 1, [], 0>;
defm : X86WriteResUnsupported<WriteFMoveZ>;
def : WriteRes<WriteFMul, [ADLPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteFMulLd, [ADLPort00_01, ADLPort02_03_11], 11, [1, 1], 2>;
defm : ADLWriteResPair<WriteFMul64, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFMul64X, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFMul64Y, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMul64Z>;
defm : ADLWriteResPair<WriteFMulX, [ADLPort00_01], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFMulY, [ADLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMulZ>;
defm : ADLWriteResPair<WriteFRcp, [ADLPort00], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFRcpX, [ADLPort00], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFRcpY, [ADLPort00], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFRcpZ>;
defm : ADLWriteResPair<WriteFRnd, [ADLPort00_01], 8, [2], 2, 7>;
defm : ADLWriteResPair<WriteFRndY, [ADLPort00_01], 8, [2], 2, 8>;
defm : X86WriteResPairUnsupported<WriteFRndZ>;
defm : ADLWriteResPair<WriteFRsqrt, [ADLPort00], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFRsqrtX, [ADLPort00], 4, [1], 1, 7>;
defm : ADLWriteResPair<WriteFRsqrtY, [ADLPort00], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFRsqrtZ>;
defm : ADLWriteResPair<WriteFShuffle, [ADLPort05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteFShuffle256, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WriteFShuffleY, [ADLPort05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFShuffleZ>;
def : WriteRes<WriteFSign, [ADLPort00]>;
defm : ADLWriteResPair<WriteFSqrt, [ADLPort00], 12, [1], 1, 7>;
defm : ADLWriteResPair<WriteFSqrt64, [ADLPort00], 18, [1], 1, 6>;
defm : ADLWriteResPair<WriteFSqrt64X, [ADLPort00], 18, [1], 1, 6>;
defm : ADLWriteResPair<WriteFSqrt64Y, [ADLPort00], 18, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFSqrt64Z>;
def : WriteRes<WriteFSqrt80, [ADLPortInvalid, ADLPort00]> {
  let ResourceCycles = [7, 1];
  let Latency = 21;
}
defm : ADLWriteResPair<WriteFSqrtX, [ADLPort00], 12, [1], 1, 7>;
defm : ADLWriteResPair<WriteFSqrtY, [ADLPort00], 12, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFSqrtZ>;
defm : X86WriteRes<WriteFStore, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [ADLPort04_09, ADLPort07_08], 518, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [ADLPort04_09, ADLPort07_08], 542, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreX, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : ADLWriteResPair<WriteFTest, [ADLPort00], 3, [1]>;
defm : ADLWriteResPair<WriteFTestY, [ADLPort00], 5, [1], 1, 6>;
defm : ADLWriteResPair<WriteFVarBlend, [ADLPort00_01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteFVarBlendY, [ADLPort00_01_05], 3, [3], 3, 7>;
defm : X86WriteResPairUnsupported<WriteFVarBlendZ>;
defm : ADLWriteResPair<WriteFVarShuffle, [ADLPort05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteFVarShuffle256, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WriteFVarShuffleY, [ADLPort05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFVarShuffleZ>;
def : WriteRes<WriteFence, [ADLPort00_06]> {
  let Latency = 2;
}
defm : ADLWriteResPair<WriteIDiv16, [ADLPort00_01_05_06_10, ADLPort01], 16, [1, 3], 4, 4>;
defm : ADLWriteResPair<WriteIDiv32, [ADLPort00_01_05_06_10, ADLPort01], 15, [1, 3], 4, 4>;
defm : ADLWriteResPair<WriteIDiv64, [ADLPort01], 18, [3], 3>;
defm : X86WriteRes<WriteIDiv8, [ADLPort01], 17, [3], 3>;
defm : X86WriteRes<WriteIDiv8Ld, [ADLPort01], 22, [3], 3>;
defm : ADLWriteResPair<WriteIMul16, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01], 5, [2, 1, 1], 4>;
defm : ADLWriteResPair<WriteIMul16Imm, [ADLPort00_01_05_06_10, ADLPort01], 4, [1, 1], 2>;
defm : ADLWriteResPair<WriteIMul16Reg, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteIMul32, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01], 4, [1, 1, 1], 3>;
defm : ADLWriteResPair<WriteIMul32Imm, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteIMul32Reg, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteIMul64, [ADLPort01, ADLPort05], 4, [1, 1], 2>;
defm : ADLWriteResPair<WriteIMul64Imm, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteIMul64Reg, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteIMul8, [ADLPort01], 3, [1]>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
def : WriteRes<WriteIMulHLd, []> {
  let Latency = 3;
}
def : WriteRes<WriteJump, [ADLPort00_06]>;
defm : X86WriteRes<WriteJumpLd, [ADLPort00_06, ADLPort02_03], 6, [1, 1], 2>;
def : WriteRes<WriteLAHFSAHF, [ADLPort00_06]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteLDMXCSR, [ADLPort00, ADLPort00_01_05, ADLPort00_06, ADLPort02_03_11], 7, [1, 1, 1, 1], 4>;
def : WriteRes<WriteLEA, [ADLPort01]>;
defm : ADLWriteResPair<WriteLZCNT, [ADLPort01], 3, [1]>;
def : WriteRes<WriteLoad, [ADLPort02_03_11]> {
  let Latency = 5;
}
def : WriteRes<WriteMMXMOVMSK, [ADLPort00]> {
  let Latency = 3;
}
defm : ADLWriteResPair<WriteMPSAD, [ADLPort01_05, ADLPort05], 4, [1, 1], 2, 7>;
defm : ADLWriteResPair<WriteMPSADY, [ADLPort01_05, ADLPort05], 4, [1, 1], 2, 8>;
defm : ADLWriteResPair<WriteMULX32, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01], 4, [1, 1, 1], 2>;
defm : ADLWriteResPair<WriteMULX64, [ADLPort01, ADLPort05], 4, [1, 1]>;
def : WriteRes<WriteMicrocoded, [ADLPort00_01_05_06]> {
  let Latency = ADLModel.MaxLatency;
}
def : WriteRes<WriteMove, [ADLPort00_01_05_06_10]>;
defm : X86WriteRes<WriteNop, [], 1, [], 0>;
defm : X86WriteRes<WritePCmpEStrI, [ADLPort00, ADLPort00_01_05, ADLPort00_06, ADLPort01, ADLPort05], 16, [3, 2, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrILd, [ADLPort00, ADLPort00_01_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05], 31, [3, 1, 1, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrM, [ADLPort00, ADLPort00_01_05, ADLPort00_06, ADLPort01, ADLPort05], 16, [3, 3, 1, 1, 1], 9>;
defm : X86WriteRes<WritePCmpEStrMLd, [ADLPort00, ADLPort00_01_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05], 17, [3, 2, 1, 1, 1, 1], 9>;
defm : ADLWriteResPair<WritePCmpIStrI, [ADLPort00], 11, [3], 3, 20>;
defm : ADLWriteResPair<WritePCmpIStrM, [ADLPort00], 11, [3], 3>;
defm : ADLWriteResPair<WritePHAdd, [ADLPort00_05, ADLPort05], 3, [1, 2], 3, 8>;
defm : ADLWriteResPair<WritePHAddX, [ADLPort00_01_05, ADLPort01_05], 2, [1, 2], 3, 7>;
defm : ADLWriteResPair<WritePHAddY, [ADLPort00_01_05, ADLPort01_05], 2, [1, 2], 3, 8>;
defm : ADLWriteResPair<WritePHMINPOS, [ADLPort00], 4, [1], 1, 7>;
defm : ADLWriteResPair<WritePMULLD, [ADLPort00_01], 10, [2], 2, 8>;
defm : ADLWriteResPair<WritePMULLDY, [ADLPort00_01], 10, [2], 2, 8>;
defm : X86WriteResPairUnsupported<WritePMULLDZ>;
defm : ADLWriteResPair<WritePOPCNT, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WritePSADBW, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WritePSADBWX, [ADLPort05], 3, [1], 1, 7>;
defm : ADLWriteResPair<WritePSADBWY, [ADLPort05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WritePSADBWZ>;
defm : X86WriteRes<WriteRMW, [ADLPort02_03_11, ADLPort04_09, ADLPort07_08], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteRotate, [ADLPort00_01_05_06_10, ADLPort00_06], 2, [1, 2], 3>;
defm : X86WriteRes<WriteRotateLd, [ADLPort00_01_05_06_10, ADLPort00_06], 12, [1, 2], 3>;
defm : X86WriteRes<WriteRotateCL, [ADLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteRotateCLLd, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01], 19, [2, 3, 2], 7>;
defm : X86WriteRes<WriteSETCC, [ADLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteSETCCStore, [ADLPort00_06, ADLPort04_09, ADLPort07_08], 13, [2, 1, 1], 4>;
defm : X86WriteRes<WriteSHDmrcl, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08], 12, [1, 1, 1, 1, 1, 1], 6>;
defm : X86WriteRes<WriteSHDmri, [ADLPort00_01_05_06_10, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08], 12, [1, 1, 1, 1, 1], 5>;
defm : X86WriteRes<WriteSHDrrcl, [ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01], 5, [1, 1, 1], 3>;
def : WriteRes<WriteSHDrri, [ADLPort01]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteSTMXCSR, [ADLPort00, ADLPort00_06, ADLPort04_09, ADLPort07_08], 12, [1, 1, 1, 1], 4>;
def : WriteRes<WriteShift, [ADLPort00_06]>;
def : WriteRes<WriteShiftLd, [ADLPort00_06]> {
  let Latency = 12;
}
defm : X86WriteRes<WriteShiftCL, [ADLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteShiftCLLd, [ADLPort00_06], 12, [2], 2>;
defm : ADLWriteResPair<WriteShuffle, [ADLPort05], 1, [1], 1, 8>;
defm : ADLWriteResPair<WriteShuffle256, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WriteShuffleX, [ADLPort01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteShuffleY, [ADLPort01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteShuffleZ>;
defm : X86WriteRes<WriteStore, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteStoreNT, [ADLPort04_09, ADLPort07_08], 512, [1, 1], 2>;
def : WriteRes<WriteSystem, [ADLPort00_01_05_06]> {
  let Latency = ADLModel.MaxLatency;
}
defm : ADLWriteResPair<WriteTZCNT, [ADLPort01], 3, [1]>;
defm : ADLWriteResPair<WriteVPMOV256, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WriteVarBlend, [ADLPort00_01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteVarBlendY, [ADLPort00_01_05], 3, [3], 3, 7>;
defm : X86WriteResPairUnsupported<WriteVarBlendZ>;
defm : ADLWriteResPair<WriteVarShuffle, [ADLPort00, ADLPort05], 3, [1, 1], 2, 8>;
defm : ADLWriteResPair<WriteVarShuffle256, [ADLPort05], 3, [1], 1, 8>;
defm : ADLWriteResPair<WriteVarShuffleX, [ADLPort01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteVarShuffleY, [ADLPort01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVarShuffleZ>;
defm : ADLWriteResPair<WriteVarVecShift, [ADLPort00_01], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteVarVecShiftY, [ADLPort00_01], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVarVecShiftZ>;
defm : ADLWriteResPair<WriteVecALU, [ADLPort00], 1, [1], 1, 8>;
defm : ADLWriteResPair<WriteVecALUX, [ADLPort00_01], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteVecALUY, [ADLPort00_01], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecALUZ>;
defm : X86WriteRes<WriteVecExtract, [ADLPort00, ADLPort01_05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecExtractSt, [ADLPort01_05, ADLPort04_09, ADLPort07_08], 19, [1, 1, 1], 3>;
defm : ADLWriteResPair<WriteVecIMul, [ADLPort00], 5, [1], 1, 8>;
defm : ADLWriteResPair<WriteVecIMulX, [ADLPort00_01], 5, [1], 1, 8>;
defm : ADLWriteResPair<WriteVecIMulY, [ADLPort00_01], 5, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecIMulZ>;
defm : X86WriteRes<WriteVecInsert, [ADLPort01_05, ADLPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecInsertLd, [ADLPort01_05, ADLPort02_03_11], 8, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [ADLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNT, [ADLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNTY, [ADLPort02_03_11]> {
  let Latency = 8;
}
def : WriteRes<WriteVecLoadX, [ADLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadY, [ADLPort02_03_11]> {
  let Latency = 8;
}
defm : ADLWriteResPair<WriteVecLogic, [ADLPort00_05], 1, [1], 1, 8>;
defm : ADLWriteResPair<WriteVecLogicX, [ADLPort00_01_05], 1, [1], 1, 7>;
defm : ADLWriteResPair<WriteVecLogicY, [ADLPort00_01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecLogicZ>;
def : WriteRes<WriteVecMOVMSK, [ADLPort00]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMOVMSKY, [ADLPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [ADLPort00_01_05, ADLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [ADLPort00_01_05, ADLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedStore32, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [ADLPort00, ADLPort04_09, ADLPort07_08], 14, [1, 1, 1], 3>;
def : WriteRes<WriteVecMove, [ADLPort00_05]>;
def : WriteRes<WriteVecMoveFromGpr, [ADLPort05]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMoveToGpr, [ADLPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteVecMoveX, [], 1, [], 0>;
defm : X86WriteRes<WriteVecMoveY, [], 1, [], 0>;
defm : X86WriteResUnsupported<WriteVecMoveZ>;
defm : ADLWriteResPair<WriteVecShift, [ADLPort00], 1, [1], 1, 8>;
def : WriteRes<WriteVecShiftImm, [ADLPort00]>;
def : WriteRes<WriteVecShiftImmX, [ADLPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmXLd>;
def : WriteRes<WriteVecShiftImmY, [ADLPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmYLd>;
defm : X86WriteResPairUnsupported<WriteVecShiftImmZ>;
defm : X86WriteRes<WriteVecShiftX, [ADLPort00_01, ADLPort01_05], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftXLd, [ADLPort00_01, ADLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftY, [ADLPort00_01, ADLPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftYLd, [ADLPort00_01, ADLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteVecShiftZ>;
defm : X86WriteRes<WriteVecStore, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [ADLPort04_09, ADLPort07_08], 511, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [ADLPort04_09, ADLPort07_08], 507, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreX, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [ADLPort04_09, ADLPort07_08], 12, [1, 1], 2>;
defm : ADLWriteResPair<WriteVecTest, [ADLPort00, ADLPort05], 4, [1, 1], 2>;
defm : ADLWriteResPair<WriteVecTestY, [ADLPort00, ADLPort05], 6, [1, 1], 2, 6>;
defm : X86WriteRes<WriteXCHG, [ADLPort00_01_05_06_10], 2, [3], 3>;
def : WriteRes<WriteZero, []>;

// Infered SchedWriteRes and InstRW definition.

def ADLWriteResGroup0 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 6;
}
def : InstRW<[ADLWriteResGroup0, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(16|32|64)mr$")>;

def ADLWriteResGroup1 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup1], (instregex "^JMP(16|32|64)m$",
                                             "^RET(16|32)$",
                                             "^RORX(32|64)mi$")>;
def : InstRW<[ADLWriteResGroup1, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                        "^AD(C|O)X(32|64)rm$")>;

def ADLWriteResGroup2 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup2], (instregex "^(ADC|SBB)8mi$")>;

def ADLWriteResGroup3 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[ADLWriteResGroup3, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)8mr$")>;

def ADLWriteResGroup4 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup4], (instregex "^CMP(8|16|32)mi$",
                                             "^CMP(16|32|64)mi8$",
                                             "^CMP64mi32$",
                                             "^MOV(8|16)rm$",
                                             "^MOVZX16rm8$",
                                             "^POP(16|32)r$")>;
def : InstRW<[ADLWriteResGroup4, ReadAfterLd], (instregex "^(ADD|CMP|SUB)(8|16|32|64)rm$",
                                                          "^(OR|AND|XOR)(8|16|32)rm$")>;
def : InstRW<[ADLWriteResGroup4, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^CMP(8|16|32|64)mr$")>;

def ADLWriteResGroup5 : SchedWriteRes<[]> {
  let NumMicroOps = 0;
}
def : InstRW<[ADLWriteResGroup5], (instregex "^(ADD|SUB)64ri8$",
                                             "^CLC$",
                                             "^(DE|IN)C64r$",
                                             "^JMP_1$",
                                             "^MOV64rr$",
                                             "^MOV64rr_REV$")>;

def ADLWriteResGroup6 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup6], (instregex "^(OR|ADD|SUB|XOR)8mi$",
                                             "^AND8mi$",
                                             "^(DEC|NEG|NOT)8m$",
                                             "^INC8m$")>;
def : InstRW<[ADLWriteResGroup6, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(OR|ADD|SUB|XOR)8mr$",
                                                                                                                           "^AND8mr$")>;

def ADLWriteResGroup7 : SchedWriteRes<[ADLPort01_05, ADLPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup7, ReadAfterVecLd], (instregex "^(V?)(ADD|SUB)SSrm_Int$")>;

def ADLWriteResGroup8 : SchedWriteRes<[ADLPort01_05]> {
  let Latency = 3;
}
def : InstRW<[ADLWriteResGroup8], (instregex "^(V?)(ADD|SUB)SSrr_Int$")>;

def ADLWriteResGroup9 : SchedWriteRes<[ADLPort02_03, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup9], (instregex "^(ADD|SUB)_FI(16|32)m$",
                                             "^SUBR_FI(16|32)m$")>;

def ADLWriteResGroup10 : SchedWriteRes<[ADLPort00_01_05_06_10]> {
  let Latency = 2;
}
def : InstRW<[ADLWriteResGroup10], (instregex "^(OR|AND|XOR)(16|32|64)ri8$",
                                              "^(OR|AND|XOR)(8|16|32|64)rr$",
                                              "^(OR|AND|XOR)(32|64)i32$",
                                              "^(OR|AND|XOR)(8|32)ri$",
                                              "^(OR|AND|XOR)64ri32$",
                                              "^(OR|AND|XOR)8i8$",
                                              "^TEST(8|16|32|64)rr$",
                                              "^TEST(32|64)i32$",
                                              "^TEST(8|32)ri$",
                                              "^TEST64ri32$",
                                              "^TEST8i8$")>;

def ADLWriteResGroup11 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup11], (instregex "^TEST(8|16|32)mi$",
                                              "^TEST64mi32$")>;
def : InstRW<[ADLWriteResGroup11, ReadAfterLd], (instregex "^(OR|AND|XOR)64rm$")>;
def : InstRW<[ADLWriteResGroup11, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^TEST(8|16|32|64)mr$")>;

def ADLWriteResGroup12 : SchedWriteRes<[ADLPort01_05_10, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup12, ReadAfterLd], (instregex "^ANDN(32|64)rm$")>;

def ADLWriteResGroup13 : SchedWriteRes<[ADLPort01_05_10]> {
  let Latency = 2;
}
def : InstRW<[ADLWriteResGroup13], (instregex "^ANDN(32|64)rr$")>;

def ADLWriteResGroup14 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [5, 2, 1, 1];
  let Latency = 10;
  let NumMicroOps = 9;
}
def : InstRW<[ADLWriteResGroup14], (instregex "^BT64mr$")>;

def ADLWriteResGroup15 : SchedWriteRes<[ADLPort01]> {
  let Latency = 3;
}
def : InstRW<[ADLWriteResGroup15], (instregex "^(B|PEX)T64rr$",
                                              "^BT(C|R|S)64rr$",
                                              "^PDEP(32|64)rr$",
                                              "^PEXT32rr$")>;

def ADLWriteResGroup16 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [4, 2, 1, 1, 1, 1];
  let Latency = 17;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup16], (instregex "^BT(C|R|S)64mr$")>;

def ADLWriteResGroup17 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup17], (instregex "^CALL(16|32|64)m$")>;

def ADLWriteResGroup18 : SchedWriteRes<[ADLPort00_06, ADLPort04_09, ADLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup18], (instregex "^CALL(16|32|64)r$")>;

def ADLWriteResGroup19 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup19], (instregex "^CALL64pcrel32$",
                                              "^MFENCE$")>;

def ADLWriteResGroup20 : SchedWriteRes<[ADLPort01_05]>;
def : InstRW<[ADLWriteResGroup20], (instregex "^C(BW|DQE|WDE)$",
                                              "^(V?)MOVS(H|L)DUPrr$",
                                              "^(V?)SHUFP(D|S)rri$",
                                              "^VMOVS(H|L)DUPYrr$",
                                              "^VPBLENDWYrri$",
                                              "^VSHUFP(D|S)Yrri$")>;

def ADLWriteResGroup21 : SchedWriteRes<[ADLPort00_06]>;
def : InstRW<[ADLWriteResGroup21], (instregex "^C(DQ|QO|LAC)$",
                                              "^STAC$")>;

def ADLWriteResGroup22 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup22], (instregex "^CLD$")>;

def ADLWriteResGroup23 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup23], (instregex "^CLDEMOTE$")>;

def ADLWriteResGroup24 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort04_09, ADLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup24], (instregex "^CLFLUSH$")>;

def ADLWriteResGroup25 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup25], (instregex "^CLFLUSHOPT$")>;

def ADLWriteResGroup26 : SchedWriteRes<[ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [2, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup26], (instregex "^CLI$")>;

def ADLWriteResGroup27 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort05]> {
  let ResourceCycles = [6, 1, 3];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup27], (instregex "^CLTS$")>;

def ADLWriteResGroup28 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup28], (instregex "^CLWB$",
                                              "^MOV16o(16|32|64)a$")>;

def ADLWriteResGroup29 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let ResourceCycles = [5, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup29], (instregex "^CMPS(B|L|Q|W)$")>;

def ADLWriteResGroup30 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [2, 7, 6, 2, 1, 1, 2, 1];
  let Latency = 32;
  let NumMicroOps = 22;
}
def : InstRW<[ADLWriteResGroup30], (instregex "^CMPXCHG16B$")>;

def ADLWriteResGroup31 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [4, 7, 2, 1, 1, 1];
  let Latency = 25;
  let NumMicroOps = 16;
}
def : InstRW<[ADLWriteResGroup31], (instregex "^CMPXCHG8B$")>;

def ADLWriteResGroup32 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[ADLWriteResGroup32], (instregex "^CMPXCHG8rm$")>;

def ADLWriteResGroup33 : SchedWriteRes<[ADLPort05]>;
def : InstRW<[ADLWriteResGroup33], (instregex "^COM(P?)_FST0r$",
                                              "^(V?)PALIGNRrri$",
                                              "^UCOM_F(P?)r$",
                                              "^VPALIGNRYrri$",
                                              "^VPBROADCAST(B|D|Q|W)rr$")>;

def ADLWriteResGroup34 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_06, ADLPort01, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 10, 6, 1, 5, 1];
  let Latency = 18;
  let NumMicroOps = 26;
}
def : InstRW<[ADLWriteResGroup34], (instregex "^CPUID$")>;

def ADLWriteResGroup35 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort02_03_11]> {
  let Latency = 26;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup35], (instregex "^(V?)CVT(T?)SD2SIrm_Int$")>;

def ADLWriteResGroup36 : SchedWriteRes<[ADLPort00_01, ADLPort02_03_11, ADLPort05]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup36, ReadAfterVecLd], (instregex "^(V?)CVTSI642SSrm_Int$")>;

def ADLWriteResGroup37 : SchedWriteRes<[ADLPort00_01, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup37, ReadDefault, ReadInt2Fpu], (instregex "^(V?)CVTSI642SSrr_Int$")>;

def ADLWriteResGroup38 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup38], (instregex "^(V?)CVT(T?)SS2SI64rr_Int$")>;

def ADLWriteResGroup39 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup39], (instregex "^CWD$",
                                              "^J(E|R)CXZ$")>;

def ADLWriteResGroup40 : SchedWriteRes<[ADLPort00_01_05_06]>;
def : InstRW<[ADLWriteResGroup40], (instregex "^DEC16r_alt$",
                                              "^(LD|ST)_Frr$",
                                              "^MOV16s(m|r)$",
                                              "^MOV(32|64)sr$",
                                              "^SALC$",
                                              "^ST_FPrr$",
                                              "^SYSCALL$")>;

def ADLWriteResGroup41 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]>;
def : InstRW<[ADLWriteResGroup41], (instregex "^DEC32r_alt$")>;

def ADLWriteResGroup42 : SchedWriteRes<[ADLPort00, ADLPort02_03, ADLPort05]> {
  let Latency = 30;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup42], (instregex "^DIVR_FI(16|32)m$")>;

def ADLWriteResGroup43 : SchedWriteRes<[ADLPort00, ADLPort02_03_11]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup43, ReadAfterVecLd], (instregex "^(V?)DIVSSrm_Int$")>;

def ADLWriteResGroup44 : SchedWriteRes<[ADLPort00]> {
  let Latency = 11;
}
def : InstRW<[ADLWriteResGroup44], (instregex "^(V?)DIVSSrr_Int$")>;

def ADLWriteResGroup45 : SchedWriteRes<[ADLPort00, ADLPort02_03]> {
  let Latency = 22;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup45], (instregex "^DIV_F(32|64)m$")>;

def ADLWriteResGroup46 : SchedWriteRes<[ADLPort00, ADLPort02_03, ADLPort05]> {
  let Latency = 25;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup46], (instregex "^DIV_FI(16|32)m$")>;

def ADLWriteResGroup47 : SchedWriteRes<[ADLPort00]> {
  let Latency = 20;
}
def : InstRW<[ADLWriteResGroup47], (instregex "^DIV_F(P?)rST0$",
                                              "^DIV_FST0r$")>;

def ADLWriteResGroup48 : SchedWriteRes<[ADLPort00, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [2, 21, 2, 14, 4, 9, 5];
  let Latency = 126;
  let NumMicroOps = 57;
}
def : InstRW<[ADLWriteResGroup48], (instregex "^ENTER$")>;

def ADLWriteResGroup49 : SchedWriteRes<[ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup49], (instregex "^(V?)EXTRACTPSmr$",
                                              "^SMSW16m$")>;

def ADLWriteResGroup50 : SchedWriteRes<[ADLPort00, ADLPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup50], (instregex "^(V?)EXTRACTPSrr$",
                                              "^MMX_PEXTRWrr$")>;

def ADLWriteResGroup51 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort02_03, ADLPort02_03_07, ADLPort04, ADLPort06]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup51], (instregex "^FARCALL64m$")>;

def ADLWriteResGroup52 : SchedWriteRes<[ADLPort02_03, ADLPort06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup52], (instregex "^FARJMP64m$")>;

def ADLWriteResGroup53 : SchedWriteRes<[ADLPort02_03_07, ADLPort04]> {
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup53], (instregex "^FBSTPm$",
                                              "^ST_FP(32|64|80)m$",
                                              "^VMPTRSTm$")>;

def ADLWriteResGroup54 : SchedWriteRes<[ADLPort00_05]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup54], (instregex "^FDECSTP$")>;

def ADLWriteResGroup55 : SchedWriteRes<[ADLPort02_03, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup55], (instregex "^FICOM(P?)(16|32)m$")>;

def ADLWriteResGroup56 : SchedWriteRes<[ADLPort00_05]>;
def : InstRW<[ADLWriteResGroup56], (instregex "^FINCSTP$",
                                              "^FNOP$",
                                              "^MMX_P(ADD|SUB)(B|D|Q|W)rr$")>;

def ADLWriteResGroup57 : SchedWriteRes<[ADLPort00, ADLPort00_05, ADLPort02_03]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup57], (instregex "^FLDCW16m$")>;

def ADLWriteResGroup58 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06, ADLPort00_05, ADLPort00_06, ADLPort02_03]> {
  let ResourceCycles = [2, 39, 5, 10, 8];
  let Latency = 62;
  let NumMicroOps = 64;
}
def : InstRW<[ADLWriteResGroup58], (instregex "^FLDENVm$")>;

def ADLWriteResGroup59 : SchedWriteRes<[ADLPort00_01_05_06]> {
  let ResourceCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup59], (instregex "^FNCLEX$")>;

def ADLWriteResGroup60 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort00_05, ADLPort05]> {
  let ResourceCycles = [6, 3, 6];
  let Latency = 75;
  let NumMicroOps = 15;
}
def : InstRW<[ADLWriteResGroup60], (instregex "^FNINIT$")>;

def ADLWriteResGroup61 : SchedWriteRes<[ADLPort02_03_07, ADLPort04, ADLPort06]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup61], (instregex "^FNSTCW16m$")>;

def ADLWriteResGroup62 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup62], (instregex "^FNSTSW16r$")>;

def ADLWriteResGroup63 : SchedWriteRes<[ADLPort00, ADLPort02_03_07, ADLPort04]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup63], (instregex "^FNSTSWm$")>;

def ADLWriteResGroup64 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06, ADLPort00_06, ADLPort01, ADLPort02_03_07, ADLPort04, ADLPort05, ADLPort06]> {
  let ResourceCycles = [9, 30, 21, 1, 11, 11, 16, 1];
  let Latency = 106;
  let NumMicroOps = 100;
}
def : InstRW<[ADLWriteResGroup64], (instregex "^FSTENVm$")>;

def ADLWriteResGroup65 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06, ADLPort00_05, ADLPort00_06, ADLPort01_05, ADLPort02_03, ADLPort06]> {
  let ResourceCycles = [4, 47, 1, 2, 1, 33, 2];
  let Latency = 63;
  let NumMicroOps = 90;
}
def : InstRW<[ADLWriteResGroup65], (instregex "^FXRSTOR$")>;

def ADLWriteResGroup66 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06, ADLPort00_05, ADLPort00_06, ADLPort01_05, ADLPort02_03, ADLPort06]> {
  let ResourceCycles = [4, 45, 1, 2, 1, 31, 4];
  let Latency = 63;
  let NumMicroOps = 88;
}
def : InstRW<[ADLWriteResGroup66], (instregex "^FXRSTOR64$")>;

def ADLWriteResGroup67 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [2, 5, 10, 10, 2, 38, 5, 38];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 110;
}
def : InstRW<[ADLWriteResGroup67], (instregex "^FXSAVE((64)?)$")>;

def ADLWriteResGroup68 : SchedWriteRes<[ADLPort00_01, ADLPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup68, ReadAfterVecXLd], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrmi$",
                                                               "^(V?)GF2P8MULBrm$",
                                                               "^VGF2P8AFFINE((INV)?)QBYrmi$",
                                                               "^VGF2P8MULBYrm$")>;

def ADLWriteResGroup69 : SchedWriteRes<[ADLPort00_01]> {
  let Latency = 5;
}
def : InstRW<[ADLWriteResGroup69], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrri$",
                                              "^(V?)GF2P8MULBrr$",
                                              "^VGF2P8AFFINE((INV)?)QBYrri$",
                                              "^VGF2P8MULBYrr$")>;

def ADLWriteResGroup70 : SchedWriteRes<[ADLPort02_03, ADLPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup70], (instregex "^ILD_F(16|32|64)m$")>;

def ADLWriteResGroup71 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 5, 26, 19, 2, 7, 21];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[ADLWriteResGroup71], (instregex "^IN16ri$")>;

def ADLWriteResGroup72 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 1, 4, 26, 19, 3, 7, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[ADLWriteResGroup72], (instregex "^IN16rr$")>;

def ADLWriteResGroup73 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 6, 28, 21, 2, 10, 20];
  let Latency = 35;
  let NumMicroOps = 94;
}
def : InstRW<[ADLWriteResGroup73], (instregex "^IN32ri$")>;

def ADLWriteResGroup74 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 9, 28, 21, 2, 11, 21];
  let NumMicroOps = 99;
}
def : InstRW<[ADLWriteResGroup74], (instregex "^IN32rr$")>;

def ADLWriteResGroup75 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 8, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[ADLWriteResGroup75], (instregex "^IN8ri$")>;

def ADLWriteResGroup76 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 7, 20];
  let Latency = 35;
  let NumMicroOps = 86;
}
def : InstRW<[ADLWriteResGroup76], (instregex "^IN8rr$")>;

def ADLWriteResGroup77 : SchedWriteRes<[ADLPort00_06]> {
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup77], (instregex "^INC16r_alt$")>;

def ADLWriteResGroup78 : SchedWriteRes<[ADLPort02_03_11]>;
def : InstRW<[ADLWriteResGroup78], (instregex "^INC32r_alt$",
                                              "^MOV32ao(16|32|64)$",
                                              "^MOV64ao64$")>;

def ADLWriteResGroup79 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [7, 6, 24, 17, 8, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 83;
}
def : InstRW<[ADLWriteResGroup79], (instregex "^INSB$")>;

def ADLWriteResGroup80 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [7, 1, 5, 1, 27, 17, 11, 1, 21, 1];
  let Latency = 20;
  let NumMicroOps = 92;
}
def : InstRW<[ADLWriteResGroup80], (instregex "^INSL$")>;

def ADLWriteResGroup81 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [7, 1, 4, 1, 25, 17, 1, 9, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 86;
}
def : InstRW<[ADLWriteResGroup81], (instregex "^INSW$")>;

def ADLWriteResGroup82 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [5, 4, 8, 6, 2, 5, 7, 5];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 42;
}
def : InstRW<[ADLWriteResGroup82], (instregex "^INVLPG$")>;

def ADLWriteResGroup83 : SchedWriteRes<[ADLPort02_03_07, ADLPort04, ADLPort05]> {
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup83], (instregex "^IST(T?)_FP(16|32|64)m$",
                                              "^IST_F(16|32)m$")>;

def ADLWriteResGroup84 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup84], (instregex "^JCXZ$")>;

def ADLWriteResGroup85 : SchedWriteRes<[ADLPort00, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [8, 2, 14, 3, 1];
  let Latency = 198;
  let NumMicroOps = 81;
}
def : InstRW<[ADLWriteResGroup85], (instregex "^LAR16rm$")>;

def ADLWriteResGroup86 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 66;
  let NumMicroOps = 22;
}
def : InstRW<[ADLWriteResGroup86], (instregex "^LAR16rr$")>;

def ADLWriteResGroup87 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 85;
}
def : InstRW<[ADLWriteResGroup87], (instregex "^LAR32rm$")>;

def ADLWriteResGroup88 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 65;
  let NumMicroOps = 22;
}
def : InstRW<[ADLWriteResGroup88], (instregex "^LAR(32|64)rr$")>;

def ADLWriteResGroup89 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 87;
}
def : InstRW<[ADLWriteResGroup89], (instregex "^LAR64rm$")>;

def ADLWriteResGroup90 : SchedWriteRes<[ADLPort02_03]> {
  let Latency = 7;
}
def : InstRW<[ADLWriteResGroup90], (instregex "^LD_F(32|64|80)m$")>;

def ADLWriteResGroup91 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup91], (instregex "^LEA16r$")>;

def ADLWriteResGroup92 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup92], (instregex "^LEAVE$",
                                              "^(LOD|SCA)S(B|W)$",
                                              "^SCAS(L|Q)$")>;

def ADLWriteResGroup93 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup93], (instregex "^LEAVE64$")>;

def ADLWriteResGroup94 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [1, 2, 4, 3, 2, 1, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[ADLWriteResGroup94], (instregex "^LGDT64m$")>;

def ADLWriteResGroup95 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [1, 1, 5, 3, 2, 1, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[ADLWriteResGroup95], (instregex "^LIDT64m$")>;

def ADLWriteResGroup96 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [5, 3, 2, 1, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 12;
}
def : InstRW<[ADLWriteResGroup96], (instregex "^LLDT16m$")>;

def ADLWriteResGroup97 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [1, 4, 3, 1, 1, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 11;
}
def : InstRW<[ADLWriteResGroup97], (instregex "^LLDT16r$")>;

def ADLWriteResGroup98 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [1, 1, 2, 8, 3, 1, 2, 7, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 27;
}
def : InstRW<[ADLWriteResGroup98], (instregex "^LMSW16m$")>;

def ADLWriteResGroup99 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [5, 7, 1, 2, 5, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 22;
}
def : InstRW<[ADLWriteResGroup99], (instregex "^LMSW16r$")>;

def ADLWriteResGroup100 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup100], (instregex "^LODS(L|Q)$")>;

def ADLWriteResGroup101 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [2, 4, 1];
  let Latency = 3;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup101], (instregex "^LOOP$")>;

def ADLWriteResGroup102 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 3;
  let NumMicroOps = 11;
}
def : InstRW<[ADLWriteResGroup102], (instregex "^LOOPE$")>;

def ADLWriteResGroup103 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 2;
  let NumMicroOps = 11;
}
def : InstRW<[ADLWriteResGroup103], (instregex "^LOOPNE$")>;

def ADLWriteResGroup104 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort02_03, ADLPort06]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup104], (instregex "^LRET64$")>;

def ADLWriteResGroup105 : SchedWriteRes<[ADLPort00, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 5, 3, 3, 1];
  let Latency = 70;
  let NumMicroOps = 13;
}
def : InstRW<[ADLWriteResGroup105], (instregex "^LSL(16|32|64)rm$")>;

def ADLWriteResGroup106 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 4, 4, 3, 2, 1];
  let Latency = 63;
  let NumMicroOps = 15;
}
def : InstRW<[ADLWriteResGroup106], (instregex "^LSL(16|32|64)rr$")>;

def ADLWriteResGroup107 : SchedWriteRes<[ADLPort00, ADLPort00_06, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [1, 1, 1, 2, 2, 1, 2];
  let Latency = 10;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup107], (instregex "^(V?)MASKMOVDQU((64)?)$")>;

def ADLWriteResGroup108 : SchedWriteRes<[ADLPort00_01, ADLPort02_03_11, ADLPort05]> {
  let Latency = 24;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup108], (instregex "^MMX_CVT(T?)PD2PIrm$")>;

def ADLWriteResGroup109 : SchedWriteRes<[ADLPort00_01, ADLPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup109], (instregex "^MMX_CVT(T?)PD2PIrr$")>;

def ADLWriteResGroup110 : SchedWriteRes<[ADLPort00_01, ADLPort05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup110], (instregex "^MMX_CVTPI2PDrr$")>;

def ADLWriteResGroup111 : SchedWriteRes<[ADLPort00, ADLPort00_01]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup111], (instregex "^MMX_CVTPI2PSrr$")>;

def ADLWriteResGroup112 : SchedWriteRes<[ADLPort00, ADLPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup112], (instregex "^MMX_CVT(T?)PS2PIrm$")>;

def ADLWriteResGroup113 : SchedWriteRes<[ADLPort00, ADLPort00_01_05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup113], (instregex "^MMX_CVT(T?)PS2PIrr$")>;

def ADLWriteResGroup114 : SchedWriteRes<[ADLPort00, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup114], (instregex "^MMX_MASKMOVQ((64)?)$")>;

def ADLWriteResGroup115 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup115], (instregex "^MMX_MOVD64mr$")>;

def ADLWriteResGroup116 : SchedWriteRes<[ADLPort02_03_11]> {
  let Latency = 8;
}
def : InstRW<[ADLWriteResGroup116], (instregex "^MMX_MOV(D|Q)64rm$",
                                               "^VBROADCAST(F|I)128$",
                                               "^VBROADCASTS(D|S)Yrm$",
                                               "^VMOV(D|SH|SL)DUPYrm$",
                                               "^VPBROADCAST(D|Q)Yrm$")>;

def ADLWriteResGroup117 : SchedWriteRes<[ADLPort00_01_05, ADLPort00_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup117], (instregex "^MMX_MOVDQ2Qrr$")>;

def ADLWriteResGroup118 : SchedWriteRes<[ADLPort00, ADLPort00_01_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup118], (instregex "^MMX_MOVQ2DQrr$")>;

def ADLWriteResGroup119 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup119, ReadAfterVecLd], (instregex "^MMX_PACKSS(DW|WB)rm$",
                                                               "^MMX_PACKUSWBrm$")>;

def ADLWriteResGroup120 : SchedWriteRes<[ADLPort05]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup120], (instregex "^MMX_PACKSS(DW|WB)rr$",
                                               "^MMX_PACKUSWBrr$")>;
def : InstRW<[ADLWriteResGroup120, ReadDefault, ReadInt2Fpu], (instregex "^MMX_PINSRWrr$")>;

def ADLWriteResGroup121 : SchedWriteRes<[ADLPort00_05, ADLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup121, ReadAfterVecLd], (instregex "^MMX_P(ADD|SUB)(B|D|Q|W)rm$")>;

def ADLWriteResGroup122 : SchedWriteRes<[ADLPort00, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup122, ReadAfterVecLd], (instregex "^MMX_PH(ADD|SUB)SWrm$")>;

def ADLWriteResGroup123 : SchedWriteRes<[ADLPort00, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup123], (instregex "^MMX_PH(ADD|SUB)SWrr$")>;

def ADLWriteResGroup124 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup124], (instregex "^VPBROADCAST(B|W)Yrm$")>;
def : InstRW<[ADLWriteResGroup124, ReadAfterLd], (instregex "^MMX_PINSRWrm$")>;
def : InstRW<[ADLWriteResGroup124, ReadAfterVecYLd], (instregex "^VPALIGNRYrmi$")>;

def ADLWriteResGroup125 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup125], (instregex "^MOV16ao(16|32|64)$")>;

def ADLWriteResGroup126 : SchedWriteRes<[ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup126], (instregex "^MOV16ms$",
                                               "^MOVBE32mr$",
                                               "^PUSH(F|G)S(16|32)$")>;

def ADLWriteResGroup127 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01]> {
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup127], (instregex "^MOV(16|32|64)rs$",
                                               "^S(TR|LDT)16r$")>;

def ADLWriteResGroup128 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup128], (instregex "^MOV(8|32)o(16|32)a$",
                                               "^MOV(8|32|64)o64a$")>;

def ADLWriteResGroup129 : SchedWriteRes<[ADLPort00_01_05_06_10]> {
  let Latency = 0;
}
def : InstRW<[ADLWriteResGroup129], (instregex "^MOV32rr$",
                                               "^MOV32rr_REV$",
                                               "^MOVZX(32|64)rr8$")>;

def ADLWriteResGroup130 : SchedWriteRes<[ADLPort02_03_11]> {
  let Latency = 5;
}
def : InstRW<[ADLWriteResGroup130], (instregex "^MOV64ao32$",
                                               "^MOVZX(32|64)rm(8|16)$")>;

def ADLWriteResGroup131 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [1, 2, 4, 16, 7, 2, 2, 12, 2];
  let Latency = 217;
  let NumMicroOps = 48;
}
def : InstRW<[ADLWriteResGroup131], (instregex "^MOV64dr$")>;

def ADLWriteResGroup132 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup132], (instregex "^MOV64o32a$")>;

def ADLWriteResGroup133 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort05]> {
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup133], (instregex "^MOV64rc$")>;

def ADLWriteResGroup134 : SchedWriteRes<[ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort05]> {
  let ResourceCycles = [3, 4, 8, 4, 2, 3];
  let Latency = 181;
  let NumMicroOps = 24;
}
def : InstRW<[ADLWriteResGroup134], (instregex "^MOV64rd$")>;

def ADLWriteResGroup135 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup135], (instregex "^MOV8ao(16|32|64)$")>;

def ADLWriteResGroup136 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup136], (instregex "^MOV8m(i|r)$")>;

def ADLWriteResGroup137 : SchedWriteRes<[ADLPort00_06, ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup137], (instregex "^MOVBE16mr$")>;

def ADLWriteResGroup138 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup138], (instregex "^MOVBE16rm$")>;

def ADLWriteResGroup139 : SchedWriteRes<[ADLPort01, ADLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup139], (instregex "^MOVBE32rm$")>;

def ADLWriteResGroup140 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup140], (instregex "^MOVBE64mr$",
                                               "^PUSHF16$",
                                               "^SLDT16m$",
                                               "^STRm$")>;

def ADLWriteResGroup141 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup141], (instregex "^MOVBE64rm$")>;

def ADLWriteResGroup142 : SchedWriteRes<[ADLPort02_03_11]> {
  let Latency = 7;
}
def : InstRW<[ADLWriteResGroup142], (instregex "^(V?)MOV(D|SH|SL)DUPrm$",
                                               "^VBROADCASTSSrm$",
                                               "^VPBROADCAST(D|Q)rm$")>;

def ADLWriteResGroup143 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup143], (instregex "^MOVDIR64B(16|32|64)$")>;

def ADLWriteResGroup144 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 511;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup144], (instregex "^MOVDIRI32$")>;

def ADLWriteResGroup145 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 514;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup145], (instregex "^MOVDIRI64$")>;

def ADLWriteResGroup146 : SchedWriteRes<[ADLPort01_05, ADLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup146, ReadAfterVecXLd], (instregex "^(V?)MOVLP(D|S)rm$",
                                                                "^(V?)SHUFP(D|S)rmi$")>;

def ADLWriteResGroup147 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 512;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup147], (instregex "^MOVNTDQmr$")>;

def ADLWriteResGroup148 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 518;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup148], (instregex "^MOVNTImr$")>;

def ADLWriteResGroup149 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup149], (instregex "^MOVSB$")>;

def ADLWriteResGroup150 : SchedWriteRes<[ADLPort00_01_05]>;
def : InstRW<[ADLWriteResGroup150], (instregex "^(V?)MOVS(D|S)rr$",
                                               "^(V?)MOVS(D|S)rr_REV$",
                                               "^(V?)P(ADD|SUB)(B|D|Q|W)rr$",
                                               "^VP(ADD|SUB)(B|D|Q|W)Yrr$",
                                               "^VPBLENDDrri$")>;

def ADLWriteResGroup151 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup151], (instregex "^MOVS(L|Q|W)$")>;

def ADLWriteResGroup152 : SchedWriteRes<[ADLPort01_05_10, ADLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup152], (instregex "^MOVSX16rm8$")>;

def ADLWriteResGroup153 : SchedWriteRes<[ADLPort01_05_10]>;
def : InstRW<[ADLWriteResGroup153], (instregex "^MOVSX(16|32|64)rr8$",
                                               "^MOVSX(32|64)rr16$",
                                               "^MOVSX64rr32$")>;

def ADLWriteResGroup154 : SchedWriteRes<[ADLPort02_03_11]> {
  let Latency = 6;
}
def : InstRW<[ADLWriteResGroup154], (instregex "^MOVSX(32|64)rm(8|16)$",
                                               "^MOVSX64rm32$")>;

def ADLWriteResGroup155 : SchedWriteRes<[ADLPort00_01]> {
  let Latency = 4;
}
def : InstRW<[ADLWriteResGroup155], (instregex "^(V?)MULSSrr_Int$")>;

def ADLWriteResGroup156 : SchedWriteRes<[ADLPort00, ADLPort02_03]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup156], (instregex "^MUL_F(32|64)m$")>;

def ADLWriteResGroup157 : SchedWriteRes<[ADLPort00, ADLPort02_03, ADLPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup157], (instregex "^MUL_FI(16|32)m$")>;

def ADLWriteResGroup158 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort05, ADLPort06]> {
  let ResourceCycles = [7, 1, 2];
  let Latency = 20;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup158], (instregex "^MWAITrr$")>;

def ADLWriteResGroup159 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [6, 4, 1, 28, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[ADLWriteResGroup159], (instregex "^OUT16ir$")>;

def ADLWriteResGroup160 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [6, 6, 27, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[ADLWriteResGroup160], (instregex "^OUT16rr$")>;

def ADLWriteResGroup161 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [6, 4, 1, 30, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[ADLWriteResGroup161], (instregex "^OUT32ir$")>;

def ADLWriteResGroup162 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [6, 6, 29, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[ADLWriteResGroup162], (instregex "^OUT32rr$")>;

def ADLWriteResGroup163 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [5, 5, 1, 25, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[ADLWriteResGroup163], (instregex "^OUT8ir$")>;

def ADLWriteResGroup164 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [5, 5, 26, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[ADLWriteResGroup164], (instregex "^OUT8rr$")>;

def ADLWriteResGroup165 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [7, 6, 25, 16, 7, 1, 17, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 80;
}
def : InstRW<[ADLWriteResGroup165], (instregex "^OUTSB$")>;

def ADLWriteResGroup166 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [7, 6, 28, 16, 10, 1, 20, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 89;
}
def : InstRW<[ADLWriteResGroup166], (instregex "^OUTSL$")>;

def ADLWriteResGroup167 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [6, 1, 5, 27, 16, 8, 1, 18, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 83;
}
def : InstRW<[ADLWriteResGroup167], (instregex "^OUTSW$")>;

def ADLWriteResGroup168 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup168, ReadAfterVecXLd], (instregex "^(V?)PACK(S|U)S(DW|WB)rm$",
                                                                "^(V?)PCMPGTQrm$")>;

def ADLWriteResGroup169 : SchedWriteRes<[ADLPort05]> {
  let Latency = 3;
}
def : InstRW<[ADLWriteResGroup169], (instregex "^(V?)PACK(S|U)S(DW|WB)rr$",
                                               "^(V?)PCMPGTQrr$",
                                               "^VPACK(S|U)S(DW|WB)Yrr$",
                                               "^VPCMPGTQYrr$")>;

def ADLWriteResGroup170 : SchedWriteRes<[ADLPort00_01_05, ADLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup170, ReadAfterVecXLd], (instregex "^(V?)P(ADD|SUB)(B|D|Q|W)rm$",
                                                                "^VPBLENDDrmi$")>;

def ADLWriteResGroup171 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup171], (instregex "^VPBROADCAST(B|W)rm$")>;
def : InstRW<[ADLWriteResGroup171, ReadAfterVecXLd], (instregex "^(V?)PALIGNRrmi$")>;

def ADLWriteResGroup172 : SchedWriteRes<[ADLPort00_06, ADLPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup172], (instregex "^PAUSE$")>;

def ADLWriteResGroup173 : SchedWriteRes<[ADLPort01, ADLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup173, ReadAfterLd], (instregex "^P(DEP|EXT)(32|64)rm$")>;

def ADLWriteResGroup174 : SchedWriteRes<[ADLPort01_05, ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup174], (instregex "^(V?)PEXTR(D|Q)mr$")>;

def ADLWriteResGroup175 : SchedWriteRes<[ADLPort00_01, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup175, ReadAfterVecXLd], (instregex "^(V?)PH(ADD|SUB)SWrm$")>;

def ADLWriteResGroup176 : SchedWriteRes<[ADLPort00_01, ADLPort01_05]> {
  let ResourceCycles = [1, 2];
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup176], (instregex "^(V?)PH(ADD|SUB)SWrr$",
                                               "^VPH(ADD|SUB)SWYrr$")>;

def ADLWriteResGroup177 : SchedWriteRes<[ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup177], (instregex "^POP(16|32|64)rmm$",
                                               "^PUSH(16|32)rmm$")>;

def ADLWriteResGroup178 : SchedWriteRes<[ADLPort02_03]> {
  let Latency = 5;
}
def : InstRW<[ADLWriteResGroup178], (instregex "^POPA(16|32)$",
                                               "^POPF32$")>;

def ADLWriteResGroup179 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [6, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup179], (instregex "^POPF16$")>;

def ADLWriteResGroup180 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup180], (instregex "^POPF64$")>;

def ADLWriteResGroup181 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11, ADLPort06]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup181], (instregex "^PTWRITE((64)?)m$")>;

def ADLWriteResGroup182 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort06]> {
  let ResourceCycles = [1, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup182], (instregex "^PTWRITE64r$")>;

def ADLWriteResGroup183 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort06]> {
  let ResourceCycles = [2, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup183], (instregex "^PTWRITEr$")>;

def ADLWriteResGroup184 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup184], (instregex "^PUSH64r$")>;

def ADLWriteResGroup185 : SchedWriteRes<[ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup185], (instregex "^PUSH64rmm$")>;

def ADLWriteResGroup186 : SchedWriteRes<[ADLPort02_03_07, ADLPort04]>;
def : InstRW<[ADLWriteResGroup186], (instregex "^PUSHA(16|32)$",
                                               "^PUSHF32$",
                                               "^ST_F(32|64)m$")>;

def ADLWriteResGroup187 : SchedWriteRes<[ADLPort00_06, ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup187], (instregex "^PUSHF64$")>;

def ADLWriteResGroup188 : SchedWriteRes<[ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup188], (instregex "^PUSH(F|G)S64$")>;

def ADLWriteResGroup189 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [2, 3, 2];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup189], (instregex "^RC(L|R)(16|32|64)rCL$")>;

def ADLWriteResGroup190 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup190, WriteRMW], (instregex "^RC(L|R)8m(1|i)$")>;

def ADLWriteResGroup191 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [1, 5, 2];
  let Latency = 20;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup191, WriteRMW], (instregex "^RCL8mCL$")>;

def ADLWriteResGroup192 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [2, 5, 2];
  let Latency = 7;
  let NumMicroOps = 9;
}
def : InstRW<[ADLWriteResGroup192], (instregex "^RCL8rCL$")>;

def ADLWriteResGroup193 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [2, 4, 3];
  let Latency = 20;
  let NumMicroOps = 9;
}
def : InstRW<[ADLWriteResGroup193, WriteRMW], (instregex "^RCR8mCL$")>;

def ADLWriteResGroup194 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [3, 4, 3];
  let Latency = 9;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup194], (instregex "^RCR8rCL$")>;

def ADLWriteResGroup195 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_05, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort01_05_10, ADLPort05]> {
  let ResourceCycles = [1, 6, 1, 10, 20, 8, 5, 1, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 54;
}
def : InstRW<[ADLWriteResGroup195], (instregex "^RDMSR$")>;

def ADLWriteResGroup196 : SchedWriteRes<[ADLPort00_01_05_06]> {
  let Latency = 10;
}
def : InstRW<[ADLWriteResGroup196], (instregex "^RDPID32$")>;

def ADLWriteResGroup197 : SchedWriteRes<[ADLPort01]> {
  let Latency = 10;
}
def : InstRW<[ADLWriteResGroup197], (instregex "^RDPID64$")>;

def ADLWriteResGroup198 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup198], (instregex "^RDPKRUr$")>;

def ADLWriteResGroup199 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort05]> {
  let ResourceCycles = [9, 6, 2, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 18;
}
def : InstRW<[ADLWriteResGroup199], (instregex "^RDPMC$")>;

def ADLWriteResGroup200 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = 1386;
  let NumMicroOps = 25;
}
def : InstRW<[ADLWriteResGroup200], (instregex "^RDRAND16r$")>;

def ADLWriteResGroup201 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[ADLWriteResGroup201], (instregex "^RDRAND(32|64)r$")>;

def ADLWriteResGroup202 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = 1381;
  let NumMicroOps = 25;
}
def : InstRW<[ADLWriteResGroup202], (instregex "^RDSEED16r$")>;

def ADLWriteResGroup203 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[ADLWriteResGroup203], (instregex "^RDSEED(32|64)r$")>;

def ADLWriteResGroup204 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort05]> {
  let ResourceCycles = [5, 6, 3, 1];
  let Latency = 18;
  let NumMicroOps = 15;
}
def : InstRW<[ADLWriteResGroup204], (instregex "^RDTSC$")>;

def ADLWriteResGroup205 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_01_05, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort05]> {
  let ResourceCycles = [2, 2, 1, 2, 7, 4, 3];
  let Latency = 42;
  let NumMicroOps = 21;
}
def : InstRW<[ADLWriteResGroup205], (instregex "^RDTSCP$")>;

def ADLWriteResGroup206 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup206], (instregex "^RET64$")>;

def ADLWriteResGroup207 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup207], (instregex "^RETI(16|32|64)$")>;

def ADLWriteResGroup208 : SchedWriteRes<[]>;
def : InstRW<[ADLWriteResGroup208], (instregex "^REX64_PREFIX$")>;

def ADLWriteResGroup209 : SchedWriteRes<[ADLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup209, WriteRMW], (instregex "^RO(L|R)(16|32|64)m(1|i|CL)$")>;

def ADLWriteResGroup210 : SchedWriteRes<[ADLPort00_06]> {
  let ResourceCycles = [2];
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup210], (instregex "^RO(L|R)(8|16|32|64)r(1|i)$")>;

def ADLWriteResGroup211 : SchedWriteRes<[ADLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup211, WriteRMW], (instregex "^RO(L|R)8m(1|i)$",
                                                         "^(ROL|SAR|SHR)8mCL$",
                                                         "^(ROR|SHL)8mCL$")>;

def ADLWriteResGroup212 : SchedWriteRes<[ADLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup212], (instregex "^SAHF$")>;

def ADLWriteResGroup213 : SchedWriteRes<[ADLPort00_06]> {
  let Latency = 13;
}
def : InstRW<[ADLWriteResGroup213, WriteRMW], (instregex "^S(A|H)R8m(1|i)$",
                                                         "^SHL8m(1|i)$")>;

def ADLWriteResGroup214 : SchedWriteRes<[ADLPort00_06, ADLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup214, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^S(A|H)RX(32|64)rm$",
                                                                                                                             "^SHLX(32|64)rm$")>;

def ADLWriteResGroup215 : SchedWriteRes<[ADLPort00_06]> {
  let Latency = 3;
}
def : InstRW<[ADLWriteResGroup215], (instregex "^S(A|H)RX(32|64)rr$",
                                               "^SHLX(32|64)rr$")>;

def ADLWriteResGroup216 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 2, 1, 1, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup216], (instregex "^SERIALIZE$")>;

def ADLWriteResGroup217 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup217], (instregex "^SFENCE$")>;

def ADLWriteResGroup218 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [1, 2, 2, 2];
  let Latency = 21;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup218], (instregex "^S(G|I)DT64m$")>;

def ADLWriteResGroup219 : SchedWriteRes<[ADLPort00_01_05, ADLPort02_03_11, ADLPort05]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup219, ReadAfterVecXLd], (instregex "^SHA1MSG1rm$")>;

def ADLWriteResGroup220 : SchedWriteRes<[ADLPort00_01_05, ADLPort05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup220], (instregex "^SHA1MSG1rr$")>;

def ADLWriteResGroup221 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_06, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [2, 2, 1, 2, 1];
  let Latency = 13;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup221, ReadAfterVecXLd], (instregex "^SHA1MSG2rm$")>;

def ADLWriteResGroup222 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_06, ADLPort01_05]> {
  let ResourceCycles = [2, 2, 1, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup222], (instregex "^SHA1MSG2rr$")>;

def ADLWriteResGroup223 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort01_05, ADLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup223, ReadAfterVecXLd], (instregex "^SHA1NEXTErm$")>;

def ADLWriteResGroup224 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort01_05]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup224], (instregex "^SHA1NEXTErr$")>;

def ADLWriteResGroup225 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup225, ReadAfterVecXLd], (instregex "^SHA1RNDS4rmi$",
                                                                "^SHA256RNDS2rm$")>;

def ADLWriteResGroup226 : SchedWriteRes<[ADLPort05]> {
  let Latency = 6;
}
def : InstRW<[ADLWriteResGroup226], (instregex "^SHA1RNDS4rri$",
                                               "^SHA256RNDS2rr$")>;

def ADLWriteResGroup227 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_06, ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [3, 2, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup227, ReadAfterVecXLd], (instregex "^SHA256MSG1rm$")>;

def ADLWriteResGroup228 : SchedWriteRes<[ADLPort00_01, ADLPort00_01_05, ADLPort00_06, ADLPort05]> {
  let ResourceCycles = [3, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[ADLWriteResGroup228], (instregex "^SHA256MSG1rr$")>;

def ADLWriteResGroup229 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup229, ReadAfterVecXLd], (instregex "^SHA256MSG2rm$")>;

def ADLWriteResGroup230 : SchedWriteRes<[ADLPort05]> {
  let ResourceCycles = [2];
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup230], (instregex "^SHA256MSG2rr$")>;

def ADLWriteResGroup231 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup231], (instregex "^SHRD16mri8$")>;

def ADLWriteResGroup232 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup232], (instregex "^SLDT(32|64)r$")>;

def ADLWriteResGroup233 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort05]> {
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup233], (instregex "^SMSW16r$")>;

def ADLWriteResGroup234 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort05]> {
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup234], (instregex "^SMSW(32|64)r$")>;

def ADLWriteResGroup235 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup235], (instregex "^STD$")>;

def ADLWriteResGroup236 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [1, 4, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 6;
}
def : InstRW<[ADLWriteResGroup236], (instregex "^STI$")>;

def ADLWriteResGroup237 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup237], (instregex "^STOSB$")>;

def ADLWriteResGroup238 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort04_09, ADLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup238], (instregex "^STOS(L|Q|W)$")>;

def ADLWriteResGroup239 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort01]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup239], (instregex "^STR(32|64)r$")>;

def ADLWriteResGroup240 : SchedWriteRes<[ADLPort00_01_05, ADLPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup240, ReadAfterVecXLd, ReadAfterVecXLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^VBLENDVP(D|S)rm$",
                                                                                                                                                  "^VPBLENDVBrm$")>;

def ADLWriteResGroup241 : SchedWriteRes<[ADLPort00_01_05]> {
  let ResourceCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup241], (instregex "^VBLENDVP(D|S)rr$",
                                               "^VPBLENDVBrr$")>;

def ADLWriteResGroup242 : SchedWriteRes<[ADLPort00, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [6, 7, 18];
  let Latency = 81;
  let NumMicroOps = 31;
}
def : InstRW<[ADLWriteResGroup242], (instregex "^VERRm$")>;

def ADLWriteResGroup243 : SchedWriteRes<[ADLPort00, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [6, 7, 17];
  let Latency = 74;
  let NumMicroOps = 30;
}
def : InstRW<[ADLWriteResGroup243], (instregex "^VERRr$")>;

def ADLWriteResGroup244 : SchedWriteRes<[ADLPort00, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [5, 8, 21];
  let Latency = 81;
  let NumMicroOps = 34;
}
def : InstRW<[ADLWriteResGroup244], (instregex "^VERWm$")>;

def ADLWriteResGroup245 : SchedWriteRes<[ADLPort00, ADLPort01, ADLPort02_03_11]> {
  let ResourceCycles = [5, 8, 20];
  let Latency = 74;
  let NumMicroOps = 33;
}
def : InstRW<[ADLWriteResGroup245], (instregex "^VERWr$")>;

def ADLWriteResGroup246 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 29;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup246, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Yrm$",
                                                                              "^VGATHERQPDYrm$",
                                                                              "^VPGATHER(D|Q)QYrm$",
                                                                              "^VPGATHERQDYrm$")>;

def ADLWriteResGroup247 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 20;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup247, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)rm$",
                                                                              "^VGATHERQPDrm$",
                                                                              "^VPGATHER(D|Q)Qrm$",
                                                                              "^VPGATHERQDrm$")>;

def ADLWriteResGroup248 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 8];
  let Latency = 30;
  let NumMicroOps = 12;
}
def : InstRW<[ADLWriteResGroup248, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSYrm$",
                                                                              "^VPGATHERDDYrm$")>;

def ADLWriteResGroup249 : SchedWriteRes<[ADLPort00, ADLPort00_01_05, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 28;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup249, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSrm$",
                                                                              "^VPGATHERDDrm$")>;

def ADLWriteResGroup250 : SchedWriteRes<[ADLPort01_05, ADLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup250], (instregex "^VH(ADD|SUB)P(D|S)rr$")>;

def ADLWriteResGroup251 : SchedWriteRes<[ADLPort00_01_05, ADLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup251, ReadAfterVecYLd], (instregex "^VINSERT(F|I)128rm$",
                                                                "^VP(ADD|SUB)(B|D|Q|W)Yrm$")>;

def ADLWriteResGroup252 : SchedWriteRes<[ADLPort00, ADLPort00_06, ADLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup252], (instregex "^VLDMXCSR$")>;

def ADLWriteResGroup253 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort01, ADLPort01_05, ADLPort02_03, ADLPort02_03_07, ADLPort04, ADLPort05, ADLPort06]> {
  let ResourceCycles = [8, 1, 1, 1, 1, 1, 2, 3];
  let Latency = 40;
  let NumMicroOps = 18;
}
def : InstRW<[ADLWriteResGroup253], (instregex "^VMCLEARm$")>;

def ADLWriteResGroup254 : SchedWriteRes<[ADLPort00]> {
  let Latency = 5;
}
def : InstRW<[ADLWriteResGroup254], (instregex "^VMOVMSKP(D|S)Yrr$")>;

def ADLWriteResGroup255 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 521;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup255], (instregex "^VMOVNTDQmr$")>;

def ADLWriteResGroup256 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 473;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup256], (instregex "^VMOVNTPDmr$")>;

def ADLWriteResGroup257 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 494;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup257], (instregex "^VMOVNTPSYmr$")>;

def ADLWriteResGroup258 : SchedWriteRes<[ADLPort04_09, ADLPort07_08]> {
  let Latency = 470;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup258], (instregex "^VMOVNTPSmr$")>;

def ADLWriteResGroup259 : SchedWriteRes<[ADLPort02_03_11, ADLPort05]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup259, ReadAfterVecYLd], (instregex "^VPACK(S|U)S(DW|WB)Yrm$",
                                                                "^VPCMPGTQYrm$")>;
def : InstRW<[ADLWriteResGroup259, ReadAfterVecXLd], (instregex "^VPCLMULQDQYrm$")>;

def ADLWriteResGroup260 : SchedWriteRes<[ADLPort01_05, ADLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup260, ReadAfterVecYLd], (instregex "^VPBLENDWYrmi$",
                                                                "^VSHUFP(D|S)Yrmi$")>;

def ADLWriteResGroup261 : SchedWriteRes<[ADLPort00_01, ADLPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup261], (instregex "^VPDP(BU|WS)SD((SY)?)rm$",
                                               "^VPDP(BU|WS)SD(S|Y)rm$")>;

def ADLWriteResGroup262 : SchedWriteRes<[ADLPort00_01, ADLPort01_05, ADLPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[ADLWriteResGroup262, ReadAfterVecYLd], (instregex "^VPH(ADD|SUB)SWYrm$")>;

def ADLWriteResGroup263 : SchedWriteRes<[ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort01_05_10]> {
  let ResourceCycles = [1, 2, 3, 3, 1];
  let Latency = 16;
  let NumMicroOps = 10;
}
def : InstRW<[ADLWriteResGroup263], (instregex "^VZEROALL$")>;

def ADLWriteResGroup264 : SchedWriteRes<[]> {
  let Latency = 0;
  let NumMicroOps = 0;
}
def : InstRW<[ADLWriteResGroup264], (instregex "^VZEROUPPER$")>;

def ADLWriteResGroup265 : SchedWriteRes<[ADLPort00_01_05_06]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup265], (instregex "^WAIT$")>;

def ADLWriteResGroup266 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [8, 6, 19, 63, 21, 15, 1, 10, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 144;
}
def : InstRW<[ADLWriteResGroup266], (instregex "^WRMSR$")>;

def ADLWriteResGroup267 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06, ADLPort01, ADLPort05]> {
  let ResourceCycles = [2, 1, 4, 1];
  let Latency = ADLModel.MaxLatency;
  let NumMicroOps = 8;
}
def : InstRW<[ADLWriteResGroup267], (instregex "^WRPKRUr$")>;

def ADLWriteResGroup268 : SchedWriteRes<[ADLPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup268, WriteRMW], (instregex "^XADD(16|32|64)rm$")>;

def ADLWriteResGroup269 : SchedWriteRes<[ADLPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[ADLWriteResGroup269, WriteRMW], (instregex "^XADD8rm$")>;

def ADLWriteResGroup270 : SchedWriteRes<[ADLPort00]> {
  let Latency = 2;
}
def : InstRW<[ADLWriteResGroup270], (instregex "^XAM_F$")>;

def ADLWriteResGroup271 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 39;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup271, WriteRMW], (instregex "^XCHG(16|32)rm$")>;

def ADLWriteResGroup272 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let ResourceCycles = [5, 1];
  let Latency = 39;
  let NumMicroOps = 6;
}
def : InstRW<[ADLWriteResGroup272, WriteRMW], (instregex "^XCHG64rm$")>;

def ADLWriteResGroup273 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 40;
  let NumMicroOps = 5;
}
def : InstRW<[ADLWriteResGroup273, WriteRMW], (instregex "^XCHG8rm$")>;

def ADLWriteResGroup274 : SchedWriteRes<[ADLPort00, ADLPort00_01_05_06, ADLPort00_05, ADLPort01, ADLPort05, ADLPort06]> {
  let ResourceCycles = [2, 4, 2, 1, 2, 4];
  let Latency = 17;
  let NumMicroOps = 15;
}
def : InstRW<[ADLWriteResGroup274], (instregex "^XCH_F$")>;

def ADLWriteResGroup275 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01]> {
  let ResourceCycles = [7, 3, 8, 5];
  let Latency = 4;
  let NumMicroOps = 23;
}
def : InstRW<[ADLWriteResGroup275], (instregex "^XGETBV$")>;

def ADLWriteResGroup276 : SchedWriteRes<[ADLPort00_01_05_06_10, ADLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[ADLWriteResGroup276], (instregex "^XLAT$")>;

def ADLWriteResGroup277 : SchedWriteRes<[ADLPort00_01_05_06, ADLPort01, ADLPort02_03, ADLPort06]> {
  let ResourceCycles = [21, 1, 1, 8];
  let Latency = 37;
  let NumMicroOps = 31;
}
def : InstRW<[ADLWriteResGroup277], (instregex "^XRSTOR((S|64|S64)?)$")>;

def ADLWriteResGroup278 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 42;
  let NumMicroOps = 140;
}
def : InstRW<[ADLWriteResGroup278], (instregex "^XSAVE$")>;

def ADLWriteResGroup279 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 41;
  let NumMicroOps = 140;
}
def : InstRW<[ADLWriteResGroup279], (instregex "^XSAVE64$")>;

def ADLWriteResGroup280 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [1, 19, 36, 52, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 151;
}
def : InstRW<[ADLWriteResGroup280], (instregex "^XSAVEC$")>;

def ADLWriteResGroup281 : SchedWriteRes<[ADLPort00, ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [1, 19, 36, 53, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 152;
}
def : InstRW<[ADLWriteResGroup281], (instregex "^XSAVEC64$")>;

def ADLWriteResGroup282 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [25, 35, 52, 27, 4, 1, 10, 1];
  let Latency = 46;
  let NumMicroOps = 155;
}
def : InstRW<[ADLWriteResGroup282], (instregex "^XSAVEOPT$")>;

def ADLWriteResGroup283 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [25, 35, 53, 27, 4, 1, 10, 1];
  let Latency = 46;
  let NumMicroOps = 156;
}
def : InstRW<[ADLWriteResGroup283], (instregex "^XSAVEOPT64$")>;

def ADLWriteResGroup284 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [23, 32, 53, 29, 30, 4, 2, 9, 2];
  let Latency = 42;
  let NumMicroOps = 184;
}
def : InstRW<[ADLWriteResGroup284], (instregex "^XSAVES$")>;

def ADLWriteResGroup285 : SchedWriteRes<[ADLPort00_01, ADLPort00_05, ADLPort00_06, ADLPort01, ADLPort01_05, ADLPort02_03_11, ADLPort04_09, ADLPort05, ADLPort07_08]> {
  let ResourceCycles = [23, 33, 53, 29, 32, 4, 2, 8, 2];
  let Latency = 42;
  let NumMicroOps = 186;
}
def : InstRW<[ADLWriteResGroup285], (instregex "^XSAVES64$")>;

def ADLWriteResGroup286 : SchedWriteRes<[ADLPort00_01_05, ADLPort00_01_05_06_10, ADLPort00_05_06, ADLPort00_06, ADLPort01, ADLPort01_05_10, ADLPort05]> {
  let ResourceCycles = [4, 23, 2, 14, 8, 1, 2];
  let Latency = 5;
  let NumMicroOps = 54;
}
def : InstRW<[ADLWriteResGroup286], (instregex "^XSETBV$")>;

}
