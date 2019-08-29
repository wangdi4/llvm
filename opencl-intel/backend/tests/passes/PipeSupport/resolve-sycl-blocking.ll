; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -sycl-pipes-hack -demangle-fpga-pipes -llvm-equalizer -verify -S %s -o - | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage = type { i32, i32, i32 }
%opencl.pipe_wo_t = type opaque
%opencl.pipe_ro_t = type opaque

@_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

@"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

@"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

@_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

@_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 0 }, align 4

@_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 1 }, align 4

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE5writeERKiRb(i32 addrspace(4)* dereferenceable(4), i8 addrspace(4)* dereferenceable(1)) #0 {
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  call spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %4, i8 addrspace(4)* %5, i32 4, i32 4)
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret void
}

; Function Attrs: nounwind
declare spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*) #0

; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE4readERb(i8 addrspace(4)* dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE9some_pipe_0iLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0

  %5 = bitcast i32* %2 to i8*
  %6 = addrspacecast i32* %2 to i32 addrspace(4)*
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  call spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %4, i8 addrspace(4)* %7, i32 4, i32 4)
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret i32 1
}

declare spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*) #0

; Function Attrs: nounwind
define internal spir_func void @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE5writeERKiRb"(i32 addrspace(4)* dereferenceable(4), i8 addrspace(4)* dereferenceable(1)) #0 {
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_2clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  call spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %4, i8 addrspace(4)* %5, i32 4, i32 4)
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret void
}

; Function Attrs: nounwind
define internal spir_func i32 @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE4readERb"(i8 addrspace(4)* dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @"_ZN2cl4sycl4pipeIZZZ4mainENK3$_3clERNS0_7handlerEENKUlNS0_2idILi1EEEE_clES6_E9some_pipeiLi1EE9m_StorageE" to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32* %2 to i8*
  %6 = addrspacecast i32* %2 to i32 addrspace(4)*
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  call spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %4, i8 addrspace(4)* %7, i32 4, i32 4)
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret i32 1
}

; Function Attrs: nounwind
define spir_func void @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE5writeERKiRb(i32 addrspace(4)* dereferenceable(4), i8 addrspace(4)* dereferenceable(1)) #0 {
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  call spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %4, i8 addrspace(4)* %5, i32 4, i32 4)
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret void
}

; Function Attrs: nounwind
define spir_func i32 @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE4readERb(i8 addrspace(4)* dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIN4some4pipeEiLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32* %2 to i8*
  %6 = addrspacecast i32* %2 to i32 addrspace(4)*
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  call spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %4, i8 addrspace(4)* %7, i32 4, i32 4)
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret i32 1
}

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE5writeERKiRb(i32 addrspace(4)* dereferenceable(4), i8 addrspace(4)* dereferenceable(1)) #0 {
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  call spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %4, i8 addrspace(4)* %5, i32 4, i32 4)
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret void
}

; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE4readERb(i8 addrspace(4)* dereferenceable(1)) #0 {
  %2 = alloca i32, align 4
  %3 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE21pipe_type_for_lambdasiLi0EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %4 = call spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %3) #0
  %5 = bitcast i32* %2 to i8*
  %6 = addrspacecast i32* %2 to i32 addrspace(4)*
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  call spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %4, i8 addrspace(4)* %7, i32 4, i32 4)
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret i32 1
}

; Function Attrs: nounwind
define internal spir_func void @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE5writeERKi(i32 addrspace(4)* dereferenceable(4)) #0 {
  %2 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %3 = call spir_func %opencl.pipe_wo_t addrspace(1)* @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %2) #0
  %4 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  call spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
; CHECK: call i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret void
}


; Function Attrs: nounwind
define internal spir_func i32 @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE4readEv() #0 {
  %1 = alloca i32, align 4
  %2 = addrspacecast %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(1)* @_ZN2cl4sycl4pipeIZ4mainE9some_pipeiLi1EE9m_StorageE to %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)*
  %3 = call spir_func %opencl.pipe_ro_t addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage addrspace(4)* %2) #0
  %4 = bitcast i32* %1 to i8*
  %5 = addrspacecast i32* %1 to i32 addrspace(4)*
  %6 = bitcast i32 addrspace(4)* %5 to i8 addrspace(4)*
  call spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %6, i32 4, i32 4)
; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
  ret i32 1
}

; Function Attrs: nounwind
declare spir_func void @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32) #0
; CHECK: declare i32 @__write_pipe_2_bl_fpga(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)* nocapture readonly, i32, i32)

; Function Attrs: nounwind
declare spir_func void @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32) #0
; CHECK: declare i32 @__read_pipe_2_bl_fpga(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32)

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
