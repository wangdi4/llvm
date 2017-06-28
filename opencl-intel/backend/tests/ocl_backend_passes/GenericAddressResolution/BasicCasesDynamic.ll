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

; CHECK: @test
; CHECK-NOT: %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %3)
; CHECK: %ToNamedPtr = addrspacecast i8 addrspace(4)* %3 to i8 addrspace(1)*
; CHECK-NOT: %call1 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %7)
; CHECK: %ToNamedPtr1 = addrspacecast i8 addrspace(4)* %7 to i8 addrspace(3)*
; CHECK-NOT: %call2 = call i8* @__to_private(i8 addrspace(4)* %10)
; CHECK: %ToNamedPtr2 = addrspacecast i8 addrspace(4)* %10 to i8*
; CHECK-NOT: %call3 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %14)
; CHECK-NOT: %call9 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
; CHECK: %AddrSpace = addrspacecast float addrspace(4)* %add.ptr to float addrspace(1)*
; CHECK: %22 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %AddrSpace)
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

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

define void @test(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %0 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %0)
  %1 = addrspacecast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %1)
  %2 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %3)
  %4 = bitcast i8 addrspace(1)* %call to i32 addrspace(1)*
  %tobool = fcmp une float %param, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %5 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  br label %if.end

if.else:                                          ; preds = %entry
  %6 = addrspacecast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %pGen1.0 = phi i32 addrspace(4)* [ %5, %if.then ], [ %6, %if.else ]
  %7 = bitcast i32 addrspace(4)* %pGen1.0 to i8 addrspace(4)*
  %call1 = call i8 addrspace(3)* @__to_local(i8 addrspace(4)* %7)
  %8 = bitcast i8 addrspace(3)* %call1 to i32 addrspace(3)*
  %9 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %10 = bitcast i32 addrspace(4)* %9 to i8 addrspace(4)*
  %call2 = call i8* @__to_private(i8 addrspace(4)* %10)
  %11 = bitcast i8* %call2 to i32*
  %12 = ptrtoint i32 addrspace(4)* %9 to i32
  %13 = inttoptr i32 %12 to i32 addrspace(4)*
  %14 = bitcast i32 addrspace(4)* %13 to i8 addrspace(4)*
  %call3 = call i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)* %14)
  %15 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %16 = addrspacecast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  %call4 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %15, i32 addrspace(4)* %16)
  %17 = bitcast i32 addrspace(4)* %call4 to i8 addrspace(4)*
  %call5 = call i8* @__to_private(i8 addrspace(4)* %17)
  %18 = bitcast i8* %call5 to i32*
  %cmp = icmp eq i32 addrspace(4)* %13, %9
  br i1 %cmp, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end
  %19 = bitcast i32 addrspace(4)* %9 to i8 addrspace(4)*
  %call7 = call i8* @__to_private(i8 addrspace(4)* %19)
  %20 = bitcast i8* %call7 to i32*
  br label %if.end8

if.end8:                                          ; preds = %if.then6, %if.end
  %21 = addrspacecast i32 addrspace(1)* %pGlobal to float addrspace(4)*
  %add.ptr = getelementptr inbounds float, float addrspace(4)* %21, i32 10
  %call9 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
  ret void
}

declare i8* @__to_private(i8 addrspace(4)*)

declare i32 @_Z9get_fencePU3AS4Kv(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test}

;;  -----  BasicCasesDynamic.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h -D__OPENCL_C_VERSION__=200 BasicCasesDynamic.cl -o BasicCasesDynamicTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesDynamicTmp.ll -S -o BasicCasesDynamic.ll

;;void test1(int* p) {
;;  __local int* a = to_local(p);
;;  __global int* b = to_global(p);
;;}

;;int* test2(int *p1, int *p2) {
;;  if (to_global(p1))
;;    return p1;
;;  return p2;
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
;;  int* pGen5 = test2(pGlobal, pLocal);
;;  __private int* d = to_private(pGen5);

;;  // ICMP, load
;;  if (pGen3 == pGen1) {
;;   __private int* e = to_private(pGen1);
;;  }

;;  // BI with generic addr space
;;  float* pGen4 = (float*)pGlobal;
;;  float res = fract(param, pGen4 + 10);
 
;;}
