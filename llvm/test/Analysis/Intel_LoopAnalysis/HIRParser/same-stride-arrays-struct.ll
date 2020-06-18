; RUN: opt < %s -hir-details-dims -hir-ssa-deconstruction -hir-framework -analyze | FileCheck %s
; RUN: opt < %s -hir-details-dims -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that "-1" and "0" array indices are parsed for appropriate dimensions.

; BEGIN REGION { }
;        + DO i1 = 0, smax(1, %n) + -1, 1   <DO_LOOP>
; CHECK: |   %tmp260 = (%tmp239)[0:-1 * i1 + -1:8(%struct.wombat*:0)].0.0[0:0:8([1 x double]:1)];
;        |   (%tmp260.out)[0:0:8(double*:0)] = %tmp260;
;        + END LOOP
; END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wombat = type { %struct.hoge }
%struct.hoge = type { [1 x double] }

; Function Attrs: nounwind uwtable
define dso_local void @blam.bb256(%struct.wombat* %tmp239, i64 %n, double* %tmp260.out) {
newFuncRoot:
  br label %bb256

bb264.exitStub:                                   ; preds = %bb256
  ret void

bb256:                                            ; preds = %newFuncRoot, %bb256
  %iv = phi i64 [ %iv.inc, %bb256 ], [ 0, %newFuncRoot ]
  %tmp257 = phi %struct.wombat* [ %tmp258, %bb256 ], [ %tmp239, %newFuncRoot ]
  %tmp258 = getelementptr inbounds %struct.wombat, %struct.wombat* %tmp257, i64 -1
  %tmp259 = getelementptr inbounds %struct.wombat, %struct.wombat* %tmp258, i64 0, i32 0, i32 0, i64 0
  %tmp260 = load double, double* %tmp259, align 8
  store double %tmp260, double* %tmp260.out
  %iv.inc = add i64 %iv, 1
  %tmp263 = icmp sgt i64 %n, %iv.inc
  br i1 %tmp263, label %bb256, label %bb264.exitStub
}

