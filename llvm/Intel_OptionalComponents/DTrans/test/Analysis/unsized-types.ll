; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -dtransanalysis  -disable-output %s 2>/dev/null
; RUN: opt -passes='require<dtransanalysis>'  -disable-output %s 2>/dev/null

; This test verifies that DTransAnalysis shouldn't compfail when processing
; unsized types.

; unsized type
%struct.test02 = type opaque

define i32 @main(i32 %argc, i8** %argv) {
  %c1 = call %struct.test02* @test01(i64 1)
  %c2 = bitcast %struct.test02* %c1 to i8*
  %c3 = getelementptr i8, i8* %c2, i64 8
  ret i32 0
}

define %struct.test02* @test01(i64 %idx1) {
  %p = call noalias i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.test02*
  ret %struct.test02* %s1
}

declare noalias i8* @malloc(i64)
