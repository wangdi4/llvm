; INTEL
; RUN: opt < %s -analyze -delinearize

; Try to delinearize %p[*][m][n] array when %m, %n are i32 and require signed extensions.

; + DO i1 = 0, sext.i32.i64(%m) + -1
; |   + DO i2 = 0, sext.i32.i64(%n) + -1
; |   |   %p[sext.i32.i64((%m * %n)) * i1 + sext.i32.i64(%n) * i2] = 1;
; |   |   // OR
; |   |   %p[i1][i2][0] = 1;
; |   + END LOOP
; + END LOOP

; CHECK: In Loop with Header: body_n
; CHECK: AccessFunction: {{0,+,(8 * (sext i32 (%m * %n) to i64))<nsw>}<%body_m>,+,(8 * (sext i32 %n to i64))<nsw>}<%body_n>
; CHECK: Base offset: %p
; CHECK: ArrayDecl[UnknownSize][(sext i32 %m to i64)][(sext i32 %n to i64)] with elements of 8 bytes.
; CHECK: ArrayRef[{0,+,1}<nuw><nsw><%body_m>][{0,+,1}<nuw><nsw><%body_n>][0]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define void @foo(i64* %p, i32 %m, i32 %n) {
entry:
  %sn = sext i32 %n to i64
  %sm = sext i32 %m to i64
  %mn = mul i32 %m, %n
  %smn = sext i32 %mn to i64
  br label %body_m

body_m:
  %i = phi i64 [0, %entry], [%inc_i, %body_m_inc]
  br label %body_n

body_n:
  %j = phi i64 [0, %body_m], [%inc_j, %body_n]

  %idx_i = mul i64 %smn, %i
  %idx_j = mul i64 %sn, %j
  %idx = add i64 %idx_i, %idx_j

  %q = getelementptr i64, i64* %p, i64 %idx
  store i64 1, i64* %q

  %inc_j = add nsw nuw i64 %j, 1
  %cmp_n = icmp ne i64 %inc_j, %sn
  br i1 %cmp_n, label %body_n, label %body_m_inc

body_m_inc:
  %inc_i = add nsw nuw i64 %i, 1
  %cmp_m = icmp ne i64 %inc_i, %sm
  br i1 %cmp_m, label %body_m, label %exit

exit:
  ret void
}

