; RUN: opt -passes="simplifycfg" < %s -S | FileCheck %s

; Verify that we suppress conversion of conditional branches comparing IVs into
; switch in the presence of "pre_loopopt" attribute but do the conversion in
; its absence.

@A = common global i32 zeroinitializer

; CHECK-LABEL: @iv-cmp-preloopopt
; CHECK-NOT: switch

; CHECK: %cmp1 =
; CHECK: br i1 %cmp1


; CHECK: %cmp2 =
; CHECK: br i1 %cmp2

define void @iv-cmp-preloopopt(i32 %t) "pre_loopopt" {
entry:
  br label %loop

loop:
  %iv.phi = phi i32 [ 0, %entry ], [ %iv.inc, %latch]
  %cmp1 = icmp eq i32 %iv.phi , 1
  br i1 %cmp1, label %then1, label %else

then1:
  store i32 2, ptr @A
  br label %latch

else:
  %cmp2 = icmp eq i32 %iv.phi, 0
  br i1 %cmp2, label %then2, label %latch

then2:
  store i32 3, ptr @A
  br label %latch

latch:
  %iv.inc = add nsw i32 %iv.phi, 1
  %cmp.exit = icmp slt i32 %iv.inc, 15
  br i1 %cmp.exit, label %loop, label %exit

exit:
  ret void
}

; CHECK-LABEL: @iv-cmp
; CHECK:   switch i32 %iv.phi, label %latch [
; CHECK:    i32 1, label %then1
; CHECK:    i32 0, label %then2

define void @iv-cmp(i32 %t) {

entry:
  br label %loop

loop:
  %iv.phi = phi i32 [ 0, %entry ], [ %iv.inc, %latch]
  %cmp1 = icmp eq i32 %iv.phi , 1
  br i1 %cmp1, label %then1, label %else

then1:
  store i32 2, ptr @A
  br label %latch

else:
  %cmp2 = icmp eq i32 %iv.phi, 0
  br i1 %cmp2, label %then2, label %latch

then2:
  store i32 3, ptr @A
  br label %latch

latch:
  %iv.inc = add nsw i32 %iv.phi, 1
  %cmp.exit = icmp slt i32 %iv.inc, 15
  br i1 %cmp.exit, label %loop, label %exit

exit:
  ret void
}
