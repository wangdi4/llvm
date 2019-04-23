;; Compiled from:
;; __kernel void test_ivdep() {
;;   int a[10];
;;   #pragma ivdep
;;   for (int i = 0; i != 10; ++i)
;;     a[i] = 0;
;;   #pragma ivdep safelen(2)
;;   for (int i = 0; i != 10; ++i)
;;     a[i] = 0;
;;   #pragma ii(2)
;;   for (int i = 0; i != 10; ++i)
;;     a[i] = 0;
;;   #pragma max_concurrency(2)
;;   for (int i = 0; i != 10; ++i)
;;     a[i] = 0;
;; }
;;
;; Command: clang -cc1 -triple spir64-unknown-unknown-intelfpga LoopIvdep.cl -emit-llvm -o LoopIvdep.ll

; RUN: llvm-as < %s > %t.bc
; RUN: llvm-spirv %t.bc -o - -spirv-text | FileCheck %s --check-prefix=CHECK-SPIRV

; ModuleID = '/localdisk2/sidorovd/xmain/ics-ws/xmain/TC/ivdep.cl'
source_filename = "/localdisk2/sidorovd/xmain/ics-ws/xmain/TC/ivdep.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

; CHECK-SPIRV: Function
; Function Attrs: convergent noinline nounwind optnone
define spir_kernel void @test_ivdep() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 {
entry:
  %a = alloca [10 x i32], align 4
  %i = alloca i32, align 4
  %i1 = alloca i32, align 4
  %i10 = alloca i32, align 4
  %i19 = alloca i32, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond
; CHECK-SPIRV: 4 LoopMerge {{[0-9]+}} {{[0-9]+}} 4
; CHECK-SPIRV: 4 BranchConditional {{[0-9]+}} {{[0-9]+}} {{[0-9]+}}
for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp ne i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32, i32* %i, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i1, align 4
  br label %for.cond2

; CHECK-SPIRV: 5 LoopMerge {{[0-9]+}} {{[0-9]+}} 8 2
; CHECK-SPIRV: 4 BranchConditional {{[0-9]+}} {{[0-9]+}} {{[0-9]+}}
for.cond2:                                        ; preds = %for.inc7, %for.end
  %3 = load i32, i32* %i1, align 4
  %cmp3 = icmp ne i32 %3, 10
  br i1 %cmp3, label %for.body4, label %for.end9

for.body4:                                        ; preds = %for.cond2
  %4 = load i32, i32* %i1, align 4
  %idxprom5 = sext i32 %4 to i64
  %arrayidx6 = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 %idxprom5
  store i32 0, i32* %arrayidx6, align 4
  br label %for.inc7

for.inc7:                                         ; preds = %for.body4
  %5 = load i32, i32* %i1, align 4
  %inc8 = add nsw i32 %5, 1
  store i32 %inc8, i32* %i1, align 4
  br label %for.cond2, !llvm.loop !6

for.end9:                                         ; preds = %for.cond2
  store i32 0, i32* %i10, align 4
  br label %for.cond11

; CHECK-SPIRV: 5 LoopMerge {{[0-9]+}} {{[0-9]+}} 5889 2
; CHECK-SPIRV: 4 BranchConditional {{[0-9]+}} {{[0-9]+}} {{[0-9]+}}
for.cond11:                                       ; preds = %for.inc16, %for.end9
  %6 = load i32, i32* %i10, align 4
  %cmp12 = icmp ne i32 %6, 10
  br i1 %cmp12, label %for.body13, label %for.end18

for.body13:                                       ; preds = %for.cond11
  %7 = load i32, i32* %i10, align 4
  %idxprom14 = sext i32 %7 to i64
  %arrayidx15 = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 %idxprom14
  store i32 0, i32* %arrayidx15, align 4
  br label %for.inc16

for.inc16:                                        ; preds = %for.body13
  %8 = load i32, i32* %i10, align 4
  %inc17 = add nsw i32 %8, 1
  store i32 %inc17, i32* %i10, align 4
  br label %for.cond11, !llvm.loop !8

for.end18:                                        ; preds = %for.cond11
  store i32 0, i32* %i19, align 4
  br label %for.cond20

; CHECK-SPIRV: 5 LoopMerge {{[0-9]+}} {{[0-9]+}} 5890 2
; CHECK-SPIRV: 4 BranchConditional {{[0-9]+}} {{[0-9]+}} {{[0-9]+}}
for.cond20:                                       ; preds = %for.inc25, %for.end18
  %9 = load i32, i32* %i19, align 4
  %cmp21 = icmp ne i32 %9, 10
  br i1 %cmp21, label %for.body22, label %for.end27

for.body22:                                       ; preds = %for.cond20
  %10 = load i32, i32* %i19, align 4
  %idxprom23 = sext i32 %10 to i64
  %arrayidx24 = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 %idxprom23
  store i32 0, i32* %arrayidx24, align 4
  br label %for.inc25

for.inc25:                                        ; preds = %for.body22
  %11 = load i32, i32* %i19, align 4
  %inc26 = add nsw i32 %11, 1
  store i32 %inc26, i32* %i19, align 4
  br label %for.cond20, !llvm.loop !10

for.end27:                                        ; preds = %for.cond20
  ret void
}

attributes #0 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.ivdep.enable"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.ivdep.safelen", i32 2}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.ii.count", i32 2}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.max_concurrency.count", i32 2}
