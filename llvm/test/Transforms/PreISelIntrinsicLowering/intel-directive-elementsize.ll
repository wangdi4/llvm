; RUN: opt -mtriple=x86_64-pc-linux-gnu -pre-isel-intrinsic-lowering -S -o - %s | FileCheck %s

; Verify that llvm.intel.directive.elementsize intrinsic is removed

; CHECK-NOT: call void @llvm.intel.directive.elementsize

define void @_Z3fooPi(ptr nocapture noundef writeonly %a) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  call void @llvm.intel.directive.elementsize(ptr nocapture noundef writeonly %a, i64 4)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv.next to i32
  store i32 %0, ptr %arrayidx, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)
