; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

; This test verifies allocation call info collection for the transforms from
; memory allocation calls.

; Test with malloc creation of a structure pointer
%struct.test01 = type { i32, i32, i32 }
define void @test01() {
  %call = tail call i8* @malloc(i64 144)
  %p = bitcast i8* %call to %struct.test01*
  ret void
}
; CHECK: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01 = type { i32, i32, i32 }


; Test with malloc result cast to more than one type.
%struct.test02a = type { i32, i32, i32 }
%struct.test02b = type { i16, i16, i16, i16, i16, i16 }
define void @test02() {
  %p = tail call i8* @malloc(i64 132)
  %s1 = bitcast i8* %p to %struct.test02a*
  %s2 = bitcast i8* %p to %struct.test02b*
  ret void
}
; CHECK: Function: test02
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test02a = type { i32, i32, i32 }
; CHECK:     Type: %struct.test02b = type { i16, i16, i16, i16, i16, i16 }

; Test with malloc result cast to pointer to an array
%struct.test03 = type { i32, i32, i32 }
define void @test03(i64 %n) {
  %call = tail call i8* @malloc(i64 240)
  %p = bitcast i8* %call to [6 x %struct.test03*]*
  ret void
}
; CHECK: Function: test03
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: [6 x %struct.test03*]

; Test with malloc result that is not an aggregate type
define noalias i32* @test04() {
  %call = tail call noalias i8* @malloc(i64 100)
  %p = bitcast i8* %call to i32*
  ret i32* %p
}
; CHECK: Function: test04
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: Non-aggregate

; Test with malloc where result in stored within a member
; of another structure
%struct.test05b = type { i32, %struct.test05a*, i32 }
%struct.test05a = type { i32, i32*, i32 }
define void @test05(%struct.test05b* %in) {
  %call = tail call i8* @malloc(i64 504)
  %b = getelementptr inbounds %struct.test05b, %struct.test05b* %in, i32 0, i32 1
  %p = bitcast %struct.test05a** %b to i8**
  store i8* %call, i8** %p, align 4
  ret void
}
; CHECK: Function: test05
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test05a = type { i32, i32*, i32 }

; Test with realloc call
%struct.test06 = type { i32, i32, i32 }
define void @test06(%struct.test06* %in) {
  %p1 = bitcast %struct.test06* %in to i8*
  %call = tail call i8* @realloc(i8* %p1, i64 240)
  %p2 = bitcast i8* %call to %struct.test06*
  ret void
}
; CHECK: Function: test06
; CHECK: AllocCallInfo:
; CHECK:   Kind: Realloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test06 = type { i32, i32, i32 }

; Test with calloc creation of a structure pointer
%struct.test07 = type { i32, i32, i32 }
define void @test07() {
  %call = tail call i8* @calloc(i64 100, i64 24)
  %p = bitcast i8* %call to %struct.test07*
  ret void
}
; CHECK: Function: test07
; CHECK: AllocCallInfo:
; CHECK:   Kind: Calloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test07 = type { i32, i32, i32 }

; Test with user alloc-like wrapper
%struct.test08 = type { i32, i32, i32 }
define i8* @test08alloc(i64 %size) {
  %call = tail call noalias i8* @malloc(i64 %size)
  ret i8* %call
}

define void @test08(i64 %size) {
  %call = tail call i8* @test08alloc(i64 %size)
  %p = bitcast i8* %call to %struct.test08*
  ret void
}

; CHECK: Function: test08
; CHECK: AllocCallInfo:
; FIXME - User allocs are currently tagged as malloc:   Kind: UserAlloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test08 = type { i32, i32, i32 }


declare noalias i8* @calloc(i64, i64)
declare noalias i8* @malloc(i64)
declare noalias i8* @realloc(i8*, i64)
