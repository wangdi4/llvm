; RUN: opt -opaque-pointers=0 -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>'  -disable-output -debug-only=parvec-analysis -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s
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
; CHECK-NEXT:           %insert = insertelement zeroinitializer,  %n.037,  0;
; CHECK-NEXT:           %insert1 = insertelement zeroinitializer,  %n2.036,  0;
; CHECK-NEXT:           %phi.temp = %insert1;
; CHECK-NEXT:           %phi.temp2 = %insert;
;
; CHECK:                + DO i1 = 0, 1023, 32   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:           |   %.vec6 = undef;
; CHECK-NEXT:           |   %.vec = (<32 x i64>*)(%tj)[i1];
; CHECK-NEXT:           |   %.vec4 = %.vec > 10;
; CHECK-NEXT:           |   %.vec5 = %.vec4  ^  -1;
; CHECK-NEXT:           |   %.vec6 = (<32 x i64>*)(%ttag)[i1], Mask = @{%.vec4};
; CHECK-NEXT:           |   %.vec7 = %.vec6 == 0;
; CHECK-NEXT:           |   %.vec8 = %.vec7  ^  -1;
; CHECK-NEXT:           |   %.vec9 = %.vec4  &  %.vec7;
; CHECK-NEXT:           |   %.vec10 = %.vec4  &  %.vec8;
; CHECK-NEXT:           |   %extract.0. = extractelement &((<32 x i64*>)(%neighptr2)[%phi.temp2]),  0;
; CHECK-NEXT:           |   @llvm.masked.compressstore.v32i64(%.vec,  %extract.0.,  %.vec9);
; CHECK-NEXT:           |   %.vec11 = %.vec10  |  %.vec5;
; CHECK-NEXT:           |   %extract.0.12 = extractelement &((<32 x i64*>)(%neighptr)[%phi.temp]),  0;
; CHECK-NEXT:           |   @llvm.masked.compressstore.v32i64(%.vec,  %extract.0.12,  %.vec11);
; CHECK-NEXT:           |   %select = (%.vec11 == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? %phi.temp + 1 : %phi.temp;
; CHECK-NEXT:           |   %select13 = (%.vec11 == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? %phi.temp2 : %phi.temp2 + 1;
; CHECK-NEXT:           |   %vec.reduce = @llvm.vector.reduce.add.v32i64(%select);
; CHECK-NEXT:           |   %insert14 = insertelement zeroinitializer,  %vec.reduce,  0;
; CHECK-NEXT:           |   %vec.reduce15 = @llvm.vector.reduce.add.v32i64(%select13);
; CHECK-NEXT:           |   %insert16 = insertelement zeroinitializer,  %vec.reduce15,  0;
; CHECK-NEXT:           |   %phi.temp = %insert14;
; CHECK-NEXT:           |   %phi.temp2 = %insert16;
; CHECK-NEXT:           + END LOOP
;
; CHECK:                %extract.0.19 = extractelement %insert16,  0;
; CHECK-NEXT:           %n.037 = %extract.0.19;
; CHECK-NEXT:           %extract.0.21 = extractelement %insert14,  0;
; CHECK-NEXT:           %n2.036 = %extract.0.21;
; CHECK-NEXT:     END REGION
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define void @foo(i64* %tj, i64* %ttag, i64* noalias %neighptr2, i64* noalias %neighptr) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %if.end15
  %n.037 = phi i64 [ 0, %entry ], [ %n.1, %if.end15 ]
  %n2.036 = phi i64 [ 0, %entry ], [ %n2.1, %if.end15 ]
  %u.035 = phi i64 [ 0, %entry ], [ %add, %if.end15 ]
  %arrayidx = getelementptr inbounds i64, i64* %tj, i64 %u.035
  %tjval = load i64, i64* %arrayidx, align 8
  %cmp4 = icmp sgt i64 %tjval, 10
  br i1 %cmp4, label %if.then5, label %if.else12

if.then5:                                         ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i64, i64* %ttag, i64 %u.035
  %ttagval = load i64, i64* %arrayidx3, align 8
  %tobool6.not = icmp eq i64 %ttagval, 0
  br i1 %tobool6.not, label %if.else, label %if.then7

if.then7:                                         ; preds = %if.then5
  br label %if.else12

if.else:                                          ; preds = %if.then5
  %inc9 = add nsw i64 %n.037, 1
  %arrayidx10 = getelementptr inbounds i64, i64* %neighptr2, i64 %n.037
  store i64 %tjval, i64* %arrayidx10, align 8
  br label %if.end15

if.else12:                                        ; preds = %for.body
  %inc13 = add nsw i64 %n2.036, 1
  %arrayidx14 = getelementptr inbounds i64, i64* %neighptr, i64 %n2.036
  store i64 %tjval, i64* %arrayidx14, align 8
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
