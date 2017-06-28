; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test1
; CHECK-NOT: %call = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %1)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %1 to i8 addrspace(3)*
; CHECK-NOT: %call1 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %4)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %4 to i8 addrspace(1)*
; CHECK: ret

; CHECK: @test2
; CHECK-NOT: %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %3)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %3 to i8 addrspace(1)*
; CHECK-NOT: %call2 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %5)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %5 to i8 addrspace(3)*
; CHECK: ret

; CHECK: @test
; CHECK-NOT: %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %7)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %7 to i8 addrspace(1)*
; CHECK-NOT: %call1 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %15)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %15 to i8 addrspace(3)*
; CHECK-NOT: %call2 = call i8* @__to_private(i8 addrspace(4)* %21)
; CHECK: %ToNamedPtr2 = addrspacecast i8 addrspace(4)* %21 to i8*
; CHECK-NOT: %call3 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %28)
; CHECK-NOT: store i32 %call3, i32* %foo, align 4
; CHECK: store i32 2, i32* %foo, align 4
; CHECK-NOT: %call12 = call float @_Z5fractfPU3AS4f(float %44, float addrspace(4)* %add.ptr)
; CHECK: %AddrSpace = addrspacecast float addrspace(4)* %add.ptr to float addrspace(1)*
; CHECK: %46 = call float @_Z5fractfPU3AS1f(float %44, float addrspace(1)* %AddrSpace)
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

define void @test1(i32 addrspace(4)* %p) nounwind {
entry:
  %p.addr = alloca i32 addrspace(4)*, align 4
  %a = alloca i32 addrspace(3)*, align 4
  %b = alloca i32 addrspace(1)*, align 4
  store i32 addrspace(4)* %p, i32 addrspace(4)** %p.addr, align 4
  %0 = load i32 addrspace(4)*, i32 addrspace(4)** %p.addr, align 4
  %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  %call = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %1)
  %2 = bitcast i8 addrspace(3)* %call to i32 addrspace(3)*
  store i32 addrspace(3)* %2, i32 addrspace(3)** %a, align 4
  %3 = load i32 addrspace(4)*, i32 addrspace(4)** %p.addr, align 4
  %4 = bitcast i32 addrspace(4)* %3 to i8 addrspace(4)*
  %call1 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %4)
  %5 = bitcast i8 addrspace(1)* %call1 to i32 addrspace(1)*
  store i32 addrspace(1)* %5, i32 addrspace(1)** %b, align 4
  ret void
}

declare i8 addrspace(3)* @__to_local(i8 addrspace(4)*)

declare i8 addrspace(1)* @__to_global(i8 addrspace(4)*)

define zeroext i1 @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %retval = alloca i1, align 1
  %p1.addr = alloca i32 addrspace(4)*, align 4
  %p2.addr = alloca i32 addrspace(4)*, align 4
  store i32 addrspace(4)* %p1, i32 addrspace(4)** %p1.addr, align 4
  store i32 addrspace(4)* %p2, i32 addrspace(4)** %p2.addr, align 4
  %0 = load i32 addrspace(4)*, i32 addrspace(4)** %p1.addr, align 4
  %1 = load i32 addrspace(4)*, i32 addrspace(4)** %p2.addr, align 4
  %cmp = icmp eq i32 addrspace(4)* %0, %1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load i32 addrspace(4)*, i32 addrspace(4)** %p1.addr, align 4
  %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %3)
  %cmp1 = icmp ne i8 addrspace(1)* %call, null
  store i1 %cmp1, i1* %retval
  br label %return

if.end:                                           ; preds = %entry
  %4 = load i32 addrspace(4)*, i32 addrspace(4)** %p2.addr, align 4
  %5 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %call2 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %5)
  %cmp3 = icmp ne i8 addrspace(3)* %call2, null
  store i1 %cmp3, i1* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %6 = load i1, i1* %retval
  ret i1 %6
}

define void @test(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %pGlobal.addr = alloca i32 addrspace(1)*, align 4
  %pLocal.addr = alloca i32 addrspace(3)*, align 4
  %param.addr = alloca float, align 4
  %pGen1 = alloca i32 addrspace(4)*, align 4
  %a = alloca i32 addrspace(1)*, align 4
  %b = alloca i32 addrspace(3)*, align 4
  %pGen2 = alloca i32 addrspace(4)*, align 4
  %c = alloca i32*, align 4
  %intGen2 = alloca i32, align 4
  %pGen3 = alloca i32 addrspace(4)*, align 4
  %foo = alloca i32, align 4
  %b1 = alloca i8, align 1
  %d = alloca i32*, align 4
  %e = alloca i32*, align 4
  %pGen4 = alloca float addrspace(4)*, align 4
  %res = alloca float, align 4
  store i32 addrspace(1)* %pGlobal, i32 addrspace(1)** %pGlobal.addr, align 4
  store i32 addrspace(3)* %pLocal, i32 addrspace(3)** %pLocal.addr, align 4
  store float %param, float* %param.addr, align 4
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %1 = addrspacecast i32 addrspace(1)* %0 to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %1)
  %2 = load i32 addrspace(3)*, i32 addrspace(3)** %pLocal.addr, align 4
  %3 = addrspacecast i32 addrspace(3)* %2 to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %3)
  %4 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %5 = addrspacecast i32 addrspace(1)* %4 to i32 addrspace(4)*
  store i32 addrspace(4)* %5, i32 addrspace(4)** %pGen1, align 4
  %6 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen1, align 4
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %7)
  %8 = bitcast i8 addrspace(1)* %call to i32 addrspace(1)*
  store i32 addrspace(1)* %8, i32 addrspace(1)** %a, align 4
  %9 = load float, float* %param.addr, align 4
  %tobool = fcmp une float %9, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %10 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %11 = addrspacecast i32 addrspace(1)* %10 to i32 addrspace(4)*
  store i32 addrspace(4)* %11, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %12 = load i32 addrspace(3)*, i32 addrspace(3)** %pLocal.addr, align 4
  %13 = addrspacecast i32 addrspace(3)* %12 to i32 addrspace(4)*
  store i32 addrspace(4)* %13, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %14 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen1, align 4
  %15 = bitcast i32 addrspace(4)* %14 to i8 addrspace(4)*
  %call1 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %15)
  %16 = bitcast i8 addrspace(3)* %call1 to i32 addrspace(3)*
  store i32 addrspace(3)* %16, i32 addrspace(3)** %b, align 4
  %17 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %18 = addrspacecast i32 addrspace(1)* %17 to i32 addrspace(4)*
  store i32 addrspace(4)* %18, i32 addrspace(4)** %pGen1, align 4
  %19 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen1, align 4
  store i32 addrspace(4)* %19, i32 addrspace(4)** %pGen2, align 4
  %20 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen2, align 4
  %21 = bitcast i32 addrspace(4)* %20 to i8 addrspace(4)*
  %call2 = call i8* @__to_private(i8 addrspace(4)* %21)
  %22 = bitcast i8* %call2 to i32*
  store i32* %22, i32** %c, align 4
  %23 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen2, align 4
  %24 = ptrtoint i32 addrspace(4)* %23 to i32
  store i32 %24, i32* %intGen2, align 4
  %25 = load i32, i32* %intGen2, align 4
  %26 = inttoptr i32 %25 to i32 addrspace(4)*
  store i32 addrspace(4)* %26, i32 addrspace(4)** %pGen3, align 4
  %27 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen3, align 4
  %28 = bitcast i32 addrspace(4)* %27 to i8 addrspace(4)*
  %call3 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %28)
  store i32 %call3, i32* %foo, align 4
  %29 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %30 = addrspacecast i32 addrspace(1)* %29 to i32 addrspace(4)*
  %31 = load i32 addrspace(3)*, i32 addrspace(3)** %pLocal.addr, align 4
  %32 = addrspacecast i32 addrspace(3)* %31 to i32 addrspace(4)*
  %call4 = call zeroext i1 @test2(i32 addrspace(4)* %30, i32 addrspace(4)* %32)
  %frombool = zext i1 %call4 to i8
  store i8 %frombool, i8* %b1, align 1
  %33 = load i8, i8* %b1, align 1
  %tobool5 = trunc i8 %33 to i1
  br i1 %tobool5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end
  %34 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen3, align 4
  %35 = bitcast i32 addrspace(4)* %34 to i8 addrspace(4)*
  %call7 = call i8* @__to_private(i8 addrspace(4)* %35)
  %36 = bitcast i8* %call7 to i32*
  store i32* %36, i32** %d, align 4
  br label %if.end8

if.end8:                                          ; preds = %if.then6, %if.end
  %37 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen3, align 4
  %38 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen1, align 4
  %cmp = icmp eq i32 addrspace(4)* %37, %38
  br i1 %cmp, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end8
  %39 = load i32 addrspace(4)*, i32 addrspace(4)** %pGen1, align 4
  %40 = bitcast i32 addrspace(4)* %39 to i8 addrspace(4)*
  %call10 = call i8* @__to_private(i8 addrspace(4)* %40)
  %41 = bitcast i8* %call10 to i32*
  store i32* %41, i32** %e, align 4
  br label %if.end11

if.end11:                                         ; preds = %if.then9, %if.end8
  %42 = load i32 addrspace(1)*, i32 addrspace(1)** %pGlobal.addr, align 4
  %43 = addrspacecast i32 addrspace(1)* %42 to float addrspace(4)*
  store float addrspace(4)* %43, float addrspace(4)** %pGen4, align 4
  %44 = load float, float* %param.addr, align 4
  %45 = load float addrspace(4)*, float addrspace(4)** %pGen4, align 4
  %add.ptr = getelementptr inbounds float, float addrspace(4)* %45, i32 10
  %call12 = call float @_Z5fractfPU3AS4f(float %44, float addrspace(4)* %add.ptr)
  store float %call12, float* %res, align 4
  ret void
}

declare i8* @__to_private(i8 addrspace(4)*)

declare i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test}

;;  -----  BasicCasesDynamicAlloca.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h -D__OPENCL_C_VERSION__=200 BasicCasesDynamic.cl -o BasicCasesDynamicAlloca.ll

;;void test1(int* p) {
;;  __local int* a = to_local(p);
;;  __global int* b = to_global(p);
;;}

;;bool test2(int *p1, int *p2) {
;;  if (p1 == p2)
;;    return (to_global(p1) != NULL);
;;  return (to_local(p2) != NULL);
;;}

;;__kernel void test(__global int* pGlobal, __local int* pLocal, float param) {

;;  // Function call
;;  test1(pGlobal);
;;  test1(pLocal);

;;  // Initialization
;;  int* pGen1 = pGlobal;
;;  __global int* a = to_global(pGen1);

;;  // Ambiguous assignment
;;  if (param) {
;;    pGen1 = pGlobal;
;;  } else {
;;    pGen1 = pLocal;
;;  }
;;  __local int* b = to_local(pGen1);

;;  // Assignment
;;  pGen1 = pGlobal;
;;  int* pGen2 = pGen1;
;;  __private int* c = to_private(pGen2);

;;  // Conversion to/from integer
;;  size_t intGen2 = (size_t)pGen2;
;;  int* pGen3 = (int*)intGen2;
;;  cl_mem_fence_flags foo = get_fence(pGen3);

;;  // Corner case
;;  bool b1 = test2(pGlobal, pLocal);
;;  if (b1) {
;;    __private int* d = to_private(pGen3);
;;  }

;;  // ICMP, load
;;  if (pGen3 == pGen1) {
;;   __private int* e = to_private(pGen1);
;;  }

;;  // BI with generic addr space
;;  float* pGen4 = (float*)pGlobal;
;;  float res = fract(param, pGen4 + 10);
 
;;}
