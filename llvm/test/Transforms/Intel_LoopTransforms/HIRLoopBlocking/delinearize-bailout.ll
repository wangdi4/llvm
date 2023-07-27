; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Verify that we do not compfail in blocking while trying to delinearize
; two memrefs.


; CHECK: + DO i1 = 0, %sub28 + -1 * smin(1, %sub28), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK: |   |   (%a)[sext.i32.i64(%stride) * i1 + i2 + sext.i32.i64(((1 + (-1 * %parity)) * %stride))] = (%a)[sext.i32.i64((2 * %stride)) * i1 + i2 + sext.i32.i64(((2 + (-1 * %parity)) * %stride))];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


define void @foo(ptr %a, i32 %stride, i32 %parity, i32 %sub28) {

for.cond34.preheader.lr.ph:                       ; preds = %while.end
  %sub11 = sub nsw i32 1, %parity
  %mul = mul nsw i32 %sub11, %stride
  %idxprom = sext i32 %mul to i64
  %ptridx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  %sub22 = sub nsw i32 2, %parity
  %mul23 = mul nsw i32 %sub22, %stride
  %idxprom24 = sext i32 %mul23 to i64
  %ptridx25 = getelementptr inbounds i32, ptr %a, i64 %idxprom24
  %idx.ext43 = sext i32 %stride to i64
  %shl45 = shl i32 %stride, 1
  %idx.ext46 = sext i32 %shl45 to i64
  br label %for.cond34.preheader

for.cond34.preheader:                             ; preds = %for.end42, %for.cond34.preheader.lr.ph
  %dec30156.in = phi i32 [ %sub28, %for.cond34.preheader.lr.ph ], [ %dec30156, %for.end42 ]
  %srcptr.1155 = phi ptr [ %ptridx25, %for.cond34.preheader.lr.ph ], [ %add.ptr47, %for.end42 ]
  %dstptr.1154 = phi ptr [ %ptridx, %for.cond34.preheader.lr.ph ], [ %add.ptr44, %for.end42 ]
  br label %for.body37

for.body37:                                       ; preds = %for.body37, %for.cond34.preheader
  %i.1151 = phi i32 [ 0, %for.cond34.preheader ], [ %inc41, %for.body37 ]
  %dstptr2.1150 = phi ptr [ %dstptr.1154, %for.cond34.preheader ], [ %incdec.ptr38, %for.body37 ]
  %srcptr2.1149 = phi ptr [ %srcptr.1155, %for.cond34.preheader ], [ %incdec.ptr39, %for.body37 ]
  %t4 = load i32, ptr %srcptr2.1149, align 4
  store i32 %t4, ptr %dstptr2.1150, align 4
  %incdec.ptr38 = getelementptr inbounds i32, ptr %dstptr2.1150, i64 1
  %incdec.ptr39 = getelementptr inbounds i32, ptr %srcptr2.1149, i64 1
  %inc41 = add nuw nsw i32 %i.1151, 1
  %exitcond165.not = icmp eq i32 %inc41, 16
  br i1 %exitcond165.not, label %for.end42, label %for.body37

for.end42:
  %dec30156 = add nsw i32 %dec30156.in, -1
  %add.ptr44 = getelementptr inbounds i32, ptr %dstptr.1154, i64 %idx.ext43
  %add.ptr47 = getelementptr inbounds i32, ptr %srcptr.1155, i64 %idx.ext46
  %cmp31 = icmp sgt i32 %dec30156.in, 1
  br i1 %cmp31, label %for.cond34.preheader, label %while.end48.loopexit

while.end48.loopexit:
  ret void
}
