; RUN: opt -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>'  -disable-output -debug-only=parvec-analysis -hir-details-no-verbose-indent -vplan-force-vf=32 < %s 2>&1 | FileCheck %s
;
; LIT test to check that we recognize compress idiom when the increment and store are
; not under an if. This can happen if the HIR framework fails to build an if and uses
; gotos/labels.
;
;
; CHECK:          Idiom List
; CHECK-NEXT:     CEIndexIncFirst: %n.037 = %n.037  +  1;
; CHECK-NEXT:       CEStore: (%neighptr2)[%n.037] = %tjval;
; CHECK-NEXT:         CELdStIndex: %n.037
; CHECK-NEXT:     CEIndexIncFirst: %n2.036 = %n2.036  +  1;
; CHECK-NEXT:       CEStore: (%neighptr)[%n2.036] = %tjval;
; CHECK-NEXT:         CELdStIndex: %n2.036
;
; CHECK:          Function: foo
;
; CHECK:          BEGIN REGION { }
; CHECK-NEXT:           %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
; CHECK:                + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK-NEXT:           |   %tjval = (%tj)[i1];
; CHECK-NEXT:           |   if (%tjval > 10)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      if ((%ttag)[i1] == 0)
; CHECK-NEXT:           |      {
; CHECK-NEXT:           |         (%neighptr2)[%n.037] = %tjval;
; CHECK-NEXT:           |         %n.037 = %n.037  +  1;
; CHECK-NEXT:           |         goto if.end15;
; CHECK-NEXT:           |      }
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   (%neighptr)[%n2.036] = %tjval;
; CHECK-NEXT:           |   %n2.036 = %n2.036  +  1;
; CHECK-NEXT:           |   if.end15:
; CHECK-NEXT:           + END LOOP
;
; CHECK:                @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:     END REGION
;
; CHECK:          Function: foo
;
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           %phi.temp = %n2.036;
; CHECK-NEXT:           %phi.temp1 = %n.037;

; CHECK:                + DO i1 = 0, 1023, 32   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:           |   %.vec5 = undef;
; CHECK-NEXT:           |   %.vec = (<32 x i64>*)(%tj)[i1];
; CHECK-NEXT:           |   %.vec3 = %.vec > 10;
; CHECK-NEXT:           |   %.vec4 = %.vec3  ^  -1;
; CHECK-NEXT:           |   %.vec5 = (<32 x i64>*)(%ttag)[i1], Mask = @{%.vec3};
; CHECK-NEXT:           |   %.vec6 = %.vec5 == 0;
; CHECK-NEXT:           |   %.vec7 = %.vec6  ^  -1;
; CHECK-NEXT:           |   %.vec8 = (%.vec > 10) ? %.vec6 : 0;
; CHECK-NEXT:           |   %.vec9 = (%.vec > 10) ? %.vec7 : 0;
; CHECK-NEXT:           |   @llvm.masked.compressstore.v32i64(%.vec,  &((i64*)(%neighptr2)[%phi.temp1]),  %.vec8);
; CHECK-NEXT:           |   %.vec10 = %.vec9  |  %.vec4;
; CHECK-NEXT:           |   @llvm.masked.compressstore.v32i64(%.vec,  &((i64*)(%neighptr)[%phi.temp]),  %.vec10);
; CHECK-NEXT:           |   %select = (%.vec10 == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? -1 : 0;
; CHECK-NEXT:           |   %select11 = (%.vec10 == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? 0 : -1;
; CHECK-NEXT:           |   %cast = bitcast.<32 x i1>.i32(%select);
; CHECK-NEXT:           |   %popcnt = @llvm.ctpop.i32(%cast);
; CHECK-NEXT:           |   %zext = zext.i32.i64(%popcnt);
; CHECK-NEXT:           |   %mul = %zext  *  1;
; CHECK-NEXT:           |   %add12 = %phi.temp  +  %mul;
; CHECK-NEXT:           |   %cast13 = bitcast.<32 x i1>.i32(%select11);
; CHECK-NEXT:           |   %popcnt14 = @llvm.ctpop.i32(%cast13);
; CHECK-NEXT:           |   %zext15 = zext.i32.i64(%popcnt14);
; CHECK-NEXT:           |   %mul16 = %zext15  *  1;
; CHECK-NEXT:           |   %add17 = %phi.temp1  +  %mul16;
; CHECK-NEXT:           |   %phi.temp = %add12;
; CHECK-NEXT:           |   %phi.temp1 = %add17;
; CHECK-NEXT:           + END LOOP

; CHECK:                %n.037 = %add17;
; CHECK-NEXT:           %n2.036 = %add12;
; CHECK-NEXT:     END REGION
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define void @foo(ptr %tj, ptr %ttag, ptr noalias %neighptr2, ptr noalias %neighptr) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %if.end15
  %n.037 = phi i64 [ 0, %entry ], [ %n.1, %if.end15 ]
  %n2.036 = phi i64 [ 0, %entry ], [ %n2.1, %if.end15 ]
  %u.035 = phi i64 [ 0, %entry ], [ %add, %if.end15 ]
  %arrayidx = getelementptr inbounds i64, ptr %tj, i64 %u.035
  %tjval = load i64, ptr %arrayidx, align 8
  %cmp4 = icmp sgt i64 %tjval, 10
  br i1 %cmp4, label %if.then5, label %if.else12

if.then5:                                         ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i64, ptr %ttag, i64 %u.035
  %ttagval = load i64, ptr %arrayidx3, align 8
  %tobool6.not = icmp eq i64 %ttagval, 0
  br i1 %tobool6.not, label %if.else, label %if.then7

if.then7:                                         ; preds = %if.then5
  br label %if.else12

if.else:                                          ; preds = %if.then5
  %inc9 = add nsw i64 %n.037, 1
  %arrayidx10 = getelementptr inbounds i64, ptr %neighptr2, i64 %n.037
  store i64 %tjval, ptr %arrayidx10, align 8
  br label %if.end15

if.else12:                                        ; preds = %for.body
  %inc13 = add nsw i64 %n2.036, 1
  %arrayidx14 = getelementptr inbounds i64, ptr %neighptr, i64 %n2.036
  store i64 %tjval, ptr %arrayidx14, align 8
  br label %if.end15

if.end15:                                         ; preds = %if.then7, %if.else, %if.else12
  %n2.1 = phi i64 [ %n2.036, %if.else ], [ %inc13, %if.else12 ]
  %n.1 = phi i64 [ %inc9, %if.else ], [ %n.037, %if.else12 ]
  %add = add nuw nsw i64 %u.035, 1
  %exitcond.not = icmp eq i64 %add, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %if.end15
  ret void
}
