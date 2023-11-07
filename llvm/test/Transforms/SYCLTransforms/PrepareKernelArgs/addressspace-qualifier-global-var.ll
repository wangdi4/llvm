; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers,sycl-kernel-prepare-args' -S %s | FileCheck %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

@gint = addrspace(1) global i32 1, align 4
@gint_array = addrspace(1) global [4 x i32] [i32 1, i32 2, i32 3, i32 4], align 4

define void @callee(ptr addrspace(4) %src, i32 %tid) {
; CHECK-LABEL: define void @callee
; CHECK-SAME:  ptr noalias [[PBUFFERRANGES:%.*]]
;
entry:
  %0 = call ptr addrspace(1) @__to_global(ptr addrspace(4) %src)
  %cmp.not = icmp eq ptr addrspace(1) %0, null
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %sub = sub nsw i32 0, %tid
  %idxprom = sext i32 %tid to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %idxprom
  store i32 %sub, ptr addrspace(1) %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

define dso_local void @test_address_space_qualifier_func1() !use_addrspace_qualifier_func !1 {
; CHECK-LABEL: define dso_local void @test_address_space_qualifier_func1
;
; CHECK:       [[PBUFFERRANGES:%.*]] = alloca i64, i64 6, align 8
; CHECK-NEXT:  [[PLOCAL_COUNT:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 0
; CHECK-NEXT:  store i64 0, ptr [[PLOCAL_COUNT]], align 4
; CHECK-NEXT:  [[PGLOBAL_COUNT:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 8
; CHECK-NEXT:  store i64 2, ptr [[PGLOBAL_COUNT]], align 4
; CHECK-NEXT:  [[PGLOBAL_VAR1_ADDR:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 16
; CHECK-NEXT:  store ptr addrspace(1) @gint, ptr [[PGLOBAL_VAR1_ADDR]], align 8
; CHECK-NEXT:  [[PGLOBAL_VAR1_SIZE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 24
; CHECK-NEXT:  store i64 4, ptr [[PGLOBAL_VAR1_SIZE]], align 4
; CHECK-NEXT:  [[PGLOBAL_VAR2_ADDR:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 32
; CHECK-NEXT:  store ptr addrspace(1) @gint_array, ptr [[PGLOBAL_VAR2_ADDR]], align 8
; CHECK-NEXT:  [[PGLOBAL_VAR2_SIZE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 40
; CHECK-NEXT:  store i64 16, ptr [[PGLOBAL_VAR2_SIZE]], align 4

entry:
  %tid = alloca i32, align 4
  %call = call spir_func i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  store i32 %conv, ptr %tid, align 4
  %0 = load i32, ptr %tid, align 4
  call spir_func void @callee(ptr addrspace(4) addrspacecast (ptr addrspace(1) @gint to ptr addrspace(4)), i32 %0)
  %1 = load i32, ptr %tid, align 4
  call spir_func void @callee(ptr addrspace(4) addrspacecast (ptr addrspace(1) @gint_array to ptr addrspace(4)), i32 %1)
  ret void
}

declare ptr addrspace(1) @__to_global(ptr addrspace(4))

declare i64 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_address_space_qualifier_func1}
!1 = !{i1 true}
!2 = !{i1 false}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-41: WARNING: Instruction with empty DebugLoc in function test_address_space_qualifier_func1
; DEBUGIFY-NOT: WARNING