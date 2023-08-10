// INTEL_FEATURE_XISA_COMMON
// REQUIRES: intel_feature_xisa_common,x86-registered-target

// Steps to fix this test if it fails:
// 1. Fix the vulnerable rules in X86EVEX2VEXTablesEmitter.cpp until the diff is reasonable
// 2. cp <generated_file>intel-x86-compress-evex-tables-xisa.inc
// 3. Add INTEL markup for intel-x86-compress-evex-tables-xisa.inc

// RUN: not diff -U 1 %p/intel-x86-compress-evex-tables.inc %llvm_lib_dir/Target/X86/X86GenEVEX2VEXTables.inc | sed "/---\|+++/d;s/@@.*/.../" >%t.diff
// RUN: grep -v "INTEL_FEATURE" %p/intel-x86-compress-evex-tables-xisa.inc >%t.ref
// RUN: diff --strip-trailing-cr %t.ref %t.diff

// end INTEL_FEATURE_XISA_COMMON
