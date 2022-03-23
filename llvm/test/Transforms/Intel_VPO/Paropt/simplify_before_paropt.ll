; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Verify that Paropt can remove the unreachable BB "lpad2", so that
; outlining for the parallel region occurres. If "lpad2" is not removed,
; Code Extractor cannot outline the region (not single-entry), and asserts:
;    bool llvm::vpo::VPOParoptTransform::genMultiThreadedCode(
;                llvm::vpo::WRegionNode*): Assertion `CE.isEligible()' failed
;
; The original test was based on "ao.c" @ AOBench
;  http://lucille.atso-net.jp/aobench/
;
; Compiling the reduced test below with icpx -c -fiopenmp shows the assertion:
;
; typedef struct _vec {
;     float x;
;     float y;
;     float z;
; } vec;
;
; inline float vdot(vec &v0, vec &v1) {
;     return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
; }
;
; inline void vnormalize(vec *c) {
;     float length = vdot((*c), (*c));
;     float dummy  = vdot((*c), (*c));
;     if (length > 1.0e-17f)
;       c->x /= length;
; }
;
; inline void ray_sphere_intersect(vec *isect) {
;     vec rs;
;     // These dummy stmts are needed to reproduce the issue
;     float dummyB = vdot(rs, rs);
;     float dummyC = vdot(rs, rs);
;     float dummyD = vdot(rs, rs);
;     float dummyE = vdot(rs, rs);
;     vnormalize(isect);
; }
;
; inline void ambient_occlusion() {
;     vec isect;
;     ray_sphere_intersect(&isect); // inlined
; }
;
; void render_offload() {
;     vec isect;
; #pragma omp parallel for shared(isect)
;     for (int y = 0; y < 3; y++) {
;        // int dummy1; //doesn't fail if it's here instead
;        ray_sphere_intersect(&isect); // not inlined
;        ray_sphere_intersect(&isect); // not inlined
;        int dummy1; //need this here to fail!
;        ambient_occlusion();          // inlined
;     }
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct._vec = type { float, float, float }

$_Z20ray_sphere_intersectP4_vec = comdat any

$_Z17ambient_occlusionv = comdat any

$__clang_call_terminate = comdat any

$_Z4vdotR4_vecS0_ = comdat any

$_Z10vnormalizeP4_vec = comdat any

; Function Attrs: nounwind uwtable
define dso_local void @_Z14render_offloadv() local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %isect = alloca %struct._vec, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %y = alloca i32, align 4
  %0 = bitcast %struct._vec* %isect to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %0) #2
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store volatile i32 2, i32* %.omp.ub, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.LOOP.1

; Verify that outlining for the OpenMP parallel region occurred
; CHECK: call void {{.*}} @__kmpc_fork_call({{.+}}, {{.+}}, {{.+}} [[PAR_OUTLINED:@.+]] to
; CHECK: define internal void [[PAR_OUTLINED]]({{.+}})

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(%struct._vec* %isect), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %y) ]
  %5 = call i8* @llvm.launder.invariant.group.p0i8(i8* %0)
  %6 = bitcast i8* %5 to %struct._vec*
  %7 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store volatile i32 %7, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %invoke.cont3, %DIR.OMP.PARALLEL.LOOP.1
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %9 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sgt i32 %8, %9
  br i1 %cmp, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  store i32 %11, i32* %y, align 4, !tbaa !2
  invoke void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %6)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %omp.inner.for.body
  invoke void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %6)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %rs.i.i = alloca %struct._vec, align 4
  %isect.i = alloca %struct._vec, align 4
  %12 = bitcast %struct._vec* %isect.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %12) #2
  %13 = bitcast %struct._vec* %rs.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %13) #2
  %x.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 0, !intel-tbaa !6
  %14 = load float, float* %x.i.i.i, align 4, !tbaa !6
  %x1.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 0, !intel-tbaa !6
  %15 = load float, float* %x1.i.i.i, align 4, !tbaa !6
  %mul.i.i.i = fmul float %14, %15
  %y.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 1, !intel-tbaa !9
  %16 = load float, float* %y.i.i.i, align 4, !tbaa !9
  %y2.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 1, !intel-tbaa !9
  %17 = load float, float* %y2.i.i.i, align 4, !tbaa !9
  %mul3.i.i.i = fmul float %16, %17
  %add.i.i.i = fadd float %mul.i.i.i, %mul3.i.i.i
  ;
  ; removed lots of computation here not needed for this test and renumbered
  ; %50~55 ==> %18~23
  ;
  %cmp.i.i.i = fcmp ogt float %add.i.i.i, 0x3C670EF540000000
  br i1 %cmp.i.i.i, label %if.then.i.i.i, label %_Z17ambient_occlusionv.exit

if.then.i.i.i:                                    ; preds = %invoke.cont1
  %x.i5.i.i = getelementptr inbounds %struct._vec, %struct._vec* %isect.i, i64 0, i32 0, !intel-tbaa !6
  %18 = load float, float* %x.i5.i.i, align 4, !tbaa !6
  %div.i.i.i = fdiv float %18, %add.i.i.i
  store float %div.i.i.i, float* %x.i5.i.i, align 4, !tbaa !6
  br label %_Z17ambient_occlusionv.exit

_Z17ambient_occlusionv.exit:                      ; preds = %invoke.cont1, %if.then.i.i.i
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %13) #2
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %12) #2
  br label %invoke.cont3

invoke.cont3:                                     ; preds = %_Z17ambient_occlusionv.exit
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #2
  %19 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add4 = add nsw i32 %19, 1
  store volatile i32 %add4, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

lpad:                                             ; preds = %invoke.cont, %omp.inner.for.body
  %20 = landingpad { i8*, i32 }
          catch i8* null
  %21 = extractvalue { i8*, i32 } %20, 0
  br label %ehcleanup

; Verify that the unreachable BB "lpad2" is removed
; CHECK-NOT: lpad2:
lpad2:                                            ; No predecessors!
  %22 = landingpad { i8*, i32 }
          catch i8* null
  %23 = extractvalue { i8*, i32 } %22, 0
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad2, %lpad
  %exn.slot.0 = phi i8* [ %23, %lpad2 ], [ %21, %lpad ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #2
  call void @__clang_call_terminate(i8* %exn.slot.0) #5
  unreachable

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #2
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: inlinehint uwtable
declare void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %isect)

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) local_unnamed_addr #4 comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #2
  call void @_ZSt9terminatev() #5
  unreachable
}

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

; Function Attrs: noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #5

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #7

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline noreturn nounwind }
attributes #5 = { noreturn nounwind }
attributes #6 = { inlinehint nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"struct@_ZTS4_vec", !8, i64 0, !8, i64 4, !8, i64 8}
!8 = !{!"float", !4, i64 0}
!9 = !{!7, !8, i64 4}
!10 = !{!7, !8, i64 8}
