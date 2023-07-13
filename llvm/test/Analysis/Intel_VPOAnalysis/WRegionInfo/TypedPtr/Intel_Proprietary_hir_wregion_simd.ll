; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
;
; RUN: opt -opaque-pointers=0 -passes="hir-vplan-vec,print<hir-framework>,print<vpo-wrncollection>,require<vpo-wrninfo>" %s 2>&1 | FileCheck %s

; WARNING!!!
; WARNING!!!      ** CONTAINS INTEL IP **
; WARNING!!!      DO NOT SHARE EXTERNALLY
; WARNING!!!

; Test src:
; void foo(int *a, int step, short *p) {
;   int x = 10;
;   int n = 2;
; #pragma omp simd linear(n : step) linear(p : 2) lastprivate(x)
;   for (int i = 0; i < 100; i++)
;     ;
; }

; IR for the test was obtained by getting the IR from FE at O0,
; removing all function attributes, and then running the following on it:
; opt -vpo-cfg-restructuring -vpo-paropt -simplifycfg -instcombine -S

; Check for wregion-collection's prints to ensure clauses from HIR were parsed.
; CHECK:      LASTPRIVATE clause (size=1): (&((LINEAR i32* %x.lpriv)[i64 0])
; CHECK:      LINEAR clause (size=3): (&((LINEAR i32* %n.linear)[i64 0]){{.*}}, LINEAR i32 %step{{[^)]*}})
; CHECK-SAME:                         (&((LINEAR i16** %p.addr.linear)[i64 0]){{.*}}, i32 2{{[^)]*}})
; CHECK-SAME:                         IV(&((LINEAR i32* %i.linear.iv)[i64 0]){{.*}}, i32 1{{[^)]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* noundef %a, i32 noundef %step, i16* noundef %p) {
entry:
  %x.lpriv = alloca i32, align 4
  %n.linear = alloca i32, align 4
  %p.addr.linear = alloca i16*, align 8
  %i.linear.iv = alloca i32, align 4
  store i32 2, i32* %n.linear, align 4
  store i16* %p, i16** %p.addr.linear, align 8
  br i1 true, label %omp.inner.for.body.lr.ph, label %DIR.OMP.END.SIMD.3

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR"(i32* %n.linear, i32 %step),
    "QUAL.OMP.LINEAR"(i16** %p.addr.linear, i32 2),
    "QUAL.OMP.LASTPRIVATE"(i32* %x.lpriv),
    "QUAL.OMP.NORMALIZED.IV"(i8* null),
    "QUAL.OMP.NORMALIZED.UB"(i8* null),
    "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]

  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.lr.ph, %omp.inner.for.body
  %.omp.iv.local.02 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  store i32 %.omp.iv.local.02, i32* %i.linear.iv, align 4, !llvm.access.group !0
  %add1 = add nuw nsw i32 %.omp.iv.local.02, 1
  %cmp = icmp ult i32 %.omp.iv.local.02, 99
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.cond.omp.inner.for.end_crit_edge, !llvm.loop !1

omp.inner.for.cond.omp.inner.for.end_crit_edge:   ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.cond.omp.inner.for.end_crit_edge, %entry
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!0 = distinct !{}
!1 = distinct !{!1, !2, !3}
!2 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!3 = !{!"llvm.loop.parallel_accesses", !0}
; end INTEL_FEATURE_SW_ADVANCED
