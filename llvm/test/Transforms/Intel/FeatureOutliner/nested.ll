; RUN: opt -featureoutliner -S < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

; CHECK-LABEL: define void @nested(i1 %p, i1 %q) #2 {
; CHECK: call x86_fastcallcc i1 @nested_if(i1 %q)
; CHECK: ret void
define void @nested(i1 %p, i1 %q) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f)
  br i1 %q, label %nest, label %end
  
nest:
  %f2 = call i1 @llvm.has.feature(i64 8388608)
  call void @llvm.assume(i1 %f2)
  br label %end
  
end:
  ret void
}

; If we're nesting an AVX block inside an AVX2 block, do not
; outline it, since AVX2 implies AVX
; CHECK-LABEL: define void @reverse_nested(i1 %p, i1 %q) #2 {
; CHECK: call x86_fastcallcc i1 @reverse_nested_if(i1 %q)
; CHECK: ret void
define void @reverse_nested(i1 %p, i1 %q) #2 {
  br i1 %p, label %if, label %end
  
if:
  %f = call i1 @llvm.has.feature(i64 8388608)
  call void @llvm.assume(i1 %f)
  br i1 %q, label %nest, label %end
  
nest:
  %f2 = call i1 @llvm.has.feature(i64 65536)
  call void @llvm.assume(i1 %f2)
  br label %end
  
end:
  ret void
}

; CHECK-LABEL: define internal x86_fastcallcc void @nested_nest() #3
; CHECK: %f2 = call i1 @llvm.has.feature(i64 8388608)
; CHECK: }
; CHECK-LABEL: define internal x86_fastcallcc i1 @nested_if(i1 %q) #4
; CHECK: %f = call i1 @llvm.has.feature(i64 65536)
; CHECK: call x86_fastcallcc void @nested_nest()
; CHECK: }
; CHECK-LABEL: define internal x86_fastcallcc i1 @reverse_nested_if(i1 %q) #3
; CHECK: %f = call i1 @llvm.has.feature(i64 8388608)
; CHECK-NOT: } 
; CHECK: %f2 = call i1 @llvm.has.feature(i64 65536)
; CHECK: }

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #2 = { "target-cpu"="x86-64" }
;CHECK: attributes #3 = { "target-cpu"="x86-64" "target-features"="+avx2" }
;CHECK: attributes #4 = { "target-cpu"="x86-64" "target-features"="+avx" }
