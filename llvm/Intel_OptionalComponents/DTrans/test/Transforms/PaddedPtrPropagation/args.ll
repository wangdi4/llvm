; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="internalize,padded-pointer-prop" < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Checks merging of the padding of function arguments

;CHECK: ==== INITIAL FUNCTION SET ====
;CHECK:      Function info(caller1):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 8, i8* null) :: 4
;CHECK-NEXT:      %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 9, i8* null) :: 32

;CHECK:      Function info(caller2):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 15, i8* null) :: 8
;CHECK-NEXT:      %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @3, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 16, i8* null) :: 16
;CHECK: ==== END OF INITIAL FUNCTION SET ====

;CHECK: ==== TRANSFORMED FUNCTION SET ====
;CHECK:      Function info(callee):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Arguments' Padding:
;CHECK-NEXT:      i32* %ip : 4
;CHECK-NEXT:      float* %fp : 16
;CHECK-NEXT:   Value paddings:

;CHECK:      Function info(caller1):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 8, i8* null) :: 4
;CHECK-NEXT:      %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 9, i8* null) :: 32

;CHECK:      Function info(caller2):
;CHECK-NEXT:   HasUnknownCallSites: 0
;CHECK-NEXT:   Value paddings:
;CHECK-NEXT:      %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 15, i8* null) :: 8
;CHECK-NEXT:      %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @3, i64 0, i64 0), i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 16, i8* null) :: 16
;CHECK: ==== END OF TRANSFORMED FUNCTION SET ====

@.str = private constant [7 x i8] c"args.c\00"
@0 = private constant [16 x i8] c"padded 32 bytes\00"
@1 = private constant [15 x i8] c"padded 8 bytes\00"
@2 = private constant [15 x i8] c"padded 4 bytes\00"
@3 = private constant [16 x i8] c"padded 16 bytes\00"

define i32 @caller1() {
entry:
  %arr = alloca [16 x i32]
  %0 = bitcast [16 x i32]* %arr to i8*
  %arrayidx = getelementptr [16 x i32], [16 x i32]* %arr, i64 0, i64 0
  %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr ([15 x i8], [15 x i8]* @2, i64 0, i64 0), i8* getelementptr ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 8, i8* null)
  %2 = bitcast [16 x i32]* %arr to float*
  %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 9, i8* null)
  %call = call i32 @callee(i32* %1, float* %3)
  ret i32 %call
}

define i32 @caller2() {
entry:
  %arr = alloca [16 x i32]
  %0 = bitcast [16 x i32]* %arr to i8*
  %arrayidx = getelementptr [16 x i32], [16 x i32]* %arr, i64 0, i64 0
  %1 = call i32* @llvm.ptr.annotation.p0i32(i32* %arrayidx, i8* getelementptr ([15 x i8], [15 x i8]* @1, i64 0, i64 0), i8* getelementptr ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 15, i8* null)
  %2 = bitcast [16 x i32]* %arr to float*
  %3 = call float* @llvm.ptr.annotation.p0f32(float* %2, i8* getelementptr ([16 x i8], [16 x i8]* @3, i64 0, i64 0), i8* getelementptr ([7 x i8], [7 x i8]* @.str, i64 0, i64 0), i32 16, i8* null)
  %call = call i32 @callee(i32* %1, float* %3)
  ret i32 %call
}

define internal i32 @callee(i32* %ip, float* %fp) {
entry:
  %0 = load float, float* %fp
  %1 = load i32, i32* %ip
  %conv = sitofp i32 %1 to float
  %cmp = fcmp olt float %0, %conv
  %conv1 = zext i1 %cmp to i32
  ret i32 %conv1
}

declare i32* @llvm.ptr.annotation.p0i32(i32*, i8*, i8*, i32, i8*)
declare float* @llvm.ptr.annotation.p0f32(float*, i8*, i8*, i32, i8*)
