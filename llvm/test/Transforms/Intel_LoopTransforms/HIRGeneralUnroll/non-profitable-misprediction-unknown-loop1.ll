; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" < %s 2>&1 | FileCheck %s

; Verify that the unknown loop with indirect call is not unrolled even though
; there is a reuse of a copy though the backedge because cloning indirect calls
; can increase misprediction.

; CHECK: BEGIN REGION { }
; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   %2:
; CHECK: |   %5 = (%3)[0].2;
; CHECK: |   %6 = %5();
; CHECK: |   (@PL_op)[0] = &((%6)[0]);
; CHECK: |   %3 = &((%6)[0]);
; CHECK: |   if (&((%6)[0]) != null)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto %2;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%struct.op.207 = type { ptr, ptr, ptr, i64, i16, i8, i8 }
%struct.interp_intern = type { ptr, ptr, i32, ptr, ptr, ptr, %struct.thread_intern, ptr, i32, i32, [16 x ptr] }
%struct.av = type { ptr, i32, i32, %union.anon.0 }
%struct.xpvav = type { ptr, %union._xmgu, i64, i64, ptr }
%struct.hv = type { ptr, i32, i32, %union.anon.0 }
%struct.xpvhv = type { ptr, %union._xmgu, i64, i64 }
%union._xmgu = type { ptr }
%struct.magic = type { ptr, ptr, i16, i8, i8, i64, ptr, ptr }
%struct.mgvtbl = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.clone_params = type { ptr, i64, ptr, ptr, ptr }
%struct.interpreter = type { i8 }
%struct.sv = type { ptr, i32, i32, %union.anon.0 }
%union.anon.0 = type { ptr }
%struct.child_tab = type { i32, [64 x i32], [64 x ptr] }
%struct.thread_intern = type { [512 x i8], %struct.servent, [128 x i8], i32, [30 x i8], i32, i16 }
%struct.servent = type { ptr, ptr, ptr, i16 }
%struct.HWND__ = type { i32 }
%struct.op = type { ptr, ptr, ptr, i64, i16, i8, i8 }

@PL_op = external hidden global ptr, align 8
@PL_tainted = external hidden global i8, align 1
@PL_sys_intern = external hidden global %struct.interp_intern, align 8

; Function Attrs: nounwind uwtable
define hidden i32 @Perl_runops_standard() {
  %1 = load ptr, ptr @PL_op, align 8
  br label %2

2:                                                ; preds = %2, %0
  %3 = phi ptr [ %1, %0 ], [ %6, %2 ]
  %4 = getelementptr inbounds %struct.op, ptr %3, i64 0, i32 2
  %5 = load ptr, ptr %4, align 8
  %6 = tail call ptr %5()
  store ptr %6, ptr @PL_op, align 8
  %7 = icmp eq ptr %6, null
  br i1 %7, label %8, label %2

8:                                                ; preds = %2
  %9 = load i32, ptr getelementptr inbounds (%struct.interp_intern, ptr @PL_sys_intern, i64 0, i32 9), align 4
  %10 = add i32 %9, 1
  store i32 %10, ptr getelementptr inbounds (%struct.interp_intern, ptr @PL_sys_intern, i64 0, i32 9), align 4
  store i8 0, ptr @PL_tainted, align 1
  ret i32 0
}

