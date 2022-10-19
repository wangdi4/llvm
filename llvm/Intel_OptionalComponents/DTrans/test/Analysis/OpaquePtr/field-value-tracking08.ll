; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify field value collection of structure fields is
; handled for global variable instances of structures.

%struct.test01 = type { i32, float }
@test01 = internal global %struct.test01 zeroinitializer
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, float }
@test02a = internal global %struct.test02 zeroinitializer
@test02b = internal global %struct.test02 { i32 1, float 1.700000e+01 }
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 0, 1 ] <complete>
; CHECK:   1)Field LLVM Type: float
; CHECK:     Multiple Value: [ 0.000000e+00, 1.700000e+01 ] <complete>
; CHECK:   Safety data: Global instance | Has initializer list{{ *$}}
; CHECK: End LLVMType: %struct.test02


; An aggregate type with zero initialization
%struct.test03 = type { %struct.test03inner, i32 }
%struct.test03inner = type { i32, i32 }
@test03 = internal global %struct.test03 zeroinitializer
; CHECK-LABEL: LLVMType: %struct.test03
; CHECK:   0)Field LLVM Type: %struct.test03inner = type { i32, i32 }
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   Safety data: Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03

; CHECK-LABEL: LLVMType: %struct.test03inner
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   Safety data: Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03inner


; An aggregate type with an initializer list
%struct.test04 = type { %struct.test04inner, i32 }
%struct.test04inner = type { i32, i32 }
@test04 = internal global %struct.test04 { %struct.test04inner { i32 5, i32 10 }, i32 15 }
; CHECK-LABEL: LLVMType: %struct.test04
; CHECK:   0)Field LLVM Type: %struct.test04inner = type { i32, i32 }
; CHECK:     Multiple Value: [  ] <incomplete>
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 15
; CHECK:   Safety data: Global instance | Has initializer list | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04

; CHECK-LABEL: LLVMType: %struct.test04inner
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 5
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 10
; CHECK:   Safety data: Global instance | Has initializer list | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04inner


; Multi-dimension array of structures with a zero initializer
%struct.test05 = type { i32, float }
@test05 = internal global [4 x [2 x %struct.test05]] zeroinitializer
; CHECK-LABEL: LLVMType: %struct.test05
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: Global instance | Global array{{ *$}}
; CHECK: End LLVMType: %struct.test05


; Array type with an initializer list
%struct.test06 = type { i32, float }
@test06 = internal global [2 x %struct.test06] [ %struct.test06 { i32 1, float 3.400000e+01 },
                                                 %struct.test06 { i32 4, float 5.600000e+01 }]
; CHECK-LABEL: LLVMType: %struct.test06
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 1, 4 ] <complete>
; CHECK:   1)Field LLVM Type: float
; CHECK:     Multiple Value: [ 3.400000e+01, 5.600000e+01 ] <complete>
; CHECK:   Safety data: Global instance | Has initializer list | Global array{{ *$}}
; CHECK: End LLVMType: %struct.test06


!1 = !{i32 0, i32 0}  ; i32
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test03inner zeroinitializer, i32 0}  ; %struct.test03inner
!4 = !{%struct.test04inner zeroinitializer, i32 0}  ; %struct.test04inner
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!6 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!7 = !{!"S", %struct.test03 zeroinitializer, i32 2, !3, !1} ; { %struct.test03inner, i32 }
!8 = !{!"S", %struct.test03inner zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test04 zeroinitializer, i32 2, !4, !1} ; { %struct.test04inner, i32 }
!10 = !{!"S", %struct.test04inner zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!12 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !2} ; { i32, float }

!intel.dtrans.types = !{!5, !6, !7, !8, !9, !10, !11, !12}
