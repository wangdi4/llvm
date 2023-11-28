// INTEL_CUSTOMIZATION
// INTEL_FEATURE_XISA_COMMON
// UNSUPPORTED: intel_feature_xisa_common
// end INTEL_FEATURE_XISA_COMMON
// end INTEL_CUSTOMIZATION

// Steps to fix this test if it fails:
// 1. Fix the vulnerable rules in X86EVEX2VEXTablesEmitter.cpp until the diff is reasonable
// 2. cp <generated_file> intel-x86-compress-evex-tables.inc

// RUN: llvm-tblgen -DINTEL_CUSTOMIZATION -gen-x86-EVEX2VEX-tables %p/../../lib/Target/X86/X86.td -I %p/../../lib/Target/X86 -I %p/../../include -o %t
// RUN: diff %p/intel-x86-compress-evex-tables.inc %t
