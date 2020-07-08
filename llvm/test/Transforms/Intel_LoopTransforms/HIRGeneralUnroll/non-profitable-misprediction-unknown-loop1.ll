; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -print-after=hir-general-unroll < %s 2>&1 | FileCheck %s
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
; CHECK: |   (%struct.op**)(@PL_op)[0] = &((%6)[0]);
; CHECK: |   %3 = &((%6)[0]);
; CHECK: |   if (&((%6)[0]) != null)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto %2;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%struct.op.207 = type { %struct.op.207*, %struct.op.207*, {}*, i64, i16, i8, i8 }
%struct.interp_intern = type { i8*, i8**, i32, %struct.av*, %struct.child_tab*, i8*, %struct.thread_intern, %struct.HWND__*, i32, i32, [16 x void (i32)*] }
%struct.av = type { %struct.xpvav*, i32, i32, %union.anon.0 }
%struct.xpvav = type { %struct.hv*, %union._xmgu, i64, i64, %struct.sv** }
%struct.hv = type { %struct.xpvhv*, i32, i32, %union.anon.0 }
%struct.xpvhv = type { %struct.hv*, %union._xmgu, i64, i64 }
%union._xmgu = type { %struct.magic* }
%struct.magic = type { %struct.magic*, %struct.mgvtbl*, i16, i8, i8, i64, %struct.sv*, i8* }
%struct.mgvtbl = type { i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*, %struct.sv*, i8*, i32)*, i32 (%struct.magic*, %struct.clone_params*)*, i32 (%struct.sv*, %struct.magic*)* }
%struct.clone_params = type { %struct.av*, i64, %struct.interpreter*, %struct.interpreter*, %struct.av* }
%struct.interpreter = type { i8 }
%struct.sv = type { i8*, i32, i32, %union.anon.0 }
%union.anon.0 = type { i8* }
%struct.child_tab = type { i32, [64 x i32], [64 x i8*] }
%struct.thread_intern = type { [512 x i8], %struct.servent, [128 x i8], i32, [30 x i8], i32, i16 }
%struct.servent = type { i8*, i8**, i8*, i16 }
%struct.HWND__ = type { i32 }
%struct.op = type { %struct.op*, %struct.op*, %struct.op* ()*, i64, i16, i8, i8 }

@PL_op = external hidden global %struct.op.207*, align 8
@PL_tainted = external hidden global i8, align 1
@PL_sys_intern = external hidden global %struct.interp_intern, align 8

; Function Attrs: nounwind uwtable
define hidden i32 @Perl_runops_standard() {
  %1 = load %struct.op*, %struct.op** bitcast (%struct.op.207** @PL_op to %struct.op**), align 8
  br label %2

2:                                                ; preds = %2, %0
  %3 = phi %struct.op* [ %1, %0 ], [ %6, %2 ]
  %4 = getelementptr inbounds %struct.op, %struct.op* %3, i64 0, i32 2
  %5 = load %struct.op* ()*, %struct.op* ()** %4, align 8
  %6 = tail call %struct.op* %5()
  store %struct.op* %6, %struct.op** bitcast (%struct.op.207** @PL_op to %struct.op**), align 8
  %7 = icmp eq %struct.op* %6, null
  br i1 %7, label %8, label %2

8:                                                ; preds = %2
  %9 = load i32, i32* getelementptr inbounds (%struct.interp_intern, %struct.interp_intern* @PL_sys_intern, i64 0, i32 9), align 4
  %10 = add i32 %9, 1
  store i32 %10, i32* getelementptr inbounds (%struct.interp_intern, %struct.interp_intern* @PL_sys_intern, i64 0, i32 9), align 4
  store i8 0, i8* @PL_tainted, align 1
  ret i32 0
}

