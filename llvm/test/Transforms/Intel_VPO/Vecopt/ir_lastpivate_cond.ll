; RUN: opt -VPlanDriver -S -vpo-codegen -vplan-predicator %s | FileCheck %s
; This test checks for a widened alloca and a wide store to the widened alloca
; CHECK:  %[[PRIV:.*]] = alloca i32, align 4
; CHECK: vector.ph
; CHECK:  %[[VEC_PRIV:.*]] = alloca <4 x i32>
; CHECK: vector.body

; CHECK:   %[[MASKINT:.*]] = bitcast <4 x i1> %[[MASK:.*]] to i4
; CHECK:   %[[NOT_ZERO:.*]] = icmp ne i4 %22, 0
; CHECK:   call void @llvm.masked.store.v4i32.p0v4i32(<4 x i32> {{.*}}, <4 x i32>* %tmp.vec, i32 4, <4 x i1> %[[MASK]])
; CHECK:   %[[MASKINT2:.*]] = bitcast <4 x i1> %[[MASK:.*]] to i4
; CHECK:   br i1 %[[NOT_ZERO]], label %pred.store.if, label 

; CHECK: pred.store.if:                                    ; preds = %vector.body
; CHECK:   store i4 %[[MASKINT2]], i4* %tmp.mask
; CHECK:   br label 
; CHECK: middle.block

; CHECK: %[[LAST_MASK:.*]] = load i4, i4* %tmp.mask
; CHECK: %ctlz = call i4 @llvm.ctlz.i4(i4 %[[LAST_MASK]], i1 true)
; CHECK: %[[IDX:.*]] = sub i4 3, %ctlz
; CHECK: %LastVal = load i32
; CHECK: store i32 %LastVal, i32* %tmp

@arr2p = external global i32*, align 8
@arrB = external global i32*, align 8

define i32 @foo1()  {
entry:
  %tmp = alloca i32, align 4
  store i32 5, i32* %tmp, align 4
  %0 = load i32*, i32** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LASTPRIVATE:CONDITIONAL", i32* nonnull %tmp)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.inc ]
  %1 = trunc i64 %indvars.iv to i32
  %arrayidxB = getelementptr inbounds i32, i32* %ptrToB, i64 %indvars.iv
  %B = load i32, i32* %arrayidxB, align 4
  %tobool = icmp eq i32 %B, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %2, %3
  store i32 %add, i32* %tmp, align 4
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

  %res = load i32, i32 *%tmp 
  ret i32 %res
}


; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

