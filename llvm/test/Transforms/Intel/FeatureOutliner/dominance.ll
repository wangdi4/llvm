; RUN: opt -featureoutliner -S < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

declare void @foo() #1

; CHECK-LABEL: define void @dom1(i1 %p) #2
; CHECK: call x86_fastcallcc void @dom1_if()
; CHECK-NOT: @llvm.has.feature
; CHECK: ret void
define void @dom1(i1 %p) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  br label %next

next:
  call void @foo()
  br label %end
  
end:
  ret void
}

; CHECK-LABEL: define internal x86_fastcallcc void @dom1_if() #3
; CHECK: %f = call i1 @llvm.has.feature(i64 65536)
; CHECK: next:
; CHECK: }

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #3 = { "target-cpu"="x86-64" "target-features"="+avx" }
