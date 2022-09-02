; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -disable-output 2>&1 | FileCheck %s

; Verify that we don't consider assume as an unsafe call.

; CHECK: + DO i1 = 0, 1023, 1
; CHECK:   Has unsafe calls: no
; CHECK: + END LOOP


define void @scalar_soa() {
entry:
  %priv = alloca [1024 x i32], align 4
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  call void @llvm.assume(i1 true)
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %priv, i64 0, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body, label %omp.loop.exit

omp.loop.exit:
  ret void
}

declare void @llvm.assume(i1 noundef) #2

attributes #2 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
