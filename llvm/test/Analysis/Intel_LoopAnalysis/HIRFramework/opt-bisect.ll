; RUN: opt -passes='default<O3>' -opt-bisect-limit=0 -S < %s 2>&1 | FileCheck %s

; Verify that the test passes when HIR deconstruction is disabled in opt-bisect mode.
; The issue is that without deconstruction the incoming IR may not be in the right form (consummable by HIR Framework) which can lead to assertion (like in this case).
; We work around the issue by discarding formed IRRegions before bailing out of HIRSSADeconstruction in opt-bisect mode.

; Any check is fine as we only want to verify that the opt command is successful.
; CHECK: datalayout

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%"class.dealii::Polynomials::Polynomial.19" = type { %"class.dealii::Subscriptor", %"class.std::vector.51" }
%"class.dealii::Subscriptor" = type { ptr, i32, %"class.std::map", ptr }
%"class.std::map" = type { %"class.std::_Rb_tree" }
%"class.std::_Rb_tree" = type { %"struct.std::_Rb_tree<const char *, std::pair<const char *const, unsigned int>, std::_Select1st<std::pair<const char *const, unsigned int> >, std::less<const char *>, std::allocator<std::pair<const char *const, unsigned int> > >::_Rb_tree_impl" }
%"struct.std::_Rb_tree<const char *, std::pair<const char *const, unsigned int>, std::_Select1st<std::pair<const char *const, unsigned int> >, std::less<const char *>, std::allocator<std::pair<const char *const, unsigned int> > >::_Rb_tree_impl" = type { %"struct.std::less", %"struct.std::_Rb_tree_node_base", i32 }
%"struct.std::less" = type { i8 }
%"struct.std::_Rb_tree_node_base" = type { i32, ptr, ptr, ptr }
%"class.std::type_info" = type { ptr, ptr }
%"class.std::vector.51" = type { %"struct.std::_Vector_base.4.104" }
%"struct.std::_Vector_base.4.104" = type { %"struct.std::_Vector_base<double, std::allocator<double> >::_Vector_impl" }
%"struct.std::_Vector_base<double, std::allocator<double> >::_Vector_impl" = type { ptr, ptr, ptr }
%"class.dealii::PolynomialSpace" = type { %"class.std::vector.32", i32, %"class.std::vector.8.550", %"class.std::vector.8.550" }
%"class.std::vector.32" = type { %"struct.std::_Vector_base.33" }
%"struct.std::_Vector_base.33" = type { %"struct.std::_Vector_base<dealii::Polynomials::Polynomial<double>, std::allocator<dealii::Polynomials::Polynomial<double> > >::_Vector_impl" }
%"struct.std::_Vector_base<dealii::Polynomials::Polynomial<double>, std::allocator<dealii::Polynomials::Polynomial<double> > >::_Vector_impl" = type { ptr, ptr, ptr }
%"class.std::vector.8.550" = type { %"struct.std::_Vector_base.9.549" }
%"struct.std::_Vector_base.9.549" = type { %"struct.std::_Vector_base<unsigned int, std::allocator<unsigned int> >::_Vector_impl" }
%"struct.std::_Vector_base<unsigned int, std::allocator<unsigned int> >::_Vector_impl" = type { ptr, ptr, ptr }
%"class.dealii::Point.40" = type { %"class.dealii::Tensor.39" }
%"class.dealii::Tensor.39" = type { [3 x double] }

$_ZNK6dealii15PolynomialSpaceILi3EE13compute_valueEjRKNS_5PointILi3EEE = comdat any

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #0

; Function Attrs: uwtable
declare dso_local double @_ZNK6dealii11Polynomials10PolynomialIdE5valueEd(ptr, double) local_unnamed_addr #1 align 2

; Function Attrs: uwtable
define weak_odr dso_local double @_ZNK6dealii15PolynomialSpaceILi3EE13compute_valueEjRKNS_5PointILi3EEE(ptr %this, i32 %i, ptr dereferenceable(24) %p) local_unnamed_addr #1 comdat align 2 {
entry:
  %ix = alloca [3 x i32], align 4
  %0 = bitcast ptr %ix to ptr
  call void @llvm.lifetime.start.p0(i64 12, ptr nonnull %0) #2
  %_M_start.i.i = getelementptr inbounds %"class.dealii::PolynomialSpace", ptr %this, i32 0, i32 2, i32 0, i32 0, i32 0
  %1 = load ptr, ptr %_M_start.i.i, align 4, !tbaa !3, !std.container.ptr !9
  %add.ptr.i.i = getelementptr inbounds i32, ptr %1, i32 %i
  %2 = load i32, ptr %add.ptr.i.i, align 4, !tbaa !10, !std.container.ptr !9
  %_M_finish.i.i = getelementptr inbounds %"class.dealii::PolynomialSpace", ptr %this, i32 0, i32 0, i32 0, i32 0, i32 1
  %3 = bitcast ptr %_M_finish.i.i to ptr
  %4 = load i32, ptr %3, align 4, !tbaa !12
  %5 = bitcast ptr %this to ptr
  %6 = load i32, ptr %5, align 4, !tbaa !16
  %sub.ptr.sub.i.i = sub i32 %4, %6
  %sub.ptr.div.i.i = sdiv exact i32 %sub.ptr.sub.i.i, 48
  %cmp60.i = icmp eq i32 %sub.ptr.div.i.i, 0
  br i1 %cmp60.i, label %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit, label %for.body.lr.ph.i

for.body.lr.ph.i:                                 ; preds = %entry
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc16.i, %for.body.lr.ph.i
  %iz.0.neg64.i = phi i32 [ 0, %for.body.lr.ph.i ], [ %iz.0.neg.i, %for.inc16.i ]
  %iz.062.i = phi i32 [ 0, %for.body.lr.ph.i ], [ %inc17.i, %for.inc16.i ]
  %k.061.i = phi i32 [ 0, %for.body.lr.ph.i ], [ %k.1.lcssa.i, %for.inc16.i ]
  %sub.i = sub i32 %sub.ptr.div.i.i, %iz.062.i
  %cmp456.i = icmp eq i32 %sub.i, 0
  br i1 %cmp456.i, label %for.inc16.i, label %for.body6.lr.ph.i

for.body6.lr.ph.i:                                ; preds = %for.body.i
  %add.i = add i32 %iz.0.neg64.i, %sub.ptr.div.i.i
  br label %for.body6.i

for.body6.i:                                      ; preds = %if.else.i, %for.body6.lr.ph.i
  %iy.0.neg59.i = phi i32 [ 0, %for.body6.lr.ph.i ], [ %iy.0.neg.i, %if.else.i ]
  %iy.058.i = phi i32 [ 0, %for.body6.lr.ph.i ], [ %inc.i, %if.else.i ]
  %k.157.i = phi i32 [ %k.061.i, %for.body6.lr.ph.i ], [ %sub8.i, %if.else.i ]
  %sub7.i = add i32 %add.i, %iy.0.neg59.i
  %sub8.i = add i32 %sub7.i, %k.157.i
  %cmp9.i = icmp ult i32 %2, %sub8.i
  br i1 %cmp9.i, label %if.then.i, label %if.else.i

if.then.i:                                        ; preds = %for.body6.i
  %sub10.i = sub i32 %2, %k.157.i
  %arrayidx.i = getelementptr inbounds [3 x i32], ptr %ix, i32 0, i32 0
  store i32 %sub10.i, ptr %arrayidx.i, align 4, !tbaa !17
  %arrayidx11.i = getelementptr inbounds [3 x i32], ptr %ix, i32 0, i32 1
  store i32 %iy.058.i, ptr %arrayidx11.i, align 4, !tbaa !17
  %arrayidx12.i = getelementptr inbounds [3 x i32], ptr %ix, i32 0, i32 2
  store i32 %iz.062.i, ptr %arrayidx12.i, align 4, !tbaa !17
  br label %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit

if.else.i:                                        ; preds = %for.body6.i
  %inc.i = add nuw i32 %iy.058.i, 1
  %iy.0.neg.i = xor i32 %iy.058.i, -1
  %cmp4.i = icmp ult i32 %inc.i, %sub.i
  br i1 %cmp4.i, label %for.body6.i, label %for.inc16.i

for.inc16.i:                                      ; preds = %if.else.i, %for.body.i
  %k.1.lcssa.i = phi i32 [ %k.061.i, %for.body.i ], [ %sub8.i, %if.else.i ]
  %inc17.i = add nuw i32 %iz.062.i, 1
  %iz.0.neg.i = xor i32 %iz.062.i, -1
  %cmp.i = icmp ugt i32 %sub.ptr.div.i.i, %inc17.i
  br i1 %cmp.i, label %for.body.i, label %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit

_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit: ; preds = %for.inc16.i, %if.then.i, %entry
  %_M_start.i = getelementptr inbounds %"class.dealii::PolynomialSpace", ptr %this, i32 0, i32 0, i32 0, i32 0, i32 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @llvm.lifetime.end.p0(i64 12, ptr nonnull %0) #2
  ret double %mul

for.body:                                         ; preds = %for.body, %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit
  %d.016 = phi i32 [ 0, %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit ], [ %inc, %for.body ]
  %result.015 = phi double [ 1.000000e+00, %_ZNK6dealii15PolynomialSpaceILi3EE13compute_indexEjRA3_j.exit ], [ %mul, %for.body ]
  %arrayidx = getelementptr inbounds [3 x i32], ptr %ix, i32 0, i32 %d.016
  %7 = load i32, ptr %arrayidx, align 4, !tbaa !17
  %8 = load ptr, ptr %_M_start.i, align 4, !tbaa !16, !std.container.ptr !19
  %add.ptr.i = getelementptr inbounds %"class.dealii::Polynomials::Polynomial.19", ptr %8, i32 %7
  %arrayidx.i10 = getelementptr inbounds %"class.dealii::Point.40", ptr %p, i32 0, i32 0, i32 0, i32 %d.016
  %9 = load double, ptr %arrayidx.i10, align 4, !tbaa !20
  %call3 = tail call fast double @_ZNK6dealii11Polynomials10PolynomialIdE5valueEd(ptr nonnull %add.ptr.i, double %9)
  %mul = fmul fast double %call3, %result.015
  %inc = add nuw nsw i32 %d.016, 1
  %exitcond = icmp eq i32 %inc, 3
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { argmemonly nounwind }
attributes #1 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e0d70afddc425d01ddbadc5ebdbff99bd3f76e36)"}
!1 = !{i32 1, !"NumRegisterParameters", i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{!4, !6, i64 0}
!4 = !{!"struct@_ZTSSt12_Vector_baseIjSaIjEE", !5, i64 0}
!5 = !{!"struct@_ZTSNSt12_Vector_baseIjSaIjEE12_Vector_implE", !6, i64 0, !6, i64 4, !6, i64 8}
!6 = !{!"pointer@_ZTSPj", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !7, i64 0}
!12 = !{!13, !15, i64 4}
!13 = !{!"struct@_ZTSSt12_Vector_baseIN6dealii11Polynomials10PolynomialIdEESaIS3_EE", !14, i64 0}
!14 = !{!"struct@_ZTSNSt12_Vector_baseIN6dealii11Polynomials10PolynomialIdEESaIS3_EE12_Vector_implE", !15, i64 0, !15, i64 4, !15, i64 8}
!15 = !{!"unspecified pointer", !7, i64 0}
!16 = !{!13, !15, i64 0}
!17 = !{!18, !11, i64 0}
!18 = !{!"array@_ZTSA3_j", !11, i64 0}
!19 = !{i32 1}
!20 = !{!21, !23, i64 0}
!21 = !{!"struct@_ZTSN6dealii6TensorILi1ELi3EEE", !22, i64 0}
!22 = !{!"array@_ZTSA3_d", !23, i64 0}
!23 = !{!"double", !7, i64 0}
