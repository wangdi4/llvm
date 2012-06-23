; RUN: llvm-as %s -o %s.bin
; RUN111: SATest -OCL -REF -config=%s.cfg -neat=1
; TODO: add NEATChecker -r %s -a %s.neat -t 0
; TODO: Rewrite test to eliminate pointer bitcast instruction which is not supported by NEAT.

; ModuleID = 'insert-extractvalue_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

%struct._str1 = type { <4 x double>, i32 }

define void @InsertExtractValue(<4 x double> addrspace(1)* %input, <4 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x double> addrspace(1)*, align 4
  %output.addr = alloca <4 x double> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %s = alloca %struct._str1, align 32
  store <4 x double> addrspace(1)* %input, <4 x double> addrspace(1)** %input.addr, align 4
  store <4 x double> addrspace(1)* %output, <4 x double> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  %0 = bitcast %struct._str1* %s to i8*
  call void @llvm.memset.p0i8.i32(i8* %0, i8 0, i32 64, i32 32, i1 false)
  %1 = bitcast i8* %0 to { <4 x double>, i32, [28 x i8] }*
  %2 = getelementptr { <4 x double>, i32, [28 x i8] }* %1, i32 0, i32 0
  store <4 x double> <double 1.000000e+00, double 2.000000e+00, double 3.000000e+00, double 4.000000e+00>, <4 x double>* %2
  %3 = getelementptr { <4 x double>, i32, [28 x i8] }* %1, i32 0, i32 2
  %f4 = getelementptr inbounds %struct._str1* %s, i32 0, i32 0
  %4 = load <4 x double>* %f4, align 32
  %5 = load i32* %tid, align 4
  %6 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx = getelementptr inbounds <4 x double> addrspace(1)* %6, i32 %5
  store <4 x double> %4, <4 x double> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i32)* @InsertExtractValue, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}