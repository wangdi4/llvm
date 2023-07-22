; RUN: opt -passes='function(sycl-kernel-phi-canonicalization),sycl-kernel-barrier' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

;;*****************************************************************************
;; This test is used to prove that the use of PhiCanon before KernelBarrier is necessary
;;*****************************************************************************

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @_ZTSN4sycl3_V16detail9reduction7MainKrnI20kernel_name_identityINS1_9half_impl4halfENS0_7minimumIS6_EEELNS2_8strategyE1EJNS0_8accessorIiLi1ELNS0_6access4modeE1026ELNSC_6targetE2014ELNSC_11placeholderE0ENS0_3ext6oneapi22accessor_property_listIJEEEEEEEE() {
entry:
  br label %for.body.i.i.i

for.body.i.i.i:                                   ; preds = %for.body.i.i.i, %entry
  br label %for.body.i.i.i

Split.Barrier.BB25:                               ; preds = %if.end29.i
  call void @dummy_barrier.()
  br i1 false, label %if.then11.i, label %if.end29.i

if.then11.i:                                      ; preds = %Split.Barrier.BB25
  br label %if.end29.i

if.else.i:                                        ; No predecessors!
  br label %if.end29.i

if.end29.i:                                       ; preds = %if.else.i, %if.then11.i, %Split.Barrier.BB25
  %Reducer.sroa.0.2.i = phi half [ 0xH0000, %if.then11.i ], [ %CallFinalizeWG4, %if.else.i ], [ %CallFinalizeWG4, %Split.Barrier.BB25 ]
  %CallFinalizeWG4 = call half null(half 0xH0000)
  br label %Split.Barrier.BB25

; uselistorder directives
  uselistorder half %CallFinalizeWG4, { 1, 0 }
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

declare void @dummy_barrier.()

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }

; DEBUGIFY: PASS
