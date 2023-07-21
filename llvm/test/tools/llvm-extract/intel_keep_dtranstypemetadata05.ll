; INTEL_FEATURE_SW_DTRANS

; This test is to verify that !intel.dtrans.func.type information is propagated
; to a 'declare' created when an 'alias' is replaced as a function declaration
; during extraction.

; RUN: llvm-extract -keep-dtranstypemetadata -func test -S < %s | FileCheck %s

@globalVar = internal global i32 zeroinitializer
@globalPtr = internal global ptr zeroinitializer, !intel_dtrans_type !1

@testAlias = internal unnamed_addr alias ptr (ptr), ptr @testTarget
@simpleAlias = private alias i32, ptr @globalVar
@simpleAlias2 = private alias ptr, ptr @globalPtr

define  "intel_dtrans_func_index"="1" ptr @testTarget(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !3 {
  ret ptr null
}

define void @test() {
  %val = load i32, ptr @simpleAlias
  %ptr = load ptr, ptr @simpleAlias2
  %tmp = call ptr @testAlias(ptr %ptr)
  ret void
}

!intel.dtrans.types = !{}
!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2, !1}

; CHECK: @simpleAlias2 = external global ptr, !intel_dtrans_type ![[MD_PI32:[0-9]+]]
; CHECK: declare !intel.dtrans.func.type ![[MD_FTYPE:[0-9]+]] "intel_dtrans_func_index"="1" ptr @testAlias(ptr "intel_dtrans_func_index"="2")

; CHECK: ![[MD_PI32]] = !{i32 0, i32 1}
; CHECK: ![[MD_FTYPE]] = distinct !{![[MD_PI8:[0-9]+]], ![[MD_PI32]]}
; CHECK: ![[MD_PI8]] = !{i8 0, i32 1}

; end INTEL_FEATURE_SW_DTRANS
