; RUN: opt < %s -vpo-paropt  -S | FileCheck %s

; This file checks the ssa update can generate correct code for the value 
; which is not live out of the loop.

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "csa"

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: norecurse nounwind uwtable
define hidden fastcc void @__omp_offloading_3b_8c022feb__ZN5Radar24apply_filter_gen_offloadEPfiS0_iS0_i_l221(i64 %image_dim, float* %output, float* %image, float* %filter, i64 %filter_dim) unnamed_addr #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %output.addr = alloca float*, align 8
  %image.addr = alloca float*, align 8
  %filter.addr = alloca float*, align 8
  %filter_dim.addr = alloca i64, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %image_dim.addr.sroa.0.0.extract.trunc = trunc i64 %image_dim to i32
  store float* %output, float** %output.addr, align 8, !tbaa !2
  store float* %image, float** %image.addr, align 8, !tbaa !2
  store float* %filter, float** %filter.addr, align 8, !tbaa !2
  store i64 %filter_dim, i64* %filter_dim.addr, align 8, !tbaa !6
  %conv1 = bitcast i64* %filter_dim.addr to i32*
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  %cmp = icmp sgt i32 %image_dim.addr.sroa.0.0.extract.trunc, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %mul = shl nsw i32 %image_dim.addr.sroa.0.0.extract.trunc, 1
  %add = or i32 %mul, 1
  %div = sdiv i32 %add, 2
  %sub4 = add nsw i32 %div, -1
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !8
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  store i32 %sub4, i32* %.omp.ub, align 4, !tbaa !8
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %omp.precond.then
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(float** %output.addr), "QUAL.OMP.SHARED"(float** %image.addr), "QUAL.OMP.SHARED"(float** %filter.addr), "QUAL.OMP.SHARED"(i32* %conv1) ]
  br label %DIR.OMP.PARALLEL.LOOP.115

DIR.OMP.PARALLEL.LOOP.115:                        ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !8
  store volatile i32 %4, i32* %.omp.iv, align 4, !tbaa !8
  %5 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp59 = icmp sgt i32 %5, %6
  br i1 %cmp59, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %_Z11filter_elemiPfS_S_i.exit, %DIR.OMP.PARALLEL.LOOP.115
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %mul6 = shl nsw i32 %8, 1
  store i32 %mul6, i32* %i, align 4, !tbaa !8
  %9 = load float*, float** %output.addr, align 8, !tbaa !2
  %10 = load float*, float** %image.addr, align 8, !tbaa !2
  %11 = load float*, float** %filter.addr, align 8, !tbaa !2
  %12 = load i32, i32* %conv1, align 8, !tbaa !8
  %mul23.i = shl nsw i32 %12, 1
  %cmp24.i = icmp sgt i32 %12, 0
  br i1 %cmp24.i, label %for.body.i, label %_Z11filter_elemiPfS_S_i.exit

for.body.i:                                       ; preds = %for.body.i, %omp.inner.for.body
  %j.0.i = phi i32 [ 0, %omp.inner.for.body ], [ %add17.i, %for.body.i ]
  %t1.1.i = phi float [ 0.000000e+00, %omp.inner.for.body ], [ %add16.i, %for.body.i ]
  %t0.1.i = phi float [ 0.000000e+00, %omp.inner.for.body ], [ %add12.i, %for.body.i ]
  %add.i = add nsw i32 %j.0.i, %mul6
  %idxprom.i = sext i32 %add.i to i64
  %arrayidx.i = getelementptr inbounds float, float* %10, i64 %idxprom.i
  %13 = load float, float* %arrayidx.i, align 4, !tbaa !10
  %add2.i = add nsw i32 %add.i, 1
  %idxprom3.i = sext i32 %add2.i to i64
  %arrayidx4.i = getelementptr inbounds float, float* %10, i64 %idxprom3.i
  %14 = load float, float* %arrayidx4.i, align 4, !tbaa !10
  %15 = zext i32 %j.0.i to i64
  %arrayidx6.i = getelementptr inbounds float, float* %11, i64 %15
  %16 = load float, float* %arrayidx6.i, align 4, !tbaa !10
  %add7.i = or i32 %j.0.i, 1
  %17 = zext i32 %add7.i to i64
  %arrayidx9.i = getelementptr inbounds float, float* %11, i64 %17
  %18 = load float, float* %arrayidx9.i, align 4, !tbaa !10
  %mul10.i = fmul float %13, %16
  %mul11.i = fmul float %14, %18
  %sub.i = fsub float %mul10.i, %mul11.i
  %add12.i = fadd float %t0.1.i, %sub.i
  %mul13.i = fmul float %13, %18
  %mul14.i = fmul float %14, %16
  %add15.i = fadd float %mul13.i, %mul14.i
  %add16.i = fadd float %t1.1.i, %add15.i
  %add17.i = add nuw nsw i32 %j.0.i, 2
  %cmp.i = icmp slt i32 %add17.i, %mul23.i
  br i1 %cmp.i, label %for.body.i, label %_Z11filter_elemiPfS_S_i.exit

_Z11filter_elemiPfS_S_i.exit:                     ; preds = %for.body.i, %omp.inner.for.body
  %t1.0.i = phi float [ %add16.i, %for.body.i ], [ 0.000000e+00, %omp.inner.for.body ]
  %t0.0.i = phi float [ %add12.i, %for.body.i ], [ 0.000000e+00, %omp.inner.for.body ]
  %idxprom18.i = sext i32 %mul6 to i64
  %arrayidx19.i = getelementptr inbounds float, float* %9, i64 %idxprom18.i
  store float %t0.0.i, float* %arrayidx19.i, align 4, !tbaa !10
  %add20.i = add nsw i32 %mul6, 1
  %idxprom21.i = sext i32 %add20.i to i64
  %arrayidx22.i = getelementptr inbounds float, float* %9, i64 %idxprom21.i
  store float %t1.0.i, float* %arrayidx22.i, align 4, !tbaa !10
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #1
  %19 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %add8 = add nsw i32 %19, 1
  store volatile i32 %add8, i32* %.omp.iv, align 4, !tbaa !8
  %20 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %21 = load i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp5 = icmp sgt i32 %20, %21
  br i1 %cmp5, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %_Z11filter_elemiPfS_S_i.exit, %DIR.OMP.PARALLEL.LOOP.115
  br label %DIR.OMP.END.PARALLEL.LOOP.2

DIR.OMP.END.PARALLEL.LOOP.2:                      ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.2, %entry
  %22 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  %23 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #1
  ret void
}

declare dso_local i32 @__gxx_personality_v0(...)

attributes #0 = { argmemonly nounwind }
attributes #1 = { nounwind }
attributes #2 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 665f615d87fa22e05018a8c403bb5d514f308fc1) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 189e5d52db145255380efeb67e748199fc18637c)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !4, i64 0}

; CHECK-LABEL: define internal void @__omp_offloading_3b_8c022feb__ZN5Radar24apply_filter_gen_offloadEPfiS0_iS0_i_l221_DIR.OMP.PARALLEL.LOOP.1
; CHECK: %t1.0.i = phi float [ 0.000000e+00, %{{.*}} ], [ %{{.*}}, %{{.*}} ]
