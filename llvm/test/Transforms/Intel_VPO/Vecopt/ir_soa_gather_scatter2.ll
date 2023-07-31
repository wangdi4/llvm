; RUN: opt -passes=vplan-vec -vplan-force-vf=4 -S %s | FileCheck %s


; CHECK: vector.body:
; CHECK:   [[MASK1:%.*]] = shufflevector <4 x i1> %[[MASK:.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:   %[[G_V_PTRS:.*]] = shufflevector <4 x ptr> %{{.*}}, <4 x ptr> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK:   %[[G_GEP:.*]] = getelementptr i32, <8 x ptr> %[[G_V_PTRS]], <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:   %wide.masked.gather = call <8 x i32> @llvm.masked.gather.v8i32.v8p0(<8 x ptr> %[[G_GEP]], i32 4, <8 x i1> [[MASK1]], <8 x i32> undef)
; CHECK:   %[[VALUE_TO_STORE:.*]] = add <8 x i32> %wide.masked.gather, <i32 5, i32 5, i32 5, i32 5, i32 6, i32 6, i32 6, i32 6>
; CHECK:   %[[ReplicatedMaskVec:.*]] = shufflevector <4 x i1> %[[MASK:.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:   %[[S_V_PTRS:.*]] = shufflevector <4 x ptr> %{{.*}}, <4 x ptr> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK:   %[[S_GEP:.*]] = getelementptr i32, <8 x ptr> %[[S_V_PTRS]], <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:   call void @llvm.masked.scatter.v8i32.v8p0(<8 x i32> %[[VALUE_TO_STORE]], <8 x ptr> %[[S_GEP]], i32 4, <8 x i1> %[[ReplicatedMaskVec]])


; XFAIL: *
; TODO: This test makes assumptions about SOA layout of the code. This will be enabled when we remove code which assumes SOA layout in Vector Codegen

@varr2p = external global ptr, align 8
@sarr2p = external global ptr, align 8
@arrB = external global ptr, align 8

define void @foo1()  {
entry:
  %0 = load ptr, ptr @varr2p, align 8
  %s0 = load ptr, ptr @sarr2p, align 8
  %ptrToB = load ptr, ptr @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.inc ]
  %1 = trunc i64 %indvars.iv to i32
  %arrayidxB = getelementptr inbounds i32, ptr %ptrToB, i64 %indvars.iv
  %B = load i32, ptr %arrayidxB, align 4
  %tobool = icmp eq i32 %B, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %s0, i32 %B
  %2 = load <2 x i32>, ptr %arrayidx, align 4
  %3 = add <2 x i32> %2, <i32 5, i32 6>
  store <2 x i32> %3, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit

  ret void
}


; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

