; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the condition wasn't hoisted to the region because
; it can't encapsulate the SIMD directives (SIMD instructions won't be inside
; the IF condition). There is an assignment to %N_fetch.29 after the SIMD entry
; intrinsic, therefore, we can't hoist the If condition outside the SIMD
; directives.

; HIR before transformation

; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:TYPED.IV(null, 0, 1, 1),  QUAL.OMP.PRIVATE:F90_DV.TYPED(&((poison)[0]), zeroinitializer, 0),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LIVEIN(&((%M)[0])),  QUAL.OMP.LIVEIN(&((%N)[0])),  QUAL.OMP.LIVEIN:F90_DV(&((%C2)[0])) ]
;       %N_fetch.29 = (%N)[0];
;
;       + DO i1 = 0, 0, 1   <DO_LOOP> <simd>
;       |   + DO i2 = 0, 0, 1   <DO_LOOP>
;       |   |   %storemerge = poison;
;       |   |   if (trunc.i32.i1(%N_fetch.29) != 0)
;       |   |   {
;       |   |      %storemerge = 2 * poison;
;       |   |   }
;       |   |   (poison)[0] = %storemerge;
;       |   + END LOOP
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
;       ret ;
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:TYPED.IV(null, 0, 1, 1),  QUAL.OMP.PRIVATE:F90_DV.TYPED(&((poison)[0]), zeroinitializer, 0),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LIVEIN(&((%M)[0])),  QUAL.OMP.LIVEIN(&((%N)[0])),  QUAL.OMP.LIVEIN:F90_DV(&((%C2)[0])) ]
; CHECK:       %N_fetch.29 = (%N)[0];
; CHECK:       + DO i1 = 0, 0, 1   <DO_LOOP> <simd>
; CHECK:       |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK:       |   |   %storemerge = poison;
; CHECK:       |   |   if (trunc.i32.i1(%N_fetch.29) != 0)
; CHECK:       |   |   {
; CHECK:       |   |      %storemerge = 2 * poison;
; CHECK:       |   |   }
; CHECK:       |   |   (poison)[0] = %storemerge;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK:       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK:       ret ;
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$.2.12" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @foo(ptr %C2, ptr %N, ptr %M) {
DIR.OMP.SIMD.2:
  br label %DIR.OMP.SIMD.172

omp.pdo.body41:                                   ; preds = %DIR.OMP.SIMD.172, %do.body56.preheader
  br label %do.body49

do.body49:                                        ; preds = %bb24_endif, %omp.pdo.body41
  br i1 %rel.7.not, label %bb24_endif, label %bb_new54_then

bb_new54_then:                                    ; preds = %do.body49
  %add.5 = add nsw i32 poison, poison
  br label %bb24_endif

bb24_endif:                                       ; preds = %bb_new54_then, %do.body49
  %storemerge = phi i32 [ %add.5, %bb_new54_then ], [ poison, %do.body49 ]
  store i32 %storemerge, ptr poison, align 1
  %exitcond66.not = icmp eq i64 poison, poison
  br i1 %exitcond66.not, label %do.body56.preheader, label %do.body49

do.body56.preheader:                              ; preds = %bb24_endif
  %exitcond71.not = icmp eq i32 poison, poison
  br i1 %exitcond71.not, label %DIR.OMP.END.SIMD.2, label %omp.pdo.body41

DIR.OMP.SIMD.172:                                 ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED.IV"(ptr null, i32 0, i64 1, i32 1), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr poison, %"QNCA_a0$i32*$rank1$.2.12" zeroinitializer, i32 0), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LIVEIN"(ptr %M), "QUAL.OMP.LIVEIN"(ptr %N), "QUAL.OMP.LIVEIN:F90_DV"(ptr %C2) ]
  %N_fetch.29 = load i32, ptr %N, align 1
  %and.1 = and i32 %N_fetch.29, 1
  %rel.7.not = icmp eq i32 %and.1, 0
  br label %omp.pdo.body41

DIR.OMP.END.SIMD.2:                               ; preds = %do.body56.preheader
  br label %DIR.OMP.END.SIMD.274

DIR.OMP.END.SIMD.274:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)


