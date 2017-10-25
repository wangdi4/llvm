; RUN: opt -VPlanDriver -S %s | FileCheck %s

; CHECK: vector.body:
; CHECK:  shufflevector <8 x i32> %transposed.wide.masked.load, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK:  sitofp <4 x i32> 
; CHECK:  shufflevector <8 x i32> %transposed.wide.masked.load, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK:  sitofp <4 x i32> 
; CHECK:  fadd <4 x float> 
; CHECK:  %[[RES1:.*]] = fptosi <4 x float> 
; CHECK:  %[[RES2:.*]] = shufflevector <4 x i32> %[[RES1]], <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 4, i32 4, i32 4>
; CHECK:  shufflevector <8 x i32> %transposed.wide.masked.load, <8 x i32> %[[RES2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>

@arr2p = external global <2 x i32>*, align 8
@arrB = external global i32*, align 8

define void @foo1()  {
entry:
  %0 = load <2 x i32>*, <2 x i32>** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
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
  %arrayidxB = getelementptr inbounds i32, i32* %ptrToB, i64 %indvars.iv
  %B = load i32, i32* %arrayidxB, align 4
  %tobool = icmp eq i32 %B, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %indvars.iv
  %2 = load <2 x i32>, <2 x i32>* %arrayidx, align 4
  %3 = extractelement <2 x i32> %2, i32 1
  %a3 = sitofp i32 %3 to float
  %a4 = extractelement <2 x i32> %2, i32 0
  %a5 = sitofp i32 %a4 to float
  %a6 = fadd float %a5, %a3
  %a7 = fptosi float %a6 to i32
  %4 = insertelement <2 x i32>%2, i32 %a7, i32 1
  store <2 x i32> %4, <2 x i32>* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %arrayidx1 = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %indvars.iv
  %x2 = load <2 x i32>, <2 x i32>* %arrayidx1, align 4
  %x3 = add <2 x i32> %x2, <i32 7, i32 8>
  store <2 x i32> %x3, <2 x i32>* %arrayidx1, align 4

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

