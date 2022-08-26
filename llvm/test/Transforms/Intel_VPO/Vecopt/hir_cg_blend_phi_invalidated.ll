; Test to check correctness of generated code when the master user of a blend PHI is invalidated.

; HIR before vectorizer
; <0>     BEGIN REGION { }
; <42>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <41>
; <41>          + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>           |   %0 = (%dct)[i1];
; <5>           |   %1 = (%mf)[i1];
; <7>           |   %2 = (%bias)[i1];
; <11>          |   if (%0 > 0)
; <11>          |   {
; <29>          |      %coef.0 = ((zext.i16.i32(%1) * (zext.i16.i32(%2) + sext.i16.i32(%0))))/u65536;
; <11>          |   }
; <11>          |   else
; <11>          |   {
; <21>          |      %coef.0 = -1 * trunc.i32.i16(((zext.i16.i32(%1) * (zext.i16.i32(%2) + (-1 * sext.i16.i32(%0)))) /u 65536));
; <11>          |   }
; <32>          |   (%dct)[i1] = %coef.0;
; <34>          |   %nz.039 = %nz.039  |  %coef.0; <Safe Reduction>
; <41>          + END LOOP
; <41>
; <43>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; In the above loop node <34> will be invalidated when VPEntities are used to represent the reduction.
; Decomposer will generate a blend PHI for %coef.0 (nodes <29> and <21>), which would be used in the
; invalidated reduction instruction. CG should generate explicit vector code for this PHI to
; prevent compfails.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @quant_4x4(i16* noalias nocapture %dct, i16* nocapture readonly %mf, i16* nocapture readonly %bias) {
; CHECK-LABEL:  Function: quant_4x4
; CHECK-EMPTY:
; CHECK-NEXT:           BEGIN REGION { modified }
; CHECK-NEXT:                %red.init = 0;
; CHECK-NEXT:                %red.init.insert = insertelement %red.init,  %nz.039,  0;
; CHECK-NEXT:                %phi.temp = %red.init.insert;
; CHECK:                     + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:                |   %.vec = (<4 x i16>*)(%dct)[i1];
; CHECK-NEXT:                |   %.vec2 = (<4 x i16>*)(%mf)[i1];
; CHECK-NEXT:                |   %.vec3 = (<4 x i16>*)(%bias)[i1];
; CHECK-NEXT:                |   %.vec4 = %.vec > 0;
; CHECK-NEXT:                |   %.vec5 = %.vec4  ^  -1;
; CHECK-NEXT:                |   %.vec6 = %.vec  *  -1;
; CHECK-NEXT:                |   %.vec7 = %.vec6  +  %.vec3;
; CHECK-NEXT:                |   %.vec8 = %.vec7  *  %.vec2;
; CHECK-NEXT:                |   %.vec9 = %.vec8  /u  65536;
; CHECK-NEXT:                |   %.vec10 = %.vec9  *  -1;
; CHECK-NEXT:                |   %.copy11 = %.vec10;
; CHECK-NEXT:                |   %.vec12 = %.vec  +  %.vec3;
; CHECK-NEXT:                |   %.vec13 = %.vec12  *  %.vec2;
; CHECK-NEXT:                |   %.vec14 = %.vec13  /u  65536;
; CHECK-NEXT:                |   %.copy15 = %.vec14;
; CHECK-NEXT:                |   %select = (%.vec4 == <i1 true, i1 true, i1 true, i1 true>) ? %.copy15 : %.copy11;
; CHECK-NEXT:                |   (<4 x i16>*)(%dct)[i1] = %select;
; CHECK-NEXT:                |   %.vec16 = %phi.temp  |  %select;
; CHECK-NEXT:                |   %phi.temp = %.vec16;
; CHECK-NEXT:                + END LOOP
; CHECK:                     %nz.039 = @llvm.vector.reduce.or.v4i32(%.vec16);
; CHECK:                END REGION
;
entry:
  br label %for.body

for.cond.cleanup:
  %or.lcssa = phi i32 [ %or, %if.end ]
  %tobool = icmp ne i32 %or.lcssa, 0
  %lnot.ext = zext i1 %tobool to i32
  ret i32 %lnot.ext

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.end ]
  %nz.039 = phi i32 [ 0, %entry ], [ %or, %if.end ]
  %idx = getelementptr inbounds i16, i16* %dct, i64 %iv
  %0 = load i16, i16* %idx
  %idx2 = getelementptr inbounds i16, i16* %mf, i64 %iv
  %1 = load i16, i16* %idx2
  %idx4 = getelementptr inbounds i16, i16* %bias, i64 %iv
  %2 = load i16, i16* %idx4
  %conv = sext i16 %0 to i32
  %cmp5 = icmp sgt i16 %0, 0
  %conv7 = zext i16 %2 to i32
  br i1 %cmp5, label %if.then, label %if.else

if.then:
  %add = add nsw i32 %conv7, %conv
  %conv9 = zext i16 %1 to i32
  %mul = mul nsw i32 %add, %conv9
  %3 = lshr i32 %mul, 16
  %conv10 = trunc i32 %3 to i16
  br label %if.end

if.else:
  %sub = sub nsw i32 %conv7, %conv
  %conv13 = zext i16 %1 to i32
  %mul14 = mul nsw i32 %sub, %conv13
  %4 = lshr i32 %mul14, 16
  %5 = trunc i32 %4 to i16
  %conv17 = sub i16 0, %5
  br label %if.end

if.end:
  %coef.0 = phi i16 [ %conv10, %if.then ], [ %conv17, %if.else ]
  store i16 %coef.0, i16* %idx
  %conv20 = sext i16 %coef.0 to i32
  %or = or i32 %nz.039, %conv20
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}
