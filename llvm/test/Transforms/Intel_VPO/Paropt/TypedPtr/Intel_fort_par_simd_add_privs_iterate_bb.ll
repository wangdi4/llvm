; INTEL_CUSTOMIZATION
; Verify that addPrivateToEnclosingRegion utility is able to handle cases where the
; enclosing SIMD directive instruction is not the first instruction in a BB.

; Input Fortran -
; PROGRAM parallel__do__simd
;   implicit none
;   INTEGER :: N1 = 64
;   INTEGER :: i1
;   INTEGER :: N2 = 64
;   INTEGER :: i2
;   LOGICAL :: almost_equal
;   REAL :: counter_N0
;   INTEGER :: expected_value
;   expected_value = N1*N2
;   counter_N0 = 0
;   !$OMP parallel
;   !$OMP do
;   DO i1 = 1, N1
;     !$OMP simd
;     DO i2 = 1, N2
;       !$OMP atomic update
;       counter_N0 = counter_N0 + 1.
;     END DO
;   END DO
;   !$OMP END parallel
; END PROGRAM parallel__do__simd

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -vpo-paropt-keep-blocks-order=false -S --vpo-utils-add-typed-privates=false %s | FileCheck %s --check-prefixes=CHECK,UNTYPED
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefixes=CHECK,TYPED
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -vpo-paropt-keep-blocks-order=false -S --vpo-utils-add-typed-privates=false %s | FileCheck %s --check-prefixes=CHECK,UNTYPED
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefixes=CHECK,TYPED

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"parallel__do__simd_$N2" = internal global i32 64, align 8
@"parallel__do__simd_$N1" = internal global i32 64, align 8
@0 = internal unnamed_addr constant i32 2

define void @MAIN__() local_unnamed_addr {
; CHECK:  define internal void @MAIN__.DIR.OMP.PARALLEL.2({{.*}})
; CHECK-NEXT:  newFuncRoot:
; CHECK:         [[TEMP90:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[TEMP0:%.*]] = alloca float, align 4
; CHECK-NEXT:    br label {{%.*}}

; CHECK:       bb6:
; CHECK-NEXT:    [[TEMP_FETCH_250:%.*]] = load i32, i32* [[TEMP100:%.*]], align 4
; CHECK-NEXT:    [[TEMP_FETCH_260:%.*]] = load i32, i32* [[TEMP120:%.*]], align 4
; CHECK-NEXT:    [[MUL_40:%.*]] = mul nsw i32 0, [[TEMP_FETCH_260]]
; CHECK-NEXT:    [[ADD_30:%.*]] = add nsw i32 [[MUL_40]], [[TEMP_FETCH_250]]
; CHECK-NEXT:    store i32 [[ADD_30]], i32* %"parallel__do__simd_$I2.linear.iv", align 1
; TYPED-NEXT:    [[TMP2:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"parallel__do__simd_$I2.linear.iv", i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE:TYPED"(float* [[TEMP90]], float 0.000000e+00, i32 1), "QUAL.OMP.PRIVATE:TYPED"(float* [[TEMP0]], float 0.000000e+00, i32 1) ]
; UNTYPED-NEXT:    [[TMP2:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"parallel__do__simd_$I2.linear.iv", i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(float* [[TEMP90]]), "QUAL.OMP.PRIVATE"(float* [[TEMP0]]) ]
; CHECK-NEXT:    br label {{%.*}}

; CHECK:       bb11:
; CHECK-NEXT:    %"(i8*)parallel__do__simd_$COUNTER_N0$" = bitcast float* %"parallel__do__simd_$COUNTER_N0" to i8*
; CHECK-NEXT:    %"(i8*)temp$" = bitcast float* [[TEMP90]] to i8*
; CHECK-NEXT:    call void @__atomic_load(i64 4, i8* %"(i8*)parallel__do__simd_$COUNTER_N0$", i8* nonnull %"(i8*)temp$", i32 0)
; CHECK-NEXT:    [[TEMP_FETCH_310:%.*]] = load float, float* [[TEMP90]], align 4
; CHECK-NEXT:    [[ADD_50:%.*]] = fadd reassoc ninf nsz arcp contract afn float [[TEMP_FETCH_310]], 1.000000e+00
; CHECK-NEXT:    store float [[ADD_50]], float* [[TEMP0]], align 4
; CHECK-NEXT:    %"(i8*)temp$6" = bitcast float* [[TEMP0]] to i8*
; CHECK-NEXT:    [[FUNC_RESULT80:%.*]] = call i1 @__atomic_compare_exchange(i64 4, i8* %"(i8*)parallel__do__simd_$COUNTER_N0$", i8* nonnull %"(i8*)temp$", i8* nonnull %"(i8*)temp$6", i32 0, i32 0)

DIR.OMP.PARALLEL.3:
  %"parallel__do__simd_$COUNTER_N0" = alloca float, align 8
  %"parallel__do__simd_$I2" = alloca i32, align 8
  %"parallel__do__simd_$I1" = alloca i32, align 8
  %func_result = call i32 @for_set_reentrancy(i32* nonnull @0)
  store float 0.000000e+00, float* %"parallel__do__simd_$COUNTER_N0", align 8
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %DIR.OMP.PARALLEL.3
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(float* %"parallel__do__simd_$COUNTER_N0"),
    "QUAL.OMP.SHARED"(i32* @"parallel__do__simd_$N2"),
    "QUAL.OMP.SHARED"(i32* @"parallel__do__simd_$N1"),
    "QUAL.OMP.PRIVATE"(i32* %"parallel__do__simd_$I2"),
    "QUAL.OMP.PRIVATE"(i32* %"parallel__do__simd_$I1") ]
  br label %DIR.OMP.PARALLEL.338

DIR.OMP.PARALLEL.338:                             ; preds = %DIR.OMP.PARALLEL.2
  br label %DIR.OMP.LOOP.12

bb1:                                              ; preds = %DIR.OMP.LOOP.4
  store volatile i32 0, i32* %temp20, align 4
  %temp_fetch.11 = load i32, i32* %temp16, align 1
  %temp_fetch.12 = load i32, i32* %temp18, align 1
  %temp_fetch.13 = load volatile i32, i32* %temp20, align 4
  %mul.2 = mul nsw i32 %temp_fetch.13, %temp_fetch.12
  %add.1 = add nsw i32 %mul.2, %temp_fetch.11
  store i32 %add.1, i32* %"parallel__do__simd_$I1", align 1
  br label %DIR.OMP.SIMD.5

DIR.OMP.SIMD.5:                                   ; preds = %DIR.OMP.END.SIMD.740, %bb1
  %temp_fetch.14 = load i32, i32* %temp16, align 1
  %temp_fetch.15 = load i32, i32* %temp18, align 1
  %temp_fetch.16 = load volatile i32, i32* %temp20, align 4
  %mul.3 = mul nsw i32 %temp_fetch.16, %temp_fetch.15
  %add.2 = add nsw i32 %mul.3, %temp_fetch.14
  store i32 %add.2, i32* %"parallel__do__simd_$I1", align 1
  %temp10 = alloca i32, align 4
  %temp12 = alloca i32, align 4
  %temp13 = alloca i32, align 4
  %temp14 = alloca i32, align 4
  %temp15 = alloca i32, align 4
  %"parallel__do__simd_$N2_fetch.17" = load i32, i32* @"parallel__do__simd_$N2", align 1
  store i32 1, i32* %temp10, align 4
  store i32 1, i32* %temp12, align 4
  store i32 1, i32* %"parallel__do__simd_$I2", align 1
  store i32 0, i32* %temp13, align 4
  store volatile i32 0, i32* %temp14, align 4
  %sub.2 = add nsw i32 %"parallel__do__simd_$N2_fetch.17", -1
  store volatile i32 %sub.2, i32* %temp15, align 4
  br label %DIR.OMP.SIMD.4

DIR.OMP.SIMD.4:                                   ; preds = %DIR.OMP.SIMD.5
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV"(i32* %"parallel__do__simd_$I2", i32 1),
    "QUAL.OMP.NORMALIZED.IV"(i32* %temp14),
    "QUAL.OMP.NORMALIZED.UB"(i32* %temp15) ]
  br label %DIR.OMP.SIMD.539

DIR.OMP.SIMD.539:                                 ; preds = %DIR.OMP.SIMD.4
  br label %DIR.OMP.SIMD.1

bb6:                                              ; preds = %DIR.OMP.SIMD.1
  store volatile i32 0, i32* %temp14, align 4
  %temp_fetch.25 = load i32, i32* %temp10, align 4
  %temp_fetch.26 = load i32, i32* %temp12, align 4
  %temp_fetch.27 = load volatile i32, i32* %temp14, align 4
  %mul.4 = mul nsw i32 %temp_fetch.27, %temp_fetch.26
  %add.3 = add nsw i32 %mul.4, %temp_fetch.25
  store i32 %add.3, i32* %"parallel__do__simd_$I2", align 1
  br label %bb10

bb10:                                             ; preds = %bb12, %bb6
  %temp_fetch.28 = load i32, i32* %temp10, align 4
  %temp_fetch.29 = load i32, i32* %temp12, align 4
  %temp_fetch.30 = load volatile i32, i32* %temp14, align 4
  %mul.5 = mul nsw i32 %temp_fetch.30, %temp_fetch.29
  %add.4 = add nsw i32 %mul.5, %temp_fetch.28
  store i32 %add.4, i32* %"parallel__do__simd_$I2", align 1
  %temp9 = alloca float, align 4
  br label %bb11

bb11:                                             ; preds = %bb11, %bb10
  %"(i8*)parallel__do__simd_$COUNTER_N0$" = bitcast float* %"parallel__do__simd_$COUNTER_N0" to i8*
  %"(i8*)temp$" = bitcast float* %temp9 to i8*
  call void @__atomic_load(i64 4, i8* %"(i8*)parallel__do__simd_$COUNTER_N0$", i8* nonnull %"(i8*)temp$", i32 0)
  %temp_fetch.31 = load float, float* %temp9, align 4
  %add.5 = fadd reassoc ninf nsz arcp contract afn float %temp_fetch.31, 1.000000e+00
  %temp = alloca float, align 4
  store float %add.5, float* %temp, align 4
  %"(i8*)temp$6" = bitcast float* %temp to i8*
  %func_result8 = call i1 @__atomic_compare_exchange(i64 4, i8* %"(i8*)parallel__do__simd_$COUNTER_N0$", i8* nonnull %"(i8*)temp$", i8* nonnull %"(i8*)temp$6", i32 0, i32 0)
  br i1 %func_result8, label %bb12, label %bb11

bb12:                                             ; preds = %bb11
  %temp_fetch.33 = load volatile i32, i32* %temp14, align 4
  %add.6 = add nsw i32 %temp_fetch.33, 1
  store volatile i32 %add.6, i32* %temp14, align 4
  %temp_fetch.34 = load volatile i32, i32* %temp15, align 4
  %temp_fetch.35 = load volatile i32, i32* %temp14, align 4
  %rel.3.not = icmp sgt i32 %temp_fetch.35, %temp_fetch.34
  br i1 %rel.3.not, label %DIR.OMP.END.SIMD.7.loopexit, label %bb10

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.539
  %temp_fetch.22 = load i32, i32* %temp13, align 4
  store volatile i32 %temp_fetch.22, i32* %temp14, align 4
  %temp_fetch.23 = load volatile i32, i32* %temp14, align 4
  %temp_fetch.24 = load volatile i32, i32* %temp15, align 4
  %rel.2 = icmp slt i32 %temp_fetch.24, %temp_fetch.23
  br i1 %rel.2, label %DIR.OMP.END.SIMD.7, label %bb6

DIR.OMP.END.SIMD.7.loopexit:                      ; preds = %bb12
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SIMD.7.loopexit, %DIR.OMP.SIMD.1
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:                               ; preds = %DIR.OMP.END.SIMD.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.740

DIR.OMP.END.SIMD.740:                             ; preds = %DIR.OMP.END.SIMD.6
  %temp_fetch.37 = load volatile i32, i32* %temp20, align 4
  %add.7 = add nsw i32 %temp_fetch.37, 1
  store volatile i32 %add.7, i32* %temp20, align 4
  %temp_fetch.38 = load volatile i32, i32* %temp21, align 4
  %temp_fetch.39 = load volatile i32, i32* %temp20, align 4
  %rel.4.not = icmp sgt i32 %temp_fetch.39, %temp_fetch.38
  br i1 %rel.4.not, label %DIR.OMP.END.LOOP.8.loopexit, label %DIR.OMP.SIMD.5

DIR.OMP.LOOP.4:                                   ; preds = %DIR.OMP.LOOP.1243
  %temp_fetch.8 = load i32, i32* %temp19, align 1
  store volatile i32 %temp_fetch.8, i32* %temp20, align 4
  %temp_fetch.9 = load volatile i32, i32* %temp20, align 4
  %temp_fetch.10 = load volatile i32, i32* %temp21, align 4
  %rel.1 = icmp slt i32 %temp_fetch.10, %temp_fetch.9
  br i1 %rel.1, label %DIR.OMP.END.LOOP.8, label %bb1

DIR.OMP.END.LOOP.8.loopexit:                      ; preds = %DIR.OMP.END.SIMD.740
  br label %DIR.OMP.END.LOOP.8

DIR.OMP.END.LOOP.8:                               ; preds = %DIR.OMP.END.LOOP.8.loopexit, %DIR.OMP.LOOP.4
  br label %DIR.OMP.END.LOOP.841

DIR.OMP.END.LOOP.841:                             ; preds = %DIR.OMP.END.LOOP.8
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.10

DIR.OMP.END.PARALLEL.10:                          ; preds = %DIR.OMP.END.LOOP.841
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.PARALLEL.10
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.1042

DIR.OMP.END.PARALLEL.1042:                        ; preds = %DIR.OMP.END.PARALLEL.9
  ret void

DIR.OMP.LOOP.12:                                  ; preds = %DIR.OMP.PARALLEL.338
  %temp16 = alloca i32, align 4
  %temp17 = alloca i32, align 4
  %temp18 = alloca i32, align 4
  %temp19 = alloca i32, align 4
  %temp20 = alloca i32, align 4
  %temp21 = alloca i32, align 4
  %"parallel__do__simd_$N1_fetch.3" = load i32, i32* @"parallel__do__simd_$N1", align 1
  store i32 1, i32* %temp16, align 4
  store i32 %"parallel__do__simd_$N1_fetch.3", i32* %temp17, align 4
  store i32 1, i32* %temp18, align 4
  store i32 1, i32* %"parallel__do__simd_$I1", align 1
  store i32 0, i32* %temp19, align 4
  store volatile i32 0, i32* %temp20, align 4
  %temp_fetch.5 = load i32, i32* %temp18, align 4
  %temp_fetch.6 = load i32, i32* %temp16, align 4
  %temp_fetch.7 = load i32, i32* %temp17, align 4
  %sub.1 = sub nsw i32 %temp_fetch.7, %temp_fetch.6
  %div.1 = sdiv i32 %sub.1, %temp_fetch.5
  store volatile i32 %div.1, i32* %temp21, align 4
  br label %DIR.OMP.LOOP.11

DIR.OMP.LOOP.11:                                  ; preds = %DIR.OMP.LOOP.12
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE"(i32* %"parallel__do__simd_$I1"),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %temp19),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %temp18),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %temp17),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %temp16),
    "QUAL.OMP.NORMALIZED.IV"(i32* %temp20),
    "QUAL.OMP.NORMALIZED.UB"(i32* %temp21) ]
  br label %DIR.OMP.LOOP.1243

DIR.OMP.LOOP.1243:                                ; preds = %DIR.OMP.LOOP.11
  br label %DIR.OMP.LOOP.4
}

declare i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @__atomic_load(i64, i8*, i8*, i32) local_unnamed_addr
declare i1 @__atomic_compare_exchange(i64, i8*, i8*, i8*, i32, i32) local_unnamed_addr
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
