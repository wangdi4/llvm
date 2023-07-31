; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output  < %s 2>&1 | grep "DO i2" | count 2
; NOTE: The test requires fusion of distributed loops. This should not happen usually and should be forced later.
; The test verifies that after the fusion there will be 2 "DO i2" loops.

; ModuleID = 'cmplrs-48988.ll'
source_filename = "mgrid.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @baz(ptr noalias readonly %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture %arg2, ptr noalias nocapture readonly %arg3) local_unnamed_addr #0 {
bb:
  %tmp = load i64, ptr %arg1, align 8
  %tmp4 = add nsw i64 %tmp, 1
  %tmp5 = mul nsw i64 %tmp4, %tmp
  %tmp6 = xor i64 %tmp5, -1
  %tmp7 = getelementptr inbounds double, ptr %arg, i64 %tmp6
  %tmp8 = load i64, ptr %arg3, align 8
  %tmp9 = add nsw i64 %tmp8, 1
  %tmp10 = mul nsw i64 %tmp9, %tmp8
  %tmp11 = xor i64 %tmp10, -1
  %tmp12 = getelementptr inbounds double, ptr %arg2, i64 %tmp11
  %tmp13 = icmp sgt i64 %tmp, 2
  br i1 %tmp13, label %bb127, label %bb288

bb127:                                            ; preds = %bb124
  br label %bb128

bb128:                                            ; preds = %bb284, %bb127
  %tmp129 = phi i64 [ 2, %bb127 ], [ %tmp285, %bb284 ]
  %tmp130 = add nsw i64 %tmp129, -1
  %tmp131 = mul nsw i64 %tmp130, %tmp
  %tmp132 = mul nsw i64 %tmp129, %tmp
  %tmp133 = shl nuw i64 %tmp129, 1
  %tmp134 = add nsw i64 %tmp133, -2
  %tmp135 = mul nsw i64 %tmp134, %tmp8
  %tmp136 = add i64 %tmp135, -1
  br label %bb137

bb137:                                            ; preds = %bb192, %bb128
  %tmp138 = phi i64 [ 2, %bb128 ], [ %tmp193, %bb192 ]
  %tmp139 = add nsw i64 %tmp138, %tmp131
  %tmp140 = mul nsw i64 %tmp139, %tmp
  %tmp141 = add nsw i64 %tmp138, %tmp132
  %tmp142 = mul nsw i64 %tmp141, %tmp
  %tmp143 = shl nuw i64 %tmp138, 1
  %tmp144 = add i64 %tmp136, %tmp143
  %tmp145 = mul nsw i64 %tmp144, %tmp8
  %tmp146 = add i64 %tmp145, -1
  br label %bb147

bb147:                                            ; preds = %bb147, %bb137
  %tmp148 = phi i64 [ 2, %bb137 ], [ %tmp162, %bb147 ]
  %tmp149 = add nsw i64 %tmp148, %tmp140
  %tmp150 = getelementptr inbounds double, ptr %tmp7, i64 %tmp149
  %tmp151 = load double, ptr %tmp150, align 8
  %tmp152 = add nsw i64 %tmp148, %tmp142
  %tmp153 = getelementptr inbounds double, ptr %tmp7, i64 %tmp152
  %tmp154 = load double, ptr %tmp153, align 8
  %tmp155 = fadd double %tmp151, %tmp154
  %tmp156 = fmul double %tmp155, 5.000000e-01
  %tmp157 = shl nuw i64 %tmp148, 1
  %tmp158 = add i64 %tmp146, %tmp157
  %tmp159 = getelementptr inbounds double, ptr %tmp12, i64 %tmp158
  %tmp160 = load double, ptr %tmp159, align 8
  %tmp161 = fadd double %tmp160, %tmp156
  store double %tmp161, ptr %tmp159, align 8
  %tmp162 = add nuw nsw i64 %tmp148, 1
  %tmp163 = icmp eq i64 %tmp162, %tmp
  br i1 %tmp163, label %bb164, label %bb147

bb164:                                            ; preds = %bb147
  %tmp165 = add i64 %tmp145, -2
  br label %bb166

bb166:                                            ; preds = %bb166, %bb164
  %tmp167 = phi i64 [ 2, %bb164 ], [ %tmp190, %bb166 ]
  %tmp168 = add nsw i64 %tmp167, -1
  %tmp169 = add nsw i64 %tmp168, %tmp140
  %tmp170 = getelementptr inbounds double, ptr %tmp7, i64 %tmp169
  %tmp171 = load double, ptr %tmp170, align 8
  %tmp172 = add nsw i64 %tmp167, %tmp140
  %tmp173 = getelementptr inbounds double, ptr %tmp7, i64 %tmp172
  %tmp174 = load double, ptr %tmp173, align 8
  %tmp175 = fadd double %tmp171, %tmp174
  %tmp176 = add nsw i64 %tmp168, %tmp142
  %tmp177 = getelementptr inbounds double, ptr %tmp7, i64 %tmp176
  %tmp178 = load double, ptr %tmp177, align 8
  %tmp179 = fadd double %tmp175, %tmp178
  %tmp180 = add nsw i64 %tmp167, %tmp142
  %tmp181 = getelementptr inbounds double, ptr %tmp7, i64 %tmp180
  %tmp182 = load double, ptr %tmp181, align 8
  %tmp183 = fadd double %tmp179, %tmp182
  %tmp184 = fmul double %tmp183, 2.500000e-01
  %tmp185 = shl nuw i64 %tmp167, 1
  %tmp186 = add i64 %tmp165, %tmp185
  %tmp187 = getelementptr inbounds double, ptr %tmp12, i64 %tmp186
  %tmp188 = load double, ptr %tmp187, align 8
  %tmp189 = fadd double %tmp188, %tmp184
  store double %tmp189, ptr %tmp187, align 8
  %tmp190 = add nuw nsw i64 %tmp167, 1
  %tmp191 = icmp eq i64 %tmp190, %tmp
  br i1 %tmp191, label %bb192, label %bb166

bb192:                                            ; preds = %bb166
  %tmp193 = add nuw nsw i64 %tmp138, 1
  %tmp194 = icmp eq i64 %tmp193, %tmp
  br i1 %tmp194, label %bb195, label %bb137

bb195:                                            ; preds = %bb192
  %tmp196 = add i64 %tmp135, -2
  br label %bb197

bb197:                                            ; preds = %bb281, %bb195
  %tmp198 = phi i64 [ 2, %bb195 ], [ %tmp282, %bb281 ]
  %tmp199 = add nsw i64 %tmp198, -1
  %tmp200 = add nsw i64 %tmp199, %tmp131
  %tmp201 = mul nsw i64 %tmp200, %tmp
  %tmp202 = add nsw i64 %tmp198, %tmp131
  %tmp203 = mul nsw i64 %tmp202, %tmp
  %tmp204 = add nsw i64 %tmp199, %tmp132
  %tmp205 = mul nsw i64 %tmp204, %tmp
  %tmp206 = add nsw i64 %tmp198, %tmp132
  %tmp207 = mul nsw i64 %tmp206, %tmp
  %tmp208 = shl nuw i64 %tmp198, 1
  %tmp209 = add i64 %tmp196, %tmp208
  %tmp210 = mul nsw i64 %tmp209, %tmp8
  %tmp211 = add i64 %tmp210, -1
  br label %bb212

bb212:                                            ; preds = %bb212, %bb197
  %tmp213 = phi i64 [ 2, %bb197 ], [ %tmp235, %bb212 ]
  %tmp214 = add nsw i64 %tmp213, %tmp201
  %tmp215 = getelementptr inbounds double, ptr %tmp7, i64 %tmp214
  %tmp216 = load double, ptr %tmp215, align 8
  %tmp217 = add nsw i64 %tmp213, %tmp203
  %tmp218 = getelementptr inbounds double, ptr %tmp7, i64 %tmp217
  %tmp219 = load double, ptr %tmp218, align 8
  %tmp220 = fadd double %tmp216, %tmp219
  %tmp221 = add nsw i64 %tmp213, %tmp205
  %tmp222 = getelementptr inbounds double, ptr %tmp7, i64 %tmp221
  %tmp223 = load double, ptr %tmp222, align 8
  %tmp224 = fadd double %tmp220, %tmp223
  %tmp225 = add nsw i64 %tmp213, %tmp207
  %tmp226 = getelementptr inbounds double, ptr %tmp7, i64 %tmp225
  %tmp227 = load double, ptr %tmp226, align 8
  %tmp228 = fadd double %tmp224, %tmp227
  %tmp229 = fmul double %tmp228, 2.500000e-01
  %tmp230 = shl nuw i64 %tmp213, 1
  %tmp231 = add i64 %tmp211, %tmp230
  %tmp232 = getelementptr inbounds double, ptr %tmp12, i64 %tmp231
  %tmp233 = load double, ptr %tmp232, align 8
  %tmp234 = fadd double %tmp233, %tmp229
  store double %tmp234, ptr %tmp232, align 8
  %tmp235 = add nuw nsw i64 %tmp213, 1
  %tmp236 = icmp eq i64 %tmp235, %tmp
  br i1 %tmp236, label %bb237, label %bb212

bb237:                                            ; preds = %bb212
  %tmp238 = add i64 %tmp210, -2
  br label %bb239

bb239:                                            ; preds = %bb239, %bb237
  %tmp240 = phi i64 [ 2, %bb237 ], [ %tmp279, %bb239 ]
  %tmp241 = add nsw i64 %tmp240, -1
  %tmp242 = add nsw i64 %tmp241, %tmp201
  %tmp243 = getelementptr inbounds double, ptr %tmp7, i64 %tmp242
  %tmp244 = load double, ptr %tmp243, align 8
  %tmp245 = add nsw i64 %tmp241, %tmp203
  %tmp246 = getelementptr inbounds double, ptr %tmp7, i64 %tmp245
  %tmp247 = load double, ptr %tmp246, align 8
  %tmp248 = fadd double %tmp244, %tmp247
  %tmp249 = add nsw i64 %tmp240, %tmp201
  %tmp250 = getelementptr inbounds double, ptr %tmp7, i64 %tmp249
  %tmp251 = load double, ptr %tmp250, align 8
  %tmp252 = fadd double %tmp248, %tmp251
  %tmp253 = add nsw i64 %tmp240, %tmp203
  %tmp254 = getelementptr inbounds double, ptr %tmp7, i64 %tmp253
  %tmp255 = load double, ptr %tmp254, align 8
  %tmp256 = fadd double %tmp252, %tmp255
  %tmp257 = add nsw i64 %tmp241, %tmp205
  %tmp258 = getelementptr inbounds double, ptr %tmp7, i64 %tmp257
  %tmp259 = load double, ptr %tmp258, align 8
  %tmp260 = fadd double %tmp256, %tmp259
  %tmp261 = add nsw i64 %tmp241, %tmp207
  %tmp262 = getelementptr inbounds double, ptr %tmp7, i64 %tmp261
  %tmp263 = load double, ptr %tmp262, align 8
  %tmp264 = fadd double %tmp260, %tmp263
  %tmp265 = add nsw i64 %tmp240, %tmp205
  %tmp266 = getelementptr inbounds double, ptr %tmp7, i64 %tmp265
  %tmp267 = load double, ptr %tmp266, align 8
  %tmp268 = fadd double %tmp264, %tmp267
  %tmp269 = add nsw i64 %tmp240, %tmp207
  %tmp270 = getelementptr inbounds double, ptr %tmp7, i64 %tmp269
  %tmp271 = load double, ptr %tmp270, align 8
  %tmp272 = fadd double %tmp268, %tmp271
  %tmp273 = fmul double %tmp272, 1.250000e-01
  %tmp274 = shl nuw i64 %tmp240, 1
  %tmp275 = add i64 %tmp238, %tmp274
  %tmp276 = getelementptr inbounds double, ptr %tmp12, i64 %tmp275
  %tmp277 = load double, ptr %tmp276, align 8
  %tmp278 = fadd double %tmp277, %tmp273
  store double %tmp278, ptr %tmp276, align 8
  %tmp279 = add nuw nsw i64 %tmp240, 1
  %tmp280 = icmp eq i64 %tmp279, %tmp
  br i1 %tmp280, label %bb281, label %bb239

bb281:                                            ; preds = %bb239
  %tmp282 = add nuw nsw i64 %tmp198, 1
  %tmp283 = icmp eq i64 %tmp282, %tmp
  br i1 %tmp283, label %bb284, label %bb197

bb284:                                            ; preds = %bb281
  %tmp285 = add nuw nsw i64 %tmp129, 1
  %tmp286 = icmp eq i64 %tmp285, %tmp
  br i1 %tmp286, label %bb287, label %bb128

bb287:                                            ; preds = %bb284
  br label %bb288

bb288:                                            ; preds = %bb287, %bb
  %tmp289 = tail call i32 @eggs(ptr %arg2, ptr %arg3)
  ret i32 0
}

; Function Attrs: norecurse nounwind uwtable
declare i32 @eggs(ptr noalias nocapture, ptr noalias nocapture readonly) local_unnamed_addr #0

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
