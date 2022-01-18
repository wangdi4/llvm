; Test to check the contents of the temporary descriptors lists populated for HIRLegality for input involving in a max reduction.

; int foo(int* ptr, int step, int n) {
;     int s = 0; char c = step;
; #pragma omp simd linear(ptr) linear(c:1) reduction(max:s)
;     for (int i = 0; i < n; i++) {
;         s = *ptr > s ? *ptr : s;
;         ptr++;
;         c++;
;     }
;     return s;
; }

; Input HIR
; <0>     BEGIN REGION { modified }
; <2>           %3 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%ptr.addr)[0])1),  QUAL.OMP.LINEAR(&((%c)[0])1),  QUAL.OMP.REDUCTION.MAX(&((%s)[0])),  QUAL.OMP.NORMALIZED.IV(&((%.omp.iv)[0])),  QUAL.OMP.NORMALIZED.UB(&((%.omp.ub)[0])) ]
; <3>           (%.omp.iv)[0] = 0;
; <4>           %4 = (%.omp.ub)[0];
; <42>
; <10>            %ptr.addr.promoted = (%ptr.addr)[0];
; <11>            %c.promoted = (%c)[0];
; <12>            %.pre = (%s)[0];
; <13>            %5 = %.pre;
; <42>         + DO i1 = 0, %4, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648> <simd>
; <20>         |   %6 = (%ptr.addr.promoted)[i1];
; <22>         |   %5 = (%6 > %5) ? %6 : %5;
; <23>         |   (%s)[0] = %5;
; <27>         |   (%.omp.iv)[0] = i1 + 1;
; <42>         + END LOOP
; <24>            %incdec.ptr = &((%ptr.addr.promoted)[sext.i32.i64(%4) + 1]);
; <25>            %inc = %c.promoted + trunc.i32.i8(%4)  +  1;
; <36>            (%ptr.addr)[0] = &((%incdec.ptr)[0]);
; <37>            (%c)[0] = %inc;
; <42>
; <40>         @llvm.directive.region.exit(%3); [ DIR.OMP.END.SIMD() ]
; <0>    END REGION


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-vplan-codegen -vplan-print-legality -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -disable-vplan-codegen -vplan-print-legality -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3fooPiii(i32* %ptr, i32 %step, i32 %n) local_unnamed_addr {
; Check contents of the lists from debug log
;
; CHECK-LABEL:  HIRLegality Descriptor Lists
; CHECK:       HIRLegality PrivatesList:
; CHECK:       HIRLegality PrivatesNonPODList:
; CHECK:       HIRLegality LinearList:
; CHECK-NEXT:  Ref: &(([[PTR_ADDR0:%.*]])[0])
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-EMPTY:
; CHECK-NEXT:    AliasRef: [[PTR_ADDR_PROMOTED0:%.*]]
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-NEXT:      InitValue: [[PTR_ADDR_PROMOTED0]]
; CHECK:       Ref: &(([[C0:%.*]])[0])
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-EMPTY:
; CHECK-NEXT:    AliasRef: [[C_PROMOTED0:%.*]]
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-EMPTY:
; CHECK-NEXT:    AliasRef: [[INC0:%.*]]
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK:       HIRLegality ReductionList:
; CHECK-NEXT:  Ref: &(([[S0:%.*]])[0])
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:             ([[S0]])[0] = [[TMP5:%.*]];
; CHECK-EMPTY:
; CHECK-NEXT:    AliasRef: [[DOTPRE0:%.*]]
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:    none
; CHECK-EMPTY:
; CHECK-NEXT:    AliasRef: [[TMP5]]
; CHECK-NEXT:    UpdateInstructions:
; CHECK-NEXT:             [[TMP5]] = ([[TMP6:%.*]] > [[TMP5]]) ? [[TMP6]] : [[TMP5]]
; CHECK-NEXT:      InitValue: [[TMP5]]
; CHECK-EMPTY:
; CHECK-NEXT:    InitValue: [[S0]]
;
entry:
  %ptr.addr = alloca i32*, align 8
  %s = alloca i32, align 4
  %c = alloca i8, align 1
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %ptr, i32** %ptr.addr, align 8, !tbaa !2
  %0 = bitcast i32* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %s, align 4, !tbaa !6
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %c) #2
  %conv = trunc i32 %step to i8
  store i8 %conv, i8* %c, align 1, !tbaa !8
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre18 = bitcast i32* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i32 %n, -1
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  store i32 %sub2, i32* %.omp.ub, align 4, !tbaa !6
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32** %ptr.addr, i32 1), "QUAL.OMP.LINEAR"(i8* %c, i32 1), "QUAL.OMP.REDUCTION.MAX"(i32* %s), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  store i32 0, i32* %.omp.iv, align 4, !tbaa !6
  %4 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp413 = icmp slt i32 %4, 0
  br i1 %cmp413, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %omp.precond.then
  %ptr.addr.promoted = load i32*, i32** %ptr.addr, align 8, !tbaa !2
  %c.promoted = load i8, i8* %c, align 1, !tbaa !8
  %.pre = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %5 = phi i32 [ %.pre, %omp.inner.for.body.lr.ph ], [ %., %omp.inner.for.body ]
  %inc16 = phi i8 [ %c.promoted, %omp.inner.for.body.lr.ph ], [ %inc, %omp.inner.for.body ]
  %incdec.ptr15 = phi i32* [ %ptr.addr.promoted, %omp.inner.for.body.lr.ph ], [ %incdec.ptr, %omp.inner.for.body ]
  %storemerge14 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add7, %omp.inner.for.body ]
  %6 = load i32, i32* %incdec.ptr15, align 4, !tbaa !6
  %cmp6 = icmp sgt i32 %6, %5
  %. = select i1 %cmp6, i32 %6, i32 %5
  store i32 %., i32* %s, align 4, !tbaa !6
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr15, i64 1
  %inc = add i8 %inc16, 1
  %add7 = add nuw nsw i32 %storemerge14, 1
  store i32 %add7, i32* %.omp.iv, align 4, !tbaa !6
  %cmp4 = icmp slt i32 %storemerge14, %4
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %incdec.ptr.lcssa = phi i32* [ %incdec.ptr, %omp.inner.for.body ]
  %inc.lcssa = phi i8 [ %inc, %omp.inner.for.body ]
  store i32* %incdec.ptr.lcssa, i32** %ptr.addr, align 8, !tbaa !2
  store i8 %inc.lcssa, i8* %c, align 1, !tbaa !8
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %omp.precond.then
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  %.pre17 = load i32, i32* %s, align 4, !tbaa !6
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %omp.loop.exit
  %.pre-phi = phi i8* [ %.pre18, %entry.omp.precond.end_crit_edge ], [ %2, %omp.loop.exit ]
  %7 = phi i32 [ 0, %entry.omp.precond.end_crit_edge ], [ %.pre17, %omp.loop.exit ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %c) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 %7
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
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!4, !4, i64 0}

