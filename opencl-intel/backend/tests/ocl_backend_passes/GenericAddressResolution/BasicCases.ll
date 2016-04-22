; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test
; CHECK: %0 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(1)
; CHECK: call void @_Z5test1PU3AS1i(i32 addrspace(1)* %0)
; CHECK: %1 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(3)
; CHECK: call void @_Z5test1PU3AS3i(i32 addrspace(3)* %1)
; CHECK: %arrayidx7 = getelementptr inbounds i32 addrspace(1)* %2, i32 2
; CHECK: store i32 2, i32 addrspace(1)* %arrayidx7, align 4
; CHECK: %3 = bitcast i32 addrspace(1)* %2 to i32 addrspace(1)*
; CHECK: %4 = addrspacecast i32 addrspace(1)* %2 to i32 addrspace(3)*
; CHECK: %6 = addrspacecast i32 addrspace(1)* %5 to i32 addrspace(4)*
; CHECK: %pGen1.0 = phi i32 addrspace(4)* [ %6, %if.then ], [ %8, %if.else ]
; CHECK: %10 = bitcast i32 addrspace(1)* %9 to i32 addrspace(1)*
; CHECK: %call11 = call i32 addrspace(4)* @_Z5test2PU3AS1iPU3AS3i(i32 addrspace(1)* %11, i32 addrspace(3)* %12)
; CHECK: %arrayidx6 = getelementptr inbounds i32 addrspace(4)* %call11, i32 8
; CHECK: store i32 8, i32 addrspace(4)* %arrayidx6, align 4
; CHECK-NOT: %call7 = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %13)
; CHECK: %ToNamedPtr = bitcast i8 addrspace(1)* %14 to i8 addrspace(1)*
; CHECK: %cmp9 = icmp eq i32 addrspace(1)* %13, %9
; CHECK: %16 = load i32 addrspace(1)* %arrayidx1312, align 4
; CHECK: %call1616 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %add.ptr14)
; CHECK: ret

; CHECK: @_Z5test1PU3AS1i
; CHECK: %a12 = bitcast i32 addrspace(1)* %a to i32 addrspace(1)*
; CHECK: %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %a12, i32 0
; CHECK: store i32 0, i32 addrspace(1)* %arrayidx3, align 4
; CHECK: ret

; CHECK: @_Z5test1PU3AS3i
; CHECK: %a12 = bitcast i32 addrspace(3)* %a to i32 addrspace(3)*
; CHECK: %arrayidx3 = getelementptr inbounds i32 addrspace(3)* %a12, i32 0
; CHECK: store i32 0, i32 addrspace(3)* %arrayidx3, align 4
; CHECK: ret

; CHECK: addrspace(4)* @_Z5test2PU3AS1iPU3AS3i
; CHECK: %p224 = addrspacecast i32 addrspace(3)* %p223 to i32 addrspace(4)*
; CHECK: %p117 = addrspacecast i32 addrspace(1)* %p116 to i32 addrspace(4)*
; CHECK: %cmp = icmp eq i32 addrspace(4)* %p117, %p224
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)

; CHECK: !{{[0-9]+}} = !{!{{[0-9]+}}, ![[GASCount:[0-9]+]], ![[GASWarnings:[0-9]+]]}
; CHECK: ![[GASCount]] = !{!"gen_addr_space_pointer_counter", i32 3}
; CHECK: ![[GASWarnings]] = !{!"gen_addr_space_pointer_warnings"}

define void @test1(i32 addrspace(4)* %a) nounwind {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %a, i32 0
  store i32 0, i32 addrspace(4)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32 addrspace(4)* %a, i32 1
  store i32 1, i32 addrspace(4)* %arrayidx1, align 4
  ret void
}

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %cmp = icmp eq i32 addrspace(4)* %p1, %p2
  br i1 %cmp, label %if.then, label %if.end

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
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %2, i32 2
  store i32 2, i32 addrspace(4)* %arrayidx, align 4
  %3 = addrspacecast i32 addrspace(4)* %2 to i32 addrspace(1)*
  %arrayidx1 = getelementptr inbounds i32 addrspace(1)* %3, i32 3
  store i32 3, i32 addrspace(1)* %arrayidx1, align 4
  %4 = addrspacecast i32 addrspace(4)* %2 to i32 addrspace(3)*
  %arrayidx2 = getelementptr inbounds i32 addrspace(3)* %4, i32 4
  store i32 4, i32 addrspace(3)* %arrayidx2, align 4
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
  %arrayidx3 = getelementptr inbounds i32 addrspace(4)* %pGen1.0, i32 5
  store i32 5, i32 addrspace(4)* %arrayidx3, align 4
  %7 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %arrayidx4 = getelementptr inbounds i32 addrspace(4)* %7, i32 6
  store i32 6, i32 addrspace(4)* %arrayidx4, align 4
  %8 = ptrtoint i32 addrspace(4)* %7 to i32
  %9 = inttoptr i32 %8 to i32 addrspace(4)*
  %arrayidx5 = getelementptr inbounds i32 addrspace(4)* %9, i32 7
  store i32 7, i32 addrspace(4)* %arrayidx5, align 4
  %10 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %11 = addrspacecast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* %10, i32 addrspace(4)* %11)
  %arrayidx6 = getelementptr inbounds i32 addrspace(4)* %call, i32 8
  store i32 8, i32 addrspace(4)* %arrayidx6, align 4
  %12 = addrspacecast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %13 = bitcast i32 addrspace(4)* %12 to i8 addrspace(4)*
  %call7 = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %13)
  %14 = bitcast i8 addrspace(1)* %call7 to i32 addrspace(1)*
  %tobool8 = icmp ne i32 addrspace(1)* %14, null
  br i1 %tobool8, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end
  %arrayidx10 = getelementptr inbounds i32 addrspace(4)* %12, i32 9
  store i32 9, i32 addrspace(4)* %arrayidx10, align 4
  br label %if.end11

if.end11:                                         ; preds = %if.then9, %if.end
  %cmp = icmp eq i32 addrspace(4)* %12, %7
  br i1 %cmp, label %if.then12, label %if.end15

if.then12:                                        ; preds = %if.end11
  %arrayidx13 = getelementptr inbounds i32 addrspace(4)* %12, i32 8
  %15 = load i32 addrspace(4)* %arrayidx13, align 4
  %arrayidx14 = getelementptr inbounds i32 addrspace(4)* %7, i32 10
  store i32 %15, i32 addrspace(4)* %arrayidx14, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.then12, %if.end11
  %16 = addrspacecast i32 addrspace(1)* %pGlobal to float addrspace(4)*
  %add.ptr = getelementptr inbounds float addrspace(4)* %16, i32 10
  %call16 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
  ret void
}

declare i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test, !1}
!1 = !{!"argument_attribute", i32 0, i32 0, i32 0}
!2 = !{!"-cl-std=CL2.0"}

;;  -----  BasicCases.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCases.cl -o BasicCasesTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesTmp.ll -S -o BasicCases.ll

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
;;  __global int* b2 = to_global(pGen3);
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
