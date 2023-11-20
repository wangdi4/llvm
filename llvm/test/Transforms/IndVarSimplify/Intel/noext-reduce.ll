; RUN: opt --passes="loop(indvars),instcombine" -S < %s | FileCheck %s

; CHECK: add nsw i32{{.*}}div.45.neg

; This test shows a problem with indvars (IndVarSimplify.cpp) causing
; a bad interaction with InstCombine, that causes no-wrap flags to be lost.
;
; After indvars, the code below becomes:
;
;  %0 = sext i32 %div.45.neg to i64
;  ....
;  %indvars.iv = phi i64 [ %indvars.iv.next, %do.end_do167 ], [ 0, %alloca_0 ]
;  %1 = add nsw i64 %indvars.iv, %0
;  %2 = trunc i64 %1 to i32
;  %"(double)sub.12$" = sitofp i32 %2 to double
;  call void @foo(i64 %indvars.iv)
;
; The shell_$J=phi, which is i32 type, is replaced with an i64.
; The add-operand %div.45.neg is sign-extended to i64, outside the loop.
; By itself, this transformation is OK. The nsw is preserved, and the ext
; has been moved outside the loop.
;
; But, when InstCombine runs on this code, it sees that the sext outside the
; loop only has 1 use. It makes this transformation:
;  %0 = sext i32 %div.45.neg to i64
;  %1 = add nsw i64 %indvars.iv, %0
;  %2 = trunc i64 %1 to i32
; =>
;  %0 = trunc i64 %indvars.iv to i32
;  %1 = add i32 %div.45.neg, %0
;
; The sext has been removed completely, by reassociating the sext/add/trunc
; into sext/trunc/add.
; The problem is that the nsw flag has been dropped. The reassociation cannot
; guarantee that the nsw i64 can be applied to i32.
;
; To fix this, we can either suppress the indvars widening transformation,
; or the InstCombine narrowing (which is in SimplifyDemandedUseBits)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @foo(i64 %val)

define void @shell_(i32 %"shell_$NY_fetch.89") {
alloca_0:
  %div.45.neg = sdiv i32 %"shell_$NY_fetch.89", -2
  br label %do.body161

do.body161:                                       ; preds = %do.end_do167, %alloca_0
  %"shell_$J.4" = phi i32 [ %add.81, %do.end_do167 ], [ 0, %alloca_0 ]
  br label %do.body166.preheader

do.body166.preheader:                             ; preds = %do.body161
  %sub.12 = add nsw i32 %"shell_$J.4", %div.45.neg
  %"(double)sub.12$" = sitofp i32 %sub.12 to double
  %int_sext442 = zext nneg i32 %"shell_$J.4" to i64
  call void @foo(i64 %int_sext442)
  %rel.121 = fcmp ugt double %"(double)sub.12$", 0.000000e+00
  br i1 %rel.121, label %do.end_do167, label %bb_new172_then

bb_new172_then:                                   ; preds = %do.body166.preheader
  br label %do.end_do167

do.end_do167:                                     ; preds = %bb_new172_then, %do.body166.preheader
  %add.81 = add nsw i32 %"shell_$J.4", 1
  br label %do.body161
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
