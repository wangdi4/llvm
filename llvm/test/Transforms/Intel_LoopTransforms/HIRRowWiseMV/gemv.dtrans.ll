; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtransanalysis -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -debug-only=hir-rowwise-mv -print-after=hir-rowwise-mv -disable-output 2>&1 < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Print before:

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:       |   |   %Aijbj = (%A)[128 * i1 + i2]  *  (%b)[i2];
; CHECK:       |   |   %sum = %sum  +  %Aijbj;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Debug output:

; CHECK: Identified these candidates for multiversioning:
; CHECK:   (%b)[i2]:
; CHECK:     double -1.000000e+00
; CHECK:     double 0.000000e+00
; CHECK-NOT: double 1.000000e+00

; (1.0 is not expected here because there are no assignments of 1.0 to the array
; in structinit below)

; Print after:

; TBI

%struct.BInfo = type { i32, double* }

declare noalias i8* @malloc(i64)

define double @gemv(double* %A, %struct.BInfo* %BI) {
entry:
  %bptr = getelementptr inbounds %struct.BInfo, %struct.BInfo* %BI, i32 0, i32 1
  %b = load double*, double** %bptr
  br label %L1

L1:
  %i = phi i32 [ 0, %entry ], [ %i.next, %L2.exit ]
  %sum = phi double [ 0.0, %entry ], [ %sum.next, %L2.exit ]
  %A_row = mul nuw nsw i32 %i, 128
  br label %L2

L2:
  %j = phi i32 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum.L2 = phi double [ %sum, %L1 ], [ %sum.next, %L2 ]
  %A_ind = add nuw nsw i32 %A_row, %j
  %Aijp = getelementptr inbounds double, double* %A, i32 %A_ind
  %Aij = load double, double* %Aijp
  %bjp = getelementptr inbounds double, double* %b, i32 %j
  %bj = load double, double* %bjp
  %Aijbj = fmul double %Aij, %bj
  %sum.next = fadd double %sum.L2, %Aijbj
  %j.next = add nuw nsw i32 %j, 1
  %L2.cond = icmp eq i32 %j.next, 128
  br i1 %L2.cond, label %L2.exit, label %L2

L2.exit:
  %i.next = add nuw nsw i32 %i, 1
  %L1.cond = icmp eq i32 %i.next, 128
  br i1 %L1.cond, label %L1.exit, label %L1

L1.exit:
  ret double %sum.next
}

@GBI = global %struct.BInfo zeroinitializer, align 8

define void @structinit() {
  %b8 = tail call noalias dereferenceable_or_null(1024) i8* @malloc(i64 1024)
	%b = bitcast i8* %b8 to double*
  store double* %b, double** getelementptr inbounds (%struct.BInfo, %struct.BInfo* @GBI, i64 0, i32 1), align 8
  %b2 = getelementptr inbounds double, double* %b, i64 2
  store double -1.0, double* %b2, align 8
  %b3 = getelementptr inbounds double, double* %b, i64 3
  store double 0.0, double* %b3, align 8
  ret void
}

