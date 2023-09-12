; INTEL_FEATURE_CSA
; RUN: opt < %s -bugpoint-enable-legacy-pm -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop-simplify),vpo-paropt' -S | FileCheck %s

; This file checks the ssa update can generate correct code for the value
; which is not live out of the loop.

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "csa"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: norecurse nounwind uwtable
define hidden fastcc void @__omp_offloading_3b_8c022feb__ZN5Radar24apply_filter_gen_offloadEPfiS0_iS0_i_l221(i64 %image_dim, ptr %output, ptr %image, ptr %filter, i64 %filter_dim) unnamed_addr #2 personality ptr @__gxx_personality_v0 {
entry:
  %output.addr = alloca ptr, align 8
  %image.addr = alloca ptr, align 8
  %filter.addr = alloca ptr, align 8
  %filter_dim.addr = alloca i64, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %image_dim.addr.sroa.0.0.extract.trunc = trunc i64 %image_dim to i32
  store ptr %output, ptr %output.addr, align 8, !tbaa !0
  store ptr %image, ptr %image.addr, align 8, !tbaa !0
  store ptr %filter, ptr %filter.addr, align 8, !tbaa !0
  store i64 %filter_dim, ptr %filter_dim.addr, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  %cmp = icmp sgt i32 %image_dim.addr.sroa.0.0.extract.trunc, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %mul = shl nsw i32 %image_dim.addr.sroa.0.0.extract.trunc, 1
  %add = or i32 %mul, 1
  %div = sdiv i32 %add, 2
  %sub4 = add nsw i32 %div, -1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !6
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 %sub4, ptr %.omp.ub, align 4, !tbaa !6
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %omp.precond.then
%0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %output.addr, ptr null, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %image.addr, ptr null, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %filter.addr, ptr null, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr %filter_dim.addr, i64 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.LOOP.115

DIR.OMP.PARALLEL.LOOP.115:                        ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !6
  store volatile i32 %1, ptr %.omp.iv, align 4, !tbaa !6
  %2 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %3 = load i32, ptr %.omp.ub, align 4, !tbaa !6
  %cmp59 = icmp sgt i32 %2, %3
  br i1 %cmp59, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %_Z11filter_elemiPfS_S_i.exit, %DIR.OMP.PARALLEL.LOOP.115
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #1
  %4 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %mul6 = shl nsw i32 %4, 1
  store i32 %mul6, ptr %i, align 4, !tbaa !6
  %5 = load ptr, ptr %output.addr, align 8, !tbaa !0
  %6 = load ptr, ptr %image.addr, align 8, !tbaa !0
  %7 = load ptr, ptr %filter.addr, align 8, !tbaa !0
  %8 = load i32, ptr %filter_dim.addr, align 8, !tbaa !6
  %mul23.i = shl nsw i32 %8, 1
  %cmp24.i = icmp sgt i32 %8, 0
  br i1 %cmp24.i, label %for.body.i, label %_Z11filter_elemiPfS_S_i.exit

for.body.i:                                       ; preds = %for.body.i, %omp.inner.for.body
  %j.0.i = phi i32 [ 0, %omp.inner.for.body ], [ %add17.i, %for.body.i ]
  %t1.1.i = phi float [ 0.000000e+00, %omp.inner.for.body ], [ %add16.i, %for.body.i ]
  %t0.1.i = phi float [ 0.000000e+00, %omp.inner.for.body ], [ %add12.i, %for.body.i ]
  %add.i = add nsw i32 %j.0.i, %mul6
  %idxprom.i = sext i32 %add.i to i64
  %arrayidx.i = getelementptr inbounds float, ptr %6, i64 %idxprom.i
  %9 = load float, ptr %arrayidx.i, align 4, !tbaa !8
  %add2.i = add nsw i32 %add.i, 1
  %idxprom3.i = sext i32 %add2.i to i64
  %arrayidx4.i = getelementptr inbounds float, ptr %6, i64 %idxprom3.i
  %10 = load float, ptr %arrayidx4.i, align 4, !tbaa !8
  %11 = zext i32 %j.0.i to i64
  %arrayidx6.i = getelementptr inbounds float, ptr %7, i64 %11
  %12 = load float, ptr %arrayidx6.i, align 4, !tbaa !8
  %add7.i = or i32 %j.0.i, 1
  %13 = zext i32 %add7.i to i64
  %arrayidx9.i = getelementptr inbounds float, ptr %7, i64 %13
  %14 = load float, ptr %arrayidx9.i, align 4, !tbaa !8
  %mul10.i = fmul float %9, %12
  %mul11.i = fmul float %10, %14
  %sub.i = fsub float %mul10.i, %mul11.i
  %add12.i = fadd float %t0.1.i, %sub.i
  %mul13.i = fmul float %9, %14
  %mul14.i = fmul float %10, %12
  %add15.i = fadd float %mul13.i, %mul14.i
  %add16.i = fadd float %t1.1.i, %add15.i
  %add17.i = add nuw nsw i32 %j.0.i, 2
  %cmp.i = icmp slt i32 %add17.i, %mul23.i
  br i1 %cmp.i, label %for.body.i, label %_Z11filter_elemiPfS_S_i.exit

_Z11filter_elemiPfS_S_i.exit:                     ; preds = %for.body.i, %omp.inner.for.body
  %t1.0.i = phi float [ %add16.i, %for.body.i ], [ 0.000000e+00, %omp.inner.for.body ]
  %t0.0.i = phi float [ %add12.i, %for.body.i ], [ 0.000000e+00, %omp.inner.for.body ]
  %idxprom18.i = sext i32 %mul6 to i64
  %arrayidx19.i = getelementptr inbounds float, ptr %5, i64 %idxprom18.i
  store float %t0.0.i, ptr %arrayidx19.i, align 4, !tbaa !8
  %add20.i = add nsw i32 %mul6, 1
  %idxprom21.i = sext i32 %add20.i to i64
  %arrayidx22.i = getelementptr inbounds float, ptr %5, i64 %idxprom21.i
  store float %t1.0.i, ptr %arrayidx22.i, align 4, !tbaa !8
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #1
  %15 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %add8 = add nsw i32 %15, 1
  store volatile i32 %add8, ptr %.omp.iv, align 4, !tbaa !6
  %16 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %17 = load i32, ptr %.omp.ub, align 4, !tbaa !6
  %cmp5 = icmp sgt i32 %16, %17
  br i1 %cmp5, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %_Z11filter_elemiPfS_S_i.exit, %DIR.OMP.PARALLEL.LOOP.115
  br label %DIR.OMP.END.PARALLEL.LOOP.2

DIR.OMP.END.PARALLEL.LOOP.2:                      ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.2, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { nounwind }
attributes #2 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{!1, !1, i64 0}
!1 = !{!"pointer@_ZTSPf", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"long", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !2, i64 0}

; CHECK-LABEL: define internal void @__omp_offloading_3b_8c022feb__ZN5Radar24apply_filter_gen_offloadEPfiS0_iS0_i_l221.DIR.OMP.PARALLEL.LOOP.1
; CHECK: %t1.0.i = phi float [ 0.000000e+00, %{{.*}} ], [ %{{.*}}, %{{.*}} ]
; end INTEL_FEATURE_CSA
