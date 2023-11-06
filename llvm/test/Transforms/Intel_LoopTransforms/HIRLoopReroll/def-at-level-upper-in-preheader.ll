; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll" -print-before=hir-loop-reroll -print-after=hir-loop-reroll -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Verify that the upper of i3 loop which has DefAtLevel of 2 is converted to
; non-linear when we create an explicit instruction for it in the loop preheader
; after loop reroll.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %t228 = (%t2)[i1];
; CHECK: |
; CHECK: |      %t233 = %t224;
; CHECK: |      %t235 = 0;
; CHECK: |   + DO i32 i2 = 0, %t228 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   %t233.out = %t233;
; CHECK: |   |
; CHECK: |   |   + DO i32 i3 = 0, -1 * i2 + smax((1 + %t235), %t228) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   | <RVAL-REG> LINEAR i32 -1 * i2 + smax((1 + %t235), %t228) + -1{def@2}

; CHECK: |   |   |   (%t4)[2 * i3 + 2 * %t233] = (%t118)[2 * i3 + 2 * %t226];
; CHECK: |   |   |   (%t4)[2 * i3 + 2 * %t233 + 1] = (%t118)[2 * i3 + 2 * %t226 + 1];
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   %t226 = %t226  +  -1 * i2 + %t225;
; CHECK: |   |   %t262 = i2  +  1;
; CHECK: |   |   %t233 = -1 * i2 + %t233 + smax(%t262, %t228);
; CHECK: |   |   %t235 = i2 + 1;
; CHECK: |   + END LOOP
; CHECK: |      %t225 = %t225  -  %t228;
; CHECK: |      %t224 = -1 * %t228 + %t233.out + smax(%t262, %t228) + 1;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i64 i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %t228 = (%t2)[i1];
; CHECK: |
; CHECK: |      %t233 = %t224;
; CHECK: |      %t235 = 0;
; CHECK: |   + DO i32 i2 = 0, %t228 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   %t233.out = %t233;
; CHECK: |   |
; CHECK: |   |      %sext = sext.i32.i64(-2 * i2 + 2 * smax((1 + %t235), %t228) + -1);
; CHECK: |   |      <RVAL-REG> NON-LINEAR i32 -2 * i2 + 2 * smax((1 + %t235), %t228) + -1
; CHECK: |   |   + DO i64 i3 = 0, %sext, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>  <LEGAL_MAX_TC = 4294967294>
; CHECK: |   |   |   (%t4)[i3 + 2 * %t233] = (%t118)[i3 + 2 * %t226];
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   %t226 = %t226  +  -1 * i2 + %t225;
; CHECK: |   |   %t262 = i2  +  1;
; CHECK: |   |   %t233 = -1 * i2 + %t233 + smax(%t262, %t228);
; CHECK: |   |   %t235 = i2 + 1;
; CHECK: |   + END LOOP
; CHECK: |      %t225 = %t225  -  %t228;
; CHECK: |      %t224 = -1 * %t228 + %t233.out + smax(%t262, %t228) + 1;
; CHECK: + END LOOP


define void @foo(ptr %t2, ptr %t4, ptr %t118, i32 %t24, i64 %n) {
entry:
  br label %loop.outer

loop.outer:                                              ; preds = %loop.outer.latch, %entry
  %t223 = phi i64 [ 0, %entry ], [ %t272, %loop.outer.latch ]
  %t224 = phi i32 [ 0, %entry ], [ %t271, %loop.outer.latch ]
  %t225 = phi i32 [ %t24, %entry ], [ %t270, %loop.outer.latch ]
  %t226 = phi i32 [ 0, %entry ], [ %t269, %loop.outer.latch ]
  %t227 = getelementptr inbounds i32, ptr %t2, i64 %t223
  %t228 = load i32, ptr %t227, align 4
  %t229 = icmp sgt i32 %t228, 0
  br i1 %t229, label %t230, label %loop.outer.latch

t230:                                              ; preds = %loop.outer
  br label %loop.mid

loop.mid:                                              ; preds = %loop.mid.latch, %t230
  %t232 = phi i32 [ %t260, %loop.mid.latch ], [ %t226, %t230 ]
  %t233 = phi i32 [ %t259, %loop.mid.latch ], [ %t224, %t230 ]
  %t234 = phi i32 [ %t261, %loop.mid.latch ], [ %t225, %t230 ]
  %t235 = phi i32 [ %t262, %loop.mid.latch ], [ 0, %t230 ]
  br label %loop.inner

loop.inner:                                              ; preds = %loop.inner, %loop.mid
  %t237 = phi i32 [ %t233, %loop.mid ], [ %t255, %loop.inner ]
  %t238 = phi i32 [ %t232, %loop.mid ], [ %t256, %loop.inner ]
  %t239 = phi i32 [ %t235, %loop.mid ], [ %t254, %loop.inner ]
  %t240 = shl nsw i32 %t238, 1
  %t241 = sext i32 %t240 to i64
  %t242 = getelementptr inbounds double, ptr %t118, i64 %t241
  %t243 = load double, ptr %t242, align 8
  %t244 = shl nsw i32 %t237, 1
  %t245 = sext i32 %t244 to i64
  %t246 = getelementptr inbounds double, ptr %t4, i64 %t245
  store double %t243, ptr %t246, align 8
  %t247 = add nuw nsw i32 %t240, 1
  %t248 = sext i32 %t247 to i64
  %t249 = getelementptr inbounds double, ptr %t118, i64 %t248
  %t250 = load double, ptr %t249, align 8
  %t251 = add nuw nsw i32 %t244, 1
  %t252 = sext i32 %t251 to i64
  %t253 = getelementptr inbounds double, ptr %t4, i64 %t252
  store double %t250, ptr %t253, align 8
  %t254 = add nuw nsw i32 %t239, 1
  %t255 = add i32 %t237, 1
  %t256 = add nsw i32 %t238, 1
  %t257 = icmp slt i32 %t254, %t228
  br i1 %t257, label %loop.inner, label %loop.mid.latch

loop.mid.latch:                                              ; preds = %loop.inner
  %t259 = phi i32 [ %t255, %loop.inner ]
  %t260 = add i32 %t232, %t234
  %t261 = add nsw i32 %t234, -1
  %t262 = add nuw nsw i32 %t235, 1
  %t263 = icmp eq i32 %t262, %t228
  br i1 %t263, label %t264, label %loop.mid

t264:                                              ; preds = %loop.mid.latch
  %t265 = phi i32 [ %t260, %loop.mid.latch ]
  %t266 = phi i32 [ %t259, %loop.mid.latch ]
  %t267 = sub i32 %t225, %t228
  br label %loop.outer.latch

loop.outer.latch:                                              ; preds = %t264, %loop.outer
  %t269 = phi i32 [ %t226, %loop.outer ], [ %t265, %t264 ]
  %t270 = phi i32 [ %t225, %loop.outer ], [ %t267, %t264 ]
  %t271 = phi i32 [ %t224, %loop.outer ], [ %t266, %t264 ]
  %t272 = add nuw nsw i64 %t223, 1
  %t273 = icmp eq i64 %t272, %n
  br i1 %t273, label %exit, label %loop.outer

exit:
  ret void
}
