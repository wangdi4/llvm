; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test case checks that "Memfunc partial write (nested structure)" is not
; set since the input size for memset is a variable.

; more bytes in field 2 from %class.inner2.
; CHECK-LABEL: LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECK-NOT: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABEL: LLVMType: %class.inner1 = type { %class.inner2, %class.inner2 }
; CHECK-NOT: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABEL: LLVMType: %class.inner2 = type { %class.TestClass, i64, i64 }
; CHECK-NOT: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABE: LLVMType: %class.outer = type { %class.inner1 }
; CHECK-NOT: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

%class.outer = type { %"class.inner1" }
%class.inner1 = type { %"class.inner2", %"class.inner2"}
%class.inner2 = type { %class.TestClass, i64, i64}
%class.TestClass = type { i64, i64, i64}

define void @set(%class.outer* %arg, %class.outer* %arg2, i64 %size) {
  %tmp1 = bitcast %class.outer* %arg to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %tmp1, i8 0, i64 %size, i1 false)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
