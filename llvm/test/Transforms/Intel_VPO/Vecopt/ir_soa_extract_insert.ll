; RUN: opt -passes="vplan-vec" -vplan-force-vf=4 -S %s | FileCheck %s
; CHECK: vector.body:
; CHECK:  [[EXTRACT1:%.*]] = shufflevector <8 x i32> %{{.*}}, <8 x i32> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; CHECK-NEXT:  [[OP1:%.*]] = sitofp <4 x i32> [[EXTRACT1]] to <4 x float> 
; CHECK:  [[EXTRACT2:%.*]] = shufflevector <8 x i32> %{{.*}}, <8 x i32> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; CHECK-NEXT:  [[OP2:%.*]] = sitofp <4 x i32> [[EXTRACT2]] to <4 x float>
; CHECK:  [[ADD:%.*]] = fadd <4 x float> [[OP2]], [[OP1]]
; CHECK:  [[RES1:%.*]] = fptosi <4 x float> [[ADD]] 
; CHECK:  [[EXTEND:%.*]] = shufflevector <4 x i32> [[RES1]], <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 poison, i32 poison, i32 poison, i32 poison>
; CHECK:  shufflevector <8 x i32> %{{.*}}, <8 x i32> [[EXTEND]], <8 x i32> <i32 0, i32 8, i32 2, i32 9, i32 4, i32 10, i32 6, i32 11>

@arr2p = external global ptr, align 8
@arrB = external global ptr, align 8

define void @foo1()  {
entry:
  %0 = load ptr, ptr @arr2p, align 8
  %ptrToB = load ptr, ptr @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
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
  %3 = extractelement <2 x i32> %2, i32 1
  %a3 = sitofp i32 %3 to float
  %a4 = extractelement <2 x i32> %2, i32 0
  %a5 = sitofp i32 %a4 to float
  %a6 = fadd float %a5, %a3
  %a7 = fptosi float %a6 to i32
  %4 = insertelement <2 x i32>%2, i32 %a7, i32 1
  store <2 x i32> %4, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %arrayidx1 = getelementptr inbounds <2 x i32>, ptr %0, i64 %indvars.iv
  %x2 = load <2 x i32>, ptr %arrayidx1, align 4
  %x3 = add <2 x i32> %x2, <i32 7, i32 8>
  store <2 x i32> %x3, ptr %arrayidx1, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
