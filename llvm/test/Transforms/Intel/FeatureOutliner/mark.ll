; RUN: opt -featureoutliner -S -consistent-vector-abi < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

; CHECK-LABEL: define void @mark() #2 {
; CHECK-NEXT:  %f = call i1 @llvm.has.feature(i64 65536)
; CHECK-NOT: call x86_fastcallc
define void @mark() #2 {
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  ret void
}

; CHECK-LABEL: define void @mark2(i32 %p, <8 x float> %q) #2
define void @mark2(i32 %p, <8 x float> %q) #2 {
  ret void
}

; CHECK-LABEL: define <8 x float> @mark3() #2
define <8 x float> @mark3() #2 {
  ret <8 x float> zeroinitializer
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
; CHECK: attributes #2 = { "target-cpu"="x86-64" "target-features"="+avx" }
