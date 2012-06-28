; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; TODO: add NEATChecker -r %s -a %s.neat -t 1200000000

; ModuleID = 'BinaryOperator_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @BinaryKernel(double addrspace(1)* %input, double addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca double addrspace(1)*, align 4
  %output.addr = alloca double addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  store double addrspace(1)* %input, double addrspace(1)** %input.addr, align 4
  store double addrspace(1)* %output, double addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load double addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds double addrspace(1)* %1, i32 %0
  %2 = load double addrspace(1)* %arrayidx
  %add = fadd double %2, 1.000000e+00
  %3 = load i32* %tid, align 4
  %4 = load double addrspace(1)** %output.addr, align 4
  %arrayidx1 = getelementptr inbounds double addrspace(1)* %4, i32 %3
  store double %add, double addrspace(1)* %arrayidx1
  %5 = load i32* %tid, align 4
  %6 = load double addrspace(1)** %output.addr, align 4
  %arrayidx2 = getelementptr inbounds double addrspace(1)* %6, i32 %5
  %7 = load double addrspace(1)* %arrayidx2
  %div = fdiv double %7, 5.000000e+00
  %8 = load i32* %tid, align 4
  %9 = load double addrspace(1)** %output.addr, align 4
  %arrayidx3 = getelementptr inbounds double addrspace(1)* %9, i32 %8
  store double %div, double addrspace(1)* %arrayidx3
  %10 = load i32* %tid, align 4
  %11 = load double addrspace(1)** %output.addr, align 4
  %arrayidx4 = getelementptr inbounds double addrspace(1)* %11, i32 %10
  %12 = load double addrspace(1)* %arrayidx4
  %sub = fsub double 2.000000e+00, %12
  %13 = load i32* %tid, align 4
  %14 = load double addrspace(1)** %output.addr, align 4
  %arrayidx5 = getelementptr inbounds double addrspace(1)* %14, i32 %13
  store double %sub, double addrspace(1)* %arrayidx5
  %15 = load i32* %tid, align 4
  %16 = load double addrspace(1)** %output.addr, align 4
  %arrayidx6 = getelementptr inbounds double addrspace(1)* %16, i32 %15
  %17 = load double addrspace(1)* %arrayidx6
  %18 = load i32* %tid, align 4
  %19 = load double addrspace(1)** %input.addr, align 4
  %arrayidx7 = getelementptr inbounds double addrspace(1)* %19, i32 %18
  %20 = load double addrspace(1)* %arrayidx7
  %mul = fmul double %17, %20
  %21 = load i32* %tid, align 4
  %22 = load double addrspace(1)** %output.addr, align 4
  %arrayidx8 = getelementptr inbounds double addrspace(1)* %22, i32 %21
  store double %mul, double addrspace(1)* %arrayidx8
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32)* @BinaryKernel, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
