; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we can successfully multiversion the loop.
; We were compfailing on getting dereferenced type for (<2 x float>*)(%t0)[i1].

; HIR-
; + DO i1 = 0, (8 * zext.i32.i64(%t1) + -8)/u8, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   %_M_value.real.i.i.i34 = (%t0)[i1].0.0;
; |   %_M_value.imag.i.i.i36 = (%t0)[i1].0.1;
; |   %mul_ad.i.i39 = %_M_value.imag.i.i.i36  *  %retval.sroa.0.0.vec.extract.i37;
; |   %mul_bc.i.i40 = %_M_value.real.i.i.i34  *  %retval.sroa.0.4.vec.extract.i38;
; |   %mul_ac.i.i42 = %_M_value.real.i.i.i34  *  %retval.sroa.0.0.vec.extract.i37;
; |   %mul_bd.i.i43.neg = %_M_value.imag.i.i.i36  *  %retval.sroa.0.4.vec.extract.i38;
; |   %_M_value.real.i.i.i26 = (%t2)[i1].0.0;
; |   %_M_value.imag.i.i.i28 = (%t2)[i1].0.1;
; |   %mul_ad.i.i = %_M_value.imag.i.i.i28  *  %retval.sroa.0.0.vec.extract.i29;
; |   %mul_bc.i.i = %_M_value.real.i.i.i26  *  %retval.sroa.0.4.vec.extract.i30;
; |   %mul_ac.i.i = %_M_value.real.i.i.i26  *  %retval.sroa.0.0.vec.extract.i29;
; |   %mul_bd.i.i.neg = %_M_value.imag.i.i.i28  *  %retval.sroa.0.4.vec.extract.i30;
; |   %reass.add = %mul_bd.i.i.neg  +  %mul_bd.i.i43.neg;
; |   %mul_r.i.i44 = %mul_ac.i.i  +  %mul_ac.i.i42;
; |   %add.r.i.i = %mul_r.i.i44  -  %reass.add;
; |   %mul_i.i.i = %mul_ad.i.i39  +  %mul_bc.i.i40;
; |   %mul_i.i.i41 = %mul_i.i.i  +  %mul_bc.i.i;
; |   %add.i.i.i = %mul_i.i.i41  +  %mul_ad.i.i;
; |   %retval.sroa.0.0.vec.insert.i = insertelement poison,  %add.r.i.i,  0;
; |   %retval.sroa.0.4.vec.insert.i = insertelement %retval.sroa.0.0.vec.insert.i,  %add.i.i.i,  1;
; |   (<2 x float>*)(%t0)[i1] = %retval.sroa.0.4.vec.insert.i;
; + END LOOP


; CHECK: %mv.test{{.*}} = &((%t0)[zext.i32.i64(%t1) + -1].0.1) >=u &((%t2)[0].0.0);
; CHECK: %mv.test{{.*}} = &((%t2)[zext.i32.i64(%t1) + -1].0.1) >=u &((float*)(%t0)[0]);

%"struct.std::complex.33" = type { { float, float } }

define void @foo(ptr %t0, ptr %t2, <2 x float> %x.coerce, <2 x float> %a.coerce, i32 %t1) {
entry:
  %idxprom.i = zext i32 %t1 to i64
  %arrayidx.i = getelementptr inbounds %"struct.std::complex.33", ptr %t0, i64 %idxprom.i
  %retval.sroa.0.0.vec.extract.i37 = extractelement <2 x float> %x.coerce, i32 0
  %retval.sroa.0.4.vec.extract.i38 = extractelement <2 x float> %x.coerce, i32 1
  %retval.sroa.0.0.vec.extract.i29 = extractelement <2 x float> %a.coerce, i32 0
  %retval.sroa.0.4.vec.extract.i30 = extractelement <2 x float> %a.coerce, i32 1
  %cmp.not51 = icmp eq i32 %t1, 0
  br i1 %cmp.not51, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %i_ptr.053 = phi ptr [ %incdec.ptr13, %for.body ], [ %t0, %for.body.preheader ]
  %v_ptr.052 = phi ptr [ %incdec.ptr, %for.body ], [ %t2, %for.body.preheader ]
  %_M_value.real.i.i.i34 = load float, ptr %i_ptr.053, align 4
  %_M_value.imagp.i.i.i35 = getelementptr inbounds %"struct.std::complex.33", ptr %i_ptr.053, i64 0, i32 0, i32 1
  %_M_value.imag.i.i.i36 = load float, ptr %_M_value.imagp.i.i.i35, align 4
  %mul_ad.i.i39 = fmul fast float %_M_value.imag.i.i.i36, %retval.sroa.0.0.vec.extract.i37
  %mul_bc.i.i40 = fmul fast float %_M_value.real.i.i.i34, %retval.sroa.0.4.vec.extract.i38
  %mul_ac.i.i42 = fmul fast float %_M_value.real.i.i.i34, %retval.sroa.0.0.vec.extract.i37
  %mul_bd.i.i43.neg = fmul fast float %_M_value.imag.i.i.i36, %retval.sroa.0.4.vec.extract.i38
  %incdec.ptr = getelementptr inbounds %"struct.std::complex.33", ptr %v_ptr.052, i64 1
  %_M_value.real.i.i.i26 = load float, ptr %v_ptr.052, align 4
  %_M_value.imagp.i.i.i27 = getelementptr inbounds %"struct.std::complex.33", ptr %v_ptr.052, i64 0, i32 0, i32 1
  %_M_value.imag.i.i.i28 = load float, ptr %_M_value.imagp.i.i.i27, align 4
  %mul_ad.i.i = fmul fast float %_M_value.imag.i.i.i28, %retval.sroa.0.0.vec.extract.i29
  %mul_bc.i.i = fmul fast float %_M_value.real.i.i.i26, %retval.sroa.0.4.vec.extract.i30
  %mul_ac.i.i = fmul fast float %_M_value.real.i.i.i26, %retval.sroa.0.0.vec.extract.i29
  %mul_bd.i.i.neg = fmul fast float %_M_value.imag.i.i.i28, %retval.sroa.0.4.vec.extract.i30
  %reass.add = fadd fast float %mul_bd.i.i.neg, %mul_bd.i.i43.neg
  %mul_r.i.i44 = fadd fast float %mul_ac.i.i, %mul_ac.i.i42
  %add.r.i.i = fsub fast float %mul_r.i.i44, %reass.add
  %mul_i.i.i = fadd fast float %mul_ad.i.i39, %mul_bc.i.i40
  %mul_i.i.i41 = fadd fast float %mul_i.i.i, %mul_bc.i.i
  %add.i.i.i = fadd fast float %mul_i.i.i41, %mul_ad.i.i
  %retval.sroa.0.0.vec.insert.i = insertelement <2 x float> poison, float %add.r.i.i, i32 0
  %retval.sroa.0.4.vec.insert.i = insertelement <2 x float> %retval.sroa.0.0.vec.insert.i, float %add.i.i.i, i32 1
  store <2 x float> %retval.sroa.0.4.vec.insert.i, ptr %i_ptr.053, align 4
  %incdec.ptr13 = getelementptr inbounds %"struct.std::complex.33", ptr %i_ptr.053, i64 1
  %cmp.not = icmp eq ptr %incdec.ptr13, %arrayidx.i
  br i1 %cmp.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

