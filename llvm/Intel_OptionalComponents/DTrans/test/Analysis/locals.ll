; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct identification of types used in alloca
; instructions.

; Note: '{{$}}' matches the end of a line. It's here to make sure there are
;       no unintended safety conditions lurking beyond the text we check.

define void @test() {
  %t1 = alloca %struct.test01, align 8

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: Local instance{{$}}

  %t2 = alloca %struct.test02*

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Local pointer{{$}}

  %t3.a = alloca %struct.test03, align 8
  %t3.b = alloca %struct.test03*

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Local pointer | Local instance{{$}}

  ; This allocates stack space for four instances of the structure.
  %t4 = alloca %struct.test04, i32 4, align 32

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Local instance{{$}}

  ; This allocates stack space for four pointers to the structure.
  %t5 = alloca %struct.test05*, i32 4

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Local pointer{{$}}

  ; This allocates a fixed array of instances of the structure.
  %t6 = alloca [16 x %struct.test06], align 128

; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Local instance{{$}}

  ; This allocates a fixed array of pointers to the structure.
  %t7 = alloca [16 x %struct.test07*]

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: Local pointer{{$}}

  ; This allocates a pointer to a fixed array of instances of the structure.
  %t8 = alloca [16 x %struct.test08]*

; CHECK: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: Local pointer{{$}}

  ; This allocates a fixed array of instances of the structure.
  %t9 = alloca [16 x [16 x %struct.test09]], align 2048

; CHECK: LLVMType: %struct.test09 = type { i32, i32 }
; CHECK: Safety data: Local instance{{$}}

  ; This allocates a fixed array of pointers to the structure.
  %t10 = alloca [16 x [16 x %struct.test10*]]

; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: Local pointer{{$}}

  ret void
}

%struct.test01 = type { i32, i32 }
%struct.test02 = type { i32, i32 }
%struct.test03 = type { i32, i32 }
%struct.test04 = type { i32, i32 }
%struct.test05 = type { i32, i32 }
%struct.test06 = type { i32, i32 }
%struct.test07 = type { i32, i32 }
%struct.test08 = type { i32, i32 }
%struct.test09 = type { i32, i32 }
%struct.test10 = type { i32, i32 }
