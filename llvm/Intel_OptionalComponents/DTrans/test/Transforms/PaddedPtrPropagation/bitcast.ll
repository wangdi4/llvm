;RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks padding propagation for bitcast

;CHECK:      ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: -1
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK:      ==== END OF INITIAL FUNCTION SET ====

;CHECK:      ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(foo):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Return Padding: 4
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:     i32* %p : -1
;CHECK-NEXT:   Value paddings:
;CHECK:          %1 = bitcast i32* %0 to float* :: 4
;CHECK:      ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@.str = private unnamed_addr constant [10 x i8] c"bitcast.c\00"

define float* @foo(i32* %p) {
entry:
    %0 = tail call i32* @llvm.ptr.annotation.p0i32(i32* %p, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 2, i8* null)
    %1 = bitcast i32* %0 to float*
    ret float* %1
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)
