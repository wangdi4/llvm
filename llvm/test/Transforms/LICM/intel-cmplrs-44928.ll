; RUN: opt -licm -disable-output < %s

%struct.baz = type { i32, i32, i32, i8* }

define void @bar() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb1, %bb
  %tmp = getelementptr inbounds %struct.baz, %struct.baz* undef, <8 x i64> undef, i32 3
  %tmp2 = load <24 x i8*>, <24 x i8*>* undef, align 8
  call void @llvm.masked.scatter.v8p0i8(<8 x i8*> undef, <8 x i8**> %tmp, i32 8, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  br label %bb1
}

; Function Attrs: nounwind
declare void @llvm.masked.scatter.v8p0i8(<8 x i8*>, <8 x i8**>, i32, <8 x i1>) #0

attributes #0 = { nounwind }
