; RUN: opt -vector-library=SVML -hir-ssa-deconstruction -disable-output -hir-temp-cleanup -hir-loop-interchange -hir-pre-vec-complete-unroll -hir-lmm -hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-pre-vec-complete-unroll,hir-lmm,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -vector-library=SVML -disable-output < %s 2>&1 | FileCheck %s

; The second inner loop with ub=29 was getting assertion fail
; This test checks if that loop is fused successfully or not

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 29, 1   <DO_LOOP>

; CHECK: + DO i2 = 0, 29, 1   <DO_LOOP>
; CHECK: + END LOOP

; CHECK: + DO i2 = 0, 29, 1   <DO_LOOP>
; CHECK: + END LOOP

; CHECK: + END LOOP

; ModuleID = 'func.bc'
source_filename = "backprop.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@next_input = external dso_local global [30 x [1221 x float]], align 16
@next_output = external dso_local global [30 x [35 x float]], align 16
@i_h_weights = external dso_local global [30 x [1221 x float]], align 16
@h_o_weights = external dso_local global [35 x [31 x float]], align 16
@i_h_w_ch_sum_array = external dso_local local_unnamed_addr global [30 x [1221 x float]], align 16
@h_o_w_ch_sum_array = external dso_local local_unnamed_addr global [35 x [31 x float]], align 16
@i_h_lrc = external dso_local local_unnamed_addr global float, align 4
@h_o_lrc = external dso_local local_unnamed_addr global float, align 4
@hidden_delta = external dso_local global [31 x float], align 16
@.str.8 = external hidden unnamed_addr constant [31 x i8], align 1
@str = external hidden unnamed_addr constant [33 x i8]
@str.9 = external hidden unnamed_addr constant [18 x i8]

; Function Attrs: nounwind uwtable
declare dso_local i32 @initialize() local_unnamed_addr #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare dso_local double @exp(double) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %psum_array.i = alloca [31 x float], align 16
  %hidden_act = alloca [31 x float], align 16
  %output_act = alloca [35 x float], align 16
  %0 = bitcast [31 x float]* %hidden_act to i8*
  call void @llvm.lifetime.start.p0i8(i64 124, i8* nonnull %0) #3
  %1 = bitcast [35 x float]* %output_act to i8*
  call void @llvm.lifetime.start.p0i8(i64 140, i8* nonnull %1) #3
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @str, i64 0, i64 0))
  %call1 = tail call i32 @initialize()
  %puts44 = tail call i32 @puts(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @str.9, i64 0, i64 0))
  %arraydecay6 = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 0
  %arrayidx12.i = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 30
  %arraydecay9 = getelementptr inbounds [35 x float], [35 x float]* %output_act, i64 0, i64 0
  %2 = bitcast [31 x float]* %psum_array.i to i8*
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %update_weights.exit, %entry
  br label %for.body5

for.body5:                                        ; preds = %hidden_input.exit, %for.cond3.preheader
  %indvars.iv = phi i64 [ 0, %for.cond3.preheader ], [ %indvars.iv.next, %hidden_input.exit ]
  %error.094 = phi float [ 0.000000e+00, %for.cond3.preheader ], [ %add.i65.lcssa, %hidden_input.exit ]
  br label %for.cond4.preheader.i

for.cond4.preheader.i:                            ; preds = %for.end.i, %for.body5
  %receiver.0.ptr32.i = phi float* [ %arraydecay6, %for.body5 ], [ %receiver.0.ptr.i, %for.end.i ]
  %receiver.0.idx31.i = phi i64 [ 0, %for.body5 ], [ %receiver.0.add.i, %for.end.i ]
  %weight.030.i = phi float* [ getelementptr inbounds ([30 x [1221 x float]], [30 x [1221 x float]]* @i_h_weights, i64 0, i64 0, i64 0), %for.body5 ], [ %scevgep.i, %for.end.i ]
  store float 0.000000e+00, float* %receiver.0.ptr32.i, align 4, !tbaa !2
  br label %for.body6.i

for.body6.i:                                      ; preds = %for.body6.i, %for.cond4.preheader.i
  %weight.129.i = phi float* [ %weight.030.i, %for.cond4.preheader.i ], [ %incdec.ptr7.i, %for.body6.i ]
  %sender.0.idx28.i = phi i64 [ 0, %for.cond4.preheader.i ], [ %sender.0.add.i, %for.body6.i ]
  %storemerge27.i = phi float [ 0.000000e+00, %for.cond4.preheader.i ], [ %add.i, %for.body6.i ]
  %sender.0.ptr.i = getelementptr inbounds [30 x [1221 x float]], [30 x [1221 x float]]* @next_input, i64 0, i64 %indvars.iv, i64 %sender.0.idx28.i
  %sender.0.add.i = add nuw nsw i64 %sender.0.idx28.i, 1
  %3 = load float, float* %sender.0.ptr.i, align 4, !tbaa !2
  %incdec.ptr7.i = getelementptr inbounds float, float* %weight.129.i, i64 1
  %4 = load float, float* %weight.129.i, align 4, !tbaa !2
  %mul.i = fmul float %3, %4
  %add.i = fadd float %storemerge27.i, %mul.i
  store float %add.i, float* %receiver.0.ptr32.i, align 4, !tbaa !2
  %exitcond.i = icmp eq i64 %sender.0.add.i, 1221
  br i1 %exitcond.i, label %for.end.i, label %for.body6.i

for.end.i:                                        ; preds = %for.body6.i
  %add.i.lcssa = phi float [ %add.i, %for.body6.i ]
  %scevgep.i = getelementptr float, float* %weight.030.i, i64 1221
  %sub.i = fsub float -0.000000e+00, %add.i.lcssa
  %conv.i = fpext float %sub.i to double
  %call.i = tail call double @exp(double %conv.i) #3
  %add8.i = fadd double %call.i, 1.000000e+00
  %div.i = fdiv double 1.000000e+00, %add8.i
  %conv9.i = fptrunc double %div.i to float
  store float %conv9.i, float* %receiver.0.ptr32.i, align 4, !tbaa !2
  %receiver.0.add.i = add nuw nsw i64 %receiver.0.idx31.i, 1
  %receiver.0.ptr.i = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 %receiver.0.add.i
  %exitcond33.i = icmp eq i64 %receiver.0.add.i, 30
  br i1 %exitcond33.i, label %input_hidden.exit, label %for.cond4.preheader.i

input_hidden.exit:                                ; preds = %for.end.i
  store float 1.000000e+00, float* %arrayidx12.i, align 8, !tbaa !2
  br label %for.cond4.preheader.i45

for.cond4.preheader.i45:                          ; preds = %for.end.i62, %input_hidden.exit
  %receiver.0.ptr30.i = phi float* [ %arraydecay9, %input_hidden.exit ], [ %receiver.0.ptr.i61, %for.end.i62 ]
  %receiver.0.idx29.i = phi i64 [ 0, %input_hidden.exit ], [ %receiver.0.add.i60, %for.end.i62 ]
  %weight.028.i = phi float* [ getelementptr inbounds ([35 x [31 x float]], [35 x [31 x float]]* @h_o_weights, i64 0, i64 0, i64 0), %input_hidden.exit ], [ %scevgep.i53, %for.end.i62 ]
  store float 0.000000e+00, float* %receiver.0.ptr30.i, align 4, !tbaa !2
  br label %for.body6.i52

for.body6.i52:                                    ; preds = %for.body6.i52, %for.cond4.preheader.i45
  %weight.127.i = phi float* [ %weight.028.i, %for.cond4.preheader.i45 ], [ %incdec.ptr7.i48, %for.body6.i52 ]
  %sender.0.idx26.i = phi i64 [ 0, %for.cond4.preheader.i45 ], [ %sender.0.add.i47, %for.body6.i52 ]
  %storemerge25.i = phi float [ 0.000000e+00, %for.cond4.preheader.i45 ], [ %add.i50, %for.body6.i52 ]
  %sender.0.ptr.i46 = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 %sender.0.idx26.i
  %sender.0.add.i47 = add nuw nsw i64 %sender.0.idx26.i, 1
  %5 = load float, float* %sender.0.ptr.i46, align 4, !tbaa !2
  %incdec.ptr7.i48 = getelementptr inbounds float, float* %weight.127.i, i64 1
  %6 = load float, float* %weight.127.i, align 4, !tbaa !2
  %mul.i49 = fmul float %5, %6
  %add.i50 = fadd float %storemerge25.i, %mul.i49
  store float %add.i50, float* %receiver.0.ptr30.i, align 4, !tbaa !2
  %exitcond.i51 = icmp eq i64 %sender.0.add.i47, 31
  br i1 %exitcond.i51, label %for.end.i62, label %for.body6.i52

for.end.i62:                                      ; preds = %for.body6.i52
  %add.i50.lcssa = phi float [ %add.i50, %for.body6.i52 ]
  %scevgep.i53 = getelementptr float, float* %weight.028.i, i64 31
  %sub.i54 = fsub float -0.000000e+00, %add.i50.lcssa
  %conv.i55 = fpext float %sub.i54 to double
  %call.i56 = tail call double @exp(double %conv.i55) #3
  %add8.i57 = fadd double %call.i56, 1.000000e+00
  %div.i58 = fdiv double 1.000000e+00, %add8.i57
  %conv9.i59 = fptrunc double %div.i58 to float
  store float %conv9.i59, float* %receiver.0.ptr30.i, align 4, !tbaa !2
  %receiver.0.add.i60 = add nuw nsw i64 %receiver.0.idx29.i, 1
  %receiver.0.ptr.i61 = getelementptr inbounds [35 x float], [35 x float]* %output_act, i64 0, i64 %receiver.0.add.i60
  %exitcond31.i = icmp eq i64 %receiver.0.add.i60, 35
  br i1 %exitcond31.i, label %for.body.i.preheader, label %for.cond4.preheader.i45

for.body.i.preheader:                             ; preds = %for.end.i62
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %7 = phi float [ %add.i65, %for.body.i ], [ %error.094, %for.body.i.preheader ]
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %for.body.i.preheader ]
  %arrayidx.i = getelementptr inbounds [30 x [35 x float]], [30 x [35 x float]]* @next_output, i64 0, i64 %indvars.iv, i64 %indvars.iv.i
  %8 = load float, float* %arrayidx.i, align 4, !tbaa !2
  %arrayidx2.i = getelementptr inbounds [35 x float], [35 x float]* %output_act, i64 0, i64 %indvars.iv.i
  %9 = load float, float* %arrayidx2.i, align 4, !tbaa !2
  %sub.i63 = fsub float %8, %9
  %mul.i64 = fmul float %sub.i63, %sub.i63
  %add.i65 = fadd float %7, %mul.i64
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i66 = icmp eq i64 %indvars.iv.next.i, 35
  br i1 %exitcond.i66, label %update_stats.exit, label %for.body.i

update_stats.exit:                                ; preds = %for.body.i
  %add.i65.lcssa = phi float [ %add.i65, %for.body.i ]
  call void @llvm.lifetime.start.p0i8(i64 124, i8* nonnull %2) #3
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %2, i8 0, i64 124, i1 false) #3
  br label %for.body3.i87

for.body3.i87:                                    ; preds = %for.inc44.i, %update_stats.exit
  %indvars.iv105.i = phi i64 [ 0, %update_stats.exit ], [ %indvars.iv.next106.i, %for.inc44.i ]
  %arrayidx5.i81 = getelementptr inbounds [30 x [35 x float]], [30 x [35 x float]]* @next_output, i64 0, i64 %indvars.iv, i64 %indvars.iv105.i
  %10 = load float, float* %arrayidx5.i81, align 4, !tbaa !2
  %arrayidx7.i = getelementptr inbounds [35 x float], [35 x float]* %output_act, i64 0, i64 %indvars.iv105.i
  %11 = load float, float* %arrayidx7.i, align 4, !tbaa !2
  %sub.i82 = fsub float %10, %11
  %mul.i83 = fmul float %11, %sub.i82
  %conv.i84 = fpext float %mul.i83 to double
  %conv12.i = fpext float %11 to double
  %sub13.i = fsub double 1.000000e+00, %conv12.i
  %mul14.i85 = fmul double %sub13.i, %conv.i84
  %conv15.i86 = fptrunc double %mul14.i85 to float
  br label %for.body21.i

for.body21.i:                                     ; preds = %for.body21.i, %for.body3.i87
  %indvars.iv102.i = phi i64 [ 0, %for.body3.i87 ], [ %indvars.iv.next103.i, %for.body21.i ]
  %arrayidx27.i = getelementptr inbounds [35 x [31 x float]], [35 x [31 x float]]* @h_o_weights, i64 0, i64 %indvars.iv105.i, i64 %indvars.iv102.i
  %12 = load float, float* %arrayidx27.i, align 4, !tbaa !6
  %mul28.i = fmul float %12, %conv15.i86
  %arrayidx30.i88 = getelementptr inbounds [31 x float], [31 x float]* %psum_array.i, i64 0, i64 %indvars.iv102.i
  %13 = load float, float* %arrayidx30.i88, align 4, !tbaa !9
  %add.i89 = fadd float %13, %mul28.i
  store float %add.i89, float* %arrayidx30.i88, align 4, !tbaa !9
  %arrayidx34.i = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 %indvars.iv102.i
  %14 = load float, float* %arrayidx34.i, align 4, !tbaa !2
  %mul35.i = fmul float %14, %conv15.i86
  %arrayidx39.i = getelementptr inbounds [35 x [31 x float]], [35 x [31 x float]]* @h_o_w_ch_sum_array, i64 0, i64 %indvars.iv105.i, i64 %indvars.iv102.i
  %15 = load float, float* %arrayidx39.i, align 4, !tbaa !6
  %add40.i = fadd float %15, %mul35.i
  store float %add40.i, float* %arrayidx39.i, align 4, !tbaa !6
  %indvars.iv.next103.i = add nuw nsw i64 %indvars.iv102.i, 1
  %exitcond104.i = icmp eq i64 %indvars.iv.next103.i, 31
  br i1 %exitcond104.i, label %for.inc44.i, label %for.body21.i

for.inc44.i:                                      ; preds = %for.body21.i
  %indvars.iv.next106.i = add nuw nsw i64 %indvars.iv105.i, 1
  %exitcond107.i = icmp eq i64 %indvars.iv.next106.i, 35
  br i1 %exitcond107.i, label %for.body50.i.preheader, label %for.body3.i87

for.body50.i.preheader:                           ; preds = %for.inc44.i
  br label %for.body50.i

for.body50.i:                                     ; preds = %for.body50.i, %for.body50.i.preheader
  %indvars.iv.i90 = phi i64 [ %indvars.iv.next.i91, %for.body50.i ], [ 0, %for.body50.i.preheader ]
  %arrayidx52.i = getelementptr inbounds [31 x float], [31 x float]* %hidden_act, i64 0, i64 %indvars.iv.i90
  %16 = load float, float* %arrayidx52.i, align 4, !tbaa !2
  %conv53.i = fpext float %16 to double
  %sub57.i = fsub double 1.000000e+00, %conv53.i
  %mul58.i = fmul double %sub57.i, %conv53.i
  %arrayidx60.i = getelementptr inbounds [31 x float], [31 x float]* %psum_array.i, i64 0, i64 %indvars.iv.i90
  %17 = load float, float* %arrayidx60.i, align 4, !tbaa !9
  %conv61.i = fpext float %17 to double
  %mul62.i = fmul double %mul58.i, %conv61.i
  %conv63.i = fptrunc double %mul62.i to float
  %arrayidx65.i = getelementptr inbounds [31 x float], [31 x float]* @hidden_delta, i64 0, i64 %indvars.iv.i90
  store float %conv63.i, float* %arrayidx65.i, align 4, !tbaa !9
  %indvars.iv.next.i91 = add nuw nsw i64 %indvars.iv.i90, 1
  %exitcond.i92 = icmp eq i64 %indvars.iv.next.i91, 31
  br i1 %exitcond.i92, label %output_hidden.exit, label %for.body50.i

output_hidden.exit:                               ; preds = %for.body50.i
  call void @llvm.lifetime.end.p0i8(i64 124, i8* nonnull %2) #3
  br label %for.cond2.preheader.i

for.cond2.preheader.i:                            ; preds = %for.end.i80, %output_hidden.exit
  %w_ch.019.i = phi float* [ getelementptr inbounds ([30 x [1221 x float]], [30 x [1221 x float]]* @i_h_w_ch_sum_array, i64 0, i64 0, i64 0), %output_hidden.exit ], [ %scevgep.i79, %for.end.i80 ]
  %delta.018.i = phi float* [ getelementptr inbounds ([31 x float], [31 x float]* @hidden_delta, i64 0, i64 0), %output_hidden.exit ], [ %incdec.ptr6.i, %for.end.i80 ]
  br label %for.body4.i

for.body4.i:                                      ; preds = %for.body4.i, %for.cond2.preheader.i
  %w_ch.117.i = phi float* [ %w_ch.019.i, %for.cond2.preheader.i ], [ %incdec.ptr5.i, %for.body4.i ]
  %sender.0.idx16.i = phi i64 [ 0, %for.cond2.preheader.i ], [ %sender.0.add.i75, %for.body4.i ]
  %sender.0.ptr.i74 = getelementptr inbounds [30 x [1221 x float]], [30 x [1221 x float]]* @next_input, i64 0, i64 %indvars.iv, i64 %sender.0.idx16.i
  %18 = load float, float* %delta.018.i, align 4, !tbaa !2
  %sender.0.add.i75 = add nuw nsw i64 %sender.0.idx16.i, 1
  %19 = load float, float* %sender.0.ptr.i74, align 4, !tbaa !2
  %mul.i76 = fmul float %18, %19
  %incdec.ptr5.i = getelementptr inbounds float, float* %w_ch.117.i, i64 1
  %20 = load float, float* %w_ch.117.i, align 4, !tbaa !2
  %add.i77 = fadd float %20, %mul.i76
  store float %add.i77, float* %w_ch.117.i, align 4, !tbaa !2
  %exitcond.i78 = icmp eq i64 %sender.0.add.i75, 1221
  br i1 %exitcond.i78, label %for.end.i80, label %for.body4.i

for.end.i80:                                      ; preds = %for.body4.i
  %scevgep.i79 = getelementptr float, float* %w_ch.019.i, i64 1221
  %incdec.ptr6.i = getelementptr inbounds float, float* %delta.018.i, i64 1
  %cmp.i = icmp ugt float* %incdec.ptr6.i, getelementptr inbounds ([31 x float], [31 x float]* @hidden_delta, i64 0, i64 29)
  br i1 %cmp.i, label %hidden_input.exit, label %for.cond2.preheader.i

hidden_input.exit:                                ; preds = %for.end.i80
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 30
  br i1 %exitcond, label %for.end, label %for.body5

for.end:                                          ; preds = %hidden_input.exit
  %add.i65.lcssa.lcssa = phi float [ %add.i65.lcssa, %hidden_input.exit ]
  %21 = load float, float* @i_h_lrc, align 4, !tbaa !2
  br label %for.end30

for.end30:                                        ; preds = %update_weights.exit
  call void @llvm.lifetime.end.p0i8(i64 140, i8* nonnull %1) #3
  call void @llvm.lifetime.end.p0i8(i64 124, i8* nonnull %0) #3
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 12022194aae7a486b5537a51f5eb9d5f116e2ab1) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f5c71dbd2987113e5e86c7972c447c3278769d4c)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA35_A31_f", !8, i64 0}
!8 = !{!"array@_ZTSA31_f", !3, i64 0}
!9 = !{!8, !3, i64 0}
!10 = !{!11, !3, i64 0}
!11 = !{!"array@_ZTSA30_A1221_f", !12, i64 0}
!12 = !{!"array@_ZTSA1221_f", !3, i64 0}
