; RUN: opt -passes="loop(loop-deletion)" < %s -S | FileCheck %s

; Test checks that loop deletion will also deletes region directives

; CHECK-NOT: SIMD

define void @foo() {
entry:
  %simd = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %pzero = alloca i64
  store i64 0, i64* %pzero
  %zero = load i64, i64* %pzero
  br label %loop.body

loop.body:
  %i = phi i64 [ %zero, %entry ], [ %nexti, %loop.body ]
  %nexti = add i64 %i, 1
  %test = icmp eq i64 %nexti, 100
  br i1 %test, label %loop.end, label %loop.body

loop.end:
  call void @llvm.directive.region.exit(token %simd) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

