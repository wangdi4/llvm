; RUN: opt -passes="print<hir-scc-formation>,hir-ssa-deconstruction" -print-after=hir-ssa-deconstruction -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not form SCC (%scc.add -> %scc.phi) for the region formed
; for simd inner.loop. outer.loop is an unknown loop so we throttle it.

; Outer loop's header/latch is included in the region because it contains
; region entry/exit directive but they are split off by deconstruction pass.
; The SCCs are constructed before SSA deconstruction so we need to bail out
; in SCC formation pass.

; CHECK-NOT: SCC1

; CHECK: %scc.phi =
; CHECK: br label %outer.loop.split


define void @foo(i32 %init) {
entry:
  %priv = alloca [1024 x i32], align 4
  br label %outer.loop

outer.loop:
  %outer.iv = phi i32 [ 0, %entry ], [ %outer.iv.next, %outer.latch ]
  %scc.phi = phi i32 [ %init, %entry ], [ %scc.add, %outer.latch ]
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.PRIVATE"(ptr %priv) ]
  br label %inner.loop

inner.loop:
  %indvars.iv = phi i64 [ 0, %outer.loop ], [ %indvars.iv.next, %inner.loop ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr %priv, i64 0, i64 %indvars.iv
  store i32 %scc.phi, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %inner.loop, label %omp.loop.exit

omp.loop.exit:
  br label %outer.latch

outer.latch:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  %gep = getelementptr inbounds [1024 x i32], ptr %priv, i64 0, i64 4
  %ld = load i32, ptr %gep
  %scc.add = add i32 %scc.phi, %ld
  %outer.iv.next = add nuw nsw i32 %outer.iv, 1
  %exitcond1 = icmp ne i32 %outer.iv.next, %ld
  br i1 %exitcond1, label %outer.loop, label %exit

exit:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
