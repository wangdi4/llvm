; RUN: opt -passes="jump-threading" -S | FileCheck %s
; CHECK-NOT: call{{.*}}directive

; The loop at bb6_endif does not execute. Jump threading merges all the blocks
; in the control-flow sequence to a single block. The OpenMP directive must
; be deleted when the blocks are merged.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
DIR.OMP.SIMD.2:
  %"simdtest_$I.linear.iv" = alloca i32, align 8
  %"simdtest_$ZDMIN" = alloca i32, align 8
  %"simdtest_$DZ" = alloca float, align 8
  %"simdtest_$RSZ" = alloca i32, align 8
  %"simdtest_$MM" = alloca i32, align 8
  %"simdtest_$BB" = alloca i32, align 8
  %"simdtest_$AA" = alloca i32, align 8
  %"simdtest_$NR1" = alloca i32, align 8
  %func_result = call i32 @for_set_fpe_(ptr nonnull @0) #2
  %func_result2 = call i32 @for_set_reentrancy(ptr nonnull @1) #2
  store i32 2, ptr %"simdtest_$RSZ", align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.219
  br i1 false, label %DIR.OMP.END.SIMD.4.loopexit, label %omp.pdo.body9.lr.ph

omp.pdo.body9.lr.ph:                              ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV"(ptr %"simdtest_$I.linear.iv", i32 1),
    "QUAL.OMP.NORMALIZED.IV"(ptr null),
    "QUAL.OMP.NORMALIZED.UB"(ptr null),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$ZDMIN"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$NR1"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$DZ"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$MM"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$BB"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$RSZ"),
    "QUAL.OMP.LIVEIN"(ptr %"simdtest_$AA") ]
  br label %omp.pdo.body9

omp.pdo.body9:                                    ; preds = %omp.pdo.body9.lr.ph, %bb6_endif
  %omp.pdo.norm.iv.local.022 = phi i32 [ 0, %omp.pdo.body9.lr.ph ], [ %add.1, %bb6_endif ]
  %add.1 = add nsw i32 %omp.pdo.norm.iv.local.022, 1
  store i32 %add.1, ptr %"simdtest_$I.linear.iv", align 8
  store i32 0, ptr %"simdtest_$AA", align 8
  %"simdtest_$RSZ_fetch.5" = load i32, ptr %"simdtest_$RSZ", align 8
  %add.2 = add nsw i32 %"simdtest_$RSZ_fetch.5", 1
  store i32 %add.2, ptr %"simdtest_$BB", align 8
  br label %bb3

bb3:                                              ; preds = %bb4_endif, %omp.pdo.body9
  %"simdtest_$AA_fetch.7" = phi i32 [ 0, %omp.pdo.body9 ]
  %"simdtest_$BB_fetch.6" = load i32, ptr %"simdtest_$BB", align 8
  %sub.3 = sub nsw i32 %"simdtest_$BB_fetch.6", %"simdtest_$AA_fetch.7"
  %rel.2 = icmp sgt i32 %sub.3, 1
  br label %bb_new21_then

bb_new21_then:                                    ; preds = %bb2
  store i32 1, ptr %"simdtest_$NR1", align 8
  store i32 -1, ptr %"simdtest_$ZDMIN", align 8
  br label %bb6_endif

bb6_endif:                                        ; preds = %bb_new21_then, %bb2
  %add.4 = add nsw i32 %add.1, 1
  store i32 %add.4, ptr %"simdtest_$I.linear.iv", align 8
  br i1 false, label %omp.pdo.body9, label %omp.pdo.cond8.DIR.OMP.END.SIMD.4.loopexit_crit_edge


omp.pdo.cond8.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %bb6_endif
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  br label %DIR.OMP.END.SIMD.4.loopexit

DIR.OMP.END.SIMD.4.loopexit:                      ; preds = %omp.pdo.cond8.DIR.OMP.END.SIMD.4.loopexit_crit_edge, %DIR.OMP.SIMD.1
  br label %DIR.OMP.END.SIMD.420

DIR.OMP.END.SIMD.420:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

declare i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" "pre_loopopt" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind }



