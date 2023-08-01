; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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

define dso_local void @_Z14render_offloadv() local_unnamed_addr personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %isect = alloca %struct._vec, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %y = alloca i32, align 4
  %0 = bitcast %struct._vec* %isect to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %0)
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2)
  store i32 0, i32* %.omp.lb, align 4
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3)
  store volatile i32 2, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

; Verify that outlining for the OpenMP parallel region occurred
; CHECK: call void {{.*}} @__kmpc_fork_call({{.+}}, {{.+}}, {{.+}} [[PAR_OUTLINED:@.+]] to
; CHECK: define internal void [[PAR_OUTLINED]]({{.+}})

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED"(%struct._vec* %isect),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %y) ]

  %5 = call i8* @llvm.launder.invariant.group.p0i8(i8* %0)
  %6 = bitcast i8* %5 to %struct._vec*
  %7 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %invoke.cont3, %DIR.OMP.PARALLEL.LOOP.1
  %8 = load volatile i32, i32* %.omp.iv, align 4
  %9 = load volatile i32, i32* %.omp.ub, align 4
  %cmp = icmp sgt i32 %8, %9
  br i1 %cmp, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10)
  %11 = load volatile i32, i32* %.omp.iv, align 4
  store i32 %11, i32* %y, align 4
  invoke void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %6)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %omp.inner.for.body
  invoke void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %6)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %rs.i.i = alloca %struct._vec, align 4
  %isect.i = alloca %struct._vec, align 4
  %12 = bitcast %struct._vec* %isect.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %12)
  %13 = bitcast %struct._vec* %rs.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %13)
  %x.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 0
  %14 = load float, float* %x.i.i.i, align 4
  %x1.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 0
  %15 = load float, float* %x1.i.i.i, align 4
  %mul.i.i.i = fmul float %14, %15
  %y.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 1
  %16 = load float, float* %y.i.i.i, align 4
  %y2.i.i.i = getelementptr inbounds %struct._vec, %struct._vec* %rs.i.i, i64 0, i32 1
  %17 = load float, float* %y2.i.i.i, align 4
  %mul3.i.i.i = fmul float %16, %17
  %add.i.i.i = fadd float %mul.i.i.i, %mul3.i.i.i
  ;
  ; removed lots of computation here not needed for this test and renumbered
  ; %50~55 ==> %18~23
  ;
  %cmp.i.i.i = fcmp ogt float %add.i.i.i, 0x3C670EF540000000
  br i1 %cmp.i.i.i, label %if.then.i.i.i, label %_Z17ambient_occlusionv.exit

if.then.i.i.i:                                    ; preds = %invoke.cont1
  %x.i5.i.i = getelementptr inbounds %struct._vec, %struct._vec* %isect.i, i64 0, i32 0
  %18 = load float, float* %x.i5.i.i, align 4
  %div.i.i.i = fdiv float %18, %add.i.i.i
  store float %div.i.i.i, float* %x.i5.i.i, align 4
  br label %_Z17ambient_occlusionv.exit

_Z17ambient_occlusionv.exit:                      ; preds = %invoke.cont1, %if.then.i.i.i
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %13)
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %12)
  br label %invoke.cont3

invoke.cont3:                                     ; preds = %_Z17ambient_occlusionv.exit
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10)
  %19 = load volatile i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %19, 1
  store volatile i32 %add4, i32* %.omp.iv, align 4
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
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1)
  call void @__clang_call_terminate(i8* %exn.slot.0)
  unreachable

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1)
  call void @llvm.lifetime.end.p0i8(i64 12, i8* %0)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_Z20ray_sphere_intersectP4_vec(%struct._vec* %isect)

declare dso_local i32 @__gxx_personality_v0(...)

declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

define linkonce_odr hidden void @__clang_call_terminate(i8*) local_unnamed_addr comdat {
  %2 = call i8* @__cxa_begin_catch(i8* %0)
  call void @_ZSt9terminatev()
  unreachable
}

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr
declare dso_local void @_ZSt9terminatev() local_unnamed_addr
declare i8* @llvm.launder.invariant.group.p0i8(i8*)
