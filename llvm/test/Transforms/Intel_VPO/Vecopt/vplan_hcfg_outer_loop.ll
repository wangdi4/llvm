; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -debug-only=VPlanDriver 2>&1 | FileCheck %s 

; Verify the HCFG construction of an outer loop with two nested loops.

; CHECK: Vectorization Plan\nVD: Initial VPlan for VF=
; CHECK: subgraph cluster_region
; CHECK: subgraph cluster_loop
; CHECK-NOT: subgraph cluster_region

; CHECK: subgraph cluster_loop
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}

; CHECK: subgraph cluster_loop
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
; CHECK: }{{[[:space:]]}}

; #define N 1600000
; int A[N][N];
; int B[N][N];
; int C[N][N];
; int D[N];
; 
; void foo() {
;   int i, j;
; 
; #pragma omp simd
;   for (i=0; i<N; i++) {
;     D[i] = A[i][0] + B[i][0];
; 
;     for (j = 0; j < N; j++) {
;       A[i][j] = B[i][j] * C[i][j];
;     }
; 
;     for (j = 0; j < N; j++) {
;       B[i][j] = C[i][j] * A[i][j];
;     }
; 
;   }
; }

@A = common local_unnamed_addr global [1600000 x [1600000 x i32]] zeroinitializer, align 16
@B = common local_unnamed_addr global [1600000 x [1600000 x i32]] zeroinitializer, align 16
@D = common local_unnamed_addr global [1600000 x i32] zeroinitializer, align 16
@C = common local_unnamed_addr global [1600000 x [1600000 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc41, %entry
  %indvars.iv70 = phi i64 [ 0, %entry ], [ %indvars.iv.next71, %for.inc41 ]
  %arrayidx1 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @A, i64 0, i64 %indvars.iv70, i64 0
  %0 = load i32, i32* %arrayidx1, align 16
  %arrayidx4 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @B, i64 0, i64 %indvars.iv70, i64 0
  %1 = load i32, i32* %arrayidx4, align 16
  %add = add nsw i32 %1, %0
  %arrayidx6 = getelementptr inbounds [1600000 x i32], [1600000 x i32]* @D, i64 0, i64 %indvars.iv70
  store i32 %add, i32* %arrayidx6, align 4 
  %arrayidx1773 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @C, i64 0, i64 %indvars.iv70, i64 0
  %2 = load i32, i32* %arrayidx1773, align 4
  %mul74 = mul nsw i32 %2, %1
  %arrayidx2175 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @A, i64 0, i64 %indvars.iv70, i64 0
  store i32 %mul74, i32* %arrayidx2175, align 4
  br label %for.body9.for.body9_crit_edge

for.body24.preheader:                             ; preds = %for.body9.for.body9_crit_edge
  br label %for.body24

for.body9.for.body9_crit_edge:                    ; preds = %for.body, %for.body9.for.body9_crit_edge
  %indvars.iv.next76 = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.body9.for.body9_crit_edge ]
  %arrayidx13.phi.trans.insert = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @B, i64 0, i64 %indvars.iv70, i64 %indvars.iv.next76
  %.pre = load i32, i32* %arrayidx13.phi.trans.insert, align 4
  %arrayidx17 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @C, i64 0, i64 %indvars.iv70, i64 %indvars.iv.next76
  %3 = load i32, i32* %arrayidx17, align 4
  %mul = mul nsw i32 %3, %.pre
  %arrayidx21 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @A, i64 0, i64 %indvars.iv70, i64 %indvars.iv.next76
  store i32 %mul, i32* %arrayidx21, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv.next76, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1600000
  br i1 %exitcond, label %for.body24.preheader, label %for.body9.for.body9_crit_edge

for.body24:                                       ; preds = %for.body24.preheader, %for.body24
  %indvars.iv67 = phi i64 [ %indvars.iv.next68, %for.body24 ], [ 0, %for.body24.preheader ]
  %arrayidx28 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @C, i64 0, i64 %indvars.iv70, i64 %indvars.iv67
  %4 = load i32, i32* %arrayidx28, align 4
  %arrayidx32 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @A, i64 0, i64 %indvars.iv70, i64 %indvars.iv67
  %5 = load i32, i32* %arrayidx32, align 4
  %mul33 = mul nsw i32 %5, %4
  %arrayidx37 = getelementptr inbounds [1600000 x [1600000 x i32]], [1600000 x [1600000 x i32]]* @B, i64 0, i64 %indvars.iv70, i64 %indvars.iv67
  store i32 %mul33, i32* %arrayidx37, align 4
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond69 = icmp eq i64 %indvars.iv.next68, 1600000
  br i1 %exitcond69, label %for.inc41, label %for.body24

for.inc41:                                        ; preds = %for.body24
  %indvars.iv.next71 = add nuw nsw i64 %indvars.iv70, 1
  %exitcond72 = icmp eq i64 %indvars.iv.next71, 1600000
  br i1 %exitcond72, label %for.end43, label %for.body

for.end43:                                        ; preds = %for.inc41
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end43
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

