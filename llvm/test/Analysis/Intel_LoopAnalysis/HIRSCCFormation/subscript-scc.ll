; RUN: opt < %s -analyze -enable-new-pm=0 -hir-scc-formation | FileCheck %s
; RUN: opt < %s -passes="print<hir-scc-formation>" 2>&1 -disable-output | FileCheck %s

; Check that SCC containing a subscript call is identified.

; CHECK: Region 1
; CHECK: SCC1: %ptr2 -> %ptr1

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.blam = type { i8**, %struct.blam*, i32, i32, i32 }

@global = external hidden unnamed_addr global %struct.blam*, align 4

define dso_local i8* @eggs(i32 %arg, i8* %arg1, i32 %arg2) local_unnamed_addr {
bb:
  %ptr0 = alloca i8, align 4
  br label %bb3

bb3:
  %tmp4 = load %struct.blam*, %struct.blam** @global, align 4
  %tmp5 = getelementptr inbounds %struct.blam, %struct.blam* %tmp4, i32 0, i32 3
  %tmp6 = load i32, i32* %tmp5, align 4
  %tmp7 = bitcast i8* %ptr0 to i8**
  %tmp8 = add i32 %tmp6, -1
  %tmp9 = mul i32 %tmp8, %arg
  br label %loop

loop:
  %tmp11 = phi i32 [ %tmp16, %loop ], [ %tmp8, %bb3 ]
  %ptr1 = phi i8* [ %ptr2, %loop ], [ %ptr0, %bb3 ]
  %tmp13 = phi i8** [ %tmp15, %loop ], [ %tmp7, %bb3 ]
  %ptr2 = call i8* @llvm.intel.subscript.p0i8.i32.i32.p0i8.i32(i8 0, i32 0, i32 1, i8* elementtype(i8) %ptr1, i32 %arg)
  store i8* %ptr2, i8** %tmp13, align 4
  %tmp15 = bitcast i8* %ptr2 to i8**
  %tmp16 = add nsw i32 %tmp11, -1
  %tmp17 = icmp sgt i32 %tmp16, 0
  br i1 %tmp17, label %loop, label %loop_exit

loop_exit:
  br label %bb19

bb19:
  ret i8* null
}

declare i8* @llvm.intel.subscript.p0i8.i32.i32.p0i8.i32(i8, i32, i32, i8*, i32)

