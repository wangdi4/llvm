; RUN: opt -passes=vplan-vec -vplan-force-vf=4 -S %s | FileCheck %s

; CHECK-LABEL: @foo1
; CHECK:  %tmp.vec = alloca <8 x i32>, align 4
; CHECK: %tmp = alloca <2 x i32>, align 4
; CHECK: vector.ph:                                        ; preds = %min.iters.checked
; CHECK:   %tmpInitVal = load <2 x i32>, ptr %tmp
; CHECK:   %[[TransposeAndSplat:.*]] = shufflevector <2 x i32> %tmpInitVal
; CHECK:   store <8 x i32> %[[TransposeAndSplat]], ptr %tmp.vec

; CHECK: vector.body:                                      ; preds = %vector.body, %vector.ph
; CHECK:   %replicatedMaskElts. = shufflevector <4 x i1> {{.*}}, <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:   %wide.masked.load = call <8 x i32> @llvm.masked.load.v8i32.p0({{.*}} <8 x i1> %replicatedMaskElts.
; CHECK:   call void @llvm.masked.store.v8i32.p0(<8 x i32> %wide.masked.load, ptr %tmp.vec, i32 4, <8 x i1> %[[ReplicatedMaskVec:.*]])

; CHECK: middle.block:                                     ; preds = %VPlannedBB{{[0-9]*}}
; CHECK:   %[[MASK_INT:.*]] = load i4, ptr %tmp.mask
; CHECK:   %ctlz = call i4 @llvm.ctlz.i4(i4 %[[MASK_INT]], i1 true)
; CHECK:   %LaneToCopyFrom = sub i4 3, %ctlz
; CHECK:   %PtrInsidePrivVec = getelementptr i32, ptr %tmp.vec, i4 %LaneToCopyFrom
; CHECK:   %LastVal = load i32, ptr %PtrInsidePrivVec
; CHECK:   store i32 %LastVal, ptr %tmp
; CHECK:   %[[LaneToCopyFrom5:.*]] = add i4 %LaneToCopyFrom, 4
; CHECK:   %[[PtrInsidePrivVec6:.*]] = getelementptr i32, ptr %tmp.vec, i4 %[[LaneToCopyFrom5]]
; CHECK:   %[[LastVal7:.*]] = load i32, ptr %[[PtrInsidePrivVec6]]
; CHECK:   %PtrToNextEltInOrigPrivate = getelementptr i32, ptr %tmp, i32 1
; CHECK:   store i32 %[[LastVal7]], ptr %PtrToNextEltInOrigPrivate

; XFAIL: *
; TODO: This test makes assumptions about SOA layout of the code. This will be enabled when we remove code which assumes SOA layout in Vector Codegen


@arr2p = external global ptr, align 8
@arrB = external global ptr, align 8

define <2 x i32> @foo1()  {
entry:
  %tmp = alloca <2 x i32>, align 4
  store <2 x i32><i32 5, i32 6>, ptr %tmp, align 4
  %0 = load ptr, ptr @arr2p, align 8
  %ptrToB = load ptr, ptr @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LASTPRIVATE:CONDITIONAL", ptr nonnull %tmp)
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
  %arrayidx = getelementptr inbounds <2 x i32>, ptr %0, i64 %indvars.iv
  %2 = load <2 x i32>, ptr %arrayidx, align 4
  store <2 x i32> %2, ptr %tmp, align 4
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

  %res = load <2 x i32>, ptr %tmp 
  ret <2 x i32> %res
}

; This test checks load/store in private space though a bitcast, 
; when the bitcast operaion converts vector to scalar.
; CHECK-LABEL: @foo2
; CHECK: %tmp.vec = alloca <8 x i32>, align 8
; CHECK: %[[WideLoad:.*]] = call <8 x i32> @llvm.masked.load.v8i32.p0(ptr
; CHECK:  %[[ToStore:.*]] = bitcast <8 x i32> %[[WideLoad:.*]] to <4 x i64>

; CHECK: call void @llvm.masked.store.v4i64.p0(<4 x i64> %[[ToStore]]

define <2 x i32> @foo2()  {
entry:
  %tmp = alloca <2 x i32>, align 8
  store <2 x i32><i32 5, i32 6>, ptr %tmp, align 4
  %0 = load ptr, ptr @arr2p, align 8
  %ptrToB = load ptr, ptr @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LASTPRIVATE:CONDITIONAL", ptr nonnull %tmp)
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
  %arrayidx = getelementptr inbounds <2 x i32>, ptr %0, i64 %indvars.iv
  %2 = load <2 x i32>, ptr %arrayidx, align 4
  %3 = bitcast <2 x i32> %2 to i64
  store i64 %3, ptr %tmp, align 8
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

  %res = load <2 x i32>, ptr %tmp 
  ret <2 x i32> %res
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

