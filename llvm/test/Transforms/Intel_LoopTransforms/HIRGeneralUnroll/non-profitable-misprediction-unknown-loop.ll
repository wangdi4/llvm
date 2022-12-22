; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Verify that the unknown loop is not unrolled due to presence of ifs and indirect calls pushing the loop's cost above threashold.
; This loop in perbench causes degradations due to increase in mispredictions.

; HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   %2:
; |   %5 = (%3)[0].2;
; |   if (%5 == @io_pp_nextstate)
; |   {
; |      %8 = (i64*)(@PL_curcop)[0];
; |      %10 = (%3)[0].4;
; |      %14 = (bitcast ([396 x %struct.__dirstream* ()*]* @PL_ppaddr to [0 x %struct.op* ()*]*))[0][trunc.i16.i9(%10)];
; |      if (%14 == @Perl_do_kv)
; |      {
; |         %17 = @Perl_do_kv();
; |         %21 = &((%17)[0]);
; |      }
; |      else
; |      {
; |         %19 = @Perl_pp_sort();
; |         %21 = &((%19)[0]);
; |      }
; |      (i64*)(@PL_curcop)[0] = %8;
; |      %25 = &((%21)[0]);
; |   }
; |   else
; |   {
; |      %23 = %5();
; |      %25 = &((%23)[0]);
; |   }
; |   (%struct.op**)(@PL_op)[0] = &((%25)[0]);
; |   %3 = &((%25)[0]);
; |   if (&((%25)[0]) != null)
; |   {
; |      <i1 = i1 + 1>
; |      goto %2;
; |   }
; + END LOOP

; CHECK-NOT: modified
; CHECK: UNKNOWN LOOP i1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__dirstream = type { %struct.__dirstream*, %struct.__dirstream*, {}*, i64, i16, i8, i8 }
%struct.cop.243 = type { %struct.__dirstream*, %struct.__dirstream*, %struct.__dirstream* ()*, i64, i16, i8, i8, i32, %struct.hv*, %struct.gv*, i32, i32, i64*, %struct.refcounted_he* }
%struct.hv = type { %struct.xpvhv*, i32, i32, %union.anon }
%struct.xpvhv = type { %struct.hv*, %union._xmgu, i64, i64 }
%union._xmgu = type { %struct.magic* }
%struct.magic = type { %struct.magic*, %struct.mgvtbl*, i16, i8, i8, i64, %struct.sv*, i8* }
%struct.mgvtbl = type { i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*)*, i32 (%struct.sv*, %struct.magic*, %struct.sv*, i8*, i32)*, i32 (%struct.magic*, %struct.clone_params*)*, i32 (%struct.sv*, %struct.magic*)* }
%struct.clone_params = type { %struct.av*, i64, %struct.interpreter*, %struct.interpreter*, %struct.av* }
%struct.interpreter = type { i8 }
%struct.av = type { %struct.xpvav*, i32, i32, %union.anon }
%struct.xpvav = type { %struct.hv*, %union._xmgu, i64, i64, %struct.sv** }
%struct.sv = type { i8*, i32, i32, %union.anon }
%union.anon = type { i8* }
%struct.gv = type { %struct.xpvgv*, i32, i32, %union.anon }
%struct.xpvgv = type { %struct.hv*, %union._xmgu, i64, %union.anon.16, %union._xivu, %union._xnvu }
%union.anon.16 = type { i64 }
%union._xivu = type { i64 }
%union._xnvu = type { double }
%struct.refcounted_he = type { %struct.refcounted_he*, %struct.hek*, %union.anon.6.59, i32, [1 x i8] }
%struct.hek = type { i32, i32, [1 x i8] }
%union.anon.6.59 = type { i64 }
%struct.op = type { %struct.op*, %struct.op*, %struct.op* ()*, i64, i16, i8, i8 }

@PL_ppaddr = external hidden unnamed_addr constant [396 x %struct.__dirstream* ()*], align 16
@PL_sig_pending = external hidden unnamed_addr global i32, align 4
@PL_op = external hidden global %struct.__dirstream*, align 8
@PL_tainted = external hidden global i8, align 1
@PL_curcop = external hidden global %struct.cop.243*, align 8

declare hidden %struct.op* @Perl_do_kv()

declare hidden fastcc void @Perl_despatch_signals() unnamed_addr

declare hidden %struct.op* @Perl_pp_sort()

define hidden i32 @Perl_runops_standard() {
  %1 = load %struct.op*, %struct.op** bitcast (%struct.__dirstream** @PL_op to %struct.op**), align 8
  br label %2

; <label>:2:                                      ; preds = %24, %0
  %3 = phi %struct.op* [ %1, %0 ], [ %25, %24 ]
  %4 = getelementptr inbounds %struct.op, %struct.op* %3, i64 0, i32 2
  %5 = load %struct.op* ()*, %struct.op* ()** %4, align 8
  %6 = icmp eq %struct.op* ()* %5, @io_pp_nextstate
  br i1 %6, label %7, label %22

; <label>:7:                                      ; preds = %2
  %8 = load i64, i64* bitcast (%struct.cop.243** @PL_curcop to i64*), align 8
  %9 = getelementptr inbounds %struct.op, %struct.op* %3, i64 0, i32 4
  %10 = load i16, i16* %9, align 8
  %11 = and i16 %10, 511
  %12 = zext i16 %11 to i64
  %13 = getelementptr inbounds [0 x %struct.op* ()*], [0 x %struct.op* ()*]* bitcast ([396 x %struct.__dirstream* ()*]* @PL_ppaddr to [0 x %struct.op* ()*]*), i64 0, i64 %12
  %14 = load %struct.op* ()*, %struct.op* ()** %13, align 8
  %15 = icmp eq %struct.op* ()* %14, @Perl_do_kv
  br i1 %15, label %16, label %18

; <label>:16:                                     ; preds = %7
  %17 = call %struct.op* @Perl_do_kv()
  br label %20

; <label>:18:                                     ; preds = %7
  %19 = call %struct.op* @Perl_pp_sort()
  br label %20

; <label>:20:                                     ; preds = %18, %16
  %21 = phi %struct.op* [ %17, %16 ], [ %19, %18 ]
  store i64 %8, i64* bitcast (%struct.cop.243** @PL_curcop to i64*), align 8
  br label %24

; <label>:22:                                     ; preds = %2
  %23 = call %struct.op* %5()
  br label %24

; <label>:24:                                     ; preds = %22, %20
  %25 = phi %struct.op* [ %21, %20 ], [ %23, %22 ]
  store %struct.op* %25, %struct.op** bitcast (%struct.__dirstream** @PL_op to %struct.op**), align 8
  %26 = icmp eq %struct.op* %25, null
  br i1 %26, label %27, label %2

; <label>:27:                                     ; preds = %24
  %28 = load i32, i32* @PL_sig_pending, align 4
  %29 = icmp eq i32 %28, 0
  br i1 %29, label %31, label %30

; <label>:30:                                     ; preds = %27
  tail call fastcc void @Perl_despatch_signals()
  br label %31

; <label>:31:                                     ; preds = %30, %27
  store i8 0, i8* @PL_tainted, align 1
  ret i32 0
}

declare hidden %struct.op* @io_pp_nextstate()


