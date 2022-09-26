; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test case checks that "Memfunc partial write (nested structure)" is not
; set since the input size for memset doesn't cover the fields exactly. In
; other words, the number of bytes will reach field 0 and 1, and 3

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

define void @set(%class.outer* %arg, %class.outer* %arg2) {
  %tmp1 = bitcast %class.outer* %arg to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %tmp1, i8 0, i64 35, i1 false)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)