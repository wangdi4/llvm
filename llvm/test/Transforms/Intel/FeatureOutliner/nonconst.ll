; UNSUPPORTED: asserts
; RUN: opt -featureoutliner -S < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

; CHECK-LABEL: define i1 @outline(i1 %p, i64 %v) #2 {
; CHECK: @llvm.has.feature(i64 %v)
; CHECK: @llvm.has.feature(i64 4096)
; CHECK: ret i1
define i1 @outline(i1 %p, i64 %v) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 %v)
  call void @llvm.assume(i1 %f)
  br label %end
  
end:
  %g = call i1 @llvm.has.feature(i64 4096)
  ret i1 %g
}

; CHECK-NOT: define
attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #2 = { "target-cpu"="x86-64" }
;CHECK-NOT: attributes
