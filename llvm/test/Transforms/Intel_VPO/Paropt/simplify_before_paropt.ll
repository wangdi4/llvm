; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
;
; Pointer bitcasts were retained when translating this test to opaque pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct._vec = type { float, float, float }

$__clang_call_terminate = comdat any

define dso_local void @_Z14render_offloadv() local_unnamed_addr personality ptr @__gxx_personality_v0 {
entry:
  %isect = alloca %struct._vec, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %y = alloca i32, align 4
  %i = bitcast ptr %isect to ptr
  call void @llvm.lifetime.start.p0(i64 12, ptr %i)
  %i1 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i1)
  %i2 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i2)
  store i32 0, ptr %.omp.lb, align 4
  %i3 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i3)
  store volatile i32 2, ptr %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.1

; Verify that outlining for the OpenMP parallel region occurred
; CHECK: call void {{.*}} @__kmpc_fork_call({{.+}}, {{.+}}, {{.+}} [[PAR_OUTLINED:@.*DIR.OMP.PARALLEL.LOOP.*]], ptr %isect
; CHECK: define internal void [[PAR_OUTLINED]]({{.+}})

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED"(ptr %isect),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i32 0, i32 1) ]

  %i5 = call ptr @llvm.launder.invariant.group.p0(ptr %i)
  %i6 = bitcast ptr %i5 to ptr
  %i7 = load i32, ptr %.omp.lb, align 4
  store volatile i32 %i7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %invoke.cont3, %DIR.OMP.PARALLEL.LOOP.1
  %i8 = load volatile i32, ptr %.omp.iv, align 4
  %i9 = load volatile i32, ptr %.omp.ub, align 4
  %cmp = icmp sgt i32 %i8, %i9
  br i1 %cmp, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i10 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %i10)
  %i11 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %i11, ptr %y, align 4
  invoke void @_Z20ray_sphere_intersectP4_vec(ptr %i6)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %omp.inner.for.body
  invoke void @_Z20ray_sphere_intersectP4_vec(ptr %i6)
          to label %invoke.cont1 unwind label %lpad

invoke.cont1:                                     ; preds = %invoke.cont
  %rs.i.i = alloca %struct._vec, align 4
  %isect.i = alloca %struct._vec, align 4
  %i12 = bitcast ptr %isect.i to ptr
  call void @llvm.lifetime.start.p0(i64 12, ptr %i12)
  %i13 = bitcast ptr %rs.i.i to ptr
  call void @llvm.lifetime.start.p0(i64 12, ptr %i13)
  %x.i.i.i = getelementptr inbounds %struct._vec, ptr %rs.i.i, i64 0, i32 0
  %i14 = load float, ptr %x.i.i.i, align 4
  %x1.i.i.i = getelementptr inbounds %struct._vec, ptr %rs.i.i, i64 0, i32 0
  %i15 = load float, ptr %x1.i.i.i, align 4
  %mul.i.i.i = fmul float %i14, %i15
  %y.i.i.i = getelementptr inbounds %struct._vec, ptr %rs.i.i, i64 0, i32 1
  %i16 = load float, ptr %y.i.i.i, align 4
  %y2.i.i.i = getelementptr inbounds %struct._vec, ptr %rs.i.i, i64 0, i32 1
  %i17 = load float, ptr %y2.i.i.i, align 4
  %mul3.i.i.i = fmul float %i16, %i17
  %add.i.i.i = fadd float %mul.i.i.i, %mul3.i.i.i
  ;
  ; removed lots of computation here not needed for this test and renumbered
  ; %50~55 ==> %18~23
  ;
  %cmp.i.i.i = fcmp ogt float %add.i.i.i, 0x3C670EF540000000
  br i1 %cmp.i.i.i, label %if.then.i.i.i, label %_Z17ambient_occlusionv.exit

if.then.i.i.i:                                    ; preds = %invoke.cont1
  %x.i5.i.i = getelementptr inbounds %struct._vec, ptr %isect.i, i64 0, i32 0
  %i18 = load float, ptr %x.i5.i.i, align 4
  %div.i.i.i = fdiv float %i18, %add.i.i.i
  store float %div.i.i.i, ptr %x.i5.i.i, align 4
  br label %_Z17ambient_occlusionv.exit

_Z17ambient_occlusionv.exit:                      ; preds = %if.then.i.i.i, %invoke.cont1
  call void @llvm.lifetime.end.p0(i64 12, ptr %i13)
  call void @llvm.lifetime.end.p0(i64 12, ptr %i12)
  br label %invoke.cont3

invoke.cont3:                                     ; preds = %_Z17ambient_occlusionv.exit
  call void @llvm.lifetime.end.p0(i64 4, ptr %i10)
  %i19 = load volatile i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %i19, 1
  store volatile i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

lpad:                                             ; preds = %invoke.cont, %omp.inner.for.body
  %i20 = landingpad { ptr, i32 }
          catch ptr null
  %i21 = extractvalue { ptr, i32 } %i20, 0
  br label %ehcleanup

; Verify that the unreachable BB "lpad2" is removed
; CHECK-NOT: lpad2:
lpad2:                                            ; No predecessors!
  %i22 = landingpad { ptr, i32 }
          catch ptr null
  %i23 = extractvalue { ptr, i32 } %i22, 0
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad2, %lpad
  %exn.slot.0 = phi ptr [ %i23, %lpad2 ], [ %i21, %lpad ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %i10)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i3)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i2)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i1)
  call void @__clang_call_terminate(ptr %exn.slot.0)
  unreachable

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.lifetime.end.p0(i64 4, ptr %i3)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i2)
  call void @llvm.lifetime.end.p0(i64 4, ptr %i1)
  call void @llvm.lifetime.end.p0(i64 12, ptr %i)
  ret void
}

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @_Z20ray_sphere_intersectP4_vec(ptr %isect)

declare dso_local i32 @__gxx_personality_v0(...)

declare void @llvm.lifetime.end.p0(i64, ptr nocapture)

define linkonce_odr hidden void @__clang_call_terminate(ptr) local_unnamed_addr comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0)
  call void @_ZSt9terminatev()
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr
declare dso_local void @_ZSt9terminatev() local_unnamed_addr
declare ptr @llvm.launder.invariant.group.p0(ptr)
