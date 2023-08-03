; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-rewrite-pipes -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-rewrite-pipes -S %s -o - | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct.__spirv_ConstantPipeStorage = type { i32, i32, i32 }
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET:[0-9]+]], !packet_align ![[PACKET]], !depth ![[DEPTH1:[0-9]+]], !io ![[IO:[0-9]+]]
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET]], !packet_align ![[PACKET]], !depth ![[DEPTH1]], !io ![[IO]]
; CHECK-DAG: @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe" = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET]], !packet_align ![[PACKET]], !depth ![[DEPTH1]], !io ![[IO]]
; CHECK-DAG: @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe.bs" = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe" = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET]], !packet_align ![[PACKET]], !depth ![[DEPTH1]], !io ![[IO]]
; CHECK-DAG: @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe.bs" = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET]], !packet_align ![[PACKET]], !depth ![[DEPTH2:[0-9]+]], !io ![[IO]]
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size ![[PACKET]], !packet_align ![[PACKET]], !depth ![[DEPTH1]], !io ![[IO]]
; CHECK-DAG: @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

@_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4
@"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4
@"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4
@_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4
@_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 0 }, align 4
@_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE = available_externally addrspace(1) constant %struct.__spirv_ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE5writeERKiRb(ptr addrspace(4) dereferenceable(4), ptr addrspace(4) dereferenceable(1)) #0 {
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST1:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST1]])
  ret void
}

; Function Attrs: nounwind
declare spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4)) #0

; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE4readERb(ptr addrspace(4) dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST2:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %3)
  ret i32 0
}

declare spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4)) #0

; Function Attrs: nounwind
define internal spir_func void @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE5writeERKiRb"(ptr addrspace(4) dereferenceable(4), ptr addrspace(4) dereferenceable(1)) #0 {
  %3 = addrspacecast ptr addrspace(1) @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST3:[0-9]+]] = addrspacecast ptr addrspace(1) @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe" to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST3]])
  ret void
}

; Function Attrs: nounwind
define internal spir_func i32 @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE4readERb"(ptr addrspace(4) dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast ptr addrspace(1) @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST4:[0-9]+]] = addrspacecast ptr addrspace(1) @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE.syclpipe" to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST4]])
  ret i32 0
}

; Function Attrs: nounwind
define spir_func void @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE5writeERKiRb(ptr addrspace(4) dereferenceable(4), ptr addrspace(4) dereferenceable(1)) #0 {
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST5:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST5]])
  ret void
}

; Function Attrs: nounwind
define spir_func i32 @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE4readERb(ptr addrspace(4) dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST6:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST6]])
  ret i32 0
}

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE5writeERKiRb(ptr addrspace(4) dereferenceable(4), ptr addrspace(4) dereferenceable(1)) #0 {
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST7:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST7]])
  ret void
}

; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE4readERb(ptr addrspace(4) dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE to ptr addrspace(4)
  %4 = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %3) #0
; CHECK: %[[CAST8:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST8]])
  ret i32 0
}

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE5writeERKi(ptr addrspace(4) dereferenceable(4)) #0 {
  %2 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE to ptr addrspace(4)
  %3 = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %2) #0
; CHECK: %[[CAST9:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST9]])
  ret void
}


; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE4readEv() #0 {
  %1 = alloca i32, align 4
  %2 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE to ptr addrspace(4)
  %3 = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %2) #0
; CHECK: %[[CAST10:[0-9]+]] = addrspacecast ptr addrspace(1) @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE.syclpipe to ptr addrspace(4)
; CHECK: call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427__spirv_ConstantPipeStorage(ptr addrspace(4) %[[CAST10]])
  ret i32 0
}

; CHECK-DAG: ![[PACKET]] = !{i32 4}
; CHECK-DAG: ![[DEPTH1]] = !{i32 1}
; CHECK-DAG: ![[IO]] = !{!""}
; CHECK-DAG: ![[DEPTH2]] = !{i32 0}

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind }
attributes #2 = { alwaysinline nounwind }
attributes #3 = { nounwind readnone }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}

!0 = !{i32 4, i32 100000}
!1 = !{i32 1, i32 2}
!2 = !{i32 1, i32 0}
!3 = !{}
!4 = !{i16 6, i16 14}
!5 = !{i32 1, i32 0, i32 0, i32 0}
!6 = !{!"none", !"none", !"none", !"none"}
!7 = !{!"int*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>"}
!8 = !{!"", !"", !"", !""}
!9 = !{!"int*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"struct _ZTSN2cl4sycl2idILi1EEE.cl::sycl::id"}

; DEBUGIFY: PASS
