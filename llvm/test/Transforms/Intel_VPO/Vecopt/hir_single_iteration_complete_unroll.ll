; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s

; Formerly we wouldn't unroll a loop with "many" instructions, even if there
; was provably only a single iteration after vectorization.  Now we do so,
; but only when there is an outer loop.  See CMPLRLLVM-40696 for a case
; where completely unrolling an outermost vectorized loop gives poor results.

; Prior to vectorization, this region looks as follows.  We still don't
; completely unroll this case after vectorization because there is no
; outer loop.
;
;          BEGIN REGION { }
;                %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(4) ]
;
;                + DO i1 = 0, 3, 1   <DO_LOOP> <simd>
;                |   @baz(1);
;                |   @baz(2);
;                |   @baz(3);
;                |   @baz(4);
;                |   @baz(5);
;                |   @baz(6);
;                |   @baz(7);
;                |   @baz(8);
;                |   @baz(9);
;                |   @baz(10);
;                |   @baz(11);
;                + END LOOP
;
;                @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
;                ret ;
;          END REGION
;
; CHECK-FUNCTION: foo
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:    DO i1 = 0, 1, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:        @baz(1);
; CHECK-NEXT:        @baz(1);
;
; CHECK:             @baz(11);
; CHECK-NEXT:        @baz(11);
; CHECK-NEXT:    END LOOP
; CHECK:       ret
; CHECK-NEXT:  END REGION


define void @foo() {
DIR.OMP.SIMD.1:
  br label %for.ph

for.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %iv.next, %for.body ]
  call void (i32) @baz(i32 1)
  call void (i32) @baz(i32 2)
  call void (i32) @baz(i32 3)
  call void (i32) @baz(i32 4)
  call void (i32) @baz(i32 5)
  call void (i32) @baz(i32 6)
  call void (i32) @baz(i32 7)
  call void (i32) @baz(i32 8)
  call void (i32) @baz(i32 9)
  call void (i32) @baz(i32 10)
  call void (i32) @baz(i32 11)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %iv.next, 2
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; The following case is similar, but there is an enclosing loop with a
; trip count of 8192.  Now we can completely unroll the inner loop.

; CHECK-FUNCTION: bar
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:   DO i1 = 0, 8191, 1   <DO_LOOP>
; CHECK-NEXT:     @baz(1);
; CHECK-NEXT:     @baz(1);

; CHECK:          @baz(11);
; CHECK-NEXT:     @baz(11);
; CHECK-NEXT:   END LOOP
; CHECK-NEXT: END REGION


define void @bar() {
entry:
  br label %outerfor.ph

outerfor.ph:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %outeriv = phi i64 [ 0, %outerfor.ph ], [ %outeriv.next, %outerfor.body ]
  br label %for.ph

for.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %iv.next, %for.body ]
  call void (i32) @baz(i32 1)
  call void (i32) @baz(i32 2)
  call void (i32) @baz(i32 3)
  call void (i32) @baz(i32 4)
  call void (i32) @baz(i32 5)
  call void (i32) @baz(i32 6)
  call void (i32) @baz(i32 7)
  call void (i32) @baz(i32 8)
  call void (i32) @baz(i32 9)
  call void (i32) @baz(i32 10)
  call void (i32) @baz(i32 11)
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %iv.next, 2
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %outerfor.body

outerfor.body:
  %outeriv.next = add nuw nsw i64 %outeriv, 1
  %outercond.not = icmp eq i64 %outeriv.next, 8192
  br i1 %outercond.not, label %exit, label %DIR.OMP.SIMD.1

exit:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @baz(i32) nounwind
