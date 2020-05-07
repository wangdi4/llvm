; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %s -S -o - \
; RUN: | FileCheck %s
; ModuleID = 'main'
source_filename = "2"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

; Function Attrs: convergent
declare <2 x i32> @_Z28intel_sub_group_shuffle_downDv2_iS_j(<2 x i32> %0, <2 x i32> %1, i32 %2) local_unnamed_addr #2

; Function Attrs: convergent nounwind readnone
declare i32 @_Z3allDv2_i(<2 x i32> %0) local_unnamed_addr #1

declare i64 @_Z14get_local_sizej(i32 %0)

declare i64 @get_base_global_id.(i32 %0)

; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel(i32 addrspace(1)* noalias %shuffle_results) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %call1 = tail call i32 @_Z18get_sub_group_sizev() #5
  %call2 = tail call i32 @_Z22get_sub_group_local_idv() #5
  %add = add i32 %call2, 1
  %add31 = add i32 %add, 100
  %add32 = add i32 %add, 100
  %0 = call <2 x i32> @fake.insert.element0(<2 x i32> undef, i32 %add, i32 0) #3
  %1 = call <2 x i32> @fake.insert.element1(<2 x i32> %0, i32 %add, i32 1) #3
  %2 = call <2 x i32> @fake.insert.element2(<2 x i32> undef, i32 %add31, i32 0) #3
  %3 = call <2 x i32> @fake.insert.element3(<2 x i32> %2, i32 %add32, i32 1) #3
; CHECK: [[A01:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 0
; CHECK: [[A02:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> undef, i32 [[A01]], i32 0
; CHECK: [[A03:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 1
; CHECK: [[A04:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A02]], i32 [[A03]], i32 2
; CHECK: [[A05:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 2
; CHECK: [[A06:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A04]], i32 [[A05]], i32 4
; CHECK: [[A07:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 3
; CHECK: [[A08:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A06]], i32 [[A07]], i32 6
; CHECK: [[A09:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 0
; CHECK: [[A10:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A08]], i32 [[A09]], i32 1
; CHECK: [[A11:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 1
; CHECK: [[A12:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A10]], i32 [[A11]], i32 3
; CHECK: [[A13:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 2
; CHECK: [[A14:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A12]], i32 [[A13]], i32 5
; CHECK: [[A15:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add{{[0-9]*}}, i32 3
; CHECK: [[A16:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[A14]], i32 [[A15]], i32 7

; CHECK: [[B01:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 0
; CHECK: [[B02:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> undef, i32 [[B01]], i32 0
; CHECK: [[B03:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 1
; CHECK: [[B04:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B02]], i32 [[B03]], i32 2
; CHECK: [[B05:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 2
; CHECK: [[B06:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B04]], i32 [[B05]], i32 4
; CHECK: [[B07:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 3
; CHECK: [[B08:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B06]], i32 [[B07]], i32 6
; CHECK: [[B09:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 0
; CHECK: [[B10:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B08]], i32 [[B09]], i32 1
; CHECK: [[B11:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 1
; CHECK: [[B12:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B10]], i32 [[B11]], i32 3
; CHECK: [[B13:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 2
; CHECK: [[B14:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B12]], i32 [[B13]], i32 5
; CHECK: [[B15:%[a-zA-Z0-9_]+]] = extractelement <4 x i32> %add3{{[0-9]*}}, i32 3
; CHECK: [[B16:%[a-zA-Z0-9_]+]] = insertelement <8 x i32> [[B14]], i32 [[B15]], i32 7
  %call4_clone = tail call <2 x i32> @_Z28intel_sub_group_shuffle_downDv2_iS_j(<2 x i32> %1, <2 x i32> %3, i32 0) #4
; CHECK: [[R0:%[a-zA-Z0-9_]+]] = call <8 x i32> @_Z28intel_sub_group_shuffle_downDv8_iS_Dv4_jS0_(<8 x i32> [[A16]], <8 x i32> [[B16]], <4 x i32> zeroinitializer, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: [[R01:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 0
; CHECK: [[R02:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> undef, i32 [[R01]], i32 0
; CHECK: [[R03:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 2
; CHECK: [[R04:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R02]], i32 [[R03]], i32 1
; CHECK: [[R05:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 4
; CHECK: [[R06:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R04]], i32 [[R05]], i32 2
; CHECK: [[R07:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 6
; CHECK: [[R08:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R06]], i32 [[R07]], i32 3
; CHECK: [[R09:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 1
; CHECK: [[R10:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> undef, i32 [[R09]], i32 0
; CHECK: [[R11:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 3
; CHECK: [[R12:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R10]], i32 [[R11]], i32 1
; CHECK: [[R13:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 5
; CHECK: [[R14:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R12]], i32 [[R13]], i32 2
; CHECK: [[R15:%[a-zA-Z0-9_]+]] = extractelement <8 x i32> [[R0]], i32 7
; CHECK: [[R16:%[a-zA-Z0-9_]+]] = insertelement <4 x i32> [[R14]], i32 [[R15]], i32 3
  %4 = call i32 @fake.extract.element0(<2 x i32> %call4_clone, i32 0) #3
  %5 = call i32 @fake.extract.element1(<2 x i32> %call4_clone, i32 1) #3
  %cmp6 = icmp ult i32 %call2, %call1
  %sub = sub i32 100, %call1
  %add11 = select i1 %cmp6, i32 0, i32 %sub
  %6 = add i32 %add, %add11
  %cmp143 = icmp eq i32 %4, %6
  %cmp144 = icmp eq i32 %5, %6
  %sext5 = sext i1 %cmp143 to i32
  %sext6 = sext i1 %cmp144 to i32
  %7 = call <2 x i32> @fake.insert.element4(<2 x i32> undef, i32 %sext5, i32 0) #3
  %8 = call <2 x i32> @fake.insert.element5(<2 x i32> %7, i32 %sext6, i32 1) #3
  %call15 = tail call i32 @_Z3allDv2_i(<2 x i32> %8) #4
  %tobool = icmp ne i32 %call15, 0
  %9 = zext i1 %tobool to i32
  %arrayidx22 = getelementptr inbounds i32, i32 addrspace(1)* %shuffle_results, i64 %call
  store i32 %9, i32 addrspace(1)* %arrayidx22, align 4, !tbaa !12
  ret void
}

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element0(<2 x i32> %0, i32 %1, i32 %2) #3

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element1(<2 x i32> %0, i32 %1, i32 %2) #3

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element2(<2 x i32> %0, i32 %1, i32 %2) #3

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element3(<2 x i32> %0, i32 %1, i32 %2) #3

; Function Attrs: nounwind readnone
declare i32 @fake.extract.element0(<2 x i32> %0, i32 %1) #3

; Function Attrs: nounwind readnone
declare i32 @fake.extract.element1(<2 x i32> %0, i32 %1) #3

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element4(<2 x i32> %0, i32 %1, i32 %2) #3

; Function Attrs: nounwind readnone
declare <2 x i32> @fake.insert.element5(<2 x i32> %0, i32 %1, i32 %2) #3

declare i1 @__ocl_allOne(i1 %0)

declare i1 @__ocl_allZero(i1 %0)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros
-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "
stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-bu
ffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}

!0 = !{i32 1, i32 2}
!1 = !{}
!4 = !{i32 1}
!5 = !{!"none"}
!6 = !{!"uint*"}
!7 = !{!""}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{!"shuffle_results"}
!11 = !{i1 true}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{null}
