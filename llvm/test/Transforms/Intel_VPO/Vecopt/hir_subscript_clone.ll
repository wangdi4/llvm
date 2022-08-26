; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -vplan-force-vf=4 -vplan-force-uf=2 -vplan-print-after-unroll -disable-output | FileCheck %s

%S = type { i64, i64 }
define void @_Z3fooPii(%S *%p) local_unnamed_addr {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]

; Verify that StructOffsets are copied by the unroller. (1x load + 1x store) x UF(2)
; CHECK-COUNT-4: subscript %S* %p i64 %{{.*}} (1 )
  %gep = getelementptr %S, %S *%p, i64 %iv, i32 1
  %ld = load i64, i64 *%gep
  %add = add i64 %ld, 42
  store i64 %add, i64 *%gep

  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 64
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
