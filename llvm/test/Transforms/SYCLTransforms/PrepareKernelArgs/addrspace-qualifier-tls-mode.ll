; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-local-buffers,sycl-kernel-prepare-args' -S %s | FileCheck %s
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-local-buffers,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
;
;;; Check whether the %pBufferRanges is generated and is initialized,
;;; when there are address space qualifier builtins.
;
; CHECK: @__pLocalMemBase = internal thread_local global ptr addrspace(3) undef
; CHECK: @__pWorkDim = internal thread_local global ptr undef
; CHECK: @__pWGId = internal thread_local global ptr undef
; CHECK: @__BaseGlbId = internal thread_local global [4 x i64] undef
; CHECK: @__RuntimeHandle = internal thread_local global ptr undef
; CHECK: @__pBufferRanges = internal thread_local global ptr undef

; // Kenrel just has one global arg
; __kernel void test_address_space_qualifier_func1(__global int *src){
;   int tid = get_global_id(0);
;   callee(src, tid);
; }
define dso_local void @test_address_space_qualifier_func1(ptr addrspace(1) %src) !use_addrspace_qualifier_func !1 {
; CHECK-LABEL: define dso_local void @test_address_space_qualifier_func1
; CHECK:         [[PBUFFERRANGES:%.*]] = alloca i64, i64 4, align 8
; CHECK-NEXT:    [[LOCALCOUNT:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 0
; CHECK-NEXT:    store i64 0, ptr [[LOCALCOUNT]], align 4
; CHECK-NEXT:    [[GlobalCount:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 8
; CHECK-NEXT:    store i64 1, ptr [[GlobalCount]], align 4
; CHECK-NEXT:    [[ARGINFO:%.*]] = getelementptr i8, ptr [[UNIFORMARGS:%.*]]
; CHECK-NEXT:    [[BUFFERRANGE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 16
; CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i64(ptr align 1 [[BUFFERRANGE]], ptr align 1 [[ARGINFO]], i64 16, i1 false)
; CHECK-NEXT:    store ptr [[PBUFFERRANGES]], ptr @__pBufferRanges, align 8
;
; CHECK:         call void @callee(ptr addrspace(4) [[TMP3:%.*]], i32 [[CONV:%.*]])
;
entry:
  %call = call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %0 = addrspacecast ptr addrspace(1) %src to ptr addrspace(4)
  call void @callee(ptr addrspace(4) %0, i32 %conv)
  ret void
}

; // Kenrel has one global arg and one local arg
; __kernel void test_address_space_qualifier_func2(__global int *src, __local int *local_src){
;   int tid = get_global_id(0);
;   callee(src, tid);
;   callee(local_src, tid);
; }
define dso_local void @test_address_space_qualifier_func2(ptr addrspace(1) %src, ptr addrspace(3) %local_src) !use_addrspace_qualifier_func !1 {
; CHECK-LABEL: define dso_local void @test_address_space_qualifier_func2
; CHECK:         [[TMP:%.*]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT:    [[LOCALARG_SIZE:%.*]] = load i64, ptr [[TMP]]
; CHECK-NEXT:    [[LOCALARG_ADDR:%.*]] = alloca i8, i64 [[LOCALARG_SIZE]]

; CHECK:         [[PBUFFERRANGES:%.*]] = alloca i64, i64 6, align 8
; CHECK-NEXT:    [[LOCALCOUNT:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 0
; CHECK-NEXT:    store i64 1, ptr [[LOCALCOUNT]], align 4
; CHECK-NEXT:    [[GlobalCount:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 8
; CHECK-NEXT:    store i64 1, ptr [[GlobalCount]], align 4
; CHECK-NEXT:    [[PLOCALARG:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 16
; CHECK-NEXT:    store ptr [[LOCALARG_ADDR]], ptr [[PLOCALARG]], align 8
; CHECK-NEXT:    [[PLOCALARGSIZE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 24
; CHECK-NEXT:    store i64 [[LOCALARG_SIZE]], ptr [[PLOCALARGSIZE]], align 4
; CHECK-NEXT:    [[ARGINFO:%.*]] = getelementptr i8, ptr [[UNIFORMARGS:%.*]]
; CHECK-NEXT:    [[BUFFERRANGE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 32
; CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i64(ptr align 1 [[BUFFERRANGE]], ptr align 1 [[ARGINFO]], i64 16, i1 false)
; CHECK-NEXT:    store ptr [[PBUFFERRANGES]], ptr @__pBufferRanges, align 8
;
; CHECK:         call void @callee(ptr addrspace(4) [[TMP1:%.*]], i32 [[CONV1:%.*]])
; CHECK:         call void @callee(ptr addrspace(4) [[TMP2:%.*]], i32 [[CONV2:%.*]])
;
entry:
  %call = call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %0 = addrspacecast ptr addrspace(1) %src to ptr addrspace(4)
  call void @callee(ptr addrspace(4) %0, i32 %conv)
  %1 = addrspacecast ptr addrspace(3) %local_src to ptr addrspace(4)
  call void @callee(ptr addrspace(4) %1, i32 %conv)
  ret void
}

@test_address_space_qualifier_func3.array = internal addrspace(3) global [16 x i32] undef, align 4

; // Kenrel has one global arg and one local arg and one local array in kernel
; __kernel void test_address_space_qualifier_func3(__global int *src, __local int *local_src){
;   int tid = get_global_id(0);
;   __local int array[16];
;   callee(src, tid);
;   callee(local_src, tid);
;   callee(array, tid);
; }
define dso_local void @test_address_space_qualifier_func3(ptr addrspace(1) %src, ptr addrspace(3) %local_src) !use_addrspace_qualifier_func !1 {
; CHECK-LABEL: define dso_local void @test_address_space_qualifier_func3
; CHECK:         [[TMP:%.*]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT:    [[LOCALARG_SIZE:%.*]] = load i64, ptr [[TMP]]
; CHECK-NEXT:    [[LOCALARG_ADDR:%.*]] = alloca i8, i64 [[LOCALARG_SIZE]]
; CHECK:         [[LOCAL_MEM:%.*]] = alloca [320 x i8]

; CHECK:         [[PBUFFERRANGES:%.*]] = alloca i64, i64 8, align 8
; CHECK-NEXT:    [[LOCALCOUNT:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 0
; CHECK-NEXT:    store i64 2, ptr [[LOCALCOUNT]], align 4
; CHECK-NEXT:    [[GlobalCount:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 8
; CHECK-NEXT:    store i64 1, ptr [[GlobalCount]], align 4
; CHECK-NEXT:    [[LOCAL_MEM_ADDR:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 16
; CHECK-NEXT:    store ptr [[LOCAL_MEM]], ptr [[LOCAL_MEM_ADDR]], align 8
; CHECK-NEXT:    [[LOCAL_MEM_SIZE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 24
; CHECK-NEXT:    store i64 320, ptr [[LOCAL_MEM_SIZE]], align 4
; CHECK-NEXT:    [[PLOCALARG:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 32
; CHECK-NEXT:    store ptr [[LOCALARG_ADDR]], ptr [[PLOCALARG]], align 8
; CHECK-NEXT:    [[PLOCALSIZE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 40
; CHECK-NEXT:    store i64 [[LOCALARG_SIZE]], ptr [[PLOCALSIZE]], align 4
; CHECK-NEXT:    [[ARGINFO:%.*]] = getelementptr i8, ptr [[UNIFORMARGS:%.*]]
; CHECK-NEXT:    [[BUFFERRANGE:%.*]] = getelementptr i8, ptr [[PBUFFERRANGES]], i32 48
; CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i64(ptr align 1 [[BUFFERRANGE]], ptr align 1 [[ARGINFO]], i64 16, i1 false)
; CHECK-NEXT:    store ptr [[PBUFFERRANGES]], ptr @__pBufferRanges, align 8
;
; CHECK:         call void @callee(ptr addrspace(4) [[TMP31:%.*]], i32 [[CONV:%.*]])
; CHECK:         call void @callee(ptr addrspace(4) [[TMP32:%.*]], i32 [[CONV:%.*]])
; CHECK:         call void @callee(ptr addrspace(4) [[TMP33:%.*]], i32 [[CONV:%.*]])
;
entry:
  %call = call i64 @_Z13get_global_idj(i32 0)
  %conv = trunc i64 %call to i32
  %0 = addrspacecast ptr addrspace(1) %src to ptr addrspace(4)
  call void @callee(ptr addrspace(4) %0, i32 %conv)
  %1 = addrspacecast ptr addrspace(3) %local_src to ptr addrspace(4)
  call void @callee(ptr addrspace(4) %1, i32 %conv)
  call void @callee(ptr addrspace(4) addrspacecast (ptr addrspace(3) @test_address_space_qualifier_func3.array to ptr addrspace(4)), i32 %conv)
  ret void
}

;;; Check whether the %pBufferRanges will not be initialized,
;;; when there are not address space qualifier builtins in kernel.

define dso_local void @test_no_address_space_qualifier_func(ptr addrspace(1)  align 4 %src, ptr addrspace(3)  align 4 %local_src) !use_addrspace_qualifier_func !2 {
; CHECK-NOT:    store ptr [[BUFFERINFO:%.*]], ptr @__pBufferRanges
entry:
  %call = call i64 @_Z13get_global_idj(i32  0)
  %conv = trunc i64 %call to i32
  %0 = addrspacecast ptr addrspace(1) %src to ptr addrspace(4)
  call void @callee_no(ptr addrspace(4)  %0, i32  %conv)
  %1 = addrspacecast ptr addrspace(3) %local_src to ptr addrspace(4)
  call void @callee_no(ptr addrspace(4)  %1, i32  %conv)
  ret void
}

; // callee souce code
; void callee(__generic int *src, int tid){
;   __global int* global_ptr = to_global(src);
;   if(global_ptr != NULL)
;      global_ptr[tid] = -tid;
; }
define internal void @callee(ptr addrspace(4) %src, i32 %tid) {
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

define internal void @callee_no(ptr addrspace(4) %src, i32 %tid) #0 {
entry:
  %sub = sub nsw i32 0, %tid
  %idxprom = sext i32 %tid to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %src, i64 %idxprom
  store i32 %sub, ptr addrspace(4) %arrayidx, align 4
  ret void
}

declare ptr addrspace(1) @__to_global(ptr addrspace(4))

declare i64 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_address_space_qualifier_func1, ptr @test_address_space_qualifier_func2, ptr @test_address_space_qualifier_func3, ptr @test_no_address_space_qualifier_func}
!1 = !{i1 true}
!2 = !{i1 false}
!3 = !{i32 4}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-191: WARNING: Instruction with empty DebugLoc in function test{{.*}}
; DEBUGIFY-NOT: WARNING
