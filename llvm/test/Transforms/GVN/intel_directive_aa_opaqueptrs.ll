; RUN: opt -passes="gvn" -S %s | FileCheck %s

; Test load/store motion around VPO directives.
; In the function "guard", the GUARD directive references x.red, and should
; block motion through the top and bottom of the region.

; The function "simd_private" has a SIMD PRIVATE clause and should also
; block motion in the same way.

; In the function "par_shared", we should not have undeclared live-in live-out
; from the region, such as %f or %tmp.
; The SHARED operands should prevent propagation of values that they alias.

; The function "reduce" has a SIMD loop computing a reduction.
; The stored reduction result can be reused live-out of the loop, instead of
; forcing a reload.

; The function "f90_livein" has a global which is declared "LIVE_IN" to the
; SIMD loop. This global can be partiall registerized in the loop and the last
; value can be reused at the region exit.

; The function "thread_local" tests thread_local and thread_private globals.
; Neither should be moved across the region.

; CHECK-LABEL: guard
; CHECK-LABEL: DIR.VPO.GUARD.MEM.MOTION.3:
; CHECK: load float, ptr %x.red
; CHECK-LABEL: DIR.OMP.END.SIMD.6:
; CHECK: load float, ptr %x.red

; CHECK-LABEL: simd_private
; CHECK-LABEL: DIR.OMP.END.SIMD.6:
; CHECK: load float, ptr %x.red

; CHECK-LABEL: par_shared
; CHECK-LABEL: DIR.OMP.PAR.1:
; CHECK: load float, ptr %x
; CHECK-LABEL: DIR.OMP.END.PAR.6:
; CHECK: load float, ptr %x
; CHECK: load float, ptr %y

; CHECK-LABEL: reduce
; CHECK-LABEL: DIR.OMP.END.SIMD.2:
; CHECK-NOT: load
; CHECK: ret float %conv4

; CHECK-LABEL: f90_livein
; CHECK-LABEL: DIR.OMP.END.SIMD.46:
; CHECK-NEXT: [[RET:%.*]] = phi float
; CHECK-NEXT: ret float [[RET]]

; CHECK-LABEL: thread_local
; CHECK-LABEL: DIR.OMP.END.PARALLEL:
; CHECK: load float, ptr @tp
; CHECK: load float, ptr @tl

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local float @guard() local_unnamed_addr #0 {
DIR.OMP.SIMD.131:
  %x.red = alloca float, align 4
  store float 6.0, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %DIR.VPO.GUARD.MEM.MOTION.3

DIR.VPO.GUARD.MEM.MOTION.3:                       ; preds = %DIR.OMP.SIMD.1
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
    "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  %load1 = load float, ptr %x.red, align 4
  br label %DIR.OMP.SCAN.5

DIR.OMP.SCAN.5:                                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.3
  br label %DIR.OMP.END.SCAN.7

DIR.OMP.END.SCAN.7:                               ; preds = %DIR.OMP.SCAN.5
  store float 5.0, ptr %x.red, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.9

DIR.VPO.END.GUARD.MEM.MOTION.9:                   ; preds = %DIR.OMP.END.SCAN.7
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.9
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  %load2 = load float, ptr %x.red, align 4
  %rez = fadd float %load1, %load2
  ret float %rez
}

define dso_local float @simd_private() local_unnamed_addr #0 {
DIR.OMP.SIMD.131:
  %x.red = alloca float, align 4
  store float 6.0, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x.red, i32 0, i32 1)]
  store float 5.0, ptr %x.red, align 4
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  %load1 = load float, ptr %x.red, align 4
  ret float %load1
}

define dso_local float @par_shared(float %f) local_unnamed_addr #0 {
DIR.OMP.PAR.131:
  %x = alloca float, align 4
  %y = alloca float, align 4
  store float %f, ptr %x, align 4
  br label %DIR.OMP.PAR.1

DIR.OMP.PAR.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y, i32 0, i32 1)]
  %tmp = load float, ptr %x, align 4
  store float %tmp, ptr %y, align 4
  br label %DIR.OMP.END.PAR.6

DIR.OMP.END.PAR.6:
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.PARALLEL"() ]
  %load1 = load float, ptr %x, align 4
  %load2 = load float, ptr %y, align 4
  %rez = fadd float %load1, %load2
  ret float %rez
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef float @reduce(ptr nocapture noundef readonly %a) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %total.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 0.000000e+00, ptr %total.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %total.red, float 0.000000e+00, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]

  br label %DIR.OMP.SIMD.118

DIR.OMP.SIMD.118:                                 ; preds = %DIR.OMP.SIMD.1
  %total.red.promoted = load float, ptr %total.red, align 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.118, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.118 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = phi float [ %total.red.promoted, %DIR.OMP.SIMD.118 ], [ %add1, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv) #2
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %3 = load float, ptr %arrayidx, align 4
  %add1 = fadd fast float %1, %3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 100
  br i1 %exitcond, label %omp.inner.for.body, label %DIR.OMP.END.SIMD.410

DIR.OMP.END.SIMD.410:                             ; preds = %omp.inner.for.body
  %add1.lcssa = phi float [ %add1, %omp.inner.for.body ]
  store float %add1.lcssa, ptr %total.red, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.410
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %4 = load float, ptr %total.red, align 4
  %conv4 = fadd fast float %4, 1.050000e+01
  ret float %conv4
}

@test_mp_sum_ = common global float 0.000000e+00, align 8

; Function Attrs: nounwind uwtable
define float @f90_livein(ptr noalias nocapture dereferenceable(4) %A, ptr noalias nocapture dereferenceable(4) %N) local_unnamed_addr #0 {
DIR.OMP.SIMD.1:
  %"test_mp_reduce_$I.linear.iv" = alloca i32, align 4
  %N_fetch.1 = load i32, ptr %N, align 1
  %rel.1.not7 = icmp slt i32 %N_fetch.1, 1
  br i1 %rel.1.not7, label %DIR.OMP.END.SIMD.46, label %omp.pdo.body5.lr.ph

omp.pdo.body5:                                    ; preds = %omp.pdo.body5.lr.ph, %omp.pdo.body5
  %indvars.iv = phi i64 [ 0, %omp.pdo.body5.lr.ph ], [ %indvars.iv.next, %omp.pdo.body5 ]
  %test_mp_sum__fetch.710 = phi float [ %test_mp_sum_.promoted, %omp.pdo.body5.lr.ph ], [ %add.3, %omp.pdo.body5 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %A, i64 %indvars.iv.next)
  %"A[]_fetch.9" = load float, ptr %"A[]", align 1
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %"A[]_fetch.9", %test_mp_sum__fetch.710
  %exitcond = icmp ne i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.pdo.body5, label %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge

omp.pdo.body5.lr.ph:                              ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %"test_mp_reduce_$I.linear.iv", i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
    "QUAL.OMP.LIVEIN:TYPED"(ptr %A, i32 0),
    "QUAL.OMP.LIVEIN:TYPED"(ptr @test_mp_sum_, i32 0),
    "QUAL.OMP.LIVEIN:TYPED"(ptr %N, i32 0) ]

  %test_mp_sum_.promoted = load float, ptr @test_mp_sum_, align 8
  %1 = add i32 %N_fetch.1, -1
  %wide.trip.count = sext i32 %N_fetch.1 to i64
  br label %omp.pdo.body5

omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.pdo.body5
  %2 = zext i32 %1 to i64
  %add.4.le = add nuw nsw i32 %1, 2
  store i32 %add.4.le, ptr %"test_mp_reduce_$I.linear.iv", align 4
  store float %add.3, ptr @test_mp_sum_, align 8
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.pdo.cond4.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  br label %DIR.OMP.END.SIMD.46

DIR.OMP.END.SIMD.46:                              ; preds = %DIR.OMP.END.SIMD.2, %DIR.OMP.SIMD.1
  %test_mp_sum__fetch.12 = load float, ptr @test_mp_sum_, align 8
  ret float %test_mp_sum__fetch.12
}

@tp = thread_private global float 0.0, align 4
@tl = thread_local global float 0.0, align 4

define dso_local float @thread_local() local_unnamed_addr #0 {
entry:
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  store float 5.0, ptr @tp, align 4
  store float 5.0, ptr @tl, align 4
  br label %DIR.OMP.END.PARALLEL

DIR.OMP.END.PARALLEL:
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.PARALLEL"() ]
  %load1 = load float, ptr @tp, align 4
  %load2 = load float, ptr @tl, align 4
  %sum = fadd float %load1, %load2
  ret float %sum
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
