; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test
; CHECK: %0 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(1)
; CHECK: call void @_Z5test1PU3AS1i(i32 addrspace(1)* %0)
; CHECK: %1 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(3)
; CHECK: call void @_Z5test1PU3AS3i(i32 addrspace(3)* %1)
; CHECK: %3 = getelementptr inbounds i32 addrspace(1)* %2, i32 2
; CHECK: store i32 2, i32 addrspace(1)* %3, align 4
; CHECK: %4 = bitcast i32 addrspace(1)* %2 to i32 addrspace(1)*
; CHECK: %5 = bitcast i32 addrspace(1)* %2 to i32 addrspace(3)*
; CHECK: %7 = bitcast i32 addrspace(1)* %6 to i32 addrspace(4)*
; CHECK: %pGen1.0 = phi i32 addrspace(4)* [ %7, %if.then ], [ %9, %if.else ]
; CHECK: %12 = ptrtoint i32 addrspace(1)* %10 to i32
; CHECK: %13 = inttoptr i32 %12 to i32 addrspace(4)*
; CHECK: %16 = call zeroext i1 @_Z5test2PU3AS1iPU3AS3i(i32 addrspace(1)* %14, i32 addrspace(3)* %15)
; CHECK-NOT: call zeroext i1 @_Z9is_globalPKU3AS4v
; CHECK: %frombool11 = zext i1 true to i8
; CHECK: %20 = icmp eq i32 addrspace(1)* %17, %10
; CHECK: %22 = load i32 addrspace(1)* %21, align 4
; CHECK: %26 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %25)
; CHECK: ret

; CHECK: @_Z5test1PU3AS1i
; CHECK: %0 = bitcast i32 addrspace(1)* %a to i32 addrspace(1)*
; CHECK: %1 = getelementptr inbounds i32 addrspace(1)* %0, i32 0
; CHECK: store i32 0, i32 addrspace(1)* %1, align 4
; CHECK: ret

; CHECK: @_Z5test1PU3AS3i
; CHECK: %0 = bitcast i32 addrspace(3)* %a to i32 addrspace(3)*
; CHECK: %1 = getelementptr inbounds i32 addrspace(3)* %0, i32 0
; CHECK: store i32 0, i32 addrspace(3)* %1, align 4
; CHECK: ret

; CHECK: @_Z5test2PU3AS1iPU3AS3i
; CHECK: %1 = bitcast i32 addrspace(3)* %0 to i32 addrspace(4)*
; CHECK: %3 = bitcast i32 addrspace(1)* %2 to i32 addrspace(4)*
; CHECK: %cmp = icmp eq i32 addrspace(4)* %3, %1
; CHECK: ret

; CHECK: declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)
; CHECK: !opencl.compiler.2_0.gen_addr_space_pointer_warnings = !{!3}
; CHECK: !opencl.compiler.2_0.gen_addr_space_pointer_counter = !{!4}
; CHECK: !3 = metadata !{i32 0}
; CHECK: !4 = metadata !{i32 2}
  

define void @test1(i32 addrspace(4)* %a) nounwind {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %a, i32 0
  store i32 0, i32 addrspace(4)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32 addrspace(4)* %a, i32 1
  store i32 1, i32 addrspace(4)* %arrayidx1, align 4
  ret void
}

define zeroext i1 @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %cmp = icmp eq i32 addrspace(4)* %p1, %p2
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i1 [ true, %if.then ], [ false, %if.end ]
  ret i1 %retval.0
}

define void @test(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %0 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %0)
  %1 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  call void @test1(i32 addrspace(4)* %1)
  %2 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %2, i32 2
  store i32 2, i32 addrspace(4)* %arrayidx, align 4
  %3 = bitcast i32 addrspace(4)* %2 to i32 addrspace(1)*
  %arrayidx1 = getelementptr inbounds i32 addrspace(1)* %3, i32 3
  store i32 3, i32 addrspace(1)* %arrayidx1, align 4
  %4 = bitcast i32 addrspace(4)* %2 to i32 addrspace(3)*
  %arrayidx2 = getelementptr inbounds i32 addrspace(3)* %4, i32 4
  store i32 4, i32 addrspace(3)* %arrayidx2, align 4
  %tobool = fcmp une float %param, 0.000000e+00
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %5 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  br label %if.end

if.else:                                          ; preds = %entry
  %6 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %pGen1.0 = phi i32 addrspace(4)* [ %5, %if.then ], [ %6, %if.else ]
  %arrayidx3 = getelementptr inbounds i32 addrspace(4)* %pGen1.0, i32 5
  store i32 5, i32 addrspace(4)* %arrayidx3, align 4
  %7 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %arrayidx4 = getelementptr inbounds i32 addrspace(4)* %7, i32 6
  store i32 6, i32 addrspace(4)* %arrayidx4, align 4
  %8 = ptrtoint i32 addrspace(4)* %7 to i32
  %9 = inttoptr i32 %8 to i32 addrspace(4)*
  %arrayidx5 = getelementptr inbounds i32 addrspace(4)* %9, i32 7
  store i32 7, i32 addrspace(4)* %arrayidx5, align 4
  %10 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %11 = bitcast i32 addrspace(3)* %pLocal to i32 addrspace(4)*
  %call = call zeroext i1 @test2(i32 addrspace(4)* %10, i32 addrspace(4)* %11)
  %frombool = zext i1 %call to i8
  %tobool6 = trunc i8 %frombool to i1
  br i1 %tobool6, label %if.then7, label %if.end9

if.then7:                                         ; preds = %if.end
  %arrayidx8 = getelementptr inbounds i32 addrspace(4)* %9, i32 8
  store i32 8, i32 addrspace(4)* %arrayidx8, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then7, %if.end
  %12 = bitcast i32 addrspace(1)* %pGlobal to i32 addrspace(4)*
  %13 = bitcast i32 addrspace(4)* %12 to i8 addrspace(4)*
  %call10 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %13)
  %frombool11 = zext i1 %call10 to i8
  %tobool12 = trunc i8 %frombool11 to i1
  br i1 %tobool12, label %if.then13, label %if.end15

if.then13:                                        ; preds = %if.end9
  %arrayidx14 = getelementptr inbounds i32 addrspace(4)* %12, i32 9
  store i32 9, i32 addrspace(4)* %arrayidx14, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.then13, %if.end9
  %cmp = icmp eq i32 addrspace(4)* %12, %7
  br i1 %cmp, label %if.then16, label %if.end19

if.then16:                                        ; preds = %if.end15
  %arrayidx17 = getelementptr inbounds i32 addrspace(4)* %12, i32 8
  %14 = load i32 addrspace(4)* %arrayidx17, align 4
  %arrayidx18 = getelementptr inbounds i32 addrspace(4)* %7, i32 10
  store i32 %14, i32 addrspace(4)* %arrayidx18, align 4
  br label %if.end19

if.end19:                                         ; preds = %if.then16, %if.end15
  %15 = bitcast i32 addrspace(1)* %pGlobal to float addrspace(4)*
  %add.ptr = getelementptr inbounds float addrspace(4)* %15, i32 10
  %call20 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr)
  ret void
}

declare zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @test, metadata !1}
!1 = metadata !{metadata !"argument_attribute", i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"-cl-std=CL2.0"}

;;  -----  BasicCases.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCases.cl -o BasicCasesTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesTmp.ll -S -o BasicCases.ll

;;void test1(int* a) {
;;  a[0] = 0;
;;  a[1] = 1;
;;}

;;bool test2(int *p1, int *p2) {
;;  if (p1 == p2)
;;    return true;
;;  return false;
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
;;  bool b1 = test2(pGlobal, pLocal);
;;  if (b1) {
;;    pGen3[8] = 8;
;;  }

;;  // Address Specifier BI
;;  pGen3 = pGlobal;
;;  bool b2 = is_global(pGen3);
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
