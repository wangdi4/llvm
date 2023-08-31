; RUN: opt -S -passes=loop-vectorize < %s | FileCheck %s

; CHECK-LABEL: @test
define hidden void @test(ptr nocapture noundef writeonly %dest, i64 noundef %stride, i64 noundef %count) {
header:
  br label %loop

loop:                                               ; preds = %header, %loop
  %i = phi i64 [ %i.next, %loop ], [ %count, %header ]
  %base = phi ptr [ %base.next, %loop ], [ %dest, %header ]
  %i.next = add nsw i64 %i, -1
  store i8 0, ptr %base, align 1
  %base.next = getelementptr inbounds i8, ptr %base, i64 %stride
  %gt1 = icmp ugt i64 %i, 1
  br i1 %gt1, label %loop, label %exit

exit:                                               ; preds = %loop
  ret void
}

