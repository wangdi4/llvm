; INTEL_FEATURE_SW_DTRANS

; This test is to verify that !intel.dtrans.func.type information is NOT
; propagated to a 'declare' created when an 'alias' is converted.
;
; In the case of @testAlias, the value type for the alias and aliasee do not
; match which prevents propagation.
;
 ; In the case of @simpleAlias2, the value types for the alias and aliasee match, but
; the metadata cannot simply be copied from the original variable. Instead,
; additional support would be needed to identify the type of the the element
; being referred to by the constant expression, which is not currently
; implemented.

; RUN: llvm-extract -keep-dtranstypemetadata -func test -S < %s | FileCheck %s

@globalVar = internal global i32 zeroinitializer
@globalPtr = internal global [4 x ptr] zeroinitializer, !intel_dtrans_type !1

@testAlias = internal unnamed_addr alias ptr (ptr), ptr @testTarget
@simpleAlias = private alias i32, ptr @globalVar
@simpleAlias2 = private alias ptr, ptr getelementptr ([4 x ptr], ptr @globalPtr, i64 1)

define   "intel_dtrans_func_index"="1" ptr @testTarget(ptr "intel_dtrans_func_index"="2" %in, ...) !intel.dtrans.func.type !4 {
  ret ptr null
}

define void @test() {
  %val = load i32, ptr @simpleAlias
  %ptr = load ptr, ptr @simpleAlias2
  %tmp = call ptr @testAlias(ptr %ptr)
  ret void
}

; CHECK: @simpleAlias2 = external global ptr
; CHECK-NOT: !intel_dtrans_type
; CHECK: declare ptr @testAlias(ptr)

!1 = !{!"A", i32 4, !2}  ; [4 x i32*]
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3, !2}

!intel.dtrans.types = !{}

; end INTEL_FEATURE_SW_DTRANS
