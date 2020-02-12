; Test to check correctness of VPlan entities generated for SIMD reduction descriptors reduced using private memory, when vectorzing with incoming HIR for two cases.

; int foo(int* ptr, int step, int n) {
;     int s = 0; char c = step;
; #pragma omp simd linear(ptr) linear(c:1) reduction(+:s)
;     for (int i = 0; i < n; i++) {
;         s += *ptr * c;
;         ptr++;
;         c++;
;     }
;     return s;
; }

; Input HIR for Case 1
; <0>     BEGIN REGION { modified }
; <2>           %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%ptr.addr)[0])1),  QUAL.OMP.LINEAR(&((%c)[0])1),  QUAL.OMP.REDUCTION.ADD(&((%s)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <3>           %ptr.addr.promoted = (%ptr.addr)[0];
; <4>           %c.promoted = (%c)[0];
; <5>           %.pre = (%s)[0];
; <6>           %2 = %.pre;
; <7>           %inc18 = %c.promoted;
; <32>
; <32>          + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <simd>
; <12>          |   %3 = (%ptr.addr.promoted)[i1];
; <15>          |   %2 = (sext.i8.i32(%inc18) * %3)  +  %2;
; <16>          |   (%s)[0] = %2;
; <23>          |   %inc18 = i1 + %c.promoted + 1;
; <32>          + END LOOP
; <17>             %incdec.ptr = &((%ptr.addr.promoted)[sext.i32.i64((-1 + %n)) + 1]);
; <18>             %inc = %c.promoted + (-1 + trunc.i32.i8(%n))  +  1;
; <32>
; <28>          (%ptr.addr)[0] = &((%incdec.ptr)[0]);
; <29>          (%c)[0] = %inc;
; <30>          @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; <0>     END REGION

; For the above HIR, the update site (add) is not live-out as the store to the variable is done immediately within loop. Hence the reduction is done using private memory.
; The initial value should be saved in private memory and the store operation must save the result in same private memory which is appropriately written to the reduction
; variable in finalize statements.

; Input HIR for Case 2
; <0>     BEGIN REGION { modified }
; <2>           %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%ptr.addr)[0])1),  QUAL.OMP.LINEAR(&((%c)[0])1),  QUAL.OMP.REDUCTION.ADD(&((%s)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <3>           %ptr.addr.promoted = (%ptr.addr)[0];
; <4>           %c.promoted = (%c)[0];
; <5>           %.pre = (%s)[0];
; <6>           %2 = %.pre;
; <7>           %inc18 = %c.promoted;
; <33>
; <33>          + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <simd>
; <12>          |   %3 = (%ptr.addr.promoted)[i1];
; <15>          |   %2 = (sext.i8.i32(%inc18) * %3)  +  %2;
; <16>          |   (%s)[0] = %2;
; <23>          |   %inc18 = i1 + %c.promoted + 1;
; <33>          + END LOOP
; <17>             %incdec.ptr = &((%ptr.addr.promoted)[sext.i32.i64((-1 + %n)) + 1]);
; <18>             %inc = %c.promoted + (-1 + trunc.i32.i8(%n))  +  1;
; <33>
; <28>          (%ptr.addr)[0] = &((%incdec.ptr)[0]);
; <29>          (%some_ptr)[0] = %2;
; <30>          (%c)[0] = %inc;
; <31>          @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; <0>     END REGION

; The updating instruction (add) is live-out of the loop used to store into %some_ptr. In this case, we perform a register reduction and the add instruction is directly
; used for last-value computation representation. The stores to private memory inside the loop are dead and expected to be removed by later optimizations.


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -VPlanDriverHIR -disable-vplan-codegen -vplan-entities-dump -vplan-use-entity-instr -vplan-print-after-linearization -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,vplan-driver-hir" -disable-vplan-codegen -vplan-entities-dump -vplan-use-entity-instr -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check entities dump and VPlan IR for case 1
; CHECK: Reduction list
; CHECK: (+) Start: i32* [[V1_START:%.*]]
; CHECK: Memory: i32* [[V1_START]]

; CHECK-LABEL: REGION
; CHECK: i32* [[PRIV:%vp.*]] = allocate-priv i32*
; CHECK: i32 [[INIT:%vp.*]] = reduction-init i32 0 i32* [[V1_START]]
; CHECK: store i32 [[INIT]] i32* [[PRIV]]
; CHECK: store i32 {{%vp.*}} i32* [[PRIV]]
; CHECK: i32 [[PRIV_LOAD:%vp.*]] = load i32* [[PRIV]]
; CHECK: i32 [[FINAL:%vp.*]] = reduction-final{u_add} i32 [[PRIV_LOAD]]
; CHECK: store i32 [[FINAL]] i32* [[V1_START]]
; CHECK: END Region

; Check entities dump and VPlan IR for case 2
; CHECK: Reduction list
; CHECK: (+) Start: i32* [[V1_START:%.*]] Exit: i32 [[ADD_EXIT:%vp.*]]
; CHECK: Memory: i32* [[V1_START]]

; CHECK-LABEL: REGION
; CHECK: i32* [[PRIV:%vp.*]] = allocate-priv i32*
; CHECK: i32 [[INIT:%vp.*]] = reduction-init i32 0 i32* [[V1_START]]
; CHECK: store i32 [[INIT]] i32* [[PRIV]]
; CHECK: i32 [[ADD_EXIT]] = add i32 {{.*}} i32 {{.*}}
; CHECK: store i32 {{%vp.*}} i32* [[PRIV]]
; CHECK: i32 [[FINAL:%vp.*]] = reduction-final{u_add} i32 [[ADD_EXIT]]
; CHECK: store i32 [[FINAL]] i32* [[V1_START]]
; CHECK: END Region


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo1(i32* %ptr, i32 %step, i32 %n) local_unnamed_addr {
entry:
  %ptr.addr = alloca i32*, align 8
  %s = alloca i32, align 4
  %c = alloca i8, align 1
  store i32* %ptr, i32** %ptr.addr, align 8, !tbaa !2
  %0 = bitcast i32* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %s, align 4, !tbaa !6
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %c) #2
  %conv = trunc i32 %step to i8
  store i8 %conv, i8* %c, align 1, !tbaa !8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.114, label %omp.precond.end

DIR.OMP.SIMD.114:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32** %ptr.addr, i32 1), "QUAL.OMP.LINEAR"(i8* %c, i32 1), "QUAL.OMP.REDUCTION.ADD"(i32* %s), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %ptr.addr.promoted = load i32*, i32** %ptr.addr, align 8, !tbaa !2
  %c.promoted = load i8, i8* %c, align 1, !tbaa !8
  %.pre = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.114
  %2 = phi i32 [ %add8, %omp.inner.for.body ], [ %.pre, %DIR.OMP.SIMD.114 ]
  %inc18 = phi i8 [ %inc, %omp.inner.for.body ], [ %c.promoted, %DIR.OMP.SIMD.114 ]
  %incdec.ptr17 = phi i32* [ %incdec.ptr, %omp.inner.for.body ], [ %ptr.addr.promoted, %DIR.OMP.SIMD.114 ]
  %.omp.iv.0 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.114 ]
  %3 = load i32, i32* %incdec.ptr17, align 4, !tbaa !6
  %conv6 = sext i8 %inc18 to i32
  %mul7 = mul nsw i32 %3, %conv6
  %add8 = add nsw i32 %mul7, %2
  store i32 %add8, i32* %s, align 4, !tbaa !6
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr17, i64 1
  %inc = add i8 %inc18, 1
  %add9 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %incdec.ptr.lcssa = phi i32* [ %incdec.ptr, %omp.inner.for.body ]
  %inc.lcssa = phi i8 [ %inc, %omp.inner.for.body ]
  store i32* %incdec.ptr.lcssa, i32** %ptr.addr, align 8, !tbaa !2
  store i8 %inc.lcssa, i8* %c, align 1, !tbaa !8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %.pre19 = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %4 = phi i32 [ %.pre19, %omp.loop.exit ], [ 0, %entry ]
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %c) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 %4
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

define dso_local i32 @foo2(i32* %ptr, i32 %step, i32 %n) local_unnamed_addr {
entry:
  %ptr.addr = alloca i32*, align 8
  %s = alloca i32, align 4
  %c = alloca i8, align 1
  %some_ptr = alloca i32, align 4
  store i32* %ptr, i32** %ptr.addr, align 8, !tbaa !2
  %0 = bitcast i32* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %s, align 4, !tbaa !6
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %c) #2
  %conv = trunc i32 %step to i8
  store i8 %conv, i8* %c, align 1, !tbaa !8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.114, label %omp.precond.end

DIR.OMP.SIMD.114:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32** %ptr.addr, i32 1), "QUAL.OMP.LINEAR"(i8* %c, i32 1), "QUAL.OMP.REDUCTION.ADD"(i32* %s), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %ptr.addr.promoted = load i32*, i32** %ptr.addr, align 8, !tbaa !2
  %c.promoted = load i8, i8* %c, align 1, !tbaa !8
  %.pre = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.114
  %2 = phi i32 [ %add8, %omp.inner.for.body ], [ %.pre, %DIR.OMP.SIMD.114 ]
  %inc18 = phi i8 [ %inc, %omp.inner.for.body ], [ %c.promoted, %DIR.OMP.SIMD.114 ]
  %incdec.ptr17 = phi i32* [ %incdec.ptr, %omp.inner.for.body ], [ %ptr.addr.promoted, %DIR.OMP.SIMD.114 ]
  %.omp.iv.0 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.114 ]
  %3 = load i32, i32* %incdec.ptr17, align 4, !tbaa !6
  %conv6 = sext i8 %inc18 to i32
  %mul7 = mul nsw i32 %3, %conv6
  %add8 = add nsw i32 %mul7, %2
  store i32 %add8, i32* %s, align 4, !tbaa !6
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr17, i64 1
  %inc = add i8 %inc18, 1
  %add9 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %incdec.ptr.lcssa = phi i32* [ %incdec.ptr, %omp.inner.for.body ]
  %inc.lcssa = phi i8 [ %inc, %omp.inner.for.body ]
  store i32* %incdec.ptr.lcssa, i32** %ptr.addr, align 8, !tbaa !2
  store i32 %add8, i32* %some_ptr
  store i8 %inc.lcssa, i8* %c, align 1, !tbaa !8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %.pre19 = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %4 = phi i32 [ %.pre19, %omp.loop.exit ], [ 0, %entry ]
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %c) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 %4
}

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }


!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!4, !4, i64 0}
