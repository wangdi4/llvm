; RUN: opt < %s -vec-clone -vpo-cfg-restructuring -avr-generate -analyze | FileCheck %s

;
; Check the correctness of generated Abstract Layer for masked function
;

;CHECK: Printing analysis 'AVR Generate' for function '_ZGVbM4v_vec_search':

;CHECK-NEXT: WRN

;CHECK:      simd.begin.region:

;CHECK:      br label %simd.loop

;CHECK:      LOOP( IV )

;CHECK:      simd.loop:
;TEMP-DO-NOT-CHECK:      br i1 %mask.cond, label %simd.loop.then, label %simd.loop.else

;CHECK:      if (%mask.cond = icmp ne i32 %mask.parm, 0
;CHECK-NEXT: )

;CHECK:      simd.loop.then:
;TEMP-DO-NOT-CHECK:      br i1 %cmp2, label %for.body.lr.ph, label %for.end

;CHECK:      if (%cmp2 = icmp slt i32 %0, 64
;CHECK-NEXT: )

;CHECK:      for.body.lr.ph:
;CHECK-NEXT: br label %for.body

;CHECK-NEXT: for.body:
;TEMP-DO-NOT-CHECK:      br i1 %cmp1, label %if.then, label %if.end

;CHECK:      if (%cmp1 = icmp eq i32 %2, %3
;CHECK-NEXT: )

;CHECK:      if.then:
;CHECK:      br label %return


;CHECK:      else

;CHECK:      if.end:
;CHECK-NEXT: br label %for.inc

;CHECK:      for.inc:
;CHECK:      %cmp = icmp slt i32 %6, 64
;CHECK-NEXT: br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

;CHECK:      for.cond.for.end_crit_edge:
;CHECK-NEXT: br label %for.end

;CHECK:      else

;CHECK:      for.end:
;CHECK-NEXT: %ret.cast.gep{{[0-9]+}} = %ret.cast getelementptr %index
;CHECK:      br label %return

;CHECK:      return:
;CHECK-NEXT: br label %simd.loop.exit

;CHECK:      else

;CHECK:      simd.loop.else:
;CHECK-NEXT: br label %simd.loop.exit

;CHECK:      simd.loop.exit:
;CHECK:      br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop

;CHECK:      simd.end.region:

;CHECK:      br label %return
;CHECK-NEXT: return{{[0-9]+}}:

;CHECK:      ret <4 x i32> %vec.ret

; ModuleID = 'test.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g1 = common global [64 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @vec_search(i32 %x) #0 {
entry:
  %retval = alloca i32, align 4
  %x.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  store i32 0, i32* %i, align 4
  %0 = load i32, i32* %i, align 4
  %cmp2 = icmp slt i32 %0, 64
  br i1 %cmp2, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [64 x i32], [64 x i32]* @g1, i32 0, i64 %idxprom
  %2 = load i32, i32* %arrayidx, align 4
  %3 = load i32, i32* %x.addr, align 4
  %cmp1 = icmp eq i32 %2, %3
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  store i32 %4, i32* %retval
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %5 = load i32, i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  %6 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %6, 64
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  store i32 -1, i32* %retval
  br label %return

return:                                           ; preds = %for.end, %if.then
  %7 = load i32, i32* %retval
  ret i32 %7
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4v_,_ZGVbN4v_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32)* @vec_search, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"x"}
!3 = !{!"arg_step", i32 undef}
!4 = !{!"arg_alig", i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.7.0 (branches/vpo 1169)"}
