; RUN: opt -S -vec-clone %s | FileCheck %s
; RUN: opt -S -passes=vec-clone %s | FileCheck %s

; Checks that the gep/store of expanded return values are placed after all phi
; nodes inside a basic block.

declare i32 @foo(i32)

define i32 @bar(i32 %N) #0 {
; CHECK-LABEL: define <16 x i32> @_ZGVeN16v_bar
; CHECK-NEXT: entry:
; CHECK:       %vec.retval = alloca <16 x i32>, align 64
; CHECK:       [[RET_CAST:%.*]] = bitcast <16 x i32>* %vec.retval to i32*
; CHECK:      for:
; CHECK-NEXT:  [[TOTAL:%.*]] = phi i32 [ 0, %simd.loop.header ], [ [[ADD:%.*]], %for ]
; CHECK-NEXT:  [[I:%.*]] = phi i32 [ 0, %simd.loop.header ], [ [[INC:%.*]], %for ]
; CHECK-NEXT:  [[RET_CAST_GEP:%.*]] = getelementptr i32, i32* [[RET_CAST]], i32
; CHECK-NEXT:  store i32 [[TOTAL]], i32* [[RET_CAST_GEP]], align 4
; CHECK-NEXT:  [[CALL:%.*]] = call i32 @foo(i32 [[I]])
; CHECK-NEXT:  [[ADD]] = add i32 [[TOTAL]], [[CALL]]
; CHECK-NEXT:  [[INC]] = add i32 [[I]], 1
entry:
  br label %for

for:
  %total = phi i32 [ 0, %entry ], [ %add, %for ]
  %i = phi i32 [ 0, %entry ], [ %inc, %for ]
  %call = call i32 @foo(i32 %i)
  %add = add i32 %total, %call
  %inc = add i32 %i, 1
  %cond = icmp eq i32 %i, %N
  br i1 %cond, label %exit, label %for

exit:
  ret i32 %total
}

attributes #0 = { nounwind "vector-variants"="_ZGVeN16v_bar" }
