;RUN: opt -vplan-vec -mtriple=x86_64-unknown-linux-gnu -S %s | FileCheck %s

; CHECK-LABEL: @reduc_select_icmp_ne
; CHECK: vector.body
; CHECK:  [[ICMP:%.*]] = icmp eq <8 x i32>
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[ICMP]]
; CHECK: VPlannedBB5
; CHECK:  [[ICMP2:%.*]] = icmp ne <8 x i32> [[SEL]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP2]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR]], i32 1, i32 2

; Original source:
;
; int reduc_select_icmp_ne(int *v, long n)
; {
;   int j = 2;
; #pragma omp simd simdlen(8)
;   for (int i = 0; i < n; i++) {
;     if (v[i] != 3)
;       j = 1;
;   }
;   return j;
; }

define noundef i32 @reduc_select_icmp_ne(i32* nocapture noundef readonly %v, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.019 = phi i32 [ 2, %DIR.OMP.SIMD.1 ], [ %spec.select, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %v, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %cmp5.not = icmp eq i32 %1, 3
  %spec.select = select i1 %cmp5.not, i32 %j.019, i32 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %spec.select.lcssa = phi i32 [ %spec.select, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %spec.select.lcssa
}


; CHECK-LABEL: @reduc_select_icmp_eq
; CHECK: vector.body
; CHECK:  [[ICMP:%.*]] = icmp eq <8 x i32>
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[ICMP]]
; CHECK: VPlannedBB5
; CHECK:  [[ICMP2:%.*]] = icmp ne <8 x i32> [[SEL]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP2]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR]], i32 7, i32 3

; Original source:
;
; int reduc_select_icmp_eq(int *v, long n)
; {
;   int j = 3;
; #pragma omp simd simdlen(8)
;   for (int i = 0; i < n; i++) {
;     if (v[i] == 3)
;       j = 7;
;   }
;   return j;
; }

define noundef i32 @reduc_select_icmp_eq(i32* nocapture noundef readonly %v, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.019 = phi i32 [ 3, %DIR.OMP.SIMD.1 ], [ %spec.select, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %v, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %cmp5 = icmp eq i32 %1, 3
  %spec.select = select i1 %cmp5, i32 7, i32 %j.019
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %spec.select.lcssa = phi i32 [ %spec.select, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %spec.select.lcssa
}


; CHECK-LABEL: @reduc_select_icmp_var
; CHECK: vector.body
; CHECK:  [[ICMP:%.*]] = icmp eq <8 x i32>
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[ICMP]]
; CHECK: VPlannedBB7
; CHECK:  [[INSERT:%.*]] = insertelement <8 x i32> poison, i32 %a, i32 0
; CHECK:  [[SPLAT:%.*]] = shufflevector <8 x i32> [[INSERT]], <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK:  [[ICMP2:%.*]] = icmp ne <8 x i32> [[SEL]], [[SPLAT]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP2]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR]], i32 %b, i32 %a

; Original source:
;
; int reduc_select_icmp_var(int *v, int a, int b, long n)
; {
;   int j = a;
; #pragma omp simd simdlen(8)
;   for (int i = 0; i < n; i++) {
;     if (v[i] != 3)
;       j = b;
;   }
;   return j;
; }

define noundef i32 @reduc_select_icmp_var(i32* nocapture noundef readonly %v, i32 noundef %a, i32 noundef %b, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.019 = phi i32 [ %a, %DIR.OMP.SIMD.1 ], [ %spec.select, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %v, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %cmp5.not = icmp eq i32 %1, 3
  %spec.select = select i1 %cmp5.not, i32 %j.019, i32 %b
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %spec.select.lcssa = phi i32 [ %spec.select, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %spec.select.lcssa
}


; CHECK-LABEL: @reduc_select_fcmp_fast
; CHECK: vector.body
; CHECK:  [[FCMP:%.*]] = fcmp fast une <8 x float>
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[FCMP]]
; CHECK: VPlannedBB5
; CHECK:  [[ICMP:%.*]] = icmp ne <8 x i32> [[SEL]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR]], i32 1, i32 2

; Original source (compiled with -ffast-math):
;
; int reduc_select_fcmp_fast(float *v, long n)
; {
;   int j = 2;
; #pragma omp simd simdlen(8)
;   for (int i = 0; i < n; i++) {
;     if (v[i] != 3.0)
;       j = 1;
;   }
;   return j;
; }

define noundef i32 @reduc_select_fcmp_fast(float* nocapture noundef readonly %v, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.020 = phi i32 [ 2, %DIR.OMP.SIMD.1 ], [ %2, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %v, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %cmp6 = fcmp fast une float %1, 3.000000e+00
  %2 = select i1 %cmp6, i32 1, i32 %j.020
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %.lcssa = phi i32 [ %2, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %.lcssa
}


; CHECK-LABEL: @reduc_select_fcmp_nofast
; CHECK: vector.body
; CHECK:  [[FCMP:%.*]] = fcmp une <8 x float>
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[FCMP]]
; CHECK: VPlannedBB5
; CHECK:  [[ICMP:%.*]] = icmp ne <8 x i32> [[SEL]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR]], i32 1, i32 2

; Original source (same as previous, compiled with -fno-fast-math):
;
; int reduc_select_fcmp_nofast(float *v, long n)
; {
;   int j = 2;
; #pragma omp simd simdlen(8)
;   for (int i = 0; i < n; i++) {
;     if (v[i] != 3.0)
;       j = 1;
;   }
;   return j;
; }

define noundef i32 @reduc_select_fcmp_nofast(float* nocapture noundef readonly %v, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.020 = phi i32 [ 2, %DIR.OMP.SIMD.1 ], [ %2, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %v, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %cmp6 = fcmp une float %1, 3.000000e+00
  %2 = select i1 %cmp6, i32 1, i32 %j.020
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %.lcssa = phi i32 [ %2, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %.lcssa
}


; CHECK-LABEL: @reduc_select_icmp_and
; CHECK: vector.body
; CHECK:  [[PHI:%.*]] = phi <8 x i32>
; CHECK:  [[ICMP:%.*]] = icmp sgt <8 x i32>
; CHECK: VPlannedBB5
; CHECK:  [[LOAD:%.*]] = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32
; CHECK:  [[ICMP2:%.*]] = icmp eq <8 x i32> [[LOAD]]
; CHECK:  [[SEL:%.*]] = select <8 x i1> [[ICMP2]]
; CHECK: VPlannedBB7
; CHECK:  [[SEL2:%.*]] = select <8 x i1> [[ICMP]], <8 x i32> [[SEL]], <8 x i32> [[PHI]]
; CHECK: VPlannedBB8
; CHECK:  [[ICMP3:%.*]] = icmp ne <8 x i32> [[SEL2]]
; CHECK:  [[REDOR:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP3]])
; CHECK:  [[SEL3:%.*]] = select i1 [[REDOR]], i32 1, i32 2

; Original source:
;
; int reduc_select_icmp_and(int *src1, int *src2, long n)
; {
;   int r = 2;
; #pragma omp simd simdlen(8)
;   for (long i = 0; i < n; i++)
;     if (src1[i] > 35 && src2[i] == 2)
;       r = 1;
;   return r;
; }

define noundef i32 @reduc_select_icmp_and(i32* nocapture noundef readonly %src1, i32* nocapture noundef readonly %src2, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i64, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i64* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %r.021 = phi i32 [ 2, %DIR.OMP.SIMD.1 ], [ %r.1, %omp.body.continue ]
  %.omp.iv.local.019 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add8, %omp.body.continue ]
  %arrayidx = getelementptr inbounds i32, i32* %src1, i64 %.omp.iv.local.019
  %1 = load i32, i32* %arrayidx, align 4
  %cmp5 = icmp sgt i32 %1, 35
  br i1 %cmp5, label %land.lhs.true, label %omp.body.continue

land.lhs.true:
  %arrayidx6 = getelementptr inbounds i32, i32* %src2, i64 %.omp.iv.local.019
  %2 = load i32, i32* %arrayidx6, align 4
  %cmp7 = icmp eq i32 %2, 2
  %spec.select = select i1 %cmp7, i32 1, i32 %r.021
  br label %omp.body.continue

omp.body.continue:
  %r.1 = phi i32 [ %r.021, %omp.inner.for.body ], [ %spec.select, %land.lhs.true ]
  %add8 = add nuw nsw i64 %.omp.iv.local.019, 1
  %exitcond.not = icmp eq i64 %add8, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %r.1.lcssa = phi i32 [ %r.1, %omp.body.continue ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %r.1.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
