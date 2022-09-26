; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test case checks that "Memfunc partial write (nested structure)" is set
; in the types since the data is being copied to the first field in the
; innermost structure.

; CHECK-LABEL: LLVMType: %class.TestClass = type { i64, i64, i64 }
; CHECK: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABEL: LLVMType: %class.inner1 = type { %class.inner2, %class.inner2 }
; CHECK: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABEL: LLVMType: %class.inner2 = type { %class.TestClass, i64, i64 }
; CHECK: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

; CHECK-LABE: LLVMType: %class.outer = type { %class.inner1 }
; CHECK: Safety data:{{.*}}Memfunc partial write (nested structure){{.*}}

%class.outer = type { %"class.inner1" }
%class.inner1 = type { %"class.inner2", %"class.inner2"}
%class.inner2 = type { %class.TestClass, i64, i64}
%class.TestClass = type { i64, i64, i64}

define void @copy(%class.outer* %arg, %class.outer* %arg2) {
  %tmp1 = bitcast %class.outer* %arg to i8*
  %tmp2 = bitcast %class.outer* %arg2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(8) %tmp1, i8* nonnull align 8 dereferenceable(8) %tmp2, i64 8, i1 false)
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)