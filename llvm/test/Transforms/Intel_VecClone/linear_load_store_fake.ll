; Check that we create a fake memory for linear(uval()) pointer-parameter pointee when we see a store to that pointer.

; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; Entry BB
; CHECK: %alloca.fake.a = alloca <4 x i32>, align 16
; CHECK: %load.a1 = load ptr, ptr %alloca.a, align 8
; CHECK: %load.elem.a = load i32, ptr %load.a1, align 4
; CHECK: %.splatinsert = insertelement <4 x i32> poison, i32 %load.elem.a, i64 0
; CHECK: %.splat = shufflevector <4 x i32> %.splatinsert, <4 x i32> poison, <4 x i32> zeroinitializer
; CHECK: store <4 x i32> %.splat, ptr %alloca.fake.a, align 16

; Loop header BB
; CHECK: %alloca.fake.a.gep = getelementptr i32, ptr %alloca.fake.a, i32 %index
; CHECK: %0 = load i32, ptr %alloca.fake.a.gep, align 4
; CHECK: store i32 %add, ptr %alloca.fake.a.gep, align 4

define dso_local noundef i32 @_Z3fooRii(ptr noundef nonnull align 4 dereferenceable(4) %a, i32 noundef %m) local_unnamed_addr #0 {
entry:
  call void @llvm.intel.directive.elementsize(ptr nonnull %a, i64 4)
  %0 = load i32, ptr %a, align 4
  %add = add nsw i32 %0, %m
  store i32 %add, ptr %a, align 4
  ret i32 %add
}

attributes #0 = { "vector-variants"="_ZGVbN4Us1u__Z3fooRii" }
declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)
