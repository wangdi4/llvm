; RUN: opt < %s -simd-function-cloning -avr-generate -analyze | FileCheck %s

; Check sequence AVRs generated for given test.
;CHECK: '_ZGVxM4v_vec_search'
;CHECK: AVR_WRN
;CHECK-NEXT: AVR_LABEL:    simd.begin.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !7)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual.opnd.i32(metadata !8, i32 4)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual(metadata !9)
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    simd.loop
;CHECK-NEXT: AVR_PHI:      %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
;CHECK-NEXT: AVR_ASSIGN:   %maskgep = getelementptr i32, i32* %veccast.2, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %mask5 = load i32, i32* %maskgep
;CHECK-NEXT: AVR_IF:       
;CHECK-NEXT: AVR_FBRANCH:  br i1 %maskcond, label %simd.loop.then, label %simd.loop.else
;CHECK-NEXT: AVR_LABEL:    simd.loop.then
;CHECK-NEXT: AVR_ASSIGN:   store i32 0, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    for.cond
;CHECK-NEXT: AVR_ASSIGN:   %0 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_IF:       %cmp = icmp slt i32 %0, 64
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp, label %for.body, label %for.end
;CHECK-NEXT: AVR_LABEL:    for.body
;CHECK-NEXT: AVR_ASSIGN:   %1 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom = sext i32 %1 to i64
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx = getelementptr inbounds [64 x i32], [64 x i32]* @g1, i32 0, i64 %idxprom
;CHECK-NEXT: AVR_ASSIGN:   %2 = load i32, i32* %arrayidx, align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP1:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.1, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %3 = load i32, i32* [[VECGEP1]], align 4
;CHECK-NEXT: AVR_IF:       %cmp1 = icmp eq i32 %2, %3
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp1, label %if.then, label %if.end
;CHECK-NEXT: AVR_LABEL:    if.then
;CHECK-NEXT: AVR_ASSIGN:   %4 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP2:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 %4, i32* [[VECGEP2]]
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK-NEXT: AVR_LABEL:    if.end
;CHECK-NEXT: AVR_FBRANCH:  br label %for.inc
;CHECK-NEXT: AVR_LABEL:    for.inc
;CHECK-NEXT: AVR_ASSIGN:   %5 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %inc = add nsw i32 %5, 1
;CHECK-NEXT: AVR_ASSIGN:   store i32 %inc, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LABEL:    for.end
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP3:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 -1, i32* [[VECGEP3]]
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK-NEXT: AVR_LABEL:    simd.loop.exit
;CHECK-NEXT: AVR_ASSIGN:   %indvar = add nuw i32 1, %index
;CHECK-NEXT: AVR_IF:       %vlcond = icmp ult i32 %indvar, 4
;CHECK-NEXT: AVR_FBRANCH:  br i1 %vlcond, label %simd.loop, label %simd.end.region
;CHECK-NEXT: AVR_LABEL:    simd.end.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !10)
;CHECK-NEXT: AVR_FBRANCH:  br label %return
;CHECK-NEXT: AVR_LABEL:    return
;CHECK-NEXT: AVR_ASSIGN:   %cast = bitcast i32* %veccast to <4 x i32>*
;CHECK-NEXT: AVR_ASSIGN:   %vec_ret = load <4 x i32>, <4 x i32>* %cast
;CHECK-NEXT: AVR_RETURN:   ret <4 x i32> %vec_ret
;CHECK-NEXT: AVR_LABEL:    simd.loop.else
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK:'_ZGVxN4v_vec_search'
;CHECK: AVR_WRN
;CHECK-NEXT: AVR_LABEL:    simd.begin.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !7)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual.opnd.i32(metadata !8, i32 4)
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive.qual(metadata !9)
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    simd.loop
;CHECK-NEXT: AVR_PHI:      %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
;CHECK-NEXT: AVR_ASSIGN:   store i32 0, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LOOP:
;CHECK-NEXT: AVR_LABEL:    for.cond
;CHECK-NEXT: AVR_ASSIGN:   %0 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_IF:       %cmp = icmp slt i32 %0, 64
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp, label %for.body, label %for.end
;CHECK-NEXT: AVR_LABEL:    for.body
;CHECK-NEXT: AVR_ASSIGN:   %1 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %idxprom = sext i32 %1 to i64
;CHECK-NEXT: AVR_ASSIGN:   %arrayidx = getelementptr inbounds [64 x i32], [64 x i32]* @g1, i32 0, i64 %idxprom
;CHECK-NEXT: AVR_ASSIGN:   %2 = load i32, i32* %arrayidx, align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP4:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast.1, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   %3 = load i32, i32* [[VECGEP4]], align 4
;CHECK-NEXT: AVR_IF:       %cmp1 = icmp eq i32 %2, %3
;CHECK-NEXT: AVR_FBRANCH:  br i1 %cmp1, label %if.then, label %if.end
;CHECK-NEXT: AVR_LABEL:    if.then
;CHECK-NEXT: AVR_ASSIGN:   %4 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP5:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 %4, i32* [[VECGEP5]]
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit
;CHECK-NEXT: AVR_LABEL:    if.end
;CHECK-NEXT: AVR_FBRANCH:  br label %for.inc
;CHECK-NEXT: AVR_LABEL:    for.inc
;CHECK-NEXT: AVR_ASSIGN:   %5 = load i32, i32* %i, align 4
;CHECK-NEXT: AVR_ASSIGN:   %inc = add nsw i32 %5, 1
;CHECK-NEXT: AVR_ASSIGN:   store i32 %inc, i32* %i, align 4
;CHECK-NEXT: AVR_FBRANCH:  br label %for.cond
;CHECK-NEXT: AVR_LABEL:    simd.loop.exit
;CHECK-NEXT: AVR_ASSIGN:   %indvar = add nuw i32 1, %index
;CHECK-NEXT: AVR_IF:       %vlcond = icmp ult i32 %indvar, 4
;CHECK-NEXT: AVR_FBRANCH:  br i1 %vlcond, label %simd.loop, label %simd.end.region
;CHECK-NEXT: AVR_LABEL:    simd.end.region
;CHECK-NEXT: AVR_CALL:     call void @llvm.intel.directive(metadata !10)
;CHECK-NEXT: AVR_FBRANCH:  br label %return
;CHECK-NEXT: AVR_LABEL:    return
;CHECK-NEXT: AVR_ASSIGN:   %cast = bitcast i32* %veccast to <4 x i32>*
;CHECK-NEXT: AVR_ASSIGN:   %vec_ret = load <4 x i32>, <4 x i32>* %cast
;CHECK-NEXT: AVR_RETURN:   ret <4 x i32> %vec_ret
;CHECK-NEXT: AVR_LABEL:    for.end
;CHECK-NEXT: AVR_ASSIGN:   [[VECGEP6:%vecgep[0-9]*]] = getelementptr i32, i32* %veccast, i32 %index
;CHECK-NEXT: AVR_ASSIGN:   store i32 -1, i32* [[VECGEP6]]
;CHECK-NEXT: AVR_FBRANCH:  br label %simd.loop.exit


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
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 64
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
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
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 -1, i32* %retval
  br label %return

return:                                           ; preds = %for.end, %if.then
  %6 = load i32, i32* %retval
  ret i32 %6
}

attributes #0 = { nounwind uwtable "_ZGVxM4v_" "_ZGVxN4v_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!cilk.functions = !{!0}
!llvm.ident = !{!6}

!0 = !{i32 (i32)* @vec_search, !1, !2, !3, !4, !5}
!1 = !{!"elemental"}
!2 = !{!"arg_name", !"x"}
!3 = !{!"arg_step", i32 undef}
!4 = !{!"arg_alig", i32 undef}
!5 = !{!"vec_length", i32 undef, i32 4}
!6 = !{!"clang version 3.7.0 (branches/vpo 1169)"}
