; RUN: opt -featureoutliner -S < %s | FileCheck %s
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

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
; CHECK: attributes #2 = { "target-cpu"="x86-64" "target-features"="+avx" }
attributes #2 = { "target-cpu"="x86-64" }
