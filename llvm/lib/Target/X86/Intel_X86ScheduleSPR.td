//=- Intel_X86ScheduleSPR.td - X86 Sapphirerapids Scheduling -*- tablegen -*-=//
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
// This file defines the machine model for Intel Sapphirerapids to support
// instruction scheduling and other instruction cost heuristics.
//
// Automatically generated file by smg tool.
//
//===----------------------------------------------------------------------===//

def SPRModel : SchedMachineModel {
  // Sapphirerapids can allocate 6 uops per cycle.
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

let SchedModel = SPRModel in {

// Sapphirerapids can issue micro-ops to 17 different ports in one cycle.
def SPRPort00 : ProcResource<1>;
def SPRPort01 : ProcResource<1>;
def SPRPort02 : ProcResource<1>;
def SPRPort03 : ProcResource<1>;
def SPRPort04 : ProcResource<1>;
def SPRPort05 : ProcResource<1>;
def SPRPort06 : ProcResource<1>;
def SPRPort07 : ProcResource<1>;
def SPRPort08 : ProcResource<1>;
def SPRPort09 : ProcResource<1>;
def SPRPort10 : ProcResource<1>;
def SPRPort11 : ProcResource<1>;

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def SPRPortInvalid :ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
def SPRPort00_01          : ProcResGroup<[SPRPort00, SPRPort01]>;
def SPRPort00_01_05       : ProcResGroup<[SPRPort00, SPRPort01, SPRPort05]>;
def SPRPort00_01_05_06    : ProcResGroup<[SPRPort00, SPRPort01, SPRPort05, SPRPort06]>;
def SPRPort00_05          : ProcResGroup<[SPRPort00, SPRPort05]>;
def SPRPort00_05_06       : ProcResGroup<[SPRPort00, SPRPort05, SPRPort06]>;
def SPRPort00_06          : ProcResGroup<[SPRPort00, SPRPort06]>;
def SPRPort01_05          : ProcResGroup<[SPRPort01, SPRPort05]>;
def SPRPort01_05_10       : ProcResGroup<[SPRPort01, SPRPort05, SPRPort10]>;
def SPRPort02_03          : ProcResGroup<[SPRPort02, SPRPort03]>;
def SPRPort02_03_07       : ProcResGroup<[SPRPort02, SPRPort03, SPRPort07]>;
def SPRPort02_03_11       : ProcResGroup<[SPRPort02, SPRPort03, SPRPort11]>;
def SPRPort07_08          : ProcResGroup<[SPRPort07, SPRPort08]>;

// EU has 112 reservation stations.
def SPRPort00_01_05_06_10 : ProcResGroup<[SPRPort00, SPRPort01, SPRPort05,
                                          SPRPort06, SPRPort10]> {
  let BufferSize = 112;
}

// STD has 48 reservation stations.
def SPRPort04_09          : ProcResGroup<[SPRPort04, SPRPort09]> {
  let BufferSize = 48;
}

// MEM has 72 reservation stations.
def SPRPort02_03_07_08_11 : ProcResGroup<[SPRPort02, SPRPort03, SPRPort07,
                                          SPRPort08, SPRPort11]> {
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
multiclass SPRWriteResPair<X86FoldableSchedWrite SchedRW,
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
  def : WriteRes<SchedRW.Folded, !listconcat([SPRPort02_03_11], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ResourceCycles = !listconcat([1], Res);
    let NumMicroOps = !add(UOps, 1);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [SPRPort00_06]>;
defm : X86WriteRes<WriteADCLd, [SPRPort00_01_05_06_10, SPRPort00_06], 11, [1, 1], 2>;
defm : SPRWriteResPair<WriteAESDecEnc, [SPRPort00_01], 5, [1], 1, 7>;
defm : SPRWriteResPair<WriteAESIMC, [SPRPort00_01], 8, [2], 2, 7>;
defm : X86WriteRes<WriteAESKeyGen, [SPRPort00, SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort01_05, SPRPort05], 7, [4, 1, 1, 2, 3, 3], 14>;
defm : X86WriteRes<WriteAESKeyGenLd, [SPRPort00, SPRPort00_01, SPRPort00_06, SPRPort01_05, SPRPort02_03_11, SPRPort05], 12, [4, 1, 2, 3, 1, 3], 14>;
def : WriteRes<WriteALU, [SPRPort00_01_05_06_10]>;
def : WriteRes<WriteALULd, [SPRPort00_01_05_06_10]> {
  let Latency = 11;
}
defm : SPRWriteResPair<WriteBEXTR, [SPRPort00_06, SPRPort01], 6, [1, 1], 2>;
defm : SPRWriteResPair<WriteBLS, [SPRPort01_05_10], 2, [1]>;
defm : SPRWriteResPair<WriteBSF, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteBSR, [SPRPort01], 3, [1]>;
def : WriteRes<WriteBSWAP32, [SPRPort01]>;
defm : X86WriteRes<WriteBSWAP64, [SPRPort00_06, SPRPort01], 2, [1, 1], 2>;
defm : SPRWriteResPair<WriteBZHI, [SPRPort01], 3, [1]>;
def : WriteRes<WriteBitTest, [SPRPort01]>;
defm : X86WriteRes<WriteBitTestImmLd, [SPRPort01, SPRPort02_03_11], 6, [1, 1], 2>;
defm : X86WriteRes<WriteBitTestRegLd, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11], 11, [4, 2, 1, 2, 1], 10>;
def : WriteRes<WriteBitTestSet, [SPRPort01]>;
def : WriteRes<WriteBitTestSetImmLd, [SPRPort01]> {
  let Latency = 11;
}
defm : X86WriteRes<WriteBitTestSetRegLd, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10], 17, [3, 2, 1, 2], 8>;
defm : SPRWriteResPair<WriteBlend, [SPRPort01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteBlendY, [SPRPort00_01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteCLMul, [SPRPort05], 3, [1], 1, 7>;
defm : SPRWriteResPair<WriteCMOV, [SPRPort00_06], 1, [1], 1, 6>;
defm : X86WriteRes<WriteCMPXCHG, [SPRPort00_01_05_06_10, SPRPort00_06], 3, [3, 2], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08], 12, [1, 2, 1, 1, 1], 6>;
defm : SPRWriteResPair<WriteCRC32, [SPRPort01], 3, [1]>;
defm : X86WriteRes<WriteCvtI2PD, [SPRPort00_01, SPRPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDLd, [SPRPort00_01, SPRPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDY, [SPRPort00_01, SPRPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2PDYLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtI2PDZ, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteCvtI2PS, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteCvtI2PSY, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteCvtI2PSZ, [SPRPort00], 4, [1], 1, 8>;
defm : X86WriteRes<WriteCvtI2SD, [SPRPort00_01, SPRPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SDLd, [SPRPort00_01, SPRPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtI2SS, [SPRPort00_01, SPRPort00_01_05, SPRPort05], 9, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtI2SSLd, [SPRPort00_01, SPRPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2I, [SPRPort00_01, SPRPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2ILd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2IY, [SPRPort00_01, SPRPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2IYLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2IZ, [SPRPort00, SPRPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPD2IZLd, [SPRPort00, SPRPort02_03_11], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtPD2PS, [SPRPort00_01, SPRPort05], 5, [1, 1], 2, 7>;
defm : SPRWriteResPair<WriteCvtPD2PSY, [SPRPort00_01, SPRPort05], 7, [1, 1], 2, 8>;
defm : SPRWriteResPair<WriteCvtPD2PSZ, [SPRPort00, SPRPort05], 7, [1, 1], 2, 8>;
defm : X86WriteRes<WriteCvtPH2PS, [SPRPort00_01, SPRPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSY, [SPRPort00_01, SPRPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPH2PSYLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtPH2PSZ, [SPRPort00, SPRPort05], 11, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtPS2I, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteCvtPS2IY, [SPRPort00_01], 4, [1], 1, 8>;
defm : X86WriteRes<WriteCvtPS2IZ, [SPRPort00, SPRPort00_05, SPRPort05], 10, [1, 2, 1], 4>;
defm : X86WriteRes<WriteCvtPS2IZLd, [SPRPort00, SPRPort00_05, SPRPort00_06, SPRPort02_03_11, SPRPort05], 18, [1, 2, 1, 1, 1], 6>;
defm : X86WriteRes<WriteCvtPS2PD, [SPRPort00_01, SPRPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDLd, [SPRPort00_01, SPRPort02_03_11], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDY, [SPRPort00_01, SPRPort05], 7, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PDYLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtPS2PDZ, [SPRPort00, SPRPort05], 7, [1, 1], 2, 6>;
defm : X86WriteRes<WriteCvtPS2PH, [SPRPort00_01, SPRPort05], 6, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHSt, [SPRPort00_01, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHY, [SPRPort00_01, SPRPort05], 8, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [SPRPort00_01, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PHZ, [SPRPort00, SPRPort05], 11, [1, 1], 2>;
defm : X86WriteRes<WriteCvtPS2PHZSt, [SPRPort00, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1], 3>;
defm : SPRWriteResPair<WriteCvtSD2I, [SPRPort00, SPRPort00_01], 7, [1, 1], 2>;
defm : SPRWriteResPair<WriteCvtSD2SS, [SPRPort00_01, SPRPort05], 5, [1, 1], 2, 7>;
defm : X86WriteRes<WriteCvtSS2I, [SPRPort00, SPRPort00_01, SPRPort00_01_05], 8, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtSS2ILd, [SPRPort00, SPRPort00_01, SPRPort02_03_11], 12, [1, 1, 1], 3>;
defm : X86WriteRes<WriteCvtSS2SD, [SPRPort00_01, SPRPort05], 5, [1, 1], 2>;
defm : X86WriteRes<WriteCvtSS2SDLd, [SPRPort00_01, SPRPort02_03_11], 11, [1, 1], 2>;
defm : SPRWriteResPair<WriteDPPD, [SPRPort00_01, SPRPort01_05], 9, [2, 1], 3, 7>;
defm : SPRWriteResPair<WriteDPPS, [SPRPort00_01, SPRPort00_06, SPRPort01_05, SPRPort05], 14, [2, 1, 2, 1], 6, 7>;
defm : SPRWriteResPair<WriteDPPSY, [SPRPort00_01, SPRPort00_06, SPRPort01_05, SPRPort05], 14, [2, 1, 2, 1], 6, 8>;
defm : SPRWriteResPair<WriteDiv16, [SPRPort00_01_05_06_10, SPRPort01], 16, [1, 3], 4, 4>;
defm : SPRWriteResPair<WriteDiv32, [SPRPort00_01_05_06_10, SPRPort01], 15, [1, 3], 4, 4>;
defm : SPRWriteResPair<WriteDiv64, [SPRPort01], 18, [3], 3>;
defm : X86WriteRes<WriteDiv8, [SPRPort01], 17, [3], 3>;
defm : X86WriteRes<WriteDiv8Ld, [SPRPort01], 22, [3], 3>;
defm : X86WriteRes<WriteEMMS, [SPRPort00, SPRPort00_05, SPRPort00_06], 10, [1, 8, 1], 10>;
defm : SPRWriteResPair<WriteFAdd, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFAdd64, [SPRPort01_05], 3, [1], 1, 7>;
defm : SPRWriteResPair<WriteFAdd64X, [SPRPort01_05], 3, [1], 1, 7>;
defm : SPRWriteResPair<WriteFAdd64Y, [SPRPort01_05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteFAdd64Z, [SPRPort00_05], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFAddX, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFAddY, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFAddZ, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFBlend, [SPRPort00_01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteFBlendY, [SPRPort00_01_05], 1, [1], 1, 8>;
def : WriteRes<WriteFCMOV, [SPRPort01]> {
  let Latency = 3;
}
defm : SPRWriteResPair<WriteFCmp, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFCmp64, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFCmp64X, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFCmp64Y, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFCmp64Z, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFCmpX, [SPRPort00_01], 4, [1], 1, 7>;
def : WriteRes<WriteFCmpY, [SPRPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFCmpYLd, [SPRPort00_01, SPRPort02_03_11], 12, [1, 1], 2>;
def : WriteRes<WriteFCmpZ, [SPRPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFCmpZLd, [SPRPort00, SPRPort02_03_11], 12, [1, 1], 2>;
def : WriteRes<WriteFCom, [SPRPort00]> {
  let Latency = 2;
}
defm : X86WriteRes<WriteFComLd, [SPRPort02_03, SPRPort05], 8, [1, 1], 2>;
defm : SPRWriteResPair<WriteFComX, [SPRPort00], 3, [1]>;
defm : SPRWriteResPair<WriteFDiv, [SPRPort00], 11, [1], 1, 7>;
defm : SPRWriteResPair<WriteFDiv64, [SPRPort00], 14, [1], 1, 6>;
defm : SPRWriteResPair<WriteFDiv64X, [SPRPort00], 14, [1], 1, 6>;
defm : SPRWriteResPair<WriteFDiv64Y, [SPRPort00], 14, [1], 1, 7>;
defm : SPRWriteResPair<WriteFDiv64Z, [SPRPort00, SPRPort00_05], 23, [2, 1], 3, 7>;
defm : SPRWriteResPair<WriteFDivX, [SPRPort00], 11, [1], 1, 7>;
defm : SPRWriteResPair<WriteFDivY, [SPRPort00], 11, [1], 1, 8>;
defm : SPRWriteResPair<WriteFDivZ, [SPRPort00, SPRPort00_05], 18, [2, 1], 3, 7>;
defm : SPRWriteResPair<WriteFHAdd, [SPRPort01_05, SPRPort05], 6, [1, 2], 3, 6>;
defm : SPRWriteResPair<WriteFHAddY, [SPRPort01_05, SPRPort05], 5, [1, 2], 3, 8>;
def : WriteRes<WriteFLD0, [SPRPort00_05]>;
defm : X86WriteRes<WriteFLD1, [SPRPort00_05], 1, [2], 2>;
defm : X86WriteRes<WriteFLDC, [SPRPort00_05], 1, [2], 2>;
def : WriteRes<WriteFLoad, [SPRPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadX, [SPRPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteFLoadY, [SPRPort02_03_11]> {
  let Latency = 8;
}
defm : SPRWriteResPair<WriteFLogic, [SPRPort00_01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteFLogicY, [SPRPort00_01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFLogicZ, [SPRPort00_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFMA, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMAX, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMAY, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFMAZ, [SPRPort00], 4, [1], 1, 8>;
def : WriteRes<WriteFMOVMSK, [SPRPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteFMaskedLoad, [SPRPort00_01_05, SPRPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [SPRPort00_01_05, SPRPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedStore32, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMoveX, [], 1, [], 0>;
defm : X86WriteRes<WriteFMoveY, [], 1, [], 0>;
def : WriteRes<WriteFMoveZ, [SPRPort00_05]>;
defm : SPRWriteResPair<WriteFMul, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMul64, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMul64X, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMul64Y, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFMul64Z, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFMulX, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFMulY, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFMulZ, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFRcp, [SPRPort00], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFRcpX, [SPRPort00], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFRcpY, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFRcpZ, [SPRPort00, SPRPort00_05], 7, [2, 1], 3, 7>;
defm : SPRWriteResPair<WriteFRnd, [SPRPort00_01], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFRndY, [SPRPort00_01], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFRndZ, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFRsqrt, [SPRPort00], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFRsqrtX, [SPRPort00], 4, [1], 1, 7>;
defm : SPRWriteResPair<WriteFRsqrtY, [SPRPort00], 4, [1], 1, 8>;
defm : SPRWriteResPair<WriteFRsqrtZ, [SPRPort00, SPRPort00_05], 9, [2, 1], 3>;
defm : SPRWriteResPair<WriteFShuffle, [SPRPort05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteFShuffle256, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteFShuffleY, [SPRPort05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFShuffleZ, [SPRPort05], 1, [1], 1, 8>;
def : WriteRes<WriteFSign, [SPRPort00]>;
defm : SPRWriteResPair<WriteFSqrt, [SPRPort00], 12, [1], 1, 7>;
defm : SPRWriteResPair<WriteFSqrt64, [SPRPort00], 18, [1], 1, 6>;
defm : SPRWriteResPair<WriteFSqrt64X, [SPRPort00], 18, [1], 1, 6>;
defm : SPRWriteResPair<WriteFSqrt64Y, [SPRPort00], 18, [1], 1, 3>;
// Warning: negtive load latency.
defm : SPRWriteResPair<WriteFSqrt64Z, [SPRPort00, SPRPort00_05], 32, [2, 1], 3, -1>;
def : WriteRes<WriteFSqrt80, [SPRPortInvalid, SPRPort00]> {
  let ResourceCycles = [7, 1];
  let Latency = 21;
}
defm : SPRWriteResPair<WriteFSqrtX, [SPRPort00], 12, [1], 1, 7>;
defm : SPRWriteResPair<WriteFSqrtY, [SPRPort00], 12, [1], 1, 8>;
defm : SPRWriteResPair<WriteFSqrtZ, [SPRPort00, SPRPort00_05], 20, [2, 1], 3, 7>;
defm : X86WriteRes<WriteFStore, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [SPRPort04_09, SPRPort07_08], 518, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [SPRPort04_09, SPRPort07_08], 542, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreX, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteFTest, [SPRPort00], 3, [1]>;
defm : SPRWriteResPair<WriteFTestY, [SPRPort00], 5, [1], 1, 6>;
defm : SPRWriteResPair<WriteFVarBlend, [SPRPort00_01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteFVarBlendY, [SPRPort00_01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFVarBlendZ, [SPRPort00_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFVarShuffle, [SPRPort05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteFVarShuffle256, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteFVarShuffleY, [SPRPort05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteFVarShuffleZ, [SPRPort05], 1, [1], 1, 8>;
def : WriteRes<WriteFence, [SPRPort00_06]> {
  let Latency = 2;
}
defm : SPRWriteResPair<WriteIDiv16, [SPRPort00_01_05_06_10, SPRPort01], 16, [1, 3], 4, 4>;
defm : SPRWriteResPair<WriteIDiv32, [SPRPort00_01_05_06_10, SPRPort01], 15, [1, 3], 4, 4>;
defm : SPRWriteResPair<WriteIDiv64, [SPRPort01], 18, [3], 3>;
defm : X86WriteRes<WriteIDiv8, [SPRPort01], 17, [3], 3>;
defm : X86WriteRes<WriteIDiv8Ld, [SPRPort01], 22, [3], 3>;
defm : SPRWriteResPair<WriteIMul16, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01], 5, [2, 1, 1], 4>;
defm : SPRWriteResPair<WriteIMul16Imm, [SPRPort00_01_05_06_10, SPRPort01], 4, [1, 1], 2>;
defm : SPRWriteResPair<WriteIMul16Reg, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteIMul32, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01], 4, [1, 1, 1], 3>;
defm : SPRWriteResPair<WriteIMul32Imm, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteIMul32Reg, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteIMul64, [SPRPort01, SPRPort05], 4, [1, 1], 2>;
defm : SPRWriteResPair<WriteIMul64Imm, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteIMul64Reg, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteIMul8, [SPRPort01], 3, [1]>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
def : WriteRes<WriteIMulHLd, []> {
  let Latency = 3;
}
def : WriteRes<WriteJump, [SPRPort00_06]>;
defm : X86WriteRes<WriteJumpLd, [SPRPort00_06, SPRPort02_03], 6, [1, 1], 2>;
def : WriteRes<WriteLAHFSAHF, [SPRPort00_06]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteLDMXCSR, [SPRPort00, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11], 7, [1, 1, 1, 1], 4>;
def : WriteRes<WriteLEA, [SPRPort01]>;
defm : SPRWriteResPair<WriteLZCNT, [SPRPort01], 3, [1]>;
def : WriteRes<WriteLoad, [SPRPort02_03_11]> {
  let Latency = 5;
}
def : WriteRes<WriteMMXMOVMSK, [SPRPort00]> {
  let Latency = 3;
}
defm : SPRWriteResPair<WriteMPSAD, [SPRPort01_05, SPRPort05], 4, [1, 1], 2, 7>;
defm : SPRWriteResPair<WriteMPSADY, [SPRPort01_05, SPRPort05], 4, [1, 1], 2, 8>;
defm : SPRWriteResPair<WriteMULX32, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01], 4, [1, 1, 1], 2>;
defm : SPRWriteResPair<WriteMULX64, [SPRPort01, SPRPort05], 4, [1, 1]>;
def : WriteRes<WriteMicrocoded, [SPRPort00_01_05_06]> {
  let Latency = SPRModel.MaxLatency;
}
def : WriteRes<WriteMove, [SPRPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteNop, [], 1, [], 0>;
defm : X86WriteRes<WritePCmpEStrI, [SPRPort00, SPRPort00_01_05, SPRPort00_06, SPRPort01, SPRPort05], 16, [3, 2, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrILd, [SPRPort00, SPRPort00_01_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05], 31, [3, 1, 1, 1, 1, 1], 8>;
defm : X86WriteRes<WritePCmpEStrM, [SPRPort00, SPRPort00_01_05, SPRPort00_06, SPRPort01, SPRPort05], 16, [3, 3, 1, 1, 1], 9>;
defm : X86WriteRes<WritePCmpEStrMLd, [SPRPort00, SPRPort00_01_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05], 17, [3, 2, 1, 1, 1, 1], 9>;
defm : SPRWriteResPair<WritePCmpIStrI, [SPRPort00], 11, [3], 3, 20>;
defm : SPRWriteResPair<WritePCmpIStrM, [SPRPort00], 11, [3], 3>;
defm : SPRWriteResPair<WritePHAdd, [SPRPort00_05, SPRPort05], 3, [1, 2], 3, 8>;
defm : SPRWriteResPair<WritePHAddX, [SPRPort00_01_05, SPRPort01_05], 2, [1, 2], 3, 7>;
defm : SPRWriteResPair<WritePHAddY, [SPRPort00_01_05, SPRPort01_05], 2, [1, 2], 3, 8>;
defm : SPRWriteResPair<WritePHMINPOS, [SPRPort00], 4, [1], 1, 7>;
defm : SPRWriteResPair<WritePMULLD, [SPRPort00_01], 10, [2], 2, 8>;
defm : SPRWriteResPair<WritePMULLDY, [SPRPort00_01], 10, [2], 2, 8>;
defm : SPRWriteResPair<WritePMULLDZ, [SPRPort00], 10, [2], 2, 8>;
defm : SPRWriteResPair<WritePOPCNT, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WritePSADBW, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WritePSADBWX, [SPRPort05], 3, [1], 1, 7>;
defm : SPRWriteResPair<WritePSADBWY, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WritePSADBWZ, [SPRPort05], 3, [1], 1, 8>;
defm : X86WriteRes<WriteRMW, [SPRPort02_03_11, SPRPort04_09, SPRPort07_08], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteRotate, [SPRPort00_01_05_06_10, SPRPort00_06], 2, [1, 2], 3>;
defm : X86WriteRes<WriteRotateLd, [SPRPort00_01_05_06_10, SPRPort00_06], 12, [1, 2], 3>;
defm : X86WriteRes<WriteRotateCL, [SPRPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteRotateCLLd, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01], 19, [2, 3, 2], 7>;
defm : X86WriteRes<WriteSETCC, [SPRPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteSETCCStore, [SPRPort00_06, SPRPort04_09, SPRPort07_08], 13, [2, 1, 1], 4>;
defm : X86WriteRes<WriteSHDmrcl, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1, 1, 1, 1], 6>;
defm : X86WriteRes<WriteSHDmri, [SPRPort00_01_05_06_10, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1, 1, 1], 5>;
defm : X86WriteRes<WriteSHDrrcl, [SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01], 5, [1, 1, 1], 3>;
def : WriteRes<WriteSHDrri, [SPRPort01]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteSTMXCSR, [SPRPort00, SPRPort00_06, SPRPort04_09, SPRPort07_08], 12, [1, 1, 1, 1], 4>;
def : WriteRes<WriteShift, [SPRPort00_06]>;
def : WriteRes<WriteShiftLd, [SPRPort00_06]> {
  let Latency = 12;
}
defm : X86WriteRes<WriteShiftCL, [SPRPort00_06], 2, [2], 2>;
defm : X86WriteRes<WriteShiftCLLd, [SPRPort00_06], 12, [2], 2>;
defm : SPRWriteResPair<WriteShuffle, [SPRPort05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteShuffle256, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteShuffleX, [SPRPort01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteShuffleY, [SPRPort01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteShuffleZ, [SPRPort05], 3, [1], 1, 6>;
defm : X86WriteRes<WriteStore, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteStoreNT, [SPRPort04_09, SPRPort07_08], 512, [1, 1], 2>;
def : WriteRes<WriteSystem, [SPRPort00_01_05_06]> {
  let Latency = SPRModel.MaxLatency;
}
defm : SPRWriteResPair<WriteTZCNT, [SPRPort01], 3, [1]>;
defm : SPRWriteResPair<WriteVPMOV256, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarBlend, [SPRPort00_01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVarBlendY, [SPRPort00_01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarBlendZ, [SPRPort00_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarShuffle, [SPRPort00, SPRPort05], 3, [1, 1], 2, 8>;
defm : X86WriteRes<WriteVarShuffle256, [SPRPort05], 6, [2], 2>;
defm : X86WriteRes<WriteVarShuffle256Ld, [SPRPort02_03_11, SPRPort05], 11, [1, 1], 2>;
defm : SPRWriteResPair<WriteVarShuffleX, [SPRPort01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVarShuffleY, [SPRPort01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarShuffleZ, [SPRPort05], 3, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarVecShift, [SPRPort00_01], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVarVecShiftY, [SPRPort00_01], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVarVecShiftZ, [SPRPort00], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecALU, [SPRPort00], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecALUX, [SPRPort00_01], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVecALUY, [SPRPort00_01], 1, [1], 1, 8>;
def : WriteRes<WriteVecALUZ, [SPRPort05]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteVecALUZLd, [SPRPort00, SPRPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecExtract, [SPRPort00, SPRPort01_05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecExtractSt, [SPRPort01_05, SPRPort04_09, SPRPort07_08], 19, [1, 1, 1], 3>;
defm : SPRWriteResPair<WriteVecIMul, [SPRPort00], 5, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecIMulX, [SPRPort00_01], 5, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecIMulY, [SPRPort00_01], 5, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecIMulZ, [SPRPort00], 5, [1], 1, 8>;
defm : X86WriteRes<WriteVecInsert, [SPRPort01_05, SPRPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecInsertLd, [SPRPort01_05, SPRPort02_03_11], 8, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [SPRPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNT, [SPRPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadNTY, [SPRPort02_03_11]> {
  let Latency = 8;
}
def : WriteRes<WriteVecLoadX, [SPRPort02_03_11]> {
  let Latency = 7;
}
def : WriteRes<WriteVecLoadY, [SPRPort02_03_11]> {
  let Latency = 8;
}
defm : SPRWriteResPair<WriteVecLogic, [SPRPort00_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecLogicX, [SPRPort00_01_05], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVecLogicY, [SPRPort00_01_05], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecLogicZ, [SPRPort00_05], 1, [1], 1, 8>;
def : WriteRes<WriteVecMOVMSK, [SPRPort00]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMOVMSKY, [SPRPort00]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [SPRPort00_01_05, SPRPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [SPRPort00_01_05, SPRPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedStore32, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [SPRPort00, SPRPort04_09, SPRPort07_08], 14, [1, 1, 1], 3>;
def : WriteRes<WriteVecMove, [SPRPort00_05]>;
def : WriteRes<WriteVecMoveFromGpr, [SPRPort05]> {
  let Latency = 3;
}
def : WriteRes<WriteVecMoveToGpr, [SPRPort00]> {
  let Latency = 3;
}
defm : X86WriteRes<WriteVecMoveX, [], 1, [], 0>;
def : WriteRes<WriteVecMoveY, [SPRPort00_01_05]>;
def : WriteRes<WriteVecMoveZ, [SPRPort00_05]>;
defm : SPRWriteResPair<WriteVecShift, [SPRPort00], 1, [1], 1, 8>;
def : WriteRes<WriteVecShiftImm, [SPRPort00]>;
defm : SPRWriteResPair<WriteVecShiftImmX, [SPRPort00_01], 1, [1], 1, 7>;
defm : SPRWriteResPair<WriteVecShiftImmY, [SPRPort00_01], 1, [1], 1, 8>;
defm : SPRWriteResPair<WriteVecShiftImmZ, [SPRPort00], 1, [1], 1, 8>;
defm : X86WriteRes<WriteVecShiftX, [SPRPort00_01, SPRPort01_05], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftXLd, [SPRPort00_01, SPRPort02_03_11], 8, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftY, [SPRPort00_01, SPRPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftYLd, [SPRPort00_01, SPRPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftZ, [SPRPort00, SPRPort05], 4, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftZLd, [SPRPort00, SPRPort02_03_11], 9, [1, 1], 2>;
defm : X86WriteRes<WriteVecStore, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [SPRPort04_09, SPRPort07_08], 511, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [SPRPort04_09, SPRPort07_08], 507, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreX, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [SPRPort04_09, SPRPort07_08], 12, [1, 1], 2>;
defm : SPRWriteResPair<WriteVecTest, [SPRPort00, SPRPort05], 4, [1, 1], 2>;
defm : SPRWriteResPair<WriteVecTestY, [SPRPort00, SPRPort05], 6, [1, 1], 2, 6>;
defm : X86WriteRes<WriteXCHG, [SPRPort00_01_05_06_10], 2, [3], 3>;
def : WriteRes<WriteZero, []>;

// Infered SchedWriteRes and InstRW definition.

def SPRWriteResGroup0 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup0, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(16|32|64)mr$")>;

def SPRWriteResGroup1 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup1], (instregex "^JMP(16|32|64)m$",
                                             "^RET(16|32)$",
                                             "^RORX(32|64)mi$")>;
def : InstRW<[SPRWriteResGroup1, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                        "^AD(C|O)X(32|64)rm$")>;

def SPRWriteResGroup2 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup2], (instregex "^(ADC|SBB)8mi$")>;

def SPRWriteResGroup3 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup3, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)8mr$")>;

def SPRWriteResGroup4 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup4], (instregex "^CMP(8|16|32)mi$",
                                             "^CMP(16|32|64)mi8$",
                                             "^CMP64mi32$",
                                             "^MOV(8|16)rm$",
                                             "^MOVZX16rm8$",
                                             "^POP(16|32)r$")>;
def : InstRW<[SPRWriteResGroup4, ReadAfterLd], (instregex "^(ADD|CMP|SUB)(8|16|32|64)rm$",
                                                          "^(OR|AND|XOR)(8|16|32)rm$")>;
def : InstRW<[SPRWriteResGroup4, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^CMP(8|16|32|64)mr$")>;

def SPRWriteResGroup5 : SchedWriteRes<[]> {
  let NumMicroOps = 0;
}
def : InstRW<[SPRWriteResGroup5], (instregex "^(ADD|SUB)64ri8$",
                                             "^CLC$",
                                             "^(DE|IN)C64r$",
                                             "^JMP_1$",
                                             "^MOV64rr$",
                                             "^MOV64rr_REV$",
                                             "^VMOV(APD|UPS)Zrr$",
                                             "^VMOV(APD|UPS)Zrr_REV$",
                                             "^VMOV(APS|UPD)Zrr$",
                                             "^VMOV(APS|UPD)Zrr_REV$",
                                             "^VMOVDQA(32|64)Z((256)?)rr$",
                                             "^VMOVDQA(32|64)Z((256)?)rr_REV$",
                                             "^VMOVDQ(A|U)Yrr$",
                                             "^VMOVDQ(A|U)Yrr_REV$",
                                             "^VMOVDQU(8|16|32|64)Z((256)?)rr$",
                                             "^VMOVDQU(8|16|32|64)Z((256)?)rr_REV$")>;

def SPRWriteResGroup6 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup6], (instregex "^(OR|ADD|SUB|XOR)8mi$",
                                             "^AND8mi$",
                                             "^(DEC|NEG|NOT)8m$",
                                             "^INC8m$")>;
def : InstRW<[SPRWriteResGroup6, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(OR|ADD|SUB|XOR)8mr$",
                                                                                                                           "^AND8mr$")>;

def SPRWriteResGroup7 : SchedWriteRes<[SPRPort01_05, SPRPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup7, ReadAfterVecXLd], (instregex "^(V?)ADDPSrm$",
                                                              "^((V|ADD)?)SUBPSrm$",
                                                              "^V(ADD|SUB)PSZ128rm((b|k|bk|kz|bkz)?)$",
                                                              "^VADDSUBPSrm$")>;
def : InstRW<[SPRWriteResGroup7, ReadAfterVecLd], (instregex "^(V?)(ADD|SUB)SSrm_Int$",
                                                             "^V(ADD|SUB)SSZrm_Int((k|kz)?)$")>;

def SPRWriteResGroup8 : SchedWriteRes<[SPRPort01_05]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup8], (instregex "^(V?)ADDPSrr$",
                                             "^(V?)(ADD|SUB)SSrr_Int$",
                                             "^((V|ADD)?)SUBPSrr$",
                                             "^V(ADD|SUB)PSYrr$",
                                             "^V(ADD|SUB)PSZ(128|256)rr(k?)$",
                                             "^V(ADD|SUB)SSZrr(b?)_Int(k?)$",
                                             "^VADDSUBPS(Y?)rr$",
                                             "^VPMOV(S|Z)XBWZ128rrk(z?)$",
                                             "^VPSHUFBZ(128|256)rrk(z?)$",
                                             "^VPSHUF(H|L)WZ(128|256)rik(z?)$",
                                             "^VPUNPCK(HBW|LWD)Z(128|256)rrk(z?)$",
                                             "^VPUNPCK(HWD|LBW)Z(128|256)rrk(z?)$")>;

def SPRWriteResGroup9 : SchedWriteRes<[SPRPort02_03, SPRPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup9], (instregex "^(ADD|SUB)_F(32|64)m$",
                                             "^ILD_F(16|32|64)m$",
                                             "^SUBR_F(32|64)m$")>;

def SPRWriteResGroup10 : SchedWriteRes<[SPRPort02_03, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup10], (instregex "^(ADD|SUB)_FI(16|32)m$",
                                              "^SUBR_FI(16|32)m$")>;

def SPRWriteResGroup11 : SchedWriteRes<[SPRPort05]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup11], (instregex "^(ADD|SUB)_F(P?)rST0$",
                                              "^(ADD|SUB)_FST0r$",
                                              "^KMOV(B|D|W)kr$",
                                              "^(V?)PACK(S|U)S(DW|WB)rr$",
                                              "^(V?)PCMPGTQrr$",
                                              "^SUBR_F(P?)rST0$",
                                              "^SUBR_FST0r$",
                                              "^VALIGN(D|Q)Z256rri((k|kz)?)$",
                                              "^V(CMP|PCM)PDZ128rri(k?)$",
                                              "^VCMPP(H|S)Z128rri(k?)$",
                                              "^VCMPS(D|H|S)Zrr(b?)_Int(k?)$",
                                              "^VFPCLASSP(D|H|S)Z128rr(k?)$",
                                              "^VFPCLASSS(D|H|S)Zrr(k?)$",
                                              "^VPACK(S|U)S(DW|WB)Yrr$",
                                              "^VPACK(S|U)S(DW|WB)Z(128|256)rr$",
                                              "^VPALIGNRZ(128|256)rrik(z?)$",
                                              "^VPBROADCAST(B|W)Z128rrk(z?)$",
                                              "^VPCMPBZ(128|256)rri(k?)$",
                                              "^VPCMP(D|Q|W|UB|UQ|UW)Z256rri(k?)$",
                                              "^VPCMP(EQB|GTD|GTQ|GTW)Z(128|256)rr(k?)$",
                                              "^VPCMP(EQD|GTB)Z(128|256)rr(k?)$",
                                              "^VPCMPEQ(Q|W)Z(128|256)rr(k?)$",
                                              "^VPCMPGTQYrr$",
                                              "^VPCMP(Q|W|UB|UD|UW)Z128rri(k?)$",
                                              "^VPCMPUDZ256rri(k?)$",
                                              "^VPCMPUQZ128rri(k?)$",
                                              "^VPERMBZ(128|256)rr$",
                                              "^VPERM(BZ|DY|I2D|I2Q|T2D|T2Q)rr$",
                                              "^VPERM(DZ|I2D|I2Q|T2D|T2Q)256rr((k|kz)?)$",
                                              "^VPERM(D|Q)Zrr$",
                                              "^VPERM(DZ|I2D|I2Q|T2D|T2Q)rrk(z?)$",
                                              "^VPERM(I2D|T2Q)128rr((k|kz)?)$",
                                              "^VPERM(I2Q|T2D)128rr((k|kz)?)$",
                                              "^VPERMQZ256rr((k|kz)?)$",
                                              "^VPERMQZrrk(z?)$",
                                              "^VPM(AXS|INU)QZ(128|256)rr((k|kz)?)$",
                                              "^VPM(AXU|INS)QZ(128|256)rr((k|kz)?)$",
                                              "^VPMULTISHIFTQBZ(128|256)rr$",
                                              "^VPOPCNT(B|D|Q|W)Z(128|256)rr$",
                                              "^VPOPCNT(D|Q)Z(128|256)rrk(z?)$",
                                              "^VPTEST(MB|NMD|NMQ|NMW)Z(128|256)rr(k?)$",
                                              "^VPTEST(MB|NMD|NMQ|NMW)Zrr(k?)$",
                                              "^VPTEST(MD|NMB)Z(128|256)rr(k?)$",
                                              "^VPTEST(MD|NMB)Zrr(k?)$",
                                              "^VPTESTM(Q|W)Z(128|256)rr(k?)$",
                                              "^VPTESTM(Q|W)Zrr(k?)$")>;

def SPRWriteResGroup12 : SchedWriteRes<[SPRPort00_01_05_06_10]> {
  let Latency = 2;
}
def : InstRW<[SPRWriteResGroup12], (instregex "^(OR|AND|XOR)(16|32|64)ri8$",
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

def SPRWriteResGroup13 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup13], (instregex "^TEST(8|16|32)mi$",
                                              "^TEST64mi32$")>;
def : InstRW<[SPRWriteResGroup13, ReadAfterLd], (instregex "^(OR|AND|XOR)64rm$")>;
def : InstRW<[SPRWriteResGroup13, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^TEST(8|16|32|64)mr$")>;

def SPRWriteResGroup14 : SchedWriteRes<[SPRPort01_05_10, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup14, ReadAfterLd], (instregex "^ANDN(32|64)rm$")>;

def SPRWriteResGroup15 : SchedWriteRes<[SPRPort01_05_10]> {
  let Latency = 2;
}
def : InstRW<[SPRWriteResGroup15], (instregex "^ANDN(32|64)rr$")>;

def SPRWriteResGroup16 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [5, 2, 1, 1];
  let Latency = 10;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup16], (instregex "^BT64mr$")>;

def SPRWriteResGroup17 : SchedWriteRes<[SPRPort01]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup17], (instregex "^(B|PEX)T64rr$",
                                              "^BT(C|R|S)64rr$",
                                              "^PDEP(32|64)rr$",
                                              "^PEXT32rr$")>;

def SPRWriteResGroup18 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [4, 2, 1, 1, 1, 1];
  let Latency = 17;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup18], (instregex "^BT(C|R|S)64mr$")>;

def SPRWriteResGroup19 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup19], (instregex "^CALL(16|32|64)m$")>;

def SPRWriteResGroup20 : SchedWriteRes<[SPRPort00_06, SPRPort04_09, SPRPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup20], (instregex "^CALL(16|32|64)r$")>;

def SPRWriteResGroup21 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup21], (instregex "^CALL64pcrel32$",
                                              "^MFENCE$")>;

def SPRWriteResGroup22 : SchedWriteRes<[SPRPort01_05]>;
def : InstRW<[SPRWriteResGroup22], (instregex "^C(BW|DQE|WDE)$",
                                              "^(V?)MOVS(H|L)DUPrr$",
                                              "^(V?)SHUFP(D|S)rri$",
                                              "^VMOVS(H|L)DUPYrr$",
                                              "^VMOVS(H|L)DUPZ(128|256)rr((k|kz)?)$",
                                              "^VPBLENDWYrri$",
                                              "^VPMOVQDZ128rr((k|kz)?)$",
                                              "^VSHUFP(D|S)Yrri$",
                                              "^VSHUFP(D|S)Z(128|256)rri((k|kz)?)$")>;

def SPRWriteResGroup23 : SchedWriteRes<[SPRPort00_06]>;
def : InstRW<[SPRWriteResGroup23], (instregex "^C(DQ|QO|LAC)$",
                                              "^STAC$")>;

def SPRWriteResGroup24 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup24], (instregex "^CLD$")>;

def SPRWriteResGroup25 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup25], (instregex "^CLDEMOTE$")>;

def SPRWriteResGroup26 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort04_09, SPRPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup26], (instregex "^CLFLUSH$")>;

def SPRWriteResGroup27 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup27], (instregex "^CLFLUSHOPT$",
                                              "^CLWB$")>;

def SPRWriteResGroup28 : SchedWriteRes<[SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [2, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup28], (instregex "^CLI$")>;

def SPRWriteResGroup29 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort05]> {
  let ResourceCycles = [6, 1, 3];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup29], (instregex "^CLTS$")>;

def SPRWriteResGroup30 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let ResourceCycles = [5, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup30], (instregex "^CMPS(B|L|Q|W)$")>;

def SPRWriteResGroup31 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [2, 7, 6, 2, 1, 1, 2, 1];
  let Latency = 32;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup31], (instregex "^CMPXCHG16B$")>;

def SPRWriteResGroup32 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [4, 7, 2, 1, 1, 1];
  let Latency = 25;
  let NumMicroOps = 16;
}
def : InstRW<[SPRWriteResGroup32], (instregex "^CMPXCHG8B$")>;

def SPRWriteResGroup33 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 13;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup33], (instregex "^CMPXCHG8rm$")>;

def SPRWriteResGroup34 : SchedWriteRes<[SPRPort05]>;
def : InstRW<[SPRWriteResGroup34], (instregex "^COM(P?)_FST0r$",
                                              "^KMOVQkr$",
                                              "^(V?)PALIGNRrri$",
                                              "^UCOM_F(P?)r$",
                                              "^VALIGN(D|Q)Z128rri((k|kz)?)$",
                                              "^VBROADCASTSSZ128rr((k|kz)?)$",
                                              "^VPALIGNR(Y|Z)rri$",
                                              "^VPALIGNRZ(128|256)rri$",
                                              "^VPBROADCAST(B|D|Q|W)rr$",
                                              "^VPSHUFBZrr$",
                                              "^VPSHUF(D|HW|LW)Zri$",
                                              "^VPSHUFDZrik(z?)$",
                                              "^VPS(L|R)LDQZri$",
                                              "^VPUNPCK(HBW|LDQ|LWD)Zrr$",
                                              "^VPUNPCK(H|LQ)DQZrr$",
                                              "^VPUNPCK(H|L|LQ)DQZrrk(z?)$",
                                              "^VPUNPCKH(WD|QDQ)Zrr$",
                                              "^VPUNPCKHQDQZrrk(z?)$",
                                              "^VPUNPCKLBWZrr$")>;

def SPRWriteResGroup35 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_06, SPRPort01, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 10, 6, 1, 5, 1];
  let Latency = 18;
  let NumMicroOps = 26;
}
def : InstRW<[SPRWriteResGroup35], (instregex "^CPUID$")>;

def SPRWriteResGroup36 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup36], (instregex "^(V?)CVT(T?)PD2DQrm$",
                                              "^VCVT(T?)PD2(U?)DQZ128rm((b|k|bk|kz|bkz)?)$",
                                              "^VCVTPH2PSXZ128rm(b?)$",
                                              "^VCVT(U?)QQ2PSZ128rm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup36, ReadAfterVecLd], (instregex "^(V?)CVTSI642SSrm_Int$",
                                                              "^VCVT(U?)SI642SSZrm_Int$")>;

def SPRWriteResGroup37 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 26;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup37], (instregex "^(V?)CVT(T?)SD2SIrm_Int$")>;
def : InstRW<[SPRWriteResGroup37, ReadAfterVecLd], (instregex "^VCVT(T?)SD2(U?)SIZrm_Int$")>;

def SPRWriteResGroup38 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup38], (instregex "^VCVT(T?)PS2(U?)QQZ256rr((k|kz)?)$",
                                              "^VCVT(U?)QQ2PSZ256rr((k|kz)?)$")>;
def : InstRW<[SPRWriteResGroup38, ReadDefault, ReadInt2Fpu], (instregex "^(V?)CVTSI2SSrr_Int$",
                                                                        "^VCVT(U?)SI2SSZrr(b?)_Int$")>;

def SPRWriteResGroup39 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup39, ReadDefault, ReadInt2Fpu], (instregex "^(V?)CVTSI642SSrr_Int$",
                                                                        "^VCVT(U?)SI642SSZrr(b?)_Int$")>;

def SPRWriteResGroup40 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup40], (instregex "^(V?)CVT(T?)SS2SI64rr_Int$",
                                              "^VCVT(T?)SS2(U?)SI64Zrr(b?)_Int$")>;

def SPRWriteResGroup41 : SchedWriteRes<[SPRPort00, SPRPort00_01]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup41], (instregex "^(V?)CVT(T?)SS2SIrr_Int$",
                                              "^MMX_CVTPI2PSrr$",
                                              "^VCVT(T?)SS2(U?)SIZrr(b?)_Int$",
                                              "^VCVTTSD2SI((64)?)rr_Int$")>;

def SPRWriteResGroup42 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup42], (instregex "^CWD$",
                                              "^J(E|R)CXZ$")>;

def SPRWriteResGroup43 : SchedWriteRes<[SPRPort00_01_05_06]>;
def : InstRW<[SPRWriteResGroup43], (instregex "^DEC16r_alt$",
                                              "^(LD|ST)_Frr$",
                                              "^MOV16s(m|r)$",
                                              "^MOV(32|64)sr$",
                                              "^SALC$",
                                              "^ST_FPrr$",
                                              "^SYSCALL$")>;

def SPRWriteResGroup44 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]>;
def : InstRW<[SPRWriteResGroup44], (instregex "^DEC32r_alt$")>;

def SPRWriteResGroup45 : SchedWriteRes<[SPRPort00, SPRPort02_03]> {
  let Latency = 27;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup45], (instregex "^DIVR_F(32|64)m$")>;

def SPRWriteResGroup46 : SchedWriteRes<[SPRPort00, SPRPort02_03, SPRPort05]> {
  let Latency = 30;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup46], (instregex "^DIVR_FI(16|32)m$")>;

def SPRWriteResGroup47 : SchedWriteRes<[SPRPort00]> {
  let Latency = 15;
}
def : InstRW<[SPRWriteResGroup47], (instregex "^DIVR_F(P?)rST0$",
                                              "^DIVR_FST0r$")>;

def SPRWriteResGroup48 : SchedWriteRes<[SPRPort00, SPRPort02_03]> {
  let Latency = 22;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup48], (instregex "^DIV_F(32|64)m$")>;

def SPRWriteResGroup49 : SchedWriteRes<[SPRPort00, SPRPort02_03, SPRPort05]> {
  let Latency = 25;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup49], (instregex "^DIV_FI(16|32)m$")>;

def SPRWriteResGroup50 : SchedWriteRes<[SPRPort00]> {
  let Latency = 20;
}
def : InstRW<[SPRWriteResGroup50], (instregex "^DIV_F(P?)rST0$",
                                              "^DIV_FST0r$")>;

def SPRWriteResGroup51 : SchedWriteRes<[SPRPort02_03_07, SPRPort04]>;
def : InstRW<[SPRWriteResGroup51], (instregex "^ENQCMD(S?)(16|32|64)$",
                                              "^PUSHA(16|32)$",
                                              "^PUSHF32$",
                                              "^ST_F(32|64)m$")>;

def SPRWriteResGroup52 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [2, 21, 2, 14, 4, 9, 5];
  let Latency = 126;
  let NumMicroOps = 57;
}
def : InstRW<[SPRWriteResGroup52], (instregex "^ENTER$")>;

def SPRWriteResGroup53 : SchedWriteRes<[SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup53], (instregex "^(V?)EXTRACTPSmr$",
                                              "^SMSW16m$",
                                              "^VEXTRACTPSZmr$",
                                              "^VPMOVQDZ((256)?)mr$")>;

def SPRWriteResGroup54 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup54], (instregex "^(V?)EXTRACTPSrr$",
                                              "^MMX_PEXTRWrr$",
                                              "^VEXTRACTPSZrr$",
                                              "^VPERMWZrr$")>;

def SPRWriteResGroup55 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort02_03, SPRPort02_03_07, SPRPort04, SPRPort06]> {
  let Latency = 7;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup55], (instregex "^FARCALL64m$")>;

def SPRWriteResGroup56 : SchedWriteRes<[SPRPort02_03, SPRPort06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup56], (instregex "^FARJMP64m$")>;

def SPRWriteResGroup57 : SchedWriteRes<[SPRPort02_03_07, SPRPort04]> {
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup57], (instregex "^FBSTPm$",
                                              "^ST_FP(32|64|80)m$",
                                              "^VMPTRSTm$")>;

def SPRWriteResGroup58 : SchedWriteRes<[SPRPort00_05]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup58], (instregex "^FDECSTP$")>;

def SPRWriteResGroup59 : SchedWriteRes<[SPRPort02_03, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup59], (instregex "^FICOM(P?)(16|32)m$")>;

def SPRWriteResGroup60 : SchedWriteRes<[SPRPort00_05]>;
def : InstRW<[SPRWriteResGroup60], (instregex "^FINCSTP$",
                                              "^FNOP$",
                                              "^MMX_P(ADD|SUB)(B|D|Q|W)rr$",
                                              "^VP(ADD|SUB)(B|D|Q|W)Zrr$",
                                              "^VP(ADD|SUB)(D|Q)Zrrk(z?)$",
                                              "^VPTERNLOG(D|Q)Zrri((k|kz)?)$")>;

def SPRWriteResGroup61 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup61], (instregex "^FLDCW16m$")>;

def SPRWriteResGroup62 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06, SPRPort00_05, SPRPort00_06, SPRPort02_03]> {
  let ResourceCycles = [2, 39, 5, 10, 8];
  let Latency = 62;
  let NumMicroOps = 64;
}
def : InstRW<[SPRWriteResGroup62], (instregex "^FLDENVm$")>;

def SPRWriteResGroup63 : SchedWriteRes<[SPRPort00_01_05_06]> {
  let ResourceCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup63], (instregex "^FNCLEX$")>;

def SPRWriteResGroup64 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [6, 3, 6];
  let Latency = 75;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup64], (instregex "^FNINIT$")>;

def SPRWriteResGroup65 : SchedWriteRes<[SPRPort02_03_07, SPRPort04, SPRPort06]> {
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup65], (instregex "^FNSTCW16m$")>;

def SPRWriteResGroup66 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup66], (instregex "^FNSTSW16r$")>;

def SPRWriteResGroup67 : SchedWriteRes<[SPRPort00, SPRPort02_03_07, SPRPort04]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup67], (instregex "^FNSTSWm$")>;

def SPRWriteResGroup68 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06, SPRPort00_06, SPRPort01, SPRPort02_03_07, SPRPort04, SPRPort05, SPRPort06]> {
  let ResourceCycles = [9, 30, 21, 1, 11, 11, 16, 1];
  let Latency = 106;
  let NumMicroOps = 100;
}
def : InstRW<[SPRWriteResGroup68], (instregex "^FSTENVm$")>;

def SPRWriteResGroup69 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06, SPRPort00_05, SPRPort00_06, SPRPort01_05, SPRPort02_03, SPRPort06]> {
  let ResourceCycles = [4, 47, 1, 2, 1, 33, 2];
  let Latency = 63;
  let NumMicroOps = 90;
}
def : InstRW<[SPRWriteResGroup69], (instregex "^FXRSTOR$")>;

def SPRWriteResGroup70 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06, SPRPort00_05, SPRPort00_06, SPRPort01_05, SPRPort02_03, SPRPort06]> {
  let ResourceCycles = [4, 45, 1, 2, 1, 31, 4];
  let Latency = 63;
  let NumMicroOps = 88;
}
def : InstRW<[SPRWriteResGroup70], (instregex "^FXRSTOR64$")>;

def SPRWriteResGroup71 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [2, 5, 10, 10, 2, 38, 5, 38];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 110;
}
def : InstRW<[SPRWriteResGroup71], (instregex "^FXSAVE((64)?)$")>;

def SPRWriteResGroup72 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup72], (instregex "^VPLZCNT(D|Q)Z256rm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup72, ReadAfterVecXLd], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrmi$",
                                                               "^(V?)GF2P8MULBrm$",
                                                               "^V(ADD|MAX|MIN|MUL|SUB)PHZ128rm((b|k|bk|kz|bkz)?)$",
                                                               "^VF(N?)MADD(132|213|231)PHZ128m((b|k|bk|kz|bkz)?)$",
                                                               "^VFM((ADD)?)SUB(132|213|231)PHZ128m((b|k|bk|kz|bkz)?)$",
                                                               "^VFMSUBADD(132|213|231)PHZ128m((b|k|bk|kz|bkz)?)$",
                                                               "^VFNMSUB(132|213|231)PHZ128m((b|k|bk|kz|bkz)?)$",
                                                               "^VGETEXPPHZ128m((b|k|bk|kz|bkz)?)$",
                                                               "^VGETEXPSHZm((k|kz)?)$",
                                                               "^VGETMANTPHZ128rm(bi|ik|ikz)$",
                                                               "^VGETMANTPHZ128rm(i|bik)$",
                                                               "^VGETMANTPHZ128rmbikz$",
                                                               "^VGETMANTSHZrmi((k|kz)?)$",
                                                               "^VGF2P8AFFINE((INV)?)QBYrmi$",
                                                               "^VGF2P8AFFINE((INV)?)QBZ128rm(b?)i$",
                                                               "^VGF2P8MULBYrm$",
                                                               "^VGF2P8MULBZ128rm$")>;
def : InstRW<[SPRWriteResGroup72, ReadAfterVecLd], (instregex "^V(ADD|MAX|MIN|MUL|SUB)SHZrm_Int((k|kz)?)$",
                                                              "^VCVTSH2SSZrm_Int$",
                                                              "^VF(N?)M(ADD|SUB)(132|213|231)SHZm_Int((k|kz)?)$")>;
def : InstRW<[SPRWriteResGroup72, ReadAfterVecYLd], (instregex "^VGF2P8AFFINE((INV)?)QBZ256rm(b?)i$",
                                                               "^VGF2P8MULBZ256rm$",
                                                               "^VPMADD52(H|L)UQZ256m((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup73 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 5;
}
def : InstRW<[SPRWriteResGroup73], (instregex "^(V?)GF2P8AFFINE((INV)?)QBrri$",
                                              "^(V?)GF2P8MULBrr$",
                                              "^V(ADD|MAX|MIN|MUL|SUB)PHZ(128|256)rr$",
                                              "^V(ADD|MAX|MIN|MUL|SUB)SHZrr(b?)_Int$",
                                              "^VCVT(T?)PH2(U?)WZ(128|256)rr$",
                                              "^VCVTSH2SSZrr(b?)_Int$",
                                              "^VCVT(U?)W2PHZ(128|256)rr$",
                                              "^VF(N?)MADD(132|213|231)PHZ(128|256)r$",
                                              "^VF(N?)M(ADD|SUB)(132|213|231)SHZr(b?)_Int$",
                                              "^VFM((ADD)?)SUB(132|213|231)PHZ(128|256)r$",
                                              "^VFMSUBADD(132|213|231)PHZ(128|256)r$",
                                              "^VFNMSUB(132|213|231)PHZ(128|256)r$",
                                              "^VGETEXPPHZ(128|256)r$",
                                              "^VGETEXPSHZr(b?)$",
                                              "^VGETMANTPHZ(128|256)rri$",
                                              "^VGETMANTSHZrri(b?)$",
                                              "^VGF2P8AFFINE((INV)?)QBYrri$",
                                              "^VGF2P8MULBYrr$",
                                              "^VGF2P8MULBZ(128|256)rr$")>;

def SPRWriteResGroup74 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 5, 26, 19, 2, 7, 21];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[SPRWriteResGroup74], (instregex "^IN16ri$")>;

def SPRWriteResGroup75 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 1, 4, 26, 19, 3, 7, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[SPRWriteResGroup75], (instregex "^IN16rr$")>;

def SPRWriteResGroup76 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 6, 28, 21, 2, 10, 20];
  let Latency = 35;
  let NumMicroOps = 94;
}
def : InstRW<[SPRWriteResGroup76], (instregex "^IN32ri$")>;

def SPRWriteResGroup77 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 9, 28, 21, 2, 11, 21];
  let NumMicroOps = 99;
}
def : InstRW<[SPRWriteResGroup77], (instregex "^IN32rr$")>;

def SPRWriteResGroup78 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 8, 20];
  let Latency = 35;
  let NumMicroOps = 87;
}
def : InstRW<[SPRWriteResGroup78], (instregex "^IN8ri$")>;

def SPRWriteResGroup79 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 6, 25, 19, 2, 7, 20];
  let Latency = 35;
  let NumMicroOps = 86;
}
def : InstRW<[SPRWriteResGroup79], (instregex "^IN8rr$")>;

def SPRWriteResGroup80 : SchedWriteRes<[SPRPort00_06]> {
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup80], (instregex "^INC16r_alt$")>;

def SPRWriteResGroup81 : SchedWriteRes<[SPRPort02_03_11]>;
def : InstRW<[SPRWriteResGroup81], (instregex "^INC32r_alt$",
                                              "^MOV32ao(16|32|64)$",
                                              "^MOV64ao64$")>;

def SPRWriteResGroup82 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [7, 6, 24, 17, 8, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 83;
}
def : InstRW<[SPRWriteResGroup82], (instregex "^INSB$")>;

def SPRWriteResGroup83 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [7, 1, 5, 1, 27, 17, 11, 1, 21, 1];
  let Latency = 20;
  let NumMicroOps = 92;
}
def : InstRW<[SPRWriteResGroup83], (instregex "^INSL$")>;

def SPRWriteResGroup84 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [7, 1, 4, 1, 25, 17, 1, 9, 1, 19, 1];
  let Latency = 20;
  let NumMicroOps = 86;
}
def : InstRW<[SPRWriteResGroup84], (instregex "^INSW$")>;

def SPRWriteResGroup85 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [5, 4, 8, 6, 2, 5, 7, 5];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 42;
}
def : InstRW<[SPRWriteResGroup85], (instregex "^INVLPG$")>;

def SPRWriteResGroup86 : SchedWriteRes<[SPRPort02_03_07, SPRPort04, SPRPort05]> {
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup86], (instregex "^IST(T?)_FP(16|32|64)m$",
                                              "^IST_F(16|32)m$")>;

def SPRWriteResGroup87 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort00_06]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup87], (instregex "^JCXZ$")>;

def SPRWriteResGroup88 : SchedWriteRes<[SPRPort05]> {
  let Latency = 4;
}
def : InstRW<[SPRWriteResGroup88], (instregex "^KADD(B|D|Q|W)rr$",
                                              "^KSHIFT(LB|RD|RQ|RW)ri$",
                                              "^KSHIFT(LD|RB)ri$",
                                              "^KSHIFTL(Q|W)ri$",
                                              "^KUNPCK(BW|DQ|WD)rr$")>;

def SPRWriteResGroup89 : SchedWriteRes<[SPRPort00]>;
def : InstRW<[SPRWriteResGroup89], (instregex "^K(OR|AND|NOT|XOR)(B|D)rr$",
                                              "^KAND(Q|W|NB)rr$",
                                              "^KANDN(D|Q|W)rr$",
                                              "^KMOV(B|D|Q|W)kk$",
                                              "^K(OR|NOT|XOR)(Q|W)rr$",
                                              "^KXNOR(B|D|Q|W)rr$",
                                              "^VP(A|SU)BS(B|W)Zrr$",
                                              "^VP(AB|MAX|MIN)SDZrr((k|kz)?)$",
                                              "^VPA(BSQ|VGB|VGW)Zrr$",
                                              "^VPABSQZrrk(z?)$",
                                              "^VP(ADD|MAX|MIN)S(B|W)Zrr$",
                                              "^VP(ADD|SUB)US(B|W)Zrr$",
                                              "^VPM(AX|IN)U(B|D|W)Zrr$",
                                              "^VPM(AX|IN)UDZrrk(z?)$",
                                              "^VPSH(LDD|RDQ|RDW)Zrri$",
                                              "^VPSH(LDQ|RDD)Zrri$",
                                              "^VPSH(L|R)DV(D|Q|W)Zr$",
                                              "^VPSH(L|R)DV(D|Q)Zrk(z?)$",
                                              "^VPSHLDWZrri$")>;

def SPRWriteResGroup90 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup90], (instregex "^KMOV(B|D|Q|W)km$")>;

def SPRWriteResGroup91 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup91], (instregex "^KMOVBmk$",
                                              "^MOV8m(i|r)$")>;

def SPRWriteResGroup92 : SchedWriteRes<[SPRPort00]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup92], (instregex "^K((OR)?)TEST(B|D|Q|W)rr$",
                                              "^VP(A|SU)BS(B|W)Zrrk(z?)$",
                                              "^VP(ADD|MAX|MIN)S(B|W)Zrrk(z?)$",
                                              "^VP(ADD|SUB)US(B|W)Zrrk(z?)$",
                                              "^VPAVG(B|W)Zrrk(z?)$",
                                              "^VPM(AX|IN)U(B|W)Zrrk(z?)$",
                                              "^VPSH(L|R)DVWZrk(z?)$",
                                              "^VPS(L|R)LVWZrrk(z?)$",
                                              "^VPS(L|R)LWZrik(z?)$",
                                              "^VPSRAVWZrrk(z?)$",
                                              "^VPSRAWZrik(z?)$")>;

def SPRWriteResGroup93 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [8, 2, 14, 3, 1];
  let Latency = 198;
  let NumMicroOps = 81;
}
def : InstRW<[SPRWriteResGroup93], (instregex "^LAR16rm$")>;

def SPRWriteResGroup94 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 66;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup94], (instregex "^LAR16rr$")>;

def SPRWriteResGroup95 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 85;
}
def : InstRW<[SPRWriteResGroup95], (instregex "^LAR32rm$")>;

def SPRWriteResGroup96 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 3, 1, 8, 5, 1, 2, 1];
  let Latency = 65;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup96], (instregex "^LAR(32|64)rr$")>;

def SPRWriteResGroup97 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2, 2, 9, 5, 3, 1];
  let Latency = 71;
  let NumMicroOps = 87;
}
def : InstRW<[SPRWriteResGroup97], (instregex "^LAR64rm$")>;

def SPRWriteResGroup98 : SchedWriteRes<[SPRPort02_03]> {
  let Latency = 7;
}
def : InstRW<[SPRWriteResGroup98], (instregex "^LD_F(32|64|80)m$")>;

def SPRWriteResGroup99 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup99], (instregex "^LEA16r$")>;

def SPRWriteResGroup100 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup100], (instregex "^LEAVE$",
                                               "^(LOD|SCA)S(B|W)$",
                                               "^SCAS(L|Q)$")>;

def SPRWriteResGroup101 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup101], (instregex "^LEAVE64$")>;

def SPRWriteResGroup102 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 4, 3, 2, 1, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[SPRWriteResGroup102], (instregex "^LGDT64m$")>;

def SPRWriteResGroup103 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 5, 3, 2, 1, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 14;
}
def : InstRW<[SPRWriteResGroup103], (instregex "^LIDT64m$")>;

def SPRWriteResGroup104 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [5, 3, 2, 1, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 12;
}
def : InstRW<[SPRWriteResGroup104], (instregex "^LLDT16m$")>;

def SPRWriteResGroup105 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 4, 3, 1, 1, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup105], (instregex "^LLDT16r$")>;

def SPRWriteResGroup106 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 2, 8, 3, 1, 2, 7, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 27;
}
def : InstRW<[SPRWriteResGroup106], (instregex "^LMSW16m$")>;

def SPRWriteResGroup107 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [5, 7, 1, 2, 5, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup107], (instregex "^LMSW16r$")>;

def SPRWriteResGroup108 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup108], (instregex "^LODS(L|Q)$")>;

def SPRWriteResGroup109 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [2, 4, 1];
  let Latency = 3;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup109], (instregex "^LOOP$")>;

def SPRWriteResGroup110 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 3;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup110], (instregex "^LOOPE$")>;

def SPRWriteResGroup111 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [4, 6, 1];
  let Latency = 2;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup111], (instregex "^LOOPNE$")>;

def SPRWriteResGroup112 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort02_03, SPRPort06]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup112], (instregex "^LRET64$")>;

def SPRWriteResGroup113 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 5, 3, 3, 1];
  let Latency = 70;
  let NumMicroOps = 13;
}
def : InstRW<[SPRWriteResGroup113], (instregex "^LSL(16|32|64)rm$")>;

def SPRWriteResGroup114 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 4, 4, 3, 2, 1];
  let Latency = 63;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup114], (instregex "^LSL(16|32|64)rr$")>;

def SPRWriteResGroup115 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 2, 2, 1, 2];
  let Latency = 10;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup115], (instregex "^(V?)MASKMOVDQU((64)?)$")>;

def SPRWriteResGroup116 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 24;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup116], (instregex "^MMX_CVT(T?)PD2PIrm$")>;

def SPRWriteResGroup117 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup117], (instregex "^MMX_CVT(T?)PD2PIrr$",
                                               "^VCVT(T?)PH2DQZ(128|256)rr$",
                                               "^VCVTPH2(PSX|UDQ)Z256rr$",
                                               "^VCVT(T?)PH2UDQZ128rr$",
                                               "^VCVTPS2PHXZ256rr$",
                                               "^VCVTTPH2UDQZ256rr$")>;

def SPRWriteResGroup118 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup118], (instregex "^MMX_CVTPI2PDrr$",
                                               "^VCVTPH2PSXZ128rr$",
                                               "^VCVTPS2PHXZ128rr$",
                                               "^VPERMWZ(128|256)rrk(z?)$",
                                               "^VPS(L|R)LWZ256rrk(z?)$",
                                               "^VPSRAWZ256rrk(z?)$")>;

def SPRWriteResGroup119 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup119], (instregex "^MMX_CVT(T?)PS2PIrm$")>;

def SPRWriteResGroup120 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup120], (instregex "^MMX_CVT(T?)PS2PIrr$")>;

def SPRWriteResGroup121 : SchedWriteRes<[SPRPort00, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup121], (instregex "^MMX_MASKMOVQ((64)?)$")>;

def SPRWriteResGroup122 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 18;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup122], (instregex "^MMX_MOVD64mr$",
                                               "^VMOV(W|SHZ)mr$")>;

def SPRWriteResGroup123 : SchedWriteRes<[SPRPort02_03_11]> {
  let Latency = 8;
}
def : InstRW<[SPRWriteResGroup123], (instregex "^MMX_MOV(D|Q)64rm$",
                                               "^VBROADCAST(F|I)128$",
                                               "^VBROADCAST(F|I)32X(2|4)Z256rm$",
                                               "^VBROADCAST(F|I)32X(8|2Z)rm$",
                                               "^VBROADCAST(F|I)(32|64)X4rm$",
                                               "^VBROADCAST(F|I)64X2Z128rm$",
                                               "^VBROADCAST(F|I)64X2rm$",
                                               "^VBROADCASTS(DY|SZ)rm$",
                                               "^VBROADCASTS(D|S)Z256rm$",
                                               "^VBROADCASTS(DZ|SY)rm$",
                                               "^VMOV(D|SH|SL)DUP(Y|Z)rm$",
                                               "^VMOV(D|SH|SL)DUPZ256rm$",
                                               "^VPBROADCAST(DY|QZ)rm$",
                                               "^VPBROADCAST(D|Q)Z256rm$",
                                               "^VPBROADCAST(DZ|QY)rm$")>;

def SPRWriteResGroup124 : SchedWriteRes<[SPRPort00_01_05, SPRPort00_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup124], (instregex "^MMX_MOVDQ2Qrr$")>;

def SPRWriteResGroup125 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup125], (instregex "^MMX_MOVQ2DQrr$")>;

def SPRWriteResGroup126 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup126, ReadAfterVecLd], (instregex "^MMX_PACKSS(DW|WB)rm$",
                                                               "^MMX_PACKUSWBrm$")>;

def SPRWriteResGroup127 : SchedWriteRes<[SPRPort05]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup127], (instregex "^MMX_PACKSS(DW|WB)rr$",
                                               "^MMX_PACKUSWBrr$",
                                               "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZrr$",
                                               "^VPMOV(D|Q|SQ|USQ)WZrr$",
                                               "^VPMOV(U?)S(DB|QD)Zrr$",
                                               "^VPMOV(U?)SDWZrr$",
                                               "^VPMOV(U?)SQDZrrk(z?)$")>;
def : InstRW<[SPRWriteResGroup127, ReadDefault, ReadInt2Fpu], (instregex "^MMX_PINSRWrr$")>;

def SPRWriteResGroup128 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup128], (instregex "^VBROADCAST(F|I)32X(8|2Z)rmk(z?)$",
                                               "^VBROADCAST(F|I)(32|64)X4rmk(z?)$",
                                               "^VBROADCAST(F|I)64X2rmk(z?)$",
                                               "^VBROADCASTS(D|S)Zrmk(z?)$",
                                               "^VMOV(APD|UPS)Zrmk(z?)$",
                                               "^VMOV(APS|UPD)Zrmk(z?)$",
                                               "^VMOV(D|SH|SL)DUPZrmk(z?)$",
                                               "^VMOVDQ(A|U)(32|64)Zrmk(z?)$",
                                               "^VPBROADCAST(D|Q)Zrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup128, ReadAfterVecLd], (instregex "^MMX_P(ADD|SUB)(B|D|Q|W)rm$")>;
def : InstRW<[SPRWriteResGroup128, ReadAfterVecYLd], (instregex "^VINSERT(F|I)(32|64)x4Zrm((k|kz)?)$",
                                                                "^VINSERT(F|I)32x8Zrm((k|kz)?)$",
                                                                "^VINSERT(F|I)64x2Zrm((k|kz)?)$",
                                                                "^VP(ADD|SUB)(B|D|Q|W)Zrm$",
                                                                "^VP(ADD|SUB)(D|Q)Zrm(b|k|kz)$",
                                                                "^VP(ADD|SUB)(D|Q)Zrmbk(z?)$",
                                                                "^VPTERNLOG(D|Q)Zrm(bi|ik|ikz)$",
                                                                "^VPTERNLOG(D|Q)Zrm(i|bik)$",
                                                                "^VPTERNLOG(D|Q)Zrmbikz$")>;

def SPRWriteResGroup129 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup129, ReadAfterVecLd], (instregex "^MMX_PH(ADD|SUB)SWrm$")>;

def SPRWriteResGroup130 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup130], (instregex "^MMX_PH(ADD|SUB)SWrr$")>;

def SPRWriteResGroup131 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup131], (instregex "^VPBROADCAST(BY|WZ)rm$",
                                               "^VPBROADCAST(B|W)Z256rm$",
                                               "^VPBROADCAST(BZ|WY)rm$")>;
def : InstRW<[SPRWriteResGroup131, ReadAfterLd], (instregex "^MMX_PINSRWrm$")>;
def : InstRW<[SPRWriteResGroup131, ReadAfterVecXLd], (instregex "^VFPCLASSP(D|S)Z128rm$")>;
def : InstRW<[SPRWriteResGroup131, ReadAfterVecLd], (instregex "^VFPCLASSS(D|H|S)Zrm$")>;
def : InstRW<[SPRWriteResGroup131, ReadAfterVecYLd], (instregex "^VPALIGNRYrmi$",
                                                                "^VPALIGNRZ256rmi$",
                                                                "^VPSHUFBZrm$")>;

def SPRWriteResGroup132 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup132], (instregex "^MOV16ao(16|32|64)$")>;

def SPRWriteResGroup133 : SchedWriteRes<[SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup133], (instregex "^MOV16ms$",
                                               "^MOVBE32mr$",
                                               "^PUSH(F|G)S(16|32)$")>;

def SPRWriteResGroup134 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup134], (instregex "^MOV16o(16|32|64)a$")>;

def SPRWriteResGroup135 : SchedWriteRes<[SPRPort00_01_05_06_10]>;
def : InstRW<[SPRWriteResGroup135], (instregex "^MOV(8|16|32|64)ri$",
                                               "^MOV(8|16)rr$",
                                               "^MOV(8|16)rr_REV$",
                                               "^MOV64ri32$")>;

def SPRWriteResGroup136 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01]> {
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup136], (instregex "^MOV(16|32|64)rs$",
                                               "^S(TR|LDT)16r$")>;

def SPRWriteResGroup137 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup137], (instregex "^MOV(8|32)o(16|32)a$",
                                               "^MOV(8|32|64)o64a$")>;

def SPRWriteResGroup138 : SchedWriteRes<[SPRPort00_01_05_06_10]> {
  let Latency = 0;
}
def : InstRW<[SPRWriteResGroup138], (instregex "^MOV32rr$",
                                               "^MOV32rr_REV$",
                                               "^MOVZX(32|64)rr8$")>;

def SPRWriteResGroup139 : SchedWriteRes<[SPRPort02_03_11]> {
  let Latency = 5;
}
def : InstRW<[SPRWriteResGroup139], (instregex "^MOV64ao32$",
                                               "^MOVZX(32|64)rm(8|16)$")>;

def SPRWriteResGroup140 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 4, 16, 7, 2, 2, 12, 2];
  let Latency = 217;
  let NumMicroOps = 48;
}
def : InstRW<[SPRWriteResGroup140], (instregex "^MOV64dr$")>;

def SPRWriteResGroup141 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup141], (instregex "^MOV64o32a$")>;

def SPRWriteResGroup142 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort05]> {
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup142], (instregex "^MOV64rc$")>;

def SPRWriteResGroup143 : SchedWriteRes<[SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort05]> {
  let ResourceCycles = [3, 4, 8, 4, 2, 3];
  let Latency = 181;
  let NumMicroOps = 24;
}
def : InstRW<[SPRWriteResGroup143], (instregex "^MOV64rd$")>;

def SPRWriteResGroup144 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup144], (instregex "^MOV8ao(16|32|64)$")>;

def SPRWriteResGroup145 : SchedWriteRes<[SPRPort00_06, SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup145], (instregex "^MOVBE16mr$")>;

def SPRWriteResGroup146 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup146], (instregex "^MOVBE16rm$")>;

def SPRWriteResGroup147 : SchedWriteRes<[SPRPort01, SPRPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup147], (instregex "^MOVBE32rm$")>;

def SPRWriteResGroup148 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup148], (instregex "^MOVBE64mr$",
                                               "^PUSHF16$",
                                               "^SLDT16m$",
                                               "^STRm$")>;

def SPRWriteResGroup149 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup149], (instregex "^MOVBE64rm$")>;

def SPRWriteResGroup150 : SchedWriteRes<[SPRPort02_03_11]> {
  let Latency = 7;
}
def : InstRW<[SPRWriteResGroup150], (instregex "^(V?)MOV(D|SH|SL)DUPrm$",
                                               "^VBROADCASTI32X2Z128rm$",
                                               "^VBROADCASTSSZ128rm$",
                                               "^VBROADCASTSSrm$",
                                               "^VMOV(D|SH|SL)DUPZ128rm$",
                                               "^VPBROADCAST(D|Q)Z128rm$",
                                               "^VPBROADCAST(D|Q)rm$")>;

def SPRWriteResGroup151 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup151], (instregex "^MOVDIR64B(16|32|64)$")>;

def SPRWriteResGroup152 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 511;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup152], (instregex "^MOVDIRI32$")>;

def SPRWriteResGroup153 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 514;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup153], (instregex "^MOVDIRI64$")>;

def SPRWriteResGroup154 : SchedWriteRes<[SPRPort01_05, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup154, ReadAfterVecXLd], (instregex "^(V?)MOVLP(D|S)rm$",
                                                                "^(V?)SHUFP(D|S)rmi$",
                                                                "^VMOVLP(D|S)Z128rm$",
                                                                "^VSHUFP(D|S)Z128rm(bi|ik|ikz)$",
                                                                "^VSHUFP(D|S)Z128rm(i|bik)$",
                                                                "^VSHUFP(D|S)Z128rmbikz$")>;

def SPRWriteResGroup155 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 512;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup155], (instregex "^MOVNTDQmr$")>;

def SPRWriteResGroup156 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 518;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup156], (instregex "^MOVNTImr$")>;

def SPRWriteResGroup157 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup157], (instregex "^MOVSB$")>;

def SPRWriteResGroup158 : SchedWriteRes<[SPRPort00_01_05]>;
def : InstRW<[SPRWriteResGroup158], (instregex "^(V?)MOVS(D|S)rr$",
                                               "^(V?)MOVS(D|S)rr_REV$",
                                               "^(V?)P(ADD|SUB)(B|D|Q|W)rr$",
                                               "^VMOV(APD|UPS)Z(128|256)rrk(z?)$",
                                               "^VMOV(APD|UPS)Z(128|256)rrk(z?)_REV$",
                                               "^VMOV(APS|UPD)Z(128|256)rrk(z?)$",
                                               "^VMOV(APS|UPD)Z(128|256)rrk(z?)_REV$",
                                               "^VMOVDQ(A|U)(32|64)Z128rrk(z?)$",
                                               "^VMOVDQ(A|U)(32|64)Z128rrk(z?)_REV$",
                                               "^VMOVS(D|H|S)Zrr$",
                                               "^VMOVS(D|H|S)Zrr_REV$",
                                               "^VMOVS(D|S)Zrrk(z?)$",
                                               "^VMOVS(D|S)Zrrk(z?)_REV$",
                                               "^VP(ADD|SUB)(B|D|Q|W)Yrr$",
                                               "^VP(ADD|SUB)(B|D|Q|W)Z(128|256)rr$",
                                               "^VP(ADD|SUB)(D|Q)Z(128|256)rrk(z?)$",
                                               "^VPBLENDDrri$",
                                               "^VPMOVM2(D|Q)Z128rr$",
                                               "^VPTERNLOG(D|Q)Z(128|256)rri((k|kz)?)$")>;

def SPRWriteResGroup159 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [4, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup159], (instregex "^MOVS(L|Q|W)$")>;

def SPRWriteResGroup160 : SchedWriteRes<[SPRPort01_05_10, SPRPort02_03_11]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup160], (instregex "^MOVSX16rm8$")>;

def SPRWriteResGroup161 : SchedWriteRes<[SPRPort01_05_10]>;
def : InstRW<[SPRWriteResGroup161], (instregex "^MOVSX(16|32|64)rr8$",
                                               "^MOVSX(32|64)rr16$",
                                               "^MOVSX64rr32$")>;

def SPRWriteResGroup162 : SchedWriteRes<[SPRPort02_03_11]> {
  let Latency = 6;
}
def : InstRW<[SPRWriteResGroup162], (instregex "^MOVSX(32|64)rm(8|16)$",
                                               "^MOVSX64rm32$")>;

def SPRWriteResGroup163 : SchedWriteRes<[SPRPort00, SPRPort02_03]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup163], (instregex "^MUL_F(32|64)m$")>;

def SPRWriteResGroup164 : SchedWriteRes<[SPRPort00, SPRPort02_03, SPRPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup164], (instregex "^MUL_FI(16|32)m$")>;

def SPRWriteResGroup165 : SchedWriteRes<[SPRPort00]> {
  let Latency = 4;
}
def : InstRW<[SPRWriteResGroup165], (instregex "^MUL_F(P?)rST0$",
                                               "^MUL_FST0r$",
                                               "^V(U?)COMISHZrr(b?)$",
                                               "^VCVT(T?)P(D2Q|S2D)QZrr((b|k|bk|kz|bkz)?)$",
                                               "^VCVT(T?)PD2UQQZrr((b|k|bk|kz|bkz)?)$",
                                               "^VCVT(T?)PS2UDQZrr((b|k|bk|kz|bkz)?)$",
                                               "^VM(AX|IN)PSZrr((b|k|bk|kz|bkz)?)$",
                                               "^VPLZCNT(D|Q)Zrr((k|kz)?)$",
                                               "^VPMADD52(H|L)UQZr((k|kz)?)$")>;

def SPRWriteResGroup166 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort05, SPRPort06]> {
  let ResourceCycles = [7, 1, 2];
  let Latency = 20;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup166], (instregex "^MWAITrr$")>;

def SPRWriteResGroup167 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [6, 4, 1, 28, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[SPRWriteResGroup167], (instregex "^OUT16ir$")>;

def SPRWriteResGroup168 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [6, 6, 27, 15, 7, 1, 16, 1];
  let Latency = 35;
  let NumMicroOps = 79;
}
def : InstRW<[SPRWriteResGroup168], (instregex "^OUT16rr$")>;

def SPRWriteResGroup169 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [6, 4, 1, 30, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[SPRWriteResGroup169], (instregex "^OUT32ir$")>;

def SPRWriteResGroup170 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [6, 6, 29, 15, 9, 1, 18, 1];
  let Latency = 35;
  let NumMicroOps = 85;
}
def : InstRW<[SPRWriteResGroup170], (instregex "^OUT32rr$")>;

def SPRWriteResGroup171 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [5, 5, 1, 25, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[SPRWriteResGroup171], (instregex "^OUT8ir$")>;

def SPRWriteResGroup172 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [5, 5, 26, 15, 5, 1, 15, 1];
  let Latency = 35;
  let NumMicroOps = 73;
}
def : InstRW<[SPRWriteResGroup172], (instregex "^OUT8rr$")>;

def SPRWriteResGroup173 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [7, 6, 25, 16, 7, 1, 17, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 80;
}
def : InstRW<[SPRWriteResGroup173], (instregex "^OUTSB$")>;

def SPRWriteResGroup174 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [7, 6, 28, 16, 10, 1, 20, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 89;
}
def : InstRW<[SPRWriteResGroup174], (instregex "^OUTSL$")>;

def SPRWriteResGroup175 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [6, 1, 5, 27, 16, 8, 1, 18, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 83;
}
def : InstRW<[SPRWriteResGroup175], (instregex "^OUTSW$")>;

def SPRWriteResGroup176 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup176], (instregex "^VPOPCNT(B|D|Q|W)Z128rm$",
                                               "^VPOPCNT(D|Q)Z128rm(b|k|kz)$",
                                               "^VPOPCNT(D|Q)Z128rmbk(z?)$")>;
def : InstRW<[SPRWriteResGroup176, ReadAfterVecXLd], (instregex "^(V?)PACK(S|U)S(DW|WB)rm$",
                                                                "^(V?)PCMPGTQrm$",
                                                                "^VFPCLASSP(D|H|S)Z128rmb$",
                                                                "^VFPCLASSPHZ128rm$",
                                                                "^VPACK(S|U)S(DW|WB)Z128rm$",
                                                                "^VPACK(S|U)SDWZ128rmb$",
                                                                "^VPM(AXS|INU)QZ128rm((b|k|bk|kz|bkz)?)$",
                                                                "^VPM(AXU|INS)QZ128rm((b|k|bk|kz|bkz)?)$",
                                                                "^VPMULTISHIFTQBZ128rm(b?)$")>;
def : InstRW<[SPRWriteResGroup176, ReadAfterVecYLd], (instregex "^VFPCLASSP(D|H|S)Z((256)?)rm$",
                                                                "^VPERM(BZ|I2D|I2Q|T2D|T2Q)128rm$",
                                                                "^VPERM(I2D|T2Q)128rm(b|k|kz)$",
                                                                "^VPERM(I2D|T2Q)128rmbk(z?)$",
                                                                "^VPERM(I|T)2P(D|S)128rm((b|k|bk|kz|bkz)?)$",
                                                                "^VPERM(I2Q|T2D)128rm(b|k|kz)$",
                                                                "^VPERM(I2Q|T2D)128rmbk(z?)$")>;

def SPRWriteResGroup177 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup177], (instregex "^VBROADCASTI32X2Z128rmk(z?)$",
                                               "^VBROADCASTSSZ128rmk(z?)$",
                                               "^VMOV(APD|UPS)Z128rmk(z?)$",
                                               "^VMOV(APS|UPD)Z128rmk(z?)$",
                                               "^VMOV(D|SH|SL)DUPZ128rmk(z?)$",
                                               "^VMOVDQ(A|U)(32|64)Z128rmk(z?)$",
                                               "^VMOVS(D|S)Zrmk(z?)$",
                                               "^VPBROADCAST(D|Q)Z128rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup177, ReadAfterVecXLd], (instregex "^(V?)P(ADD|SUB)(B|D|Q|W)rm$",
                                                                "^VP(ADD|SUB)(B|D|Q|W)Z128rm$",
                                                                "^VP(ADD|SUB)(D|Q)Z128rm(b|k|kz)$",
                                                                "^VP(ADD|SUB)(D|Q)Z128rmbk(z?)$",
                                                                "^VPBLENDDrmi$",
                                                                "^VPTERNLOG(D|Q)Z128rm(bi|ik|ikz)$",
                                                                "^VPTERNLOG(D|Q)Z128rm(i|bik)$",
                                                                "^VPTERNLOG(D|Q)Z128rmbikz$")>;

def SPRWriteResGroup178 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup178], (instregex "^VPBROADCAST(B|W)Z128rm$",
                                               "^VPBROADCAST(B|W)rm$")>;
def : InstRW<[SPRWriteResGroup178, ReadAfterVecXLd], (instregex "^(V?)PALIGNRrmi$",
                                                                "^VALIGN(D|Q)Z128rm(bi|ik|ikz)$",
                                                                "^VALIGN(D|Q)Z128rm(i|bik)$",
                                                                "^VALIGN(D|Q)Z128rmbikz$",
                                                                "^VPALIGNRZ128rmi$")>;

def SPRWriteResGroup179 : SchedWriteRes<[SPRPort00_06, SPRPort05]> {
  let Latency = 140;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup179], (instregex "^PAUSE$")>;

def SPRWriteResGroup180 : SchedWriteRes<[SPRPort01, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup180, ReadAfterLd], (instregex "^P(DEP|EXT)(32|64)rm$")>;

def SPRWriteResGroup181 : SchedWriteRes<[SPRPort01_05, SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup181], (instregex "^(V?)PEXTR(D|Q)mr$",
                                               "^VPEXTR(D|Q)Zmr$",
                                               "^VPMOVQDZ128mr(k?)$")>;

def SPRWriteResGroup182 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup182, ReadAfterVecXLd], (instregex "^(V?)PH(ADD|SUB)SWrm$")>;

def SPRWriteResGroup183 : SchedWriteRes<[SPRPort00_01, SPRPort01_05]> {
  let ResourceCycles = [1, 2];
  let Latency = 2;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup183], (instregex "^(V?)PH(ADD|SUB)SWrr$",
                                               "^VPH(ADD|SUB)SWYrr$")>;

def SPRWriteResGroup184 : SchedWriteRes<[SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup184], (instregex "^POP(16|32|64)rmm$",
                                               "^PUSH(16|32)rmm$")>;

def SPRWriteResGroup185 : SchedWriteRes<[SPRPort02_03]> {
  let Latency = 5;
}
def : InstRW<[SPRWriteResGroup185], (instregex "^POPA(16|32)$",
                                               "^POPF32$")>;

def SPRWriteResGroup186 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [6, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup186], (instregex "^POPF16$")>;

def SPRWriteResGroup187 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup187], (instregex "^POPF64$")>;

def SPRWriteResGroup188 : SchedWriteRes<[SPRPort02_03_11]> {
  let Latency = 0;
}
def : InstRW<[SPRWriteResGroup188], (instregex "^PREFETCH(T0|T1|T2|NTA)$")>;

def SPRWriteResGroup189 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11, SPRPort06]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup189], (instregex "^PTWRITE((64)?)m$")>;

def SPRWriteResGroup190 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort06]> {
  let ResourceCycles = [1, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup190], (instregex "^PTWRITE64r$")>;

def SPRWriteResGroup191 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort06]> {
  let ResourceCycles = [2, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup191], (instregex "^PTWRITEr$")>;

def SPRWriteResGroup192 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup192], (instregex "^PUSH64r$")>;

def SPRWriteResGroup193 : SchedWriteRes<[SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup193], (instregex "^PUSH64rmm$")>;

def SPRWriteResGroup194 : SchedWriteRes<[SPRPort00_06, SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup194], (instregex "^PUSHF64$")>;

def SPRWriteResGroup195 : SchedWriteRes<[SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup195], (instregex "^PUSH(F|G)S64$")>;

def SPRWriteResGroup196 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [2, 3, 2];
  let Latency = 8;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup196], (instregex "^RC(L|R)(16|32|64)rCL$")>;

def SPRWriteResGroup197 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup197, WriteRMW], (instregex "^RC(L|R)8m(1|i)$")>;

def SPRWriteResGroup198 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [1, 5, 2];
  let Latency = 20;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup198, WriteRMW], (instregex "^RCL8mCL$")>;

def SPRWriteResGroup199 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [2, 5, 2];
  let Latency = 7;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup199], (instregex "^RCL8rCL$")>;

def SPRWriteResGroup200 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [2, 4, 3];
  let Latency = 20;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup200, WriteRMW], (instregex "^RCR8mCL$")>;

def SPRWriteResGroup201 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [3, 4, 3];
  let Latency = 9;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup201], (instregex "^RCR8rCL$")>;

def SPRWriteResGroup202 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_05, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort01_05_10, SPRPort05]> {
  let ResourceCycles = [1, 6, 1, 10, 20, 8, 5, 1, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 54;
}
def : InstRW<[SPRWriteResGroup202], (instregex "^RDMSR$")>;

def SPRWriteResGroup203 : SchedWriteRes<[SPRPort00_01_05_06]> {
  let Latency = 10;
}
def : InstRW<[SPRWriteResGroup203], (instregex "^RDPID32$")>;

def SPRWriteResGroup204 : SchedWriteRes<[SPRPort01]> {
  let Latency = 10;
}
def : InstRW<[SPRWriteResGroup204], (instregex "^RDPID64$")>;

def SPRWriteResGroup205 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup205], (instregex "^RDPKRUr$")>;

def SPRWriteResGroup206 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort05]> {
  let ResourceCycles = [9, 6, 2, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 18;
}
def : InstRW<[SPRWriteResGroup206], (instregex "^RDPMC$")>;

def SPRWriteResGroup207 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = 1386;
  let NumMicroOps = 25;
}
def : InstRW<[SPRWriteResGroup207], (instregex "^RDRAND16r$")>;

def SPRWriteResGroup208 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 3, 2, 5, 7, 3, 1, 2];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[SPRWriteResGroup208], (instregex "^RDRAND(32|64)r$")>;

def SPRWriteResGroup209 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = 1381;
  let NumMicroOps = 25;
}
def : InstRW<[SPRWriteResGroup209], (instregex "^RDSEED16r$")>;

def SPRWriteResGroup210 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 3, 3, 5, 7, 1, 4];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 25;
}
def : InstRW<[SPRWriteResGroup210], (instregex "^RDSEED(32|64)r$")>;

def SPRWriteResGroup211 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort05]> {
  let ResourceCycles = [5, 6, 3, 1];
  let Latency = 18;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup211], (instregex "^RDTSC$")>;

def SPRWriteResGroup212 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort05]> {
  let ResourceCycles = [2, 2, 1, 2, 7, 4, 3];
  let Latency = 42;
  let NumMicroOps = 21;
}
def : InstRW<[SPRWriteResGroup212], (instregex "^RDTSCP$")>;

def SPRWriteResGroup213 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup213], (instregex "^RET64$")>;

def SPRWriteResGroup214 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup214], (instregex "^RETI(16|32|64)$")>;

def SPRWriteResGroup215 : SchedWriteRes<[]>;
def : InstRW<[SPRWriteResGroup215], (instregex "^REX64_PREFIX$")>;

def SPRWriteResGroup216 : SchedWriteRes<[SPRPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup216, WriteRMW], (instregex "^RO(L|R)(16|32|64)m(1|i|CL)$")>;

def SPRWriteResGroup217 : SchedWriteRes<[SPRPort00_06]> {
  let ResourceCycles = [2];
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup217], (instregex "^RO(L|R)(8|16|32|64)r(1|i)$")>;

def SPRWriteResGroup218 : SchedWriteRes<[SPRPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup218, WriteRMW], (instregex "^RO(L|R)8m(1|i)$",
                                                         "^(ROL|SAR|SHR)8mCL$",
                                                         "^(ROR|SHL)8mCL$")>;

def SPRWriteResGroup219 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 15;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup219], (instregex "^(V?)ROUNDP(D|S)m$")>;
def : InstRW<[SPRWriteResGroup219, ReadAfterVecXLd], (instregex "^(V?)ROUNDS(D|S)m_Int$",
                                                                "^VRNDSCALEP(D|S)Z128rm(bi|ik|ikz)$",
                                                                "^VRNDSCALEP(D|S)Z128rm(i|bik)$",
                                                                "^VRNDSCALEP(D|S)Z128rmbikz$",
                                                                "^VRNDSCALES(D|S)Zm_Int((k|kz)?)$")>;

def SPRWriteResGroup220 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [2];
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup220], (instregex "^(V?)ROUNDP(D|S)r$",
                                               "^(V?)ROUNDS(D|S)r_Int$",
                                               "^VRNDSCALEP(D|S)Z(128|256)rri((k|kz)?)$",
                                               "^VRNDSCALES(D|S)Zr(b?)_Int((k|kz)?)$",
                                               "^VROUNDP(D|S)Yr$")>;

def SPRWriteResGroup221 : SchedWriteRes<[SPRPort00_06]> {
  let ResourceCycles = [2];
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup221], (instregex "^SAHF$")>;

def SPRWriteResGroup222 : SchedWriteRes<[SPRPort00_06]> {
  let Latency = 13;
}
def : InstRW<[SPRWriteResGroup222, WriteRMW], (instregex "^S(A|H)R8m(1|i)$",
                                                         "^SHL8m(1|i)$")>;

def SPRWriteResGroup223 : SchedWriteRes<[SPRPort00_06, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup223, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^S(A|H)RX(32|64)rm$",
                                                                                                                             "^SHLX(32|64)rm$")>;

def SPRWriteResGroup224 : SchedWriteRes<[SPRPort00_06]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup224], (instregex "^S(A|H)RX(32|64)rr$",
                                               "^SHLX(32|64)rr$")>;

def SPRWriteResGroup225 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 2, 1, 1, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup225], (instregex "^SERIALIZE$")>;

def SPRWriteResGroup226 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup226], (instregex "^SFENCE$")>;

def SPRWriteResGroup227 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 2, 2];
  let Latency = 21;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup227], (instregex "^S(G|I)DT64m$")>;

def SPRWriteResGroup228 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup228, ReadAfterVecXLd], (instregex "^SHA1MSG1rm$")>;

def SPRWriteResGroup229 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup229], (instregex "^SHA1MSG1rr$")>;

def SPRWriteResGroup230 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 2, 1, 2, 1];
  let Latency = 13;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup230, ReadAfterVecXLd], (instregex "^SHA1MSG2rm$")>;

def SPRWriteResGroup231 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort01_05]> {
  let ResourceCycles = [2, 2, 1, 2];
  let Latency = 6;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup231], (instregex "^SHA1MSG2rr$")>;

def SPRWriteResGroup232 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup232, ReadAfterVecXLd], (instregex "^SHA1NEXTErm$")>;

def SPRWriteResGroup233 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup233], (instregex "^SHA1NEXTErr$")>;

def SPRWriteResGroup234 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup234], (instregex "^VPMOV(S|Z)XBWZ((256)?)rmk(z?)$",
                                               "^VPOPCNT(B|W)Z(128|256)rmk(z?)$",
                                               "^VPOPCNT(B|W)Zrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup234, ReadAfterVecXLd], (instregex "^SHA1RNDS4rmi$",
                                                                "^SHA256RNDS2rm$",
                                                                "^VDBPSADBWZ128rmik(z?)$",
                                                                "^VPACK(S|U)SDWZ128rm(bk|kz)$",
                                                                "^VPACK(S|U)SDWZ128rm(k|bkz)$",
                                                                "^VPACK(S|U)SWBZ128rmk(z?)$",
                                                                "^VPMULTISHIFTQBZ128rm(bk|kz)$",
                                                                "^VPMULTISHIFTQBZ128rm(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup234, ReadAfterVecYLd], (instregex "^VDBPSADBWZ((256)?)rmik(z?)$",
                                                                "^VPACK(S|U)SDWZ((256)?)rmbk(z?)$",
                                                                "^VPACK(S|U)S(DW|WB)Z((256)?)rmk(z?)$",
                                                                "^VPERMBZ(128|256)rmk(z?)$",
                                                                "^VPERMBZrmk(z?)$",
                                                                "^VPMULTISHIFTQBZ((256)?)rm(bk|kz)$",
                                                                "^VPMULTISHIFTQBZ((256)?)rm(k|bkz)$")>;

def SPRWriteResGroup235 : SchedWriteRes<[SPRPort05]> {
  let Latency = 6;
}
def : InstRW<[SPRWriteResGroup235], (instregex "^SHA1RNDS4rri$",
                                               "^SHA256RNDS2rr$")>;

def SPRWriteResGroup236 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [3, 2, 1, 1, 1];
  let Latency = 12;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup236, ReadAfterVecXLd], (instregex "^SHA256MSG1rm$")>;

def SPRWriteResGroup237 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [3, 2, 1, 1];
  let Latency = 5;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup237], (instregex "^SHA256MSG1rr$")>;

def SPRWriteResGroup238 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup238, ReadAfterVecXLd], (instregex "^SHA256MSG2rm$")>;
def : InstRW<[SPRWriteResGroup238, ReadAfterVecYLd], (instregex "^VPEXPAND(B|W)Z(128|256)rmk(z?)$",
                                                                "^VPEXPAND(B|W)Zrmk(z?)$")>;

def SPRWriteResGroup239 : SchedWriteRes<[SPRPort05]> {
  let ResourceCycles = [2];
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup239], (instregex "^SHA256MSG2rr$",
                                               "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZrrk(z?)$",
                                               "^VPMOV(D|Q|SQ|USQ)WZrrk(z?)$",
                                               "^VPMOV(U?)SD(B|W)Zrrk(z?)$")>;

def SPRWriteResGroup240 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup240], (instregex "^SHRD16mri8$")>;

def SPRWriteResGroup241 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup241], (instregex "^SLDT(32|64)r$")>;

def SPRWriteResGroup242 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort05]> {
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup242], (instregex "^SMSW16r$")>;

def SPRWriteResGroup243 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort05]> {
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup243], (instregex "^SMSW(32|64)r$")>;

def SPRWriteResGroup244 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup244], (instregex "^STD$")>;

def SPRWriteResGroup245 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [1, 4, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup245], (instregex "^STI$")>;

def SPRWriteResGroup246 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup246], (instregex "^STOSB$")>;

def SPRWriteResGroup247 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 7;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup247], (instregex "^STOS(L|Q|W)$")>;

def SPRWriteResGroup248 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort01]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup248], (instregex "^STR(32|64)r$")>;

def SPRWriteResGroup249 : SchedWriteRes<[SPRPort01_05]> {
  let Latency = 4;
}
def : InstRW<[SPRWriteResGroup249], (instregex "^V(ADD|SUB)P(D|S)Z(128|256)rrkz$",
                                               "^V(ADD|SUB)S(D|S)Zrr(b?)_Intkz$")>;

def SPRWriteResGroup250 : SchedWriteRes<[SPRPort00_05]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup250], (instregex "^V(ADD|SUB)P(D|S)Zrr(b?)$",
                                               "^VMOVDQU(8|16)Zrrk(z?)$",
                                               "^VMOVDQU(8|16)Zrrk(z?)_REV$",
                                               "^VP(ADD|SUB)(B|W)Zrrk(z?)$",
                                               "^VPBLENDM(B|W)Zrrk(z?)$",
                                               "^VPMOVM2(B|W)Zrr$")>;

def SPRWriteResGroup251 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 6;
}
def : InstRW<[SPRWriteResGroup251], (instregex "^V(ADD|MAX|MIN|MUL|SUB)PHZ(128|256)rrk(z?)$",
                                               "^V(ADD|MAX|MIN|MUL|SUB)SHZrr(b?)_Intk(z?)$",
                                               "^VCVT(T?)PH2(U?)WZ(128|256)rrk(z?)$",
                                               "^VCVT(U?)W2PHZ(128|256)rrk(z?)$",
                                               "^VF(N?)MADD(132|213|231)PHZ(128|256)rk(z?)$",
                                               "^VF(N?)M(ADD|SUB)(132|213|231)SHZr(b?)_Intk(z?)$",
                                               "^VFM((ADD)?)SUB(132|213|231)PHZ(128|256)rk(z?)$",
                                               "^VFMSUBADD(132|213|231)PHZ(128|256)rk(z?)$",
                                               "^VFNMSUB(132|213|231)PHZ(128|256)rk(z?)$",
                                               "^VGETEXPPHZ(128|256)rk(z?)$",
                                               "^VGETEXPSHZr(bk|kz)$",
                                               "^VGETEXPSHZr(k|bkz)$",
                                               "^VGETMANTPHZ(128|256)rrik(z?)$",
                                               "^VGETMANTSHZrri(bk|kz)$",
                                               "^VGETMANTSHZrri(k|bkz)$")>;

def SPRWriteResGroup252 : SchedWriteRes<[SPRPort00]> {
  let Latency = 5;
}
def : InstRW<[SPRWriteResGroup252], (instregex "^V(ADD|MAX|MIN|MUL|SUB)PHZrr(b?)$",
                                               "^VAES(DE|EN)CLASTZrr$",
                                               "^VAES(DE|EN)CZrr$",
                                               "^VCVT(T?)PH2(U?)WZrr(b?)$",
                                               "^VCVT(U?)W2PHZrr(b?)$",
                                               "^VF(N?)MADD(132|213|231)PHZr(b?)$",
                                               "^VFM((ADD)?)SUB(132|213|231)PHZr(b?)$",
                                               "^VFMSUBADD(132|213|231)PHZr(b?)$",
                                               "^VFNMSUB(132|213|231)PHZr(b?)$",
                                               "^VGETEXPPHZr(b?)$",
                                               "^VGETMANTPHZrri(b?)$",
                                               "^VGF2P8MULBZrr$",
                                               "^VMOVMSKP(D|S)Yrr$")>;

def SPRWriteResGroup253 : SchedWriteRes<[SPRPort00]> {
  let Latency = 6;
}
def : InstRW<[SPRWriteResGroup253], (instregex "^V(ADD|MAX|MIN|MUL|SUB)PHZrr(bk|kz)$",
                                               "^V(ADD|MAX|MIN|MUL|SUB)PHZrr(k|bkz)$",
                                               "^VCVT(T?)PH2(U?)WZrr(bk|kz)$",
                                               "^VCVT(T?)PH2(U?)WZrr(k|bkz)$",
                                               "^VCVT(U?)W2PHZrr(bk|kz)$",
                                               "^VCVT(U?)W2PHZrr(k|bkz)$",
                                               "^VF(N?)MADD(132|213|231)PHZr(bk|kz)$",
                                               "^VF(N?)MADD(132|213|231)PHZr(k|bkz)$",
                                               "^VFM((ADD)?)SUB(132|213|231)PHZr(bk|kz)$",
                                               "^VFM((ADD)?)SUB(132|213|231)PHZr(k|bkz)$",
                                               "^VFMSUBADD(132|213|231)PHZr(bk|kz)$",
                                               "^VFMSUBADD(132|213|231)PHZr(k|bkz)$",
                                               "^VFNMSUB(132|213|231)PHZr(bk|kz)$",
                                               "^VFNMSUB(132|213|231)PHZr(k|bkz)$",
                                               "^VGETEXPPHZr(bk|kz)$",
                                               "^VGETEXPPHZr(k|bkz)$",
                                               "^VGETMANTPHZrri(bk|kz)$",
                                               "^VGETMANTPHZrri(k|bkz)$")>;

def SPRWriteResGroup254 : SchedWriteRes<[SPRPort01_05, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup254], (instregex "^VPMOV(S|Z)XBWZ128rmk(z?)$",
                                               "^VPSHUF(H|L)WZ(128|256)mik(z?)$")>;
def : InstRW<[SPRWriteResGroup254, ReadAfterVecYLd], (instregex "^V(ADD|SUB)PSYrm$",
                                                                "^V(ADD|SUB)PSZ256rm((b|k|bk|kz|bkz)?)$",
                                                                "^VADDSUBPSYrm$",
                                                                "^VPSHUFBZ256rmk(z?)$",
                                                                "^VPUNPCK(HBW|LWD)Z256rmk(z?)$",
                                                                "^VPUNPCK(HWD|LBW)Z256rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup254, ReadAfterVecXLd], (instregex "^VPSHUFBZ128rmk(z?)$",
                                                                "^VPUNPCK(HBW|LWD)Z128rmk(z?)$",
                                                                "^VPUNPCK(HWD|LBW)Z128rmk(z?)$")>;

def SPRWriteResGroup255 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup255], (instregex "^VMOVDQU(8|16)Zrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup255, ReadAfterVecYLd], (instregex "^V(ADD|SUB)PSZrm((b|k|bk|kz|bkz)?)$",
                                                                "^VP(ADD|SUB)(B|W)Zrmk(z?)$",
                                                                "^VPBLENDM(B|W)Zrmk(z?)$")>;

def SPRWriteResGroup256 : SchedWriteRes<[SPRPort00_05]> {
  let Latency = 4;
}
def : InstRW<[SPRWriteResGroup256], (instregex "^V(ADD|SUB)PSZrr(bk|kz)$",
                                               "^V(ADD|SUB)PSZrr(k|bkz)$")>;

def SPRWriteResGroup257 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup257], (instregex "^VCVT(T?)PS2(U?)DQZrm((b|k|bk|kz|bkz)?)$",
                                               "^VPLZCNT(D|Q)Zrm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup257, ReadAfterVecXLd], (instregex "^VAES(DE|EN)CLASTZrm$",
                                                                "^VAES(DE|EN)CZrm$")>;
def : InstRW<[SPRWriteResGroup257, ReadAfterVecYLd], (instregex "^VGF2P8AFFINE((INV)?)QBZrm(b?)i$",
                                                                "^VGF2P8MULBZrm$",
                                                                "^VPMADD52(H|L)UQZm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup258 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup258], (instregex "^VPBROADCAST(B|W)Z128rmk(z?)$",
                                               "^VPOPCNT(B|D|Q|W)Z((256)?)rm$",
                                               "^VPOPCNT(D|Q)Z((256)?)rm(b|k|kz)$",
                                               "^VPOPCNT(D|Q)Z((256)?)rmbk(z?)$",
                                               "^VPSHUF(H|L)WZmik(z?)$")>;
def : InstRW<[SPRWriteResGroup258, ReadAfterVecYLd], (instregex "^VALIGN(D|Q)Z((256)?)rm(bi|ik|ikz)$",
                                                                "^VALIGN(D|Q)Z((256)?)rm(i|bik)$",
                                                                "^VALIGN(D|Q)Z((256)?)rmbikz$",
                                                                "^VFPCLASSP(D|H|S)Z((256)?)rmb$",
                                                                "^VPACK(S|U)S(DWY|WBZ)rm$",
                                                                "^VPACK(S|U)S(DW|WB)Z256rm$",
                                                                "^VPACK(S|U)SDWZ((256)?)rmb$",
                                                                "^VPACK(S|U)S(DWZ|WBY)rm$",
                                                                "^VPALIGNRZ((256)?)rmik(z?)$",
                                                                "^VPCMPGTQYrm$",
                                                                "^VPM(AXS|INU)QZ((256)?)rm((b|k|bk|kz|bkz)?)$",
                                                                "^VPM(AXU|INS)QZ((256)?)rm((b|k|bk|kz|bkz)?)$",
                                                                "^VPMULTISHIFTQBZ((256)?)rm(b?)$",
                                                                "^VPUNPCK(HBW|LWD)Zrmk(z?)$",
                                                                "^VPUNPCK(HWD|LBW)Zrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup258, ReadAfterVecXLd], (instregex "^VPALIGNRZ128rmik(z?)$",
                                                                "^VPCLMULQDQ(Y|Z)rm$",
                                                                "^VPCLMULQDQZ256rm$")>;

def SPRWriteResGroup259 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup259, ReadAfterVecYLd, ReadAfterVecYLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^VBLENDVP(D|S)Yrm$",
                                                                                                                                                  "^VPBLENDVBYrm$")>;

def SPRWriteResGroup260 : SchedWriteRes<[SPRPort00_01_05]> {
  let ResourceCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup260], (instregex "^VBLENDVP(S|DY)rr$",
                                               "^VBLENDVP(D|SY)rr$",
                                               "^VPBLENDVB(Y?)rr$")>;

def SPRWriteResGroup261 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 9;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup261, ReadAfterVecXLd, ReadAfterVecXLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^VBLENDVP(D|S)rm$",
                                                                                                                                                  "^VPBLENDVBrm$")>;

def SPRWriteResGroup262 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup262], (instregex "^VBROADCAST(F|I)32X(2|4)Z256rmk(z?)$",
                                               "^VBROADCAST(F|I)64X2Z128rmk(z?)$",
                                               "^VBROADCASTS(D|S)Z256rmk(z?)$",
                                               "^VMOV(APD|UPS)Z256rmk(z?)$",
                                               "^VMOV(APS|UPD)Z256rmk(z?)$",
                                               "^VMOV(D|SH|SL)DUPZ256rmk(z?)$",
                                               "^VMOVDQ(A|U)(32|64)Z256rmk(z?)$",
                                               "^VPBROADCAST(D|Q)Z256rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup262, ReadAfterVecYLd], (instregex "^VINSERT(F|I)128rm$",
                                                                "^VINSERT(F|I)32x4Z256rm((k|kz)?)$",
                                                                "^VINSERT(F|I)64x2Z256rm((k|kz)?)$",
                                                                "^VP(ADD|SUB)(B|D|Q|W)Yrm$",
                                                                "^VP(ADD|SUB)(B|D|Q|W)Z256rm$",
                                                                "^VP(ADD|SUB)(D|Q)Z256rm(b|k|kz)$",
                                                                "^VP(ADD|SUB)(D|Q)Z256rmbk(z?)$",
                                                                "^VPTERNLOG(D|Q)Z256rm(bi|ik|ikz)$",
                                                                "^VPTERNLOG(D|Q)Z256rm(i|bik)$",
                                                                "^VPTERNLOG(D|Q)Z256rmbikz$")>;

def SPRWriteResGroup263 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup263, ReadAfterVecXLd], (instregex "^VCMPP(D|H|S)Z128rmbi(k?)$",
                                                                "^V(CMP|PCM)PDZ128rmi(k?)$",
                                                                "^VCMPP(H|S)Z128rmi(k?)$",
                                                                "^VFPCLASSP(D|H|S)Z128rm(b?)k$",
                                                                "^VPCMP(B|Q|W|UD|UQ|UW)Z128rmi(k?)$",
                                                                "^VPCMP(D|Q|UQ)Z128rmib(k?)$",
                                                                "^VPCMP(EQB|GTD|GTQ|GTW)Z128rm(k?)$",
                                                                "^VPCMP(EQD|GTB)Z128rm(k?)$",
                                                                "^VPCMP(EQD|GTQ)Z128rmb(k?)$",
                                                                "^VPCMPEQ(Q|W)Z128rm(k?)$",
                                                                "^VPCMP(EQQ|GTD)Z128rmb(k?)$",
                                                                "^VPCMPUBZ128rmi(k?)$",
                                                                "^VPCMPUDZ128rmib(k?)$",
                                                                "^VPTEST(MB|NMD|NMQ|NMW)Z128rm(k?)$",
                                                                "^VPTEST(MD|NMB)Z128rm(k?)$",
                                                                "^VPTEST(MD|NMQ)Z128rmb(k?)$",
                                                                "^VPTESTM(Q|W)Z128rm(k?)$",
                                                                "^VPTEST(MQ|NMD)Z128rmb(k?)$")>;
def : InstRW<[SPRWriteResGroup263, ReadAfterVecYLd], (instregex "^VCMPP(D|H|S)Z((256)?)rmbi(k?)$",
                                                                "^V(CMP|PCM)PDZ((256)?)rmi(k?)$",
                                                                "^VCMPP(H|S)Z((256)?)rmi(k?)$",
                                                                "^VFPCLASSP(D|H|S)Z((256)?)rm(b?)k$",
                                                                "^VPCMP(B|Q|W|UD|UQ|UW)Z((256)?)rmi(k?)$",
                                                                "^VPCMP(D|Q|UQ)Z((256)?)rmib(k?)$",
                                                                "^VPCMP(EQB|GTD|GTQ|GTW)Z((256)?)rm(k?)$",
                                                                "^VPCMP(EQD|GTB)Z((256)?)rm(k?)$",
                                                                "^VPCMP(EQD|GTQ)Z((256)?)rmb(k?)$",
                                                                "^VPCMPEQ(Q|W)Z((256)?)rm(k?)$",
                                                                "^VPCMP(EQQ|GTD)Z((256)?)rmb(k?)$",
                                                                "^VPCMPUBZ((256)?)rmi(k?)$",
                                                                "^VPCMPUDZ((256)?)rmib(k?)$",
                                                                "^VPTEST(MB|NMD|NMQ|NMW)Z((256)?)rm(k?)$",
                                                                "^VPTEST(MD|NMB)Z((256)?)rm(k?)$",
                                                                "^VPTEST(MD|NMQ)Z((256)?)rmb(k?)$",
                                                                "^VPTESTM(Q|W)Z((256)?)rm(k?)$",
                                                                "^VPTEST(MQ|NMD)Z((256)?)rmb(k?)$")>;
def : InstRW<[SPRWriteResGroup263, ReadAfterVecLd], (instregex "^VCMPS(D|H|S)Zrm_Int(k?)$",
                                                               "^VFPCLASSS(D|H|S)Zrmk$")>;

def SPRWriteResGroup264 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 4;
}
def : InstRW<[SPRWriteResGroup264], (instregex "^VCMPPSYrri$",
                                               "^VCVT(T?)PD2(U?)QQZ(128|256)rr((k|kz)?)$",
                                               "^VCVT(U?)QQ2PDZ(128|256)rr((k|kz)?)$",
                                               "^VM(AX|IN)PSYrr$",
                                               "^VM(AX|IN)PSZ256rr((k|kz)?)$",
                                               "^VPLZCNT(D|Q)Z(128|256)rr((k|kz)?)$",
                                               "^VPMADD52(H|L)UQZ(128|256)r((k|kz)?)$")>;
def : InstRW<[SPRWriteResGroup264, ReadAfterVecLd], (instregex "^VFIXUPIMMS(D|S)Zrrib((k|kz)?)$")>;

def SPRWriteResGroup265 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup265, ReadAfterVecLd], (instregex "^V(U?)COMISHZrm$")>;

def SPRWriteResGroup266 : SchedWriteRes<[SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup266], (instregex "^VCOMPRESSP(D|S)Z(128|256)mr$",
                                               "^VCOMPRESSP(D|S)Zmr$",
                                               "^VPCOMPRESS(D|Q)Z(128|256)mr$",
                                               "^VPCOMPRESS(D|Q)Zmr$",
                                               "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZmr$",
                                               "^VPMOV(D|Q|SQ|USQ)WZmr$",
                                               "^VPMOV(U?)S(DB|QD)Zmr$",
                                               "^VPMOV(U?)SDWZmr$")>;

def SPRWriteResGroup267 : SchedWriteRes<[SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup267], (instregex "^VCOMPRESSP(D|S)Z(128|256)mrk$",
                                               "^VCOMPRESSP(D|S)Zmrk$",
                                               "^VPCOMPRESS(D|Q)Z(128|256)mrk$",
                                               "^VPCOMPRESS(D|Q)Zmrk$",
                                               "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZmrk$",
                                               "^VPMOV(D|Q|SQ|USQ)WZmrk$",
                                               "^VPMOV(U?)S(DB|QD)Zmrk$",
                                               "^VPMOV(U?)SDWZmrk$")>;

def SPRWriteResGroup268 : SchedWriteRes<[SPRPort05]> {
  let ResourceCycles = [2];
  let Latency = 3;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup268], (instregex "^VCOMPRESSP(D|S)Z(128|256)rr$",
                                               "^VCOMPRESSP(D|S)Zrr$",
                                               "^VEXPANDP(D|S)Z(128|256)rr$",
                                               "^VEXPANDP(D|S)Zrr$",
                                               "^VPCOMPRESS(B|D|Q|W)Z(128|256)rr$",
                                               "^VPCOMPRESS(B|D|Q|W)Zrr$",
                                               "^VPEXPAND(B|D|Q|W)Z(128|256)rr$",
                                               "^VPEXPAND(B|D|Q|W)Zrr$")>;

def SPRWriteResGroup269 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let Latency = 7;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup269], (instregex "^VCVT(U?)DQ2PDZrr((k|kz)?)$",
                                               "^VCVT(T?)PS2(U?)QQZrr((b|k|bk|kz|bkz)?)$",
                                               "^VCVT(U?)QQ2PSZrr((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup270 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup270], (instregex "^VCVT(U?)DQ2PHZ128rm(b?)$",
                                               "^VCVTNEPS2BF16Z128rm(b?)$")>;

def SPRWriteResGroup271 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 19;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup271], (instregex "^VCVT(U?)DQ2PHZ128rm(bk|kz)$",
                                               "^VCVT(U?)DQ2PHZ128rm(k|bkz)$")>;

def SPRWriteResGroup272 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup272], (instregex "^VCVT(U?)DQ2PHZ128rr$")>;

def SPRWriteResGroup273 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup273], (instregex "^VCVT(U?)DQ2PHZ128rrk(z?)$")>;

def SPRWriteResGroup274 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 17;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup274], (instregex "^VCVT(U?)DQ2PHZ256rm(b?)$",
                                               "^VCVTNEPS2BF16Z128rm(bk|kz)$",
                                               "^VCVTNEPS2BF16Z128rm(k|bkz)$")>;

def SPRWriteResGroup275 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 21;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup275], (instregex "^VCVT(U?)DQ2PHZ256rm(bk|kz)$",
                                               "^VCVT(U?)DQ2PHZ256rm(k|bkz)$")>;

def SPRWriteResGroup276 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup276], (instregex "^VCVT(U?)DQ2PHZ256rr$")>;

def SPRWriteResGroup277 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup277], (instregex "^VCVT(U?)DQ2PHZ256rrk(z?)$")>;

def SPRWriteResGroup278 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 17;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup278], (instregex "^VCVT(U?)DQ2PHZrm(b?)$")>;

def SPRWriteResGroup279 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 21;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup279], (instregex "^VCVT(U?)DQ2PHZrm(bk|kz)$",
                                               "^VCVT(U?)DQ2PHZrm(k|bkz)$")>;

def SPRWriteResGroup280 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup280], (instregex "^VCVT(U?)DQ2PHZrr(b?)$")>;

def SPRWriteResGroup281 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup281], (instregex "^VCVT(U?)DQ2PHZrr(bk|kz)$",
                                               "^VCVT(U?)DQ2PHZrr(k|bkz)$")>;

def SPRWriteResGroup282 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1];
  let Latency = 15;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup282, ReadAfterVecXLd], (instregex "^VCVTNE2PS2BF16Z128rm(b?)$")>;

def SPRWriteResGroup283 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1];
  let Latency = 17;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup283, ReadAfterVecXLd], (instregex "^VCVTNE2PS2BF16Z128rm(bk|kz)$",
                                                                "^VCVTNE2PS2BF16Z128rm(k|bkz)$")>;

def SPRWriteResGroup284 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup284], (instregex "^VCVTNE2PS2BF16Z(128|256)rr$")>;

def SPRWriteResGroup285 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup285], (instregex "^VCVTNE2PS2BF16Z(128|256)rrk(z?)$")>;

def SPRWriteResGroup286 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1];
  let Latency = 16;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup286, ReadAfterVecYLd], (instregex "^VCVTNE2PS2BF16Z256rm(b?)$")>;

def SPRWriteResGroup287 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1];
  let Latency = 18;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup287, ReadAfterVecYLd], (instregex "^VCVTNE2PS2BF16Z256rm(bk|kz)$",
                                                                "^VCVTNE2PS2BF16Z256rm(k|bkz)$")>;

def SPRWriteResGroup288 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 16;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup288, ReadAfterVecYLd], (instregex "^VCVTNE2PS2BF16Zrm(b?)$",
                                                                "^VDPBF16PSZm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup289 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 18;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup289, ReadAfterVecYLd], (instregex "^VCVTNE2PS2BF16Zrm(bk|kz)$",
                                                                "^VCVTNE2PS2BF16Zrm(k|bkz)$")>;

def SPRWriteResGroup290 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup290], (instregex "^VCVTNE2PS2BF16Zrr$",
                                               "^VDPBF16PSZr((k|kz)?)$")>;

def SPRWriteResGroup291 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup291], (instregex "^VCVTNE2PS2BF16Zrrk(z?)$")>;

def SPRWriteResGroup292 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup292], (instregex "^VCVTNEPS2BF16Z(128|256)rr$")>;

def SPRWriteResGroup293 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort05]> {
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup293], (instregex "^VCVTNEPS2BF16Z(128|256)rrk(z?)$")>;

def SPRWriteResGroup294 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup294], (instregex "^VCVTNEPS2BF16Z256rm(b?)$")>;

def SPRWriteResGroup295 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup295], (instregex "^VCVTNEPS2BF16Z256rm(bk|kz)$",
                                               "^VCVTNEPS2BF16Z256rm(k|bkz)$")>;

def SPRWriteResGroup296 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup296], (instregex "^VCVTNEPS2BF16Zrm(b?)$")>;

def SPRWriteResGroup297 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup297], (instregex "^VCVTNEPS2BF16Zrm(bk|kz)$",
                                               "^VCVTNEPS2BF16Zrm(k|bkz)$")>;

def SPRWriteResGroup298 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup298], (instregex "^VCVTNEPS2BF16Zrr$")>;

def SPRWriteResGroup299 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup299], (instregex "^VCVTNEPS2BF16Zrrk(z?)$")>;

def SPRWriteResGroup300 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 15;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup300], (instregex "^VCVT(T?)PD2DQYrm$",
                                               "^VCVT(PD|TPH)2(U?)DQZ256rm(b?)$",
                                               "^VCVT(T?)PD2(U?)DQZ256rm(bk|kz)$",
                                               "^VCVT(T?)PD2(U?)DQZ256rm(k|bkz)$",
                                               "^VCVT(PH|TPD)2DQZ256rm(b?)$",
                                               "^VCVTPH2PSXZ128rm(bk|kz)$",
                                               "^VCVTPH2PSXZ128rm(k|bkz)$",
                                               "^VCVTPH2(PSX|UDQ)Z256rm(b?)$",
                                               "^VCVT(U?)QQ2PSZ256rm((b|k|bk|kz|bkz)?)$",
                                               "^VCVTTPD2UDQZ256rm(b?)$")>;

def SPRWriteResGroup301 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 15;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup301], (instregex "^VCVT(PD|TPH)2(U?)DQZrm(b?)$",
                                               "^VCVT(T?)PD2(U?)DQZrm(bk|kz)$",
                                               "^VCVT(T?)PD2(U?)DQZrm(k|bkz)$",
                                               "^VCVT(PH|TPD)2DQZrm(b?)$",
                                               "^VCVTPH2(PSX|UDQ)Zrm(b?)$",
                                               "^VCVT(U?)QQ2PSZrm((b|k|bk|kz|bkz)?)$",
                                               "^VCVTTPD2UDQZrm(b?)$")>;

def SPRWriteResGroup302 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1, 2];
  let Latency = 19;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup302], (instregex "^VCVTPD2PHZ128rm(b?)$")>;

def SPRWriteResGroup303 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1, 2];
  let Latency = 22;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup303], (instregex "^VCVTPD2PHZ128rm(bk|kz)$",
                                               "^VCVTPD2PHZ128rm(k|bkz)$")>;

def SPRWriteResGroup304 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 12;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup304], (instregex "^VCVTPD2PHZ128rr$")>;

def SPRWriteResGroup305 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 15;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup305], (instregex "^VCVTPD2PHZ128rrk(z?)$")>;

def SPRWriteResGroup306 : SchedWriteRes<[SPRPort00_01, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 21;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup306], (instregex "^VCVTPD2PHZ256rm(b?)$")>;

def SPRWriteResGroup307 : SchedWriteRes<[SPRPort00_01, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 24;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup307], (instregex "^VCVTPD2PHZ256rm(bk|kz)$",
                                               "^VCVTPD2PHZ256rm(k|bkz)$")>;

def SPRWriteResGroup308 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup308], (instregex "^VCVTPD2PHZ256rr$")>;

def SPRWriteResGroup309 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup309], (instregex "^VCVTPD2PHZ256rrk(z?)$")>;

def SPRWriteResGroup310 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 23;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup310], (instregex "^VCVTPD2PHZrm(b?)$",
                                               "^VCVTPH2PDZrm(b?)$")>;

def SPRWriteResGroup311 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 26;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup311], (instregex "^VCVTPD2PHZrm(bk|kz)$",
                                               "^VCVTPD2PHZrm(k|bkz)$",
                                               "^VCVTPH2PDZrm(bk|kz)$",
                                               "^VCVTPH2PDZrm(k|bkz)$")>;

def SPRWriteResGroup312 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup312], (instregex "^VCVTPD2PHZrr(b?)$",
                                               "^VCVTPH2PDZrr(b?)$")>;

def SPRWriteResGroup313 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup313], (instregex "^VCVTPD2PHZrr(bk|kz)$",
                                               "^VCVTPD2PHZrr(k|bkz)$",
                                               "^VCVTPH2PDZrr(bk|kz)$",
                                               "^VCVTPH2PDZrr(k|bkz)$")>;

def SPRWriteResGroup314 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup314], (instregex "^VCVT(T?)PD2(U?)QQZ128rm((b|k|bk|kz|bkz)?)$",
                                               "^VPABS(B|W)Z(128|256)rmk(z?)$",
                                               "^VPLZCNT(D|Q)Z128rm((b|k|bk|kz|bkz)?)$",
                                               "^VPS(L|R)LWZ(128|256)mik(z?)$",
                                               "^VPSRAWZ(128|256)mik(z?)$")>;
def : InstRW<[SPRWriteResGroup314, ReadAfterVecXLd], (instregex "^VP(ADD|MAX|MIN|SUB)S(B|W)Z128rmk(z?)$",
                                                                "^VP(ADD|SUB)US(B|W)Z128rmk(z?)$",
                                                                "^VPAVG(B|W)Z128rmk(z?)$",
                                                                "^VPMADD52(H|L)UQZ128m((b|k|bk|kz|bkz)?)$",
                                                                "^VPM(AX|IN)U(B|W)Z128rmk(z?)$",
                                                                "^VPSH(L|R)DVWZ128mk(z?)$",
                                                                "^VPS(RA|RL|LLV)WZ128rmk(z?)$",
                                                                "^VPS(LL|RAV|RLV)WZ128rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup314, ReadAfterVecYLd], (instregex "^VP(ADD|MAX|MIN|SUB)S(B|W)Z256rmk(z?)$",
                                                                "^VP(ADD|SUB)US(B|W)Z256rmk(z?)$",
                                                                "^VPAVG(B|W)Z256rmk(z?)$",
                                                                "^VPM(AX|IN)U(B|W)Z256rmk(z?)$",
                                                                "^VPSH(L|R)DVWZ256mk(z?)$",
                                                                "^VPS(RA|RL|LLV)WZ256rmk(z?)$",
                                                                "^VPS(LL|RAV|RLV)WZ256rmk(z?)$")>;

def SPRWriteResGroup315 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup315], (instregex "^VCVT(T?)PH2(U?)DQZ128rm(b?)$",
                                               "^VCVTPS2PHXZ128rm(b?)$")>;

def SPRWriteResGroup316 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 17;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup316], (instregex "^VCVT(T?)PH2(U?)DQZ128rm(bk|kz)$",
                                               "^VCVT(T?)PH2(U?)DQZ128rm(k|bkz)$")>;

def SPRWriteResGroup317 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup317], (instregex "^VCVT(T?)PH2DQZ(128|256)rrk(z?)$",
                                               "^VCVTPH2(PSX|UDQ)Z256rrk(z?)$",
                                               "^VCVTPH2PSZ256rrk(z?)$",
                                               "^VCVT(T?)PH2UDQZ128rrk(z?)$",
                                               "^VCVTPS2PH(X?)Z256rrk(z?)$",
                                               "^VCVTTPH2UDQZ256rrk(z?)$")>;

def SPRWriteResGroup318 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 18;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup318], (instregex "^VCVT(T?)PH2DQZ256rm(bk|kz)$",
                                               "^VCVT(T?)PH2DQZ256rm(k|bkz)$",
                                               "^VCVTPH2(PSX|UDQ)Z256rm(bk|kz)$",
                                               "^VCVTPH2(PSX|UDQ)Z256rm(k|bkz)$",
                                               "^VCVTPS2PHXZ256rm(bk|kz)$",
                                               "^VCVTPS2PHXZ256rm(k|bkz)$",
                                               "^VCVTTPH2UDQZ256rm(bk|kz)$",
                                               "^VCVTTPH2UDQZ256rm(k|bkz)$")>;

def SPRWriteResGroup319 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 18;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup319], (instregex "^VCVT(T?)PH2DQZrm(bk|kz)$",
                                               "^VCVT(T?)PH2DQZrm(k|bkz)$",
                                               "^VCVTPH2(PSX|UDQ)Zrm(bk|kz)$",
                                               "^VCVTPH2(PSX|UDQ)Zrm(k|bkz)$",
                                               "^VCVTPS2PHXZrm(bk|kz)$",
                                               "^VCVTPS2PHXZrm(k|bkz)$",
                                               "^VCVTTPH2UDQZrm(bk|kz)$",
                                               "^VCVTTPH2UDQZrm(k|bkz)$")>;

def SPRWriteResGroup320 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup320], (instregex "^VCVT(T?)PH2DQZrr(b?)$",
                                               "^VCVTPH2(PSX|UDQ)Zrr(b?)$",
                                               "^VCVTPH2PSZrr(b?)$",
                                               "^VCVTPS2PH(X?)Zrr(b?)$",
                                               "^VCVTTPH2UDQZrr(b?)$",
                                               "^VPSHUFBITQMBZ(128|256)rrk$",
                                               "^VPSHUFBITQMBZrrk$")>;

def SPRWriteResGroup321 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup321], (instregex "^VCVT(T?)PH2DQZrr(bk|kz)$",
                                               "^VCVT(T?)PH2DQZrr(k|bkz)$",
                                               "^VCVTPH2(PSX|UDQ)Zrr(bk|kz)$",
                                               "^VCVTPH2(PSX|UDQ)Zrr(k|bkz)$",
                                               "^VCVTPS2PHXZrr(bk|kz)$",
                                               "^VCVTPS2PHXZrr(k|bkz)$",
                                               "^VCVTTPH2UDQZrr(bk|kz)$",
                                               "^VCVTTPH2UDQZrr(k|bkz)$")>;

def SPRWriteResGroup322 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1, 2];
  let Latency = 23;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup322], (instregex "^VCVTPH2PDZ128rm(b?)$")>;

def SPRWriteResGroup323 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 1, 2];
  let Latency = 26;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup323], (instregex "^VCVTPH2PDZ128rm(bk|kz)$",
                                               "^VCVTPH2PDZ128rm(k|bkz)$")>;

def SPRWriteResGroup324 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 16;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup324], (instregex "^VCVTPH2PDZ128rr$")>;

def SPRWriteResGroup325 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [2, 1, 1, 2];
  let Latency = 19;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup325], (instregex "^VCVTPH2PDZ128rrk(z?)$")>;

def SPRWriteResGroup326 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 22;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup326], (instregex "^VCVTPH2PDZ256rm(b?)$")>;

def SPRWriteResGroup327 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 25;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup327], (instregex "^VCVTPH2PDZ256rm(bk|kz)$",
                                               "^VCVTPH2PDZ256rm(k|bkz)$")>;

def SPRWriteResGroup328 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup328], (instregex "^VCVTPH2PDZ256rr$")>;

def SPRWriteResGroup329 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup329], (instregex "^VCVTPH2PDZ256rrk(z?)$")>;

def SPRWriteResGroup330 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup330], (instregex "^VCVTPH2PS(X?)Z128rrk(z?)$",
                                               "^VCVTPS2PH(X?)Z128rrk(z?)$")>;

def SPRWriteResGroup331 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 14;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup331], (instregex "^VCVTPH2PSZ(128|256)rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup331, ReadAfterVecLd], (instregex "^VCVTSH2SSZrm_Intk(z?)$")>;
def : InstRW<[SPRWriteResGroup331, ReadAfterVecXLd], (instregex "^VPMADDUBSWZ128rmk(z?)$",
                                                                "^VPMUL(L|HRS)WZ128rmk(z?)$",
                                                                "^VPMULH(U?)WZ128rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup331, ReadAfterVecYLd], (instregex "^VPMADDUBSWZ256rmk(z?)$",
                                                                "^VPMUL(L|HRS)WZ256rmk(z?)$",
                                                                "^VPMULH(U?)WZ256rmk(z?)$")>;

def SPRWriteResGroup332 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup332], (instregex "^VCVTPH2PSZrm$",
                                               "^VCVT(T?)PS2(U?)QQZrm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup332, ReadAfterVecYLd], (instregex "^VPERMWZrmk(z?)$")>;

def SPRWriteResGroup333 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 17;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup333], (instregex "^VCVT(T?)PH2(U?)QQZ128rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup334 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup334], (instregex "^VCVT(T?)PH2(U?)QQZ(128|256)rr((k|kz)?)$")>;

def SPRWriteResGroup335 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2, 1, 1, 1];
  let Latency = 18;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup335], (instregex "^VCVT(T?)PH2(U?)QQZ256rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup336 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup336], (instregex "^VCVTPS2PHXZ128rm(bk|kz)$",
                                               "^VCVTPS2PHXZ128rm(k|bkz)$",
                                               "^VCVTPS2PHXZ256rm(b?)$")>;

def SPRWriteResGroup337 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup337], (instregex "^VCVTPS2PHXZrm(b?)$")>;

def SPRWriteResGroup338 : SchedWriteRes<[SPRPort00_01, SPRPort04_09, SPRPort07_08]> {
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup338], (instregex "^VCVTPS2PHZ(128|256)mrk$")>;

def SPRWriteResGroup339 : SchedWriteRes<[SPRPort00, SPRPort04_09, SPRPort07_08]> {
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup339], (instregex "^VCVTPS2PHZmrk$")>;

def SPRWriteResGroup340 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup340], (instregex "^VCVT(T?)PS2(U?)QQZ128rr((k|kz)?)$",
                                               "^VCVT(U?)QQ2PSZ128rr((k|kz)?)$")>;

def SPRWriteResGroup341 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 15;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup341], (instregex "^VCVT(U?)QQ2PHZ128rm(b?)$")>;

def SPRWriteResGroup342 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 17;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup342], (instregex "^VCVT(U?)QQ2PHZ128rm(bk|kz)$",
                                               "^VCVT(U?)QQ2PHZ128rm(k|bkz)$")>;

def SPRWriteResGroup343 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup343], (instregex "^VCVT(U?)QQ2PHZ128rr$")>;

def SPRWriteResGroup344 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort05]> {
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup344], (instregex "^VCVT(U?)QQ2PHZ128rrk(z?)$",
                                               "^VCVT(U?)QQ2PHZ256rr$")>;

def SPRWriteResGroup345 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 18;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup345], (instregex "^VCVT(U?)QQ2PHZ256rm(b?)$")>;

def SPRWriteResGroup346 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let Latency = 20;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup346], (instregex "^VCVT(U?)QQ2PHZ256rm(bk|kz)$",
                                               "^VCVT(U?)QQ2PHZ256rm(k|bkz)$")>;

def SPRWriteResGroup347 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort05]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup347], (instregex "^VCVT(U?)QQ2PHZ256rrk(z?)$")>;

def SPRWriteResGroup348 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 18;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup348], (instregex "^VCVT(U?)QQ2PHZrm(b?)$")>;

def SPRWriteResGroup349 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 20;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup349], (instregex "^VCVT(U?)QQ2PHZrm(bk|kz)$",
                                               "^VCVT(U?)QQ2PHZrm(k|bkz)$")>;

def SPRWriteResGroup350 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup350], (instregex "^VCVT(U?)QQ2PHZrr(b?)$")>;

def SPRWriteResGroup351 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup351], (instregex "^VCVT(U?)QQ2PHZrr(bk|kz)$",
                                               "^VCVT(U?)QQ2PHZrr(k|bkz)$")>;

def SPRWriteResGroup352 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 2, 1, 1, 1];
  let Latency = 18;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup352, ReadAfterVecLd], (instregex "^VCVTSD2SHZrm_Int$")>;

def SPRWriteResGroup353 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 2, 1, 1, 1];
  let Latency = 21;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup353, ReadAfterVecLd], (instregex "^VCVTSD2SHZrm_Intk(z?)$")>;

def SPRWriteResGroup354 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup354], (instregex "^VCVTSD2SHZrr(b?)_Int$")>;

def SPRWriteResGroup355 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 14;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup355], (instregex "^VCVTSD2SHZrr(b?)_Intk(z?)$")>;

def SPRWriteResGroup356 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 18;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup356, ReadAfterVecLd], (instregex "^VCVTSH2SDZrm_Int$")>;

def SPRWriteResGroup357 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 20;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup357, ReadAfterVecLd], (instregex "^VCVTSH2SDZrm_Intk(z?)$")>;

def SPRWriteResGroup358 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 1];
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup358], (instregex "^VCVTSH2SDZrr(b?)_Int$")>;

def SPRWriteResGroup359 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 1];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup359], (instregex "^VCVTSH2SDZrr(b?)_Intk(z?)$")>;

def SPRWriteResGroup360 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup360, ReadAfterVecLd], (instregex "^VCVT(T?)SH2(U?)SI((64)?)Zrm_Int$")>;

def SPRWriteResGroup361 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 8;
}
def : InstRW<[SPRWriteResGroup361], (instregex "^VCVTSH2SSZrr(b?)_Intk(z?)$")>;

def SPRWriteResGroup362 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup362, ReadAfterVecLd], (instregex "^VCVT(U?)SI((64)?)2SHZrm_Int$",
                                                               "^VCVTSS2SHZrm_Int$")>;

def SPRWriteResGroup363 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup363, ReadAfterVecLd], (instregex "^VCVTSS2SHZrm_Intk(z?)$")>;

def SPRWriteResGroup364 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup364], (instregex "^VCVTSS2SHZrr(b?)_Int$")>;

def SPRWriteResGroup365 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup365], (instregex "^VCVTSS2SHZrr(b?)_Intk(z?)$")>;

def SPRWriteResGroup366 : SchedWriteRes<[SPRPort05]> {
  let Latency = 5;
}
def : InstRW<[SPRWriteResGroup366], (instregex "^VDBPSADBWZ(128|256)rrik(z?)$",
                                               "^VDBPSADBWZrrik(z?)$",
                                               "^VPACK(S|U)S(DW|WB)Z(128|256)rrk(z?)$",
                                               "^VPACK(S|U)S(DW|WB)Zrrk(z?)$",
                                               "^VPBROADCAST(B|W|Dr|Qr|Wr)Z((256)?)rrk(z?)$",
                                               "^VPBROADCAST(B|D|Q|W)rZ(128|256)rr$",
                                               "^VPBROADCASTBrZ(128|256)rrk(z?)$",
                                               "^VPBROADCAST(B|D|Q|W)rZrr$",
                                               "^VPBROADCASTBrZrrk(z?)$",
                                               "^VPBROADCAST(D|Q|W)rZ128rrk(z?)$",
                                               "^VPERMBZ(128|256)rrk(z?)$",
                                               "^VPERMBZrrk(z?)$",
                                               "^VPMOV(S|Z)XBWZ((256)?)rrk(z?)$",
                                               "^VPMULTISHIFTQBZ(128|256)rrk(z?)$",
                                               "^VPMULTISHIFTQBZrrk(z?)$",
                                               "^VPOPCNT(B|W)Z(128|256)rrk(z?)$",
                                               "^VPOPCNT(B|W)Zrrk(z?)$")>;

def SPRWriteResGroup367 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 36;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup367, ReadAfterVecXLd], (instregex "^VDIVPHZ128rm(b?)$")>;

def SPRWriteResGroup368 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 38;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup368, ReadAfterVecXLd], (instregex "^VDIVPHZ128rm(bk|kz)$",
                                                                "^VDIVPHZ128rm(k|bkz)$")>;

def SPRWriteResGroup369 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 31;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup369], (instregex "^VDIVPHZ(128|256)rr$")>;

def SPRWriteResGroup370 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 33;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup370], (instregex "^VDIVPHZ(128|256)rrk$",
                                               "^VDIVPHZ128rrkz$",
                                               "^VSQRTPHZ(128|256)r$")>;

def SPRWriteResGroup371 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 37;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup371, ReadAfterVecYLd], (instregex "^VDIVPHZ256rm(b?)$")>;

def SPRWriteResGroup372 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 39;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup372, ReadAfterVecYLd], (instregex "^VDIVPHZ256rm(bk|kz)$",
                                                                "^VDIVPHZ256rm(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup372, ReadAfterVecXLd], (instregex "^VSQRTPHZ128m(b?)$")>;

def SPRWriteResGroup373 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup373], (instregex "^VDIVPHZ256rrkz$")>;

def SPRWriteResGroup374 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [4, 2, 1, 1, 1];
  let Latency = 49;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup374, ReadAfterVecYLd], (instregex "^VDIVPHZrm(b?)$")>;

def SPRWriteResGroup375 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [4, 2, 1, 1, 1];
  let Latency = 51;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup375, ReadAfterVecYLd], (instregex "^VDIVPHZrm(bk|kz)$",
                                                                "^VDIVPHZrm(k|bkz)$")>;

def SPRWriteResGroup376 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [4, 1, 1];
  let Latency = 41;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup376], (instregex "^VDIVPHZrr(b?)$")>;

def SPRWriteResGroup377 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [4, 1, 1];
  let Latency = 43;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup377], (instregex "^VDIVPHZrr(bk|kz)$",
                                               "^VDIVPHZrr(k|bkz)$")>;

def SPRWriteResGroup378 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 17;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup378], (instregex "^VDIVPSZrr$")>;

def SPRWriteResGroup379 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 21;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup379, ReadAfterVecLd], (instregex "^VDIVSHZrm_Int((k|kz)?)$")>;

def SPRWriteResGroup380 : SchedWriteRes<[SPRPort00]> {
  let Latency = 14;
}
def : InstRW<[SPRWriteResGroup380], (instregex "^VDIVSHZrr_Int$",
                                               "^VSQRTSHZr_Int$")>;

def SPRWriteResGroup381 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 15;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup381, ReadAfterVecXLd], (instregex "^VDPBF16PSZ128m((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup382 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let ResourceCycles = [2, 2];
  let Latency = 8;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup382], (instregex "^VDPBF16PSZ(128|256)r((k|kz)?)$")>;

def SPRWriteResGroup383 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [2, 1, 2];
  let Latency = 16;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup383, ReadAfterVecYLd], (instregex "^VDPBF16PSZ256m((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup384 : SchedWriteRes<[SPRPort00, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [6, 7, 18];
  let Latency = 81;
  let NumMicroOps = 31;
}
def : InstRW<[SPRWriteResGroup384], (instregex "^VERRm$")>;

def SPRWriteResGroup385 : SchedWriteRes<[SPRPort00, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [6, 7, 17];
  let Latency = 74;
  let NumMicroOps = 30;
}
def : InstRW<[SPRWriteResGroup385], (instregex "^VERRr$")>;

def SPRWriteResGroup386 : SchedWriteRes<[SPRPort00, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [5, 8, 21];
  let Latency = 81;
  let NumMicroOps = 34;
}
def : InstRW<[SPRWriteResGroup386], (instregex "^VERWm$")>;

def SPRWriteResGroup387 : SchedWriteRes<[SPRPort00, SPRPort01, SPRPort02_03_11]> {
  let ResourceCycles = [5, 8, 20];
  let Latency = 74;
  let NumMicroOps = 33;
}
def : InstRW<[SPRWriteResGroup387], (instregex "^VERWr$")>;

def SPRWriteResGroup388 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup388, ReadAfterVecYLd], (instregex "^VEXPANDP(D|S)Z128rm((k|kz)?)$",
                                                                "^VPEXPAND(B|D|Q|W)Z128rm$",
                                                                "^VPEXPAND(D|Q)Z128rmk(z?)$")>;

def SPRWriteResGroup389 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup389, ReadAfterVecYLd], (instregex "^VEXPANDP(D|S)Z((256)?)rm((k|kz)?)$",
                                                                "^VPEXPAND(B|D|Q|W)Z((256)?)rm$",
                                                                "^VPEXPAND(D|Q)Z((256)?)rmk(z?)$")>;

def SPRWriteResGroup390 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup390], (instregex "^VF(C?)MADDCPHZ(128|256)m(b?)$",
                                               "^VROUNDP(D|S)Ym$")>;
def : InstRW<[SPRWriteResGroup390, ReadAfterVecXLd], (instregex "^VF(C?)MADDCSHZm$",
                                                                "^VF(C?)MULCPHZ128rm(b?)$",
                                                                "^VF(C?)MULCSHZrm$",
                                                                "^VRNDSCALEPHZ128rm(b?)i$",
                                                                "^VRNDSCALESHZm_Int$",
                                                                "^VSCALEFPHZ128rm(b?)$")>;
def : InstRW<[SPRWriteResGroup390, ReadAfterVecYLd], (instregex "^VF(C?)MULCPHZ256rm(b?)$",
                                                                "^VRNDSCALEP(D|H|S)Z256rm(b?)i$",
                                                                "^VRNDSCALEP(D|S)Z256rm(bik|ikz)$",
                                                                "^VRNDSCALEP(D|S)Z256rmbikz$",
                                                                "^VRNDSCALEP(D|S)Z256rmik$",
                                                                "^VSCALEFPHZ256rm(b?)$")>;
def : InstRW<[SPRWriteResGroup390, ReadAfterVecLd], (instregex "^VSCALEFSHZrm_Int$")>;

def SPRWriteResGroup391 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 21;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup391], (instregex "^VF(C?)MADDCPHZ(128|256)m(bk|kz)$",
                                               "^VF(C?)MADDCPHZ(128|256)m(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup391, ReadAfterVecXLd], (instregex "^VF(C?)MADDCSHZmk(z?)$",
                                                                "^VF(C?)MULCPHZ128rm(bk|kz)$",
                                                                "^VF(C?)MULCPHZ128rm(k|bkz)$",
                                                                "^VF(C?)MULCSHZrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup391, ReadAfterVecYLd], (instregex "^VF(C?)MULCPHZ256rm(bk|kz)$",
                                                                "^VF(C?)MULCPHZ256rm(k|bkz)$")>;

def SPRWriteResGroup392 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [2];
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup392], (instregex "^VF(C?)MADDCPHZ(128|256)r$",
                                               "^VF(C?)MADDCSHZr(b?)$",
                                               "^VF(C?)MULCPHZ(128|256)rr$",
                                               "^VF(C?)MULCSHZrr(b?)$",
                                               "^VRNDSCALEPHZ(128|256)rri$",
                                               "^VRNDSCALESHZr(b?)_Int$",
                                               "^VSCALEFPHZ(128|256)rr$",
                                               "^VSCALEFSHZrr(b?)_Int$")>;

def SPRWriteResGroup393 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [2];
  let Latency = 15;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup393], (instregex "^VF(C?)MADDCPHZ(128|256)rk(z?)$",
                                               "^VF(C?)MADDCSHZr(bk|kz)$",
                                               "^VF(C?)MADDCSHZr(k|bkz)$",
                                               "^VF(C?)MULCPHZ(128|256)rrk(z?)$",
                                               "^VF(C?)MULCSHZrr(bk|kz)$",
                                               "^VF(C?)MULCSHZrr(k|bkz)$")>;

def SPRWriteResGroup394 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup394], (instregex "^VF(C?)MADDCPHZm(b?)$")>;
def : InstRW<[SPRWriteResGroup394, ReadAfterVecYLd], (instregex "^VF(C?)MULCPHZrm(b?)$",
                                                                "^VRNDSCALEP(D|H|S)Zrm(b?)i$",
                                                                "^VRNDSCALEP(D|S)Zrm(bik|ikz)$",
                                                                "^VRNDSCALEP(D|S)Zrmbikz$",
                                                                "^VRNDSCALEP(D|S)Zrmik$",
                                                                "^VSCALEFPHZrm(b?)$")>;

def SPRWriteResGroup395 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 21;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup395], (instregex "^VF(C?)MADDCPHZm(bk|kz)$",
                                               "^VF(C?)MADDCPHZm(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup395, ReadAfterVecYLd], (instregex "^VF(C?)MULCPHZrm(bk|kz)$",
                                                                "^VF(C?)MULCPHZrm(k|bkz)$")>;

def SPRWriteResGroup396 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [2];
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup396], (instregex "^VF(C?)MADDCPHZr(b?)$",
                                               "^VF(C?)MULCPHZrr(b?)$",
                                               "^VRNDSCALEPHZrri(b?)$",
                                               "^VSCALEFPHZrr(b?)$")>;

def SPRWriteResGroup397 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [2];
  let Latency = 15;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup397], (instregex "^VF(C?)MADDCPHZr(bk|kz)$",
                                               "^VF(C?)MADDCPHZr(k|bkz)$",
                                               "^VF(C?)MULCPHZrr(bk|kz)$",
                                               "^VF(C?)MULCPHZrr(k|bkz)$")>;

def SPRWriteResGroup398 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 29;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup398, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Yrm$",
                                                                              "^VGATHERQPDYrm$",
                                                                              "^VPGATHER(D|Q)QYrm$",
                                                                              "^VPGATHERQDYrm$")>;

def SPRWriteResGroup399 : SchedWriteRes<[SPRPort00, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 20;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup399, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Z128rm$",
                                                                              "^VGATHERQPDZ128rm$",
                                                                              "^VPGATHER(D|Q)QZ128rm$",
                                                                              "^VPGATHERQDZ128rm$")>;

def SPRWriteResGroup400 : SchedWriteRes<[SPRPort00, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 2, 4];
  let Latency = 28;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup400, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Z256rm$",
                                                                              "^VGATHERQPDZ256rm$",
                                                                              "^VPGATHER(D|Q)QZ256rm$",
                                                                              "^VPGATHERQDZ256rm$")>;

def SPRWriteResGroup401 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 8, 2];
  let Latency = 28;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup401, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)Zrm$",
                                                                              "^VGATHERQPDZrm$",
                                                                              "^VPGATHER(D|Q)QZrm$",
                                                                              "^VPGATHERQDZrm$")>;

def SPRWriteResGroup402 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 1, 1, 2];
  let Latency = 20;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup402, WriteVecMaskedGatherWriteback], (instregex "^VGATHER(DPD|QPS)rm$",
                                                                              "^VGATHERQPDrm$",
                                                                              "^VPGATHER(D|Q)Qrm$",
                                                                              "^VPGATHERQDrm$")>;

def SPRWriteResGroup403 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 8];
  let Latency = 30;
  let NumMicroOps = 12;
}
def : InstRW<[SPRWriteResGroup403, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSYrm$",
                                                                              "^VPGATHERDDYrm$")>;

def SPRWriteResGroup404 : SchedWriteRes<[SPRPort00, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 2, 4];
  let Latency = 27;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup404, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSZ128rm$",
                                                                              "^VPGATHERDDZ128rm$")>;

def SPRWriteResGroup405 : SchedWriteRes<[SPRPort00, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 2, 8];
  let Latency = 29;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup405, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSZ256rm$",
                                                                              "^VPGATHERDDZ256rm$")>;

def SPRWriteResGroup406 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 16, 2];
  let Latency = 30;
  let NumMicroOps = 19;
}
def : InstRW<[SPRWriteResGroup406, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSZrm$",
                                                                              "^VPGATHERDDZrm$")>;

def SPRWriteResGroup407 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 1, 2, 4];
  let Latency = 28;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup407, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDPSrm$",
                                                                              "^VPGATHERDDrm$")>;

def SPRWriteResGroup408 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 15;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup408, ReadAfterVecXLd], (instregex "^VGF2P8AFFINE((INV)?)QBZ128rm(bik|ikz)$",
                                                                "^VGF2P8AFFINE((INV)?)QBZ128rmbikz$",
                                                                "^VGF2P8AFFINE((INV)?)QBZ128rmik$",
                                                                "^VGF2P8MULBZ128rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup408, ReadAfterVecYLd], (instregex "^VGF2P8AFFINE((INV)?)QBZ256rm(bik|ikz)$",
                                                                "^VGF2P8AFFINE((INV)?)QBZ256rmbikz$",
                                                                "^VGF2P8AFFINE((INV)?)QBZ256rmik$",
                                                                "^VGF2P8MULBZ256rmk(z?)$")>;

def SPRWriteResGroup409 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 9;
}
def : InstRW<[SPRWriteResGroup409], (instregex "^VGF2P8AFFINE((INV)?)QBZ(128|256)rrik$",
                                               "^VGF2P8MULBZ(128|256)rrk$")>;

def SPRWriteResGroup410 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 10;
}
def : InstRW<[SPRWriteResGroup410], (instregex "^VGF2P8AFFINE((INV)?)QBZ(128|256)rrikz$",
                                               "^VGF2P8MULBZ(128|256)rrkz$")>;

def SPRWriteResGroup411 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 15;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup411, ReadAfterVecYLd], (instregex "^VGF2P8AFFINE((INV)?)QBZrm(bik|ikz)$",
                                                                "^VGF2P8AFFINE((INV)?)QBZrmbikz$",
                                                                "^VGF2P8AFFINE((INV)?)QBZrmik$",
                                                                "^VGF2P8MULBZrmk(z?)$")>;

def SPRWriteResGroup412 : SchedWriteRes<[SPRPort00]> {
  let Latency = 9;
}
def : InstRW<[SPRWriteResGroup412], (instregex "^VGF2P8AFFINE((INV)?)QBZrrik$",
                                               "^VGF2P8MULBZrrk$")>;

def SPRWriteResGroup413 : SchedWriteRes<[SPRPort00]> {
  let Latency = 10;
}
def : InstRW<[SPRWriteResGroup413], (instregex "^VGF2P8AFFINE((INV)?)QBZrrikz$",
                                               "^VGF2P8MULBZrrkz$")>;

def SPRWriteResGroup414 : SchedWriteRes<[SPRPort01_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup414], (instregex "^VH(ADD|SUB)P(D|S)rr$")>;

def SPRWriteResGroup415 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort02_03_11]> {
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup415], (instregex "^VLDMXCSR$")>;

def SPRWriteResGroup416 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort01, SPRPort01_05, SPRPort02_03, SPRPort02_03_07, SPRPort04, SPRPort05, SPRPort06]> {
  let ResourceCycles = [8, 1, 1, 1, 1, 1, 2, 3];
  let Latency = 40;
  let NumMicroOps = 18;
}
def : InstRW<[SPRWriteResGroup416], (instregex "^VMCLEARm$")>;

def SPRWriteResGroup417 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup417], (instregex "^VMOVDQU(8|16)Z(128|256)rmk(z?)$",
                                               "^VMOVSHZrmk(z?)$")>;
def : InstRW<[SPRWriteResGroup417, ReadAfterVecXLd], (instregex "^VP(ADD|SUB)(B|W)Z128rmk(z?)$",
                                                                "^VPBLENDM(B|W)Z128rmk(z?)$")>;
def : InstRW<[SPRWriteResGroup417, ReadAfterVecYLd], (instregex "^VP(ADD|SUB)(B|W)Z256rmk(z?)$",
                                                                "^VPBLENDM(B|W)Z256rmk(z?)$")>;

def SPRWriteResGroup418 : SchedWriteRes<[SPRPort00_01_05]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup418], (instregex "^VMOVDQU(8|16)Z(128|256)rrk(z?)$",
                                               "^VMOVDQU(8|16)Z(128|256)rrk(z?)_REV$",
                                               "^VMOVSHZrrk(z?)$",
                                               "^VMOVSHZrrk(z?)_REV$",
                                               "^VP(ADD|SUB)(B|W)Z(128|256)rrk(z?)$",
                                               "^VPBLENDM(B|W)Z(128|256)rrk(z?)$",
                                               "^VPMOVM2(B|W)Z(128|256)rr$")>;

def SPRWriteResGroup419 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 2, 2];
  let Latency = 12;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup419], (instregex "^VMOVDQU8Zmrk$")>;

def SPRWriteResGroup420 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 477;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup420], (instregex "^VMOVNTDQZ128mr$")>;

def SPRWriteResGroup421 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 470;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup421], (instregex "^VMOVNTDQZ256mr$",
                                               "^VMOVNTPSmr$")>;

def SPRWriteResGroup422 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 473;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup422], (instregex "^VMOVNT(PD|DQZ)mr$")>;

def SPRWriteResGroup423 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 521;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup423], (instregex "^VMOVNTDQmr$")>;

def SPRWriteResGroup424 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 550;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup424], (instregex "^VMOVNTPDZ128mr$")>;

def SPRWriteResGroup425 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 474;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup425], (instregex "^VMOVNTPDZ256mr$")>;

def SPRWriteResGroup426 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 464;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup426], (instregex "^VMOVNTPDZmr$")>;

def SPRWriteResGroup427 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 494;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup427], (instregex "^VMOVNTPSYmr$")>;

def SPRWriteResGroup428 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 475;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup428], (instregex "^VMOVNTPSZ128mr$")>;

def SPRWriteResGroup429 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 476;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup429], (instregex "^VMOVNTPSZ256mr$")>;

def SPRWriteResGroup430 : SchedWriteRes<[SPRPort04_09, SPRPort07_08]> {
  let Latency = 471;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup430], (instregex "^VMOVNTPSZmr$")>;

def SPRWriteResGroup431 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [3, 1, 8];
  let Latency = 10;
  let NumMicroOps = 12;
}
def : InstRW<[SPRWriteResGroup431, ReadAfterVecXLd], (instregex "^VP2INTERSECTDZ128rm(b?)$")>;
def : InstRW<[SPRWriteResGroup431, ReadAfterVecYLd], (instregex "^VP2INTERSECTQZ256rm(b?)$")>;

def SPRWriteResGroup432 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [4, 8];
  let Latency = 10;
  let NumMicroOps = 12;
}
def : InstRW<[SPRWriteResGroup432], (instregex "^VP2INTERSECTDZ128rr$",
                                               "^VP2INTERSECTQZ256rr$")>;

def SPRWriteResGroup433 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 8, 7, 2, 1, 11];
  let Latency = 27;
  let NumMicroOps = 30;
}
def : InstRW<[SPRWriteResGroup433, ReadAfterVecYLd], (instregex "^VP2INTERSECTDZ256rm(b?)$")>;

def SPRWriteResGroup434 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05, SPRPort01_05, SPRPort05]> {
  let ResourceCycles = [1, 8, 8, 2, 11];
  let Latency = 27;
  let NumMicroOps = 30;
}
def : InstRW<[SPRWriteResGroup434], (instregex "^VP2INTERSECTDZ256rr$")>;

def SPRWriteResGroup435 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [13, 9, 1, 23];
  let Latency = 40;
  let NumMicroOps = 46;
}
def : InstRW<[SPRWriteResGroup435, ReadAfterVecYLd], (instregex "^VP2INTERSECTDZrm(b?)$")>;

def SPRWriteResGroup436 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [13, 10, 23];
  let Latency = 40;
  let NumMicroOps = 46;
}
def : InstRW<[SPRWriteResGroup436], (instregex "^VP2INTERSECTDZrr$")>;

def SPRWriteResGroup437 : SchedWriteRes<[SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 4];
  let Latency = 6;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup437, ReadAfterVecXLd], (instregex "^VP2INTERSECTQZ128rm(b?)$")>;

def SPRWriteResGroup438 : SchedWriteRes<[SPRPort05]> {
  let ResourceCycles = [4];
  let Latency = 6;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup438], (instregex "^VP2INTERSECTQZ128rr$")>;

def SPRWriteResGroup439 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [8, 7, 1, 14];
  let Latency = 29;
  let NumMicroOps = 30;
}
def : InstRW<[SPRWriteResGroup439, ReadAfterVecYLd], (instregex "^VP2INTERSECTQZrm(b?)$")>;

def SPRWriteResGroup440 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [8, 8, 14];
  let Latency = 30;
  let NumMicroOps = 30;
}
def : InstRW<[SPRWriteResGroup440], (instregex "^VP2INTERSECTQZrr$")>;

def SPRWriteResGroup441 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 3;
}
def : InstRW<[SPRWriteResGroup441], (instregex "^VP(A|SU)BS(B|W)Z(128|256)rrk(z?)$",
                                               "^VP(ADD|MAX|MIN)S(B|W)Z(128|256)rrk(z?)$",
                                               "^VP(ADD|SUB)US(B|W)Z(128|256)rrk(z?)$",
                                               "^VPAVG(B|W)Z(128|256)rrk(z?)$",
                                               "^VPM(AX|IN)U(B|W)Z(128|256)rrk(z?)$",
                                               "^VPSH(L|R)DVWZ(128|256)rk(z?)$",
                                               "^VPS(L|R)LVWZ(128|256)rrk(z?)$",
                                               "^VPS(L|R)LWZ(128|256)rik(z?)$",
                                               "^VPSRAVWZ(128|256)rrk(z?)$",
                                               "^VPSRAWZ(128|256)rik(z?)$")>;

def SPRWriteResGroup442 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup442], (instregex "^VPABS(B|W)Zrmk(z?)$",
                                               "^VPS(L|R)LWZmik(z?)$",
                                               "^VPSRAWZmik(z?)$")>;
def : InstRW<[SPRWriteResGroup442, ReadAfterVecYLd], (instregex "^VP(ADD|MAX|MIN|SUB)S(B|W)Zrmk(z?)$",
                                                                "^VP(ADD|SUB)US(B|W)Zrmk(z?)$",
                                                                "^VPAVG(B|W)Zrmk(z?)$",
                                                                "^VPM(AX|IN)U(B|W)Zrmk(z?)$",
                                                                "^VPSH(L|R)DVWZmk(z?)$",
                                                                "^VPS(RA|RL|LLV)WZrmk(z?)$",
                                                                "^VPS(LL|RAV|RLV)WZrmk(z?)$")>;

def SPRWriteResGroup443 : SchedWriteRes<[SPRPort01_05, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup443, ReadAfterVecYLd], (instregex "^VPBLENDWYrmi$",
                                                                "^VSHUFP(D|S)Yrmi$",
                                                                "^VSHUFP(D|S)Z256rm(bi|ik|ikz)$",
                                                                "^VSHUFP(D|S)Z256rm(i|bik)$",
                                                                "^VSHUFP(D|S)Z256rmbikz$")>;

def SPRWriteResGroup444 : SchedWriteRes<[SPRPort00, SPRPort05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup444], (instregex "^VPBROADCASTM(B2Q|W2D)Z(128|256)rr$",
                                               "^VPBROADCASTM(B2Q|W2D)Zrr$",
                                               "^VP(ERM|SLL|SRA|SRL)WZrrk(z?)$",
                                               "^VPSHUFBITQMBZ(128|256)rr$",
                                               "^VPSHUFBITQMBZrr$")>;

def SPRWriteResGroup445 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 2, 1];
  let Latency = 12;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup445], (instregex "^VPCOMPRESS(B|W)Z(128|256)mr$",
                                               "^VPCOMPRESSWZmr$")>;

def SPRWriteResGroup446 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 2, 1];
  let Latency = 14;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup446], (instregex "^VPCOMPRESS(B|W)Z(128|256)mrk$",
                                               "^VPCOMPRESSWZmrk$")>;

def SPRWriteResGroup447 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 2, 2, 2];
  let Latency = 12;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup447], (instregex "^VPCOMPRESSBZmr$")>;

def SPRWriteResGroup448 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 2, 2, 2];
  let Latency = 14;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup448], (instregex "^VPCOMPRESSBZmrk$")>;

def SPRWriteResGroup449 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [5, 4, 1, 5];
  let Latency = 17;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup449], (instregex "^VPCONFLICTDZ128rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup450 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [5, 5, 5];
  let Latency = 12;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup450], (instregex "^VPCONFLICTDZ128rr((k|kz)?)$")>;

def SPRWriteResGroup451 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 5, 1, 1, 9];
  let Latency = 24;
  let NumMicroOps = 23;
}
def : InstRW<[SPRWriteResGroup451], (instregex "^VPCONFLICTDZ256rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup452 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [7, 6, 1, 9];
  let Latency = 17;
  let NumMicroOps = 23;
}
def : InstRW<[SPRWriteResGroup452], (instregex "^VPCONFLICTDZ256rr((k|kz)?)$")>;

def SPRWriteResGroup453 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [11, 8, 1, 17];
  let Latency = 33;
  let NumMicroOps = 37;
}
def : InstRW<[SPRWriteResGroup453], (instregex "^VPCONFLICTDZrm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup454 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [11, 9, 17];
  let Latency = 26;
  let NumMicroOps = 37;
}
def : InstRW<[SPRWriteResGroup454], (instregex "^VPCONFLICTDZrr((kz)?)$")>;

def SPRWriteResGroup455 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [11, 9, 17];
  let Latency = 25;
  let NumMicroOps = 37;
}
def : InstRW<[SPRWriteResGroup455], (instregex "^VPCONFLICTDZrrk$")>;

def SPRWriteResGroup456 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 11;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup456], (instregex "^VPCONFLICTQZ128rm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup456, ReadAfterVecYLd], (instregex "^VPERM(I|T)2B128rm$")>;

def SPRWriteResGroup457 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 4;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup457], (instregex "^VPCONFLICTQZ128rr((k|kz)?)$")>;

def SPRWriteResGroup458 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [5, 4, 1, 5];
  let Latency = 20;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup458], (instregex "^VPCONFLICTQZ256rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup459 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [5, 5, 5];
  let Latency = 13;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup459], (instregex "^VPCONFLICTQZ256rr((k|kz)?)$")>;

def SPRWriteResGroup460 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [7, 5, 1, 9];
  let Latency = 23;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup460], (instregex "^VPCONFLICTQZrm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup461 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [7, 6, 9];
  let Latency = 17;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup461], (instregex "^VPCONFLICTQZrr((kz)?)$")>;

def SPRWriteResGroup462 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [7, 6, 9];
  let Latency = 16;
  let NumMicroOps = 22;
}
def : InstRW<[SPRWriteResGroup462], (instregex "^VPCONFLICTQZrrk$")>;

def SPRWriteResGroup463 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup463], (instregex "^VPDP(BU|WS)SD((SY)?)rm$",
                                               "^VPDP(BU|WS)SD(S|Y)rm$")>;

def SPRWriteResGroup464 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup464, ReadAfterVecYLd], (instregex "^VPERM(I|T)2B128rmk(z?)$",
                                                                "^VPERMT2W128rm$")>;

def SPRWriteResGroup465 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup465], (instregex "^VPERM(I|T)2B(128|256)rr$")>;

def SPRWriteResGroup466 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup466], (instregex "^VPERM(I|T)2B(128|256)rrk(z?)$",
                                               "^VPERM(I|T)2W(128|256)rr$")>;

def SPRWriteResGroup467 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup467, ReadAfterVecYLd], (instregex "^VPERM(I|T)2B256rm$")>;

def SPRWriteResGroup468 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 14;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup468, ReadAfterVecYLd], (instregex "^VPERM(I|T)2B256rmk(z?)$",
                                                                "^VPERMI2W128rm$",
                                                                "^VPERMT2W256rm$")>;

def SPRWriteResGroup469 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup469, ReadAfterVecYLd], (instregex "^VPERM(I|T)2Brm$")>;

def SPRWriteResGroup470 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 14;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup470, ReadAfterVecYLd], (instregex "^VPERM(I|T)2Brmk(z?)$",
                                                                "^VPERMT2Wrm$")>;

def SPRWriteResGroup471 : SchedWriteRes<[SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 5;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup471], (instregex "^VPERM(I|T)2Brr$")>;

def SPRWriteResGroup472 : SchedWriteRes<[SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup472], (instregex "^VPERM(I|T)2Brrk(z?)$",
                                               "^VPERM(I|T)2Wrr$")>;

def SPRWriteResGroup473 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup473, ReadAfterVecYLd], (instregex "^VPERMI2W128rmk(z?)$",
                                                                "^VPERMT2W256rmk(z?)$")>;

def SPRWriteResGroup474 : SchedWriteRes<[SPRPort00_01_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup474], (instregex "^VPERM(I|T)2W(128|256)rrk(z?)$")>;

def SPRWriteResGroup475 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup475, ReadAfterVecYLd], (instregex "^VPERMI2W256rm$",
                                                                "^VPERMT2W128rmk(z?)$")>;

def SPRWriteResGroup476 : SchedWriteRes<[SPRPort00_01_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 17;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup476, ReadAfterVecYLd], (instregex "^VPERMI2W256rmk(z?)$")>;

def SPRWriteResGroup477 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup477, ReadAfterVecYLd], (instregex "^VPERMI2Wrm$")>;

def SPRWriteResGroup478 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 17;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup478, ReadAfterVecYLd], (instregex "^VPERMI2Wrmk(z?)$")>;

def SPRWriteResGroup479 : SchedWriteRes<[SPRPort00_05, SPRPort05]> {
  let ResourceCycles = [1, 2];
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup479], (instregex "^VPERM(I|T)2Wrrk(z?)$")>;

def SPRWriteResGroup480 : SchedWriteRes<[SPRPort00_05, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [1, 1, 2];
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup480, ReadAfterVecYLd], (instregex "^VPERMT2Wrmk(z?)$")>;

def SPRWriteResGroup481 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup481, ReadAfterVecYLd], (instregex "^VPERMWZ128rm$")>;

def SPRWriteResGroup482 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup482, ReadAfterVecYLd], (instregex "^VPERMWZ(128|256)rmk(z?)$")>;

def SPRWriteResGroup483 : SchedWriteRes<[SPRPort00_01, SPRPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup483], (instregex "^VPERMWZ(128|256)rr$")>;

def SPRWriteResGroup484 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11, SPRPort05]> {
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup484, ReadAfterVecYLd], (instregex "^VPERMWZ256rm$")>;

def SPRWriteResGroup485 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup485, ReadAfterVecYLd], (instregex "^VPERMWZrm$")>;

def SPRWriteResGroup486 : SchedWriteRes<[SPRPort05]> {
  let ResourceCycles = [2];
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup486], (instregex "^VPEXPAND(B|W)Z(128|256)rrk(z?)$",
                                               "^VPEXPAND(B|W)Zrrk(z?)$")>;

def SPRWriteResGroup487 : SchedWriteRes<[SPRPort00_01, SPRPort01_05, SPRPort02_03_11]> {
  let ResourceCycles = [1, 2, 1];
  let Latency = 10;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup487, ReadAfterVecYLd], (instregex "^VPH(ADD|SUB)SWYrm$")>;

def SPRWriteResGroup488 : SchedWriteRes<[SPRPort00_01]> {
  let Latency = 7;
}
def : InstRW<[SPRWriteResGroup488], (instregex "^VPMADDUBSWZ(128|256)rrk(z?)$",
                                               "^VPMUL(L|HRS)WZ(128|256)rrk(z?)$",
                                               "^VPMULH(U?)WZ(128|256)rrk(z?)$")>;

def SPRWriteResGroup489 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 14;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup489, ReadAfterVecYLd], (instregex "^VPMADDUBSWZrmk(z?)$",
                                                                "^VPMUL(L|HRS)WZrmk(z?)$",
                                                                "^VPMULH(U?)WZrmk(z?)$")>;

def SPRWriteResGroup490 : SchedWriteRes<[SPRPort00]> {
  let Latency = 7;
}
def : InstRW<[SPRWriteResGroup490], (instregex "^VPMADDUBSWZrrk(z?)$",
                                               "^VPMUL(L|HRS)WZrrk(z?)$",
                                               "^VPMULH(U?)WZrrk(z?)$")>;

def SPRWriteResGroup491 : SchedWriteRes<[SPRPort01_05, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 12;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup491], (instregex "^VPMOV(DB|QW|SDW|SQD|SQW)Z(128|256)mr$",
                                               "^VPMOV(D|USQ)WZ(128|256)mr$",
                                               "^VPMOV(Q|W|SD|SW|USD|USW)BZ256mr$",
                                               "^VPMOV(W|SD|USW)BZ128mr$",
                                               "^VPMOV(U?)SQBZ256mr$",
                                               "^VPMOV(SW|USD)BZ128mr$",
                                               "^VPMOVUS(DW|QD)Z(128|256)mr$")>;

def SPRWriteResGroup492 : SchedWriteRes<[SPRPort01_05, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 13;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup492], (instregex "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ128mrk$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ128mrk$",
                                               "^VPMOV(U?)S(DB|QD)Z128mrk$",
                                               "^VPMOV(U?)SDWZ128mrk$")>;

def SPRWriteResGroup493 : SchedWriteRes<[SPRPort01_05, SPRPort05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup493], (instregex "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ128rr$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ128rr$",
                                               "^VPMOV(U?)S(DB|QD)Z128rr$",
                                               "^VPMOV(U?)SDWZ128rr$",
                                               "^VPMOV(U?)SQDZ128rrk(z?)$")>;

def SPRWriteResGroup494 : SchedWriteRes<[SPRPort01_05, SPRPort05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup494], (instregex "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ128rrk(z?)$",
                                               "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ256rr$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ128rrk(z?)$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ256rr$",
                                               "^VPMOV(U?)SD(B|W)Z128rrk(z?)$",
                                               "^VPMOV(U?)S(DB|QD)Z256rr$",
                                               "^VPMOV(U?)SDWZ256rr$",
                                               "^VPMOV(U?)SQDZ256rrk(z?)$")>;

def SPRWriteResGroup495 : SchedWriteRes<[SPRPort01_05, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 15;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup495], (instregex "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ256mrk$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ256mrk$",
                                               "^VPMOV(U?)S(DB|QD)Z256mrk$",
                                               "^VPMOV(U?)SDWZ256mrk$")>;

def SPRWriteResGroup496 : SchedWriteRes<[SPRPort01_05, SPRPort05]> {
  let Latency = 6;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup496], (instregex "^VPMOV(D|Q|W|SQ|SW|USQ|USW)BZ256rrk(z?)$",
                                               "^VPMOV(D|Q|SQ|USQ)WZ256rrk(z?)$",
                                               "^VPMOV(U?)SD(B|W)Z256rrk(z?)$")>;

def SPRWriteResGroup497 : SchedWriteRes<[SPRPort01_05, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 20;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup497], (instregex "^VPMOV((S|US)?)QBZ128mr$")>;

def SPRWriteResGroup498 : SchedWriteRes<[SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let Latency = 14;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup498], (instregex "^VPMOVQDZ((256)?)mrk$")>;

def SPRWriteResGroup499 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 23;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup499, ReadAfterVecXLd], (instregex "^VPMULLQZ128rm((b|k|bk|kz|bkz)?)$")>;
def : InstRW<[SPRWriteResGroup499, ReadAfterVecYLd], (instregex "^VPMULLQZ256rm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup500 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [3];
  let Latency = 15;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup500], (instregex "^VPMULLQZ(128|256)rr((k|kz)?)$")>;

def SPRWriteResGroup501 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 23;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup501, ReadAfterVecYLd], (instregex "^VPMULLQZrm((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup502 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [3];
  let Latency = 15;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup502], (instregex "^VPMULLQZrr((k|kz)?)$")>;

def SPRWriteResGroup503 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 4, 4];
  let Latency = 12;
  let NumMicroOps = 11;
}
def : InstRW<[SPRWriteResGroup503], (instregex "^VPSCATTERDDZ128mr$",
                                               "^VPSCATTER(D|Q)QZ256mr$",
                                               "^VPSCATTERQDZ256mr$",
                                               "^VSCATTER(DPD|QPS)Z256mr$",
                                               "^VSCATTERDPSZ128mr$",
                                               "^VSCATTERQPDZ256mr$")>;

def SPRWriteResGroup504 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 8, 8];
  let Latency = 12;
  let NumMicroOps = 19;
}
def : InstRW<[SPRWriteResGroup504], (instregex "^VPSCATTERDDZ256mr$",
                                               "^VSCATTERDPSZ256mr$")>;

def SPRWriteResGroup505 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 16, 16];
  let Latency = 19;
  let NumMicroOps = 35;
}
def : InstRW<[SPRWriteResGroup505], (instregex "^VPSCATTERDDZmr$",
                                               "^VSCATTERDPSZmr$")>;

def SPRWriteResGroup506 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [1, 1, 1, 2, 2];
  let Latency = 12;
  let NumMicroOps = 7;
}
def : InstRW<[SPRWriteResGroup506], (instregex "^VPSCATTER(D|Q)QZ128mr$",
                                               "^VPSCATTERQDZ128mr$",
                                               "^VSCATTER(DPD|QPS)Z128mr$",
                                               "^VSCATTERQPDZ128mr$")>;

def SPRWriteResGroup507 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06_10, SPRPort04_09, SPRPort07_08]> {
  let ResourceCycles = [2, 1, 8, 8];
  let Latency = 12;
  let NumMicroOps = 19;
}
def : InstRW<[SPRWriteResGroup507], (instregex "^VPSCATTER(D|Q)QZmr$",
                                               "^VPSCATTERQDZmr$",
                                               "^VSCATTER(DPD|QPS)Zmr$",
                                               "^VSCATTERQPDZmr$")>;

def SPRWriteResGroup508 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup508, ReadAfterVecXLd], (instregex "^VPSH(LDD|RDQ)Z128rmbi$",
                                                                "^VPSH(LDD|RDQ|RDW)Z128rmi$",
                                                                "^VPSH(LDQ|RDD)Z128rm(b?)i$",
                                                                "^VPSH(L|R)DV(D|Q|W)Z128m$",
                                                                "^VPSH(L|R)DV(D|Q)Z128m(b|k|kz)$",
                                                                "^VPSH(L|R)DV(D|Q)Z128mbk(z?)$",
                                                                "^VPSHLDWZ128rmi$")>;

def SPRWriteResGroup509 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup509, ReadAfterVecXLd], (instregex "^VPSH(LDD|RDQ)Z128rm(bik|ikz)$",
                                                                "^VPSH(LDD|RDQ)Z128rmbikz$",
                                                                "^VPSH(LDD|RDQ)Z128rmik$",
                                                                "^VPSH(LDQ|RDD)Z128rm(bik|ikz)$",
                                                                "^VPSH(LDQ|RDD)Z128rmbikz$",
                                                                "^VPSH(LDQ|RDD)Z128rmik$")>;

def SPRWriteResGroup510 : SchedWriteRes<[SPRPort00_01]>;
def : InstRW<[SPRWriteResGroup510], (instregex "^VPSH(LDD|RDQ|RDW)Z(128|256)rri$",
                                               "^VPSH(LDQ|RDD)Z(128|256)rri$",
                                               "^VPSH(L|R)DV(D|Q|W)Z(128|256)r$",
                                               "^VPSH(L|R)DV(D|Q)Z(128|256)rk(z?)$",
                                               "^VPSHLDWZ(128|256)rri$")>;

def SPRWriteResGroup511 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup511], (instregex "^VPSH(LDD|RDQ)Z(128|256)rrik(z?)$",
                                               "^VPSH(LDQ|RDD)Z(128|256)rrik(z?)$")>;

def SPRWriteResGroup512 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup512, ReadAfterVecYLd], (instregex "^VPSH(LDD|RDQ)Z256rmbi$",
                                                                "^VPSH(LDD|RDQ|RDW)Z256rmi$",
                                                                "^VPSH(LDQ|RDD)Z256rm(b?)i$",
                                                                "^VPSH(L|R)DV(D|Q|W)Z256m$",
                                                                "^VPSH(L|R)DV(D|Q)Z256m(b|k|kz)$",
                                                                "^VPSH(L|R)DV(D|Q)Z256mbk(z?)$",
                                                                "^VPSHLDWZ256rmi$")>;

def SPRWriteResGroup513 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup513, ReadAfterVecYLd], (instregex "^VPSH(LDD|RDQ)Z256rm(bik|ikz)$",
                                                                "^VPSH(LDD|RDQ)Z256rmbikz$",
                                                                "^VPSH(LDD|RDQ)Z256rmik$",
                                                                "^VPSH(LDQ|RDD)Z256rm(bik|ikz)$",
                                                                "^VPSH(LDQ|RDD)Z256rmbikz$",
                                                                "^VPSH(LDQ|RDD)Z256rmik$")>;

def SPRWriteResGroup514 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 9;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup514, ReadAfterVecYLd], (instregex "^VPSH(LDD|RDQ)Zrmbi$",
                                                                "^VPSH(LDD|RDQ|RDW)Zrmi$",
                                                                "^VPSH(LDQ|RDD)Zrm(b?)i$",
                                                                "^VPSH(L|R)DV(D|Q|W)Zm$",
                                                                "^VPSH(L|R)DV(D|Q)Zm(b|k|kz)$",
                                                                "^VPSH(L|R)DV(D|Q)Zmbk(z?)$",
                                                                "^VPSHLDWZrmi$")>;

def SPRWriteResGroup515 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11]> {
  let Latency = 10;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup515, ReadAfterVecYLd], (instregex "^VPSH(LDD|RDQ)Zrm(bik|ikz)$",
                                                                "^VPSH(LDD|RDQ)Zrmbikz$",
                                                                "^VPSH(LDD|RDQ)Zrmik$",
                                                                "^VPSH(LDQ|RDD)Zrm(bik|ikz)$",
                                                                "^VPSH(LDQ|RDD)Zrmbikz$",
                                                                "^VPSH(LDQ|RDD)Zrmik$")>;

def SPRWriteResGroup516 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup516], (instregex "^VPSH(LDD|RDQ)Zrrik(z?)$",
                                               "^VPSH(LDQ|RDD)Zrrik(z?)$")>;

def SPRWriteResGroup517 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 11;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup517, ReadAfterVecXLd], (instregex "^VPSH(L|R)DWZ128rmik(z?)$")>;

def SPRWriteResGroup518 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup518], (instregex "^VPSH(L|R)DWZ(128|256)rrik(z?)$")>;

def SPRWriteResGroup519 : SchedWriteRes<[SPRPort00_01, SPRPort00_01_05, SPRPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup519, ReadAfterVecYLd], (instregex "^VPSH(L|R)DWZ256rmik(z?)$")>;

def SPRWriteResGroup520 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11]> {
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup520, ReadAfterVecYLd], (instregex "^VPSH(L|R)DWZrmik(z?)$")>;

def SPRWriteResGroup521 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup521], (instregex "^VPSH(L|R)DWZrrik(z?)$")>;

def SPRWriteResGroup522 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup522, ReadAfterVecXLd], (instregex "^VPSHUFBITQMBZ128rm$")>;
def : InstRW<[SPRWriteResGroup522, ReadAfterVecYLd], (instregex "^VPSHUFBITQMBZ((256)?)rm$")>;

def SPRWriteResGroup523 : SchedWriteRes<[SPRPort00, SPRPort02_03_11, SPRPort05]> {
  let Latency = 8;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup523, ReadAfterVecXLd], (instregex "^VPSHUFBITQMBZ128rmk$")>;
def : InstRW<[SPRWriteResGroup523, ReadAfterVecYLd], (instregex "^VPSHUFBITQMBZ((256)?)rmk$")>;

def SPRWriteResGroup524 : SchedWriteRes<[SPRPort00_01, SPRPort01_05]> {
  let Latency = 4;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup524], (instregex "^VPS(L|R)LWZ128rrk(z?)$",
                                               "^VPSRAWZ128rrk(z?)$")>;

def SPRWriteResGroup525 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 16;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup525, ReadAfterVecYLd], (instregex "^VRCPPHZm(bk|kz)$",
                                                                "^VRCPPHZm(k|bkz)$",
                                                                "^VRSQRTPHZm(bk|kz)$",
                                                                "^VRSQRTPHZm(k|bkz)$")>;

def SPRWriteResGroup526 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 9;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup526], (instregex "^VRCPPHZrk(z?)$")>;

def SPRWriteResGroup527 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 20;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup527, ReadAfterVecXLd], (instregex "^VREDUCEPHZ128rm(b?)i$",
                                                                "^VREDUCESHZrmi$")>;
def : InstRW<[SPRWriteResGroup527, ReadAfterVecYLd], (instregex "^VREDUCEPHZ256rm(b?)i$")>;

def SPRWriteResGroup528 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 22;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup528, ReadAfterVecXLd], (instregex "^VREDUCEPHZ128rm(bik|ikz)$",
                                                                "^VREDUCEPHZ128rmbikz$",
                                                                "^VREDUCEPHZ128rmik$",
                                                                "^VREDUCESHZrmik(z?)$")>;
def : InstRW<[SPRWriteResGroup528, ReadAfterVecYLd], (instregex "^VREDUCEPHZ256rm(bik|ikz)$",
                                                                "^VREDUCEPHZ256rmbikz$",
                                                                "^VREDUCEPHZ256rmik$")>;

def SPRWriteResGroup529 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [3];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup529], (instregex "^VREDUCEPHZ(128|256)rri$",
                                               "^VREDUCESHZrri(b?)$")>;

def SPRWriteResGroup530 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [3];
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup530], (instregex "^VREDUCEPHZ(128|256)rrik(z?)$",
                                               "^VREDUCESHZrri(bk|kz)$",
                                               "^VREDUCESHZrri(k|bkz)$")>;

def SPRWriteResGroup531 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 20;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup531, ReadAfterVecYLd], (instregex "^VREDUCEPHZrm(b?)i$")>;

def SPRWriteResGroup532 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [3, 1];
  let Latency = 22;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup532, ReadAfterVecYLd], (instregex "^VREDUCEPHZrm(bik|ikz)$",
                                                                "^VREDUCEPHZrmbikz$",
                                                                "^VREDUCEPHZrmik$")>;

def SPRWriteResGroup533 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [3];
  let Latency = 13;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup533], (instregex "^VREDUCEPHZrri(b?)$")>;

def SPRWriteResGroup534 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [3];
  let Latency = 16;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup534], (instregex "^VREDUCEPHZrri(bk|kz)$",
                                               "^VREDUCEPHZrri(k|bkz)$")>;

def SPRWriteResGroup535 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [2];
  let Latency = 8;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup535], (instregex "^VRNDSCALEP(D|S)Zrri((b|k|bk|kz|bkz)?)$")>;

def SPRWriteResGroup536 : SchedWriteRes<[SPRPort00_01, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 17;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup536, ReadAfterVecXLd], (instregex "^VRNDSCALEPHZ128rm(bik|ikz)$",
                                                                "^VRNDSCALEPHZ128rmbikz$",
                                                                "^VRNDSCALEPHZ128rmik$",
                                                                "^VRNDSCALESHZm_Intk(z?)$",
                                                                "^VSCALEFPHZ128rm(bk|kz)$",
                                                                "^VSCALEFPHZ128rm(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup536, ReadAfterVecYLd], (instregex "^VRNDSCALEPHZ256rm(bik|ikz)$",
                                                                "^VRNDSCALEPHZ256rmbikz$",
                                                                "^VRNDSCALEPHZ256rmik$",
                                                                "^VSCALEFPHZ256rm(bk|kz)$",
                                                                "^VSCALEFPHZ256rm(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup536, ReadAfterVecLd], (instregex "^VSCALEFSHZrm_Intk(z?)$")>;

def SPRWriteResGroup537 : SchedWriteRes<[SPRPort00_01]> {
  let ResourceCycles = [2];
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup537], (instregex "^VRNDSCALEPHZ(128|256)rrik(z?)$",
                                               "^VRNDSCALESHZr(b?)_Intk(z?)$",
                                               "^VSCALEFPHZ(128|256)rrk(z?)$",
                                               "^VSCALEFSHZrr(b?)_Intk(z?)$")>;

def SPRWriteResGroup538 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 17;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup538, ReadAfterVecYLd], (instregex "^VRNDSCALEPHZrm(bik|ikz)$",
                                                                "^VRNDSCALEPHZrmbikz$",
                                                                "^VRNDSCALEPHZrmik$",
                                                                "^VSCALEFPHZrm(bk|kz)$",
                                                                "^VSCALEFPHZrm(k|bkz)$")>;

def SPRWriteResGroup539 : SchedWriteRes<[SPRPort00]> {
  let ResourceCycles = [2];
  let Latency = 11;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup539], (instregex "^VRNDSCALEPHZrri(bk|kz)$",
                                               "^VRNDSCALEPHZrri(k|bkz)$",
                                               "^VSCALEFPHZrr(bk|kz)$",
                                               "^VSCALEFPHZrr(k|bkz)$")>;

def SPRWriteResGroup540 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 6;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup540], (instregex "^VRSQRT14P(D|S)Zr$",
                                               "^VRSQRT14PSZrk$",
                                               "^VRSQRTPHZr$")>;

def SPRWriteResGroup541 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 25;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup541], (instregex "^VSQRTPDYm$")>;
def : InstRW<[SPRWriteResGroup541, ReadAfterVecYLd], (instregex "^VSQRTPDZ256m(b?)$")>;

def SPRWriteResGroup542 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 20;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup542, ReadAfterVecXLd], (instregex "^VSQRTPDZ128m(bk|kz)$",
                                                                "^VSQRTPDZ128m(k|bkz)$")>;
def : InstRW<[SPRWriteResGroup542, ReadAfterVecLd], (instregex "^VSQRTSDZm_Intk(z?)$")>;

def SPRWriteResGroup543 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 38;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup543, ReadAfterVecYLd], (instregex "^VSQRTPDZm$")>;

def SPRWriteResGroup544 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 39;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup544, ReadAfterVecYLd], (instregex "^VSQRTPDZmb$")>;

def SPRWriteResGroup545 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 31;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup545], (instregex "^VSQRTPDZr$")>;

def SPRWriteResGroup546 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 41;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup546, ReadAfterVecXLd], (instregex "^VSQRTPHZ128m(bk|kz)$",
                                                                "^VSQRTPHZ128m(k|bkz)$")>;

def SPRWriteResGroup547 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 35;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup547], (instregex "^VSQRTPHZ(128|256)rk$",
                                               "^VSQRTPHZ256rkz$")>;

def SPRWriteResGroup548 : SchedWriteRes<[SPRPort00, SPRPort00_01_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 12;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup548], (instregex "^VSQRTPHZ128rkz$")>;

def SPRWriteResGroup549 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 40;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup549, ReadAfterVecYLd], (instregex "^VSQRTPHZ256m(b?)$")>;

def SPRWriteResGroup550 : SchedWriteRes<[SPRPort00, SPRPort00_01_05, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1, 1];
  let Latency = 42;
  let NumMicroOps = 4;
}
def : InstRW<[SPRWriteResGroup550, ReadAfterVecYLd], (instregex "^VSQRTPHZ256m(bk|kz)$",
                                                                "^VSQRTPHZ256m(k|bkz)$")>;

def SPRWriteResGroup551 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [4, 2, 1, 1, 1];
  let Latency = 53;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup551, ReadAfterVecYLd], (instregex "^VSQRTPHZm(b?)$")>;

def SPRWriteResGroup552 : SchedWriteRes<[SPRPort00, SPRPort00_05, SPRPort00_06, SPRPort02_03_11, SPRPort05]> {
  let ResourceCycles = [4, 2, 1, 1, 1];
  let Latency = 55;
  let NumMicroOps = 9;
}
def : InstRW<[SPRWriteResGroup552, ReadAfterVecYLd], (instregex "^VSQRTPHZm(bk|kz)$",
                                                                "^VSQRTPHZm(k|bkz)$")>;

def SPRWriteResGroup553 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [4, 1, 1];
  let Latency = 45;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup553], (instregex "^VSQRTPHZr(b?)$")>;

def SPRWriteResGroup554 : SchedWriteRes<[SPRPort00, SPRPort00_06, SPRPort05]> {
  let ResourceCycles = [4, 1, 1];
  let Latency = 47;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup554], (instregex "^VSQRTPHZr(bk|kz)$",
                                               "^VSQRTPHZr(k|bkz)$")>;

def SPRWriteResGroup555 : SchedWriteRes<[SPRPort00, SPRPort00_05]> {
  let ResourceCycles = [2, 1];
  let Latency = 19;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup555], (instregex "^VSQRTPSZr$")>;

def SPRWriteResGroup556 : SchedWriteRes<[SPRPort00, SPRPort02_03_11]> {
  let Latency = 22;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup556, ReadAfterVecLd], (instregex "^VSQRTSHZm_Int((k|kz)?)$")>;

def SPRWriteResGroup557 : SchedWriteRes<[SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort01_05_10]> {
  let ResourceCycles = [1, 2, 3, 3, 1];
  let Latency = 12;
  let NumMicroOps = 10;
}
def : InstRW<[SPRWriteResGroup557], (instregex "^VZEROALL$")>;

def SPRWriteResGroup558 : SchedWriteRes<[]> {
  let Latency = 0;
  let NumMicroOps = 0;
}
def : InstRW<[SPRWriteResGroup558], (instregex "^VZEROUPPER$")>;

def SPRWriteResGroup559 : SchedWriteRes<[SPRPort00_01_05_06]> {
  let ResourceCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup559], (instregex "^WAIT$")>;

def SPRWriteResGroup560 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [8, 6, 19, 63, 21, 15, 1, 10, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 144;
}
def : InstRW<[SPRWriteResGroup560], (instregex "^WRMSR$")>;

def SPRWriteResGroup561 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06, SPRPort01, SPRPort05]> {
  let ResourceCycles = [2, 1, 4, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 8;
}
def : InstRW<[SPRWriteResGroup561], (instregex "^WRPKRUr$")>;

def SPRWriteResGroup562 : SchedWriteRes<[SPRPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 12;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup562, WriteRMW], (instregex "^XADD(16|32|64)rm$")>;

def SPRWriteResGroup563 : SchedWriteRes<[SPRPort00_01_05_06_10]> {
  let ResourceCycles = [2];
  let Latency = 13;
  let NumMicroOps = 2;
}
def : InstRW<[SPRWriteResGroup563, WriteRMW], (instregex "^XADD8rm$")>;

def SPRWriteResGroup564 : SchedWriteRes<[SPRPort00]> {
  let Latency = 2;
}
def : InstRW<[SPRWriteResGroup564], (instregex "^XAM_F$")>;

def SPRWriteResGroup565 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 39;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup565, WriteRMW], (instregex "^XCHG(16|32)rm$")>;

def SPRWriteResGroup566 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let ResourceCycles = [5, 1];
  let Latency = 39;
  let NumMicroOps = 6;
}
def : InstRW<[SPRWriteResGroup566, WriteRMW], (instregex "^XCHG64rm$")>;

def SPRWriteResGroup567 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let ResourceCycles = [4, 1];
  let Latency = 40;
  let NumMicroOps = 5;
}
def : InstRW<[SPRWriteResGroup567, WriteRMW], (instregex "^XCHG8rm$")>;

def SPRWriteResGroup568 : SchedWriteRes<[SPRPort00, SPRPort00_01_05_06, SPRPort00_05, SPRPort01, SPRPort05, SPRPort06]> {
  let ResourceCycles = [2, 4, 2, 1, 2, 4];
  let Latency = 17;
  let NumMicroOps = 15;
}
def : InstRW<[SPRWriteResGroup568], (instregex "^XCH_F$")>;

def SPRWriteResGroup569 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01]> {
  let ResourceCycles = [7, 3, 8, 5];
  let Latency = 4;
  let NumMicroOps = 23;
}
def : InstRW<[SPRWriteResGroup569], (instregex "^XGETBV$")>;

def SPRWriteResGroup570 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort02_03_11]> {
  let ResourceCycles = [2, 1];
  let Latency = 7;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup570], (instregex "^XLAT$")>;

def SPRWriteResGroup571 : SchedWriteRes<[SPRPort00_01_05_06, SPRPort01, SPRPort02_03, SPRPort06]> {
  let ResourceCycles = [21, 1, 1, 8];
  let Latency = 37;
  let NumMicroOps = 31;
}
def : InstRW<[SPRWriteResGroup571], (instregex "^XRSTOR((S|64|S64)?)$")>;

def SPRWriteResGroup572 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 42;
  let NumMicroOps = 140;
}
def : InstRW<[SPRWriteResGroup572], (instregex "^XSAVE$")>;

def SPRWriteResGroup573 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [14, 25, 44, 21, 21, 4, 1, 9, 1];
  let Latency = 41;
  let NumMicroOps = 140;
}
def : InstRW<[SPRWriteResGroup573], (instregex "^XSAVE64$")>;

def SPRWriteResGroup574 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 19, 36, 52, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 151;
}
def : InstRW<[SPRWriteResGroup574], (instregex "^XSAVEC$")>;

def SPRWriteResGroup575 : SchedWriteRes<[SPRPort00, SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [1, 19, 36, 53, 23, 4, 2, 12, 2];
  let Latency = 42;
  let NumMicroOps = 152;
}
def : InstRW<[SPRWriteResGroup575], (instregex "^XSAVEC64$")>;

def SPRWriteResGroup576 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [25, 35, 52, 27, 4, 1, 10, 1];
  let Latency = 42;
  let NumMicroOps = 155;
}
def : InstRW<[SPRWriteResGroup576], (instregex "^XSAVEOPT$")>;

def SPRWriteResGroup577 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [25, 35, 53, 27, 4, 1, 10, 1];
  let Latency = 42;
  let NumMicroOps = 156;
}
def : InstRW<[SPRWriteResGroup577], (instregex "^XSAVEOPT64$")>;

def SPRWriteResGroup578 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [23, 32, 53, 29, 30, 4, 2, 9, 2];
  let Latency = 42;
  let NumMicroOps = 184;
}
def : InstRW<[SPRWriteResGroup578], (instregex "^XSAVES$")>;

def SPRWriteResGroup579 : SchedWriteRes<[SPRPort00_01, SPRPort00_05, SPRPort00_06, SPRPort01, SPRPort01_05, SPRPort02_03_11, SPRPort04_09, SPRPort05, SPRPort07_08]> {
  let ResourceCycles = [23, 33, 53, 29, 32, 4, 2, 8, 2];
  let Latency = 42;
  let NumMicroOps = 186;
}
def : InstRW<[SPRWriteResGroup579], (instregex "^XSAVES64$")>;

def SPRWriteResGroup580 : SchedWriteRes<[SPRPort00_01_05, SPRPort00_01_05_06_10, SPRPort00_05_06, SPRPort00_06, SPRPort01, SPRPort01_05_10, SPRPort05]> {
  let ResourceCycles = [4, 23, 2, 14, 8, 1, 2];
  let Latency = 5;
  let NumMicroOps = 54;
}
def : InstRW<[SPRWriteResGroup580], (instregex "^XSETBV$")>;

def SPRWriteResGroup581 : SchedWriteRes<[SPRPort00_01_05_06_10, SPRPort00_06]> {
  let ResourceCycles = [2, 1];
  let Latency = SPRModel.MaxLatency;
  let NumMicroOps = 3;
}
def : InstRW<[SPRWriteResGroup581], (instregex "^XTEST$")>;

}
