; RUN: opt -passes=sycl-kernel-analysis -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-analysis -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefix=CHECK-DEBUG

; Check that use_addrspace_qualifier_func attribute assigned to the kernel attribute.

; CHECK-DEBUG: SYCLKernelAnalysisPass
; CHECK-DEBUG: Kernel <test_address_space_qualifier_func1>:
; CHECK-DEBUG:   UseAddrSpaceQualifierFunc=1
; CHECK-DEBUG: Kernel <test_no_address_space_qualifier_func>:
; CHECK-DEBUG-NOT:   UseAddrSpaceQualifierFunc=0

define internal void @callee(ptr addrspace(1) %src, i32 %tid) {
entry:
  %0 = addrspacecast ptr addrspace(1) %src to ptr addrspace(4)
  %1 = call ptr addrspace(1) @__to_global(ptr addrspace(4) %0)
  %sub = sub nsw i32 0, %tid
  %idxprom = sext i32 %tid to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 %idxprom
  store i32 %sub, ptr addrspace(1) %arrayidx, align 4
  ret void
}

declare ptr addrspace(1) @__to_global(ptr addrspace(4))

; CHECK-LABEL: @test_address_space_qualifier_func1
; CHECK-SAME: !use_addrspace_qualifier_func [[ASMD_TRUE:![0-9]+]]
define dso_local void @test_address_space_qualifier_func1(ptr addrspace(1) %src){
entry:
  %call = call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  call void @callee(ptr addrspace(1) %src, i32 %conv)
  ret void
}

; CHECK-NOT: !use_addrspace_qualifier_func
define dso_local void @test_no_address_space_qualifier_func(ptr addrspace(1) %src){
entry:
  %call = call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  ret void
}

declare i64 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

; CHECK-DAG: [[ASMD_TRUE]] = !{i1 true}
!0 = !{ptr @test_address_space_qualifier_func1, ptr @test_no_address_space_qualifier_func}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
