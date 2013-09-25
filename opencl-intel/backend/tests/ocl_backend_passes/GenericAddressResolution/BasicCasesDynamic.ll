; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK-NOT: @test1
; CHECK-NOT: @test2

; CHECK: @test
; CHECK: %AllocaSpace = alloca i32
; CHECK: call void @_Z5test1PU3AS4i(i32 addrspace(4)* %0, i32 1)
; CHECK: call void @_Z5test1PU3AS4i(i32 addrspace(4)* %1, i32 3)
; CHECK-NOT: %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %3)
; CHECK: %AddrSpace2 = icmp eq i32 1, 1
; CHECK: %AddrSpace1 = phi i32 [ 1, %if.then ], [ 3, %if.else ]
; CHECK-NOT: %call1 = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %6)
; CHECK: %AddrSpace3 = icmp eq i32 %AddrSpace1, 3
; CHECK-NOT: %call3 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %8)
; CHECK: %AddrSpace4 = icmp eq i32 1, 0
; CHECK-NOT: %call5 = call i32 @get_fence(i8 addrspace(4)* %11)
; CHECK: %AddrSpace5 = icmp eq i32 1, 1
; CHECK: %AddrSpace6 = icmp eq i32 1, 3
; CHECK: %AddrSpace7 = select i1 %AddrSpace5, i32 2, i32 0
; CHECK: %AddrSpace8 = select i1 %AddrSpace6, i32 1, i32 %AddrSpace7
; CHECK-NOT: %call6 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %12, i32 addrspace(4)* %13)
; CHECK: %14 = call i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0i(i32 addrspace(4)* %12, i32 addrspace(4)* %13, i32 1, i32 3, i32* %AllocaSpace)
; CHECK: %AddrSpace = load i32* %AllocaSpace
; CHECK-NOT: %call7 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %14)
; CHECK: %AddrSpace9 = icmp eq i32 %AddrSpace, 0
; CHECK-NOT: %call10 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %15)
; CHECK: %AddrSpace10 = icmp eq i32 1, 0
; CHECK:   %AddrSpace11 = bitcast float addrspace(4)* %add.ptr to float addrspace(1)*
; CHECK-NOT: %call13 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
; CHECK: %18 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %AddrSpace11)
; CHECK: ret

; CHECK: define i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0i(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2, i32 %ArgSpace, i32 %ArgSpace1, i32* %ArgSpace3)
; CHECK: %0 = bitcast i32 addrspace(4)* %p1 to i8 addrspace(4)*
; CHECK: %AddrSpace2 = icmp eq i32 %ArgSpace, 1
; CHECK: %retval.0 = phi i32 addrspace(4)* [ %p1, %if.then ], [ %p2, %if.end ]
; CHECK: %AddrSpace = phi i32 [ %ArgSpace, %if.then ], [ %ArgSpace1, %if.end ]
; CHECK: store i32 %AddrSpace, i32* %ArgSpace3
; CHECK: ret i32 addrspace(4)* %retval.0

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

define void @test1(i32 addrspace(4)* %p) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %0)
  %frombool = zext i1 %call to i8
  %1 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %1)
  %frombool2 = zext i1 %call1 to i8
  ret void
}

declare zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)*)

declare zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)*)

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p1 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %0)
  br i1 %call, label %if.then, label %if.end

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
  %0 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %0)
  %1 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %1)
  %2 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %3)
  %frombool = zext i1 %call to i8
  %tobool = fcmp une float %param, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %4 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  br label %if.end

if.else:                                          ; preds = %entry
  %5 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %pGen1.0 = phi i32 addrspace(4)* [ %4, %if.then ], [ %5, %if.else ]
  %6 = bitcast i32 addrspace(4)* %pGen1.0 to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %6)
  %frombool2 = zext i1 %call1 to i8
  %7 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %8 = bitcast i32 addrspace(4)* %7 to i8 addrspace(4)*
  %call3 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %8)
  %frombool4 = zext i1 %call3 to i8
  %9 = ptrtoint i32 addrspace(4)* %7 to i32
  %10 = inttoptr i32 %9 to i32 addrspace(4)*
  %11 = bitcast i32 addrspace(4)* %10 to i8 addrspace(4)*
  %call5 = call i32 @get_fence(i8 addrspace(4)* %11)
  %12 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %13 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  %call6 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %12, i32 addrspace(4)* %13)
  %14 = bitcast i32 addrspace(4)* %call6 to i8 addrspace(4)*
  %call7 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %14)
  %frombool8 = zext i1 %call7 to i8
  %cmp = icmp eq i32 addrspace(4)* %10, %7
  br i1 %cmp, label %if.then9, label %if.end12

if.then9:                                         ; preds = %if.end
  %15 = bitcast i32 addrspace(4)* %7 to i8 addrspace(4)*
  %call10 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %15)
  %frombool11 = zext i1 %call10 to i8
  br label %if.end12

if.end12:                                         ; preds = %if.then9, %if.end
  %16 = bitcast i32 addrspace(1)* %pGlobal to float addrspace(4)*
  %add.ptr = getelementptr inbounds float addrspace(4)* %16, i32 10
  %call13 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
  ret void
}

declare zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)*)

declare i32 @get_fence(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test}

;;  -----  BasicCasesDynamic.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h -D__OPENCL_C_VERSION__=200 BasicCasesDynamic.cl -o BasicCasesDynamicTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesDynamicTmp.ll -S -o BasicCasesDynamic.ll

;;void test1(int* p) {
;;  bool a = is_local(p);
;;  bool b = is_global(p);
;;}

;;int* test2(int *p1, int *p2) {
;;  if (is_global(p1))
;;    return p1;
;;  return p2;
;;}

;;__kernel void test(__global int* pGlobal, __local int* pLocal, float param) {

;;  // Function call
;;  test1(pGlobal);
;;  test1(pLocal);

;;  // Initialization
;;  int* pGen1 = pGlobal;
;;  bool a = is_global(pGen1);

;;  // Ambiguous assignment
;;  if (param) {
;;    pGen1 = pGlobal;
;;  } else {
;;    pGen1 = pLocal;
;;  }
;;  bool b = is_local(pGen1);

;;  // Assignment
;;  pGen1 = pGlobal;
;;  int* pGen2 = pGen1;
;;  bool c = is_private(pGen2);

;;  // Conversion to/from integer
;;  size_t intGen2 = (size_t)pGen2;
;;  int* pGen3 = (int*)intGen2;
;;  cl_mem_fence_flags foo = get_fence(pGen3);

;;  // Corner case
;;  int* pGen5 = test2(pGlobal, pLocal);
;;  bool d = is_private(pGen5);

;;  // ICMP, load
;;  if (pGen3 == pGen1) {
;;   bool e = is_private(pGen1);
;;  }

;;  // BI with generic addr space
;;  float* pGen4 = (float*)pGlobal;
;;  float res = fract(param, pGen4 + 10);
 
;;}
