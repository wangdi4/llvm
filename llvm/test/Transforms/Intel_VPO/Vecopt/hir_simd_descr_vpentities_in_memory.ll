; Test to check correctness of VPlan entities generated for in-memory SIMD
; reduction descriptors when vectorzing with incoming HIR.

; long ub[N];
;
; long foo(long n, long *ub) {
;     long i, ret = 0;
; #pragma omp simd simdlen(4) reduction(+:ret)
;     for (i = 0; i < n; i++) {
;           if (ub[i] > 42) {
;             ret += ub[i];
;             ub[i] += 5;
;           }
;           else
;             ret += ub[i-1];
;     }
;     return ret;
; }

; Input HIR
; <0>     BEGIN REGION { }
; <2>           %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(4),  QUAL.OMP.REDUCTION.ADD(&((%ret)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <37>
; <37>          + DO i1 = 0, %n + -1, 1   <DO_LOOP> <simd>
; <7>           |   %2 = (%ub)[i1];
; <9>           |   if (%2 > 42)
; <9>           |   {
; <21>          |      %3 = (%ret)[0];
; <23>          |      (%ret)[0] = %2 + %3;
; <25>          |      (%ub)[i1] = %2 + 5;
; <9>           |   }
; <9>           |   else
; <9>           |   {
; <15>          |      %4 = (%ub)[i1 + -1];
; <16>          |      %5 = (%ret)[0];
; <18>          |      (%ret)[0] = %4 + %5;
; <9>           |   }
; <37>          + END LOOP
; <37>
; <35>          @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; <0>    END REGION

; For the above HIR, both the update sites (in then and else branches) are
; represented directly as stores. Hence there are no associated StartPHI
; nodes and it is an in-memory reduction.
; The initial value should be saved in private memory and both the store
; operations must save the results in same private memory  which is
; appropriately written to the reduction variable in finalize statements.


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-enable-inmemory-entities -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-enable-inmemory-entities -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check entities dump and VPlan IR
; CHECK: Reduction list
; CHECK: (+) Start: i64* [[V1_START:%.*]]
; CHECK: Memory: i64* [[V1_START]]
; CHECK: i64* [[PRIV:%vp.*]] = allocate-priv i64*
; CHECK: i64 [[START_LD:%vp.*]] = load i64* [[V1_START]]
; CHECK: i64 [[INIT:%vp.*]] = reduction-init i64 0 i64 [[START_LD]]
; CHECK: store i64 [[INIT]] i64* [[PRIV]]
; CHECK: store i64 {{%vp.*}} i64* [[PRIV]]
; CHECK: store i64 {{%vp.*}} i64* [[PRIV]]
; CHECK: i64 [[PRIV_LOAD:%vp.*]] = load i64* [[PRIV]]
; CHECK: i64 [[FINAL:%vp.*]] = reduction-final{u_add} i64 [[PRIV_LOAD]]
; CHECK: store i64 [[FINAL]] i64* [[V1_START]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ub = dso_local local_unnamed_addr global [101 x i64] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i64 @_Z3foolPl(i64 %n, i64* nocapture %ub) local_unnamed_addr {
entry:
  %ret = alloca i64, align 8
  %0 = bitcast i64* %ret to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  store i64 0, i64* %ret, align 8, !tbaa !2
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.123, label %omp.precond.end

DIR.OMP.SIMD.123:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.REDUCTION.ADD:TYPED"(i64* %ret, i64 0, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.123
  %.omp.iv.0 = phi i64 [ %add14, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.123 ]
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %.omp.iv.0
  %2 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %cmp6 = icmp sgt i64 %2, 42
  br i1 %cmp6, label %if.then, label %if.else

if.then:                                          ; preds = %omp.inner.for.body
  %3 = load i64, i64* %ret, align 8, !tbaa !2
  %add8 = add nsw i64 %3, %2
  store i64 %add8, i64* %ret, align 8, !tbaa !2
  %add10 = add nsw i64 %2, 5
  store i64 %add10, i64* %arrayidx, align 8, !tbaa !2
  br label %omp.inner.for.inc

if.else:                                          ; preds = %omp.inner.for.body
  %sub11 = add nsw i64 %.omp.iv.0, -1
  %arrayidx12 = getelementptr inbounds i64, i64* %ub, i64 %sub11
  %4 = load i64, i64* %arrayidx12, align 8, !tbaa !2
  %5 = load i64, i64* %ret, align 8, !tbaa !2
  %add13 = add nsw i64 %5, %4
  store i64 %add13, i64* %ret, align 8, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.else, %if.then
  %add14 = add nuw nsw i64 %.omp.iv.0, 1
  %exitcond = icmp eq i64 %add14, %n
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %.pre = load i64, i64* %ret, align 8, !tbaa !2
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %6 = phi i64 [ %.pre, %omp.loop.exit ], [ 0, %entry ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret i64 %6
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
