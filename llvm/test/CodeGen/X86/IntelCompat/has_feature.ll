; RUN: llc < %s -mtriple=x86_64-pc-linux | FileCheck %s
; RUN: llc < %s -O0 -mtriple=x86_64-pc-linux | FileCheck %s

define void @has_feature(i1 %p, i64 %v) #2 {
;CHECK-LABEL: has_feature:
;CHECK: retq
  %f = call i1 @llvm.has.feature(i64 64)
  br label %end
  
end:
  call void @llvm.assume(i1 %f)
  ret void
}

declare i1 @llvm.has.feature(i64) #0
declare void @llvm.assume(i1) #1

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { "target-cpu"="x86-64" }
