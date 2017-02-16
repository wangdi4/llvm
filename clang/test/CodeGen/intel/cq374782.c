// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s
// This test fails in xmain. CQ 416723
// XFAIL: *

void foo(void * volatile *vpvp,
         long volatile *lvp,
         long *lp,
         void *vp,
         void volatile *vvp,
         long l) {
  _InterlockedCompareExchange(lvp, l, l);
// CHECK: {{%.* = cmpxchg volatile i64\* %.*, i64 %.*, i64 %.* seq_cst seq_cst}}
  _InterlockedCompareExchangePointer(vpvp, vp, vp);
// CHECK: {{%.* = cmpxchg volatile i64\* %.*, i64 %.*, i64 %.* seq_cst seq_cst}}
  _InterlockedExchangeAdd(lvp, l);
// CHECK: {{%.* = atomicrmw volatile add i64\* %.*, i64 %.* seq_cst}}
  _InterlockedExchange(lvp, l);
// CHECK: {{%.* = atomicrmw xchg i64\* %.*, i64 %.* seq_cst}}
  _InterlockedExchangePointer(vpvp, vp);
// CHECK: {{%.* = atomicrmw xchg i64\* %.*, i64 %.* seq_cst}}
}

