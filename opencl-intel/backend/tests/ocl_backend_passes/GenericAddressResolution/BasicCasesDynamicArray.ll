; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test1
; CHECK-NOT: %call = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %0)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %0 to i8 addrspace(3)*
; CHECK-NOT: %call1 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %2)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %2 to i8 addrspace(1)*
; CHECK: ret

; CHECK: @test2
; CHECK-NOT: %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %0)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %0 to i8 addrspace(1)*
; CHECK: ret

; CHECK: @func
; CHECK-NOT: %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %4)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %4 to i8 addrspace(1)*
; CHECK-NOT: %call9 = call i8* @__to_private(i8 addrspace(4)* %9)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %9 to i8*
; CHECK-NOT: %call11 = call i8* @__to_private(i8 addrspace(4)* %13)
; CHECK: %ToNamedPtr2 = addrspacecast i8 addrspace(4)* %13 to i8*
; CHECK-NOT: %call13 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %18)
; CHECK-NOT: %call31 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr30)
; CHECK: %AddrSpace = addrspacecast float addrspace(4)* %add.ptr30 to float addrspace(1)*
; CHECK: %32 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %AddrSpace)
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

; ModuleID = 'BasicCasesDynamicArrayTmp.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @test1(i32 addrspace(4)* %p) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %0)
  %1 = bitcast i8 addrspace(3)* %call to i32 addrspace(3)*
  %2 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call1 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %2)
  %3 = bitcast i8 addrspace(1)* %call1 to i32 addrspace(1)*
  ret void
}

declare i8 addrspace(3)* @__to_local(i8 addrspace(4)*)

declare i8 addrspace(1)* @__to_global(i8 addrspace(4)*)

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p1 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %0)
  %tobool = icmp ne i8 addrspace(1)* %call, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 addrspace(4)* [ %p1, %if.then ], [ %p2, %if.end ]
  ret i32 addrspace(4)* %retval.0
}

define void @func(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %ptrs = alloca [10 x i32 addrspace(4)*], align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %rem = srem i32 %i.0, 2
  %tobool = icmp ne i32 %rem, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %add.ptr = getelementptr inbounds i32, i32 addrspace(1)* %pGlobal, i32 %i.0
  %0 = addrspacecast i32 addrspace(1)* %add.ptr to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add.ptr1 = getelementptr inbounds i32, i32 addrspace(3)* %pLocal, i32 %i.0
  %1 = addrspacecast i32 addrspace(3)* %add.ptr1 to i32 addrspace(4)*
  %arrayidx2 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %arrayidx3 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %2 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx3, align 4
  store i32 %i.0, i32 addrspace(4)* %2, align 4
  %arrayidx4 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %3 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx4, align 4
  %4 = bitcast i32 addrspace(4)* %3 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %4)
  %5 = bitcast i8 addrspace(1)* %call to i32 addrspace(1)*
  %arrayidx5 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %6 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx5, align 4
  call void @test1(i32 addrspace(4)* %6)
  %arrayidx6 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %7 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx6, align 4
  %add = add nsw i32 %i.0, 1
  %arrayidx7 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add
  %8 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx7, align 4
  %call8 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %7, i32 addrspace(4)* %8)
  %9 = bitcast i32 addrspace(4)* %call8 to i8 addrspace(4)*
  %call9 = call i8* @__to_private(i8 addrspace(4)* %9)
  %10 = bitcast i8* %call9 to i32*
  %arrayidx10 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %11 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx10, align 4
  %12 = addrspacecast i32 addrspace(4)* %11 to i32 addrspace(1)*
  %13 = addrspacecast i32 addrspace(1)* %12 to i8 addrspace(4)*
  %call11 = call i8* @__to_private(i8 addrspace(4)* %13)
  %14 = bitcast i8* %call11 to i32*
  %arrayidx12 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %15 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx12, align 4
  %16 = ptrtoint i32 addrspace(4)* %15 to i32
  %17 = inttoptr i32 %16 to i32 addrspace(4)*
  %18 = bitcast i32 addrspace(4)* %17 to i8 addrspace(4)*
  %call13 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %18)
  %arrayidx14 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %19 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx14, align 4
  %20 = bitcast i32 addrspace(4)* %19 to i8 addrspace(4)*
  %call15 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %20)
  %21 = bitcast i8 addrspace(1)* %call15 to i32 addrspace(1)*
  %tobool16 = icmp ne i32 addrspace(1)* %21, null
  br i1 %tobool16, label %if.then17, label %if.end20

if.then17:                                        ; preds = %if.end
  %arrayidx18 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 9
  %22 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx18, align 4
  %23 = bitcast i32 addrspace(4)* %22 to i8 addrspace(4)*
  %call19 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %23)
  %24 = bitcast i8 addrspace(1)* %call19 to i32 addrspace(1)*
  br label %if.end20

if.end20:                                         ; preds = %if.then17, %if.end
  %arrayidx21 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %25 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx21, align 4
  %add22 = add nsw i32 %i.0, 1
  %arrayidx23 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add22
  %26 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx23, align 4
  %cmp24 = icmp eq i32 addrspace(4)* %25, %26
  br i1 %cmp24, label %if.then25, label %if.end28

if.then25:                                        ; preds = %if.end20
  %arrayidx26 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %27 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx26, align 4
  %28 = bitcast i32 addrspace(4)* %27 to i8 addrspace(4)*
  %call27 = call i8* @__to_private(i8 addrspace(4)* %28)
  %29 = bitcast i8* %call27 to i32*
  br label %if.end28

if.end28:                                         ; preds = %if.then25, %if.end20
  %arrayidx29 = getelementptr inbounds [10 x i32 addrspace(4)*], [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %30 = load i32 addrspace(4)*, i32 addrspace(4)** %arrayidx29, align 4
  %31 = bitcast i32 addrspace(4)* %30 to float addrspace(4)*
  %add.ptr30 = getelementptr inbounds float, float addrspace(4)* %31, i32 10
  %call31 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr30)
  br label %for.inc

for.inc:                                          ; preds = %if.end28
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i8* @__to_private(i8 addrspace(4)*)

declare i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @func}

;;  -----  BasicCasesDynamicArray.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCasesDynamicArray.cl -o BasicCasesDynamicArrayTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesDynamicArrayTmp.ll -S -o BasicCasesDynamicArray.ll

;;void test1(int* p) {
;;  __local int* a = to_local(p);
;;  __global int* b = to_global(p);
;;}


;;int* test2(int *p1, int *p2) {
;;  if (to_global(p1))
;;    return p1;
;;  return p2;
;;}

;;__kernel void func(__global int *pGlobal, __local int *pLocal, float param) {
;;  int* ptrs[10];
;;  for (int i = 0; i < 10; i++) {
  
;;    // Initialization
;;    if (i%2) {
;;       ptrs[i] = pGlobal + i;
;;    } else {
;;       ptrs[i] = pLocal + i;
;;    }
;;    *ptrs[i] = i;
;;    __global int* a = to_global(ptrs[i]);
    
;;    // Function calls
;;    test1(ptrs[i]);
;;    int* pGen5 = test2(ptrs[i], ptrs[i+1]);
;;    __private int* d = to_private(pGen5);
    
;;    // Casting
;;    __global int* pGlobal1 = (__global int*)ptrs[i];
;;    __private int* c = to_private(pGlobal1);
    
;;    // Conversion to/from integer
;;  	size_t intGen2 = (size_t)ptrs[i];
;;  	int* pGen3 = (int*)intGen2;
;;  	cl_mem_fence_flags foo = get_fence(pGen3);
  	
;;  	// Address Specifier BI
;;  	__global int* b2 = to_global(ptrs[i]);
;;  	if (b2) {
;;    	__global int* f = to_global(ptrs[9]);
;;    }

;;	  // ICMP, load
;;	  if (ptrs[i] == ptrs[i+1]) {
;;	   __private int* e = to_private(ptrs[i]);
;;	  }

;;	  // BI with generic addr space
;;	  float* pGen4 = (float*)ptrs[i];
;;	  float res = fract(param, pGen4 + 10);   
;;  }
;;}
