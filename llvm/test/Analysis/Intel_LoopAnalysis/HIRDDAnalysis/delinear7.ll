; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that tryDelinearize() function can correctly handle MemRef @c[2][0] when it represents out-of-bound global array access.

; <0>          BEGIN REGION { }
; <25>               + DO i1 = 0, -1 * undef + 3, 1   <DO_LOOP>
; <26>               |   + DO i2 = 0, 63, 1   <DO_LOOP>
; <5>                |   |   if (undef #UNDEF# undef)
; <5>                |   |   {
; <9>                |   |      (@c)[2][0] = undef;
; <5>                |   |   }
; <26>               |   + END LOOP
; <25>               + END LOOP
; <0>          END REGION

; CHECK: (@c)[2][0] --> (@c)[2][0] OUTPUT (* *) (? ?)

target triple = "x86_64-unknown-linux-gnu"

@c = external dso_local local_unnamed_addr global [1 x i32], align 4

define dso_local void @main() local_unnamed_addr {
entry:
  br label %for.body3

for.body3:                                        ; preds = %for.end, %entry
  %d.060 = phi i32 [ undef, %entry ], [ %inc29, %for.end ]
  br label %for.body6

for.body6:                                        ; preds = %if.end24, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end24 ], [ 0, %for.body3 ]
  br i1 undef, label %if.else, label %if.end24

if.else:                                          ; preds = %for.body6
  store i32 undef, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @c, i64 2, i64 0), align 4
  br label %if.end24

if.end24:                                         ; preds = %if.else, %for.body6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.end, label %for.body6

for.end:                                          ; preds = %if.end24
  %inc29 = add nuw nsw i32 %d.060, 1
  %exitcond65.not = icmp eq i32 %inc29, 4
  br i1 %exitcond65.not, label %for.inc31, label %for.body3

for.inc31:                                        ; preds = %for.end
  ret void
}

