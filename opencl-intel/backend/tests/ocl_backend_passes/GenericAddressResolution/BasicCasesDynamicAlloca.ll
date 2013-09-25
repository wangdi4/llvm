; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK-NOT: @test1
; CHECK-NOT: @test2

; CHECK: @test
; CHECK: %AllocaSpace3 = alloca i32
; CHECK: %AllocaSpace2 = alloca i32
; CHECK: %AllocaSpace1 = alloca i32
; CHECK: %AllocaSpace = alloca i32
; CHECK-NOT: call void @test1(i32 addrspace(4)* %1)
; CHECK: call void @_Z5test1PU3AS4i(i32 addrspace(4)* %1, i32 1)
; CHECK-NOT: call void @test1(i32 addrspace(4)* %3)
; CHECK: call void @_Z5test1PU3AS4i(i32 addrspace(4)* %3, i32 3)
; CHECK: store i32 addrspace(4)* %5, i32 addrspace(4)** %pGen1, align 4
; CHECK: store i32 1, i32* %AllocaSpace
; CHECK: %6 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %AddrSpace11 = load i32* %AllocaSpace
; CHECK-NOT: %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %7)
; CHECK: %AddrSpace12 = icmp eq i32 %AddrSpace11, 1
; CHECK: store i32 addrspace(4)* %10, i32 addrspace(4)** %pGen1, align 4
; CHECK: store i32 1, i32* %AllocaSpace
; CHECK: store i32 addrspace(4)* %12, i32 addrspace(4)** %pGen1, align 4
; CHECK: store i32 3, i32* %AllocaSpace
; CHECK: %13 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %AddrSpace10 = load i32* %AllocaSpace
; CHECK-NOT: %call1 = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %14)
; CHECK: %AddrSpace13 = icmp eq i32 %AddrSpace10, 3
; CHECK: store i32 addrspace(4)* %16, i32 addrspace(4)** %pGen1, align 4
; CHECK: store i32 1, i32* %AllocaSpace
; CHECK: %17 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %AddrSpace5 = load i32* %AllocaSpace
; CHECK: store i32 addrspace(4)* %17, i32 addrspace(4)** %pGen2, align 4
; CHECK: store i32 %AddrSpace5, i32* %AllocaSpace1
; CHECK: %18 = load i32 addrspace(4)** %pGen2, align 4
; CHECK: %AddrSpace9 = load i32* %AllocaSpace1
; CHECK-NOT: %call3 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %19)
; CHECK: %AddrSpace14 = icmp eq i32 %AddrSpace9, 0
; CHECK-NOT: %call5 = call i32 @get_fence(i8 addrspace(4)* %25)
; CHECK: %AddrSpace15 = icmp eq i32 %AddrSpace8, 1
; CHECK: %AddrSpace16 = icmp eq i32 %AddrSpace8, 3
; CHECK: %AddrSpace17 = select i1 %AddrSpace15, i32 2, i32 0
; CHECK: %AddrSpace18 = select i1 %AddrSpace16, i32 1, i32 %AddrSpace17
; CHECK-NOT: %call6 = call zeroext i1 @test2(i32 addrspace(4)* %27, i32 addrspace(4)* %29)
; CHECK: %30 = call zeroext i1 @_Z5test2PU3AS4iS_(i32 addrspace(4)* %27, i32 addrspace(4)* %29, i32 1, i32 3)
; CHECK: %32 = load i32 addrspace(4)** %pGen3, align 4
; CHECK: %AddrSpace7 = load i32* %AllocaSpace2
; CHECK-NOT: %call10 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %32)
; CHECK: %AddrSpace19 = icmp eq i32 %AddrSpace7, 0
; CHECK: %34 = load i32 addrspace(4)** %pGen3, align 4
; CHECK: %35 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %cmp = icmp eq i32 addrspace(4)* %34, %35
; CHECK: %36 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %AddrSpace6 = load i32* %AllocaSpace
; CHECK: %AddrSpace20 = icmp eq i32 %AddrSpace6, 0
; CHECK: %AddrSpace21 = bitcast float addrspace(4)* %add.ptr to float addrspace(1)*
; CHECK-NOT: %call17 = call float @_Z5fractfPU3AS4f(float %39, float addrspace(4)* %add.ptr)
; CHECK: %42 = call float @_Z5fractfPU3AS1f(float %40, float addrspace(1)* %AddrSpace21)
; CHECK: ret

; CHECK: @_Z5test2PU3AS4iS_(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2, i32 %ArgSpace, i32 %ArgSpace2)
; CHECK: store i32 addrspace(4)* %p1, i32 addrspace(4)** %p1.addr, align 4
; CHECK: store i32 %ArgSpace, i32* %AllocaSpace
; CHECK: store i32 addrspace(4)* %p2, i32 addrspace(4)** %p2.addr, align 4
; CHECK: store i32 %ArgSpace2, i32* %AllocaSpace1
; CHECK: %2 = load i32 addrspace(4)** %p1.addr, align 4
; CHECK: %AddrSpace3 = load i32* %AllocaSpace
; CHECK: %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
; CHECK: %AddrSpace4 = icmp eq i32 %AddrSpace3, 1
; CHECK: store i1 %AddrSpace4, i1* %retval
; CHECK: %4 = load i32 addrspace(4)** %p2.addr, align 4
; CHECK: %AddrSpace = load i32* %AllocaSpace1
; CHECK: %5 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
; CHECK: %AddrSpace5 = icmp eq i32 %AddrSpace, 3
; CHECK: store i1 %AddrSpace5, i1* %retval
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

define void @test1(i32 addrspace(4)* %p) nounwind {
entry:
  %p.addr = alloca i32 addrspace(4)*, align 4
  %a = alloca i8, align 1
  %b = alloca i8, align 1
  store i32 addrspace(4)* %p, i32 addrspace(4)** %p.addr, align 4
  %0 = load i32 addrspace(4)** %p.addr, align 4
  %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %1)
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %a, align 1
  %2 = load i32 addrspace(4)** %p.addr, align 4
  %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %3)
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8* %b, align 1
  ret void
}

declare zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)*)

declare zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)*)

define zeroext i1 @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %retval = alloca i1, align 1
  %p1.addr = alloca i32 addrspace(4)*, align 4
  %p2.addr = alloca i32 addrspace(4)*, align 4
  store i32 addrspace(4)* %p1, i32 addrspace(4)** %p1.addr, align 4
  store i32 addrspace(4)* %p2, i32 addrspace(4)** %p2.addr, align 4
  %0 = load i32 addrspace(4)** %p1.addr, align 4
  %1 = load i32 addrspace(4)** %p2.addr, align 4
  %cmp = icmp eq i32 addrspace(4)* %0, %1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %2 = load i32 addrspace(4)** %p1.addr, align 4
  %3 = bitcast i32 addrspace(4)* %2 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %3)
  store i1 %call, i1* %retval
  br label %return

if.end:                                           ; preds = %entry
  %4 = load i32 addrspace(4)** %p2.addr, align 4
  %5 = bitcast i32 addrspace(4)* %4 to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %5)
  store i1 %call1, i1* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %6 = load i1* %retval
  ret i1 %6
}

define void @test(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %pGlobal.addr = alloca i32 addrspace(1)*, align 4
  %pLocal.addr = alloca i32 addrspace(3)*, align 4
  %param.addr = alloca float, align 4
  %pGen1 = alloca i32 addrspace(4)*, align 4
  %a = alloca i8, align 1
  %b = alloca i8, align 1
  %pGen2 = alloca i32 addrspace(4)*, align 4
  %c = alloca i8, align 1
  %intGen2 = alloca i32, align 4
  %pGen3 = alloca i32 addrspace(4)*, align 4
  %foo = alloca i32, align 4
  %b1 = alloca i8, align 1
  %d = alloca i8, align 1
  %e = alloca i8, align 1
  %pGen4 = alloca float addrspace(4)*, align 4
  %res = alloca float, align 4
  store i32 addrspace(1)* %pGlobal, i32 addrspace(1)** %pGlobal.addr, align 4
  store i32 addrspace(3)* %pLocal, i32 addrspace(3)** %pLocal.addr, align 4
  store float %param, float* %param.addr, align 4
  %0 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %1 = bitcast i32 addrspace(1)* %0 to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %1)
  %2 = load i32 addrspace(3)** %pLocal.addr, align 4
  %3 = bitcast i32 addrspace(3)* %2 to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %3)
  %4 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %5 = bitcast i32 addrspace(1)* %4 to i32 addrspace(4)*
  store i32 addrspace(4)* %5, i32 addrspace(4)** %pGen1, align 4
  %6 = load i32 addrspace(4)** %pGen1, align 4
  %7 = bitcast i32 addrspace(4)* %6 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %7)
  %frombool = zext i1 %call to i8
  store i8 %frombool, i8* %a, align 1
  %8 = load float* %param.addr, align 4
  %tobool = fcmp une float %8, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %9 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %10 = bitcast i32 addrspace(1)* %9 to i32 addrspace(4)*
  store i32 addrspace(4)* %10, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %11 = load i32 addrspace(3)** %pLocal.addr, align 4
  %12 = bitcast i32 addrspace(3)* %11 to i32 addrspace(4)*
  store i32 addrspace(4)* %12, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %13 = load i32 addrspace(4)** %pGen1, align 4
  %14 = bitcast i32 addrspace(4)* %13 to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %14)
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8* %b, align 1
  %15 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %16 = bitcast i32 addrspace(1)* %15 to i32 addrspace(4)*
  store i32 addrspace(4)* %16, i32 addrspace(4)** %pGen1, align 4
  %17 = load i32 addrspace(4)** %pGen1, align 4
  store i32 addrspace(4)* %17, i32 addrspace(4)** %pGen2, align 4
  %18 = load i32 addrspace(4)** %pGen2, align 4
  %19 = bitcast i32 addrspace(4)* %18 to i8 addrspace(4)*
  %call3 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %19)
  %frombool4 = zext i1 %call3 to i8
  store i8 %frombool4, i8* %c, align 1
  %20 = load i32 addrspace(4)** %pGen2, align 4
  %21 = ptrtoint i32 addrspace(4)* %20 to i32
  store i32 %21, i32* %intGen2, align 4
  %22 = load i32* %intGen2, align 4
  %23 = inttoptr i32 %22 to i32 addrspace(4)*
  store i32 addrspace(4)* %23, i32 addrspace(4)** %pGen3, align 4
  %24 = load i32 addrspace(4)** %pGen3, align 4
  %25 = bitcast i32 addrspace(4)* %24 to i8 addrspace(4)*
  %call5 = call i32 @get_fence(i8 addrspace(4)* %25)
  store i32 %call5, i32* %foo, align 4
  %26 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %27 = bitcast i32 addrspace(1)* %26 to i32 addrspace(4)*
  %28 = load i32 addrspace(3)** %pLocal.addr, align 4
  %29 = bitcast i32 addrspace(3)* %28 to i32 addrspace(4)*
  %call6 = call zeroext i1 @test2(i32 addrspace(4)* %27, i32 addrspace(4)* %29)
  %frombool7 = zext i1 %call6 to i8
  store i8 %frombool7, i8* %b1, align 1
  %30 = load i8* %b1, align 1
  %tobool8 = trunc i8 %30 to i1
  br i1 %tobool8, label %if.then9, label %if.end12

if.then9:                                         ; preds = %if.end
  %31 = load i32 addrspace(4)** %pGen3, align 4
  %32 = bitcast i32 addrspace(4)* %31 to i8 addrspace(4)*
  %call10 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %32)
  %frombool11 = zext i1 %call10 to i8
  store i8 %frombool11, i8* %d, align 1
  br label %if.end12

if.end12:                                         ; preds = %if.then9, %if.end
  %33 = load i32 addrspace(4)** %pGen3, align 4
  %34 = load i32 addrspace(4)** %pGen1, align 4
  %cmp = icmp eq i32 addrspace(4)* %33, %34
  br i1 %cmp, label %if.then13, label %if.end16

if.then13:                                        ; preds = %if.end12
  %35 = load i32 addrspace(4)** %pGen1, align 4
  %36 = bitcast i32 addrspace(4)* %35 to i8 addrspace(4)*
  %call14 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %36)
  %frombool15 = zext i1 %call14 to i8
  store i8 %frombool15, i8* %e, align 1
  br label %if.end16

if.end16:                                         ; preds = %if.then13, %if.end12
  %37 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %38 = bitcast i32 addrspace(1)* %37 to float addrspace(4)*
  store float addrspace(4)* %38, float addrspace(4)** %pGen4, align 4
  %39 = load float* %param.addr, align 4
  %40 = load float addrspace(4)** %pGen4, align 4
  %add.ptr = getelementptr inbounds float addrspace(4)* %40, i32 10
  %call17 = call float @_Z5fractfPU3AS4f(float %39, float addrspace(4)* %add.ptr)
  store float %call17, float* %res, align 4
  ret void
}

declare zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)*)

declare i32 @get_fence(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test}

;;  -----  BasicCasesDynamicAlloca.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h -D__OPENCL_C_VERSION__=200 BasicCasesDynamic.cl -o BasicCasesDynamicAlloca.ll

;;void test1(int* p) {
;;  bool a = is_local(p);
;;  bool b = is_global(p);
;;}

;;bool test2(int *p1, int *p2) {
;;  if (p1 == p2)
;;    return is_global(p1);
;;  return is_local(p2);
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
;;  bool b1 = test2(pGlobal, pLocal);
;;  if (b1) {
;;    bool d = is_private(pGen3);
;;  }

;;  // ICMP, load
;;  if (pGen3 == pGen1) {
;;   bool e = is_private(pGen1);
;;  }

;;  // BI with generic addr space
;;  float* pGen4 = (float*)pGlobal;
;;  float res = fract(param, pGen4 + 10);
 
;;}
