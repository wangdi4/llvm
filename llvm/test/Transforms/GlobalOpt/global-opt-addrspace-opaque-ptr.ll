; RUN: opt -S -passes=globalopt < %s | FileCheck %s
;; Check that Global Opt preserves address space of llvm.used and
;; llvm.compiler.used variables.

; ModuleID = 'global-opt-addrspace.ll'
source_filename = "device_global_internal_failure.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

%struct.FakeDeviceGlobal = type { ptr addrspace(4) }
%class.anon = type { i8 }

$_ZTSZ4mainEUlvE_ = comdat any

@_ZL1C = internal addrspace(1) global %struct.FakeDeviceGlobal zeroinitializer, align 8 #0
@llvm.compiler.used = appending global [1 x ptr addrspace(4)] [ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZL1C to ptr addrspace(4))], section "llvm.metadata"

; CHECK: @llvm.compiler.used = appending global [1 x ptr addrspace(4)] [ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZL1C to ptr addrspace(4))], section "llvm.metadata"

; Function Attrs: convergent mustprogress noinline norecurse optnone
define weak_odr dso_local spir_kernel void @_ZTSZ4mainEUlvE_() #1 comdat !kernel_arg_buffer_location !5 {
entry:
  %0 = alloca %class.anon, align 1
  %1 = addrspacecast ptr %0 to ptr addrspace(4)
  call spir_func void @_ZZ4mainENKUlvE_clEv(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %1) #3
  ret void
}

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define internal spir_func void @_ZZ4mainENKUlvE_clEv(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %this) #2 align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %0 = load ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZL1C to ptr addrspace(4)), align 8
  store i32 42, ptr addrspace(4) %0, align 4
  ret void
}

declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2Kcz(ptr addrspace(2), ...)

attributes #0 = { "sycl-unique-id"="c5b4ae0b52852573____ZL1C" }
attributes #1 = { convergent mustprogress noinline norecurse optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "sycl-module-id"="device_global_internal_failure.cpp" "uniform-work-group-size"="true" }
attributes #2 = { convergent mustprogress noinline norecurse nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent }

!llvm.module.flags = !{!0, !1}
!opencl.spir.version = !{!2}
!spirv.Source = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"frame-pointer", i32 2}
!2 = !{i32 1, i32 2}
!3 = !{i32 4, i32 100000}
!4 = !{!"clang version 15.0.0 (https://github.com/intel/llvm.git 537e51b08b74438ed64eb7de6e254922e8ddeaa0)"}
!5 = !{}
