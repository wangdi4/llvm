; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; CMPLRLLVM-8956: This verifies that compiler (Dtrans) shouldn't
; fail when handling bitcast instructions involving vector types
; in foo and bar. Makes sure Bad casting is set for %struct.C.

; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check "Bad casting" set for %struct.C
;CHECK: DTRANS_StructInfo:
;CHECK:   LLVMType: %struct.C = type { i64, i64, i64 }
;CHECK:   Safety data: {{.*}}Bad casting{{.*}}

%struct.A = type { i64, i64, i64 }
%struct.B = type { i64, i64, i64 }
%struct.C = type { i64, i64, i64 }

define dso_local void @foo() {
 %call = tail call i8* @malloc(i64 24);
 %p0 = bitcast i8* %call to i32*
 %p1 = bitcast i8* %call to <4 x i32>*
 store <4 x i32> <i32 48, i32 57, i32 1632, i32 1641>, <4 x i32>* %p1, align 4
 %p2 = getelementptr inbounds i32, i32* %p0, i64 8
 %p3 = bitcast i32* %p2 to <4 x i32>*
 ret void
}

define dso_local void @bar() {
 %call = tail call i8* @malloc(i64 24);
 %p0 = bitcast i8* %call to %struct.A*
 %p1 = bitcast i8* %call to i32*
 %p2 = bitcast i32* %p1 to <8 x i32>*
 ret void
}

define dso_local <8 x i32>* @baz(%struct.C* %p0) {
 %p3 = bitcast %struct.C* %p0 to <8 x i32>*
 ret <8 x i32>* %p3
}
declare i8* @malloc(i64)
