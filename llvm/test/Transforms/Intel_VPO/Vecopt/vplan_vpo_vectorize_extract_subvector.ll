; Test to check that extract-element instruction, in incoming IR is correctly handled.
; We want to check that the right reduce-shuffle is generated.

; RUN: opt -S -VPlanDriver -vplan-force-vf=8 -enable-vp-value-codegen=false %s | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
; RUN: opt -S -VPlanDriver -vplan-force-vf=8 -enable-vp-value-codegen %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPVALUE


; CHECK:              [[E:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK-LLVM-NEXT:    [[C:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr:%.*]], <2 x i32> [[E]])
; CHECK-VPVALUE-NEXT: [[C:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr:%.*]], <2 x i32> [[E]])
; CHECK:              [[E1:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 2, i32 3>
; CHECK-LLVM-NEXT:    [[C1:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E1]])
; CHECK-VPVALUE-NEXT: [[C1:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E1]])
; CHECK:              [[E2:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 4, i32 5>
; CHECK-LLVM-NEXT:    [[C2:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E2]])
; CHECK-VPVALUE-NEXT: [[C2:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E2]])
; CHECK:              [[E3:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 6, i32 7>
; CHECK-LLVM-NEXT:    [[C3:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E3]])
; CHECK-VPVALUE-NEXT: [[C3:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E3]])
; CHECK: [[E4:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 8, i32 9>
; CHECK-LLVM-NEXT:    [[C4:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E4]])
; CHECK-VPVALUE-NEXT: [[C4:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E4]])
; CHECK:              [[E5:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 10, i32 11>
; CHECK-LLVM-NEXT:    [[C5:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E5]])
; CHECK-VPVALUE-NEXT: [[C5:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E5]])
; CHECK:              [[E6:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 12, i32 13>
; CHECK-LLVM-NEXT:    [[C6:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E6]])
; CHECK-VPVALUE-NEXT: [[C6:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E6]])
; CHECK: [[E7:%.*]] = shufflevector <16 x i32> [[VAR:%.*]], <16 x i32> undef, <2 x i32> <i32 14, i32 15>
; CHECK-LLVM-NEXT:    [[C7:%.*]] = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E7]])
; CHECK-VPVALUE-NEXT: [[C7:%.*]] = call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* [[srcimg:%.*]], %SPLR_Ty addrspace(2)* [[splr]], <2 x i32> [[E7]])

; CHECK-LLVM: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg:%.*]], <2 x i32> [[E]], <4 x float> [[C]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E1]], <4 x float> [[C1]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E2]], <4 x float> [[C2]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E3]], <4 x float> [[C3]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E4]], <4 x float> [[C4]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E5]], <4 x float> [[C5]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E6]], <4 x float> [[C6]])
; CHECK-LLVM-NEXT: tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E7]], <4 x float> [[C7]])


; CHECK-VPVALUE: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg:%.*]], <2 x i32> [[E]], <4 x float> [[C]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E1]], <4 x float> [[C1]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E2]], <4 x float> [[C2]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E3]], <4 x float> [[C3]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E4]], <4 x float> [[C4]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E5]], <4 x float> [[C5]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E6]], <4 x float> [[C6]])
; CHECK-VPVALUE-NEXT: call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* [[dstimg]], <2 x i32> [[E7]], <4 x float> [[C7]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%IMG_R_Ty = type opaque
%IMG_W_Ty = type opaque
%SPLR_Ty = type opaque

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i32 @_Z15get_image_width14ocl_image2d_wo(%IMG_W_Ty addrspace(1)*) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i32 @_Z16get_image_height14ocl_image2d_wo(%IMG_W_Ty addrspace(1)*) local_unnamed_addr #1

; Function Attrs: convergent nounwind readonly
declare <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)*, %SPLR_Ty addrspace(2)*, <2 x i32>) local_unnamed_addr #2

; Function Attrs: convergent
declare void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)*, <2 x i32>, <4 x float>) local_unnamed_addr #3

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent nounwind
define void @_ZGVdN8uuu_test_rgba8888(%IMG_R_Ty addrspace(1)* %srcimg, %IMG_W_Ty addrspace(1)* %dstimg, %SPLR_Ty addrspace(2)* %splr) {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(%IMG_R_Ty addrspace(1)* %srcimg, %IMG_W_Ty addrspace(1)* %dstimg, %SPLR_Ty addrspace(2)* %splr) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  %conv = trunc i64 %add to i32
  %call1 = tail call i64 @_Z13get_global_idj(i32 1) #5
  %conv2 = trunc i64 %call1 to i32
  %assembled.vect = insertelement <2 x i32> undef, i32 %conv, i32 0
  %assembled.vect11 = insertelement <2 x i32> %assembled.vect, i32 %conv2, i32 1
  %call9 = tail call <4 x float> @_Z11read_imagef14ocl_image2d_ro11ocl_splrDv2_i(%IMG_R_Ty addrspace(1)* %srcimg, %SPLR_Ty addrspace(2)* %splr, <2 x i32> %assembled.vect11) #6
  tail call void @_Z12write_imagef14ocl_image2d_woDv2_iDv4_f(%IMG_W_Ty addrspace(1)* %dstimg, <2 x i32> %assembled.vect11, <4 x float> %call9) #7
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.unroll.disable"}
