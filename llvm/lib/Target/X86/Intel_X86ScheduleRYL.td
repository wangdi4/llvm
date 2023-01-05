#if INTEL_FEATURE_CPU_RYL
//===- X86SchedRYL.td - X86 Royal Scheduling ----*- tablegen -*-=//
//
//      Copyright (c) 2022 Intel Corporation.
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
// This file defines the machine model for Intel Royal to support
// instruction scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

def RYLModel : SchedMachineModel {
  // Royal core can allocate 6 uops per cycle.
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

let SchedModel = RYLModel in {

// Royal core can issue micro-ops to 12 different ports in one cycle.
def RYLPort00 : ProcResource<1>;
def RYLPort01 : ProcResource<1>;
def RYLPort02 : ProcResource<1>;
def RYLPort03 : ProcResource<1>;
def RYLPort04 : ProcResource<1>;
def RYLPort05 : ProcResource<1>;
def RYLPort06 : ProcResource<1>;
def RYLPort07 : ProcResource<1>;
def RYLPort08 : ProcResource<1>;
def RYLPort09 : ProcResource<1>;
def RYLPort10 : ProcResource<1>;
def RYLPort11 : ProcResource<1>;

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def RYLPortInvalid : ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
def RYLPort00_01          : ProcResGroup<[RYLPort00, RYLPort01]>;
def RYLPort00_01_05       : ProcResGroup<[RYLPort00, RYLPort01, RYLPort05]>;
def RYLPort00_01_05_06    : ProcResGroup<[RYLPort00, RYLPort01, RYLPort05, RYLPort06]>;
def RYLPort00_05          : ProcResGroup<[RYLPort00, RYLPort05]>;
def RYLPort00_05_06       : ProcResGroup<[RYLPort00, RYLPort05, RYLPort06]>;
def RYLPort00_06          : ProcResGroup<[RYLPort00, RYLPort06]>;
def RYLPort01_05          : ProcResGroup<[RYLPort01, RYLPort05]>;
def RYLPort01_05_10       : ProcResGroup<[RYLPort01, RYLPort05, RYLPort10]>;
def RYLPort02_03          : ProcResGroup<[RYLPort02, RYLPort03]>;
def RYLPort02_03_07       : ProcResGroup<[RYLPort02, RYLPort03, RYLPort07]>;
def RYLPort02_03_11       : ProcResGroup<[RYLPort02, RYLPort03, RYLPort11]>;
def RYLPort07_08          : ProcResGroup<[RYLPort07, RYLPort08]>;

// EU has 112 reservation stations.
def RYLPort00_01_05_06_10 : ProcResGroup<[RYLPort00, RYLPort01, RYLPort05,
                                           RYLPort06, RYLPort10]> {
  let BufferSize = 112;
}

// STD has 48 reservation stations.
def RYLPort04_09          : ProcResGroup<[RYLPort04, RYLPort09]> {
  let BufferSize = 48;
}

// MEM has 72 reservation stations.
def RYLPort02_03_07_08_11 : ProcResGroup<[RYLPort02, RYLPort03, RYLPort07,
                                           RYLPort08, RYLPort11]> {
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
multiclass RYLWriteResPair<X86FoldableSchedWrite SchedRW,
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
  // the latency (default = 5).
  def : WriteRes<SchedRW.Folded, !listconcat([RYLPort02_03_11], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ResourceCycles = !listconcat([1], Res);
    let NumMicroOps = !add(UOps, 1);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [RYLPort00_06]>;
defm : X86WriteRes<WriteADCLd, [RYLPort00_01_05_06_10, RYLPort00_06], 11, [1, 1], 2>;
defm : RYLWriteResPair<WriteAESDecEnc, [RYLPort00_01], 5, [1], 1, 7>;
defm : RYLWriteResPair<WriteAESIMC, [RYLPort00_01], 8, [2], 2, 7>;
defm : X86WriteRes<WriteAESKeyGen, [RYLPort00, RYLPort00_01, RYLPort00_01_05, RYLPort00_06, RYLPort01_05, RYLPort05], 7, [4, 1, 1, 2, 3, 3], 14>;
defm : X86WriteRes<WriteAESKeyGenLd, [RYLPort00, RYLPort00_01, RYLPort00_06, RYLPort01_05, RYLPort02_03_11, RYLPort05], 12, [4, 1, 2, 3, 1, 3], 14>;
def : WriteRes<WriteALU, [RYLPort00_01_05_06_10]>;
def : WriteRes<WriteALULd, [RYLPort00_01_05_06_10]> {
  let Latency = 11;
}
defm : RYLWriteResPair<WriteBEXTR, [RYLPort00_06, RYLPort01], 6, [1, 1], 2>;
defm : RYLWriteResPair<WriteBLS, [RYLPort01_05_10], 2, [1]>;
defm : RYLWriteResPair<WriteBSF, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteBSR, [RYLPort01], 3, [1]>;
def : WriteRes<WriteBSWAP32, [RYLPort01]>;
defm : X86WriteRes<WriteBSWAP64, [RYLPort00_06, RYLPort01], 2, [1, 1], 2>;
defm : RYLWriteResPair<WriteBZHI, [RYLPort01], 3, [1]>;
def : WriteRes<WriteBitTest, [RYLPort01]>;
defm : X86WriteRes<WriteBitTestImmLd, [RYLPort01, RYLPort02_03_11], 6, [1, 1], 2>;
defm : X86WriteRes<WriteBitTestRegLd, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11], 11, [4, 2, 1, 2, 1], 10>;
def : WriteRes<WriteBitTestSet, [RYLPort01]>;
def : WriteRes<WriteBitTestSetImmLd, [RYLPort01]> {
  let Latency = 11;
}
defm : X86WriteRes<WriteBitTestSetRegLd, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10], 17, [3, 2, 1, 2], 8>;
defm : RYLWriteResPair<WriteBlend, [RYLPort01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteBlendY, [RYLPort00_01_05], 1, [1], 1, 8>;
defm : RYLWriteResPair<WriteCLMul, [RYLPort05], 3, [1], 1, 7>;
defm : RYLWriteResPair<WriteCMOV, [RYLPort00_06], 1, [1], 1, 6>;
defm : X86WriteRes<WriteCMPXCHG, [RYLPort00_01_05_06_10, RYLPort00_06], 3, [3, 2], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08], 12, [1, 2, 1, 1, 1], 6>;
defm : RYLWriteResPair<WriteCRC32, [RYLPort01], 3, [1]>;
defm : X86WriteRes<WriteCvtI2PD, [RYLPort00_01, RYLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDY, [RYLPort00_01, RYLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDYLd, [RYLPort00_01, RYLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtI2PDZ>;
defm : RYLWriteResPair<WriteCvtI2PS, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteCvtI2PSY, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteCvtI2PSZ>;
defm : X86WriteRes<WriteCvtI2SD, [RYLPort00_01, RYLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SDLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SS, [RYLPort00_01, RYLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SSLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : RYLWriteResPair<WriteCvtPD2I, [RYLPort00_01, RYLPort05], 5, [1, 1], 2, 7>;
defm : RYLWriteResPair<WriteCvtPD2IY, [RYLPort00_01, RYLPort05], 7, [1, 1], 2, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPD2IZ>;
defm : RYLWriteResPair<WriteCvtPD2PS, [RYLPort00_01, RYLPort05], 5, [1, 1], 2, 7>;
defm : RYLWriteResPair<WriteCvtPD2PSY, [RYLPort00_01, RYLPort05], 7, [1, 1], 2, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPD2PSZ>;
defm : X86WriteRes<WriteCvtPH2PS, [RYLPort00_01, RYLPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSLd, [RYLPort00_01, RYLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSY, [RYLPort00_01, RYLPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSYLd, [RYLPort00_01, RYLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPH2PSZ>;
defm : RYLWriteResPair<WriteCvtPS2I, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteCvtPS2IY, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteCvtPS2IZ>;
defm : X86WriteRes<WriteCvtPS2PD, [RYLPort00_01, RYLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDY, [RYLPort00_01, RYLPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDYLd, [RYLPort00_01, RYLPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteCvtPS2PDZ>;
defm : X86WriteRes<WriteCvtPS2PH, [RYLPort00_01, RYLPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHSt, [RYLPort00_01, RYLPort04_09, RYLPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHY, [RYLPort00_01, RYLPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [RYLPort00_01, RYLPort04_09, RYLPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZ>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZSt>;
defm : RYLWriteResPair<WriteCvtSD2I, [RYLPort00, RYLPort00_01], 7, [1, 1], 2>;
defm : RYLWriteResPair<WriteCvtSD2SS, [RYLPort00_01, RYLPort05], 5, [1, 1], 2, 7>;
defm : RYLWriteResPair<WriteCvtSS2I, [RYLPort00, RYLPort00_01], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SD, [RYLPort00_01, RYLPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SDLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : RYLWriteResPair<WriteDPPD, [RYLPort00_01, RYLPort01_05], 9, [2, 1], 3, 7>;
defm : RYLWriteResPair<WriteDPPS, [RYLPort00_01, RYLPort00_06, RYLPort01_05, RYLPort05], 14, [2, 1, 2, 1], 6, 7>;
defm : RYLWriteResPair<WriteDPPSY, [RYLPort00_01, RYLPort00_06, RYLPort01_05, RYLPort05], 14, [2, 1, 2, 1], 6, 8>;
defm : RYLWriteResPair<WriteDiv16, [RYLPort00_01_05_06_10, RYLPort01], 16, [1, 3], 4, 4>;
defm : RYLWriteResPair<WriteDiv32, [RYLPort00_01_05_06_10, RYLPort01], 15, [1, 3], 4, 4>;
defm : RYLWriteResPair<WriteDiv64, [RYLPort01], 18, [3], 3>;
defm : X86WriteRes<WriteDiv8, [RYLPort01], 17, [3], 3>;
defm : X86WriteRes<WriteDiv8Ld, [RYLPort01], 22, [3], 3>;
defm : X86WriteRes<WriteEMMS, [RYLPort00, RYLPort00_05, RYLPort00_06], 10, [1, 8, 1], 10>;
def : WriteRes<WriteFAdd, [RYLPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFAddLd, [RYLPort02_03, RYLPort05], 10, [1, 1], 2>;
defm : RYLWriteResPair<WriteFAdd64, [RYLPort01_05], 3, [1], 1, 7>;
defm : RYLWriteResPair<WriteFAdd64X, [RYLPort01_05], 3, [1], 1, 7>;
defm : RYLWriteResPair<WriteFAdd64Y, [RYLPort01_05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAdd64Z>;
defm : RYLWriteResPair<WriteFAddX, [RYLPort01_05], 3, [1], 1, 7>;
defm : RYLWriteResPair<WriteFAddY, [RYLPort01_05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFAddZ>;
defm : RYLWriteResPair<WriteFBlend, [RYLPort00_01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteFBlendY, [RYLPort00_01_05], 1, [1], 1, 8>;
def : WriteRes<WriteFCMOV, [RYLPort01]> {
  let Latency = 3;
}
defm : RYLWriteResPair<WriteFCmp, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFCmp64, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFCmp64X, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFCmp64Y, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFCmp64Z>;
defm : RYLWriteResPair<WriteFCmpX, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFCmpY, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFCmpZ>;
def : WriteRes<WriteFCom, [RYLPort05]>;
defm : X86WriteRes<WriteFComLd, [RYLPort02_03, RYLPort05], 8, [1, 1], 2>;
defm : RYLWriteResPair<WriteFComX, [RYLPort00], 3, [1]>;
def : WriteRes<WriteFDiv, [RYLPort00]> {
  let Latency = 15;
}
defm : X86WriteRes<WriteFDivLd, [RYLPort00, RYLPort02_03], 27, [1, 1], 2>;
defm : RYLWriteResPair<WriteFDiv64, [RYLPort00], 14, [1], 1, 6>;
defm : RYLWriteResPair<WriteFDiv64X, [RYLPort00], 14, [1], 1, 6>;
defm : RYLWriteResPair<WriteFDiv64Y, [RYLPort00], 14, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFDiv64Z>;
defm : RYLWriteResPair<WriteFDivX, [RYLPort00], 11, [1], 1, 7>;
defm : RYLWriteResPair<WriteFDivY, [RYLPort00], 11, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFDivZ>;
defm : RYLWriteResPair<WriteFHAdd, [RYLPort01_05, RYLPort05], 6, [1, 2], 3, 6>;
defm : RYLWriteResPair<WriteFHAddY, [RYLPort01_05, RYLPort05], 5, [1, 2], 3, 8>;
def : WriteRes<WriteFLD0, [RYLPort00_05]>;
defm : X86WriteRes<WriteFLD1, [RYLPort00_05], 1, [2], 2>;
defm : X86WriteRes<WriteFLDC, [RYLPort00_05], 1, [2], 2>;
def : WriteRes<WriteFLoad, [RYLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadX, [RYLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadY, [RYLPort02_03_11]> {
  let Latency = 8;
}
defm : RYLWriteResPair<WriteFLogic, [RYLPort00_01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteFLogicY, [RYLPort00_01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFLogicZ>;
defm : RYLWriteResPair<WriteFMA, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFMAX, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFMAY, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMAZ>;
def : WriteRes<WriteFMOVMSK, [RYLPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFMaskedLoad, [RYLPort00_01_05, RYLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [RYLPort00_01_05, RYLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedStore32, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMoveX, [], 1, [], 0>;
defm : X86WriteRes<WriteFMoveY, [], 1, [], 0>;
defm : X86WriteResUnsupported<WriteFMoveZ>;
def : WriteRes<WriteFMul, [RYLPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteFMulLd, [RYLPort00_01, RYLPort02_03_11], 11, [1, 1], 2>;
defm : RYLWriteResPair<WriteFMul64, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFMul64X, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFMul64Y, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMul64Z>;
defm : RYLWriteResPair<WriteFMulX, [RYLPort00_01], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFMulY, [RYLPort00_01], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFMulZ>;
defm : RYLWriteResPair<WriteFRcp, [RYLPort00], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFRcpX, [RYLPort00], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFRcpY, [RYLPort00], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFRcpZ>;
defm : RYLWriteResPair<WriteFRnd, [RYLPort00_01], 8, [2], 2, 7>;
defm : RYLWriteResPair<WriteFRndY, [RYLPort00_01], 8, [2], 2, 8>;
defm : X86WriteResPairUnsupported<WriteFRndZ>;
defm : RYLWriteResPair<WriteFRsqrt, [RYLPort00], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFRsqrtX, [RYLPort00], 4, [1], 1, 7>;
defm : RYLWriteResPair<WriteFRsqrtY, [RYLPort00], 4, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFRsqrtZ>;
defm : RYLWriteResPair<WriteFShuffle, [RYLPort05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteFShuffle256, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WriteFShuffleY, [RYLPort05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFShuffleZ>;
def : WriteRes<WriteFSign, [RYLPort00]>;
defm : RYLWriteResPair<WriteFSqrt, [RYLPort00], 12, [1], 1, 7>;
defm : RYLWriteResPair<WriteFSqrt64, [RYLPort00], 18, [1], 1, 6>;
defm : RYLWriteResPair<WriteFSqrt64X, [RYLPort00], 18, [1], 1, 6>;
defm : RYLWriteResPair<WriteFSqrt64Y, [RYLPort00], 18, [1], 1, 7>;
defm : X86WriteResPairUnsupported<WriteFSqrt64Z>;
def : WriteRes<WriteFSqrt80, [RYLPortInvalid, RYLPort00]> {
  let ResourceCycles = [7, 1];
  let Latency = 21;
}
defm : RYLWriteResPair<WriteFSqrtX, [RYLPort00], 12, [1], 1, 7>;
defm : RYLWriteResPair<WriteFSqrtY, [RYLPort00], 12, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFSqrtZ>;
defm : X86WriteRes<WriteFStore, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [RYLPort04_09, RYLPort07_08], 518, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [RYLPort04_09, RYLPort07_08], 542, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreX, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : RYLWriteResPair<WriteFTest, [RYLPort00], 3, [1]>;
defm : RYLWriteResPair<WriteFTestY, [RYLPort00], 5, [1], 1, 6>;
defm : RYLWriteResPair<WriteFVarBlend, [RYLPort00_01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteFVarBlendY, [RYLPort00_01_05], 3, [3], 3, 7>;
defm : X86WriteResPairUnsupported<WriteFVarBlendZ>;
defm : RYLWriteResPair<WriteFVarShuffle, [RYLPort05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteFVarShuffle256, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WriteFVarShuffleY, [RYLPort05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteFVarShuffleZ>;
def : WriteRes<WriteFence, [RYLPort00_06]> {
  let Latency = 2;
}
defm : RYLWriteResPair<WriteIDiv16, [RYLPort00_01_05_06_10, RYLPort01], 16, [1, 3], 4, 4>;
defm : RYLWriteResPair<WriteIDiv32, [RYLPort00_01_05_06_10, RYLPort01], 15, [1, 3], 4, 4>;
defm : RYLWriteResPair<WriteIDiv64, [RYLPort01], 18, [3], 3>;
defm : X86WriteRes<WriteIDiv8, [RYLPort01], 17, [3], 3>;
defm : X86WriteRes<WriteIDiv8Ld, [RYLPort01], 22, [3], 3>;
defm : RYLWriteResPair<WriteIMul16, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01], 5, [2, 1, 1], 4>;
defm : RYLWriteResPair<WriteIMul16Imm, [RYLPort00_01_05_06_10, RYLPort01], 4, [1, 1], 2>;
defm : RYLWriteResPair<WriteIMul16Reg, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteIMul32, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01], 4, [1, 1, 1], 3>;
defm : RYLWriteResPair<WriteIMul32Imm, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteIMul32Reg, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteIMul64, [RYLPort01, RYLPort05], 4, [1, 1], 2>;
defm : RYLWriteResPair<WriteIMul64Imm, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteIMul64Reg, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteIMul8, [RYLPort01], 3, [1]>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
def : WriteRes<WriteIMulHLd, []> {
  let Latency = 3;
}
def : WriteRes<WriteJump, [RYLPort00_06]>;
defm : X86WriteRes<WriteJumpLd, [RYLPort00_06, RYLPort02_03], 6, [1, 1], 2>;
def : WriteRes<WriteLAHFSAHF, [RYLPort00_06]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteLDMXCSR, [RYLPort00, RYLPort00_01_05, RYLPort00_06, RYLPort02_03_11], 7, [1, 1, 1, 1], 4>;
def : WriteRes<WriteLEA, [RYLPort01]>;
defm : RYLWriteResPair<WriteLZCNT, [RYLPort01], 3, [1]>;
def : WriteRes<WriteLoad, [RYLPort02_03_11]> {
  let Latency = 5;
}
def : WriteRes<WriteMMXMOVMSK, [RYLPort00]> {
  let Latency = 3;
}
defm : RYLWriteResPair<WriteMPSAD, [RYLPort01_05, RYLPort05], 4, [1, 1], 2, 7>;
defm : RYLWriteResPair<WriteMPSADY, [RYLPort01_05, RYLPort05], 4, [1, 1], 2, 8>;
defm : RYLWriteResPair<WriteMULX32, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01], 4, [1, 1, 1], 2>;
defm : RYLWriteResPair<WriteMULX64, [RYLPort01, RYLPort05], 4, [1, 1]>;
def : WriteRes<WriteMicrocoded, [RYLPort00_01_05_06]> {
  let Latency = RYLModel.MaxLatency;
}
def : WriteRes<WriteMove, [RYLPort00_01_05_06_10]>;
defm : X86WriteRes<WriteNop, [], 1, [], 0>;
defm : X86WriteRes<WritePCmpEStrI, [RYLPort00, RYLPort00_01_05, RYLPort00_06, RYLPort01, RYLPort05], 16, [3, 2, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrILd, [RYLPort00, RYLPort00_01_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05], 31, [3, 1, 1, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrM, [RYLPort00, RYLPort00_01_05, RYLPort00_06, RYLPort01, RYLPort05], 16, [3, 3, 1, 1, 1], 9>;
defm : X86WriteRes<WritePCmpEStrMLd, [RYLPort00, RYLPort00_01_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05], 17, [3, 2, 1, 1, 1, 1], 9>;
defm : RYLWriteResPair<WritePCmpIStrI, [RYLPort00], 11, [3], 3, 20>;
defm : RYLWriteResPair<WritePCmpIStrM, [RYLPort00], 11, [3], 3>;
defm : RYLWriteResPair<WritePHAdd, [RYLPort00_05, RYLPort05], 3, [1, 2], 3, 8>;
defm : RYLWriteResPair<WritePHAddX, [RYLPort00_01_05, RYLPort01_05], 2, [1, 2], 3, 7>;
defm : RYLWriteResPair<WritePHAddY, [RYLPort00_01_05, RYLPort01_05], 2, [1, 2], 3, 8>;
defm : RYLWriteResPair<WritePHMINPOS, [RYLPort00], 4, [1], 1, 7>;
defm : RYLWriteResPair<WritePMULLD, [RYLPort00_01], 10, [2], 2, 8>;
defm : RYLWriteResPair<WritePMULLDY, [RYLPort00_01], 10, [2], 2, 8>;
defm : X86WriteResPairUnsupported<WritePMULLDZ>;
defm : RYLWriteResPair<WritePOPCNT, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WritePSADBW, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WritePSADBWX, [RYLPort05], 3, [1], 1, 7>;
defm : RYLWriteResPair<WritePSADBWY, [RYLPort05], 3, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WritePSADBWZ>;
defm : X86WriteRes<WriteRMW, [RYLPort02_03_11, RYLPort04_09, RYLPort07_08], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteRotate, [RYLPort00_01_05_06_10, RYLPort00_06], 2, [1, 2], 3>;
defm : X86WriteRes<WriteRotateLd, [RYLPort00_01_05_06_10, RYLPort00_06], 12, [1, 2], 3>;
defm : X86WriteRes<WriteRotateCL, [RYLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteRotateCLLd, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01], 19, [2, 3, 2], 7>;
defm : X86WriteRes<WriteSETCC, [RYLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteSETCCStore, [RYLPort00_06, RYLPort04_09, RYLPort07_08], 13, [2, 1, 1], 4>;
defm : X86WriteRes<WriteSHDmrcl, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08], 12, [1, 1, 1, 1, 1, 1], 6>;
defm : X86WriteRes<WriteSHDmri, [RYLPort00_01_05_06_10, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08], 12, [1, 1, 1, 1, 1], 5>;
defm : X86WriteRes<WriteSHDrrcl, [RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01], 5, [1, 1, 1], 3>;
def : WriteRes<WriteSHDrri, [RYLPort01]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteSTMXCSR, [RYLPort00, RYLPort00_06, RYLPort04_09, RYLPort07_08], 12, [1, 1, 1, 1], 4>;
def : WriteRes<WriteShift, [RYLPort00_06]>;
def : WriteRes<WriteShiftLd, [RYLPort00_06]> {
  let Latency = 12;
}
defm : X86WriteRes<WriteShiftCL, [RYLPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteShiftCLLd, [RYLPort00_06], 12, [2], 2>;
defm : RYLWriteResPair<WriteShuffle, [RYLPort05], 1, [1], 1, 8>;
defm : RYLWriteResPair<WriteShuffle256, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WriteShuffleX, [RYLPort01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteShuffleY, [RYLPort01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteShuffleZ>;
defm : X86WriteRes<WriteStore, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteStoreNT, [RYLPort04_09, RYLPort07_08], 512, [1, 1], 2>;
def : WriteRes<WriteSystem, [RYLPort00_01_05_06]> {
  let Latency = RYLModel.MaxLatency;
}
defm : RYLWriteResPair<WriteTZCNT, [RYLPort01], 3, [1]>;
defm : RYLWriteResPair<WriteVPMOV256, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WriteVarBlend, [RYLPort00_01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteVarBlendY, [RYLPort00_01_05], 3, [3], 3, 7>;
defm : X86WriteResPairUnsupported<WriteVarBlendZ>;
defm : RYLWriteResPair<WriteVarShuffle, [RYLPort00, RYLPort05], 3, [1, 1], 2, 8>;
defm : RYLWriteResPair<WriteVarShuffle256, [RYLPort05], 3, [1], 1, 8>;
defm : RYLWriteResPair<WriteVarShuffleX, [RYLPort01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteVarShuffleY, [RYLPort01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVarShuffleZ>;
defm : RYLWriteResPair<WriteVarVecShift, [RYLPort00_01], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteVarVecShiftY, [RYLPort00_01], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVarVecShiftZ>;
defm : RYLWriteResPair<WriteVecALU, [RYLPort00], 1, [1], 1, 8>;
defm : RYLWriteResPair<WriteVecALUX, [RYLPort00_01], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteVecALUY, [RYLPort00_01], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecALUZ>;
defm : X86WriteRes<WriteVecExtract, [RYLPort00, RYLPort01_05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecExtractSt, [RYLPort01_05, RYLPort04_09, RYLPort07_08], 19, [1, 1, 1], 3>;
defm : RYLWriteResPair<WriteVecIMul, [RYLPort00], 5, [1], 1, 8>;
defm : RYLWriteResPair<WriteVecIMulX, [RYLPort00_01], 5, [1], 1, 8>;
defm : RYLWriteResPair<WriteVecIMulY, [RYLPort00_01], 5, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecIMulZ>;
defm : X86WriteRes<WriteVecInsert, [RYLPort01_05, RYLPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecInsertLd, [RYLPort01_05, RYLPort02_03_11], 8, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [RYLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNT, [RYLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNTY, [RYLPort02_03_11]> {
  let Latency = 8;
}
def : WriteRes<WriteVecLoadX, [RYLPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadY, [RYLPort02_03_11]> {
  let Latency = 8;
}
defm : RYLWriteResPair<WriteVecLogic, [RYLPort00_05], 1, [1], 1, 8>;
defm : RYLWriteResPair<WriteVecLogicX, [RYLPort00_01_05], 1, [1], 1, 7>;
defm : RYLWriteResPair<WriteVecLogicY, [RYLPort00_01_05], 1, [1], 1, 8>;
defm : X86WriteResPairUnsupported<WriteVecLogicZ>;
def : WriteRes<WriteVecMOVMSK, [RYLPort00]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMOVMSKY, [RYLPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [RYLPort00_01_05, RYLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [RYLPort00_01_05, RYLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedStore32, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [RYLPort00, RYLPort04_09, RYLPort07_08], 14, [1, 1, 1], 3>;
def : WriteRes<WriteVecMove, [RYLPort00_05]>;
def : WriteRes<WriteVecMoveFromGpr, [RYLPort05]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMoveToGpr, [RYLPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteVecMoveX, [], 1, [], 0>;
defm : X86WriteRes<WriteVecMoveY, [], 1, [], 0>;
defm : X86WriteResUnsupported<WriteVecMoveZ>;
defm : RYLWriteResPair<WriteVecShift, [RYLPort00], 1, [1], 1, 8>;
def : WriteRes<WriteVecShiftImm, [RYLPort00]>;
def : WriteRes<WriteVecShiftImmX, [RYLPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmXLd>;
def : WriteRes<WriteVecShiftImmY, [RYLPort00_01]>;
defm : X86WriteResUnsupported<WriteVecShiftImmYLd>;
defm : X86WriteResPairUnsupported<WriteVecShiftImmZ>;
defm : X86WriteRes<WriteVecShiftX, [RYLPort00_01, RYLPort01_05], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftXLd, [RYLPort00_01, RYLPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftY, [RYLPort00_01, RYLPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftYLd, [RYLPort00_01, RYLPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteVecShiftZ>;
defm : X86WriteRes<WriteVecStore, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [RYLPort04_09, RYLPort07_08], 511, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [RYLPort04_09, RYLPort07_08], 507, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreX, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [RYLPort04_09, RYLPort07_08], 12, [1, 1], 2>;
defm : RYLWriteResPair<WriteVecTest, [RYLPort00, RYLPort05], 4, [1, 1], 2>;
defm : RYLWriteResPair<WriteVecTestY, [RYLPort00, RYLPort05], 6, [1, 1], 2, 6>;
defm : X86WriteRes<WriteXCHG, [RYLPort00_01_05_06_10], 2, [3], 3>;
def : WriteRes<WriteZero, []>;

// Infered SchedWriteRes and InstRW definition.

def RYLWriteResGroup0 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup0, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(16|32|64)mr$")>;

def RYLWriteResGroup1 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup1], (instregex "^JMP(16|32|64)m$",
                                              "^RET(16|32)$",
                                              "^RORX(32|64)mi$")>;
def : InstRW<[RYLWriteResGroup1, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                         "^AD(C|O)X(32|64)rm$")>;

def RYLWriteResGroup2 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup2], (instregex "^(ADC|SBB)8mi$")>;

def RYLWriteResGroup3 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup3, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)8mr$")>;

def RYLWriteResGroup4 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup4], (instregex "^CMP(8|16|32)mi$",
                                              "^CMP(16|32|64)mi8$",
                                              "^CMP64mi32$",
                                              "^MOV(8|16)rm$",
                                              "^MOVZX16rm8$",
                                              "^POP(16|32)r$")>;
def : InstRW<[RYLWriteResGroup4, ReadAfterLd], (instregex "^(ADD|CMP|SUB)(8|16|32|64)rm$",
                                                           "^(OR|AND|XOR)(8|16|32)rm$")>;
def : InstRW<[RYLWriteResGroup4, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^CMP(8|16|32|64)mr$")>;

def RYLWriteResGroup5 : SchedWriteRes<[]> {
  let NumMicroOps = 0;
}
def : InstRW<[RYLWriteResGroup5], (instregex "^(ADD|SUB)64ri8$",
                                              "^CLC$",
                                              "^(DE|IN)C64r$",
                                              "^MOV64rr$",
                                              "^MOV64rr_REV$")>;

def RYLWriteResGroup6 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup6], (instregex "^(OR|ADD|SUB|XOR)8mi$",
                                              "^AND8mi$",
                                              "^(DEC|NEG|NOT)8m$",
                                              "^INC8m$")>;
def : InstRW<[RYLWriteResGroup6, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(OR|ADD|SUB|XOR)8mr$",
                                                                                                                            "^AND8mr$")>;

def RYLWriteResGroup7 : SchedWriteRes<[RYLPort01_05, RYLPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup7, ReadAfterVecLd], (instregex "^(V?)(ADD|SUB)SSrm_Int$")>;

def RYLWriteResGroup8 : SchedWriteRes<[RYLPort01_05]> {
  let Latency = 3;
}
def : InstRW<[RYLWriteResGroup8], (instregex "^(V?)(ADD|SUB)SSrr_Int$")>;

def RYLWriteResGroup9 : SchedWriteRes<[RYLPort02_03, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup9], (instregex "^(ADD|SUB)_FI(16|32)m$",
                                              "^SUBR_FI(16|32)m$")>;

def RYLWriteResGroup10 : SchedWriteRes<[RYLPort00_01_05_06_10]> {
  let Latency = 2;
}
def : InstRW<[RYLWriteResGroup10], (instregex "^(OR|AND|XOR)(16|32|64)ri8$",
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

def RYLWriteResGroup11 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup11], (instregex "^TEST(8|16|32)mi$",
                                               "^TEST64mi32$")>;
def : InstRW<[RYLWriteResGroup11, ReadAfterLd], (instregex "^(OR|AND|XOR)64rm$")>;
def : InstRW<[RYLWriteResGroup11, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^TEST(8|16|32|64)mr$")>;

def RYLWriteResGroup12 : SchedWriteRes<[RYLPort01_05_10, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup12, ReadAfterLd], (instregex "^ANDN(32|64)rm$")>;

def RYLWriteResGroup13 : SchedWriteRes<[RYLPort01_05_10]> {
  let Latency = 2;
}
def : InstRW<[RYLWriteResGroup13], (instregex "^ANDN(32|64)rr$")>;

def RYLWriteResGroup14 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [5, 2, 1, 1];
  let Latency = 10;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup14], (instregex "^BT64mr$")>;

def RYLWriteResGroup15 : SchedWriteRes<[RYLPort01]> {
  let Latency = 3;
}
def : InstRW<[RYLWriteResGroup15], (instregex "^(B|PEX)T64rr$",
                                               "^BT(C|R|S)64rr$",
                                               "^PDEP(32|64)rr$",
                                               "^PEXT32rr$")>;

def RYLWriteResGroup16 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [4, 2, 1, 1, 1, 1];
  let Latency = 17;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup16], (instregex "^BT(C|R|S)64mr$")>;

def RYLWriteResGroup17 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup17], (instregex "^CALL(16|32|64)m$")>;

def RYLWriteResGroup18 : SchedWriteRes<[RYLPort00_06, RYLPort04_09, RYLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup18], (instregex "^CALL(16|32|64)r$")>;

def RYLWriteResGroup19 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup19], (instregex "^CALL64pcrel32$",
                                               "^MFENCE$")>;

def RYLWriteResGroup20 : SchedWriteRes<[RYLPort01_05]>;
def : InstRW<[RYLWriteResGroup20], (instregex "^C(BW|DQE|WDE)$",
                                               "^(V?)MOVS(H|L)DUPrr$",
                                               "^(V?)SHUFP(D|S)rri$",
                                               "^VMOVS(H|L)DUPYrr$",
                                               "^VPBLENDWYrri$",
                                               "^VSHUFP(D|S)Yrri$")>;

def RYLWriteResGroup21 : SchedWriteRes<[RYLPort00_06]>;
def : InstRW<[RYLWriteResGroup21], (instregex "^C(DQ|QO|LAC)$",
                                               "^STAC$")>;

def RYLWriteResGroup22 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup22], (instregex "^CLD$")>;

def RYLWriteResGroup23 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup23], (instregex "^CLDEMOTE$")>;

def RYLWriteResGroup24 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort04_09, RYLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup24], (instregex "^CLFLUSH$")>;

def RYLWriteResGroup25 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup25], (instregex "^CLFLUSHOPT$")>;

def RYLWriteResGroup26 : SchedWriteRes<[RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [2, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup26], (instregex "^CLI$")>;

def RYLWriteResGroup27 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort05]> {
  let ResourceCycles = [6, 1, 3];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup27], (instregex "^CLTS$")>;

def RYLWriteResGroup28 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup28], (instregex "^CLWB$",
                                               "^MOV16o(16|32|64)a$")>;

def RYLWriteResGroup29 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let ResourceCycles = [5, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup29], (instregex "^CMPS(B|L|Q|W)$")>;

def RYLWriteResGroup30 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01_05, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [2, 7, 6, 2, 1, 1, 2, 1];
  let Latency = 32;
  let NumMicroOps = 22;
}
def : InstRW<[RYLWriteResGroup30], (instregex "^CMPXCHG16B$")>;

def RYLWriteResGroup31 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [4, 7, 2, 1, 1, 1];
  let Latency = 25;
  let NumMicroOps = 16;
}
def : InstRW<[RYLWriteResGroup31], (instregex "^CMPXCHG8B$")>;

def RYLWriteResGroup32 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup32], (instregex "^CMPXCHG8rm$")>;

def RYLWriteResGroup33 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_06, RYLPort01, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 10, 6, 1, 5, 1];
  let Latency = 18;
  let NumMicroOps = 26;
}
def : InstRW<[RYLWriteResGroup33], (instregex "^CPUID$")>;

def RYLWriteResGroup34 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort02_03_11]> {
  let Latency = 26;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup34], (instregex "^(V?)CVT(T?)SD2SIrm_Int$")>;

def RYLWriteResGroup35 : SchedWriteRes<[RYLPort00_01, RYLPort02_03_11, RYLPort05]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup35, ReadAfterVecLd], (instregex "^(V?)CVTSI642SSrm_Int$")>;

def RYLWriteResGroup36 : SchedWriteRes<[RYLPort00_01, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup36, ReadDefault, ReadInt2Fpu], (instregex "^(V?)CVTSI642SSrr_Int$")>;

def RYLWriteResGroup37 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup37], (instregex "^(V?)CVT(T?)SS2SI64rr_Int$")>;

def RYLWriteResGroup38 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup38], (instregex "^CWD$",
                                               "^J(E|R)CXZ$")>;

def RYLWriteResGroup39 : SchedWriteRes<[RYLPort00_01_05_06]>;
def : InstRW<[RYLWriteResGroup39], (instregex "^DEC16r_alt$",
                                               "^(LD|ST)_Frr$",
                                               "^MOV16s(m|r)$",
                                               "^MOV(32|64)sr$",
                                               "^SALC$",
                                               "^ST_FPrr$",
                                               "^SYSCALL$")>;

def RYLWriteResGroup40 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 7;
}
def : InstRW<[RYLWriteResGroup40], (instregex "^DEC32r_alt$")>;

def RYLWriteResGroup41 : SchedWriteRes<[RYLPort00, RYLPort02_03, RYLPort05]> {
  let Latency = 30;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup41], (instregex "^DIVR_FI(16|32)m$")>;

def RYLWriteResGroup42 : SchedWriteRes<[RYLPort00, RYLPort02_03_11]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup42, ReadAfterVecLd], (instregex "^(V?)DIVSSrm_Int$")>;

def RYLWriteResGroup43 : SchedWriteRes<[RYLPort00]> {
  let Latency = 11;
}
def : InstRW<[RYLWriteResGroup43], (instregex "^(V?)DIVSSrr_Int$")>;

def RYLWriteResGroup44 : SchedWriteRes<[RYLPort00, RYLPort02_03]> {
  let Latency = 22;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup44], (instregex "^DIV_F(32|64)m$")>;

def RYLWriteResGroup45 : SchedWriteRes<[RYLPort00, RYLPort02_03, RYLPort05]> {
  let Latency = 25;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup45], (instregex "^DIV_FI(16|32)m$")>;

def RYLWriteResGroup46 : SchedWriteRes<[RYLPort00]> {
  let Latency = 20;
}
def : InstRW<[RYLWriteResGroup46], (instregex "^DIV_F(P?)rST0$",
                                               "^DIV_FST0r$")>;

def RYLWriteResGroup47 : SchedWriteRes<[RYLPort00, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [2, 21, 2, 14, 4, 9, 5];
  let Latency = 126;
  let NumMicroOps = 57;
}
def : InstRW<[RYLWriteResGroup47], (instregex "^ENTER$")>;

def RYLWriteResGroup48 : SchedWriteRes<[RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup48], (instregex "^(V?)EXTRACTPSmr$",
                                               "^SMSW16m$")>;

def RYLWriteResGroup49 : SchedWriteRes<[RYLPort00, RYLPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup49], (instregex "^(V?)EXTRACTPSrr$",
                                               "^MMX_PEXTRWrr$")>;

def RYLWriteResGroup50 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort02_03, RYLPort02_03_07, RYLPort04, RYLPort06]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup50], (instregex "^FARCALL64m$")>;

def RYLWriteResGroup51 : SchedWriteRes<[RYLPort02_03, RYLPort06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup51], (instregex "^FARJMP64m$")>;

def RYLWriteResGroup52 : SchedWriteRes<[RYLPort02_03_07, RYLPort04]> {
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup52], (instregex "^FBSTPm$",
                                               "^(V?)MASKMOVDQU((64)?)$",
                                               "^ST_FP(32|64|80)m$",
                                               "^VMPTRSTm$")>;

def RYLWriteResGroup53 : SchedWriteRes<[RYLPort00_05]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup53], (instregex "^FDECSTP$")>;

def RYLWriteResGroup54 : SchedWriteRes<[RYLPort02_03, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup54], (instregex "^FICOM(P?)(16|32)m$")>;

def RYLWriteResGroup55 : SchedWriteRes<[RYLPort00_05]>;
def : InstRW<[RYLWriteResGroup55], (instregex "^FINCSTP$",
                                               "^FNOP$",
                                               "^MMX_P(ADD|SUB)(B|D|Q|W)rr$")>;

def RYLWriteResGroup56 : SchedWriteRes<[RYLPort00, RYLPort00_05, RYLPort02_03]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup56], (instregex "^FLDCW16m$")>;

def RYLWriteResGroup57 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06, RYLPort00_05, RYLPort00_06, RYLPort02_03]> {
  let ResourceCycles = [2, 39, 5, 10, 8];
  let Latency = 62;
  let NumMicroOps = 64;
}
def : InstRW<[RYLWriteResGroup57], (instregex "^FLDENVm$")>;

def RYLWriteResGroup58 : SchedWriteRes<[RYLPort00_01_05_06]> {
  let ResourceCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup58], (instregex "^FNCLEX$")>;

def RYLWriteResGroup59 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort00_05, RYLPort05]> {
  let ResourceCycles = [6, 3, 6];
  let Latency = 75;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup59], (instregex "^FNINIT$")>;

def RYLWriteResGroup60 : SchedWriteRes<[RYLPort02_03_07, RYLPort04, RYLPort06]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup60], (instregex "^FNSTCW16m$")>;

def RYLWriteResGroup61 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup61], (instregex "^FNSTSW16r$")>;

def RYLWriteResGroup62 : SchedWriteRes<[RYLPort00, RYLPort02_03_07, RYLPort04]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup62], (instregex "^FNSTSWm$")>;

def RYLWriteResGroup63 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06, RYLPort00_06, RYLPort01, RYLPort02_03_07, RYLPort04, RYLPort05, RYLPort06]> {
  let ResourceCycles = [9, 30, 21, 1, 11, 11, 16, 1];
  let Latency = 106;
  let NumMicroOps = 100;
}
def : InstRW<[RYLWriteResGroup63], (instregex "^FSTENVm$")>;

def RYLWriteResGroup64 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06, RYLPort00_05, RYLPort00_06, RYLPort01_05, RYLPort02_03, RYLPort06]> {
  let ResourceCycles = [4, 47, 1, 2, 1, 33, 2];
  let Latency = 63;
  let NumMicroOps = 90;
}
def : InstRW<[RYLWriteResGroup64], (instregex "^FXRSTOR$")>;

def RYLWriteResGroup65 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06, RYLPort00_05, RYLPort00_06, RYLPort01_05, RYLPort02_03, RYLPort06]> {
  let ResourceCycles = [4, 45, 1, 2, 1, 31, 4];
  let Latency = 63;
  let NumMicroOps = 88;
}
def : InstRW<[RYLWriteResGroup65], (instregex "^FXRSTOR64$")>;

def RYLWriteResGroup66 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [2, 5, 10, 10, 2, 38, 5, 38];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 110;
}
def : InstRW<[RYLWriteResGroup66], (instregex "^FXSAVE((64)?)$")>;

def RYLWriteResGroup67 : SchedWriteRes<[RYLPort00_01, RYLPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup67, ReadAfterVecXLd], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrmi$",
                                                                "^(V?)GF2P8MULBrm$",
                                                                "^VGF2P8AFFINE((INV)?)QBYrmi$",
                                                                "^VGF2P8MULBYrm$")>;

def RYLWriteResGroup68 : SchedWriteRes<[RYLPort00_01]> {
  let Latency = 5;
}
def : InstRW<[RYLWriteResGroup68], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrri$",
                                               "^(V?)GF2P8MULBrr$",
                                               "^VGF2P8AFFINE((INV)?)QBYrri$",
                                               "^VGF2P8MULBYrr$")>;

def RYLWriteResGroup69 : SchedWriteRes<[RYLPort02_03, RYLPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup69], (instregex "^ILD_F(16|32|64)m$")>;

def RYLWriteResGroup70 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 5, 26, 19, 2, 7, 21];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[RYLWriteResGroup70], (instregex "^IN16ri$")>;

def RYLWriteResGroup71 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 1, 4, 26, 19, 3, 7, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[RYLWriteResGroup71], (instregex "^IN16rr$")>;

def RYLWriteResGroup72 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 6, 28, 21, 2, 10, 20];
  let Latency = 35;
  let NumMicroOps = 94;
}
def : InstRW<[RYLWriteResGroup72], (instregex "^IN32ri$")>;

def RYLWriteResGroup73 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 9, 28, 21, 2, 11, 21];
  let NumMicroOps = 99;
}
def : InstRW<[RYLWriteResGroup73], (instregex "^IN32rr$")>;

def RYLWriteResGroup74 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 8, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[RYLWriteResGroup74], (instregex "^IN8ri$")>;

def RYLWriteResGroup75 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 7, 20];
  let Latency = 35;
  let NumMicroOps = 86;
}
def : InstRW<[RYLWriteResGroup75], (instregex "^IN8rr$")>;

def RYLWriteResGroup76 : SchedWriteRes<[RYLPort00_06]> {
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup76], (instregex "^INC16r_alt$")>;

def RYLWriteResGroup77 : SchedWriteRes<[RYLPort02_03_11]> {
  let Latency = 7;
}
def : InstRW<[RYLWriteResGroup77], (instregex "^INC32r_alt$",
                                               "^(V?)MOV(D|SH|SL)DUPrm$",
                                               "^VBROADCASTSSrm$",
                                               "^VPBROADCAST(D|Q)rm$")>;

def RYLWriteResGroup78 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [7, 6, 24, 17, 8, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 83;
}
def : InstRW<[RYLWriteResGroup78], (instregex "^INSB$")>;

def RYLWriteResGroup79 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [7, 1, 5, 1, 27, 17, 11, 1, 21, 1];
  let Latency = 20;
  let NumMicroOps = 92;
}
def : InstRW<[RYLWriteResGroup79], (instregex "^INSL$")>;

def RYLWriteResGroup80 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [7, 1, 4, 1, 25, 17, 1, 9, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 86;
}
def : InstRW<[RYLWriteResGroup80], (instregex "^INSW$")>;

def RYLWriteResGroup81 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [5, 4, 8, 6, 2, 5, 7, 5];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 42;
}
def : InstRW<[RYLWriteResGroup81], (instregex "^INVLPG$")>;

def RYLWriteResGroup82 : SchedWriteRes<[RYLPort02_03_07, RYLPort04, RYLPort05]> {
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup82], (instregex "^IST(T?)_FP(16|32|64)m$",
                                               "^IST_F(16|32)m$")>;

def RYLWriteResGroup83 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup83], (instregex "^JCXZ$")>;

def RYLWriteResGroup84 : SchedWriteRes<[]> {
  let Latency = 0;
  let NumMicroOps = 0;
}
def : InstRW<[RYLWriteResGroup84], (instregex "^JMP_1$",
                                               "^VZEROUPPER$")>;

def RYLWriteResGroup85 : SchedWriteRes<[RYLPort00, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [8, 2, 14, 3, 1];
  let Latency = 198;
  let NumMicroOps = 81;
}
def : InstRW<[RYLWriteResGroup85], (instregex "^LAR16rm$")>;

def RYLWriteResGroup86 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 66;
  let NumMicroOps = 22;
}
def : InstRW<[RYLWriteResGroup86], (instregex "^LAR16rr$")>;

def RYLWriteResGroup87 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 85;
}
def : InstRW<[RYLWriteResGroup87], (instregex "^LAR32rm$")>;

def RYLWriteResGroup88 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 65;
  let NumMicroOps = 22;
}
def : InstRW<[RYLWriteResGroup88], (instregex "^LAR(32|64)rr$")>;

def RYLWriteResGroup89 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 87;
}
def : InstRW<[RYLWriteResGroup89], (instregex "^LAR64rm$")>;

def RYLWriteResGroup90 : SchedWriteRes<[RYLPort02_03]> {
  let Latency = 7;
}
def : InstRW<[RYLWriteResGroup90], (instregex "^LD_F(32|64|80)m$")>;

def RYLWriteResGroup91 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup91], (instregex "^LEA16r$")>;

def RYLWriteResGroup92 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup92], (instregex "^LEAVE$",
                                               "^(LOD|SCA)S(B|W)$",
                                               "^SCAS(L|Q)$")>;

def RYLWriteResGroup93 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup93], (instregex "^LEAVE64$")>;

def RYLWriteResGroup94 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [1, 2, 4, 3, 2, 1, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup94], (instregex "^LGDT64m$")>;

def RYLWriteResGroup95 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [1, 1, 5, 3, 2, 1, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup95], (instregex "^LIDT64m$")>;

def RYLWriteResGroup96 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [5, 3, 2, 1, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 12;
}
def : InstRW<[RYLWriteResGroup96], (instregex "^LLDT16m$")>;

def RYLWriteResGroup97 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [1, 4, 3, 1, 1, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup97], (instregex "^LLDT16r$")>;

def RYLWriteResGroup98 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [1, 1, 2, 8, 3, 1, 2, 7, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 27;
}
def : InstRW<[RYLWriteResGroup98], (instregex "^LMSW16m$")>;

def RYLWriteResGroup99 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [5, 7, 1, 2, 5, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 22;
}
def : InstRW<[RYLWriteResGroup99], (instregex "^LMSW16r$")>;

def RYLWriteResGroup100 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup100], (instregex "^LODS(L|Q)$")>;

def RYLWriteResGroup101 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [2, 4, 1];
  let Latency = 3;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup101], (instregex "^LOOP$")>;

def RYLWriteResGroup102 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 3;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup102], (instregex "^LOOPE$")>;

def RYLWriteResGroup103 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 2;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup103], (instregex "^LOOPNE$")>;

def RYLWriteResGroup104 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort02_03, RYLPort06]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup104], (instregex "^LRET64$")>;

def RYLWriteResGroup105 : SchedWriteRes<[RYLPort00, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 5, 3, 3, 1];
  let Latency = 70;
  let NumMicroOps = 13;
}
def : InstRW<[RYLWriteResGroup105], (instregex "^LSL(16|32|64)rm$")>;

def RYLWriteResGroup106 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 4, 4, 3, 2, 1];
  let Latency = 63;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup106], (instregex "^LSL(16|32|64)rr$")>;

def RYLWriteResGroup107 : SchedWriteRes<[RYLPort00_01, RYLPort02_03_11, RYLPort05]> {
  let Latency = 24;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup107], (instregex "^MMX_CVT(T?)PD2PIrm$")>;

def RYLWriteResGroup108 : SchedWriteRes<[RYLPort00_01, RYLPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup108], (instregex "^MMX_CVT(T?)PD2PIrr$")>;

def RYLWriteResGroup109 : SchedWriteRes<[RYLPort00_01, RYLPort05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup109], (instregex "^MMX_CVTPI2PDrr$")>;

def RYLWriteResGroup110 : SchedWriteRes<[RYLPort00, RYLPort00_01]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup110], (instregex "^MMX_CVTPI2PSrr$")>;

def RYLWriteResGroup111 : SchedWriteRes<[RYLPort00, RYLPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup111], (instregex "^MMX_CVT(T?)PS2PIrm$")>;

def RYLWriteResGroup112 : SchedWriteRes<[RYLPort00, RYLPort00_01_05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup112], (instregex "^MMX_CVT(T?)PS2PIrr$")>;

def RYLWriteResGroup113 : SchedWriteRes<[RYLPort00, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup113], (instregex "^MMX_MASKMOVQ((64)?)$")>;

def RYLWriteResGroup114 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup114], (instregex "^MMX_MOVD64mr$")>;

def RYLWriteResGroup115 : SchedWriteRes<[RYLPort02_03_11]> {
  let Latency = 8;
}
def : InstRW<[RYLWriteResGroup115], (instregex "^MMX_MOV(D|Q)64rm$",
                                                "^VBROADCAST(F|I)128$",
                                                "^VBROADCASTS(D|S)Yrm$",
                                                "^VMOV(D|SH|SL)DUPYrm$",
                                                "^VPBROADCAST(D|Q)Yrm$")>;

def RYLWriteResGroup116 : SchedWriteRes<[RYLPort00_01_05, RYLPort00_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup116], (instregex "^MMX_MOVDQ2Qrr$")>;

def RYLWriteResGroup117 : SchedWriteRes<[RYLPort00, RYLPort00_01_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup117], (instregex "^MMX_MOVQ2DQrr$")>;

def RYLWriteResGroup118 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup118, ReadAfterVecLd], (instregex "^MMX_PACKSS(DW|WB)rm$",
                                                                "^MMX_PACKUSWBrm$")>;

def RYLWriteResGroup119 : SchedWriteRes<[RYLPort05]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup119], (instregex "^MMX_PACKSS(DW|WB)rr$",
                                                "^MMX_PACKUSWBrr$")>;
def : InstRW<[RYLWriteResGroup119, ReadDefault, ReadInt2Fpu], (instregex "^MMX_PINSRWrr$")>;

def RYLWriteResGroup120 : SchedWriteRes<[RYLPort00_05, RYLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup120, ReadAfterVecLd], (instregex "^MMX_P(ADD|SUB)(B|D|Q|W)rm$")>;

def RYLWriteResGroup121 : SchedWriteRes<[RYLPort00, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup121, ReadAfterVecLd], (instregex "^MMX_PH(ADD|SUB)SWrm$")>;

def RYLWriteResGroup122 : SchedWriteRes<[RYLPort00, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup122], (instregex "^MMX_PH(ADD|SUB)SWrr$")>;

def RYLWriteResGroup123 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup123], (instregex "^VPBROADCAST(B|W)Yrm$")>;
def : InstRW<[RYLWriteResGroup123, ReadAfterLd], (instregex "^MMX_PINSRWrm$")>;
def : InstRW<[RYLWriteResGroup123, ReadAfterVecYLd], (instregex "^VPALIGNRYrmi$")>;

def RYLWriteResGroup124 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup124], (instregex "^MOV16ao(16|32|64)$")>;

def RYLWriteResGroup125 : SchedWriteRes<[RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup125], (instregex "^MOV16ms$",
                                                "^MOVBE32mr$",
                                                "^PUSH(F|G)S(16|32)$")>;

def RYLWriteResGroup126 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01]> {
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup126], (instregex "^MOV(16|32|64)rs$",
                                                "^S(TR|LDT)16r$")>;

def RYLWriteResGroup127 : SchedWriteRes<[RYLPort02_03_11]>;
def : InstRW<[RYLWriteResGroup127], (instregex "^MOV32ao(16|32|64)$",
                                                "^MOV64ao64$")>;

def RYLWriteResGroup128 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup128], (instregex "^MOV(8|32)o(16|32)a$",
                                                "^MOV(8|32|64)o64a$")>;

def RYLWriteResGroup129 : SchedWriteRes<[RYLPort00_01_05_06_10]> {
  let Latency = 0;
}
def : InstRW<[RYLWriteResGroup129], (instregex "^MOV32rr$",
                                                "^MOV32rr_REV$",
                                                "^MOVZX(32|64)rr8$")>;

def RYLWriteResGroup130 : SchedWriteRes<[RYLPort02_03_11]> {
  let Latency = 5;
}
def : InstRW<[RYLWriteResGroup130], (instregex "^MOV64ao32$",
                                                "^MOVZX(32|64)rm(8|16)$")>;

def RYLWriteResGroup131 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [1, 2, 4, 16, 7, 2, 2, 12, 2];
  let Latency = 217;
  let NumMicroOps = 48;
}
def : InstRW<[RYLWriteResGroup131], (instregex "^MOV64dr$")>;

def RYLWriteResGroup132 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup132], (instregex "^MOV64o32a$")>;

def RYLWriteResGroup133 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort05]> {
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup133], (instregex "^MOV64rc$")>;

def RYLWriteResGroup134 : SchedWriteRes<[RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort05]> {
  let ResourceCycles = [3, 4, 8, 4, 2, 3];
  let Latency = 181;
  let NumMicroOps = 24;
}
def : InstRW<[RYLWriteResGroup134], (instregex "^MOV64rd$")>;

def RYLWriteResGroup135 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup135], (instregex "^MOV8ao(16|32|64)$")>;

def RYLWriteResGroup136 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup136], (instregex "^MOV8m(i|r)$")>;

def RYLWriteResGroup137 : SchedWriteRes<[RYLPort00_06, RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup137], (instregex "^MOVBE16mr$")>;

def RYLWriteResGroup138 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup138], (instregex "^MOVBE16rm$")>;

def RYLWriteResGroup139 : SchedWriteRes<[RYLPort01, RYLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup139], (instregex "^MOVBE32rm$")>;

def RYLWriteResGroup140 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup140], (instregex "^MOVBE64mr$",
                                                "^PUSHF16$",
                                                "^SLDT16m$",
                                                "^STRm$")>;

def RYLWriteResGroup141 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup141], (instregex "^MOVBE64rm$")>;

def RYLWriteResGroup142 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup142], (instregex "^MOVDIR64B(16|32|64)$")>;

def RYLWriteResGroup143 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 511;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup143], (instregex "^MOVDIRI32$")>;

def RYLWriteResGroup144 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 514;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup144], (instregex "^MOVDIRI64$")>;

def RYLWriteResGroup145 : SchedWriteRes<[RYLPort01_05, RYLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup145, ReadAfterVecXLd], (instregex "^(V?)MOVLP(D|S)rm$",
                                                                 "^(V?)SHUFP(D|S)rmi$")>;

def RYLWriteResGroup146 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 512;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup146], (instregex "^MOVNTDQmr$")>;

def RYLWriteResGroup147 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 518;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup147], (instregex "^MOVNTImr$")>;

def RYLWriteResGroup148 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup148], (instregex "^MOVSB$")>;

def RYLWriteResGroup149 : SchedWriteRes<[RYLPort00_01_05]>;
def : InstRW<[RYLWriteResGroup149], (instregex "^(V?)MOVS(D|S)rr$",
                                                "^(V?)MOVS(D|S)rr_REV$",
                                                "^(V?)P(ADD|SUB)(B|D|Q|W)rr$",
                                                "^VP(ADD|SUB)(B|D|Q|W)Yrr$",
                                                "^VPBLENDDrri$")>;

def RYLWriteResGroup150 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup150], (instregex "^MOVS(L|Q|W)$")>;

def RYLWriteResGroup151 : SchedWriteRes<[RYLPort01_05_10, RYLPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup151], (instregex "^MOVSX16rm8$")>;

def RYLWriteResGroup152 : SchedWriteRes<[RYLPort01_05_10]>;
def : InstRW<[RYLWriteResGroup152], (instregex "^MOVSX(16|32|64)rr8$",
                                                "^MOVSX(32|64)rr16$",
                                                "^MOVSX64rr32$")>;

def RYLWriteResGroup153 : SchedWriteRes<[RYLPort02_03_11]> {
  let Latency = 6;
}
def : InstRW<[RYLWriteResGroup153], (instregex "^MOVSX(32|64)rm(8|16)$",
                                                "^MOVSX64rm32$")>;

def RYLWriteResGroup154 : SchedWriteRes<[RYLPort00_01]> {
  let Latency = 4;
}
def : InstRW<[RYLWriteResGroup154], (instregex "^(V?)MULSSrr_Int$")>;

def RYLWriteResGroup155 : SchedWriteRes<[RYLPort00, RYLPort02_03]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup155], (instregex "^MUL_F(32|64)m$")>;

def RYLWriteResGroup156 : SchedWriteRes<[RYLPort00, RYLPort02_03, RYLPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup156], (instregex "^MUL_FI(16|32)m$")>;

def RYLWriteResGroup157 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort05, RYLPort06]> {
  let ResourceCycles = [7, 1, 2];
  let Latency = 20;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup157], (instregex "^MWAITrr$")>;

def RYLWriteResGroup158 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [6, 4, 1, 28, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[RYLWriteResGroup158], (instregex "^OUT16ir$")>;

def RYLWriteResGroup159 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [6, 6, 27, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[RYLWriteResGroup159], (instregex "^OUT16rr$")>;

def RYLWriteResGroup160 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [6, 4, 1, 30, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[RYLWriteResGroup160], (instregex "^OUT32ir$")>;

def RYLWriteResGroup161 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [6, 6, 29, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[RYLWriteResGroup161], (instregex "^OUT32rr$")>;

def RYLWriteResGroup162 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [5, 5, 1, 25, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[RYLWriteResGroup162], (instregex "^OUT8ir$")>;

def RYLWriteResGroup163 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [5, 5, 26, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[RYLWriteResGroup163], (instregex "^OUT8rr$")>;

def RYLWriteResGroup164 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [7, 6, 25, 16, 7, 1, 17, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 80;
}
def : InstRW<[RYLWriteResGroup164], (instregex "^OUTSB$")>;

def RYLWriteResGroup165 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [7, 6, 28, 16, 10, 1, 20, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 89;
}
def : InstRW<[RYLWriteResGroup165], (instregex "^OUTSL$")>;

def RYLWriteResGroup166 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [6, 1, 5, 27, 16, 8, 1, 18, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 83;
}
def : InstRW<[RYLWriteResGroup166], (instregex "^OUTSW$")>;

def RYLWriteResGroup167 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup167, ReadAfterVecXLd], (instregex "^(V?)PACK(S|U)S(DW|WB)rm$",
                                                                 "^(V?)PCMPGTQrm$")>;

def RYLWriteResGroup168 : SchedWriteRes<[RYLPort05]> {
  let Latency = 3;
}
def : InstRW<[RYLWriteResGroup168], (instregex "^(V?)PACK(S|U)S(DW|WB)rr$",
                                                "^(V?)PCMPGTQrr$",
                                                "^VPACK(S|U)S(DW|WB)Yrr$",
                                                "^VPCMPGTQYrr$")>;

def RYLWriteResGroup169 : SchedWriteRes<[RYLPort00_01_05, RYLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup169, ReadAfterVecXLd], (instregex "^(V?)P(ADD|SUB)(B|D|Q|W)rm$",
                                                                 "^VPBLENDDrmi$")>;

def RYLWriteResGroup170 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup170], (instregex "^VPBROADCAST(B|W)rm$")>;
def : InstRW<[RYLWriteResGroup170, ReadAfterVecXLd], (instregex "^(V?)PALIGNRrmi$")>;

def RYLWriteResGroup171 : SchedWriteRes<[RYLPort05]>;
def : InstRW<[RYLWriteResGroup171], (instregex "^(V?)PALIGNRrri$",
                                                "^VPALIGNRYrri$",
                                                "^VPBROADCAST(B|D|Q|W)rr$")>;

def RYLWriteResGroup172 : SchedWriteRes<[RYLPort00_06, RYLPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup172], (instregex "^PAUSE$")>;

def RYLWriteResGroup173 : SchedWriteRes<[RYLPort01, RYLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup173, ReadAfterLd], (instregex "^P(DEP|EXT)(32|64)rm$")>;

def RYLWriteResGroup174 : SchedWriteRes<[RYLPort01_05, RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup174], (instregex "^(V?)PEXTR(D|Q)mr$")>;

def RYLWriteResGroup175 : SchedWriteRes<[RYLPort00_01, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup175, ReadAfterVecXLd], (instregex "^(V?)PH(ADD|SUB)SWrm$")>;

def RYLWriteResGroup176 : SchedWriteRes<[RYLPort00_01, RYLPort01_05]> {
  let ResourceCycles = [1, 2];
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup176], (instregex "^(V?)PH(ADD|SUB)SWrr$",
                                                "^VPH(ADD|SUB)SWYrr$")>;

def RYLWriteResGroup177 : SchedWriteRes<[RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup177], (instregex "^POP(16|32|64)rmm$",
                                                "^PUSH(16|32)rmm$")>;

def RYLWriteResGroup178 : SchedWriteRes<[RYLPort02_03]> {
  let Latency = 5;
}
def : InstRW<[RYLWriteResGroup178], (instregex "^POPA(16|32)$",
                                                "^POPF32$")>;

def RYLWriteResGroup179 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [6, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup179], (instregex "^POPF16$")>;

def RYLWriteResGroup180 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup180], (instregex "^POPF64$")>;

def RYLWriteResGroup181 : SchedWriteRes<[RYLPort02_03_11]> {
  let Latency = 0;
}
def : InstRW<[RYLWriteResGroup181], (instregex "^PREFETCH(T0|T1|T2|NTA)$")>;

def RYLWriteResGroup182 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11, RYLPort06]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup182], (instregex "^PTWRITE((64)?)m$")>;

def RYLWriteResGroup183 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort06]> {
  let ResourceCycles = [1, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup183], (instregex "^PTWRITE64r$")>;

def RYLWriteResGroup184 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort06]> {
  let ResourceCycles = [2, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup184], (instregex "^PTWRITEr$")>;

def RYLWriteResGroup185 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup185], (instregex "^PUSH64r$")>;

def RYLWriteResGroup186 : SchedWriteRes<[RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup186], (instregex "^PUSH64rmm$")>;

def RYLWriteResGroup187 : SchedWriteRes<[RYLPort02_03_07, RYLPort04]>;
def : InstRW<[RYLWriteResGroup187], (instregex "^PUSHA(16|32)$",
                                                "^PUSHF32$",
                                                "^ST_F(32|64)m$")>;

def RYLWriteResGroup188 : SchedWriteRes<[RYLPort00_06, RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup188], (instregex "^PUSHF64$")>;

def RYLWriteResGroup189 : SchedWriteRes<[RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup189], (instregex "^PUSH(F|G)S64$")>;

def RYLWriteResGroup190 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [2, 3, 2];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup190], (instregex "^RC(L|R)(16|32|64)rCL$")>;

def RYLWriteResGroup191 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup191, WriteRMW], (instregex "^RC(L|R)8m(1|i)$")>;

def RYLWriteResGroup192 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [1, 5, 2];
  let Latency = 20;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup192, WriteRMW], (instregex "^RCL8mCL$")>;

def RYLWriteResGroup193 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [2, 5, 2];
  let Latency = 7;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup193], (instregex "^RCL8rCL$")>;

def RYLWriteResGroup194 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [2, 4, 3];
  let Latency = 20;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup194, WriteRMW], (instregex "^RCR8mCL$")>;

def RYLWriteResGroup195 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [3, 4, 3];
  let Latency = 9;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup195], (instregex "^RCR8rCL$")>;

def RYLWriteResGroup196 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_05, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort01_05_10, RYLPort05]> {
  let ResourceCycles = [1, 6, 1, 10, 20, 8, 5, 1, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 54;
}
def : InstRW<[RYLWriteResGroup196], (instregex "^RDMSR$")>;

def RYLWriteResGroup197 : SchedWriteRes<[RYLPort01]> {
  let Latency = RYLModel.MaxLatency;
}
def : InstRW<[RYLWriteResGroup197], (instregex "^RDPID64$")>;

def RYLWriteResGroup198 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup198], (instregex "^RDPKRUr$")>;

def RYLWriteResGroup199 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort05]> {
  let ResourceCycles = [9, 6, 2, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 18;
}
def : InstRW<[RYLWriteResGroup199], (instregex "^RDPMC$")>;

def RYLWriteResGroup200 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = 1386;
  let NumMicroOps = 25;
}
def : InstRW<[RYLWriteResGroup200], (instregex "^RDRAND16r$")>;

def RYLWriteResGroup201 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[RYLWriteResGroup201], (instregex "^RDRAND(32|64)r$")>;

def RYLWriteResGroup202 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = 1381;
  let NumMicroOps = 25;
}
def : InstRW<[RYLWriteResGroup202], (instregex "^RDSEED16r$")>;

def RYLWriteResGroup203 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[RYLWriteResGroup203], (instregex "^RDSEED(32|64)r$")>;

def RYLWriteResGroup204 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort05]> {
  let ResourceCycles = [5, 6, 3, 1];
  let Latency = 18;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup204], (instregex "^RDTSC$")>;

def RYLWriteResGroup205 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_01_05, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort05]> {
  let ResourceCycles = [2, 2, 1, 2, 7, 4, 3];
  let Latency = 42;
  let NumMicroOps = 21;
}
def : InstRW<[RYLWriteResGroup205], (instregex "^RDTSCP$")>;

def RYLWriteResGroup206 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup206], (instregex "^RET64$")>;

def RYLWriteResGroup207 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup207], (instregex "^RETI(16|32|64)$")>;

def RYLWriteResGroup208 : SchedWriteRes<[]>;
def : InstRW<[RYLWriteResGroup208], (instregex "^REX64_PREFIX$")>;

def RYLWriteResGroup209 : SchedWriteRes<[RYLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup209, WriteRMW], (instregex "^RO(L|R)(16|32|64)m(1|i|CL)$")>;

def RYLWriteResGroup210 : SchedWriteRes<[RYLPort00_06]> {
  let ResourceCycles = [2];
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup210], (instregex "^RO(L|R)(8|16|32|64)r(1|i)$")>;

def RYLWriteResGroup211 : SchedWriteRes<[RYLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup211, WriteRMW], (instregex "^RO(L|R)8m(1|i)$",
                                                          "^(ROL|SAR|SHR)8mCL$",
                                                          "^(ROR|SHL)8mCL$")>;

def RYLWriteResGroup212 : SchedWriteRes<[RYLPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup212], (instregex "^SAHF$")>;

def RYLWriteResGroup213 : SchedWriteRes<[RYLPort00_06]> {
  let Latency = 13;
}
def : InstRW<[RYLWriteResGroup213, WriteRMW], (instregex "^S(A|H)R8m(1|i)$",
                                                          "^SHL8m(1|i)$")>;

def RYLWriteResGroup214 : SchedWriteRes<[RYLPort00_06, RYLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup214, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^S(A|H)RX(32|64)rm$",
                                                                                                                              "^SHLX(32|64)rm$")>;

def RYLWriteResGroup215 : SchedWriteRes<[RYLPort00_06]> {
  let Latency = 3;
}
def : InstRW<[RYLWriteResGroup215], (instregex "^S(A|H)RX(32|64)rr$",
                                                "^SHLX(32|64)rr$")>;

def RYLWriteResGroup216 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 2, 1, 1, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup216], (instregex "^SERIALIZE$")>;

def RYLWriteResGroup217 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup217], (instregex "^SFENCE$")>;

def RYLWriteResGroup218 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [1, 2, 2, 2];
  let Latency = 21;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup218], (instregex "^S(G|I)DT64m$")>;

def RYLWriteResGroup219 : SchedWriteRes<[RYLPort00_01_05, RYLPort02_03_11, RYLPort05]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup219, ReadAfterVecXLd], (instregex "^SHA1MSG1rm$")>;

def RYLWriteResGroup220 : SchedWriteRes<[RYLPort00_01_05, RYLPort05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup220], (instregex "^SHA1MSG1rr$")>;

def RYLWriteResGroup221 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_06, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [2, 2, 1, 2, 1];
  let Latency = 13;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup221, ReadAfterVecXLd], (instregex "^SHA1MSG2rm$")>;

def RYLWriteResGroup222 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_06, RYLPort01_05]> {
  let ResourceCycles = [2, 2, 1, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup222], (instregex "^SHA1MSG2rr$")>;

def RYLWriteResGroup223 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort01_05, RYLPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup223, ReadAfterVecXLd], (instregex "^SHA1NEXTErm$")>;

def RYLWriteResGroup224 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort01_05]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup224], (instregex "^SHA1NEXTErr$")>;

def RYLWriteResGroup225 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup225, ReadAfterVecXLd], (instregex "^SHA1RNDS4rmi$",
                                                                 "^SHA256RNDS2rm$")>;

def RYLWriteResGroup226 : SchedWriteRes<[RYLPort05]> {
  let Latency = 6;
}
def : InstRW<[RYLWriteResGroup226], (instregex "^SHA1RNDS4rri$",
                                                "^SHA256RNDS2rr$")>;

def RYLWriteResGroup227 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_06, RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [3, 2, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup227, ReadAfterVecXLd], (instregex "^SHA256MSG1rm$")>;

def RYLWriteResGroup228 : SchedWriteRes<[RYLPort00_01, RYLPort00_01_05, RYLPort00_06, RYLPort05]> {
  let ResourceCycles = [3, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup228], (instregex "^SHA256MSG1rr$")>;

def RYLWriteResGroup229 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup229, ReadAfterVecXLd], (instregex "^SHA256MSG2rm$")>;

def RYLWriteResGroup230 : SchedWriteRes<[RYLPort05]> {
  let ResourceCycles = [2];
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup230], (instregex "^SHA256MSG2rr$")>;

def RYLWriteResGroup231 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup231], (instregex "^SHRD16mri8$")>;

def RYLWriteResGroup232 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup232], (instregex "^SLDT(32|64)r$")>;

def RYLWriteResGroup233 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort05]> {
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup233], (instregex "^SMSW16r$")>;

def RYLWriteResGroup234 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort05]> {
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup234], (instregex "^SMSW(32|64)r$")>;

def RYLWriteResGroup235 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup235], (instregex "^STD$")>;

def RYLWriteResGroup236 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [1, 4, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup236], (instregex "^STI$")>;

def RYLWriteResGroup237 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup237], (instregex "^STOSB$")>;

def RYLWriteResGroup238 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort04_09, RYLPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup238], (instregex "^STOS(L|Q|W)$")>;

def RYLWriteResGroup239 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort01]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup239], (instregex "^STR(32|64)r$")>;

def RYLWriteResGroup240 : SchedWriteRes<[RYLPort00]> {
  let Latency = 2;
}
def : InstRW<[RYLWriteResGroup240], (instregex "^(TST|XAM)_F$",
                                                "^UCOM_FPPr$")>;

def RYLWriteResGroup241 : SchedWriteRes<[RYLPort00_01_05, RYLPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup241, ReadAfterVecXLd, ReadAfterVecXLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^VBLENDVP(D|S)rm$",
                                                                                                                                                   "^VPBLENDVBrm$")>;

def RYLWriteResGroup242 : SchedWriteRes<[RYLPort00_01_05]> {
  let ResourceCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup242], (instregex "^VBLENDVP(D|S)rr$",
                                                "^VPBLENDVBrr$")>;

def RYLWriteResGroup243 : SchedWriteRes<[RYLPort00, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [6, 7, 18];
  let Latency = 81;
  let NumMicroOps = 31;
}
def : InstRW<[RYLWriteResGroup243], (instregex "^VERRm$")>;

def RYLWriteResGroup244 : SchedWriteRes<[RYLPort00, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [6, 7, 17];
  let Latency = 74;
  let NumMicroOps = 30;
}
def : InstRW<[RYLWriteResGroup244], (instregex "^VERRr$")>;

def RYLWriteResGroup245 : SchedWriteRes<[RYLPort00, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [5, 8, 21];
  let Latency = 81;
  let NumMicroOps = 34;
}
def : InstRW<[RYLWriteResGroup245], (instregex "^VERWm$")>;

def RYLWriteResGroup246 : SchedWriteRes<[RYLPort00, RYLPort01, RYLPort02_03_11]> {
  let ResourceCycles = [5, 8, 20];
  let Latency = 74;
  let NumMicroOps = 33;
}
def : InstRW<[RYLWriteResGroup246], (instregex "^VERWr$")>;

def RYLWriteResGroup247 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 29;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup247, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Yrm$",
                                                                               "^VGATHERQPDYrm$",
                                                                               "^VPGATHER(D|Q)QYrm$",
                                                                               "^VPGATHERQDYrm$")>;

def RYLWriteResGroup248 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 20;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup248, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)rm$",
                                                                               "^VGATHERQPDrm$",
                                                                               "^VPGATHER(D|Q)Qrm$",
                                                                               "^VPGATHERQDrm$")>;

def RYLWriteResGroup249 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 8];
  let Latency = 30;
  let NumMicroOps = 12;
}
def : InstRW<[RYLWriteResGroup249, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSYrm$",
                                                                               "^VPGATHERDDYrm$")>;

def RYLWriteResGroup250 : SchedWriteRes<[RYLPort00, RYLPort00_01_05, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 28;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup250, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSrm$",
                                                                               "^VPGATHERDDrm$")>;

def RYLWriteResGroup251 : SchedWriteRes<[RYLPort01_05, RYLPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup251], (instregex "^VH(ADD|SUB)P(D|S)rr$")>;

def RYLWriteResGroup252 : SchedWriteRes<[RYLPort00_01_05, RYLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup252, ReadAfterVecYLd], (instregex "^VINSERT(F|I)128rm$",
                                                                 "^VP(ADD|SUB)(B|D|Q|W)Yrm$")>;

def RYLWriteResGroup253 : SchedWriteRes<[RYLPort00, RYLPort00_06, RYLPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup253], (instregex "^VLDMXCSR$")>;

def RYLWriteResGroup254 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort01, RYLPort01_05, RYLPort02_03, RYLPort02_03_07, RYLPort04, RYLPort05, RYLPort06]> {
  let ResourceCycles = [8, 1, 1, 1, 1, 1, 2, 3];
  let Latency = 40;
  let NumMicroOps = 18;
}
def : InstRW<[RYLWriteResGroup254], (instregex "^VMCLEARm$")>;

def RYLWriteResGroup255 : SchedWriteRes<[RYLPort00]> {
  let Latency = 5;
}
def : InstRW<[RYLWriteResGroup255], (instregex "^VMOVMSKP(D|S)Yrr$")>;

def RYLWriteResGroup256 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 521;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup256], (instregex "^VMOVNTDQmr$")>;

def RYLWriteResGroup257 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 473;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup257], (instregex "^VMOVNTPDmr$")>;

def RYLWriteResGroup258 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 494;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup258], (instregex "^VMOVNTPSYmr$")>;

def RYLWriteResGroup259 : SchedWriteRes<[RYLPort04_09, RYLPort07_08]> {
  let Latency = 470;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup259], (instregex "^VMOVNTPSmr$")>;

def RYLWriteResGroup260 : SchedWriteRes<[RYLPort02_03_11, RYLPort05]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup260, ReadAfterVecYLd], (instregex "^VPACK(S|U)S(DW|WB)Yrm$",
                                                                 "^VPCMPGTQYrm$")>;
def : InstRW<[RYLWriteResGroup260, ReadAfterVecXLd], (instregex "^VPCLMULQDQYrm$")>;

def RYLWriteResGroup261 : SchedWriteRes<[RYLPort01_05, RYLPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup261, ReadAfterVecYLd], (instregex "^VPBLENDWYrmi$",
                                                                 "^VSHUFP(D|S)Yrmi$")>;

def RYLWriteResGroup262 : SchedWriteRes<[RYLPort00_01, RYLPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup262], (instregex "^VPDP(BU|WS)SD((SY)?)rm$",
                                                "^VPDP(BU|WS)SD(S|Y)rm$")>;

def RYLWriteResGroup263 : SchedWriteRes<[RYLPort00_01, RYLPort01_05, RYLPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup263, ReadAfterVecYLd], (instregex "^VPH(ADD|SUB)SWYrm$")>;

def RYLWriteResGroup264 : SchedWriteRes<[RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort01_05_10]> {
  let ResourceCycles = [1, 2, 3, 3, 1];
  let Latency = 16;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup264], (instregex "^VZEROALL$")>;

def RYLWriteResGroup265 : SchedWriteRes<[RYLPort00_01_05_06]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup265], (instregex "^WAIT$")>;

def RYLWriteResGroup266 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [8, 6, 19, 63, 21, 15, 1, 10, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 144;
}
def : InstRW<[RYLWriteResGroup266], (instregex "^WRMSR$")>;

def RYLWriteResGroup267 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06, RYLPort01, RYLPort05]> {
  let ResourceCycles = [2, 1, 4, 1];
  let Latency = RYLModel.MaxLatency;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup267], (instregex "^WRPKRUr$")>;

def RYLWriteResGroup268 : SchedWriteRes<[RYLPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup268, WriteRMW], (instregex "^XADD(16|32|64)rm$")>;

def RYLWriteResGroup269 : SchedWriteRes<[RYLPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup269, WriteRMW], (instregex "^XADD8rm$")>;

def RYLWriteResGroup270 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 39;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup270, WriteRMW], (instregex "^XCHG(16|32)rm$")>;

def RYLWriteResGroup271 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let ResourceCycles = [5, 1];
  let Latency = 39;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup271, WriteRMW], (instregex "^XCHG64rm$")>;

def RYLWriteResGroup272 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 40;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup272, WriteRMW], (instregex "^XCHG8rm$")>;

def RYLWriteResGroup273 : SchedWriteRes<[RYLPort00, RYLPort00_01_05_06, RYLPort00_05, RYLPort01, RYLPort05, RYLPort06]> {
  let ResourceCycles = [2, 4, 2, 1, 2, 4];
  let Latency = 17;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup273], (instregex "^XCH_F$")>;

def RYLWriteResGroup274 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01]> {
  let ResourceCycles = [7, 3, 8, 5];
  let Latency = 4;
  let NumMicroOps = 23;
}
def : InstRW<[RYLWriteResGroup274], (instregex "^XGETBV$")>;

def RYLWriteResGroup275 : SchedWriteRes<[RYLPort00_01_05_06_10, RYLPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup275], (instregex "^XLAT$")>;

def RYLWriteResGroup276 : SchedWriteRes<[RYLPort00_01_05_06, RYLPort01, RYLPort02_03, RYLPort06]> {
  let ResourceCycles = [21, 1, 1, 8];
  let Latency = 37;
  let NumMicroOps = 31;
}
def : InstRW<[RYLWriteResGroup276], (instregex "^XRSTOR((S|64|S64)?)$")>;

def RYLWriteResGroup277 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 42;
  let NumMicroOps = 140;
}
def : InstRW<[RYLWriteResGroup277], (instregex "^XSAVE$")>;

def RYLWriteResGroup278 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 41;
  let NumMicroOps = 140;
}
def : InstRW<[RYLWriteResGroup278], (instregex "^XSAVE64$")>;

def RYLWriteResGroup279 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [1, 19, 36, 52, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 151;
}
def : InstRW<[RYLWriteResGroup279], (instregex "^XSAVEC$")>;

def RYLWriteResGroup280 : SchedWriteRes<[RYLPort00, RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [1, 19, 36, 53, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 152;
}
def : InstRW<[RYLWriteResGroup280], (instregex "^XSAVEC64$")>;

def RYLWriteResGroup281 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [25, 35, 52, 27, 4, 1, 10, 1];
  let Latency = 46;
  let NumMicroOps = 155;
}
def : InstRW<[RYLWriteResGroup281], (instregex "^XSAVEOPT$")>;

def RYLWriteResGroup282 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [25, 35, 53, 27, 4, 1, 10, 1];
  let Latency = 46;
  let NumMicroOps = 156;
}
def : InstRW<[RYLWriteResGroup282], (instregex "^XSAVEOPT64$")>;

def RYLWriteResGroup283 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [23, 32, 53, 29, 30, 4, 2, 9, 2];
  let Latency = 42;
  let NumMicroOps = 184;
}
def : InstRW<[RYLWriteResGroup283], (instregex "^XSAVES$")>;

def RYLWriteResGroup284 : SchedWriteRes<[RYLPort00_01, RYLPort00_05, RYLPort00_06, RYLPort01, RYLPort01_05, RYLPort02_03_11, RYLPort04_09, RYLPort05, RYLPort07_08]> {
  let ResourceCycles = [23, 33, 53, 29, 32, 4, 2, 8, 2];
  let Latency = 42;
  let NumMicroOps = 186;
}
def : InstRW<[RYLWriteResGroup284], (instregex "^XSAVES64$")>;

def RYLWriteResGroup285 : SchedWriteRes<[RYLPort00_01_05, RYLPort00_01_05_06_10, RYLPort00_05_06, RYLPort00_06, RYLPort01, RYLPort01_05_10, RYLPort05]> {
  let ResourceCycles = [4, 23, 2, 14, 8, 1, 2];
  let Latency = 5;
  let NumMicroOps = 54;
}
def : InstRW<[RYLWriteResGroup285], (instregex "^XSETBV$")>;

}
#endif // INTEL_FEATURE_CPU_RYL
