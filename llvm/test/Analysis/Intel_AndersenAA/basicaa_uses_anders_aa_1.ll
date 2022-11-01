; CMPLRLLVM-25949: This test verifies that BasicAA tries to take advantage of
; AndersAA’s results if AndersAA has a better response. This LIT test is
; a little bit more complicated than the other AA LIT tests. AndersAA goes
; conservative for the new instructions that are created after AndersAA
; results are computed. First, this test runs AndersAA and then runs SROA pass
; intentionally to create new IR instructions. SROA will create new instruction
; like below after eliminating local variable. AndersAA alone can’t tell
; %local_var.0 and %G3 are NoAlias since AndersAA doesn’t know anything about
; %local_var.0, which is created after AndersAA results are computed. BasicAA
; can walk through PHI operands recursively and take advantage of AndersAA to
; detect that %local_var.0 and %G3 are NoAlias.  AndersAA can help BasicAA to
; find that %G1 and %G2 are NoAlias with %G3. This should be same when
; NeedLoopCarried flag is true or false.
;
; L2:
;  %local_var.0 = phi double* [ %G1, %L0 ], [ %G2, %L1 ]

; RUN: opt < %s -passes='require<anders-aa>,function(sroa),function(aa-eval)' -aa-pipeline=basic-aa,anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(sroa),function(aa-eval)' -aa-pipeline=basic-aa,anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK: NoAlias:     double* %G3, double* %local_var.0

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
  %G2 = load double*, double** @sumdeijda
  store double* %G2, double** %local_var
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
