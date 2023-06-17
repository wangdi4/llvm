; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-nested-simd-strategy=outermost -disable-output 2>&1 | FileCheck %s

define void @foo(ptr %a) {
; CHECK-LABEL: Function: foo
; CHECK:       BEGIN REGION { modified }
; CHECK-NOT:     @llvm.directive.region.entry
; CHECK-NOT:     @llvm.directive.region.exit
; CHECK:       END REGION
entry:
  br label %header

header:
  %token = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body

body:
  %i = phi i32 [ 0, %header ], [ %i.next, %exit.inner ]
  %index = getelementptr ptr, ptr %a, i32 %i
  %arr = load ptr, ptr %index
  br label %header.inner

header.inner:
  %token.inner = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body.inner

body.inner:
  %j = phi i32 [ 0, %header.inner ], [ %j.next, %body.inner ]

  %index.inner = getelementptr i32, ptr %arr, i32 %j
  %val = add i32 %i, %j
  store i32 %val, ptr %index.inner

  %j.next = add i32 %j, 1
  %cmp.inner = icmp eq i32 %j, 63
  br i1 %cmp.inner, label %latch.inner, label %body.inner

latch.inner:
  call void @llvm.directive.region.exit(token %token.inner) [ "DIR.OMP.END.SIMD"() ]
  br label %exit.inner

exit.inner:
  %i.next = add i32 %i, 1
  %cmp = icmp eq i32 %i, 32
  br i1 %cmp, label %latch, label %body

latch:
  call void @llvm.directive.region.exit(token %token) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

define void @three_nested_loops(ptr %a) {
; CHECK-LABEL: Function: three_nested_loops
; CHECK:       BEGIN REGION { modified }
; CHECK-NOT:     @llvm.directive.region.entry
; CHECK-NOT:     @llvm.directive.region.exit
; CHECK:       END REGION
entry:
  br label %header

header:
  %token = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body

body:
  %i = phi i32 [ 0, %header ], [ %i.next, %exit.inner ]
  %index = getelementptr ptr, ptr %a, i32 %i
  %arr = load ptr, ptr %index
  br label %header.inner

header.inner:
  %token.inner = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body.inner

body.inner:
  %j = phi i32 [ 0, %header.inner ], [ %j.next, %exit.inner2 ]
  %index.inner = getelementptr ptr, ptr %arr, i32 %j
  %arr.inner = load ptr, ptr %index.inner
  %val = add i32 %i, %j
  br label %header.inner2

header.inner2:
  %token.inner2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body.inner2

body.inner2:
  %k = phi i32 [ 0, %header.inner2 ], [ %k.next, %body.inner2 ]

  %index.inner2 = getelementptr i32, ptr %arr.inner, i32 %k
  %val2 = add i32 %val, %k
  store i32 %val2, ptr %index.inner2

  %k.next = add i32 %k, 1
  %cmp.inner2 = icmp eq i32 %k, 127
  br i1 %cmp.inner2, label %latch.inner2, label %body.inner2

latch.inner2:
  call void @llvm.directive.region.exit(token %token.inner2) [ "DIR.OMP.END.SIMD"() ]
  br label %exit.inner2

exit.inner2:
  %j.next = add i32 %j, 1
  %cmp.inner = icmp eq i32 %j, 63
  br i1 %cmp.inner, label %latch.inner, label %body.inner

latch.inner:
  call void @llvm.directive.region.exit(token %token.inner) [ "DIR.OMP.END.SIMD"() ]
  br label %exit.inner

exit.inner:
  %i.next = add i32 %i, 1
  %cmp = icmp eq i32 %i, 32
  br i1 %cmp, label %latch, label %body

latch:
  call void @llvm.directive.region.exit(token %token) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
