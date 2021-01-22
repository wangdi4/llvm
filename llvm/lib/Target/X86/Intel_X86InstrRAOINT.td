#if INTEL_FEATURE_ISA_RAO_INT
//===---------------------*- tablegen -*-----------------------------------===//
//=- Intel_X86InstrRAOINT.td - RAOINT Instruction Set Extension -*- tablegen -*=//
//
//      Copyright (c) 2018-2021 Intel Corporation.
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
// This file describes the instructions that make up the Intel RAOINT instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// RAOINT instructions
multiclass RAOINT_BASE<bits<8> Opcode, string OpcodeStr,
                      Intrinsic Int32, Intrinsic Int64> {
  let Predicates = [HasRAOINT] in
    def 32mr : I<Opcode, MRMDestMem, (outs), (ins i32mem:$dst, GR32:$src),
                !strconcat(OpcodeStr, "{l}\t{$src, $dst|$dst, $src}"),
                [(Int32 addr:$dst, GR32:$src)]>;

  let Predicates = [HasRAOINT, In64BitMode] in
    def 64mr : I<Opcode, MRMDestMem, (outs), (ins i64mem:$dst, GR64:$src),
                !strconcat(OpcodeStr, "{q}\t{$src, $dst|$dst, $src}"),
                [(Int64 addr:$dst, GR64:$src)]>, REX_W;
}

let SchedRW = [WriteALURMW] in {
  defm AADD : RAOINT_BASE<0xfc, "aadd", int_x86_aadd32, int_x86_aadd64>, T8PS;
  defm AAND : RAOINT_BASE<0xfc, "aand", int_x86_aand32, int_x86_aand64>, T8PD;
  defm AOR  : RAOINT_BASE<0xfc, "aor",  int_x86_aor32,  int_x86_aor64>,  T8XD;
  defm AXOR : RAOINT_BASE<0xfc, "axor", int_x86_axor32, int_x86_axor64>, T8XS;
}

// FIXME: We can't select RAO when EFLAGS used.
class RAO_PatFrag<SDNode Node> : PatFrag<(ops node:$lhs, node:$rhs),
                         (Node node:$lhs, node:$rhs), [{
  MemSDNode *MN = cast<MemSDNode>(N);
  if (MN->hasAnyUseOfValue(0) ||
    MN->getAlignment() < MN->getMemoryVT().getStoreSize())
    return false;

  return Subtarget->hasMFence();
  }]>;

def raoint_add : RAO_PatFrag<X86lock_add>;
def raoint_and : RAO_PatFrag<X86lock_and>;
def raoint_or  : RAO_PatFrag<X86lock_or>;
def raoint_xor : RAO_PatFrag<X86lock_xor>;

let Predicates = [HasRAOINT] in {
  def : Pat<(raoint_add addr:$dst, GR32:$src), (AADD32mr addr:$dst, GR32:$src)>;
  def : Pat<(raoint_and addr:$dst, GR32:$src), (AAND32mr addr:$dst, GR32:$src)>;
  def : Pat<(raoint_or  addr:$dst, GR32:$src), (AOR32mr  addr:$dst, GR32:$src)>;
  def : Pat<(raoint_xor addr:$dst, GR32:$src), (AXOR32mr addr:$dst, GR32:$src)>;
}

let Predicates = [HasRAOINT, In64BitMode] in {
  def : Pat<(raoint_add addr:$dst, GR64:$src), (AADD64mr addr:$dst, GR64:$src)>;
  def : Pat<(raoint_and addr:$dst, GR64:$src), (AAND64mr addr:$dst, GR64:$src)>;
  def : Pat<(raoint_or  addr:$dst, GR64:$src), (AOR64mr  addr:$dst, GR64:$src)>;
  def : Pat<(raoint_xor addr:$dst, GR64:$src), (AXOR64mr addr:$dst, GR64:$src)>;
}
#endif // INTEL_FEATURE_ISA_RAO_INT
