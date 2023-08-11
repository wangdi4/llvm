; RUN: opt -mtriple=x86_64-pc-linux-gnu -pre-isel-intrinsic-lowering -S -o - %s | FileCheck %s

; Verify that LoopOpt related region entry/exit directives are eliminated.

; CHECK: @foo
; CHECK-NEXT: ret void

define void @foo() {
  %t1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  call void @llvm.directive.region.exit(token %t1) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %t2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"() ]
  call void @llvm.directive.region.exit(token %t2) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  %t3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"() ]
  call void @llvm.directive.region.exit(token %t3) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
