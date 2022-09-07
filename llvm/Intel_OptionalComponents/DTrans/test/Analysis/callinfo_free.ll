; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s


; This test verifies allocation call info collection for the transforms from
; memory free calls.

; Test with free of a structure pointer whose type is resolved by the
; allocation type analyzer
%struct.test01 = type { i32, i32, i32 }
define void @test01() {
  %call = call i8* @malloc(i64 144)
  %pts = bitcast i8* %call to %struct.test01*
  %ptv = bitcast %struct.test01* %pts to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01 = type { i32, i32, i32 }
; CHECK: Function: test01
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01 = type { i32, i32, i32 }


; Test with free of a bitcast structure pointer
%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* %in) {
  %ptv = bitcast %struct.test02* %in to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test02
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test02 = type { i32, i32, i32 }


; Test with free that is passed a ptr-to-member of a struct type.
%struct.test03b = type { i32, %struct.test03a*, i32 }
%struct.test03a = type { i32, i32*, i32 }
define void @test03(%struct.test03b* %in) {
  %addr = getelementptr inbounds %struct.test03b, %struct.test03b* %in, i32 0, i32 1
  %ptr = load %struct.test03a*, %struct.test03a** %addr
  %ptv = bitcast %struct.test03a* %ptr to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test03
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test03a = type { i32, i32*, i32 }


; Test with free-like wrapper
%struct.test04 = type { i32, i32, i32 }
define void @test04_free_wrapper(i8* %in, i32 %size) {
  call void @free(i8* %in)
  ret void
}

define void @test04(i32 %size, %struct.test04* %in) {
  %ptv = bitcast %struct.test04* %in to i8*
  call void @test04_free_wrapper(i8* %ptv, i32 %size)
  ret void
}
; CHECK: Function: test04
; CHECK: FreeCallInfo:
; CHECK:   Kind: UserFree
; CHECK:   Aliased types:
; CHECK:     Type: Not analyzed

declare noalias i8* @malloc(i64)
declare void @free(i8*)
