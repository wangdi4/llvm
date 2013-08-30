; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK-NOT: @test2

; CHECK: @func
; CHECK: %AllocaSpace10 = alloca i32
; CHECK: %ptrs1 = alloca [10 x i32 addrspace(4)*], align 4
; CHECK: %AllocaSpace = mul i32 10, 1
; CHECK: %AllocaSpace1 = alloca i32, i32 %AllocaSpace
; CHECK: %AllocaSpace2 = ptrtoint [10 x i32 addrspace(4)*]* %ptrs1 to i64
; CHECK: %AllocaSpace3 = ptrtoint i32* %AllocaSpace1 to i64
; CHECK: %AllocaSpace4 = sub i64 %AllocaSpace3, %AllocaSpace2
; CHECK: %ptrs2 = alloca [10 x i32 addrspace(4)*], align 4
; CHECK: %AllocaSpace5 = mul i32 10, 1
; CHECK: %AllocaSpace6 = alloca i32, i32 %AllocaSpace5
; CHECK: %AllocaSpace7 = ptrtoint [10 x i32 addrspace(4)*]* %ptrs2 to i64
; CHECK: %AllocaSpace8 = ptrtoint i32* %AllocaSpace6 to i64
; CHECK: %AllocaSpace9 = sub i64 %AllocaSpace8, %AllocaSpace7
; CHECK: store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
; CHECK: %AllocaSpace14 = ptrtoint i32 addrspace(4)** %arrayidx to i64
; CHECK: %AllocaSpace15 = add i64 %AllocaSpace14, %AllocaSpace4
; CHECK: %AllocaSpace16 = inttoptr i64 %AllocaSpace15 to i32*
; CHECK: store i32 1, i32* %AllocaSpace16
; CHECK: %AllocaSpace20 = ptrtoint i32 addrspace(4)** %arrayidx2 to i64
; CHECK: %AllocaSpace21 = add i64 %AllocaSpace20, %AllocaSpace9
; CHECK: %AllocaSpace22 = inttoptr i64 %AllocaSpace21 to i32*
; CHECK: store i32 3, i32* %AllocaSpace22
; CHECK-NOT: %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %4, i32 addrspace(4)* addrspace(4)* %5)
; CHECK: %6 = call i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0iS0_S0_(i32 addrspace(4)* addrspace(4)* %4, i32 addrspace(4)* addrspace(4)* %5, i32* %AllocaSpace1, i32* %AllocaSpace6, i32* %AllocaSpace10)
; CHECK: %AddrSpace = load i32* %AllocaSpace10
; CHECK-NOT: %call8 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %6)
; CHECK: %AddrSpace23 = icmp eq i32 %AddrSpace, 0
; CHECK: ret

; CHECK: define i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0iS0_S0_(i32 addrspace(4)* addrspace(4)* %p1, i32 addrspace(4)* addrspace(4)* %p2, i32* %ArgSpace, i32* %ArgSpace3, i32* %ArgSpace20) nounwind {
; CHECK: %AllocaSpace4 = ptrtoint i32 addrspace(4)* addrspace(4)* %p2 to i64
; CHECK: %AllocaSpace5 = ptrtoint i32* %ArgSpace3 to i64
; CHECK: %AllocaSpace6 = sub i64 %AllocaSpace5, %AllocaSpace4
; CHECK: %AllocaSpace = ptrtoint i32 addrspace(4)* addrspace(4)* %p1 to i64
; CHECK: %AllocaSpace1 = ptrtoint i32* %ArgSpace to i64
; CHECK: %AllocaSpace2 = sub i64 %AllocaSpace1, %AllocaSpace
; CHECK: %0 = load i32 addrspace(4)* addrspace(4)* %arrayidx, align 4
; CHECK: %AllocaSpace15 = ptrtoint i32 addrspace(4)* addrspace(4)* %arrayidx to i64
; CHECK: %AllocaSpace16 = add i64 %AllocaSpace15, %AllocaSpace2
; CHECK: %AllocaSpace17 = inttoptr i64 %AllocaSpace16 to i32*
; CHECK: %AddrSpace18 = load i32* %AllocaSpace17
; CHECK: %AddrSpace19 = icmp eq i32 %AddrSpace18, 1
; CHECK: %2 = load i32 addrspace(4)* addrspace(4)* %arrayidx1, align 4
; CHECK: %AllocaSpace10 = ptrtoint i32 addrspace(4)* addrspace(4)* %arrayidx1 to i64
; CHECK: %AllocaSpace11 = add i64 %AllocaSpace10, %AllocaSpace2
; CHECK: %AllocaSpace12 = inttoptr i64 %AllocaSpace11 to i32*
; CHECK: %AddrSpace13 = load i32* %AllocaSpace12
; CHECK: %3 = load i32 addrspace(4)* addrspace(4)* %arrayidx2, align 4
; CHECK: %AllocaSpace7 = ptrtoint i32 addrspace(4)* addrspace(4)* %arrayidx2 to i64
; CHECK: %AllocaSpace8 = add i64 %AllocaSpace7, %AllocaSpace6
; CHECK: %AllocaSpace9 = inttoptr i64 %AllocaSpace8 to i32*
; CHECK: %AddrSpace = load i32* %AllocaSpace9
; CHECK: %retval.0 = phi i32 addrspace(4)* [ %2, %if.then ], [ %3, %if.end ]
; CHECK: %AddrSpace14 = phi i32 [ %AddrSpace13, %if.then ], [ %AddrSpace, %if.end ]
; CHECK: store i32 %AddrSpace14, i32* %ArgSpace20
; CHECK: ret i32 addrspace(4)* %retval.0

; ModuleID = 'ArrayParameter.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %p1, i32 addrspace(4)* addrspace(4)* %p2) nounwind {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* addrspace(4)* %p1, i32 5
  %0 = load i32 addrspace(4)* addrspace(4)* %arrayidx, align 4
  %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %1)
  br i1 %call, label %if.then, label %if.end

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

declare zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)*)

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
  %0 = bitcast i32 addrspace(1)* %add.ptr to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs1, i32 0, i32 %i.0
  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
  %add.ptr1 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %1 = bitcast i32 addrspace(3)* %add.ptr1 to i32 addrspace(4)*
  %arrayidx2 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs2, i32 0, i32 %i.0
  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add.ptr3 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %2 = bitcast i32 addrspace(3)* %add.ptr3 to i32 addrspace(4)*
  %arrayidx4 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs1, i32 0, i32 %i.0
  store i32 addrspace(4)* %2, i32 addrspace(4)** %arrayidx4, align 4
  %add.ptr5 = getelementptr inbounds i32 addrspace(1)* %pGlobal, i32 %i.0
  %3 = bitcast i32 addrspace(1)* %add.ptr5 to i32 addrspace(4)*
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
  %4 = bitcast i32 addrspace(4)** %arraydecay to i32 addrspace(4)* addrspace(4)*
  %arraydecay7 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs2, i32 0, i32 0
  %5 = bitcast i32 addrspace(4)** %arraydecay7 to i32 addrspace(4)* addrspace(4)*
  %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %4, i32 addrspace(4)* addrspace(4)* %5)
  %6 = bitcast i32 addrspace(4)* %call to i8 addrspace(4)*
  %call8 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %6)
  %frombool = zext i1 %call8 to i8
  ret void
}

declare zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @func}

;;  -----  ArrayParameter.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 ArrayParameter.cl -o ArrayParameterTmp.ll
;;               oclopt.exe -mem2reg -verify ArrayParameterTmp.ll -S -o ArrayParameter.ll

;;int* test2(int **p1, int **p2) {
;;  if (is_global(p1[5]))
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
;;  bool d = is_private(pGen5);
 
;;}
