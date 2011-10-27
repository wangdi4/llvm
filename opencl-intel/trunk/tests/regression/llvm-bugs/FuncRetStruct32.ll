; Reproduder test for CSSD100005511
; The Win32 native compiler, VC, generates a call to a function returning a struct in the following manner:
; 1. The caller allocates a buffer which will be filled by the callee with the returned value
; 2. Before calling, the caller pushes the address of this buffer on the stack
; 3. Unlike other targets, such as Linux32, the callee does not pop the address before returning

; RUN: llc -mtriple=i686-pc-win32 < %s | FileCheck %s -check-prefix=WIN32
; RUN: llc -mtriple=i686-pc-linux < %s | FileCheck %s -check-prefix=LINUX32

; LINUX32: test_callee5:
; LINUX32: ret     $4
; LINUX32: test_caller5:
; LINUX32: call   test_callee5
; LINUX32: subl    $4, %esp
; LINUX32: call   test_callee5
; LINUX32: ret

; WIN32: test_callee5:
; WIN32-NOT: ret $4
; WIN32: ret
; WIN32: call   _test_callee5
; WIN32-NOT: subl    $4, %esp
; WIN32: call   _test_callee5
; WIN32: ret

%struct.AStruct = type { i32, i32, float, float, i32 }

define void @test_callee5(%struct.AStruct* noalias nocapture sret %agg.result) nounwind noinline {
entry:
  %0 = getelementptr inbounds %struct.AStruct* %agg.result, i64 0, i32 0
  store i32 0, i32* %0, align 4
  %1 = getelementptr inbounds %struct.AStruct* %agg.result, i64 0, i32 1
  %2 = bitcast i32* %1 to i8*
  call void @llvm.memset.i64(i8* %2, i8 0, i64 16, i32 4)
  ret void
}

define void @test_caller5() nounwind {
entry:
  %g = alloca %struct.AStruct, align 8
  %h = alloca %struct.AStruct, align 8
  call void @test_callee5(%struct.AStruct* noalias sret %g) nounwind
  call void @test_callee5(%struct.AStruct* noalias sret %h) nounwind
  ret void
}

declare void @llvm.memset.i64(i8* nocapture, i8, i64, i32) nounwind


