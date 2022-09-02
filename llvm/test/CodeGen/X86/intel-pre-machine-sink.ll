; RUN: llc -mtriple=x86_64 -mattr=+avx512vl,+avx512dq,+avx512bw  -stop-after=machine-sink < %s -o - | FileCheck %s --check-prefixes=AVX512
; RUN: llc -mtriple=x86_64 -mattr=+avx2  -stop-after=machine-sink < %s -o - | FileCheck %s --check-prefixes=AVX2
%class.Node = type { i32 }
declare i8 @llvm.cttz.i8(i8, i1 immarg)
declare i16 @llvm.cttz.i16(i16, i1 immarg)

define i1 @sink_test_0(<16 x i64> %.splat1613, <16 x i64> %mul, <16 x i64> %cmp) "prefer-vector-width"="512" {
; AVX512:     entry
; AVX512-NOT: KUNPCKBWrr
; AVX512:     KORTESTBrr
; AVX2:     entry
; AVX2-NOT: VPACKSSDWYrr
; AVX2:     VPORYrr
; AVX2:     VMOVMSKPDYrr
entry:
  %mul.res = add <16 x i64> %.splat1613, %mul
  %hir.cmp.1050 = icmp ugt <16 x i64> %mul.res, %cmp
  %bc = bitcast <16 x i1> %hir.cmp.1050 to i16
  %hir.cmp.1054.not = icmp eq i16 %bc, 0
  br i1 %hir.cmp.1054.not, label %ifmerge.1057, label %then.1057
ifmerge.1057:
  ret i1 0
then.1057:
  ret i1 1
}

define i1 @sink_test_1(<16 x double> %gepload7562) "prefer-vector-width"="256" {
; AVX512:     entry
; AVX512-NOT: KUNPCKBWrr
; AVX512:     KORTESTBrr
; AVX2:     entry
; AVX2-NOT: VPACKSSDWYrr
; AVX2:     VPORYrr
; AVX2:     VMOVMSKPDYrr
entry:
  %fcmp.0 = fcmp olt <16 x double> %gepload7562, <double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15, double 1.000000e-15>
  %bc = bitcast <16 x i1> %fcmp.0 to i16
  %hir.cmp.3158.not = icmp eq i16 %bc, 0
  br i1 %hir.cmp.3158.not, label %ifmerge.3159, label %then.3159
ifmerge.3159:
  ret i1 0
then.3159:
  ret i1 1
}
; AVX512:    sink_stdfind_ptr_vf16_512
define %class.Node** @sink_stdfind_ptr_vf16_512(%class.Node** %base, %class.Node* %a, i64 %n, i64 %c) "prefer-vector-width"="512" {
then.284:                                          ; preds = %then.88
  %nn = add nsw i64 %n, -1
  %.splatinsert = insertelement <16 x %class.Node*> poison, %class.Node* %a, i64 0
  %.splat534 = shufflevector <16 x %class.Node*> %.splatinsert, <16 x %class.Node*> poison, <16 x i32> zeroinitializer
  %.splatinsert_8 = insertelement <8 x %class.Node*> poison, %class.Node* %a, i64 0
  %.splat534_8 = shufflevector <8 x %class.Node*> %.splatinsert_8, <8 x %class.Node*> poison, <8 x i32> zeroinitializer
  br label %loop.286
; AVX512:     loop.286
; AVX512-NOT: KUNPCKBWrr
; AVX512:     KORTESTBrr
loop.286:                                         ; preds = %then.284, %ifmerge.291
  %i1.i64507.1 = phi i64 [ %nextivloop.286, %ifmerge.291 ], [ 0, %then.284 ]
  %addr45 = add i64 %c, %i1.i64507.1
  %addr46 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %addr45
  %addr47 = bitcast %class.Node** %addr46 to <16 x %class.Node*>*
  %gepload531 = load <16 x %class.Node*>, <16 x %class.Node*>* %addr47, align 8
  %hir.cmp.289 = icmp eq <16 x %class.Node*> %gepload531, %.splat534
  %cmp48 = bitcast <16 x i1> %hir.cmp.289 to i16
  %hir.cmp.291.not = icmp eq i16 %cmp48, 0
  br i1 %hir.cmp.291.not, label %ifmerge.291, label %then.291
; AVX512:     then.291
; AVX512: KUNPCKBWrr
then.291:                                         ; preds = %loop.286
  %cttz49 = call i16 @llvm.cttz.i16(i16 %cmp48, i1 true)
  %cttz50 = zext i16 %cttz49 to i64
  %addr51 = add i64 %addr45, %cttz50
  %addr52 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %addr51
  ret %class.Node** %addr52

ifmerge.291:                                      ; preds = %loop.286
  %nextivloop.286 = add nuw i64 %i1.i64507.1, 16
  %condloop.286.not = icmp ugt i64 %nextivloop.286, %nn
  br i1 %condloop.286.not, label %exit, label %loop.286

exit:
  ret %class.Node** %base
}
; AVX512: sink_stdfind_ptr_vf16_256
; AVX2: sink_stdfind_ptr_vf16_256
define %class.Node** @sink_stdfind_ptr_vf16_256(%class.Node** %base, %class.Node* %a, i64 %n, i64 %c) "prefer-vector-width"="256" {
then.284:                                          ; preds = %then.88
  %nn = add nsw i64 %n, -1
  %.splatinsert = insertelement <16 x %class.Node*> poison, %class.Node* %a, i64 0
  %.splat534 = shufflevector <16 x %class.Node*> %.splatinsert, <16 x %class.Node*> poison, <16 x i32> zeroinitializer
  %.splatinsert_8 = insertelement <8 x %class.Node*> poison, %class.Node* %a, i64 0
  %.splat534_8 = shufflevector <8 x %class.Node*> %.splatinsert_8, <8 x %class.Node*> poison, <8 x i32> zeroinitializer
  br label %loop.286
; AVX512:     bb.1.loop.286
; AVX512-NOT: KSHIFTLBri
; AVX512:     KORBrr
; AVX512:     KORTESTBrr
; AVX2:     bb.1.loop.286
; AVX2-NOT: VPACKSSDWYrr
; AVX2:     VPORYrr
; AVX2:     VMOVMSKPDYrr
loop.286:                                         ; preds = %then.284, %ifmerge.291
  %i1.i64507.1 = phi i64 [ %nextivloop.286, %ifmerge.291 ], [ 0, %then.284 ]
  %addr45 = add i64 %c, %i1.i64507.1
  %addr46 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %addr45
  %addr47 = bitcast %class.Node** %addr46 to <16 x %class.Node*>*
  %gepload531 = load <16 x %class.Node*>, <16 x %class.Node*>* %addr47, align 8
  %hir.cmp.289 = icmp eq <16 x %class.Node*> %gepload531, %.splat534
  %cmp48 = bitcast <16 x i1> %hir.cmp.289 to i16
  %hir.cmp.291.not = icmp eq i16 %cmp48, 0
  br i1 %hir.cmp.291.not, label %ifmerge.291, label %then.291
; AVX512:     then.291
; AVX512: KSHIFTLBri
; AVX2:     then.291
; AVX2: VPACKSSDWYrr
then.291:                                         ; preds = %loop.286
  %cttz49 = call i16 @llvm.cttz.i16(i16 %cmp48, i1 true)
  %cttz50 = zext i16 %cttz49 to i64
  %addr51 = add i64 %addr45, %cttz50
  %addr52 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %addr51
  ret %class.Node** %addr52

ifmerge.291:                                      ; preds = %loop.286
  %nextivloop.286 = add nuw i64 %i1.i64507.1, 16
  %condloop.286.not = icmp ugt i64 %nextivloop.286, %nn
  br i1 %condloop.286.not, label %exit, label %loop.286

exit:
  ret %class.Node** %base
}

; AVX512:      sink_stdfind_ptr_vf8_256
; AVX2:      sink_stdfind_ptr_vf8_256
define %class.Node** @sink_stdfind_ptr_vf8_256(%class.Node** %base, %class.Node* %a, i64 %n, i64 %c)  "prefer-vector-width"="256" {
then.91:                                          ; preds = %then.88
  %nn = add nsw i64 %n, -1
  %.splatinsert = insertelement <8 x %class.Node*> poison, %class.Node* %a, i64 0
  %.splat = shufflevector <8 x %class.Node*> %.splatinsert, <8 x %class.Node*> poison, <8 x i32> zeroinitializer
  br label %loop
; AVX512:      loop
; AVX512-NOT:  KSHIFTLBri
; AVX512-NOT:  KORBrr
; AVX512:      KORTESTBrr
; AVX2:      loop
; AVX2-NOT:  VPACKSSDWYrr
; AVX2:  VPORYrr
; AVX2:      VMOVMSKPDYrr
loop:                                          ; preds = %ifmerge.95, %then.91  ltl
  %i1.i64.1 = phi i64 [ 0, %then.91 ], [ %nextivloop.90, %ifmerge.95 ]
  %addr42 = add i64 %c, %i1.i64.1
  %addr43 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %i1.i64.1
  %addr44 = bitcast %class.Node** %addr43 to <8 x %class.Node*>*
  %gepload259 = load <8 x %class.Node*>, <8 x %class.Node*>* %addr44, align 8
  %hir.cmp.93 = icmp eq <8 x %class.Node*> %gepload259, %.splat
  %cmp45 = bitcast <8 x i1> %hir.cmp.93 to i8
  %hir.cmp.95.not = icmp eq i8 %cmp45, 0
  br i1 %hir.cmp.95.not, label %ifmerge.95, label %then.95
; AVX512:      then.95
; AVX512:      KSHIFTLBri
; AVX512:      KORBrr
; AVX2:      then.95
; AVX2:      VPACKSSDWYrr
then.95:                                          ; preds = %loop
  %cttz46 = call i8 @llvm.cttz.i8(i8 %cmp45, i1 true)
  %cttz47 = zext i8 %cttz46 to i64
  %addr48 = add i64 %addr42, %cttz47
  %addr49 = getelementptr inbounds %class.Node*, %class.Node** %base, i64 %addr48
  ret %class.Node** %addr49

ifmerge.95:                                       ; preds = %loop
  %nextivloop.90 = add nuw i64 %i1.i64.1, 8
  %condloop.90.not = icmp ugt i64 %nextivloop.90, %nn
  br i1 %condloop.90.not, label %exit, label %loop

exit:
  ret %class.Node** %base
}
