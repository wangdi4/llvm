#if INTEL_FEATURE_CPU_RYL
//=- Intel_X86ScheduleRYL.td - X86 Royal Scheduling ------*- tablegen -*-=//
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
// Automatically generated file by smg tool.
//
//===----------------------------------------------------------------------===//

def RYLModel : SchedMachineModel {
  let IssueWidth = 10; // Based on allocator width.
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

// Royal can issue micro-ops to 24 different ports in one cycle.
def RYLPort00 : ProcResource<1>; // idiv0
def RYLPort01 : ProcResource<1>; // imul00
def RYLPort02 : ProcResource<1>; // imul10
def RYLPort03 : ProcResource<1>; // int_br0
def RYLPort04 : ProcResource<1>; // int_br1
def RYLPort05 : ProcResource<1>; // int_ind0
def RYLPort06 : ProcResource<1>; // ldagu0
def RYLPort07 : ProcResource<1>; // staagu0
def RYLPort08 : ProcResource<1>; // ldagu1
def RYLPort09 : ProcResource<1>; // staagu1
def RYLPort10 : ProcResource<1>; // ldagu2
def RYLPort11 : ProcResource<1>; // staagu2
def RYLPort12 : ProcResource<1>; // ldagu3
def RYLPort13 : ProcResource<1>; // sink0
def RYLPort14 : ProcResource<1>; // sink1
def RYLPort15 : ProcResource<1>; // sink2
def RYLPort16 : ProcResource<1>; // vec00
def RYLPort17 : ProcResource<1>; // vec01
def RYLPort18 : ProcResource<1>; // vec10
def RYLPort19 : ProcResource<1>; // vec11
def RYLPort20 : ProcResource<1>; // vec20
def RYLPort21 : ProcResource<1>; // vec21
def RYLPort22 : ProcResource<1>; // vec_sink00
def RYLPort23 : ProcResource<1>; // vec_sink01
def RYLPort24 : ProcResource<1>; // vec_sink10

// Workaround to represent invalid ports. WriteRes shouldn't use this resource.
def RYLPortInvalid : ProcResource<1>;

// Many micro-ops are capable of issuing on multiple ports.
// TODO(ekudrias): Specify BufferSize for ProcResGroup
def RYLPort00_01_02          : ProcResGroup<[RYLPort00, RYLPort01, RYLPort02]>;
def RYLPort00_01_02_03_04_05 : ProcResGroup<[RYLPort00, RYLPort01, RYLPort02, RYLPort03, RYLPort04, RYLPort05]>;
def RYLPort00_01_05          : ProcResGroup<[RYLPort00, RYLPort01, RYLPort05]>;
def RYLPort01_02             : ProcResGroup<[RYLPort01, RYLPort02]>;
def RYLPort03_04_05          : ProcResGroup<[RYLPort03, RYLPort04, RYLPort05]>;
def RYLPort06_08_10_12       : ProcResGroup<[RYLPort06, RYLPort08, RYLPort10, RYLPort12]>;
def RYLPort07_09_11          : ProcResGroup<[RYLPort07, RYLPort09, RYLPort11]>;
def RYLPort13_14_15          : ProcResGroup<[RYLPort13, RYLPort14, RYLPort15]>;
def RYLPort16_17             : ProcResGroup<[RYLPort16, RYLPort17]>;
def RYLPort16_17_18_19       : ProcResGroup<[RYLPort16, RYLPort17, RYLPort18, RYLPort19]>;
def RYLPort16_17_18_19_20_21 : ProcResGroup<[RYLPort16, RYLPort17, RYLPort18, RYLPort19, RYLPort20, RYLPort21]>;
def RYLPort16_17_20_21       : ProcResGroup<[RYLPort16, RYLPort17, RYLPort20, RYLPort21]>;
def RYLPort18_19             : ProcResGroup<[RYLPort18, RYLPort19]>;
def RYLPort18_19_20_21       : ProcResGroup<[RYLPort18, RYLPort19, RYLPort20, RYLPort21]>;
def RYLPort20_21             : ProcResGroup<[RYLPort20, RYLPort21]>;
def RYLPort22_23_24          : ProcResGroup<[RYLPort22, RYLPort23, RYLPort24]>;

// TODO(ekudrias): The rest part is from alderlake. We need to verify or adjust it.

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
    let ReleaseAtCycles = Res;
    let NumMicroOps = UOps;
  }

  // Memory variant also uses a cycle on port 6/8/10/12 and adds LoadLat cycles to
  // the latency (default = 5).
  def : WriteRes<SchedRW.Folded, !listconcat([RYLPort06_08_10_12], ExePorts)> {
    let Latency = !add(Lat, LoadLat);
    let ReleaseAtCycles = !listconcat([1], Res);
    let NumMicroOps = !add(UOps, 1);
  }
}

//===----------------------------------------------------------------------===//
// The following definitons are infered by smg.
//===----------------------------------------------------------------------===//

// Infered SchedWrite definition.
def : WriteRes<WriteADC, [RYLPort00_01_05]>;
def : WriteRes<WriteADCLd, [RYLPort00_01_05]> {
  let Latency = 3;
}
defm : RYLWriteResPair<WriteAESDecEnc, [RYLPort20_21], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteAESIMC, [RYLPort20_21], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteAESKeyGen, [RYLPort20_21], 3, [3], 3, 1>;
def : WriteRes<WriteALU, [RYLPort00_01_02_03_04_05]>;
def : WriteRes<WriteALULd, [RYLPort00_01_02_03_04_05]> {
  let Latency = 3;
}
defm : RYLWriteResPair<WriteBEXTR, [RYLPort00_01_02_03_04_05], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteBLS, [RYLPort00_01_02_03_04_05], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteBSF, [RYLPort00], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteBSR, [RYLPort00], 1, [1], 1, 1>;
def : WriteRes<WriteBSWAP32, [RYLPort00_01_02_03_04_05]>;
def : WriteRes<WriteBSWAP64, [RYLPort00_01_02_03_04_05]>;
defm : RYLWriteResPair<WriteBZHI, [RYLPort00_01_02_03_04_05], 1, [1], 1, 1>;
def : WriteRes<WriteBitTest, [RYLPort00_01_02_03_04_05]>;
defm : X86WriteRes<WriteBitTestImmLd, [RYLPort00_01_02_03_04_05, RYLPort06_08_10_12], 2, [1, 1], 2>;
defm : X86WriteRes<WriteBitTestRegLd, [RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12], 6, [4, 1, 1], 6>;
def : WriteRes<WriteBitTestSet, [RYLPort00_01_02_03_04_05]>;
def : WriteRes<WriteBitTestSetImmLd, [RYLPort00_01_02_03_04_05]> {
  let Latency = 3;
}
def : WriteRes<WriteBitTestSetRegLd, [RYLPort06_08_10_12]> {
  let Latency = 5;
}
defm : RYLWriteResPair<WriteBlend, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteBlendY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteCLMul, [RYLPort18_19], 4, [4], 4, 1>;
defm : RYLWriteResPair<WriteCMOV, [RYLPort00_01_05], 1, [1], 1, 1>;
defm : X86WriteRes<WriteCMPXCHG, [RYLPort00_01_02_03_04_05], 5, [5], 5>;
defm : X86WriteRes<WriteCMPXCHGRMW, [RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15], 6, [3, 1, 1, 1], 6>;
defm : RYLWriteResPair<WriteCRC32, [RYLPort01_02], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtI2PD, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtI2PDY, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3, 1>;
defm : X86WriteResPairUnsupported<WriteCvtI2PDZ>;
defm : RYLWriteResPair<WriteCvtI2PS, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtI2PSY, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteCvtI2PSZ>;
defm : X86WriteRes<WriteCvtI2SD, [RYLPort00, RYLPort16_17_18_19], 6, [4, 2], 6>;
defm : X86WriteRes<WriteCvtI2SDLd, [RYLPort06_08_10_12, RYLPort16_17_18_19], 3, [1, 2], 3>;
defm : X86WriteRes<WriteCvtI2SS, [RYLPort00, RYLPort16_17_18_19], 6, [4, 2], 6>;
defm : X86WriteRes<WriteCvtI2SSLd, [RYLPort06_08_10_12, RYLPort16_17_18_19], 3, [1, 2], 3>;
defm : RYLWriteResPair<WriteCvtPD2I, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtPD2IY, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3, 1>;
defm : X86WriteResPairUnsupported<WriteCvtPD2IZ>;
defm : RYLWriteResPair<WriteCvtPD2PS, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : X86WriteRes<WriteCvtPD2PSY, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3>;
defm : X86WriteRes<WriteCvtPD2PSYLd, [RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort20_21], 5, [1, 2, 2], 5>;
defm : X86WriteResPairUnsupported<WriteCvtPD2PSZ>;
defm : X86WriteRes<WriteCvtPH2PS, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3>;
defm : X86WriteRes<WriteCvtPH2PSLd, [RYLPort06_08_10_12, RYLPort16_17_18_19], 3, [1, 2], 3>;
defm : RYLWriteResPair<WriteCvtPH2PSY, [RYLPort16_17_18_19, RYLPort20_21], 4, [2, 2], 4, 1>;
defm : X86WriteResPairUnsupported<WriteCvtPH2PSZ>;
defm : RYLWriteResPair<WriteCvtPS2I, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtPS2IY, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteCvtPS2IZ>;
defm : RYLWriteResPair<WriteCvtPS2PD, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : X86WriteRes<WriteCvtPS2PDY, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3>;
defm : X86WriteRes<WriteCvtPS2PDYLd, [RYLPort06_08_10_12, RYLPort16_17_18_19], 3, [1, 2], 3>;
defm : X86WriteResPairUnsupported<WriteCvtPS2PDZ>;
defm : X86WriteRes<WriteCvtPS2PH, [RYLPort16_17_18_19, RYLPort18_19], 6, [5, 1], 6>;
defm : X86WriteRes<WriteCvtPS2PHSt, [RYLPort07_09_11, RYLPort16_17_18_19, RYLPort22_23_24], 4, [1, 2, 1], 4>;
defm : X86WriteRes<WriteCvtPS2PHY, [RYLPort16_17_18_19, RYLPort18_19], 6, [5, 1], 6>;
defm : X86WriteRes<WriteCvtPS2PHYSt, [RYLPort07_09_11, RYLPort16_17_18_19, RYLPort22_23_24], 4, [1, 2, 1], 4>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZ>;
defm : X86WriteResUnsupported<WriteCvtPS2PHZSt>;
defm : RYLWriteResPair<WriteCvtSD2I, [RYLPort16_17_18_19, RYLPort22_23_24], 6, [2, 4], 6, 1>;
defm : RYLWriteResPair<WriteCvtSD2SS, [RYLPort16_17_18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteCvtSS2I, [RYLPort16_17_18_19, RYLPort22_23_24], 6, [2, 4], 6, 1>;
defm : X86WriteRes<WriteCvtSS2SD, [RYLPort16_17_18_19, RYLPort18_19], 3, [2, 1], 3>;
defm : X86WriteRes<WriteCvtSS2SDLd, [RYLPort06_08_10_12, RYLPort16_17_18_19], 3, [1, 2], 3>;
defm : RYLWriteResPair<WriteDPPD, [RYLPort16_17_18_19, RYLPort16_17_20_21, RYLPort18_19], 6, [3, 2, 1], 6, 1>;
defm : RYLWriteResPair<WriteDPPS, [RYLPort05, RYLPort16_17_18_19, RYLPort16_17_20_21, RYLPort18_19], 10, [1, 3, 4, 2], 10, 1>;
defm : RYLWriteResPair<WriteDPPSY, [RYLPort16_17_18_19, RYLPort16_17_20_21, RYLPort18_19], 8, [3, 4, 1], 8, 1>;
defm : RYLWriteResPair<WriteDiv16, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 12, [9, 1, 2], 12, 1>;
defm : RYLWriteResPair<WriteDiv32, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 12, [9, 1, 2], 12, 1>;
defm : RYLWriteResPair<WriteDiv64, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 16, [12, 1, 3], 16, 1>;
defm : RYLWriteResPair<WriteDiv8, [RYLPort00], 9, [9], 9, 1>;
defm : X86WriteRes<WriteEMMS, [RYLPort00_01_02_03_04_05, RYLPort16_17], 10, [2, 8], 10>;
defm : X86WriteRes<WriteFAdd, [RYLPort16_17], 3, [3], 3>;
defm : X86WriteRes<WriteFAddLd, [RYLPort06_08_10_12, RYLPort16_17, RYLPort18_19], 5, [1, 3, 1], 5>;
defm : RYLWriteResPair<WriteFAdd64, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFAdd64X, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFAdd64Y, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteFAdd64Z>;
defm : RYLWriteResPair<WriteFAddX, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFAddY, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteFAddZ>;
defm : RYLWriteResPair<WriteFBlend, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFBlendY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteRes<WriteFCMOV, [RYLPort00, RYLPort16_17, RYLPort16_17_18_19_20_21], 7, [4, 2, 1], 7>;
defm : RYLWriteResPair<WriteFCmp, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFCmp64, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFCmp64X, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFCmp64Y, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteFCmp64Z>;
defm : RYLWriteResPair<WriteFCmpX, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFCmpY, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteFCmpZ>;
defm : X86WriteRes<WriteFCom, [RYLPort16_17], 3, [3], 3>;
defm : X86WriteRes<WriteFComLd, [RYLPort06_08_10_12, RYLPort16_17, RYLPort18_19], 5, [1, 3, 1], 5>;
defm : RYLWriteResPair<WriteFComX, [RYLPort18_19_20_21, RYLPort22_23_24], 5, [1, 4], 5, 1>;
defm : X86WriteRes<WriteFDiv, [RYLPort16_17], 10, [10], 10>;
defm : X86WriteRes<WriteFDivLd, [RYLPort06_08_10_12, RYLPort16_17, RYLPort18_19], 12, [1, 10, 1], 12>;
defm : RYLWriteResPair<WriteFDiv64, [RYLPort18_19], 10, [10], 10, 1>;
defm : RYLWriteResPair<WriteFDiv64X, [RYLPort18_19], 10, [10], 10, 1>;
defm : RYLWriteResPair<WriteFDiv64Y, [RYLPort18_19], 10, [10], 10, 1>;
defm : X86WriteResPairUnsupported<WriteFDiv64Z>;
defm : RYLWriteResPair<WriteFDivX, [RYLPort18_19], 7, [7], 7, 1>;
defm : RYLWriteResPair<WriteFDivY, [RYLPort18_19], 7, [7], 7, 1>;
defm : X86WriteResPairUnsupported<WriteFDivZ>;
defm : RYLWriteResPair<WriteFHAdd, [RYLPort00_01_02_03_04_05, RYLPort16_17_20_21, RYLPort18_19], 5, [1, 2, 2], 5, 1>;
defm : RYLWriteResPair<WriteFHAddY, [RYLPort16_17_20_21, RYLPort18_19], 4, [2, 2], 4, 1>;
def : WriteRes<WriteFLD0, [RYLPort16_17]>;
defm : X86WriteRes<WriteFLD1, [RYLPort16_17], 4, [4], 4>;
defm : X86WriteRes<WriteFLDC, [RYLPort16_17], 4, [4], 4>;
defm : X86WriteRes<WriteFLoad, [RYLPort06_08_10_12, RYLPort18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteRes<WriteFLoadX, [RYLPort06_08_10_12, RYLPort18_19_20_21], 2, [1, 1], 2>;
def : WriteRes<WriteFLoadY, [RYLPort06_08_10_12]>;
defm : RYLWriteResPair<WriteFLogic, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFLogicY, [RYLPort18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteFLogicZ>;
defm : RYLWriteResPair<WriteFMA, [RYLPort16_17_18_19], 4, [4], 4, 1>;
defm : RYLWriteResPair<WriteFMAX, [RYLPort16_17_18_19], 4, [4], 4, 1>;
defm : RYLWriteResPair<WriteFMAY, [RYLPort16_17_18_19], 4, [4], 4, 1>;
defm : X86WriteResPairUnsupported<WriteFMAZ>;
defm : X86WriteRes<WriteFMOVMSK, [RYLPort18_19, RYLPort22_23_24], 5, [1, 4], 5>;
defm : X86WriteRes<WriteFMaskedLoad, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedLoadY, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteRes<WriteFMaskedStore32, [RYLPort07_09_11, RYLPort20_21, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore32Y, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 6, [2, 2, 2], 6>;
defm : X86WriteRes<WriteFMaskedStore64, [RYLPort07_09_11, RYLPort20_21, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteFMaskedStore64Y, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 6, [2, 2, 2], 6>;
def : WriteRes<WriteFMoveX, [RYLPort18_19_20_21]>;
def : WriteRes<WriteFMoveY, [RYLPort18_19_20_21]>;
defm : X86WriteResUnsupported<WriteFMoveZ>;
defm : RYLWriteResPair<WriteFMul, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFMul64, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFMul64X, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFMul64Y, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WriteFMul64Z>;
defm : RYLWriteResPair<WriteFMulX, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFMulY, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WriteFMulZ>;
defm : RYLWriteResPair<WriteFRcp, [RYLPort18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFRcpX, [RYLPort18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFRcpY, [RYLPort18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WriteFRcpZ>;
defm : RYLWriteResPair<WriteFRnd, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFRndY, [RYLPort16_17_20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteFRndZ>;
defm : RYLWriteResPair<WriteFRsqrt, [RYLPort18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFRsqrtX, [RYLPort18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteFRsqrtY, [RYLPort18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WriteFRsqrtZ>;
defm : RYLWriteResPair<WriteFShuffle, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFShuffle256, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFShuffleY, [RYLPort18_19], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteFShuffleZ>;
def : WriteRes<WriteFSign, [RYLPort16_17]>;
defm : RYLWriteResPair<WriteFSqrt, [RYLPort18_19], 7, [7], 7, 1>;
defm : RYLWriteResPair<WriteFSqrt64, [RYLPort18_19], 10, [10], 10, 1>;
defm : RYLWriteResPair<WriteFSqrt64X, [RYLPort18_19], 10, [10], 10, 1>;
defm : RYLWriteResPair<WriteFSqrt64Y, [RYLPort18_19], 10, [10], 10, 1>;
defm : X86WriteResPairUnsupported<WriteFSqrt64Z>;
defm : X86WriteRes<WriteFSqrt80, [RYLPort16_17], 10, [10], 10>;
defm : RYLWriteResPair<WriteFSqrtX, [RYLPort18_19], 7, [7], 7, 1>;
defm : RYLWriteResPair<WriteFSqrtY, [RYLPort18_19], 7, [7], 7, 1>;
defm : X86WriteResPairUnsupported<WriteFSqrtZ>;
defm : X86WriteRes<WriteFStore, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteResUnsupported<WriteFStoreNT>;
defm : X86WriteRes<WriteFStoreNTX, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreNTY, [RYLPort07_09_11, RYLPort22_23_24], 4, [2, 2], 4>;
defm : X86WriteRes<WriteFStoreX, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteRes<WriteFStoreY, [RYLPort07_09_11, RYLPort22_23_24], 4, [2, 2], 4>;
defm : RYLWriteResPair<WriteFTest, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFTestY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFVarBlend, [RYLPort18_19], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFVarBlendY, [RYLPort16_17_18_19_20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteFVarBlendZ>;
defm : RYLWriteResPair<WriteFVarShuffle, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteFVarShuffle256, [RYLPort20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteFVarShuffleY, [RYLPort18_19], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteFVarShuffleZ>;
defm : X86WriteRes<WriteFence, [RYLPort00_01_02_03_04_05, RYLPort07_09_11], 2, [1, 1], 2>;
defm : RYLWriteResPair<WriteIDiv16, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 12, [9, 1, 2], 12, 1>;
defm : RYLWriteResPair<WriteIDiv32, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 12, [9, 1, 2], 12, 1>;
defm : RYLWriteResPair<WriteIDiv64, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort01_02], 16, [12, 1, 3], 16, 1>;
defm : RYLWriteResPair<WriteIDiv8, [RYLPort00], 9, [9], 9, 1>;
defm : RYLWriteResPair<WriteIMul16, [RYLPort00_01_02_03_04_05, RYLPort01_02], 5, [3, 2], 5, 1>;
defm : RYLWriteResPair<WriteIMul16Imm, [RYLPort00_01_02_03_04_05, RYLPort01_02], 3, [1, 2], 3, 1>;
defm : RYLWriteResPair<WriteIMul16Reg, [RYLPort00_01_02_03_04_05, RYLPort01_02], 3, [1, 2], 3, 1>;
defm : RYLWriteResPair<WriteIMul32, [RYLPort00_01_02_03_04_05, RYLPort01_02], 3, [1, 2], 3, 1>;
defm : RYLWriteResPair<WriteIMul32Imm, [RYLPort01_02], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteIMul32Reg, [RYLPort01_02], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteIMul64, [RYLPort00_01_02_03_04_05, RYLPort01_02], 4, [1, 3], 4, 1>;
defm : RYLWriteResPair<WriteIMul64Imm, [RYLPort01_02], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteIMul64Reg, [RYLPort01_02], 3, [3], 3, 1>;
defm : X86WriteRes<WriteIMul8, [RYLPort00_01_02_03_04_05, RYLPort01_02], 4, [2, 2], 4>;
defm : X86WriteRes<WriteIMul8Ld, [RYLPort00_01_02_03_04_05, RYLPort01_02, RYLPort06_08_10_12], 4, [1, 2, 1], 4>;
def : WriteRes<WriteIMulH, []> {
  let Latency = 3;
}
def : WriteRes<WriteIMulHLd, []> {
  let Latency = 3;
}
defm : X86WriteRes<WriteJump, [RYLPort00_01_05, RYLPort05, RYLPort07_09_11, RYLPort13_14_15], 4, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteJumpLd, [RYLPort05, RYLPort06_08_10_12], 2, [1, 1], 2>;
def : WriteRes<WriteLAHFSAHF, [RYLPort00_01_02_03_04_05]>;
defm : X86WriteRes<WriteLDMXCSR, [RYLPort00, RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12], 10, [4, 1, 3, 1, 1], 10>;
def : WriteRes<WriteLEA, [RYLPort00_01_05]>;
defm : RYLWriteResPair<WriteLZCNT, [RYLPort00_01_02_03_04_05], 1, [1], 1, 1>;
def : WriteRes<WriteLoad, [RYLPort06_08_10_12]> {
  let Latency = 5;
}
defm : X86WriteRes<WriteMMXMOVMSK, [RYLPort18_19, RYLPort22_23_24], 5, [1, 4], 5>;
defm : RYLWriteResPair<WriteMPSAD, [RYLPort20_21], 4, [4], 4, 1>;
defm : X86WriteRes<WriteMPSADY, [RYLPort18_19, RYLPort20_21], 3, [1, 2], 3>;
defm : X86WriteRes<WriteMPSADYLd, [RYLPort06_08_10_12, RYLPort20_21], 5, [1, 4], 5>;
defm : RYLWriteResPair<WriteMULX32, [RYLPort00_01_02_03_04_05, RYLPort01_02], 4, [2, 2], 3, 1>;
defm : X86WriteRes<WriteMULX64, [RYLPort00_01_02_03_04_05, RYLPort01_02], 7, [2, 5], 6>;
defm : X86WriteRes<WriteMULX64Ld, [RYLPort00_01_02_03_04_05, RYLPort01_02, RYLPort06_08_10_12], 7, [1, 5, 1], 6>;
defm : X86WriteRes<WriteMicrocoded, [RYLPort00_01_02_03_04_05, RYLPort06_08_10_12], 3, [2, 1], 3>;
def : WriteRes<WriteMove, [RYLPort00_01_02_03_04_05]>;
def : WriteRes<WriteNop, [RYLPort16_17]>;
defm : X86WriteRes<WritePCmpEStrI, [RYLPort00, RYLPort00_01_02_03_04_05, RYLPort16_17_18_19, RYLPort22_23_24], 25, [8, 1, 12, 4], 25>;
defm : X86WriteRes<WritePCmpEStrILd, [RYLPort00, RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort22_23_24], 25, [8, 1, 12, 4], 25>;
defm : X86WriteRes<WritePCmpEStrM, [RYLPort00, RYLPort05, RYLPort16_17_18_19, RYLPort16_17_18_19_20_21, RYLPort22_23_24], 29, [8, 1, 15, 1, 4], 29>;
defm : X86WriteRes<WritePCmpEStrMLd, [RYLPort00, RYLPort05, RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort16_17_18_19_20_21, RYLPort22_23_24], 27, [8, 1, 1, 12, 1, 4], 27>;
defm : RYLWriteResPair<WritePCmpIStrI, [RYLPort16_17_18_19, RYLPort22_23_24], 10, [6, 4], 10, 1>;
defm : RYLWriteResPair<WritePCmpIStrM, [RYLPort16_17_18_19, RYLPort22_23_24], 10, [6, 4], 10, 1>;
defm : RYLWriteResPair<WritePHAdd, [RYLPort16_17_18_19_20_21], 3, [3], 3, 1>;
defm : RYLWriteResPair<WritePHAddX, [RYLPort16_17_18_19_20_21], 3, [3], 3, 1>;
defm : RYLWriteResPair<WritePHAddY, [RYLPort16_17_18_19_20_21, RYLPort18_19], 3, [1, 2], 3, 1>;
defm : RYLWriteResPair<WritePHMINPOS, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WritePMULLD, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WritePMULLDY, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WritePMULLDZ>;
defm : RYLWriteResPair<WritePOPCNT, [RYLPort01_02], 2, [2], 2, 1>;
defm : RYLWriteResPair<WritePSADBW, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WritePSADBWX, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WritePSADBWY, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WritePSADBWZ>;
defm : X86WriteRes<WriteRMW, [RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15], 1, [1, 1, 1], 3>;
defm : X86WriteRes<WriteRotate, [RYLPort00_01_02_03_04_05], 2, [2], 2>;
defm : X86WriteRes<WriteRotateLd, [RYLPort00_01_02_03_04_05], 5, [2], 2>;
defm : X86WriteRes<WriteRotateCL, [RYLPort00_01_02_03_04_05], 2, [2], 2>;
defm : X86WriteRes<WriteRotateCLLd, [RYLPort00_01_02_03_04_05], 5, [2], 2>;
def : WriteRes<WriteSETCC, [RYLPort00_01_02_03_04_05]>;
defm : X86WriteRes<WriteSETCCStore, [RYLPort00_01_02_03_04_05, RYLPort07_09_11, RYLPort13_14_15], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteSHDmrcl, [RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15], 5, [1, 1, 1, 1, 1], 5>;
defm : X86WriteRes<WriteSHDmri, [RYLPort00_01_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15], 4, [1, 1, 1, 1], 4>;
defm : X86WriteRes<WriteSHDrrcl, [RYLPort00_01_02_03_04_05, RYLPort00_01_05], 2, [1, 1], 2>;
def : WriteRes<WriteSHDrri, [RYLPort00_01_05]>;
defm : X86WriteRes<WriteSTMXCSR, [RYLPort00_01_02_03_04_05, RYLPort05, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24], 9, [1, 1, 1, 1, 1, 4], 9>;
def : WriteRes<WriteShift, [RYLPort00_01_02_03_04_05]>;
def : WriteRes<WriteShiftLd, [RYLPort00_01_02_03_04_05]> {
  let Latency = 4;
}
defm : X86WriteRes<WriteShiftCL, [RYLPort00_01_02_03_04_05], 2, [2], 2>;
defm : X86WriteRes<WriteShiftCLLd, [RYLPort00_01_02_03_04_05], 5, [2], 2>;
defm : RYLWriteResPair<WriteShuffle, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteShuffle256, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteShuffleX, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteShuffleY, [RYLPort18_19], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteShuffleZ>;
defm : X86WriteRes<WriteStore, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteStoreNT, [RYLPort07_09_11, RYLPort13_14_15], 2, [1, 1], 2>;
defm : X86WriteRes<WriteSystem, [RYLPort00_01_02, RYLPort00_01_02_03_04_05], 3, [1, 2], 3>;
defm : RYLWriteResPair<WriteTZCNT, [RYLPort00_01_02_03_04_05], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVPMOV256, [RYLPort18_19], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVarBlend, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVarBlendY, [RYLPort16_17_18_19_20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteVarBlendZ>;
defm : RYLWriteResPair<WriteVarShuffle, [RYLPort20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteVarShuffle256, [RYLPort20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteVarShuffleX, [RYLPort20_21], 2, [2], 2, 1>;
defm : RYLWriteResPair<WriteVarShuffleY, [RYLPort20_21], 2, [2], 2, 1>;
defm : X86WriteResPairUnsupported<WriteVarShuffleZ>;
defm : RYLWriteResPair<WriteVarVecShift, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVarVecShiftY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteVarVecShiftZ>;
defm : RYLWriteResPair<WriteVecALU, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVecALUX, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVecALUY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteVecALUZ>;
defm : X86WriteRes<WriteVecExtract, [RYLPort18_19, RYLPort22_23_24], 5, [1, 4], 5>;
defm : X86WriteRes<WriteVecExtractSt, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : RYLWriteResPair<WriteVecIMul, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteVecIMulX, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : RYLWriteResPair<WriteVecIMulY, [RYLPort16_17_18_19], 3, [3], 3, 1>;
defm : X86WriteResPairUnsupported<WriteVecIMulZ>;
defm : X86WriteRes<WriteVecInsert, [RYLPort00, RYLPort18_19], 5, [4, 1], 5>;
defm : X86WriteRes<WriteVecInsertLd, [RYLPort06_08_10_12, RYLPort18_19], 2, [1, 1], 2>;
def : WriteRes<WriteVecLoad, [RYLPort06_08_10_12]>;
defm : X86WriteRes<WriteVecLoadNT, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
def : WriteRes<WriteVecLoadNTY, [RYLPort06_08_10_12]>;
defm : X86WriteRes<WriteVecLoadX, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
def : WriteRes<WriteVecLoadY, [RYLPort06_08_10_12]>;
defm : RYLWriteResPair<WriteVecLogic, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVecLogicX, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : RYLWriteResPair<WriteVecLogicY, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteResPairUnsupported<WriteVecLogicZ>;
defm : X86WriteRes<WriteVecMOVMSK, [RYLPort18_19, RYLPort22_23_24], 5, [1, 4], 5>;
def : WriteRes<WriteVecMOVMSKY, [RYLPort18_19]>;
defm : X86WriteRes<WriteVecMaskedGatherWriteback, [], 5, [], 0>;
defm : X86WriteRes<WriteVecMaskedLoad, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedLoadY, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecMaskedStore32, [RYLPort07_09_11, RYLPort20_21, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore32Y, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 6, [2, 2, 2], 6>;
defm : X86WriteRes<WriteVecMaskedStore64, [RYLPort07_09_11, RYLPort20_21, RYLPort22_23_24], 3, [1, 1, 1], 3>;
defm : X86WriteRes<WriteVecMaskedStore64Y, [RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24], 6, [2, 2, 2], 6>;
def : WriteRes<WriteVecMove, [RYLPort16_17_18_19_20_21]>;
defm : X86WriteRes<WriteVecMoveFromGpr, [RYLPort00, RYLPort16_17_18_19_20_21], 5, [4, 1], 5>;
defm : X86WriteRes<WriteVecMoveToGpr, [RYLPort22_23_24], 4, [4], 4>;
def : WriteRes<WriteVecMoveX, [RYLPort16_17_18_19_20_21]>;
def : WriteRes<WriteVecMoveY, [RYLPort16_17_18_19_20_21]>;
defm : X86WriteResUnsupported<WriteVecMoveZ>;
defm : RYLWriteResPair<WriteVecShift, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
def : WriteRes<WriteVecShiftImm, [RYLPort16_17_18_19_20_21]>;
def : WriteRes<WriteVecShiftImmX, [RYLPort16_17_18_19_20_21]>;
defm : X86WriteResUnsupported<WriteVecShiftImmXLd>;
def : WriteRes<WriteVecShiftImmY, [RYLPort16_17_18_19_20_21]>;
defm : X86WriteResUnsupported<WriteVecShiftImmYLd>;
defm : X86WriteResPairUnsupported<WriteVecShiftImmZ>;
defm : RYLWriteResPair<WriteVecShiftX, [RYLPort16_17_18_19_20_21], 1, [1], 1, 1>;
defm : X86WriteRes<WriteVecShiftY, [RYLPort16_17_18_19_20_21, RYLPort18_19], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecShiftYLd, [RYLPort06_08_10_12, RYLPort16_17_18_19_20_21], 2, [1, 1], 2>;
defm : X86WriteResPairUnsupported<WriteVecShiftZ>;
defm : X86WriteRes<WriteVecStore, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNT, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreNTY, [RYLPort07_09_11, RYLPort22_23_24], 4, [2, 2], 4>;
defm : X86WriteRes<WriteVecStoreX, [RYLPort07_09_11, RYLPort22_23_24], 2, [1, 1], 2>;
defm : X86WriteRes<WriteVecStoreY, [RYLPort07_09_11, RYLPort22_23_24], 4, [2, 2], 4>;
defm : RYLWriteResPair<WriteVecTest, [RYLPort16_17_18_19_20_21, RYLPort22_23_24], 5, [1, 4], 5, 1>;
defm : RYLWriteResPair<WriteVecTestY, [RYLPort16_17_18_19_20_21, RYLPort22_23_24], 5, [1, 4], 5, 1>;
defm : X86WriteRes<WriteXCHG, [RYLPort00_01_02_03_04_05], 3, [3], 3>;
def : WriteRes<WriteZero, []>;

// Infered SchedWriteRes and InstRW definition.

def RYLWriteResGroup0 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup0], (instregex "^LEAVE((64)?)$",
                                             "^POP(16|32|64)r((mr)?)$")>;
def : InstRW<[RYLWriteResGroup0, ReadAfterLd, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(ADC|SBB)(8|16|32|64)rm$",
                                                                                                                                        "^AD(C|O)X(32|64)rm$")>;

def RYLWriteResGroup1 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup1], (instregex "^(CMP|TEST)(8|16|32)mi$",
                                             "^CMP(8|16|32|64)mi8$",
                                             "^(CMP|TEST)64mi32$",
                                             "^MOV(8|16)rm$",
                                             "^MOVBE(16|32|64)rm$",
                                             "^RORX(32|64)mi$")>;
def : InstRW<[RYLWriteResGroup1], (instrs MOV8rm_NOREX,
                                          MOVZX16rm8)>;
def : InstRW<[RYLWriteResGroup1, ReadAfterLd], (instregex "^A(D|N)D(8|16|32|64)rm$",
                                                          "^(ANDN|PDEP|PEXT)(32|64)rm$",
                                                          "^(CMP|SUB)(8|16|32|64)rm$",
                                                          "^(X?)OR(8|16|32|64)rm$")>;
def : InstRW<[RYLWriteResGroup1, ReadAfterLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^(CMP|TEST)(8|16|32|64)mr$",
                                                                                                                           "^S(A|H)RX(32|64)rm$",
                                                                                                                           "^SHLX(32|64)rm$")>;

def RYLWriteResGroup2 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_20_21]> {
  let ReleaseAtCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup2, ReadAfterVecLd], (instregex "^(V?)(ADD|SUB)SSrm((_Int)?)$")>;

def RYLWriteResGroup3 : SchedWriteRes<[RYLPort16_17_20_21]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup3], (instregex "^(V?)(ADD|SUB)SSrr((_Int)?)$")>;

def RYLWriteResGroup4 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup4], (instregex "^BT((C|R|S)?)(16|32|64)rr$",
                                             "^(CWD|WAIT)$")>;
def : InstRW<[RYLWriteResGroup4], (instrs CLFLUSH,
                                          XGETBV)>;

def RYLWriteResGroup5 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [3, 1, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup5], (instregex "^BT(C|R|S)(16|32|64)mr$")>;

def RYLWriteResGroup6 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [1, 1, 2, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup6], (instregex "^CALL(16|32|64)m((_NT)?)$")>;

def RYLWriteResGroup7 : SchedWriteRes<[RYLPort00_01_05, RYLPort03_04_05, RYLPort07_09_11, RYLPort13_14_15]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup7], (instregex "^CALL((64)?)pcrel32$")>;
def : InstRW<[RYLWriteResGroup7], (instrs CALLpcrel16)>;

def RYLWriteResGroup8 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup8], (instregex "^(CL|ST)AC$")>;

def RYLWriteResGroup9 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [4, 1];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup9], (instrs CLD)>;

def RYLWriteResGroup10 : SchedWriteRes<[RYLPort00_01_05, RYLPort07_09_11, RYLPort13_14_15]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup10], (instregex "^PUSH(16|32|64)(r|i8)$",
                                              "^PUSH(16|32|64)rmr$",
                                              "^PUSH((64)?)i32$")>;
def : InstRW<[RYLWriteResGroup10], (instrs CLDEMOTE,
                                           CLFLUSHOPT,
                                           PUSH16i)>;

def RYLWriteResGroup11 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup11], (instrs CLTS)>;

def RYLWriteResGroup12 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [3, 1, 2];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup12], (instregex "^CMPS(B|L|Q|W)$")>;

def RYLWriteResGroup13 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [8, 9, 1, 1, 2, 9];
  let Latency = 30;
  let NumMicroOps = 30;
}
def : InstRW<[RYLWriteResGroup13], (instrs CMPXCHG16B)>;

def RYLWriteResGroup14 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [7];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup14], (instregex "^RCR(16|32|64)rCL$")>;
def : InstRW<[RYLWriteResGroup14], (instrs CMPXCHG32rr)>;

def RYLWriteResGroup15 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [12, 1, 1, 1];
  let Latency = 15;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup15], (instrs CMPXCHG8B)>;

def RYLWriteResGroup16 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [9, 2];
  let Latency = 11;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup16], (instrs CPUID)>;

def RYLWriteResGroup17 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 7];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup17, ReadAfterVecLd], (instregex "^(V?)DIVSSrm((_Int)?)$")>;

def RYLWriteResGroup18 : SchedWriteRes<[RYLPort18_19]> {
  let ReleaseAtCycles = [7];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup18], (instregex "^(V?)DIVSSrr((_Int)?)$")>;

def RYLWriteResGroup19 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [12, 1, 1, 1, 2];
  let Latency = 17;
  let NumMicroOps = 17;
}
def : InstRW<[RYLWriteResGroup19], (instrs ENTER)>;

def RYLWriteResGroup20 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [3, 3, 18, 8];
  let Latency = 32;
  let NumMicroOps = 32;
}
def : InstRW<[RYLWriteResGroup20], (instrs F2XM1)>;

def RYLWriteResGroup21 : SchedWriteRes<[RYLPort07_09_11, RYLPort18_19, RYLPort22_23_24]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup21], (instrs FBSTPm)>;

def RYLWriteResGroup22 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup22], (instregex "^MUL_F(P?)rST0$")>;
def : InstRW<[RYLWriteResGroup22], (instrs FCOMPP,
                                           MUL_FST0r,
                                           UCOM_FPPr)>;

def RYLWriteResGroup23 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [4, 3, 1, 2, 4];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup23], (instrs FCOS)>;

def RYLWriteResGroup24 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup24], (instregex "^FFREE(P?)$")>;
def : InstRW<[RYLWriteResGroup24], (instrs FDECSTP,
                                           SHA256RNDS2rr)>;

def RYLWriteResGroup25 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 1, 1, 1, 1, 4];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup25], (instrs FLDCW16m)>;

def RYLWriteResGroup26 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort07_09_11, RYLPort16_17, RYLPort22_23_24]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup26], (instrs FNSTCW16m)>;

def RYLWriteResGroup27 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 8];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup27], (instrs FNSTSW16r)>;

def RYLWriteResGroup28 : SchedWriteRes<[RYLPort07_09_11, RYLPort16_17, RYLPort22_23_24]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup28], (instrs FNSTSWm)>;

def RYLWriteResGroup29 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [24, 14, 9, 229, 1, 44];
  let Latency = 321;
  let NumMicroOps = 321;
}
def : InstRW<[RYLWriteResGroup29], (instrs FPATAN)>;

def RYLWriteResGroup30 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort16_17]> {
  let ReleaseAtCycles = [1, 13];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup30], (instrs FPREM)>;

def RYLWriteResGroup31 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [11];
  let Latency = 11;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup31], (instrs FPREM1)>;

def RYLWriteResGroup32 : SchedWriteRes<[RYLPort16_17, RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [14, 2];
  let Latency = 16;
  let NumMicroOps = 16;
}
def : InstRW<[RYLWriteResGroup32], (instrs FPTAN)>;

def RYLWriteResGroup33 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort16_17_20_21, RYLPort18_19]> {
  let ReleaseAtCycles = [4, 1, 16, 2, 1];
  let Latency = 24;
  let NumMicroOps = 24;
}
def : InstRW<[RYLWriteResGroup33], (instrs FRNDINT)>;

def RYLWriteResGroup34 : SchedWriteRes<[RYLPort00, RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [32, 12, 72, 1, 27, 7, 23, 52];
  let Latency = 226;
  let NumMicroOps = 226;
}
def : InstRW<[RYLWriteResGroup34], (instrs FSAVEm)>;

def RYLWriteResGroup35 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17]> {
  let ReleaseAtCycles = [3, 1, 4];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup35], (instrs FSCALE)>;

def RYLWriteResGroup36 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 21, 6, 130, 2, 32];
  let Latency = 203;
  let NumMicroOps = 203;
}
def : InstRW<[RYLWriteResGroup36], (instrs FSIN)>;

def RYLWriteResGroup37 : SchedWriteRes<[RYLPort16_17, RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [22, 2];
  let Latency = 24;
  let NumMicroOps = 24;
}
def : InstRW<[RYLWriteResGroup37], (instrs FSINCOS)>;

def RYLWriteResGroup38 : SchedWriteRes<[RYLPort00, RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [32, 12, 48, 1, 11, 7, 10, 36];
  let Latency = 157;
  let NumMicroOps = 157;
}
def : InstRW<[RYLWriteResGroup38], (instrs FSTENVm)>;

def RYLWriteResGroup39 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [24, 52, 1, 33, 4, 16, 4];
  let Latency = 134;
  let NumMicroOps = 134;
}
def : InstRW<[RYLWriteResGroup39], (instrs FXRSTOR)>;

def RYLWriteResGroup40 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [48, 50, 1, 31, 4, 16, 4];
  let Latency = 154;
  let NumMicroOps = 154;
}
def : InstRW<[RYLWriteResGroup40], (instrs FXRSTOR64)>;

def RYLWriteResGroup41 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort07_09_11, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [40, 1, 36, 2, 36];
  let Latency = 115;
  let NumMicroOps = 115;
}
def : InstRW<[RYLWriteResGroup41], (instrs FXSAVE)>;

def RYLWriteResGroup42 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [37, 1, 32, 1, 2, 31];
  let Latency = 104;
  let NumMicroOps = 104;
}
def : InstRW<[RYLWriteResGroup42], (instrs FXSAVE64)>;

def RYLWriteResGroup43 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [14];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup43], (instrs FXTRACT)>;

def RYLWriteResGroup44 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort16_17, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 6, 4, 118, 1, 24];
  let Latency = 165;
  let NumMicroOps = 165;
}
def : InstRW<[RYLWriteResGroup44], (instrs FYL2X)>;

def RYLWriteResGroup45 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [10];
  let Latency = 10;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup45], (instrs FYL2XP1)>;

def RYLWriteResGroup46 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [12, 3, 2];
  let Latency = 17;
  let NumMicroOps = 17;
}
def : InstRW<[RYLWriteResGroup46], (instrs GETSEC)>;

def RYLWriteResGroup47 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 4];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup47, ReadAfterVecXLd], (instregex "^GF2P8AFFINE((INV)?)QBrmi$")>;
def : InstRW<[RYLWriteResGroup47, ReadAfterVecXLd], (instrs GF2P8MULBrm)>;

def RYLWriteResGroup48 : SchedWriteRes<[RYLPort18_19]> {
  let ReleaseAtCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup48], (instregex "^GF2P8AFFINE((INV)?)QBrri$")>;
def : InstRW<[RYLWriteResGroup48], (instrs GF2P8MULBrr)>;

def RYLWriteResGroup49 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup49], (instregex "^MOV(32|64)cr$")>;
def : InstRW<[RYLWriteResGroup49], (instrs HLT)>;

def RYLWriteResGroup50 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [13, 4, 3];
  let Latency = 20;
  let NumMicroOps = 20;
}
def : InstRW<[RYLWriteResGroup50], (instrs HRESET)>;

def RYLWriteResGroup51 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort18_19]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup51], (instregex "^ILD_F(16|32|64)m$",
                                              "^LD_F(32|64)m$")>;
def : InstRW<[RYLWriteResGroup51, ReadAfterVecXLd], (instregex "^(V?)PADDQrm$")>;

def RYLWriteResGroup52 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [5, 2, 1, 1, 1];
  let Latency = 10;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup52], (instregex "^INS(B|L|W)$")>;

def RYLWriteResGroup53 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [24, 5, 1, 3, 1, 2];
  let Latency = 36;
  let NumMicroOps = 36;
}
def : InstRW<[RYLWriteResGroup53], (instrs INVLPG)>;

def RYLWriteResGroup54 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [32, 3, 1, 1, 1, 2];
  let Latency = 40;
  let NumMicroOps = 40;
}
def : InstRW<[RYLWriteResGroup54], (instregex "^INV(PC|VP)ID(32|64)$")>;

def RYLWriteResGroup55 : SchedWriteRes<[RYLPort03_04_05]>;
def : InstRW<[RYLWriteResGroup55], (instregex "^J(CC|MP)_(1|2|4)$")>;

def RYLWriteResGroup56 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort03_04_05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup56], (instrs JCXZ)>;

def RYLWriteResGroup57 : SchedWriteRes<[RYLPort05]>;
def : InstRW<[RYLWriteResGroup57], (instregex "^JMP(16|32|64)r((_NT)?)$")>;

def RYLWriteResGroup58 : SchedWriteRes<[RYLPort06_08_10_12]>;
def : InstRW<[RYLWriteResGroup58], (instregex "^MOV(32|64)ao(32|64)$",
                                              "^MOV(32|64)rm$",
                                              "^MOVSX(16|32|64)rm(8|16|32)$",
                                              "^MOV(S|Z)X32rm8_NOREX$",
                                              "^MOVZX(16|32|64)rm16$",
                                              "^MOVZX(32|64)rm8$",
                                              "^PREFETCHT(0|1|2)$",
                                              "^VBROADCAST(F|I)128$",
                                              "^VBROADCASTS(S|DY)rm$",
                                              "^V(LD|MOV)DQUrm$",
                                              "^VMOV(A|U)P(D|S)rm$",
                                              "^VMOV(D|SH|SL)DUP(Y?)rm$",
                                              "^VMOV((NT)?)DQArm$",
                                              "^VMOVS(D|S)rm((_alt)?)$",
                                              "^VPBROADCAST(Q|DY)rm$",
                                              "^VPBROADCAST(D|QY)rm$")>;
def : InstRW<[RYLWriteResGroup58], (instrs LD_F80m,
                                           MOV16ao16,
                                           PREFETCHNTA,
                                           VBROADCASTSSYrm)>;

def RYLWriteResGroup59 : SchedWriteRes<[RYLPort16_17]>;
def : InstRW<[RYLWriteResGroup59], (instregex "^(LD|ST)_Frr$",
                                              "^(TST|XAM)_F$")>;
def : InstRW<[RYLWriteResGroup59], (instrs ST_FPrr)>;

def RYLWriteResGroup60 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup60], (instrs LEA16r)>;

def RYLWriteResGroup61 : SchedWriteRes<[RYLPort00_01_02_03_04_05]>;
def : InstRW<[RYLWriteResGroup61], (instrs LFENCE,
                                           NOOP,
                                           PREFETCHW,
                                           VZEROALL)>;

def RYLWriteResGroup62 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [1, 1, 1, 3];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup62], (instregex "^LGDT(16|32|64)m$")>;

def RYLWriteResGroup63 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [1, 1, 3];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup63], (instregex "^LIDT(16|32|64)m$")>;

def RYLWriteResGroup64 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort03_04_05]> {
  let ReleaseAtCycles = [10, 1];
  let Latency = 11;
  let NumMicroOps = 11;
}
def : InstRW<[RYLWriteResGroup64], (instrs LOOP)>;

def RYLWriteResGroup65 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort03_04_05]> {
  let ReleaseAtCycles = [12, 1];
  let Latency = 13;
  let NumMicroOps = 13;
}
def : InstRW<[RYLWriteResGroup65], (instregex "^LOOP(N?)E$")>;

def RYLWriteResGroup66 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [24, 1, 1, 2, 1];
  let Latency = 29;
  let NumMicroOps = 29;
}
def : InstRW<[RYLWriteResGroup66], (instregex "^LSL(16|32|64)rr$")>;

def RYLWriteResGroup67 : SchedWriteRes<[RYLPort07_09_11, RYLPort20_21, RYLPort22_23_24]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup67], (instregex "^(V?)MASKMOVDQU((64)?)$")>;

def RYLWriteResGroup68 : SchedWriteRes<[RYLPort16_17_18_19, RYLPort18_19]> {
  let ReleaseAtCycles = [2, 1];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup68], (instregex "^VCVT(DQ|PS)2PDrr$",
                                              "^VCVT(T?)PD2DQrr$")>;
def : InstRW<[RYLWriteResGroup68], (instrs MMX_CVTPI2PSrr,
                                           VCVTPD2PSrr)>;

def RYLWriteResGroup69 : SchedWriteRes<[RYLPort00_01_02]> {
  let ReleaseAtCycles = [12];
  let Latency = 12;
  let NumMicroOps = 12;
}
def : InstRW<[RYLWriteResGroup69], (instregex "^MMX_MASKMOVQ((64)?)$",
                                              "^PUSH(E|G|S)S16$")>;

def RYLWriteResGroup70 : SchedWriteRes<[RYLPort18_19]>;
def : InstRW<[RYLWriteResGroup70], (instregex "^MMX_MOVQ2(DQ|FR64)rr$",
                                              "^(V?)MOVPQI2QIrr$",
                                              "^(V?)MOVZPQILo2PQIrr$",
                                              "^(V?)PADDQrr$",
                                              "^VMOVMSKP(D|S)rr$")>;
def : InstRW<[RYLWriteResGroup70], (instrs VPMOVMSKBrr)>;

def RYLWriteResGroup71 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05]> {
  let ReleaseAtCycles = [13, 3, 1, 1];
  let Latency = 18;
  let NumMicroOps = 18;
}
def : InstRW<[RYLWriteResGroup71], (instregex "^MONITOR(32|64)rrr$")>;

def RYLWriteResGroup72 : SchedWriteRes<[RYLPort07_09_11, RYLPort13_14_15]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup72], (instregex "^MOV(8|16|32)(mi|o16a)$",
                                              "^MOV(8|16|32|64)mr$",
                                              "^MOV(8|16|32|64)o(32|64)a$",
                                              "^MOVDIRI(32|64)$")>;
def : InstRW<[RYLWriteResGroup72], (instrs MOV64mi32,
                                           MOV8mr_NOREX,
                                           VMPTRSTm)>;

def RYLWriteResGroup73 : SchedWriteRes<[RYLPort00_01_02, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [12, 1, 1];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup73], (instrs MOV16ms)>;

def RYLWriteResGroup74 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [12, 1];
  let Latency = 13;
  let NumMicroOps = 13;
}
def : InstRW<[RYLWriteResGroup74], (instregex "^MOV(16|32|64)rs$")>;

def RYLWriteResGroup75 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_18_19_20_21]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup75], (instregex "^MOV(QI2|64to)PQIrm$",
                                              "^MOV(D|SH|SL)DUPrm$")>;
def : InstRW<[RYLWriteResGroup75], (instrs MOVDI2PDIrm)>;
def : InstRW<[RYLWriteResGroup75, ReadAfterVecXLd], (instrs MOVLPDrm)>;
def : InstRW<[RYLWriteResGroup75, ReadAfterVecYLd], (instregex "^VINSERT(F|I)128rm$")>;

def RYLWriteResGroup76 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort07_09_11, RYLPort13_14_15]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup76], (instregex "^MOVBE(16|32|64)mr$",
                                              "^STOS(B|L|Q|W)$")>;

def RYLWriteResGroup77 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [2, 1, 1, 1, 1];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup77], (instregex "^MOVS(B|L|Q|W)$")>;

def RYLWriteResGroup78 : SchedWriteRes<[RYLPort18_19_20_21]>;
def : InstRW<[RYLWriteResGroup78], (instregex "^(V?)MOVS(D|S)rr((_REV)?)$")>;

def RYLWriteResGroup79 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 4, 1];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup79], (instregex "^MUL_F(32|64)m$",
                                              "^MUL_FI(16|32)m$")>;

def RYLWriteResGroup80 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [13, 5, 1];
  let Latency = 19;
  let NumMicroOps = 19;
}
def : InstRW<[RYLWriteResGroup80], (instrs MWAITrr)>;

def RYLWriteResGroup81 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [5, 2, 1, 1];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup81], (instregex "^OUTS(B|L|W)$")>;

def RYLWriteResGroup82 : SchedWriteRes<[RYLPort05, RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [1, 3];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup82], (instrs PAUSE)>;

def RYLWriteResGroup83 : SchedWriteRes<[RYLPort07_09_11, RYLPort13_14_15, RYLPort18_19]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup83], (instrs PEXTRBmr)>;

def RYLWriteResGroup84 : SchedWriteRes<[RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 4];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup84], (instregex "^(V?)PEXTRWrr((_REV)?)$")>;

def RYLWriteResGroup85 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [2, 2, 1, 1, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup85], (instregex "^POP(16|32|64)rmm$")>;

def RYLWriteResGroup86 : SchedWriteRes<[RYLPort16_17_18_19_20_21]>;
def : InstRW<[RYLWriteResGroup86], (instregex "^(V?)PS(L|R)LDQri$",
                                              "^VPS(L|R)LDQYri$")>;
def : InstRW<[RYLWriteResGroup86], (instrs VZEROUPPER)>;

def RYLWriteResGroup87 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup87], (instregex "^PUSH(16|32|64)rmm$")>;

def RYLWriteResGroup88 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_05, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [12, 1, 1, 1];
  let Latency = 15;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup88], (instregex "^PUSH(F|G)S(32|64)$")>;

def RYLWriteResGroup89 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let Latency = 4;
}
def : InstRW<[RYLWriteResGroup89, WriteRMW], (instregex "^RCL(8|16|32|64)m(1|i)$")>;

def RYLWriteResGroup90 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort22_23_24]> {
  let ReleaseAtCycles = [4, 11, 4];
  let Latency = 19;
  let NumMicroOps = 19;
}
def : InstRW<[RYLWriteResGroup90], (instregex "^RCL(8|16|32|64)rCL$")>;

def RYLWriteResGroup91 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [2, 1, 1];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup91], (instregex "^RCR(8|16|32|64)m(1|i)$")>;
def : InstRW<[RYLWriteResGroup91], (instrs WBINVD)>;

def RYLWriteResGroup92 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [3, 1, 1];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup92], (instregex "^RCR(8|16|32|64)mCL$")>;

def RYLWriteResGroup93 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [8];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup93], (instregex "^RCR(16|32|64)r(1|i)$")>;

def RYLWriteResGroup94 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [5];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup94], (instregex "^RCR8r(1|i|CL)$")>;
def : InstRW<[RYLWriteResGroup94], (instrs XSETBV)>;

def RYLWriteResGroup95 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [12, 6, 1];
  let Latency = 19;
  let NumMicroOps = 19;
}
def : InstRW<[RYLWriteResGroup95], (instrs RDMSR)>;

def RYLWriteResGroup96 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [12, 2];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup96], (instregex "^RDPID(32|64)$")>;

def RYLWriteResGroup97 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [1, 3];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup97], (instregex "^(RD|WR)PKRUr$")>;
def : InstRW<[RYLWriteResGroup97], (instrs TRAP)>;

def RYLWriteResGroup98 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [12, 20];
  let Latency = 32;
  let NumMicroOps = 32;
}
def : InstRW<[RYLWriteResGroup98], (instrs RDPMC)>;

def RYLWriteResGroup99 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [6, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup99], (instregex "^RDRAND(16|32|64)r$")>;

def RYLWriteResGroup100 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [4, 1];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup100], (instregex "^RDSEED(16|32|64)r$")>;

def RYLWriteResGroup101 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [12, 3];
  let Latency = 15;
  let NumMicroOps = 15;
}
def : InstRW<[RYLWriteResGroup101], (instrs RDTSC)>;

def RYLWriteResGroup102 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [24, 20];
  let Latency = 44;
  let NumMicroOps = 44;
}
def : InstRW<[RYLWriteResGroup102], (instrs RDTSCP)>;

def RYLWriteResGroup103 : SchedWriteRes<[RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup103], (instregex "^RET(16|32|64)$")>;

def RYLWriteResGroup104 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup104], (instregex "^RETI(16|32|64)$")>;

def RYLWriteResGroup105 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [37, 14, 1, 4, 2, 2];
  let Latency = 60;
  let NumMicroOps = 60;
}
def : InstRW<[RYLWriteResGroup105], (instregex "^S(G|I)DT(16|32|64)m$")>;

def RYLWriteResGroup106 : SchedWriteRes<[RYLPort16_17_18_19_20_21, RYLPort18_19]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup106], (instregex "^VPS(L|R)L(D|Q|W)rr$",
                                               "^VPSRA(D|W)rr$")>;
def : InstRW<[RYLWriteResGroup106], (instrs SHA1MSG1rr)>;

def RYLWriteResGroup107 : SchedWriteRes<[RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [6];
  let Latency = 6;
  let NumMicroOps = 6;
}
def : InstRW<[RYLWriteResGroup107], (instrs SHA1MSG2rr)>;

def RYLWriteResGroup108 : SchedWriteRes<[RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup108], (instrs SHA1NEXTErr)>;

def RYLWriteResGroup109 : SchedWriteRes<[RYLPort18_19]> {
  let ReleaseAtCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup109], (instregex "^SHA256MSG(1|2)rr$")>;
def : InstRW<[RYLWriteResGroup109], (instrs SHA1RNDS4rri)>;

def RYLWriteResGroup110 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 3];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup110, ReadAfterVecXLd], (instregex "^SHA256MSG(1|2)rm$")>;

def RYLWriteResGroup111 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [2, 1];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup111], (instrs STD)>;

def RYLWriteResGroup112 : SchedWriteRes<[RYLPort07_09_11, RYLPort22_23_24]> {
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup112], (instrs ST_FP80m)>;

def RYLWriteResGroup113 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [50, 1, 2];
  let Latency = 53;
  let NumMicroOps = 53;
}
def : InstRW<[RYLWriteResGroup113], (instrs SWAPGS)>;

def RYLWriteResGroup114 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [72, 27, 6, 9];
  let Latency = 114;
  let NumMicroOps = 114;
}
def : InstRW<[RYLWriteResGroup114], (instrs SYSCALL)>;

def RYLWriteResGroup115 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [48, 16, 2, 5];
  let Latency = 71;
  let NumMicroOps = 71;
}
def : InstRW<[RYLWriteResGroup115], (instrs SYSENTER)>;

def RYLWriteResGroup116 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort01_02, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [24, 34, 2, 2, 5];
  let Latency = 67;
  let NumMicroOps = 67;
}
def : InstRW<[RYLWriteResGroup116], (instregex "^SYSEXIT((64)?)$")>;

def RYLWriteResGroup117 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [24, 24, 3, 5];
  let Latency = 56;
  let NumMicroOps = 56;
}
def : InstRW<[RYLWriteResGroup117], (instregex "^SYSRET((64)?)$")>;

def RYLWriteResGroup118 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup118, ReadAfterVecXLd, ReadAfterVecXLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instregex "^VBLENDVP(D|S)rm$")>;
def : InstRW<[RYLWriteResGroup118, ReadAfterVecXLd, ReadAfterVecXLd, ReadDefault, ReadDefault, ReadDefault, ReadDefault, ReadDefault], (instrs VPBLENDVBrm)>;

def RYLWriteResGroup119 : SchedWriteRes<[RYLPort16_17_18_19_20_21]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup119], (instregex "^VBLENDVP(D|S)rr$")>;
def : InstRW<[RYLWriteResGroup119], (instrs VPBLENDVBrr)>;

def RYLWriteResGroup120 : SchedWriteRes<[RYLPort20_21]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup120], (instregex "^VINSERT(F|I)128rr$")>;
def : InstRW<[RYLWriteResGroup120], (instrs VBROADCASTSSrr,
                                            VPBROADCASTQrr)>;

def RYLWriteResGroup121 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 2, 1];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup121], (instregex "^VCVT(T?)PD2DQrm$")>;
def : InstRW<[RYLWriteResGroup121], (instrs VCVTPD2PSrm)>;
def : InstRW<[RYLWriteResGroup121, ReadAfterVecLd], (instregex "^VCVTSI642SSrm((_Int)?)$")>;

def RYLWriteResGroup122 : SchedWriteRes<[RYLPort00, RYLPort16_17_18_19, RYLPort18_19]> {
  let ReleaseAtCycles = [4, 2, 1];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup122, ReadDefault, ReadInt2Fpu], (instregex "^VCVTSI642SSrr((_Int)?)$")>;

def RYLWriteResGroup123 : SchedWriteRes<[RYLPort16_17_18_19]> {
  let ReleaseAtCycles = [2];
  let Latency = 2;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup123], (instregex "^VCVTSS2SDrr((_Int)?)$")>;

def RYLWriteResGroup124 : SchedWriteRes<[RYLPort16_17_18_19, RYLPort20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [2, 2, 4];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup124], (instregex "^VCVTTSS2SI((64)?)rr_Int$")>;
def : InstRW<[RYLWriteResGroup124, ReadDefault], (instregex "^VCVTTSS2SI((64)?)rr$")>;

def RYLWriteResGroup125 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort16_17_20_21, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 3, 4, 1];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup125, ReadAfterVecXLd], (instrs VDPPSrmi)>;

def RYLWriteResGroup126 : SchedWriteRes<[RYLPort16_17_18_19, RYLPort16_17_20_21, RYLPort18_19]> {
  let ReleaseAtCycles = [3, 4, 1];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup126], (instrs VDPPSrri)>;

def RYLWriteResGroup127 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 4, 2, 2, 8];
  let Latency = 17;
  let NumMicroOps = 17;
}
def : InstRW<[RYLWriteResGroup127, WriteVecMaskedGatherWriteback], (instregex "^VGATHERDP(S|DY)rm$",
                                                                              "^VPGATHERD(D|QY)rm$")>;

def RYLWriteResGroup128 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [2, 2, 1, 4];
  let Latency = 9;
  let NumMicroOps = 9;
}
def : InstRW<[RYLWriteResGroup128, WriteVecMaskedGatherWriteback], (instrs VGATHERDPDrm,
                                                                           VPGATHERDQrm)>;

def RYLWriteResGroup129 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 8, 2, 4, 16];
  let Latency = 31;
  let NumMicroOps = 31;
}
def : InstRW<[RYLWriteResGroup129, WriteVecMaskedGatherWriteback], (instrs VGATHERDPSYrm,
                                                                           VPGATHERDDYrm)>;

def RYLWriteResGroup130 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 4, 2, 2, 16];
  let Latency = 25;
  let NumMicroOps = 25;
}
def : InstRW<[RYLWriteResGroup130, WriteVecMaskedGatherWriteback], (instregex "^VGATHERQP(D|S)Yrm$")>;

def RYLWriteResGroup131 : SchedWriteRes<[RYLPort00_01_05, RYLPort06_08_10_12, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [1, 2, 2, 1, 8];
  let Latency = 14;
  let NumMicroOps = 14;
}
def : InstRW<[RYLWriteResGroup131, WriteVecMaskedGatherWriteback], (instregex "^VGATHERQP(D|S)rm$")>;

def RYLWriteResGroup132 : SchedWriteRes<[RYLPort06_08_10_12, RYLPort16_17_20_21, RYLPort18_19]> {
  let ReleaseAtCycles = [1, 2, 2];
  let Latency = 5;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup132, ReadAfterVecXLd], (instregex "^VH(ADD|SUB)P(D|S)rm$")>;

def RYLWriteResGroup133 : SchedWriteRes<[RYLPort16_17_20_21, RYLPort18_19]> {
  let ReleaseAtCycles = [2, 2];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup133], (instregex "^VH(ADD|SUB)P(D|S)rr$")>;

def RYLWriteResGroup134 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort16_17]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup134], (instrs VLDMXCSR)>;

def RYLWriteResGroup135 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [567, 128, 22, 54, 66, 66];
  let Latency = 903;
  let NumMicroOps = 903;
}
def : InstRW<[RYLWriteResGroup135], (instrs VMCALL)>;

def RYLWriteResGroup136 : SchedWriteRes<[RYLPort00]> {
  let ReleaseAtCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup136], (instregex "^VMOV64to(SD|PQI)rr$",
                                               "^VMOVDI2(SS|PDI)rr$")>;

def RYLWriteResGroup137 : SchedWriteRes<[RYLPort22_23_24]> {
  let ReleaseAtCycles = [4];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup137], (instregex "^VMOVMSKP(D|S)Yrr$")>;

def RYLWriteResGroup138 : SchedWriteRes<[RYLPort18_19, RYLPort20_21]> {
  let ReleaseAtCycles = [1, 2];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup138], (instrs VMPSADBWrri)>;

def RYLWriteResGroup139 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05, RYLPort07_09_11, RYLPort13_14_15]> {
  let ReleaseAtCycles = [5, 1, 1, 1];
  let Latency = 8;
  let NumMicroOps = 8;
}
def : InstRW<[RYLWriteResGroup139], (instregex "^VMREAD(32|64)mr$")>;

def RYLWriteResGroup140 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [3, 1];
  let Latency = 4;
  let NumMicroOps = 4;
}
def : InstRW<[RYLWriteResGroup140], (instregex "^VMREAD(32|64)rr$")>;

def RYLWriteResGroup141 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12]> {
  let ReleaseAtCycles = [1, 5, 3, 1];
  let Latency = 10;
  let NumMicroOps = 10;
}
def : InstRW<[RYLWriteResGroup141], (instregex "^VMWRITE(32|64)rm$")>;

def RYLWriteResGroup142 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort05]> {
  let ReleaseAtCycles = [5, 2];
  let Latency = 7;
  let NumMicroOps = 7;
}
def : InstRW<[RYLWriteResGroup142], (instregex "^VMWRITE(32|64)rr$")>;

def RYLWriteResGroup143 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort22_23_24]> {
  let ReleaseAtCycles = [8, 1, 1, 12, 4];
  let Latency = 26;
  let NumMicroOps = 26;
}
def : InstRW<[RYLWriteResGroup143, ReadAfterVecXLd], (instrs VPCMPESTRIrm)>;

def RYLWriteResGroup144 : SchedWriteRes<[RYLPort00, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17_18_19, RYLPort16_17_18_19_20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [8, 1, 1, 1, 12, 1, 4];
  let Latency = 28;
  let NumMicroOps = 28;
}
def : InstRW<[RYLWriteResGroup144, ReadAfterVecXLd], (instrs VPCMPESTRMrm)>;

def RYLWriteResGroup145 : SchedWriteRes<[RYLPort07_09_11, RYLPort18_19_20_21, RYLPort22_23_24]> {
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup145], (instrs VSTMXCSR)>;

def RYLWriteResGroup146 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11]> {
  let ReleaseAtCycles = [1, 10, 6, 1, 1];
  let Latency = 19;
  let NumMicroOps = 19;
}
def : InstRW<[RYLWriteResGroup146], (instrs WRMSR)>;

def RYLWriteResGroup147 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [5];
  let Latency = 8;
  let NumMicroOps = 5;
}
def : InstRW<[RYLWriteResGroup147, WriteRMW], (instregex "^XADD(8|16|32|64)rm$")>;

def RYLWriteResGroup148 : SchedWriteRes<[RYLPort00_01_02_03_04_05]> {
  let ReleaseAtCycles = [2];
  let Latency = 5;
  let NumMicroOps = 2;
}
def : InstRW<[RYLWriteResGroup148, WriteRMW], (instregex "^XCHG(8|16|32|64)rm$")>;

def RYLWriteResGroup149 : SchedWriteRes<[RYLPort16_17]> {
  let ReleaseAtCycles = [3];
  let Latency = 3;
  let NumMicroOps = 3;
}
def : InstRW<[RYLWriteResGroup149], (instrs XCH_F)>;

def RYLWriteResGroup150 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [108, 217, 14, 29, 103, 13, 72, 96, 4];
  let Latency = 656;
  let NumMicroOps = 656;
}
def : InstRW<[RYLWriteResGroup150], (instrs XRSTOR)>;

def RYLWriteResGroup151 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [132, 243, 16, 38, 102, 13, 64, 96, 4];
  let Latency = 708;
  let NumMicroOps = 708;
}
def : InstRW<[RYLWriteResGroup151], (instrs XRSTOR64)>;

def RYLWriteResGroup152 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort18_19, RYLPort18_19_20_21, RYLPort20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [168, 110, 3, 7, 48, 17, 17, 3, 18, 2, 12];
  let Latency = 405;
  let NumMicroOps = 405;
}
def : InstRW<[RYLWriteResGroup152], (instrs XRSTORS)>;

def RYLWriteResGroup153 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort16_17, RYLPort16_17_18_19_20_21, RYLPort20_21, RYLPort22_23_24]> {
  let ReleaseAtCycles = [144, 221, 2, 41, 100, 13, 64, 96, 4];
  let Latency = 685;
  let NumMicroOps = 685;
}
def : InstRW<[RYLWriteResGroup153], (instrs XRSTORS64)>;

def RYLWriteResGroup154 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 162, 4, 23, 1, 152, 10, 2, 142];
  let Latency = 508;
  let NumMicroOps = 508;
}
def : InstRW<[RYLWriteResGroup154], (instrs XSAVE)>;

def RYLWriteResGroup155 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 216, 5, 28, 1, 186, 10, 4, 176];
  let Latency = 638;
  let NumMicroOps = 638;
}
def : InstRW<[RYLWriteResGroup155], (instrs XSAVE64)>;

def RYLWriteResGroup156 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 185, 3, 22, 1, 169, 10, 2, 159];
  let Latency = 563;
  let NumMicroOps = 563;
}
def : InstRW<[RYLWriteResGroup156], (instrs XSAVEC)>;

def RYLWriteResGroup157 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [24, 234, 5, 27, 1, 183, 10, 4, 173];
  let Latency = 661;
  let NumMicroOps = 661;
}
def : InstRW<[RYLWriteResGroup157], (instrs XSAVEC64)>;

def RYLWriteResGroup158 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [138, 3, 12, 1, 156, 10, 2, 146];
  let Latency = 468;
  let NumMicroOps = 468;
}
def : InstRW<[RYLWriteResGroup158], (instrs XSAVEOPT)>;

def RYLWriteResGroup159 : SchedWriteRes<[RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [152, 4, 15, 1, 156, 10, 2, 146];
  let Latency = 486;
  let NumMicroOps = 486;
}
def : InstRW<[RYLWriteResGroup159], (instrs XSAVEOPT64)>;

def RYLWriteResGroup160 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [12, 178, 2, 21, 1, 169, 10, 2, 159];
  let Latency = 554;
  let NumMicroOps = 554;
}
def : InstRW<[RYLWriteResGroup160], (instrs XSAVES)>;

def RYLWriteResGroup161 : SchedWriteRes<[RYLPort00_01_02, RYLPort00_01_02_03_04_05, RYLPort00_01_05, RYLPort05, RYLPort06_08_10_12, RYLPort07_09_11, RYLPort13_14_15, RYLPort16_17, RYLPort22_23_24]> {
  let ReleaseAtCycles = [24, 227, 4, 26, 1, 183, 10, 4, 173];
  let Latency = 652;
  let NumMicroOps = 652;
}
def : InstRW<[RYLWriteResGroup161], (instrs XSAVES64)>;

}
#endif // INTEL_FEATURE_CPU_RYL
