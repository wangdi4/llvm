; RUN: opt -featureoutliner -S < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

; CHECK-LABEL: define void @nomark() #2 {
; CHECK-NEXT:  %f = call i1 @llvm.has.feature(i64 65536)
; CHECK-NOT: call x86_fastcallc
define void @nomark() #2 {
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  ret void
}

; CHECK-LABEL: define void @nooutline(i1 %p) #2 {
; CHECK:       %f = call i1 @llvm.has.feature(i64 65536)
; CHECK-NOT: call x86_fastcallc
define void @nooutline(i1 %p) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  br label %end
  
end:
  ret void
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="sandybridge" }
