; RUN: opt -passes="vplan-vec" -vplan-dump-da < %s 2>&1 | FileCheck %s

; Check whether llvm.stacksave and llvm.stackrestore intrinsic are marked as uniform.
; CHECK: Uniform: [Shape: Uniform] ptr [[TMP_VP1:%.*]] = call ptr @llvm.stacksave
; CHECK: Uniform: [Shape: Uniform] call ptr [[TMP_VP1]] ptr @llvm.stackrestore

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct.ident_t.0.15.22.40.49.55.65.81.110.119.137.161.200.290 = type { i32, i32, i32, i32, ptr }

@.kmpc_loc.0.0.15 = external hidden unnamed_addr global %struct.ident_t.0.15.22.40.49.55.65.81.110.119.137.161.200.290
@.kmpc_loc.0.0.17 = external hidden unnamed_addr global %struct.ident_t.0.15.22.40.49.55.65.81.110.119.137.161.200.290

define hidden void @_ZN8JuliaSet16execute_part_cpuEii.DIR.OMP.PARALLEL.LOOP.2.split95(ptr %tid, ptr %bid, ptr %Q, i64 %k.0.val, i64 %X0.0.val, i64 %Y0.0.val, i64 %Z0.0.val, i64 %W0.0.val, i64 %vz.0.val, i64 %step.0.val, i64 %.capture_expr.7.0.val, ptr %this, i64 %.omp.lb.val.zext, i64 %.omp.ub.0.val) #4 {
DIR.OMP.PARALLEL.LOOP.3:
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.PARALLEL.LOOP.3
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr null, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.1200

DIR.OMP.SIMD.1200:                                ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body34

omp.inner.for.body34:                             ; preds = %for.cond.cleanup, %DIR.OMP.SIMD.1200
  %.omp.iv30.local.081 = phi i32 [ 0, %DIR.OMP.SIMD.1200 ], [ %add88, %for.cond.cleanup ]
  %conv38 = sitofp i32 %.omp.iv30.local.081 to float
  %mul39 = fmul fast float %conv38, poison
  %add40 = fadd fast float %mul39, -1.500000e+00
  br label %for.body

for.cond.cleanup:                                 ; preds = %afterloop.49, %if.end
  %add88 = add nuw nsw i32 %.omp.iv30.local.081, 1
  %exitcond82.not = icmp eq i32 %add88, poison
  br i1 %exitcond82.not, label %DIR.OMP.END.SIMD.288, label %omp.inner.for.body34

for.body:                                         ; preds = %if.end, %omp.inner.for.body34
  %conv69 = fpext float %add40 to double
  %mul70 = fmul fast double %conv69, 0x3FE003C409A835DC
  %add73 = fadd fast double %mul70, poison
  %conv74 = fptrunc double %add73 to float
  %agg.tmp.priv.sroa.0.0.vec.insert = insertelement <2 x float> poison, float %conv74, i64 0
  %agg.tmp.priv.sroa.0.4.vec.insert = insertelement <2 x float> %agg.tmp.priv.sroa.0.0.vec.insert, float poison, i64 1
  %savedstack = call ptr @llvm.stacksave()
  br label %loop.49

_Z6Search6quaterS_.exit.thread:                   ; preds = %loop.49
  call void @llvm.stackrestore(ptr %savedstack)
  br label %if.end

if.end:                                           ; preds = %afterloop.49, %_Z6Search6quaterS_.exit.thread
  %cmp37 = icmp ult i32 poison, 511
  br i1 %cmp37, label %for.body, label %for.cond.cleanup

DIR.OMP.END.SIMD.288:                             ; preds = %for.cond.cleanup
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.288
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  unreachable

loop.49:                                          ; preds = %ifmerge.32, %for.body
  %t4.0 = phi <2 x float> [ %agg.tmp.priv.sroa.0.4.vec.insert, %for.body ], [ poison, %ifmerge.32 ]
  %z.i.sroa.0.0.vec.extract93 = extractelement <2 x float> %t4.0, i64 0
  %1 = fmul fast float %z.i.sroa.0.0.vec.extract93, %z.i.sroa.0.0.vec.extract93
  %2 = fsub fast float %1, poison
  %3 = fadd fast float %2, poison
  br i1 poison, label %_Z6Search6quaterS_.exit.thread, label %ifmerge.80

ifmerge.80:                                       ; preds = %loop.49
  %4 = fmul fast float poison, %3
  %5 = fadd fast float %4, poison
  %6 = fmul fast float %5, %5
  %7 = fadd fast float poison, %6
  %8 = fadd fast float %7, poison
  %9 = call fast float @llvm.sqrt.f32(float %8)
  br label %ifmerge.32

ifmerge.32:                                       ; preds = %ifmerge.80
  br i1 poison, label %afterloop.49, label %loop.49

afterloop.49:                                     ; preds = %ifmerge.32
  %.lcssa = phi float [ %9, %ifmerge.32 ]
  %cmp84 = fcmp fast olt float %.lcssa, 1.000000e+03
  br i1 %cmp84, label %for.cond.cleanup, label %if.end
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.sqrt.f32(float) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #3

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #3

; Function Attrs: nounwind
declare void @__kmpc_for_static_init_4u(ptr, i32, i32, ptr, ptr, ptr, ptr, i32, i32) local_unnamed_addr #1

; Function Attrs: nounwind
declare void @__kmpc_for_static_fini(ptr, i32) local_unnamed_addr #1

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind willreturn }
attributes #4 = { "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
