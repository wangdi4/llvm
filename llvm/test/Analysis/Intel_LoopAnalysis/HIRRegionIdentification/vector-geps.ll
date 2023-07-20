; REQUIRES: asserts
; RUN: opt < %s  -passes="print<hir-region-identification>" -debug-only=hir-region-identification 2>&1 | FileCheck %s

; Verify that we bail out on GEP %mm_vectorGEP which contains vector type operands.


; CHECK: LOOPOPT_OPTREPORT: Loop %vector.body: GEP related vector types currently not supported.


define void @foo(<8 x ptr> %broadcast.splat, <8 x i64> %broadcast.splat20, i64 %n.vec) {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %vec.ind = phi <8 x i64> [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, %entry ], [ %vec.ind.next, %vector.body ]
  %t1 = trunc <8 x i64> %vec.ind to <8 x i32>
  %t2 = sitofp <8 x i32> %t1 to <8 x float>
  %t3 = mul nuw nsw <8 x i64> %vec.ind, <i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3, i64 3>
  %mm_vectorGEP = getelementptr inbounds float, <8 x ptr> %broadcast.splat, <8 x i64> %t3
  call void @llvm.masked.scatter.v8f32.v8p0(<8 x float> %t2, <8 x ptr> %mm_vectorGEP, i32 4, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
  %t4 = add nuw nsw <8 x i64> %vec.ind, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %t5 = icmp eq <8 x i64> %t4, %broadcast.splat20
  %t6 = extractelement <8 x i1> %t5, i32 0
  %index.next = add i64 %index, 8
  %t7 = icmp eq i64 %index.next, %n.vec
  %vec.ind.next = add <8 x i64> %vec.ind, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br i1 %t7, label %exit, label %vector.body


exit:
  ret void
}

declare void @llvm.masked.scatter.v8f32.v8p0(<8 x float>, <8 x ptr>, i32 immarg, <8 x i1>)
