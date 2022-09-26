; Test to check correctness of generated code when master user of loop IV PHI is invalidated.

; HIR incoming to vectorizer
; <0>     BEGIN REGION { }
; <15>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <14>
; <14>          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>
; <2>           |   %a.010.out = %a.010; <Safe Reduction>
; <4>           |   %0 = (%A)[i1];
; <7>           |   %a.010 = i1 + %a.010.out  +  %0; <Safe Reduction>
; <14>          + END LOOP
; <14>
; <16>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; In the above loop node <7> will be invalidated when VPEntities are used to represent the reduction.
; CG should generate explicit vector code for the loop IV PHI to prevent compfails.
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 < %s 2>&1 | FileCheck %s


; CHECK:            %red.init = 0;
; CHECK-NEXT:       [[RED_INIT_INSERT:%.*]] = insertelement %red.init,  %a.010,  0;
; CHECK-NEXT:       [[PHI_TEMP:%.*]] = [[RED_INIT_INSERT]];

; CHECK:            + DO i1 = 0, {{.*}}, 4   <DO_LOOP>  <MAX_TC_EST = 536870911>   <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:       |   [[COPY:%.*]] = [[PHI_TEMP]];
; CHECK-NEXT:       |   [[VEC:%.*]] = (<4 x i32>*)(%A)[i1];
; CHECK-NEXT:       |   [[VEC2:%.*]] = [[COPY]]  +  i1 + <i32 0, i32 1, i32 2, i32 3>;
; CHECK-NEXT:       |   [[VEC3:%.*]] = [[VEC2]]  +  [[VEC]];
; CHECK-NEXT:       |   [[PHI_TEMP]] = [[VEC3]];
; CHECK-NEXT:       + END LOOP

; CHECK:            %a.010 = @llvm.vector.reduce.add.v4i32([[VEC3]]);


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %n, i32* nocapture readonly %A) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count11 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %a.010 = phi i32 [ 0, %for.body.preheader ], [ %add1, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add i32 %a.010, %1
  %add1 = add i32 %add, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count11
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %add1.lcssa = phi i32 [ %add1, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %a.0.lcssa = phi i32 [ 0, %entry ], [ %add1.lcssa, %for.end.loopexit ]
  ret i32 %a.0.lcssa
}
