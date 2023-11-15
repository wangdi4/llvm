; Test to verify that an early exit loop with non-unique exit
; blocks is vectorized correctly.

; Input HIR:
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;       + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %1 = (%a)[i1];
;       |   if (%1 == %val)
;       |   {
;       |      goto side.exit;
;       |   }
;       + END LOOP
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-enable-early-exit-loops -vplan-force-vf=4 -disable-output 2>&1 | FileCheck %s

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       %tgu = sext.i32.i64(%n)  /u  4;
; CHECK-NEXT:       %vec.tc = %tgu  *  4;
; CHECK-NEXT:       %.vec = 0 == %vec.tc;
; CHECK-NEXT:       %phi.temp = 0;
; CHECK-NEXT:       %extract.0. = extractelement %.vec,  0;
; CHECK-NEXT:       if (%extract.0. == 1)
; CHECK-NEXT:       {
; CHECK-NEXT:          goto merge.blk16.26;
; CHECK-NEXT:       }
; CHECK-NEXT:       %tgu1 = sext.i32.i64(%n)  /u  4;
; CHECK-NEXT:       %vec.tc2 = %tgu1  *  4;
; CHECK-NEXT:       %loop.ub = %vec.tc2  -  1;

; CHECK:            + DO i1 = 0, %loop.ub, 4   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 536870911>  <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:       |   %.vec3 = (<4 x i8>*)(%a)[i1];
; CHECK-NEXT:       |   %.vec4 = %.vec3 == %val;
; CHECK-NEXT:       |   %ee.execmask.ctz.intmask = bitcast.<4 x i1>.i4(%.vec4);
; CHECK-NEXT:       |   %ee.execmask.ctz. = @llvm.cttz.i4(%ee.execmask.ctz.intmask,  0);
; CHECK-NEXT:       |   %ee.execmask.zext = zext.i4.i5(%ee.execmask.ctz.);
; CHECK-NEXT:       |   %ee.execmask.shl = 1  <<  %ee.execmask.zext;
; CHECK-NEXT:       |   %ee.execmask.int = %ee.execmask.shl  -  1;
; CHECK-NEXT:       |   %ee.execmask.int.trunc = trunc.i5.i4(%ee.execmask.int);
; CHECK-NEXT:       |   %ee.execmask = bitcast.i4.<4 x i1>(%ee.execmask.int.trunc);
; CHECK-NEXT:       |   %.vec5 = %ee.execmask  ^  -1;
; CHECK-NEXT:       |   %.vec6 = i1 + <i64 0, i64 1, i64 2, i64 3> + 4 < %vec.tc2;
; CHECK-NEXT:       |   %select = (%ee.execmask == <i1 true, i1 true, i1 true, i1 true>) ? i1 + <i64 0, i64 1, i64 2, i64 3> + 4 : i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:       |   %select7 = (%ee.execmask == <i1 true, i1 true, i1 true, i1 true>) ? 0 : 1;
; CHECK-NEXT:       |   %select8 = (%ee.execmask == <i1 true, i1 true, i1 true, i1 true>) ? %.vec6 : 0;
; CHECK-NEXT:       |   %.vec9 = %select8  ^  -1;
; CHECK-NEXT:       |   %0 = bitcast.<4 x i1>.i4(%.vec9);
; CHECK-NEXT:       |   %cmp10 = %0 == 0;
; CHECK-NEXT:       |   %all.zero.check = %cmp10;
; CHECK-NEXT:       |   if (%cmp10 != 1)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      goto cascaded.if.block11.61;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP

; CHECK:            cascaded.if.block11.61:
; CHECK-NEXT:       %.vec11 = %select7 != 0;
; CHECK-NEXT:       %ee.lane.intmask = bitcast.<4 x i1>.i4(%.vec11);
; CHECK-NEXT:       %ee.lane. = @llvm.cttz.i4(%ee.lane.intmask,  0);
; CHECK-NEXT:       %select12 = (%ee.lane. == 4) ? -1 : %ee.lane.;
; CHECK-NEXT:       %ee.lane.sext = sext.i4.i32(%select12);
; CHECK-NEXT:       %ee.or.first.lane.sel = (%ee.lane.sext != -1) ? %ee.lane.sext : 0;
; CHECK-NEXT:       %ee.or.last.lane.sel = (%ee.lane.sext != -1) ? %ee.lane.sext : 3;
; CHECK-NEXT:       %extracted.lval = extractelement %select,  %ee.or.first.lane.sel;
; CHECK-NEXT:       %ee.id.final = extractelement %select7,  %ee.or.last.lane.sel;
; CHECK-NEXT:       %.vec13 = %ee.id.final == 1;
; CHECK-NEXT:       %extract.0.14 = extractelement %.vec13,  0;
; CHECK-NEXT:       if (%extract.0.14 == 1)
; CHECK-NEXT:       {
; CHECK-NEXT:          goto BB5.75;
; CHECK-NEXT:       }
; CHECK-NEXT:       goto BB3.79;
; CHECK-NEXT:       BB5.75:
; CHECK-NEXT:       goto side.exit;
; CHECK-NEXT:       BB3.79:
; CHECK-NEXT:       %.vec15 = sext.i32.i64(%n) == %vec.tc2;
; CHECK-NEXT:       %phi.temp = %extracted.lval;
; CHECK-NEXT:       %phi.temp17 = %extracted.lval;
; CHECK-NEXT:       %extract.0.19 = extractelement %.vec15,  0;
; CHECK-NEXT:       if (%extract.0.19 == 1)
; CHECK-NEXT:       {
; CHECK-NEXT:          goto final.merge.91;
; CHECK-NEXT:       }
; CHECK-NEXT:       merge.blk16.26:
; CHECK-NEXT:       %lb.tmp = %phi.temp;

; CHECK:            + DO i1 = %lb.tmp, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:       |   %2 = (%a)[i1];
; CHECK-NEXT:       |   if (%2 == %val)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      goto side.exit;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP

; CHECK:            %phi.temp17 = sext.i32.i64(%n) + -1;
; CHECK-NEXT:       final.merge.91:
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z3fooiPKaPaa(i32 %n, ptr nocapture readonly %a, i8 signext %val) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  %1 = load i8, ptr %arrayidx, align 1
  %cmp2 = icmp eq i8 %1, %val
  br i1 %cmp2, label %side.exit, label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit

side.exit:                                        ; preds = %for.body
  br label %cleanup

cleanup.loopexit:                                 ; preds = %for.inc
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %side.exit, %entry
  ret i32 0
}
