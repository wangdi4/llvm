; Test to check that DA marks array type accesses with appropriate shape especially
; when the GEP is split.
; Test generated from the following C Source code.
; void foo(double (*arr)[100][100],
;          double (*arr2)[100][100],
;          double (*arr3)[100][100])
; {
;   int i, j, k;
;   double d;
;
;   for (i = 0; i < 10; ++i)
;     for (j = 0; j < 20; ++j) {
;       d = i + j;
; #pragma omp simd
;       for (k = 0; k < 30; ++k) {
;       arr[i][k][j] = d;
;       arr2[i][k][j] = d;
;         arr3[i][j][k] = d;
;       }
;     }
; }
;
; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -debug-only=vplan-divergence-analysis -vplan-force-vf=4 -S 2>&1 | FileCheck %s
; Check DA results
; CHECK: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[IV:%vp.*]] = phi  [ i64 [[IV_ADD:%vp.*]], {{.*}} ],  [ i64 0, {{.*}} ]
; The first GEP from the GEP split should be unit strided and the next one should be strided.
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 800] [100 x double]* [[BASE:%vp.*]] = getelementptr inbounds [100 x [100 x double]]* %arr i64 [[IINDEX:%.*]] i64 [[IV]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 800] double* [[ELEMENT:%vp.*]] = getelementptr inbounds [100 x double]* [[BASE]] i64 0 i64 [[JINDEX:%.*]]
; CHECK: Divergent: [Shape: Random] double* [[ELEMENT2:%vp.*]] = getelementptr inbounds [100 x [100 x double]]* %arr2 i64 [[IINDEX]] i64 [[IV]] i64 [[JINDEX]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] double* {{%vp.*}} = getelementptr inbounds [100 x [100 x double]]* %arr3 i64 [[IINDEX]] i64 [[JINDEX]] i64 [[IV]]
; Check that we generate two scatters followed by a wide store.
; CHECK-LABEL: vector.body:
; CHECK: call void @llvm.masked.scatter{{.*}}<4 x double> [[STOREVAL:%.*]], <4 x double*>
; CHECK: call void @llvm.masked.scatter{{.*}}<4 x double> [[STOREVAL]], <4 x double*>
; CHECK: store <4 x double> [[STOREVAL]], <4 x double>*

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo([100 x [100 x double]]* nocapture %arr, [100 x [100 x double]]* nocapture %arr2, [100 x [100 x double]]* nocapture %arr3) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc24, %entry
  %indvars.iv55 = phi i64 [ 0, %entry ], [ %indvars.iv.next56, %for.inc24 ]
  br label %simdloop.for.body.lr.ph

simdloop.for.body.lr.ph:                         ; preds = %DIR.OMP.END.SIMD.3, %for.cond1.preheader
  %indvars.iv51 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next52, %DIR.OMP.END.SIMD.3 ]
  %0 = add nuw nsw i64 %indvars.iv51, %indvars.iv55
  %1 = trunc i64 %0 to i32
  %conv = sitofp i32 %1 to double
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %simdloop.for.body.lr.ph
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %simdloop.for.body

simdloop.for.body:                               ; preds = %simdloop.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %simdloop.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  ; Check DA results by splitting the GEP
  %arrayidx10.p = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* %arr, i64 %indvars.iv55, i64 %indvars.iv
  %arrayidx10 = getelementptr inbounds [100 x double], [100 x double]* %arrayidx10.p, i64 0, i64 %indvars.iv51
  store double %conv, double* %arrayidx10, align 8
  %arrayidx16 = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* %arr2, i64 %indvars.iv55, i64 %indvars.iv, i64 %indvars.iv51
  store double %conv, double* %arrayidx16, align 8
  %arrayidx22 = getelementptr inbounds [100 x [100 x double]], [100 x [100 x double]]* %arr3, i64 %indvars.iv55, i64 %indvars.iv51, i64 %indvars.iv
  store double %conv, double* %arrayidx22, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 30
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %simdloop.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %simdloop.for.body
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond54 = icmp eq i64 %indvars.iv.next52, 20
  br i1 %exitcond54, label %for.inc24, label %simdloop.for.body.lr.ph

for.inc24:                                        ; preds = %DIR.OMP.END.SIMD.3
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57 = icmp eq i64 %indvars.iv.next56, 10
  br i1 %exitcond57, label %for.end26, label %for.cond1.preheader

for.end26:                                        ; preds = %for.inc24
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

