; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

%struct.test01 = type { i32, i32 }
@g_instance.test01 = internal unnamed_addr global %struct.test01 zeroinitializer

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: Global instance
; CHECK-NOT: Global pointer

%struct.test02 = type { i32, i32 }
@g_ptr.test02 = internal unnamed_addr global %struct.test02* null

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Global pointer
; CHECK-NOT: Global instance

%struct.test03 = type { i32, i32 }
@g_ptr.test03 = internal unnamed_addr global %struct.test03* null
@g_instance.test03 = internal unnamed_addr global %struct.test03 zeroinitializer

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Global pointer | Global instance

%struct.test04 = type { i32, i32 }
@g_instance.test04 = internal unnamed_addr global %struct.test04 { i32 2, i32 3 }

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Global instance | Has initializer list
