; CMPLRLLVM-26345: This test verifies that AndersAA uses operand of bitcast
; when AndersAA doesn't know about the bitcast.
; BasicAA tries to take advantage of AndersAA’s results if AndersAA has
; a better response. This LIT test is a little bit more complicated than
; the other AA LIT tests. AndersAA goes conservative for the new instructions
; that are created after AndersAA results are computed. First, this test runs
; AndersAA and then runs SROA pass intentionally to create new IR instructions.
; SROA will create %0 and %local_var.0 new instructions as below in the IR
; dump. AndersAA doesn't know anything about %local_var.0 and %0 since they
; are created by SROA after AndersAA ran. For %local_var.0 and %G3 locations,
; BasicAA can walk through PHI operands recursively and take advantage of
; AndersAA to detect that %local_var.0 and %G3 are NoAlias. BasicAA does the
; following queries for both operands of PHI node (%G1 and %0):
;
; Query 1 (%G1 and %G3 locations): AndersAA returns NoAlias
;
; Query 2 (%0 and %G3 locations): AndersAA doesn't know anything about
; %0 since it is created after Andersens results are computed. So, AndersAA
; uses operand of bitcast (i.e %call2) since it knows about it.
; It returns NoAlias in this case also.
;
; Using above two queries, BasicAA is able to find %G3 and %local_var.0 don't
; alias.
;
; IR dump after SROA:
;
; define dso_local i32 @foo(double* %Ptr, i32 %b) {
; L0:
;   %call1 = tail call noalias i8* @malloc(i64 1024)
;   store i8* %call1, i8** bitcast (double** @sumdeijda to i8**), align 8
;   %G1 = load double*, double** @sumdeijda, align 8
;   %cmp = icmp eq i32 %b, 0
;   br i1 %cmp, label %L2, label %L1
;
; L1:                                               ; preds = %L0
;   %call2 = tail call noalias i8* @malloc(i64 1024)
;   store i8* %call2, i8** bitcast (double** @sumdeijda to i8**), align 8
;   %0 = bitcast i8* %call2 to double*
;   br label %L2
;
; L2:                                               ; preds = %L1, %L0
;   %local_var.0 = phi double* [ %G1, %L0 ], [ %0, %L1 ]
;   %bc2 = bitcast double* %local_var.0 to i8*
;   call void @llvm.memset.p0i8.i64(i8* %bc2, i8 1, i64 1024, i1 false)
;   %call3 = tail call noalias i8* @malloc(i64 1024)
;   store i8* %call3, i8** bitcast (double** @sumdeijda to i8**), align 8
;   %G3 = load double*, double** @riff, align 8
;   ret i32 0
; }
;

; RUN: opt < %s -passes='require<anders-aa>,function(sroa),function(aa-eval)' -aa-pipeline=basic-aa,anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(sroa),function(aa-eval)' -aa-pipeline=basic-aa,anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; This is sufficient to check both incoming values of local_var.0.
; CHECK-DAG: NoAlias:     double* %G3, double* %local_var.0

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sumdeijda = internal unnamed_addr global double* null, align 8
@riff = internal unnamed_addr global double* null, align 8

define dso_local i32 @foo(double* %Ptr, i32 %b) {
L0:
  %local_var = alloca double*, align 8
  %call1 = tail call noalias i8* @malloc(i64 1024)
  store i8* %call1, i8** bitcast (double** @sumdeijda to i8**)
  %G1 = load double*, double** @sumdeijda
  store double* %G1, double** %local_var
  %cmp = icmp eq i32 %b, 0
  br i1 %cmp, label %L2, label %L1
L1:
  %call2 = tail call noalias i8* @malloc(i64 1024)
  store i8* %call2, i8** bitcast (double** @sumdeijda to i8**)
;  %G2 = load double*, double** @sumdeijda
  %bc0 = bitcast double** %local_var to i8**
  store i8* %call2, i8** %bc0
  br label %L2
L2:
  %ld2 = load double*, double** %local_var
  %bc2 = bitcast double* %ld2 to i8*
  call void @llvm.memset.p0i8.i64(i8* %bc2, i8 1, i64 1024, i1 false)
  %call3 = tail call noalias i8* @malloc(i64 1024)
  store i8* %call3, i8** bitcast (double** @sumdeijda to i8**)
  %G3 = load double*, double** @riff

; these loads are needed to trigger aa-eval
  %ld.G3 = load double, double* %G3, align 8
  %ld.ld2 = load double, double* %ld2

  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
