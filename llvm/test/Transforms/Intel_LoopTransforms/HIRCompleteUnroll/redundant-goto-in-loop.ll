; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that last conditional GOTO in the i2 loop will not be removed by redundant node removal after the post vec complete unroll.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 1, 1   <DO_LOOP>
;       |   |   (@A)[0][i2] = i2;
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
;       |   |   if (%x >=u 100)
;       |   |   {
;       |   |      goto out_latch;
;       |   |   }
;       |   |   (@A)[0][i1 + i2] = i2;
;       |   |   if (%x != 4)
;       |   |   {
;       |   |      goto out_latch;
;       |   |   }
;       |   + END LOOP
;       |
;       |   out_latch:
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: DO i1

; CHECK-NOT: DO i2
; CHECK: (@A)[0][0] = 0;
; CHECK: (@A)[0][1] = 1;
; CHECK: DO i2
; CHECK: goto out_latch;
; CHECK: goto out_latch;
; CHECK: END LOOP

; CHECK: out_latch:
; CHECK: END LOOP
; CHECK: END REGION

@A = global [100 x i64] zeroinitializer

define void @foo(i64 %x) {
entry:
  %c1 = icmp ult i64 %x, 100
  %c2 = icmp eq i64 %x, 4
  br label %out_header

out_header:
  br label %out_body

out_body:
  %jv = phi i64 [0, %out_header], [%jvp, %out_latch]
  br label %unroll_header

unroll_header:
  br label %unroll_body

unroll_body:
  %i = phi i64 [0, %unroll_header], [%ip, %unroll_body]
  %up = getelementptr [100 x i64], ptr @A, i64 0, i64 %i
  store i64 %i, ptr %up
  %ip = add nsw i64 %i, 1
  %ucmp = icmp ult i64 %i, 1
  br i1 %ucmp, label %unroll_body, label %header

header:
  br label %body

body:
  %iv = phi i64 [0, %header], [%ivp, %body2]
  br i1 %c1, label %body1, label %out_latch

body1:
  %add = add i64 %iv, %jv
  %p = getelementptr [100 x i64], ptr @A, i64 0, i64 %add
  store i64 %iv, ptr %p
  br i1 %c2, label %body2, label %out_latch

body2:
  %ivp = add nsw i64 %iv, 1
  %cmp = icmp ult i64 %ivp, 100
  br i1 %cmp, label %body, label %out_latch

out_latch:
  %jvp = add nsw i64 %jv, 1
  %jcmp = icmp ult i64 %jvp, 100
  br i1 %jcmp, label %out_body, label %exit

exit:
  ret void
}
