; Verify that we have correct types for 'zext' instruction and the subsequent operations
; Input LLVM IR is generated for below code with command:  icx -O2 -mllvm -print-module-before-loopopt

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-plain-cfg -disable-output < %s 2>&1 | FileCheck %s

; Source code
; #define N 1600
;
; int a[N];
; int b[N];
; int c[N];
;
; void foo(int factor) {
;
;   int i;
;   int max = N < factor ? factor : N;
; #pragma omp simd
;   for (i=0; i<max; i++){
;     c[i] = a[i] + b[i];
;   }
; }




;======================= HIR-dump=====================================================
;<16>         + DO i1 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1600>
;<3>          |   %2 = (@a)[0][i1];
;<5>          |   %3 = (@b)[0][i1];
;<8>          |   (@c)[0][i1] = %2 + %3;
;<16>         + END LOOP
;=====================================================================================


;=======================VPlan-dump after-simplify-CFG=================================
;  REGION: region1
;  BB2:
;   <Empty Block>
;  SUCCESSORS(1):BB3
;
;  BB3:
;   i64 %vp1680 = zext i32 %vp1616
;   i64 %vp1904 = add i64 %vp1680 i64 -1
;  SUCCESSORS(1):BB4
;
;  BB4:
;   i64 %vp64720 = phi [ i64 0, BB3 ], [ i64 %vp1344, BB7 ]
;   i32 %vp64944 = load i32 %vp64880
;   i32 %vp672 = load i32 %vp608
;   i32 %vp928 = add i32 %vp64944 i32 %vp672
;   store i32 %vp928 i32 %vp864
;   i64 %vp1344 = add i64 %vp64720 i64 1
;   i1 %vp2064 = icmp i64 %vp1344 i64 %vp1904
;  SUCCESSORS(1):BB7
;
;  BB7:
;   <Empty Block>
;   Condition(BB4): i1 %vp2064 = icmp i64 %vp1344 i64 %vp1904
;  SUCCESSORS(2):BB4(i1 %vp2064), BB5(!i1 %vp2064)
;
;  BB5:
;   <Empty Block>
;  SUCCESSORS(1):BB6
;
;  BB6:
;   <Empty Block>
;  END Block - no SUCCESSORS
;  END Region(region1)
;=====================================================================================



; CHECK: i64 [[Zext:%vp.*]] = zext i32 {{.*}}
; CHECK-NEXT: [[Add:%.*]] = add i64 [[Zext]] i64 -1
; CHECK-NEXT: SUCCESSORS(1):[[H:BB.*]]
; CHECK: [[H]]:
; CHECK-NEXT: [[IVPhi:%.*]] = phi

target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

define void @foo(i32 %factor) local_unnamed_addr {

entry:
  %cmp = icmp sgt i32 %factor, 1600
  %0 = select i1 %cmp, i32 %factor, i32 1600
  %cmp4 = icmp sgt i32 %0, 0
  br i1 %cmp4, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %1 = zext i32 %0 to i64
  %iv.in = bitcast i64 0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.precond.then, %omp.inner.for.body
  %iv = phi i64 [ 0, %omp.precond.then ], [ %iv.next, %omp.inner.for.body ]
  %idx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %iv
  %2 = load i32, i32* %idx
  %idx9 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %iv
  %3 = load i32, i32* %idx9
  %add10 = add nsw i32 %3, %2
  %idx12 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %iv
  store i32 %add10, i32* %idx12
  %iv.next = add nuw nsw i64 %iv, 1
  %cmp6 = icmp ult i64 %iv.next, %1
  %iv.in27 = bitcast i64 %iv.next to i64
  br i1 %cmp6, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}
