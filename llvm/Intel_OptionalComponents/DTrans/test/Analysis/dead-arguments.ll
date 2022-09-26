; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKAT
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s -check-prefix=CHECKAT

; If the formal argument is not used inside the function, then it
; shouldn't be marked as address taken
%struct.test.a = type { i32, i32 }
%struct.test.b = type { i32, i32 }

%"class.std::allocator" = type { i8 }

declare void @baz(%struct.test.b* %arg)

define void @bar(%"class.std::allocator"* %arg, %struct.test.b* %arg2) {
  call void @baz(%struct.test.b* %arg2)
  ret void
}

define void @foo(%struct.test.a* %arg, %struct.test.b* %arg2) {
  %tmp = bitcast %struct.test.a* %arg to %"class.std::allocator"*
  call void @bar(%"class.std::allocator"* %tmp, %struct.test.b* %arg2)
  ret void
}

; CHECKAT: LLVMType: %struct.test.a = type { i32, i32 }
; CHECKAT-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECKAT: LLVMType: %struct.test.b = type { i32, i32 }
; CHECKAT: Safety data:{{.*}}Address taken{{.*}}