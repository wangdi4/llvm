; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug-only=vplan-vls-analysis \
; RUN:     2>&1 | FileCheck %s

;
; REQUIRES: asserts
;

; CHECK: Information about {{.*}} SStore: {{.*}} (%tmp)[0][4 * i1]
; CHECK-NEXT:    SStore: {{.*}} (%tmp)[0][4 * i1 + 1]  -4 | can be moved
; CHECK-NEXT:    SStore: {{.*}} (%tmp)[0][4 * i1 + 2]  -8 | can be moved
; CHECK-NEXT:    SStore: {{.*}} (%tmp)[0][4 * i1 + 3]  -12 | can be moved

source_filename = "pixel_hadamard_ac.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define dso_local i64 @_Z17pixel_hadamard_acPhi(i8* nocapture readonly %pix, i32 %stride) local_unnamed_addr #0 {
entry:
  %tmp = alloca [32 x i32], align 16
  %0 = bitcast [32 x i32]* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 128, i8* nonnull %0) #2
  %idx.ext55 = sext i32 %stride to i64
  br label %for.body61.preheader

for.body61.preheader:                             ; preds = %for.body
  br label %for.body61

for.cond128.preheader:                            ; preds = %for.body61
  %add123.lcssa = phi i32 [ %add123, %for.body61 ]
  br label %for.body131

for.body61:                                       ; preds = %for.body61.preheader, %for.body61
  %indvars.iv338 = phi i64 [ %indvars.iv.next339, %for.body61 ], [ 0, %for.body61.preheader ]
  %sum4.0332 = phi i32 [ %add123, %for.body61 ], [ 0, %for.body61.preheader ]
  %1 = shl nsw i64 %indvars.iv338, 2
  %arrayidx64 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %1, !intel-tbaa !7
  %2 = load i32, i32* %arrayidx64, align 16, !tbaa !7
  %3 = or i64 %1, 1
  %arrayidx68 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %3, !intel-tbaa !7
  %4 = load i32, i32* %arrayidx68, align 4, !tbaa !7
  %add69 = add i32 %4, %2
  %sub78 = sub i32 %2, %4
  %5 = or i64 %1, 2
  %arrayidx82 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %5, !intel-tbaa !7
  %6 = load i32, i32* %arrayidx82, align 8, !tbaa !7
  %7 = or i64 %1, 3
  %arrayidx86 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %7, !intel-tbaa !7
  %8 = load i32, i32* %arrayidx86, align 4, !tbaa !7
  %add87 = add i32 %8, %6
  %sub96 = sub i32 %6, %8
  %add97 = add nsw i32 %add87, %add69
  %sub98 = sub nsw i32 %add69, %add87
  %add99 = add nsw i32 %sub96, %sub78
  %sub100 = sub nsw i32 %sub78, %sub96
  store i32 %add97, i32* %arrayidx64, align 16, !tbaa !7
  store i32 %add99, i32* %arrayidx68, align 4, !tbaa !7
  store i32 %sub98, i32* %arrayidx82, align 8, !tbaa !7
  store i32 %sub100, i32* %arrayidx86, align 4, !tbaa !7
  %shr.i = lshr i32 %add97, 15
  %and.i = and i32 %shr.i, 65537
  %mul.i = mul nuw i32 %and.i, 65535
  %add.i = add i32 %mul.i, %add97
  %xor.i = xor i32 %add.i, %mul.i
  %shr.i324 = lshr i32 %add99, 15
  %and.i325 = and i32 %shr.i324, 65537
  %mul.i326 = mul nuw i32 %and.i325, 65535
  %add.i327 = add i32 %mul.i326, %add99
  %xor.i328 = xor i32 %add.i327, %mul.i326
  %shr.i319 = lshr i32 %sub98, 15
  %and.i320 = and i32 %shr.i319, 65537
  %mul.i321 = mul nuw i32 %and.i320, 65535
  %add.i322 = add i32 %mul.i321, %sub98
  %xor.i323 = xor i32 %add.i322, %mul.i321
  %shr.i314 = lshr i32 %sub100, 15
  %and.i315 = and i32 %shr.i314, 65537
  %mul.i316 = mul nuw i32 %and.i315, 65535
  %add.i317 = add i32 %mul.i316, %sub100
  %xor.i318 = xor i32 %add.i317, %mul.i316
  %add118 = add i32 %xor.i328, %sum4.0332
  %add120 = add i32 %add118, %xor.i
  %add122 = add i32 %add120, %xor.i323
  %add123 = add i32 %add122, %xor.i318
  %indvars.iv.next339 = add nuw nsw i64 %indvars.iv338, 1
  %exitcond344 = icmp eq i64 %indvars.iv.next339, 8
  br i1 %exitcond344, label %for.cond128.preheader, label %for.body61

for.cond.cleanup130:                              ; preds = %for.body131
  %add173.lcssa = phi i32 [ %add173, %for.body131 ]
  %arrayidx177 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 0, !intel-tbaa !7
  %9 = load i32, i32* %arrayidx177, align 16, !tbaa !7
  %arrayidx178 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 8, !intel-tbaa !7
  %10 = load i32, i32* %arrayidx178, align 16, !tbaa !7
  %add179 = add i32 %10, %9
  %arrayidx180 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 16, !intel-tbaa !7
  %11 = load i32, i32* %arrayidx180, align 16, !tbaa !7
  %add181 = add i32 %add179, %11
  %arrayidx182 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 24, !intel-tbaa !7
  %12 = load i32, i32* %arrayidx182, align 16, !tbaa !7
  %add183 = add i32 %add181, %12
  %conv185 = and i32 %add183, 65535
  %conv187 = and i32 %add123.lcssa, 65535
  %shr = lshr i32 %add123.lcssa, 16
  %add188 = add nuw nsw i32 %conv187, %shr
  %sub189 = sub nsw i32 %add188, %conv185
  %conv191 = and i32 %add173.lcssa, 65535
  %shr192 = lshr i32 %add173.lcssa, 16
  %add193 = add nuw nsw i32 %conv191, %shr192
  %sub194 = sub nsw i32 %add193, %conv185
  %conv195293 = zext i32 %sub194 to i64
  %shl196 = shl nuw i64 %conv195293, 32
  %conv197 = sext i32 %sub189 to i64
  %add198 = add i64 %shl196, %conv197
  call void @llvm.lifetime.end.p0i8(i64 128, i8* nonnull %0) #2
  ret i64 %add198

for.body131:                                      ; preds = %for.body131, %for.cond128.preheader
  %indvars.iv = phi i64 [ 0, %for.cond128.preheader ], [ %indvars.iv.next, %for.body131 ]
  %sum8.0329 = phi i32 [ 0, %for.cond128.preheader ], [ %add173, %for.body131 ]
  %arrayidx134 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %indvars.iv, !intel-tbaa !7
  %13 = load i32, i32* %arrayidx134, align 4, !tbaa !7
  %14 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx137 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %14, !intel-tbaa !7
  %15 = load i32, i32* %arrayidx137, align 4, !tbaa !7
  %add138 = add i32 %15, %13
  %sub145 = sub i32 %13, %15
  %16 = add nuw nsw i64 %indvars.iv, 16
  %arrayidx149 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %16, !intel-tbaa !7
  %17 = load i32, i32* %arrayidx149, align 4, !tbaa !7
  %18 = add nuw nsw i64 %indvars.iv, 24
  %arrayidx152 = getelementptr inbounds [32 x i32], [32 x i32]* %tmp, i64 0, i64 %18, !intel-tbaa !7
  %19 = load i32, i32* %arrayidx152, align 4, !tbaa !7
  %add153 = add i32 %19, %17
  %sub161 = sub i32 %17, %19
  %add162 = add nsw i32 %add153, %add138
  %sub163 = sub nsw i32 %add138, %add153
  %add164 = add nsw i32 %sub161, %sub145
  %sub165 = sub nsw i32 %sub145, %sub161
  %shr.i309 = lshr i32 %add162, 15
  %and.i310 = and i32 %shr.i309, 65537
  %mul.i311 = mul nuw i32 %and.i310, 65535
  %add.i312 = add i32 %mul.i311, %add162
  %xor.i313 = xor i32 %add.i312, %mul.i311
  %shr.i304 = lshr i32 %add164, 15
  %and.i305 = and i32 %shr.i304, 65537
  %mul.i306 = mul nuw i32 %and.i305, 65535
  %add.i307 = add i32 %mul.i306, %add164
  %xor.i308 = xor i32 %add.i307, %mul.i306
  %shr.i299 = lshr i32 %sub163, 15
  %and.i300 = and i32 %shr.i299, 65537
  %mul.i301 = mul nuw i32 %and.i300, 65535
  %add.i302 = add i32 %mul.i301, %sub163
  %xor.i303 = xor i32 %add.i302, %mul.i301
  %shr.i294 = lshr i32 %sub165, 15
  %and.i295 = and i32 %shr.i294, 65537
  %mul.i296 = mul nuw i32 %and.i295, 65535
  %add.i297 = add i32 %mul.i296, %sub165
  %xor.i298 = xor i32 %add.i297, %mul.i296
  %add168 = add i32 %xor.i308, %sum8.0329
  %add170 = add i32 %add168, %xor.i313
  %add172 = add i32 %add170, %xor.i303
  %add173 = add i32 %add172, %xor.i298
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond, label %for.cond.cleanup130, label %for.body131
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 846c7b63b34b5c4bd6d402aa32e62c2fc1a2d82d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a5a87bf91f3b624d444b7ef3f4a976be213f2a5d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!4, !4, i64 0}
!7 = !{!8, !3, i64 0}
!8 = !{!"array@_ZTSA32_j", !3, i64 0}
