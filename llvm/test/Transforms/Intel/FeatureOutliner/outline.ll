; RUN: opt -featureoutliner -S -consistent-vector-abi < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

; CHECK-LABEL: define void @outline(i1 %p) #2 {
; CHECK: call x86_fastcallcc void @outline_if()
; CHECK-NOT: @llvm.has.feature
; CHECK: ret void
define void @outline(i1 %p) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  br label %end
  
end:
  ret void
}

declare void @vec(i32 %b, <8 x float> %v)

define void @implicit(i1 %p, <8 x float>* %ptr) #2 {
  br i1 %p, label %if, label %end
  
if:
  %v = load <8 x float>, <8 x float>* %ptr, align 16
  call void @vec(i32 1, <8 x float> %v)
  br label %end
  
end:
  ret void
}

; CHECK-LABEL: define internal x86_fastcallcc void @outline_if() #3
; CHECK: %f = call i1 @llvm.has.feature(i64 65536)
; CHECK: }
; CHECK-LABEL: define internal x86_fastcallcc void @implicit_if(<8 x float>* %ptr) #3
; CHECK: call void @vec(i32 1, <8 x float> %v)
; CHECK: }
attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #3 = { "target-cpu"="x86-64" "target-features"="+avx" }
