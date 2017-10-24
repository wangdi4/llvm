;RUN: opt -VPODriver -disable-vplan-subregions -disable-vplan-predicator -disable-vplan-codegen -S %s | FileCheck %s
;RUN: opt -VPlanDriver -disable-vplan-subregions -disable-vplan-predicator -S %s | FileCheck %s


;#define N 1024
;#define SIZE 1024*10
;unsigned long A[SIZE];
;unsigned long B[SIZE];
;
;void load_consecutive() {
;  for (unsigned int i=0; i<N; i++) {
;    B[i] = A[i] + 5;
;  }
;}
; This test checks consecutive load A[i]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@A = global [10240 x i64] zeroinitializer, align 16
@B = global [10240 x i64] zeroinitializer, align 16

; CHECK-LABEL: load_consecutive
; CHECK: vector.body
; CHECK: load <4 x i64>, <4 x i64>*
; CHECK: br {{.*}} label %for.end

define void @load_consecutive() {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10240 x i64], [10240 x i64]* @A, i64 0, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8
  %add = add i64 %0, 5
  %arrayidx2 = getelementptr inbounds [10240 x i64], [10240 x i64]* @B, i64 0, i64 %indvars.iv
  store i64 %add, i64* %arrayidx2, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  ret void
}

declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

;#define N 1024
;#define SIZE 1024*10
;unsigned long A[SIZE];
;unsigned long B[SIZE];
;
;void load_invariant(unsigned long *C) {
;  for (unsigned int i=0; i<N; i++) {
;    B[i] = A[i] + *C;
;  }
;}
; This test checks loop invariant load from C

; CHECK-LABEL: load_invariant
; CHECK: vector.body
; CHECK:  %[[WIDE_LOAD:.*]] = load <4 x i64>, <4 x i64>*
; CHECK:  %[[TMP0:.*]] = load i64, i64* %C, align 8
; CHECK:  %[[TMP1:.*]] = insertelement <4 x i64> undef, i64 %[[TMP0]], i32 0
; CHECK:  %[[SPLAT:.*]] = shufflevector <4 x i64> %[[TMP1]], <4 x i64> undef, <4 x i32> zeroinitializer
; CHECK:  add <4 x i64> %[[WIDE_LOAD]], %[[SPLAT]]
; CHECK: br {{.*}} label %for.end

define void @load_invariant(i64* nocapture readonly %C) {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10240 x i64], [10240 x i64]* @A, i64 0, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8
  %1 = load i64, i64* %C, align 8
  %add = add i64 %0, %1
  %arrayidx2 = getelementptr inbounds [10240 x i64], [10240 x i64]* @B, i64 0, i64 %indvars.iv
  store i64 %add, i64* %arrayidx2, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

