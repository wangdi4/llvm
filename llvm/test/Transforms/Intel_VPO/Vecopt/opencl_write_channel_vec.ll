; Note: Test is explicitly disabled for VPValue-based CG as the feature is not uplifted yet from
; IR-based CG.
; RUN: opt %s -S -VPlanDriver -vplan-force-vf=4 -enable-vp-value-codegen=false 2>&1 | FileCheck %s


; CHECK: __write_pipe_2_bl_fpga_v4i8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%opencl.channel_t = type opaque
%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, [56 x i8], i32, [60 x i8], i32, [60 x i8], %struct.__pipe_internal_buf, [52 x i8], %struct.__pipe_internal_buf, [52 x i8] }
%struct.__pipe_internal_buf = type { i32, i32, i32 }
%struct.labelled_component = type { i32, i16, i16, i32, i8, i8 }

@chan_0 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 1
@chan_1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@pipe.chan_0 = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4
@pipe.chan_1 = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4
@pipe.chan_0.bs = common addrspace(1) global [1049152 x i8] zeroinitializer, align 64
@pipe.chan_1.bs = common addrspace(1) global [16781632 x i8] zeroinitializer, align 64

; Function Attrs: nounwind
define void @src_kernel(i8 addrspace(1)* noalias %input, i16 zeroext %image_width, i16 zeroext %image_height) #0 {
entry:
  %write.src = alloca i8, align 1
  %conv = zext i16 %image_height to i32
  %cmp4 = icmp eq i16 %image_height, 0
  br i1 %cmp4, label %for.end, label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %i.06 = phi i32 [ %inc13, %for.inc ], [ 0, %entry ]
  %pointer.05 = phi i32 [ %pointer.2, %for.inc ], [ 0, %entry ]
  %conv2 = zext i16 %image_width to i32
  %cmp6 = icmp eq i16 %image_width, 0
  br i1 %cmp6, label %for.inc, label %omp.precond.then

omp.precond.then:                                 ; preds = %for.body
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %.omp.iv.03 = phi i32 [ %add12, %omp.inner.for.body ], [ 0, %omp.precond.then ]
  %pointer.12 = phi i32 [ %inc, %omp.inner.for.body ], [ %pointer.05, %omp.precond.then ]
  %idxprom = sext i32 %pointer.12 to i64
  %arrayidx = getelementptr inbounds i8, i8 addrspace(1)* %input, i64 %idxprom
  %0 = load i8, i8 addrspace(1)* %arrayidx, align 1
  store i8 %0, i8* %write.src, align 1
  %1 = load %struct.__pipe_t addrspace(1)*, %struct.__pipe_t addrspace(1)* addrspace(1)* bitcast (%opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.chan_0 to %struct.__pipe_t addrspace(1)* addrspace(1)*), align 8
  %2 = addrspacecast i8* %write.src to i8 addrspace(4)*
  %3 = call i32 @__write_pipe_2_bl_fpga(%struct.__pipe_t addrspace(1)* %1, i8 addrspace(4)* %2)
  %inc = add nsw i32 %pointer.12, 1
  %add12 = add nsw i32 %.omp.iv.03, 1
  %cmp9 = icmp slt i32 %add12, %conv2
  br i1 %cmp9, label %omp.inner.for.body, label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %inc.lcssa = phi i32 [ %inc, %omp.inner.for.body ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.inc

for.inc:                                          ; preds = %DIR.OMP.END.SIMD.1, %for.body
  %pointer.2 = phi i32 [ %inc.lcssa, %DIR.OMP.END.SIMD.1 ], [ %pointer.05, %for.body ]
  %inc13 = add nsw i32 %i.06, 1
  %cmp = icmp slt i32 %inc13, %conv
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc, %entry
  ret void
}

; Function Attrs: alwaysinline nounwind
declare i32 @__read_pipe_2_bl_fpga(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #5

; Function Attrs: alwaysinline nounwind
declare i32 @__write_pipe_2_bl_fpga(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*) #5

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
