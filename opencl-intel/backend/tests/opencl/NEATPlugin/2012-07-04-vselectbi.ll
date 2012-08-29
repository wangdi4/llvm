; RUN: llvm-as %s -o %s.bin
; RUN SATest -OCL -VAL -tsize=0 -config=%s.cfg --neat=1 --single_wg=1 --force_ref=0 -build-iterations=1 -execute-iterations=1

; ModuleID = '2012-07-04-vselectbi.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @test_vselectbi(<4 x i8> addrspace(1)* %out, <4 x i8> addrspace(1)* %in1, <4 x i8> addrspace(1)* %in2, <4 x i8> addrspace(1)* %in3) nounwind {
entry:
  %out.addr = alloca <4 x i8> addrspace(1)*, align 4
  %in1.addr = alloca <4 x i8> addrspace(1)*, align 4
  %in2.addr = alloca <4 x i8> addrspace(1)*, align 4
  %in3.addr = alloca <4 x i8> addrspace(1)*, align 4
  %index = alloca i32, align 4
  store <4 x i8> addrspace(1)* %out, <4 x i8> addrspace(1)** %out.addr, align 4
  store <4 x i8> addrspace(1)* %in1, <4 x i8> addrspace(1)** %in1.addr, align 4
  store <4 x i8> addrspace(1)* %in2, <4 x i8> addrspace(1)** %in2.addr, align 4
  store <4 x i8> addrspace(1)* %in3, <4 x i8> addrspace(1)** %in3.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %index, align 4
  %0 = load i32* %index, align 4
  %1 = load <4 x i8> addrspace(1)** %in1.addr, align 4
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %1, i32 %0
  %2 = load <4 x i8> addrspace(1)* %arrayidx
  %3 = load i32* %index, align 4
  %4 = load <4 x i8> addrspace(1)** %in2.addr, align 4
  %arrayidx1 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i32 %3
  %5 = load <4 x i8> addrspace(1)* %arrayidx1
  %6 = load i32* %index, align 4
  %7 = load <4 x i8> addrspace(1)** %in3.addr, align 4
  %arrayidx2 = getelementptr inbounds <4 x i8> addrspace(1)* %7, i32 %6
  %8 = load <4 x i8> addrspace(1)* %arrayidx2
  %call3 = call <4 x i8> @_Z6selectDv4_cS_S_(<4 x i8> %2, <4 x i8> %5, <4 x i8> %8) nounwind readnone
  %9 = load i32* %index, align 4
  %10 = load <4 x i8> addrspace(1)** %out.addr, align 4
  %arrayidx4 = getelementptr inbounds <4 x i8> addrspace(1)* %10, i32 %9
  store <4 x i8> %call3, <4 x i8> addrspace(1)* %arrayidx4
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

declare <4 x i8> @_Z6selectDv4_cS_S_(<4 x i8>, <4 x i8>, <4 x i8>) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*)* @test_vselectbi, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
