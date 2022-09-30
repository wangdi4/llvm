; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s

; Formerly we wouldn't unroll a loop with "many" instructions, even if there
; was provably only a single iteration after vectorization.

; Prior to vectorization, this region looks as follows.  We can collapse
; (fully unroll) the loop after vectorization.
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
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        @baz(1);
; CHECK-NEXT:        @baz(1);
;
; CHECK:             @baz(11);
; CHECK-NEXT:        @baz(11);
; CHECK-NEXT:        ret
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

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @baz(i32) nounwind
