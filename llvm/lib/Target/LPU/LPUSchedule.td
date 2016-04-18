//===-- LPUSchedule.td -- LPU Scheduling Definitions -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Functional units across LPU implementations.
// See LPU Simulator lpu.h - UNIT_LIST
//
// Note that this information is provided in itinerary form, but isn't actually
// intended for "instruction scheduling".
//===----------------------------------------------------------------------===//
def VIR : FuncUnit; // Virtual unit - doesn't really exist
def ALU : FuncUnit; // Integer arithmetic and logical
def SHF : FuncUnit; // Shift unit
def IMA : FuncUnit; // Integer multiply/accumulate
def FMA : FuncUnit; // Floating Multiply Accumulate
def FCM : FuncUnit; // Floating point comparisons
def CFI : FuncUnit; // Conversion to Floating from Integer
def CIF : FuncUnit; // Conversion to Integer of Floating
def DIV : FuncUnit; // Division
def MEM : FuncUnit; // Memory access
def SXU : FuncUnit; // Sequential eXecution Unit
def SPD : FuncUnit; // Scratchpad

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes used for LPU
// See LPU Simulator lpu_asim_bridge.h - OPTYPE_LIST
//===----------------------------------------------------------------------===//
def IIPseudo  : InstrItinClass;
def IIVir     : InstrItinClass;
def IIALU     : InstrItinClass;
def IISAdd    : InstrItinClass;
def IIShft    : InstrItinClass;
def IICmpF    : InstrItinClass;
def IIAddF16  : InstrItinClass;
def IIAddF32  : InstrItinClass;
def IIAddF64  : InstrItinClass;
def IIMulI8   : InstrItinClass;
def IIMulI16  : InstrItinClass;
def IIMulI32  : InstrItinClass;
def IIMulI64  : InstrItinClass;
def IIMulF16  : InstrItinClass;
def IIMulF32  : InstrItinClass;
def IIMulF64  : InstrItinClass;
def IIFMAF16  : InstrItinClass;
def IIFMAF32  : InstrItinClass;
def IIFMAF64  : InstrItinClass;
def IIDivI8   : InstrItinClass;
def IIDivI16  : InstrItinClass;
def IIDivI32  : InstrItinClass;
def IIDivI64  : InstrItinClass;
def IIDivF16  : InstrItinClass;
def IIDivF32  : InstrItinClass;
def IIDivF64  : InstrItinClass;
def IISqrtF16 : InstrItinClass;
def IISqrtF32 : InstrItinClass;
def IISqrtF64 : InstrItinClass;
def IIMathF16 : InstrItinClass;
def IIMathF32 : InstrItinClass;
def IIMathF64 : InstrItinClass;
def IICvtIF   : InstrItinClass;
def IICvtFI   : InstrItinClass;
def IICvtFF   : InstrItinClass;
def IILD      : InstrItinClass;
def IIST      : InstrItinClass;
def IISeq     : InstrItinClass;
def IICtl     : InstrItinClass;


//===----------------------------------------------------------------------===//
// LPU Generic instruction itineraries.
//===----------------------------------------------------------------------===//
def LPUGenericItineraries :
ProcessorItineraries<
  [VIR, ALU, SHF, IMA, FMA, FCM, CFI, CIF, DIV, MEM, SXU, SPD], [], [

  InstrItinData<IIPseudo  , [InstrStage<1, [ALU]>]>,
  InstrItinData<IIVir     , [InstrStage<1, [VIR]>]>,
  InstrItinData<IIALU     , [InstrStage<1, [ALU]>]>,
  InstrItinData<IISAdd    , [InstrStage<1, [ALU]>]>,
  InstrItinData<IIShft    , [InstrStage<1, [SHF]>]>,
  InstrItinData<IICmpF    , [InstrStage<1, [FCM]>]>,
  InstrItinData<IIAddF16  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIAddF32  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIAddF64  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIMulI8   , [InstrStage<2, [IMA]>]>,
  InstrItinData<IIMulI16  , [InstrStage<2, [IMA]>]>,
  InstrItinData<IIMulI32  , [InstrStage<2, [IMA]>]>,
  InstrItinData<IIMulI64  , [InstrStage<2, [IMA]>]>,
  InstrItinData<IIMulF16  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIMulF32  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIMulF64  , [InstrStage<2, [FMA]>]>,
  InstrItinData<IIFMAF16  , [InstrStage<3, [FMA]>]>,
  InstrItinData<IIFMAF32  , [InstrStage<3, [FMA]>]>,
  InstrItinData<IIFMAF64  , [InstrStage<3, [FMA]>]>,
  InstrItinData<IIDivI8   , [InstrStage<5, [DIV]>]>,
  InstrItinData<IIDivI16  , [InstrStage<5, [DIV]>]>,
  InstrItinData<IIDivI32  , [InstrStage<5, [DIV]>]>,
  InstrItinData<IIDivI64  , [InstrStage<7, [DIV]>]>,
  InstrItinData<IIDivF16  , [InstrStage<5, [DIV]>]>,
  InstrItinData<IIDivF32  , [InstrStage<5, [DIV]>]>,
  InstrItinData<IIDivF64  , [InstrStage<5, [DIV]>]>,
  InstrItinData<IISqrtF16 , [InstrStage<8, [DIV]>]>,
  InstrItinData<IISqrtF32 , [InstrStage<8, [DIV]>]>,
  InstrItinData<IISqrtF64 , [InstrStage<8, [DIV]>]>,
  InstrItinData<IIMathF16 , [InstrStage<24,[DIV]>]>,
  InstrItinData<IIMathF32 , [InstrStage<24,[DIV]>]>,
  InstrItinData<IIMathF64 , [InstrStage<24,[DIV]>]>,
  InstrItinData<IICvtIF   , [InstrStage<1, [CIF]>]>,
  InstrItinData<IICvtFI   , [InstrStage<1, [CFI]>]>,
  InstrItinData<IICvtFF   , [InstrStage<1, [FMA]>]>,
  InstrItinData<IILD      , [InstrStage<12,[MEM]>]>,
  InstrItinData<IIST      , [InstrStage<4, [MEM]>]>,
  InstrItinData<IISeq     , [InstrStage<1, [ALU]>]>,
  InstrItinData<IICtl     , [InstrStage<1, [SXU]>]>

]>;

def LPUGeneric : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = LPUGenericItineraries;
}

def LPUConfig0 : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = LPUGenericItineraries;
}

def LPUConfig1 : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = LPUGenericItineraries;
}
