; RUN: opt < %s -passes=instcombine -S | FileCheck %s

; REQUIRES: typed_pointers
; This test was for CMPLRLLVM-24134, commit 7ec7d45834.
; The memcpy calls are expected to be lowered to load/stores by
; code in InstCombineCalls. The optimization relies on either pointer types or
; TBAA MD, neither of which are being produced by the FFE.
; The optimization and the tests both need to be rewritten to match what
; the FFE is currently producing for CMPLRLLVM-24131 (cam4).

; For now, it's fine to leave it, as it's not actually clear that this
; optimization really created a performance gain for cam4.


; Check memcpy lowering for Fortran

%"ESMF_BASETIMEMOD$.btESMF_BASETIME" = type { i64, i64, i64, i64 }
%"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED" = type { %"ESMF_BASETIMEMOD$.btESMF_BASETIME", i64, i64 }

declare void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.i64"(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

declare void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.i64"(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

; @test1_: Check that a non-nested struct copy gets lowered if it is marked
; as Fortran.

; CHECK-LABEL: define void @test1_(ptr %0, ptr %1) #1 {
; CHECK-NEXT:  %3 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %1, i64 0, i32 0
; CHECK-NEXT:  %4 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %0, i64 0, i32 0
; CHECK-NEXT:  %5 = load i64, i64* %3, align 4
; CHECK-NEXT:  store i64 %5, i64* %4, align 1
; CHECK-NEXT:  %6 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %1, i64 0, i32 1
; CHECK-NEXT:  %7 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %0, i64 0, i32 1
; CHECK-NEXT:  %8 = load i64, i64* %6, align 4
; CHECK-NEXT:  store i64 %8, i64* %7, align 1
; CHECK-NEXT:  %9 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %1, i64 0, i32 2
; CHECK-NEXT:  %10 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %0, i64 0, i32 2
; CHECK-NEXT:  %11 = load i64, i64* %9, align 4
; CHECK-NEXT:  store i64 %11, i64* %10, align 1
; CHECK-NEXT:  %12 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %1, i64 0, i32 3
; CHECK-NEXT:  %13 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME", ptr %0, i64 0, i32 3
; CHECK-NEXT:  %14 = load i64, i64* %12, align 4
; CHECK-NEXT:  store i64 %14, i64* %13, align 1
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

define void @test1_(ptr %0, ptr %1) #1 {
  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.i64"(ptr nonnull align 1 dereferenceable(32) %0, ptr nonnull align 8 dereferenceable(32) %1, i64 32, i1 false)
  ret void
}

; @test2_: Check that a nested struct copy gets lowered if it is marked
; as Fortran.

; CHECK-LABEL: define void @test2_(ptr %0, ptr %1) #1 {
; CHECK-NEXT:  %3 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 0, i32 0
; CHECK-NEXT:  %4 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 0, i32 0
; CHECK-NEXT:  %5 = load i64, i64* %3, align 4
; CHECK-NEXT:  store i64 %5, i64* %4, align 1
; CHECK-NEXT:  %6 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 0, i32 1
; CHECK-NEXT:  %7 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 0, i32 1
; CHECK-NEXT:  %8 = load i64, i64* %6, align 4
; CHECK-NEXT:  store i64 %8, i64* %7, align 1
; CHECK-NEXT:  %9 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 0, i32 2
; CHECK-NEXT:  %10 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 0, i32 2
; CHECK-NEXT:  %11 = load i64, i64* %9, align 4
; CHECK-NEXT:  store i64 %11, i64* %10, align 1
; CHECK-NEXT:  %12 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 0, i32 3
; CHECK-NEXT:  %13 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 0, i32 3
; CHECK-NEXT:  %14 = load i64, i64* %12, align 4
; CHECK-NEXT:  store i64 %14, i64* %13, align 1
; CHECK-NEXT:  %15 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 1
; CHECK-NEXT:  %16 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 1
; CHECK-NEXT:  %17 = load i64, i64* %15, align 4
; CHECK-NEXT:  store i64 %17, i64* %16, align 1
; CHECK-NEXT:  %18 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %1, i64 0, i32 2
; CHECK-NEXT:  %19 = getelementptr inbounds %"ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTED", ptr %0, i64 0, i32 2
; CHECK-NEXT:  %20 = load i64, i64* %18, align 4
; CHECK-NEXT:  store i64 %20, i64* %19, align 1
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

define void @test2_(ptr %0, ptr %1) #1 {
  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.i64"(ptr nonnull align 1 dereferenceable(48) %0, ptr nonnull align 8 dereferenceable(48) %1, i64 48, i1 false)
  ret void
}


; @test3_: Check that a sufficienty large non-nested struct copy does NOT
; get lowered if it is NOT marked Fortran

; CHECK-LABEL: define void @test3_(ptr %0, ptr %1) {
; CHECK-NEXT:  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.i64"(ptr noundef nonnull align 1 dereferenceable(32) %0, ptr noundef nonnull align 8 dereferenceable(32) %1, i64 32, i1 false)
; CHECK-NEXT:  ret void
; CHECK-NEXT: }


define void @test3_(ptr %0, ptr %1) {
  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIMEs.i64"(ptr nonnull align 1 dereferenceable(32) %0, ptr nonnull align 8 dereferenceable(32) %1, i64 32, i1 false)
  ret void
}

; @test4_: Check that a sufficienty large nested struct copy does NOT
; get lowered if it is NOT marked Fortran

; CHECK-LABEL: define void @test4_(ptr %0, ptr %1) {
; CHECK-NEXT:  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.i64"(ptr noundef nonnull align 1 dereferenceable(48) %0, ptr noundef nonnull align 8 dereferenceable(48) %1, i64 48, i1 false)
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

define void @test4_(ptr %0, ptr %1) {
  call void @"llvm.memcpy.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.p0s_ESMF_BASETIMEMOD$.btESMF_BASETIME_NESTEDs.i64"(ptr nonnull align 1 dereferenceable(48) %0, ptr nonnull align 8 dereferenceable(48) %1, i64 48, i1 false)
  ret void
}

attributes #1 = { "intel-lang"="fortran" }

