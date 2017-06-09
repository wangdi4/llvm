; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -debug-only=VPlanDriver 2>&1 | FileCheck %s
; REQUIRES: asserts

; Verify the HCFG construction of an innermost loop with a single nested region.

; CHECK: Vectorization Plan\nVD: Initial VPlan for VF=
; CHECK: subgraph cluster_region
; CHECK: subgraph cluster_loop
; CHECK: subgraph cluster_region

; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop

;#define N 1600000
;int A[N];
;int B[N];
;int C[N];
;
;void foo() {
;  int i, j;
;
;#pragma omp simd
;  for (j = 0; j < N; j++) {
;    if (A[j] != 0)
;      A[j] = B[j] * C[j];
;    B[j] ++;
;  }
;}


@A = common local_unnamed_addr global [1600000 x i32] zeroinitializer, align 16
@B = common local_unnamed_addr global [1600000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1600000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [1600000 x i32], [1600000 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  %1 = getelementptr inbounds [1600000 x i32], [1600000 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %1, align 4 
  br i1 %cmp1, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [1600000 x i32], [1600000 x i32]* @C, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx5, align 4
  %mul = mul nsw i32 %3, %2
  store i32 %mul, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %for.body, %if.then
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %1, align 4 
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1600000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

