; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the inverted ztt is recognized successfully.

; CHECK: + DO i1 = 0, %num_rows + -1, 1   <DO_LOOP>
; CHECK: |   %8 = (%input_buf)[0];
; CHECK: |   %9 = (%8)[i1 + %input_row];
; CHECK: |   %10 = (%input_buf)[1];
; CHECK: |   %11 = (%10)[i1 + %input_row];
; CHECK: |   %12 = (%input_buf)[2];
; CHECK: |   %13 = (%12)[i1 + %input_row];
; CHECK: |
; CHECK: |      %14 = (%output_buf)[i1];
; CHECK: |   + DO i2 = 0, zext.i32.i64((-1 + %2)), 1   <DO_LOOP>
; CHECK: |   |   %15 = (%9)[i2];
; CHECK: |   |   %16 = (%11)[i2];
; CHECK: |   |   %17 = (%13)[i2];
; CHECK: |   |   %18 = (%4)[%17];
; CHECK: |   |   %19 = (%3)[zext.i8.i64(%15) + sext.i32.i64(%18)];
; CHECK: |   |   (%14)[3 * i2] = %19;
; CHECK: |   |   %20 = (%7)[%16];
; CHECK: |   |   %21 = (%6)[%17];
; CHECK: |   |   %22 = (%3)[zext.i8.i64(%15) + sext.i32.i64(trunc.i64.i32(((%21 + %20) /u 65536)))];
; CHECK: |   |   (%14)[3 * i2 + 1] = %22;
; CHECK: |   |   %23 = (%5)[%16];
; CHECK: |   |   %24 = (%3)[zext.i8.i64(%15) + sext.i32.i64(%23)];
; CHECK: |   |   (%14)[3 * i2 + 2] = %24;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; ModuleID = 'module.ll'
source_filename = "djpegv2/jdcolor.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.jpeg_decompress_struct = type { ptr, ptr, ptr, ptr, i8, i32, ptr, i32, i32, i32, i32, i32, i32, i32, double, i8, i8, i32, i8, i8, i8, i32, i8, i32, i8, i8, i8, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32, i32, ptr, [4 x ptr], [4 x ptr], [4 x ptr], i32, ptr, i8, i8, [16 x i8], [16 x i8], [16 x i8], i32, i8, i8, i8, i8, i16, i16, i8, i8, i8, ptr, i32, i32, i32, i32, ptr, i32, [4 x ptr], i32, i32, i32, [10 x i32], i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }
%struct.jpeg_error_mgr = type { ptr, ptr, ptr, ptr, ptr, i32, %union.anon, i32, i64, ptr, i32, ptr, i32, i32 }
%struct.jpeg_common_struct = type { ptr, ptr, ptr, ptr, i8, i32 }
%union.anon = type { [8 x i32], [48 x i8] }
%struct.jpeg_memory_mgr = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64 }
%struct.jvirt_sarray_control = type opaque
%struct.jvirt_barray_control = type opaque
%struct.jpeg_progress_mgr = type { ptr, i64, i64, i32, i32 }
%struct.jpeg_source_mgr = type { ptr, i64, ptr, ptr, ptr, ptr, ptr }
%struct.JQUANT_TBL = type { [64 x i16], i8 }
%struct.JHUFF_TBL = type { [17 x i8], [256 x i8], i8 }
%struct.jpeg_component_info = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8, i32, i32, i32, i32, i32, i32, ptr, ptr }
%struct.jpeg_marker_struct = type { ptr, i8, i32, i32, ptr }
%struct.jpeg_decomp_master = type { ptr, ptr, i8 }
%struct.jpeg_d_main_controller = type { ptr, ptr }
%struct.jpeg_d_coef_controller = type { ptr, ptr, ptr, ptr, ptr }
%struct.jpeg_d_post_controller = type { ptr, ptr }
%struct.jpeg_input_controller = type { ptr, ptr, ptr, ptr, i8, i8 }
%struct.jpeg_marker_reader = type { ptr, ptr, ptr, i8, i8, i32, i32 }
%struct.jpeg_entropy_decoder = type { ptr, ptr, i8 }
%struct.jpeg_inverse_dct = type { ptr, [5 x ptr] }
%struct.jpeg_upsampler = type { ptr, ptr, i8 }
%struct.jpeg_color_deconverter = type { ptr, ptr }
%struct.jpeg_color_quantizer = type { ptr, ptr, ptr, ptr }
%struct.my_color_deconverter = type { %struct.jpeg_color_deconverter, ptr, ptr, ptr, ptr }

; Function Attrs: norecurse nounwind uwtable
define hidden void @ycc_rgb_convert(ptr nocapture readonly %cinfo, ptr nocapture readonly %input_buf, i32 %input_row, ptr nocapture readonly %output_buf, i32 %num_rows) #0 {
entry:
  %cconvert1 = getelementptr inbounds %struct.jpeg_decompress_struct, ptr %cinfo, i64 0, i32 86
  %0 = bitcast ptr %cconvert1 to ptr
  %1 = load ptr, ptr %0, align 8
  %output_width = getelementptr inbounds %struct.jpeg_decompress_struct, ptr %cinfo, i64 0, i32 27
  %2 = load i32, ptr %output_width, align 4
  %sample_range_limit = getelementptr inbounds %struct.jpeg_decompress_struct, ptr %cinfo, i64 0, i32 65
  %3 = load ptr, ptr %sample_range_limit, align 8
  %Cr_r_tab = getelementptr inbounds %struct.my_color_deconverter, ptr %1, i64 0, i32 1
  %4 = load ptr, ptr %Cr_r_tab, align 8
  %Cb_b_tab = getelementptr inbounds %struct.my_color_deconverter, ptr %1, i64 0, i32 2
  %5 = load ptr, ptr %Cb_b_tab, align 8
  %Cr_g_tab = getelementptr inbounds %struct.my_color_deconverter, ptr %1, i64 0, i32 3
  %6 = load ptr, ptr %Cr_g_tab, align 8
  %Cb_g_tab = getelementptr inbounds %struct.my_color_deconverter, ptr %1, i64 0, i32 4
  %7 = load ptr, ptr %Cb_g_tab, align 8
  %cmp83 = icmp sgt i32 %num_rows, 0
  br i1 %cmp83, label %while.body.lr.ph, label %while.end

while.body.lr.ph:                                 ; preds = %entry
  %arrayidx4 = getelementptr inbounds ptr, ptr %input_buf, i64 1
  %arrayidx7 = getelementptr inbounds ptr, ptr %input_buf, i64 2
  %cmp979 = icmp eq i32 %2, 0
  br label %while.body

while.cond.loopexit.loopexit:                     ; preds = %for.body
  br label %while.cond.loopexit

while.cond.loopexit:                              ; preds = %while.body, %while.cond.loopexit.loopexit
  %cmp = icmp sgt i32 %dec86.in, 1
  br i1 %cmp, label %while.body, label %while.end.loopexit

while.body:                                       ; preds = %while.cond.loopexit, %while.body.lr.ph
  %dec86.in = phi i32 [ %num_rows, %while.body.lr.ph ], [ %dec86, %while.cond.loopexit ]
  %input_row.addr.085 = phi i32 [ %input_row, %while.body.lr.ph ], [ %inc, %while.cond.loopexit ]
  %output_buf.addr.084 = phi ptr [ %output_buf, %while.body.lr.ph ], [ %incdec.ptr, %while.cond.loopexit ]
  %dec86 = add nsw i32 %dec86.in, -1
  %idxprom = zext i32 %input_row.addr.085 to i64
  %8 = load ptr, ptr %input_buf, align 8
  %arrayidx2 = getelementptr inbounds ptr, ptr %8, i64 %idxprom
  %9 = load ptr, ptr %arrayidx2, align 8
  %10 = load ptr, ptr %arrayidx4, align 8
  %arrayidx5 = getelementptr inbounds ptr, ptr %10, i64 %idxprom
  %11 = load ptr, ptr %arrayidx5, align 8
  %12 = load ptr, ptr %arrayidx7, align 8
  %arrayidx8 = getelementptr inbounds ptr, ptr %12, i64 %idxprom
  %13 = load ptr, ptr %arrayidx8, align 8
  %inc = add i32 %input_row.addr.085, 1
  %incdec.ptr = getelementptr inbounds ptr, ptr %output_buf.addr.084, i64 1
  br i1 %cmp979, label %while.cond.loopexit, label %for.body.preheader

for.body.preheader:                               ; preds = %while.body
  %14 = load ptr, ptr %output_buf.addr.084, align 8
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %outptr.081 = phi ptr [ %14, %for.body.preheader ], [ %add.ptr, %for.body ]
  %arrayidx11 = getelementptr inbounds i8, ptr %9, i64 %indvars.iv
  %15 = load i8, ptr %arrayidx11, align 1
  %conv = zext i8 %15 to i32
  %arrayidx13 = getelementptr inbounds i8, ptr %11, i64 %indvars.iv
  %16 = load i8, ptr %arrayidx13, align 1
  %arrayidx16 = getelementptr inbounds i8, ptr %13, i64 %indvars.iv
  %17 = load i8, ptr %arrayidx16, align 1
  %idxprom18 = zext i8 %17 to i64
  %arrayidx19 = getelementptr inbounds i32, ptr %4, i64 %idxprom18
  %18 = load i32, ptr %arrayidx19, align 4
  %add = add nsw i32 %18, %conv
  %idxprom20 = sext i32 %add to i64
  %arrayidx21 = getelementptr inbounds i8, ptr %3, i64 %idxprom20
  %19 = load i8, ptr %arrayidx21, align 1
  store i8 %19, ptr %outptr.081, align 1
  %idxprom23 = zext i8 %16 to i64
  %arrayidx24 = getelementptr inbounds i64, ptr %7, i64 %idxprom23
  %20 = load i64, ptr %arrayidx24, align 8
  %arrayidx26 = getelementptr inbounds i64, ptr %6, i64 %idxprom18
  %21 = load i64, ptr %arrayidx26, align 8
  %add27 = add nsw i64 %21, %20
  %shr78 = lshr i64 %add27, 16
  %conv28 = trunc i64 %shr78 to i32
  %add29 = add nsw i32 %conv28, %conv
  %idxprom30 = sext i32 %add29 to i64
  %arrayidx31 = getelementptr inbounds i8, ptr %3, i64 %idxprom30
  %22 = load i8, ptr %arrayidx31, align 1
  %arrayidx32 = getelementptr inbounds i8, ptr %outptr.081, i64 1
  store i8 %22, ptr %arrayidx32, align 1
  %arrayidx34 = getelementptr inbounds i32, ptr %5, i64 %idxprom23
  %23 = load i32, ptr %arrayidx34, align 4
  %add35 = add nsw i32 %23, %conv
  %idxprom36 = sext i32 %add35 to i64
  %arrayidx37 = getelementptr inbounds i8, ptr %3, i64 %idxprom36
  %24 = load i8, ptr %arrayidx37, align 1
  %arrayidx38 = getelementptr inbounds i8, ptr %outptr.081, i64 2
  store i8 %24, ptr %arrayidx38, align 1
  %add.ptr = getelementptr inbounds i8, ptr %outptr.081, i64 3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %2
  br i1 %exitcond, label %while.cond.loopexit.loopexit, label %for.body

while.end.loopexit:                               ; preds = %while.cond.loopexit
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}


