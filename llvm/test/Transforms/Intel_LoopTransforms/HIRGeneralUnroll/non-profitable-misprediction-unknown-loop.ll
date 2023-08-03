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

%struct.__dirstream = type { ptr, ptr, ptr, i64, i16, i8, i8 }
%struct.cop.243 = type { ptr, ptr, ptr, i64, i16, i8, i8, i32, ptr, ptr, i32, i32, ptr, ptr }
%struct.hv = type { ptr, i32, i32, %union.anon }
%struct.xpvhv = type { ptr, %union._xmgu, i64, i64 }
%union._xmgu = type { ptr }
%struct.magic = type { ptr, ptr, i16, i8, i8, i64, ptr, ptr }
%struct.mgvtbl = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.clone_params = type { ptr, i64, ptr, ptr, ptr }
%struct.interpreter = type { i8 }
%struct.av = type { ptr, i32, i32, %union.anon }
%struct.xpvav = type { ptr, %union._xmgu, i64, i64, ptr }
%struct.sv = type { ptr, i32, i32, %union.anon }
%union.anon = type { ptr }
%struct.gv = type { ptr, i32, i32, %union.anon }
%struct.xpvgv = type { ptr, %union._xmgu, i64, %union.anon.16, %union._xivu, %union._xnvu }
%union.anon.16 = type { i64 }
%union._xivu = type { i64 }
%union._xnvu = type { double }
%struct.refcounted_he = type { ptr, ptr, %union.anon.6.59, i32, [1 x i8] }
%struct.hek = type { i32, i32, [1 x i8] }
%union.anon.6.59 = type { i64 }
%struct.op = type { ptr, ptr, ptr, i64, i16, i8, i8 }

@PL_ppaddr = external hidden unnamed_addr constant [396 x ptr], align 16
@PL_sig_pending = external hidden unnamed_addr global i32, align 4
@PL_op = external hidden global ptr, align 8
@PL_tainted = external hidden global i8, align 1
@PL_curcop = external hidden global ptr, align 8

declare hidden ptr @Perl_do_kv()

declare hidden fastcc void @Perl_despatch_signals() unnamed_addr

declare hidden ptr @Perl_pp_sort()

define hidden i32 @Perl_runops_standard() {
  %1 = load ptr, ptr @PL_op, align 8
  br label %2

; <label>:2:                                      ; preds = %24, %0
  %3 = phi ptr [ %1, %0 ], [ %25, %24 ]
  %4 = getelementptr inbounds %struct.op, ptr %3, i64 0, i32 2
  %5 = load ptr, ptr %4, align 8
  %6 = icmp eq ptr %5, @io_pp_nextstate
  br i1 %6, label %7, label %22

; <label>:7:                                      ; preds = %2
  %8 = load i64, ptr @PL_curcop, align 8
  %9 = getelementptr inbounds %struct.op, ptr %3, i64 0, i32 4
  %10 = load i16, ptr %9, align 8
  %11 = and i16 %10, 511
  %12 = zext i16 %11 to i64
  %13 = getelementptr inbounds [0 x ptr], ptr @PL_ppaddr, i64 0, i64 %12
  %14 = load ptr, ptr %13, align 8
  %15 = icmp eq ptr %14, @Perl_do_kv
  br i1 %15, label %16, label %18

; <label>:16:                                     ; preds = %7
  %17 = call ptr @Perl_do_kv()
  br label %20

; <label>:18:                                     ; preds = %7
  %19 = call ptr @Perl_pp_sort()
  br label %20

; <label>:20:                                     ; preds = %18, %16
  %21 = phi ptr [ %17, %16 ], [ %19, %18 ]
  store i64 %8, ptr @PL_curcop, align 8
  br label %24

; <label>:22:                                     ; preds = %2
  %23 = call ptr %5()
  br label %24

; <label>:24:                                     ; preds = %22, %20
  %25 = phi ptr [ %21, %20 ], [ %23, %22 ]
  store ptr %25, ptr @PL_op, align 8
  %26 = icmp eq ptr %25, null
  br i1 %26, label %27, label %2

; <label>:27:                                     ; preds = %24
  %28 = load i32, ptr @PL_sig_pending, align 4
  %29 = icmp eq i32 %28, 0
  br i1 %29, label %31, label %30

; <label>:30:                                     ; preds = %27
  tail call fastcc void @Perl_despatch_signals()
  br label %31

; <label>:31:                                     ; preds = %30, %27
  store i8 0, ptr @PL_tainted, align 1
  ret i32 0
}

declare hidden ptr @io_pp_nextstate()


