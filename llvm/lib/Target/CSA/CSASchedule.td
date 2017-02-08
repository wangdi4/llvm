//===-- CSASchedule.td -- CSA Scheduling Definitions -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Functional units across CSA implementations.
// See CSA Simulator csa.h - UNIT_LIST
//
// Note that this information is provided in itinerary form, but isn't actually
// intended for "instruction scheduling".
//===----------------------------------------------------------------------===//

// NOTE: This enumeration is effectively duplicated in CSAInstrInfo.h
// for now.  Eventually, that information will hopefully just rely on
// information here.  Note that the CSAInstrInfo.h copy doesn't have
// the FU_ prefix.  Here, it is needed because of collisions with things
// like FMA and DIV with generic opcodes.
def FU_VIR : FuncUnit; // Virtual unit - doesn't really exist
def FU_ALU : FuncUnit; // Integer arithmetic and logical
def FU_SHF : FuncUnit; // Shift unit
def FU_IMA : FuncUnit; // Integer multiply/accumulate
def FU_FMA : FuncUnit; // Floating Multiply Accumulate
def FU_FCM : FuncUnit; // Floating point comparisons
def FU_CFI : FuncUnit; // Conversion to Floating from Integer
def FU_CIF : FuncUnit; // Conversion to Integer of Floating
def FU_DIV : FuncUnit; // Division
def FU_MEM : FuncUnit; // Memory access
def FU_SXU : FuncUnit; // Sequential eXecution Unit
def FU_SPD : FuncUnit; // Scratchpad

//===----------------------------------------------------------------------===//
// Instruction Itinerary classes used for CSA
// See CSA Simulator csa_asim_bridge.h - OPTYPE_LIST
//===----------------------------------------------------------------------===//
def IIPseudo    : InstrItinClass;
def IIVir       : InstrItinClass;
def IIALU       : InstrItinClass;
def IISAdd      : InstrItinClass;
def IIShft      : InstrItinClass;
def IICmpF      : InstrItinClass;
def IIAddF16    : InstrItinClass;
def IIAddF32    : InstrItinClass;
def IIAddF64    : InstrItinClass;
def IIMulI8     : InstrItinClass;
def IIMulI16    : InstrItinClass;
def IIMulI32    : InstrItinClass;
def IIMulI64    : InstrItinClass;
def IIMulF16    : InstrItinClass;
def IIMulF32    : InstrItinClass;
def IIMulF64    : InstrItinClass;
def IIFMAF16    : InstrItinClass;
def IIFMAF32    : InstrItinClass;
def IIFMAF64    : InstrItinClass;
def IIDivI8     : InstrItinClass;
def IIDivI16    : InstrItinClass;
def IIDivI32    : InstrItinClass;
def IIDivI64    : InstrItinClass;
def IIDivF16    : InstrItinClass;
def IIDivF32    : InstrItinClass;
def IIDivF64    : InstrItinClass;
def IISqrtF16   : InstrItinClass;
def IISqrtF32   : InstrItinClass;
def IISqrtF64   : InstrItinClass;
def IIMathF16   : InstrItinClass;
def IIMathF32   : InstrItinClass;
def IIMathF64   : InstrItinClass;
def IIRcpAF32   : InstrItinClass;
def IIRcpAF64   : InstrItinClass;
def IIRSqrtAF32 : InstrItinClass;
def IIRSqrtAF64 : InstrItinClass;
def IICvtIF     : InstrItinClass;
def IICvtFI     : InstrItinClass;
def IICvtFF     : InstrItinClass;
def IILD        : InstrItinClass;
def IIST        : InstrItinClass;
def IIATM       : InstrItinClass;
def IISeq       : InstrItinClass;
def IICtl       : InstrItinClass;


//===----------------------------------------------------------------------===//
// CSA Generic instruction itineraries.
//===----------------------------------------------------------------------===//

// Note: This information is currently replicated in CSAAllocUnitPass.cpp
//
def CSAGenericItineraries :
ProcessorItineraries<
  [FU_VIR, FU_ALU, FU_SHF, FU_IMA, FU_FMA, FU_FCM, FU_CFI, FU_CIF, FU_DIV, FU_MEM, FU_SXU, FU_SPD], [], [

  InstrItinData<IIPseudo    , [InstrStage<1, [FU_ALU]>]>,
  InstrItinData<IIVir       , [InstrStage<1, [FU_VIR]>]>,
  InstrItinData<IIALU       , [InstrStage<1, [FU_ALU]>]>,
  InstrItinData<IISAdd      , [InstrStage<1, [FU_ALU]>]>,
  InstrItinData<IIShft      , [InstrStage<1, [FU_SHF]>]>,
  InstrItinData<IICmpF      , [InstrStage<1, [FU_FCM]>]>,
  InstrItinData<IIAddF16    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIAddF32    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIAddF64    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIMulI8     , [InstrStage<2, [FU_IMA]>]>,
  InstrItinData<IIMulI16    , [InstrStage<2, [FU_IMA]>]>,
  InstrItinData<IIMulI32    , [InstrStage<2, [FU_IMA]>]>,
  InstrItinData<IIMulI64    , [InstrStage<2, [FU_IMA]>]>,
  InstrItinData<IIMulF16    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIMulF32    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIMulF64    , [InstrStage<2, [FU_FMA]>]>,
  InstrItinData<IIFMAF16    , [InstrStage<3, [FU_FMA]>]>,
  InstrItinData<IIFMAF32    , [InstrStage<3, [FU_FMA]>]>,
  InstrItinData<IIFMAF64    , [InstrStage<3, [FU_FMA]>]>,
  InstrItinData<IIDivI8     , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IIDivI16    , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IIDivI32    , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IIDivI64    , [InstrStage<7, [FU_DIV]>]>,
  InstrItinData<IIDivF16    , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IIDivF32    , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IIDivF64    , [InstrStage<5, [FU_DIV]>]>,
  InstrItinData<IISqrtF16   , [InstrStage<8, [FU_DIV]>]>,
  InstrItinData<IISqrtF32   , [InstrStage<8, [FU_DIV]>]>,
  InstrItinData<IISqrtF64   , [InstrStage<8, [FU_DIV]>]>,
  InstrItinData<IIMathF16   , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIMathF32   , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIMathF64   , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIRcpAF32   , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIRcpAF64   , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIRSqrtAF32 , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IIRSqrtAF64 , [InstrStage<24,[FU_DIV]>]>,
  InstrItinData<IICvtIF     , [InstrStage<1, [FU_CIF]>]>,
  InstrItinData<IICvtFI     , [InstrStage<1, [FU_CFI]>]>,
  InstrItinData<IICvtFF     , [InstrStage<1, [FU_FMA]>]>,
  InstrItinData<IILD        , [InstrStage<12,[FU_MEM]>]>,
  InstrItinData<IIST        , [InstrStage<4, [FU_MEM]>]>,
  InstrItinData<IIATM       , [InstrStage<12,[FU_MEM]>]>,
  InstrItinData<IISeq       , [InstrStage<1, [FU_ALU]>]>,
  InstrItinData<IICtl       , [InstrStage<1, [FU_SXU]>]>

]>;

def CSAGeneric : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = CSAGenericItineraries;
}

def CSAConfig0 : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = CSAGenericItineraries;
}

def CSAConfig1 : SchedMachineModel {
  let IssueWidth = 1;   // This is for the SXU only
  let LoadLatency = 1;  // If L1 for SXU?
  let HighLatency = 30;
  let PostRAScheduler = 0;
  let Itineraries = CSAGenericItineraries;
}
