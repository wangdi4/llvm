; Test to check correctness of mixed-CG approach when the master user of a blend PHI is invalidated.

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
; <34>          |   %nz.039 = %nz.039  ||  %coef.0; <Safe Reduction>
; <41>          + END LOOP
; <41>
; <43>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; In the above loop node <34> will be invalidated when VPEntities are used to represent the reduction.
; Decomposer will generate a blend PHI for %coef.0 (nodes <29> and <21>), which would be used in the
; invalidated reduction instruction. Mixed CG should generate explicit vector code for this PHI to
; prevent compfails. NOTE: Selects are optimized away for the blend PHIs since the same widened ref
; is being blended from all incoming edges.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-use-entity-instr -enable-vp-value-codegen-hir=false -vplan-force-vf=4 -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; CHECK:            %red.var = 0;
; CHECK-NEXT:       %red.var = insertelement %red.var,  %nz.039,  0;

; CHECK:            + DO i1 = 0, 1023, 4   <DO_LOOP> <novectorize>
; CHECK-NEXT:       |   %.vec = (<4 x i16>*)(%dct)[i1];
; CHECK-NEXT:       |   %.vec2 = (<4 x i16>*)(%mf)[i1];
; CHECK-NEXT:       |   %.vec3 = (<4 x i16>*)(%bias)[i1];
; CHECK-NEXT:       |   %wide.cmp. = %.vec > 0;
; CHECK-NEXT:       |   %.vec4 = %wide.cmp.  ^  -1;
; CHECK-NEXT:       |   %NBConv = zext.<4 x i16>.<4 x i32>(%.vec2);
; CHECK-NEXT:       |   %NBConv5 = zext.<4 x i16>.<4 x i32>(%.vec3);
; CHECK-NEXT:       |   %NBConv6 = sext.<4 x i16>.<4 x i32>(%.vec);
; CHECK-NEXT:       |   %NAry = <i32 -1, i32 -1, i32 -1, i32 -1>  *  %NBConv6;
; CHECK-NEXT:       |   %NAry7 = %NBConv5  +  %NAry;
; CHECK-NEXT:       |   %NAry8 = %NBConv  *  %NAry7;
; CHECK-NEXT:       |   %UDiv = %NAry8  /u  <i32 65536, i32 65536, i32 65536, i32 65536>;
; CHECK-NEXT:       |   %NBConv9 = trunc.<4 x i32>.<4 x i16>(%UDiv);
; CHECK-NEXT:       |   %coef.0.in1.vec = -1 * %NBConv9; Mask = @{%.vec4}
; CHECK-NEXT:       |   %NBConv10 = zext.<4 x i16>.<4 x i32>(%.vec2);
; CHECK-NEXT:       |   %NBConv11 = zext.<4 x i16>.<4 x i32>(%.vec3);
; CHECK-NEXT:       |   %NBConv12 = sext.<4 x i16>.<4 x i32>(%.vec);
; CHECK-NEXT:       |   %NAry13 = %NBConv11  +  %NBConv12;
; CHECK-NEXT:       |   %NAry14 = %NBConv10  *  %NAry13;
; CHECK-NEXT:       |   %coef.0.in1.vec = (%NAry14)/u65536; Mask = @{%wide.cmp.}
; CHECK-NEXT:       |   (<4 x i16>*)(%dct)[i1] = %coef.0.in1.vec;
; CHECK-NEXT:       |   %.vec15 = sext.<4 x i16>.<4 x i32>(%coef.0.in1.vec);
; CHECK-NEXT:       |   %red.var = %red.var  ||  %.vec15;
; CHECK-NEXT:       + END LOOP

; CHECK:            %nz.039 = @llvm.experimental.vector.reduce.or.v4i32(%red.var);

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @quant_4x4(i16* noalias nocapture %dct, i16* nocapture readonly %mf, i16* nocapture readonly %bias) {
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
