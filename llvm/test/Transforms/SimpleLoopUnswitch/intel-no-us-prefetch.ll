; RUN: opt -S -passes="simple-loop-unswitch<nontrivial>"  %s | FileCheck %s

; Verify that loop unswitching doesn't occur when loop has prefetch directive.
; LoopOpt framework cannot deal with the unswitched loops.

; CHECK-NOT: loop.us

define internal void @foo(i1 %cond) {
entry:
  br label %preheader

preheader:
  %dir = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"() ]
  br label %loop

loop:                          ; preds = %latch, %preheader
  %iv = phi i64 [ 0, %preheader ], [ 0, %latch ]
  br i1 %cond, label %then, label %else

then:
  br label %latch

else:
  br label %latch

latch:                           
  %phi = phi i32 [ 4, %then ], [ 5, %else ]
  store i32 %phi, i32* null, align 8
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv, 0
  br i1 %cmp, label %exit, label %loop

exit:
  call void @llvm.directive.region.exit(token none) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry() #0

declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
