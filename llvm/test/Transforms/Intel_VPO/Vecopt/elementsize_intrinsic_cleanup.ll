; RUN: opt -passes="vpo-directive-cleanup" -print-after=vpo-directive-cleanup -disable-output < %s 2>&1 | FileCheck %s

; CHECK-NOT: call void @llvm.intel.directive.elementsize

define void @_Z3fooPi(ptr nocapture noundef writeonly %a) {
DIR.OMP.SIMD.1:
  br label %loop.22

loop.22:                                          ; preds = %loop.22, %DIR.OMP.SIMD.1
  %i1.i64.0 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %nextivloop.22, %loop.22 ]
  call void @llvm.intel.directive.elementsize(ptr nocapture noundef writeonly %a, i64 4)
  %0 = getelementptr inbounds i32, ptr %a, i64 %i1.i64.0
  %1 = trunc i64 %i1.i64.0 to i32
  %2 = insertelement <8 x i32> undef, i32 %1, i64 0
  %3 = shufflevector <8 x i32> %2, <8 x i32> poison, <8 x i32> zeroinitializer
  %4 = add <8 x i32> %3, <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8>
  store <8 x i32> %4, ptr %0, align 4
  %nextivloop.22 = add nuw nsw i64 %i1.i64.0, 8
  %condloop.22 = icmp ult i64 %i1.i64.0, 248
  br i1 %condloop.22, label %loop.22, label %afterloop.22

afterloop.22:                                     ; preds = %loop.22
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)
