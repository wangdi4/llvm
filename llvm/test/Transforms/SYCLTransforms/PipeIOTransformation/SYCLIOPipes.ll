; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-demangle-fpga-pipes -passes='sycl-kernel-rewrite-pipes,sycl-kernel-equalizer,sycl-kernel-pipe-io-transform' %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -sycl-demangle-fpga-pipes -passes='sycl-kernel-rewrite-pipes,sycl-kernel-equalizer,sycl-kernel-pipe-io-transform' %s -S | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

%struct._ZTS19ConstantPipeStorage.ConstantPipeStorage = type { i32, i32, i32 }
%"class._ZTSZZ15test_io_nb_pipeN2cl4sycl5queueEENK3$_0clERNS0_7handlerEEUlvE_.anon" = type { i8 }

; CHECK-DAG: @_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !{{[0-9]+}}, !packet_align !{{[0-9]+}}, !depth !{{[0-9]+}}, !io ![[READ_IO_MD:[0-9]+]]
; CHECK-DAG: @_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
; CHECK-DAG: _ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE9m_StorageE.syclpipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !{{[0-9]+}}, !packet_align !{{[0-9]+}}, !depth !{{[0-9]+}}, !io ![[WRITE_IO_MD:[0-9]+]]
; CHECK-DAG: @_ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE9m_StorageE.syclpipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

@_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 0 }, align 4, !io_pipe_id !0
@_ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE9m_StorageE = available_externally addrspace(1) constant %struct._ZTS19ConstantPipeStorage.ConstantPipeStorage { i32 4, i32 4, i32 0 }, align 4, !io_pipe_id !1

; Function Attrs: nounwind
define spir_kernel void @"_ZTSZZ15test_io_nb_pipeN2cl4sycl5queueEENK3$_0clERNS0_7handlerEE14nb_io_transfer"() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_type_qual !5 !kernel_arg_base_type !5 {
entry:
  %0 = alloca %"class._ZTSZZ15test_io_nb_pipeN2cl4sycl5queueEENK3$_0clERNS0_7handlerEEUlvE_.anon", align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr %0)
  %1 = addrspacecast ptr %0 to ptr addrspace(4)
  call spir_func void @"_ZZZ15test_io_nb_pipeN2cl4sycl5queueEENK3$_0clERNS0_7handlerEENKUlvE_clEv"(ptr addrspace(4) %1) #0
  call void @llvm.lifetime.end.p0(i64 1, ptr %0)
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
define internal spir_func void @"_ZZZ15test_io_nb_pipeN2cl4sycl5queueEENK3$_0clERNS0_7handlerEENKUlvE_clEv"(ptr addrspace(4) %this) #0 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %SuccessCodeI = alloca i8, align 1
  %data = alloca i32, align 4
  %SuccessCodeO = alloca i8, align 1
  store ptr addrspace(4) %this, ptr %this.addr, align 8
  %this1 = load ptr addrspace(4), ptr %this.addr, align 8
  call void @llvm.lifetime.start.p0(i64 1, ptr %SuccessCodeI)
  store i8 0, ptr %SuccessCodeI, align 1
  call void @llvm.lifetime.start.p0(i64 4, ptr %data)
  store i32 0, ptr %data, align 4
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %0 = addrspacecast ptr %SuccessCodeI to ptr addrspace(4)
  %call = call spir_func i32 @_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE4readERb(ptr addrspace(4) dereferenceable(1) %0) #0
  store i32 %call, ptr %data, align 4
  br label %do.cond

do.cond:                                          ; preds = %do.body
  %1 = load i8, ptr %SuccessCodeI, align 1
  %tobool = icmp ne i8 %1, 0
  %lnot = icmp ne i1 %tobool, true
  br i1 %lnot, label %do.body, label %do.end

do.end:                                           ; preds = %do.cond
  call void @llvm.lifetime.start.p0(i64 1, ptr %SuccessCodeO)
  store i8 0, ptr %SuccessCodeO, align 1
  br label %do.body2

do.body2:                                         ; preds = %do.cond3, %do.end
  %2 = addrspacecast ptr %data to ptr addrspace(4)
  %3 = addrspacecast ptr %SuccessCodeO to ptr addrspace(4)
  call spir_func void @_ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE5writeERKiRb(ptr addrspace(4) dereferenceable(4) %2, ptr addrspace(4) dereferenceable(1) %3) #0
  br label %do.cond3

do.cond3:                                         ; preds = %do.body2
  %4 = load i8, ptr %SuccessCodeO, align 1
  %tobool4 = icmp ne i8 %4, 0
  %lnot5 = icmp ne i1 %tobool4, true
  br i1 %lnot5, label %do.body2, label %do.end6

do.end6:                                          ; preds = %do.cond3
  call void @llvm.lifetime.end.p0(i64 1, ptr %SuccessCodeO)
  call void @llvm.lifetime.end.p0(i64 4, ptr %data)
  call void @llvm.lifetime.end.p0(i64 1, ptr %SuccessCodeI)
  ret void
}

; Function Attrs: nounwind
define spir_func i32 @_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE4readERb(ptr addrspace(4) dereferenceable(1) %Success) #0 {
entry:
  %Success.addr = alloca ptr addrspace(4), align 8
  %RPipe = alloca ptr addrspace(1), align 8
  %TempData = alloca i32, align 4
  store ptr addrspace(4) %Success, ptr %Success.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %RPipe)
  %0 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl5intel23kernel_readable_io_pipeIN9intelfpga16ethernet_pipe_idILj0EEEiLm0EE9m_StorageE to ptr addrspace(4)
  %call = call spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %0) #0
  store ptr addrspace(1) %call, ptr %RPipe, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %TempData)
  %1 = load ptr addrspace(1), ptr %RPipe, align 8
  %2 = addrspacecast ptr %TempData to ptr addrspace(4)
  ; CHECK: %{{[0-9]+}} = call i32 @__read_pipe_2_io_fpga(ptr addrspace(1) %{{[0-9]+}}, ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %{{[0-9]+}}, i32 4, i32 4)
  %call1 = call spir_func i32 @__read_pipe_2(ptr addrspace(1) %1, ptr addrspace(4) %2, i32 4, i32 4) #0
  %tobool = icmp ne i32 %call1, 0
  %lnot = icmp ne i1 %tobool, true
  %3 = load ptr addrspace(4), ptr %Success.addr, align 8
  %frombool = select i1 %lnot, i8 1, i8 0
  store i8 %frombool, ptr addrspace(4) %3, align 1
  %4 = load i32, ptr %TempData, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %TempData)
  call void @llvm.lifetime.end.p0(i64 8, ptr %RPipe)
  ret i32 %4
}

; Function Attrs: nounwind
declare spir_func ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4)) #0

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
define spir_func void @_ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE5writeERKiRb(ptr addrspace(4) dereferenceable(4) %Data, ptr addrspace(4) dereferenceable(1) %Success) #0 {
entry:
  %Data.addr = alloca ptr addrspace(4), align 8
  %Success.addr = alloca ptr addrspace(4), align 8
  %WPipe = alloca ptr addrspace(1), align 8
  store ptr addrspace(4) %Data, ptr %Data.addr, align 8
  store ptr addrspace(4) %Success, ptr %Success.addr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %WPipe)
  %0 = addrspacecast ptr addrspace(1) @_ZN2cl4sycl5intel24kernel_writeable_io_pipeIN9intelfpga16ethernet_pipe_idILj1EEEiLm0EE9m_StorageE to ptr addrspace(4)
  %call = call spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4) %0) #0
  store ptr addrspace(1) %call, ptr %WPipe, align 8
  %1 = load ptr addrspace(1), ptr %WPipe, align 8
  %2 = load ptr addrspace(4), ptr %Data.addr, align 8
  ; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_io_fpga(ptr addrspace(1) %{{[0-9]+}}, ptr addrspace(4) %{{[0-9]+}}, ptr addrspace(4) %{{[0-9]+}}, i32 4, i32 4)
  %call1 = call spir_func i32 @__write_pipe_2(ptr addrspace(1) %1, ptr addrspace(4) %2, i32 4, i32 4) #0
  %tobool = icmp ne i32 %call1, 0
  %lnot = icmp ne i1 %tobool, true
  %3 = load ptr addrspace(4), ptr %Success.addr, align 8
  %frombool = select i1 %lnot, i8 1, i8 0
  store i8 %frombool, ptr addrspace(4) %3, align 1
  call void @llvm.lifetime.end.p0(i64 8, ptr %WPipe)
  ret void
}

; Function Attrs: nounwind
declare spir_func ptr addrspace(1) @_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS445_ZTS19ConstantPipeStorage.ConstantPipeStorage(ptr addrspace(4)) #0

; Function Attrs: nounwind
; CHECK: declare i32 @__read_pipe_2_io_fpga(ptr addrspace(1) nocapture, ptr addrspace(4) nocapture, ptr addrspace(4) nocapture readonly, i32, i32)
declare spir_func i32 @__read_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32) #0

; Function Attrs: nounwind
; CHECK: declare i32 @__write_pipe_2_io_fpga(ptr addrspace(1) nocapture, ptr addrspace(4) nocapture, ptr addrspace(4) nocapture readonly, i32, i32)
declare spir_func i32 @__write_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32) #0

attributes #0 = { nounwind }
attributes #1 = { argmemonly nounwind willreturn }

!opencl.enable.FP_CONTRACT = !{}
!spirv.Source = !{!2}
!opencl.spir.version = !{!3}
!opencl.ocl.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!spirv.Generator = !{!6}

; CHECK-DAG: ![[READ_IO_MD]] = !{!"{{.*}}"}
; CHECK-DAG: ![[WRITE_IO_MD]] = !{!"{{.*}}"}

!0 = !{i32 0}
!1 = !{i32 1}
!2 = !{i32 4, i32 100000}
!3 = !{i32 1, i32 2}
!4 = !{i32 1, i32 0}
!5 = !{}
!6 = !{i16 6, i16 14}

; DEBUGIFY-NOT: WARNING: Missing line
