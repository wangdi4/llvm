;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output  < %s 2>&1 | FileCheck %s
;
; LIT test to ensure that reduction initialization/finalization instructions do
; not get hoisted out incorrectly when we generate a peel loop.
;
; Incoming HIR:
;
;                + DO i1 = 0, 6, 1   <DO_LOOP>
;                |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ] 
;                |   
;                |   + DO i2 = 0, 6, 1   <DO_LOOP>
;                |   |   %4 = (%3)[i2 + 1];
;                |   |   %cmp4 = i1 == %4;
;                |   |   %k.022 = %k.022  +  %cmp4; <Safe Reduction>
;                |   + END LOOP
;                |   
;                |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ] 
;                + END LOOP
;
; Default implementation peels 3 iterations to align the load on a 16-byte boundary. One
; vector iteration runs the remaining iterations and we do not generate a remainder loop.
; when trying to see if we can hoist reduction initialization/finalization related
; instructions we were only checking to see that a remainder is not needed. We also need
; to check that a peel loop is not emitted.
;
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:      + DO i1 = 0, 6, 1   <DO_LOOP>
; CHECK-NEXT:      |   %ub.tmp = 3;
; CHECK-NEXT:      |   %peel.ub = %ub.tmp  -  1;
;
; CHECK:           |   + DO i2 = 0, %peel.ub, 1   <DO_LOOP>
; CHECK:           |   + END LOOP                  
;
; CHECK:           |   %red.init = 0;
; CHECK-NEXT:      |   %red.init.insert = insertelement %red.init,  %k.022,  0;
; CHECK:           |   %phi.temp = %red.init.insert;
;
; CHECK:           |   + DO i2 = %ub.tmp, %loop.ub, 4   <DO_LOOP>
; CHECK:           |   + END LOOP
; CHECK:           |   %k.022 = @llvm.vector.reduce.add.v4i32(%.vec4);
;
; CHECK:           + END LOOP
; CHECK-NEXT:  END REGION
;

define dso_local noundef i32 @_Z3fooPv(i8* noundef %vperm) local_unnamed_addr #0 {
entry:
  %0 = ptrtoint i8* %vperm to i64
  %add.i = add i64 %0, 15
  %and.i = and i64 %add.i, -16
  %sub2.i = sub i64 -1901, %0
  %1 = add i64 %sub2.i, %and.i
  %cmp.i = icmp ult i64 %1, -2001
  %2 = inttoptr i64 %and.i to i32*
  %3 = select i1 %cmp.i, i32* null, i32* %2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %k.022 = phi i32 [ 0, %entry ], [ %spec.select.lcssa, %for.inc5 ]
  %i.021 = phi i64 [ 0, %entry ], [ %inc6, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %k.120 = phi i32 [ %k.022, %for.cond1.preheader ], [ %spec.select, %for.body3 ]
  %j.019 = phi i64 [ 1, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, i32* %3, i64 %j.019
  %4 = load i32, i32* %arrayidx, align 4
  %conv = sext i32 %4 to i64
  %cmp4 = icmp eq i64 %i.021, %conv
  %add = zext i1 %cmp4 to i32
  %spec.select = add nsw i32 %k.120, %add
  %inc = add nuw nsw i64 %j.019, 1
  %exitcond.not = icmp eq i64 %inc, 8
  br i1 %exitcond.not, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body3 ]
  %inc6 = add nuw nsw i64 %i.021, 1
  %exitcond23.not = icmp eq i64 %inc6, 7
  br i1 %exitcond23.not, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  %spec.select.lcssa.lcssa = phi i32 [ %spec.select.lcssa, %for.inc5 ]
  ret i32 %spec.select.lcssa.lcssa
}
