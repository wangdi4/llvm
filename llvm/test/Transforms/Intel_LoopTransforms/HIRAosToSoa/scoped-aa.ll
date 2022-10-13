; RUN: opt -aa-pipeline=scoped-noalias-aa -hir-ssa-deconstruction -hir-temp-cleanup -hir-aos-to-soa -print-after=hir-aos-to-soa < %s -disable-output 2>&1 | FileCheck --check-prefixes=CHECK,RESTRICT %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-aos-to-soa,print<hir>" -aa-pipeline="basic-aa,scoped-noalias-aa" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -hir-ssa-deconstruction -hir-temp-cleanup -hir-aos-to-soa -print-after=hir-aos-to-soa < %s -disable-output 2>&1 | FileCheck --check-prefixes=CHECK,NORESTRICT %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-aos-to-soa,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,NORESTRICT %s

; Check that AosToSoa kicks in for cases where memory references may have outer
; loop dependencies but are understood to have no inner loop dependence due to
; ScopedNoAlias metadata. This is important for CPU2017's 538.imagick_r.
;
; We also check that AosToSoa does not occur when the metadata is not
; available; in this case the transformation should be legal only with the
; metadata.

; CHECK: Function: foo

; CHECK: DO i1 =
; RESTRICT: [[ADDR:%[0-9a-z]+]] = @llvm.stacksave();
; RESTRICT: [[SUM:%[0-9a-z]+]] = 1024 + 1024;
; NORESTRICT-NOT: %[0-9a-z]+ = @llvm.stacksave();

; RESTRICT: [[ALLOC_0:%[0-9a-z]+]] = alloca %array_size;
; RESTRICT-NEXT: [[ALLOC_1:%[0-9a-z]+]] = alloca %array_size;
; RESTRICT-NEXT: [[ALLOC_2:%[0-9a-z]+]] = alloca %array_size;
; NORESTRICT-NOT: %[0-9a-z]+ = alloca %array_size;
; NORESTRICT: [[INNERARRAY:%[0-9a-z]+]] = (%Ap)[i1];

; RESTRICT: DO i2 =
; RESTRICT: DO i3 =
; RESTRICT:     [[ALLOC_0]]
; RESTRICT:     [[ALLOC_1]]
; RESTRICT:     [[ALLOC_2]]

; CHECK: DO i2 =
; CHECK: DO i3 =
; CHECK: DO i4 =
; RESTRICT:   = uitofp.i32.float(([[ALLOC_0]])[i2 + [[SUM]] * i3 + i4]);
; RESTRICT:   = uitofp.i32.float(([[ALLOC_1]])[i2 + [[SUM]] * i3 + i4]);
; RESTRICT:   = uitofp.i32.float(([[ALLOC_2]])[i2 + [[SUM]] * i3 + i4]);
; NORESTRICT: ([[INNERARRAY]])[i2 + %alpha * i3 + i4].0);
; NORESTRICT: ([[INNERARRAY]])[i2 + %alpha * i3 + i4].1);
; NORESTRICT: ([[INNERARRAY]])[i2 + %alpha * i3 + i4].2);
; CHECK: END LOOP
; CHECK: END LOOP
; CHECK: END LOOP

; RESTRICT: @llvm.stackrestore(&(([[ADDR]])[0]));
; NORESTRICT-NOT: @llvm.stackrestore(%[0-9a-z]+);
; CHECK: END LOOP

%struct.point = type { i32, i32, i32 }

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local float @foo(%struct.point** nocapture readonly %Ap, %struct.point** nocapture readonly %Bp, i64 %alpha) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  %add41.lcssa.lcssa.lcssa.lcssa = phi float [ %add41.lcssa.lcssa.lcssa, %for.cond.cleanup5 ]
  ret float %add41.lcssa.lcssa.lcssa.lcssa

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv123 = phi i64 [ 0, %entry ], [ %indvars.iv.next124, %for.cond.cleanup5 ]
  %sum.0115 = phi float [ 0.000000e+00, %entry ], [ %add41.lcssa.lcssa.lcssa, %for.cond.cleanup5 ]
  %ptridx = getelementptr inbounds %struct.point*, %struct.point** %Ap, i64 %indvars.iv123
  %0 = load %struct.point*, %struct.point** %ptridx, align 8
  %ptridx2 = getelementptr inbounds %struct.point*, %struct.point** %Bp, i64 %indvars.iv123
  %1 = load %struct.point*, %struct.point** %ptridx2, align 8
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.body, %for.cond.cleanup9
  %indvars.iv119 = phi i64 [ 0, %for.body ], [ %indvars.iv.next120, %for.cond.cleanup9 ]
  %sum.1113 = phi float [ %sum.0115, %for.body ], [ %add41.lcssa.lcssa, %for.cond.cleanup9 ]
  br label %for.cond11.preheader

for.cond.cleanup5:                                ; preds = %for.cond.cleanup9
  %add41.lcssa.lcssa.lcssa = phi float [ %add41.lcssa.lcssa, %for.cond.cleanup9 ]
  %indvars.iv.next124 = add nuw nsw i64 %indvars.iv123, 1
  %exitcond125 = icmp eq i64 %indvars.iv.next124, 999
  br i1 %exitcond125, label %for.cond.cleanup, label %for.body

for.cond11.preheader:                             ; preds = %for.cond7.preheader, %for.cond.cleanup13
  %indvars.iv116 = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next117, %for.cond.cleanup13 ]
  %sum.2111 = phi float [ %sum.1113, %for.cond7.preheader ], [ %add41.lcssa, %for.cond.cleanup13 ]
  %mul = mul nsw i64 %indvars.iv116, %alpha
  %add = add nsw i64 %mul, %indvars.iv119
  br label %for.body14

for.cond.cleanup9:                                ; preds = %for.cond.cleanup13
  %add41.lcssa.lcssa = phi float [ %add41.lcssa, %for.cond.cleanup13 ]
  %conv45 = fptoui float %add41.lcssa.lcssa to i32
  %2 = add nuw nsw i64 %indvars.iv119, %indvars.iv123
  %x49 = getelementptr inbounds %struct.point, %struct.point* %1, i64 %2, i32 0
  store i32 %conv45, i32* %x49, align 4, !alias.scope !9, !noalias !12
  %y54 = getelementptr inbounds %struct.point, %struct.point* %1, i64 %2, i32 1
  store i32 %conv45, i32* %y54, align 4, !alias.scope !9, !noalias !12
  %z59 = getelementptr inbounds %struct.point, %struct.point* %1, i64 %2, i32 2
  store i32 %conv45, i32* %z59, align 4, !alias.scope !9, !noalias !12
  %indvars.iv.next120 = add nuw nsw i64 %indvars.iv119, 1
  %exitcond122 = icmp eq i64 %indvars.iv.next120, 1024
  br i1 %exitcond122, label %for.cond.cleanup5, label %for.cond7.preheader

for.cond.cleanup13:                               ; preds = %for.body14
  %add41.lcssa = phi float [ %add41, %for.body14 ]
  %indvars.iv.next117 = add nuw nsw i64 %indvars.iv116, 1
  %exitcond118 = icmp eq i64 %indvars.iv.next117, 1024
  br i1 %exitcond118, label %for.cond.cleanup9, label %for.cond11.preheader

for.body14:                                       ; preds = %for.cond11.preheader, %for.body14
  %indvars.iv = phi i64 [ 0, %for.cond11.preheader ], [ %indvars.iv.next, %for.body14 ]
  %sum.3109 = phi float [ %sum.2111, %for.cond11.preheader ], [ %add41, %for.body14 ]
  %add17 = add nsw i64 %add, %indvars.iv
  %x19 = getelementptr inbounds %struct.point, %struct.point* %0, i64 %add17, i32 0
  %3 = load i32, i32* %x19, align 4, !alias.scope !12, !noalias !9
  %conv20 = uitofp i32 %3 to float
  %y28 = getelementptr inbounds %struct.point, %struct.point* %0, i64 %add17, i32 1
  %4 = load i32, i32* %y28, align 4, !alias.scope !12, !noalias !9
  %conv29 = uitofp i32 %4 to float
  %z37 = getelementptr inbounds %struct.point, %struct.point* %0, i64 %add17, i32 2
  %5 = load i32, i32* %z37, align 4, !alias.scope !12, !noalias !9
  %conv38 = uitofp i32 %5 to float
  %add39 = fadd float %conv20, %conv29
  %add40 = fadd float %add39, %conv38
  %add41 = fadd float %sum.3109, %add40
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup13, label %for.body14
}

!9 = !{!10}
!10 = distinct !{!10, !11, !"foo: B"}
!11 = distinct !{!11, !"foo"}
!12 = !{!13}
!13 = distinct !{!13, !11, !"foo: A"}
