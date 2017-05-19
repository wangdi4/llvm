; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -debug-only=VPlanDriver 2>&1 | FileCheck %s

; Verify the HCFG construction of an outer loop with an if statement
; enclosing an inner loop.

; CHECK: Vectorization Plan\nVD: Initial VPlan for VF=
; CHECK: subgraph cluster_region
; CHECK: subgraph cluster_loop
; CHECK: subgraph cluster_region
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


; #define SIZE 1024
; int A[SIZE][SIZE], B[SIZE][SIZE];

; int foo () {
; #pragma omp simd
;     for (int j = 0; j < SIZE; j+=1) {
;         if (B[j] > 0) {
;             for (int i = 0; i < SIZE; i+=1) {
;                 A[i][j] = B[i][j];
;             }
;         }
;     }
;     return 0;
; }


; ModuleID = 'inner.c'
source_filename = "inner.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1024 x [1024 x i32]] zeroinitializer, align 16
@A = common local_unnamed_addr global [1024 x [1024 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  %i = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* nonnull %i)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  %0 = bitcast i32* %i to i8*
  br label %for.body

for.body:                                         ; preds = %for.inc14, %DIR.QUAL.LIST.END.2
  %indvars.iv24 = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next25, %for.inc14 ]
  %arraydecay = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @B, i64 0, i64 %indvars.iv24, i64 0
  %cmp1 = icmp eq i32* %arraydecay, null
  br i1 %cmp1, label %for.inc14, label %if.then

if.then:                                          ; preds = %for.body
  call void @llvm.lifetime.start(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %i, align 4, !tbaa !1
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  call void @llvm.lifetime.end(i64 4, i8* nonnull %0) #2
  br label %for.inc14

for.body5:                                        ; preds = %for.body5, %if.then
  %indvars.iv = phi i64 [ 0, %if.then ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv24
  %1 = load i32, i32* %arrayidx9, align 4, !tbaa !5
  %arrayidx13 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @A, i64 0, i64 %indvars.iv, i64 %indvars.iv24
  store i32 %1, i32* %arrayidx13, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5, !llvm.loop !8

for.inc14:                                        ; preds = %for.body, %for.cond.cleanup4
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 1024
  br i1 %exitcond26, label %for.end16, label %for.body

for.end16:                                        ; preds = %for.inc14
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %for.end16
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21001)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA1024_A1024_i", !7, i64 0}
!7 = !{!"array@_ZTSA1024_i", !2, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"vplan.vect.candidate"}


