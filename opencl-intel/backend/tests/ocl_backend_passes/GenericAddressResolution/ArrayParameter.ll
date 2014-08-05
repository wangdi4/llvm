; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @func
; CHECK: %0 = bitcast i32 addrspace(1)* %add.ptr to i32 addrspace(1)*
; CHECK: %1 = addrspacecast i32 addrspace(1)* %0 to i32 addrspace(4)*
; CHECK: store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx, align 4
; CHECK: %8 = addrspacecast i32 addrspace(4)** %arraydecay to i32 addrspace(4)* addrspace(4)*
; CHECK: %9 = addrspacecast i32 addrspace(4)** %arraydecay7 to i32 addrspace(4)* addrspace(4)*
; CHECK: %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %8, i32 addrspace(4)* addrspace(4)* %9)
; CHECK: ret

; ModuleID = 'ArrayParameter.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %p1, i32 addrspace(4)* addrspace(4)* %p2) nounwind {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* addrspace(4)* %p1, i32 5
  %0 = load i32 addrspace(4)* addrspace(4)* %arrayidx, align 4
  %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  %call = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %1)
  %tobool = icmp ne i8 addrspace(1)* %call, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx1 = getelementptr inbounds i32 addrspace(4)* addrspace(4)* %p1, i32 6
  %2 = load i32 addrspace(4)* addrspace(4)* %arrayidx1, align 4
  br label %return

if.end:                                           ; preds = %entry
  %arrayidx2 = getelementptr inbounds i32 addrspace(4)* addrspace(4)* %p2, i32 7
  %3 = load i32 addrspace(4)* addrspace(4)* %arrayidx2, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 addrspace(4)* [ %2, %if.then ], [ %3, %if.end ]
  ret i32 addrspace(4)* %retval.0
}

declare i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)*)

define void @func(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %ptrs1 = alloca [10 x i32 addrspace(4)*], align 4
  %ptrs2 = alloca [10 x i32 addrspace(4)*], align 4
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
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %pGlobal, i32 %i.0
  %0 = addrspacecast i32 addrspace(1)* %add.ptr to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs1, i32 0, i32 %i.0
  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
  %add.ptr1 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %1 = addrspacecast i32 addrspace(3)* %add.ptr1 to i32 addrspace(4)*
  %arrayidx2 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs2, i32 0, i32 %i.0
  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add.ptr3 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %2 = addrspacecast i32 addrspace(3)* %add.ptr3 to i32 addrspace(4)*
  %arrayidx4 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs1, i32 0, i32 %i.0
  store i32 addrspace(4)* %2, i32 addrspace(4)** %arrayidx4, align 4
  %add.ptr5 = getelementptr inbounds i32 addrspace(1)* %pGlobal, i32 %i.0
  %3 = addrspacecast i32 addrspace(1)* %add.ptr5 to i32 addrspace(4)*
  %arrayidx6 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs2, i32 0, i32 %i.0
  store i32 addrspace(4)* %3, i32 addrspace(4)** %arrayidx6, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arraydecay = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs1, i32 0, i32 0
  %4 = addrspacecast i32 addrspace(4)** %arraydecay to i32 addrspace(4)* addrspace(4)*
  %arraydecay7 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs2, i32 0, i32 0
  %5 = addrspacecast i32 addrspace(4)** %arraydecay7 to i32 addrspace(4)* addrspace(4)*
  %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %4, i32 addrspace(4)* addrspace(4)* %5)
  %6 = bitcast i32 addrspace(4)* %call to i8 addrspace(4)*
  %call8 = call i8* @_Z10to_privatePKU3AS4v(i8 addrspace(4)* %6)
  %7 = bitcast i8* %call8 to i32*
  ret void
}

declare i8* @_Z10to_privatePKU3AS4v(i8 addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @func}

;;  -----  ArrayParameter.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 ArrayParameter.cl -o ArrayParameterTmp.ll
;;               oclopt.exe -mem2reg -verify ArrayParameterTmp.ll -S -o ArrayParameter.ll

;;int* test2(int **p1, int **p2) {
;;  if (to_global(p1[5]))
;;    return p1[6];
;;  return p2[7];
;;}

;;__kernel void func(__global int *pGlobal, __local int *pLocal, float param) {
;;  int* ptrs1[10];
;;  int* ptrs2[10];
;;  for (int i = 0; i < 10; i++) {
  
;;    // Initialization
;;    if (i%2) {
;;       ptrs1[i] = pGlobal + i;
;;       ptrs2[i] = pLocal + i;
;;    } else {
;;       ptrs1[i] = pLocal + i;
;;       ptrs2[i] = pGlobal + i;
;;    }
;;  }

;;  int* pGen5 = test2(ptrs1, ptrs2);
;;  __private int* d = to_private(pGen5);
 
;;}
