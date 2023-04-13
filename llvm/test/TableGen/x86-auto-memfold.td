// INTEL_CUSTOMIZATION
// INTEL_FEATURE_XISA_COMMON
// UNSUPPORTED: intel_feature_xisa_common
// end INTEL_FEATURE_XISA_COMMON
// RUN: llvm-tblgen -DINTEL_CUSTOMIZATION -gen-x86-fold-tables -asmwriternum=1 %p/../../lib/Target/X86/X86.td -I %p/../../include -I %p/../../lib/Target/X86/ -I %p/../../include/ -I %p/../../lib/Target/ --write-if-changed  -o %t1
// end INTEL_CUSTOMIZATION
// RUN: cmp %p/../../lib/Target/X86/X86MemFoldTables.inc %t1
