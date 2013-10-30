; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test
; CHECK: %1 = bitcast i32 addrspace(1)* %0 to i32 addrspace(1)*
; CHECK: call void @_Z5test1PU3AS1i(i32 addrspace(1)* %1)
; CHECK: %3 = bitcast i32 addrspace(3)* %2 to i32 addrspace(3)*
; CHECK: call void @_Z5test1PU3AS3i(i32 addrspace(3)* %3)
; CHECK: %5 = bitcast i32 addrspace(1)* %4 to i32 addrspace(1)*
; CHECK: %6 = bitcast i32 addrspace(1)* %5 to i32 addrspace(4)*
; CHECK: store i32 addrspace(4)* %6, i32 addrspace(4)** %pGen1, align 4
; CHECK: %7 = load i32 addrspace(4)** %pGen1, align 4
; CHECK: %28 = ptrtoint i32 addrspace(4)* %27 to i32
; CHECK: store i32 %28, i32* %intGen2, align 4
; CHECK: %29 = load i32* %intGen2, align 4
; CHECK: %30 = inttoptr i32 %29 to i32 addrspace(4)*
; CHECK: %32 = load i32 addrspace(1)** %pGlobal.addr, align 4
; CHECK: %33 = bitcast i32 addrspace(1)* %32 to i32 addrspace(1)*
; CHECK: %34 = load i32 addrspace(3)** %pLocal.addr, align 4
; CHECK: %35 = bitcast i32 addrspace(3)* %34 to i32 addrspace(3)*
; CHECK: %call1 = call i32 addrspace(4)* @_Z5test2PU3AS1iPU3AS3i(i32 addrspace(1)* %33, i32 addrspace(3)* %35)
; CHECK: store i32 addrspace(4)* %call1, i32 addrspace(4)** %pGen5, align 4
; CHECK: %40 = load i32 addrspace(4)** %pGen3, align 4
; CHECK: %41 = bitcast i32 addrspace(4)* %40 to i8 addrspace(4)*
; CHECK: %call7 = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %41)
; CHECK: %54 = load float addrspace(4)** %pGen4, align 4
; CHECK: %add.ptr = getelementptr inbounds float addrspace(4)* %54, i32 10
; CHECK: %call16 = call float @_Z5fractfPU3AS4f(float %53, float addrspace(4)* %add.ptr)
; CHECK: store float %call16, float* %res, align 4
; CHECK: ret

; CHECK: @_Z5test1PU3AS1i(i32 addrspace(1)* %a)
; CHECK: %a12 = bitcast i32 addrspace(1)* %a to i32 addrspace(1)*
; CHECK: %a13 = bitcast i32 addrspace(1)* %a12 to i32 addrspace(4)*
; CHECK: %a.addr = alloca i32 addrspace(4)*, align 4
; CHECK: store i32 addrspace(4)* %a13, i32 addrspace(4)** %a.addr, align 4
; CHECK: %0 = load i32 addrspace(4)** %a.addr, align 4
; CHECK: %arrayidx = getelementptr inbounds i32 addrspace(4)* %0, i32 0
; CHECK: store i32 0, i32 addrspace(4)* %arrayidx, align 4
; CHECK: ret

; CHECK: @_Z5test1PU3AS3i(i32 addrspace(3)* %a)
; CHECK: %a12 = bitcast i32 addrspace(3)* %a to i32 addrspace(3)*
; CHECK: %a13 = bitcast i32 addrspace(3)* %a12 to i32 addrspace(4)*
; CHECK: %a.addr = alloca i32 addrspace(4)*, align 4
; CHECK: store i32 addrspace(4)* %a13, i32 addrspace(4)** %a.addr, align 4
; CHECK: %0 = load i32 addrspace(4)** %a.addr, align 4
; CHECK: %arrayidx = getelementptr inbounds i32 addrspace(4)* %0, i32 0
; CHECK: store i32 0, i32 addrspace(4)* %arrayidx, align 4
; CHECK: ret

; CHECK: i32 addrspace(4)* @_Z5test2PU3AS1iPU3AS3i(i32 addrspace(1)* %p1, i32 addrspace(3)* %p2)
; CHECK: %p223 = bitcast i32 addrspace(3)* %p2 to i32 addrspace(3)*
; CHECK: %p224 = bitcast i32 addrspace(3)* %p223 to i32 addrspace(4)*
; CHECK: %p115 = bitcast i32 addrspace(1)* %p1 to i32 addrspace(1)*
; CHECK: %p116 = bitcast i32 addrspace(1)* %p115 to i32 addrspace(4)*
; CHECK: store i32 addrspace(4)* %p116, i32 addrspace(4)** %p1.addr, align 4
; CHECK: store i32 addrspace(4)* %p224, i32 addrspace(4)** %p2.addr, align 4
; CHECK: %0 = load i32 addrspace(4)** %p1.addr, align 4
; CHECK: %1 = load i32 addrspace(4)** %p2.addr, align 4
; CHECK: %cmp = icmp eq i32 addrspace(4)* %0, %1
; CHECK: ret

define void @test1(i32 addrspace(4)* %a) nounwind {
entry:
  %a.addr = alloca i32 addrspace(4)*, align 4
  store i32 addrspace(4)* %a, i32 addrspace(4)** %a.addr, align 4
  %0 = load i32 addrspace(4)** %a.addr, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %0, i32 0
  store i32 0, i32 addrspace(4)* %arrayidx, align 4
  %1 = load i32 addrspace(4)** %a.addr, align 4
  %arrayidx1 = getelementptr inbounds i32 addrspace(4)* %1, i32 1
  store i32 1, i32 addrspace(4)* %arrayidx1, align 4
  ret void
}

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %retval = alloca i32 addrspace(4)*, align 4
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
  store i32 addrspace(4)* %2, i32 addrspace(4)** %retval
  br label %return

if.end:                                           ; preds = %entry
  %3 = load i32 addrspace(4)** %p2.addr, align 4
  store i32 addrspace(4)* %3, i32 addrspace(4)** %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %4 = load i32 addrspace(4)** %retval
  ret i32 addrspace(4)* %4
}

define void @test(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %pGlobal.addr = alloca i32 addrspace(1)*, align 4
  %pLocal.addr = alloca i32 addrspace(3)*, align 4
  %param.addr = alloca float, align 4
  %pGen1 = alloca i32 addrspace(4)*, align 4
  %pGlobal1 = alloca i32 addrspace(1)*, align 4
  %pLocal1 = alloca i32 addrspace(3)*, align 4
  %pGen2 = alloca i32 addrspace(4)*, align 4
  %intGen2 = alloca i32, align 4
  %pGen3 = alloca i32 addrspace(4)*, align 4
  %pGen5 = alloca i32 addrspace(4)*, align 4
  %b2 = alloca i32 addrspace(1)*, align 4
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
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %6, i32 2
  store i32 2, i32 addrspace(4)* %arrayidx, align 4
  %7 = load i32 addrspace(4)** %pGen1, align 4
  %8 = bitcast i32 addrspace(4)* %7 to i32 addrspace(1)*
  store i32 addrspace(1)* %8, i32 addrspace(1)** %pGlobal1, align 4
  %9 = load i32 addrspace(1)** %pGlobal1, align 4
  %arrayidx1 = getelementptr inbounds i32 addrspace(1)* %9, i32 3
  store i32 3, i32 addrspace(1)* %arrayidx1, align 4
  %10 = load i32 addrspace(4)** %pGen1, align 4
  %11 = bitcast i32 addrspace(4)* %10 to i32 addrspace(3)*
  store i32 addrspace(3)* %11, i32 addrspace(3)** %pLocal1, align 4
  %12 = load i32 addrspace(3)** %pLocal1, align 4
  %arrayidx2 = getelementptr inbounds i32 addrspace(3)* %12, i32 4
  store i32 4, i32 addrspace(3)* %arrayidx2, align 4
  %13 = load float* %param.addr, align 4
  %tobool = fcmp une float %13, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %14 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %15 = bitcast i32 addrspace(1)* %14 to i32 addrspace(4)*
  store i32 addrspace(4)* %15, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %16 = load i32 addrspace(3)** %pLocal.addr, align 4
  %17 = bitcast i32 addrspace(3)* %16 to i32 addrspace(4)*
  store i32 addrspace(4)* %17, i32 addrspace(4)** %pGen1, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %18 = load i32 addrspace(4)** %pGen1, align 4
  %arrayidx3 = getelementptr inbounds i32 addrspace(4)* %18, i32 5
  store i32 5, i32 addrspace(4)* %arrayidx3, align 4
  %19 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %20 = bitcast i32 addrspace(1)* %19 to i32 addrspace(4)*
  store i32 addrspace(4)* %20, i32 addrspace(4)** %pGen1, align 4
  %21 = load i32 addrspace(4)** %pGen1, align 4
  store i32 addrspace(4)* %21, i32 addrspace(4)** %pGen2, align 4
  %22 = load i32 addrspace(4)** %pGen2, align 4
  %arrayidx4 = getelementptr inbounds i32 addrspace(4)* %22, i32 6
  store i32 6, i32 addrspace(4)* %arrayidx4, align 4
  %23 = load i32 addrspace(4)** %pGen2, align 4
  %24 = ptrtoint i32 addrspace(4)* %23 to i32
  store i32 %24, i32* %intGen2, align 4
  %25 = load i32* %intGen2, align 4
  %26 = inttoptr i32 %25 to i32 addrspace(4)*
  store i32 addrspace(4)* %26, i32 addrspace(4)** %pGen3, align 4
  %27 = load i32 addrspace(4)** %pGen3, align 4
  %arrayidx5 = getelementptr inbounds i32 addrspace(4)* %27, i32 7
  store i32 7, i32 addrspace(4)* %arrayidx5, align 4
  %28 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %29 = bitcast i32 addrspace(1)* %28 to i32 addrspace(4)*
  %30 = load i32 addrspace(3)** %pLocal.addr, align 4
  %31 = bitcast i32 addrspace(3)* %30 to i32 addrspace(4)*
  %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* %29, i32 addrspace(4)* %31)
  store i32 addrspace(4)* %call, i32 addrspace(4)** %pGen5, align 4
  %32 = load i32 addrspace(4)** %pGen5, align 4
  %arrayidx6 = getelementptr inbounds i32 addrspace(4)* %32, i32 8
  store i32 8, i32 addrspace(4)* %arrayidx6, align 4
  %33 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %34 = bitcast i32 addrspace(1)* %33 to i32 addrspace(4)*
  store i32 addrspace(4)* %34, i32 addrspace(4)** %pGen3, align 4
  %35 = load i32 addrspace(4)** %pGen3, align 4
  %36 = bitcast i32 addrspace(4)* %35 to i8 addrspace(4)*
  %call7 = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %36)
  %37 = bitcast i8 addrspace(1)* %call7 to i32 addrspace(1)*
  store i32 addrspace(1)* %37, i32 addrspace(1)** %b2, align 4
  %38 = load i32 addrspace(1)** %b2, align 4
  %tobool8 = icmp ne i32 addrspace(1)* %38, null
  br i1 %tobool8, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end
  %39 = load i32 addrspace(4)** %pGen3, align 4
  %arrayidx10 = getelementptr inbounds i32 addrspace(4)* %39, i32 9
  store i32 9, i32 addrspace(4)* %arrayidx10, align 4
  br label %if.end11

if.end11:                                         ; preds = %if.then9, %if.end
  %40 = load i32 addrspace(4)** %pGen3, align 4
  %41 = load i32 addrspace(4)** %pGen1, align 4
  %cmp = icmp eq i32 addrspace(4)* %40, %41
  br i1 %cmp, label %if.then12, label %if.end15

if.then12:                                        ; preds = %if.end11
  %42 = load i32 addrspace(4)** %pGen3, align 4
  %arrayidx13 = getelementptr inbounds i32 addrspace(4)* %42, i32 8
  %43 = load i32 addrspace(4)* %arrayidx13, align 4
  %44 = load i32 addrspace(4)** %pGen1, align 4
  %arrayidx14 = getelementptr inbounds i32 addrspace(4)* %44, i32 10
  store i32 %43, i32 addrspace(4)* %arrayidx14, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.then12, %if.end11
  %45 = load i32 addrspace(1)** %pGlobal.addr, align 4
  %46 = bitcast i32 addrspace(1)* %45 to float addrspace(4)*
  store float addrspace(4)* %46, float addrspace(4)** %pGen4, align 4
  %47 = load float* %param.addr, align 4
  %48 = load float addrspace(4)** %pGen4, align 4
  %add.ptr = getelementptr inbounds float addrspace(4)* %48, i32 10
  %call16 = call float @_Z5fractfPU3AS4f(float %47, float addrspace(4)* %add.ptr)
  store float %call16, float* %res, align 4
  ret void
}

declare i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test, metadata !1}
!1 = metadata !{metadata !"argument_attribute", i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"-cl-std=CL2.0"}

;;  -----  BasicCasesAlloca.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCasesAlloca.cl -o BasicCasesAlloca.ll

;;void test1(int* a) {
;;  a[0] = 0;
;;  a[1] = 1;
;;}

;;int* test2(int *p1, int *p2) {
;;  if (p1 == p2)
;;    return p1;
;;  return p2;
;;}

;;__kernel void test(__global int* pGlobal, __local int* pLocal, float param) {

;;  // Function call
;;  test1(pGlobal);
;;  test1(pLocal);

;;  // Initialization
;;  int* pGen1 = pGlobal;
;;  pGen1[2] = 2;

;;  // Casting (correct)
;;  __global int* pGlobal1 = (__global int*)pGen1;
;;  pGlobal1[3] = 3;

;;  // Casting (incorrect) 
;;  __local int* pLocal1 = (__local int*)pGen1;
;;  pLocal1[4] = 4;

;;  // Ambiguous assignment
;;  if (param) {
;;    pGen1 = pGlobal;
;;  } else {
;;    pGen1 = pLocal;
;;  }
;;  pGen1[5] = 5;

;;  // Assignment
;;  pGen1 = pGlobal;
;;  int* pGen2 = pGen1;
;;  pGen2[6] = 6;

;;  // Conversion to/from integer
;;  size_t intGen2 = (size_t)pGen2;
;;  int* pGen3 = (int*)intGen2;
;;  pGen3[7] = 7;

;;  // Corner case
;;  int* pGen5 = test2(pGlobal, pLocal);
;;  pGen5[8] = 8;

;;  // Address Specifier BI
;;  pGen3 = pGlobal;
;;  __global int*  b2 = to_global(pGen3);
;;  if (b2) {
;;    pGen3[9] = 9;
;;  }

;;  // ICMP, load
;;  if (pGen3 == pGen1) {
;;   pGen1[10] = pGen3[8];
;;  }

;;  // BI with generic addr space
;;  float* pGen4 = (float*)pGlobal;
;;  float res = fract(param, pGen4 + 10);
 
;;}
