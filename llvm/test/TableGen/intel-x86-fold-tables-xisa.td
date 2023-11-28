// INTEL_FEATURE_XISA_COMMON
// REQUIRES: intel_feature_xisa_common,x86-registered-target

// Steps to fix this test if it fails:
//   1. Update X86ManualFoldTables.def to manually correct the entries in fold tables, or
//      fix the vulnerable rules in X86FoldTablesEmitter.cpp until the diff is reasonable
//   2. cp %t.diff intel-x86-fold-tables-xisa.inc
//   3. Add INTEL markup for intel-x86-fold-tables-xisa.inc

// RUN: grep -v "INTEL_CUSTOMIZATION" %p/x86-fold-tables.inc >%t.noxisa
// RUN: not diff -U 1 %t.noxisa %llvm_lib_dir/Target/X86/X86GenFoldTables.inc | sed "/---\|+++/d;s/@@.*/.../" >%t.diff
// RUN: grep -v "INTEL_FEATURE" %p/intel-x86-fold-tables-xisa.inc >%t.ref
// RUN: diff --strip-trailing-cr %t.ref %t.diff

// end INTEL_FEATURE_XISA_COMMON
