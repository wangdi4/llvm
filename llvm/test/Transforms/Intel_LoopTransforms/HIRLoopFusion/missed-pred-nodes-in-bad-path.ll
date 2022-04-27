; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-loop-fusion -hir-loop-fusion-skip-vec-prof-check -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -hir-loop-fusion-skip-vec-prof-check -disable-output < %s 2>&1 | FileCheck %s

; Check the loops got fused as checked. Initially there was an issue in Fusion Graph with updating
; BadPathTo/BadPathFrom slices.

;   BEGIN REGION { }
;         + DO i1 = 0, 0, 1   <DO_LOOP>
;         |   if (undef #UNDEF# undef)
;         |   {
;         |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; 0       |      |   (i64*)(%tmp113)[0][undef][i2] = undef;
;         |      + END LOOP
;         |
;         |
;         |      + DO i2 = 0, 0, 1   <DO_LOOP>
; 1       |      |   (i64*)(%tmp)[0][undef][0] = undef;
;         |      + END LOOP
;         |
; 2       |      %tmp230 = 0xFFF0000000000000;
;         |
;         |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; 3       |      |   %tmp230 = (undef #UNDEF# undef) ? (%tmp113)[0][undef][i2] : %tmp230;
;         |      + END LOOP
;         |
; 4       |      (undef)[0] = %tmp230;
; 5       |      %tmp241 = 0x7FF0000000000000;
;         |
;         |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; 6       |      |   %tmp241 = (undef #UNDEF# undef) ? (%tmp113)[0][undef][i2] : %tmp241;
;         |      + END LOOP
;         |
; 7       |      (undef)[0] = %tmp241;
; 8       |      %tmp251 = 0xFFF0000000000000;
;         |
;         |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; 9       |      |   %tmp251 = (undef #UNDEF# undef) ? (%tmp)[0][undef][i2] : %tmp251;
;         |      + END LOOP
;         |
; 10      |      (undef)[0] = %tmp251;
; 11      |      %tmp261 = 0x7FF0000000000000;
;         |
;         |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; 12      |      |   %tmp261 = (undef #UNDEF# undef) ? (%tmp)[0][undef][i2] : %tmp261;
;         |      + END LOOP
;         |   }
;         + END LOOP
;   END REGION

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK-NEXT:       |   if (undef #UNDEF# undef)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %tmp230 = 0xFFF0000000000000;
; CHECK-NEXT:       |      %tmp241 = 0x7FF0000000000000;
; CHECK-NEXT:       |
; CHECK-NEXT:       |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK-NEXT:       |      |   (i64*)(%tmp113)[0][undef][i2] = undef;
; CHECK-DAG:        |      |   %tmp241 = (undef #UNDEF# undef) ? (%tmp113)[0][undef][i2] : %tmp241;
; CHECK-DAG:        |      |   %tmp230 = (undef #UNDEF# undef) ? (%tmp113)[0][undef][i2] : %tmp230;
; CHECK-NEXT:       |      + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |
; CHECK-NEXT:       |      + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK-NEXT:       |      |   (i64*)(%tmp)[0][undef][0] = undef;
; CHECK-NEXT:       |      + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |      (undef)[0] = %tmp230;
; CHECK-NEXT:       |      (undef)[0] = %tmp241;
; CHECK-NEXT:       |      %tmp251 = 0xFFF0000000000000;
; CHECK-NEXT:       |      %tmp261 = 0x7FF0000000000000;
; CHECK-NEXT:       |
; CHECK-NEXT:       |      + DO i2 = 0, undef + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK-DAG:        |      |   %tmp251 = (undef #UNDEF# undef) ? (%tmp)[0][undef][i2] : %tmp251;
; CHECK-DAG:        |      |   %tmp261 = (undef #UNDEF# undef) ? (%tmp)[0][undef][i2] : %tmp261;
; CHECK-NEXT:       |      + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |      (undef)[0] = %tmp251;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

define hidden fastcc void @hoge() unnamed_addr #1 {
bb:
  br i1 undef, label %bb2, label %bb3

bb1:                                              ; preds = %bb2
  br label %bb3

bb2:                                              ; preds = %bb
  br i1 undef, label %bb3, label %bb1

bb3:                                              ; preds = %bb2, %bb1, %bb
  br i1 undef, label %bb4, label %bb5

bb4:                                              ; preds = %bb3
  br label %bb5

bb5:                                              ; preds = %bb4, %bb3
  br i1 undef, label %bb9, label %bb6

bb6:                                              ; preds = %bb5
  br label %bb7

bb7:                                              ; preds = %bb7, %bb6
  br i1 undef, label %bb8, label %bb7

bb8:                                              ; preds = %bb7
  br label %bb9

bb9:                                              ; preds = %bb8, %bb5
  br i1 undef, label %bb289, label %bb296

bb10:                                             ; preds = %bb14, %bb10
  br i1 undef, label %bb11, label %bb10

bb11:                                             ; preds = %bb10
  br label %bb12

bb12:                                             ; preds = %bb13, %bb11
  br i1 undef, label %bb15, label %bb13

bb13:                                             ; preds = %bb18, %bb12
  br i1 undef, label %bb12, label %bb14

bb14:                                             ; preds = %bb13
  br label %bb10

bb15:                                             ; preds = %bb12
  br label %bb16

bb16:                                             ; preds = %bb17, %bb15
  br i1 undef, label %bb19, label %bb17

bb17:                                             ; preds = %bb295, %bb16
  br i1 undef, label %bb16, label %bb18

bb18:                                             ; preds = %bb17
  br label %bb13

bb19:                                             ; preds = %bb16
  br label %bb20

bb20:                                             ; preds = %bb294, %bb19
  br i1 undef, label %bb45, label %bb21

bb21:                                             ; preds = %bb20
  br label %bb42

bb22:                                             ; preds = %bb274
  br i1 undef, label %bb34, label %bb23

bb23:                                             ; preds = %bb22
  br label %bb31

bb24:                                             ; preds = %bb28, %bb24
  br i1 undef, label %bb25, label %bb24

bb25:                                             ; preds = %bb24
  br label %bb26

bb26:                                             ; preds = %bb27, %bb25
  br i1 undef, label %bb29, label %bb27

bb27:                                             ; preds = %bb32, %bb26
  br i1 undef, label %bb26, label %bb28

bb28:                                             ; preds = %bb27
  br label %bb24

bb29:                                             ; preds = %bb26
  br label %bb30

bb30:                                             ; preds = %bb31, %bb29
  br i1 undef, label %bb33, label %bb31

bb31:                                             ; preds = %bb30, %bb23
  br i1 undef, label %bb30, label %bb32

bb32:                                             ; preds = %bb31
  br label %bb27

bb33:                                             ; preds = %bb30
  br label %bb34

bb34:                                             ; preds = %bb274, %bb33, %bb22
  br i1 undef, label %bb47, label %bb59

bb35:                                             ; preds = %bb39, %bb35
  br i1 undef, label %bb36, label %bb35

bb36:                                             ; preds = %bb35
  br label %bb37

bb37:                                             ; preds = %bb38, %bb36
  br i1 undef, label %bb40, label %bb38

bb38:                                             ; preds = %bb43, %bb37
  br i1 undef, label %bb37, label %bb39

bb39:                                             ; preds = %bb38
  br label %bb35

bb40:                                             ; preds = %bb37
  br label %bb41

bb41:                                             ; preds = %bb42, %bb40
  br i1 undef, label %bb44, label %bb42

bb42:                                             ; preds = %bb41, %bb21
  br i1 undef, label %bb41, label %bb43

bb43:                                             ; preds = %bb42
  br label %bb38

bb44:                                             ; preds = %bb41
  br label %bb45

bb45:                                             ; preds = %bb44, %bb20
  br i1 undef, label %bb66, label %bb46

bb46:                                             ; preds = %bb45
  br label %bb63

bb47:                                             ; preds = %bb34
  br i1 undef, label %bb59, label %bb48

bb48:                                             ; preds = %bb47
  br label %bb56

bb49:                                             ; preds = %bb53, %bb49
  br i1 undef, label %bb50, label %bb49

bb50:                                             ; preds = %bb49
  br label %bb51

bb51:                                             ; preds = %bb52, %bb50
  br i1 undef, label %bb54, label %bb52

bb52:                                             ; preds = %bb57, %bb51
  br i1 undef, label %bb51, label %bb53

bb53:                                             ; preds = %bb52
  br label %bb49

bb54:                                             ; preds = %bb51
  br label %bb55

bb55:                                             ; preds = %bb56, %bb54
  br i1 undef, label %bb58, label %bb56

bb56:                                             ; preds = %bb55, %bb48
  br i1 undef, label %bb55, label %bb57

bb57:                                             ; preds = %bb56
  br label %bb52

bb58:                                             ; preds = %bb55
  br label %bb59

bb59:                                             ; preds = %bb58, %bb47, %bb34
  br i1 undef, label %bb76, label %bb68

bb60:                                             ; preds = %bb64, %bb60
  br i1 undef, label %bb61, label %bb60

bb61:                                             ; preds = %bb60
  br label %bb62

bb62:                                             ; preds = %bb63, %bb61
  br i1 undef, label %bb65, label %bb63

bb63:                                             ; preds = %bb62, %bb46
  br i1 undef, label %bb62, label %bb64

bb64:                                             ; preds = %bb63
  br label %bb60

bb65:                                             ; preds = %bb62
  br label %bb66

bb66:                                             ; preds = %bb65, %bb45
  br i1 undef, label %bb87, label %bb67

bb67:                                             ; preds = %bb66
  br label %bb84

bb68:                                             ; preds = %bb59
  br i1 undef, label %bb76, label %bb69

bb69:                                             ; preds = %bb68
  br label %bb73

bb70:                                             ; preds = %bb74, %bb70
  br i1 undef, label %bb71, label %bb70

bb71:                                             ; preds = %bb70
  br label %bb72

bb72:                                             ; preds = %bb73, %bb71
  br i1 undef, label %bb75, label %bb73

bb73:                                             ; preds = %bb72, %bb69
  br i1 undef, label %bb72, label %bb74

bb74:                                             ; preds = %bb73
  br label %bb70

bb75:                                             ; preds = %bb72
  br label %bb76

bb76:                                             ; preds = %bb75, %bb68, %bb59
  br i1 undef, label %bb89, label %bb101

bb77:                                             ; preds = %bb81, %bb77
  br i1 undef, label %bb78, label %bb77

bb78:                                             ; preds = %bb77
  br label %bb79

bb79:                                             ; preds = %bb80, %bb78
  br i1 undef, label %bb82, label %bb80

bb80:                                             ; preds = %bb85, %bb79
  br i1 undef, label %bb79, label %bb81

bb81:                                             ; preds = %bb80
  br label %bb77

bb82:                                             ; preds = %bb79
  br label %bb83

bb83:                                             ; preds = %bb84, %bb82
  br i1 undef, label %bb86, label %bb84

bb84:                                             ; preds = %bb83, %bb67
  br i1 undef, label %bb83, label %bb85

bb85:                                             ; preds = %bb84
  br label %bb80

bb86:                                             ; preds = %bb83
  br label %bb87

bb87:                                             ; preds = %bb86, %bb66
  br i1 undef, label %bb112, label %bb88

bb88:                                             ; preds = %bb87
  br label %bb109

bb89:                                             ; preds = %bb76
  br i1 undef, label %bb101, label %bb90

bb90:                                             ; preds = %bb89
  br label %bb98

bb91:                                             ; preds = %bb95, %bb91
  br i1 undef, label %bb92, label %bb91

bb92:                                             ; preds = %bb91
  br label %bb93

bb93:                                             ; preds = %bb94, %bb92
  br i1 undef, label %bb96, label %bb94

bb94:                                             ; preds = %bb99, %bb93
  br i1 undef, label %bb93, label %bb95

bb95:                                             ; preds = %bb94
  br label %bb91

bb96:                                             ; preds = %bb93
  br label %bb97

bb97:                                             ; preds = %bb98, %bb96
  br i1 undef, label %bb100, label %bb98

bb98:                                             ; preds = %bb97, %bb90
  br i1 undef, label %bb97, label %bb99

bb99:                                             ; preds = %bb98
  br label %bb94

bb100:                                            ; preds = %bb97
  br label %bb101

bb101:                                            ; preds = %bb100, %bb89, %bb76
  br i1 undef, label %bb275, label %bb287

bb102:                                            ; preds = %bb106, %bb102
  br i1 undef, label %bb103, label %bb102

bb103:                                            ; preds = %bb102
  br label %bb104

bb104:                                            ; preds = %bb105, %bb103
  br i1 undef, label %bb107, label %bb105

bb105:                                            ; preds = %bb110, %bb104
  br i1 undef, label %bb104, label %bb106

bb106:                                            ; preds = %bb105
  br label %bb102

bb107:                                            ; preds = %bb104
  br label %bb108

bb108:                                            ; preds = %bb109, %bb107
  br i1 undef, label %bb111, label %bb109

bb109:                                            ; preds = %bb108, %bb88
  br i1 undef, label %bb108, label %bb110

bb110:                                            ; preds = %bb109
  br label %bb105

bb111:                                            ; preds = %bb108
  br label %bb112

bb112:                                            ; preds = %bb111, %bb87
  %tmp = alloca double, i64 undef, align 1
  %tmp113 = alloca double, i64 undef, align 1
  br i1 undef, label %bb114, label %bb115

bb114:                                            ; preds = %bb112
  br label %bb115

bb115:                                            ; preds = %bb114, %bb112
  br i1 undef, label %bb174, label %bb116

bb116:                                            ; preds = %bb115
  br label %bb117

bb117:                                            ; preds = %bb172, %bb116
  br i1 undef, label %bb118, label %bb119

bb118:                                            ; preds = %bb117
  br label %bb124

bb119:                                            ; preds = %bb117
  br label %bb120

bb120:                                            ; preds = %bb120, %bb119
  br i1 undef, label %bb121, label %bb120

bb121:                                            ; preds = %bb120
  br label %bb122

bb122:                                            ; preds = %bb122, %bb121
  br i1 undef, label %bb123, label %bb122

bb123:                                            ; preds = %bb122
  br label %bb124

bb124:                                            ; preds = %bb123, %bb118
  br label %bb128

bb125:                                            ; preds = %bb129, %bb125
  br i1 undef, label %bb126, label %bb125

bb126:                                            ; preds = %bb125
  br label %bb127

bb127:                                            ; preds = %bb128, %bb126
  br i1 undef, label %bb130, label %bb128

bb128:                                            ; preds = %bb127, %bb124
  br i1 undef, label %bb127, label %bb129

bb129:                                            ; preds = %bb128
  br label %bb125

bb130:                                            ; preds = %bb127
  br label %bb134

bb131:                                            ; preds = %bb135, %bb131
  br i1 undef, label %bb132, label %bb131

bb132:                                            ; preds = %bb131
  br label %bb133

bb133:                                            ; preds = %bb134, %bb132
  br i1 undef, label %bb136, label %bb134

bb134:                                            ; preds = %bb133, %bb130
  br i1 undef, label %bb133, label %bb135

bb135:                                            ; preds = %bb134
  br label %bb131

bb136:                                            ; preds = %bb133
  br label %bb140

bb137:                                            ; preds = %bb141, %bb137
  br i1 undef, label %bb138, label %bb137

bb138:                                            ; preds = %bb137
  br label %bb139

bb139:                                            ; preds = %bb140, %bb138
  br i1 undef, label %bb142, label %bb140

bb140:                                            ; preds = %bb139, %bb136
  br i1 undef, label %bb139, label %bb141

bb141:                                            ; preds = %bb140
  br label %bb137

bb142:                                            ; preds = %bb139
  br label %bb146

bb143:                                            ; preds = %bb147, %bb143
  br i1 undef, label %bb144, label %bb143

bb144:                                            ; preds = %bb143
  br label %bb145

bb145:                                            ; preds = %bb146, %bb144
  br i1 undef, label %bb148, label %bb146

bb146:                                            ; preds = %bb145, %bb142
  br i1 undef, label %bb145, label %bb147

bb147:                                            ; preds = %bb146
  br label %bb143

bb148:                                            ; preds = %bb145
  br label %bb152

bb149:                                            ; preds = %bb153, %bb149
  br i1 undef, label %bb150, label %bb149

bb150:                                            ; preds = %bb149
  br label %bb151

bb151:                                            ; preds = %bb152, %bb150
  br i1 undef, label %bb154, label %bb152

bb152:                                            ; preds = %bb151, %bb148
  br i1 undef, label %bb151, label %bb153

bb153:                                            ; preds = %bb152
  br label %bb149

bb154:                                            ; preds = %bb151
  br label %bb158

bb155:                                            ; preds = %bb159, %bb155
  br i1 undef, label %bb156, label %bb155

bb156:                                            ; preds = %bb155
  br label %bb157

bb157:                                            ; preds = %bb158, %bb156
  br i1 undef, label %bb160, label %bb158

bb158:                                            ; preds = %bb157, %bb154
  br i1 undef, label %bb157, label %bb159

bb159:                                            ; preds = %bb158
  br label %bb155

bb160:                                            ; preds = %bb157
  br label %bb164

bb161:                                            ; preds = %bb165, %bb161
  br i1 undef, label %bb162, label %bb161

bb162:                                            ; preds = %bb161
  br label %bb163

bb163:                                            ; preds = %bb164, %bb162
  br i1 undef, label %bb166, label %bb164

bb164:                                            ; preds = %bb163, %bb160
  br i1 undef, label %bb163, label %bb165

bb165:                                            ; preds = %bb164
  br label %bb161

bb166:                                            ; preds = %bb163
  br label %bb170

bb167:                                            ; preds = %bb171, %bb167
  br i1 undef, label %bb168, label %bb167

bb168:                                            ; preds = %bb167
  br label %bb169

bb169:                                            ; preds = %bb170, %bb168
  br i1 undef, label %bb172, label %bb170

bb170:                                            ; preds = %bb169, %bb166
  br i1 undef, label %bb169, label %bb171

bb171:                                            ; preds = %bb170
  br label %bb167

bb172:                                            ; preds = %bb169
  br i1 undef, label %bb117, label %bb173

bb173:                                            ; preds = %bb172
  br label %bb174

bb174:                                            ; preds = %bb173, %bb115
  br i1 undef, label %bb182, label %bb175

bb175:                                            ; preds = %bb174
  br label %bb179

bb176:                                            ; preds = %bb180, %bb176
  br i1 undef, label %bb177, label %bb176

bb177:                                            ; preds = %bb176
  br label %bb178

bb178:                                            ; preds = %bb179, %bb177
  br i1 undef, label %bb181, label %bb179

bb179:                                            ; preds = %bb178, %bb175
  br i1 undef, label %bb178, label %bb180

bb180:                                            ; preds = %bb179
  br label %bb176

bb181:                                            ; preds = %bb178
  br label %bb182

bb182:                                            ; preds = %bb181, %bb174
  br i1 undef, label %bb201, label %bb183

bb183:                                            ; preds = %bb182
  br label %bb184

bb184:                                            ; preds = %bb193, %bb183
  br i1 undef, label %bb193, label %bb185

bb185:                                            ; preds = %bb184
  br label %bb186

bb186:                                            ; preds = %bb191, %bb185
  br i1 undef, label %bb187, label %bb188

bb187:                                            ; preds = %bb186
  br label %bb191

bb188:                                            ; preds = %bb186
  br label %bb189

bb189:                                            ; preds = %bb189, %bb188
  br i1 undef, label %bb190, label %bb189

bb190:                                            ; preds = %bb189
  br label %bb191

bb191:                                            ; preds = %bb190, %bb187
  br i1 undef, label %bb192, label %bb186

bb192:                                            ; preds = %bb191
  br label %bb193

bb193:                                            ; preds = %bb192, %bb184
  br i1 undef, label %bb194, label %bb184

bb194:                                            ; preds = %bb193
  br label %bb195

bb195:                                            ; preds = %bb199, %bb194
  br i1 undef, label %bb199, label %bb196

bb196:                                            ; preds = %bb195
  br label %bb197

bb197:                                            ; preds = %bb197, %bb196
  br i1 undef, label %bb198, label %bb197

bb198:                                            ; preds = %bb197
  br label %bb199

bb199:                                            ; preds = %bb198, %bb195
  br i1 undef, label %bb200, label %bb195

bb200:                                            ; preds = %bb199
  br label %bb201

bb201:                                            ; preds = %bb200, %bb182
  br i1 undef, label %bb202, label %bb203

bb202:                                            ; preds = %bb201
  br label %bb203

bb203:                                            ; preds = %bb202, %bb201
  br i1 undef, label %bb204, label %bb205

bb204:                                            ; preds = %bb203
  br label %bb205

bb205:                                            ; preds = %bb204, %bb203
  br i1 undef, label %bb206, label %bb207

bb206:                                            ; preds = %bb205
  br label %bb207

bb207:                                            ; preds = %bb206, %bb205
  br i1 undef, label %bb274, label %bb208

bb208:                                            ; preds = %bb207
  br i1 undef, label %bb272, label %bb209

bb209:                                            ; preds = %bb208
  %tmp210 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 undef, double* elementtype(double) nonnull %tmp113, i64 1) #2
  %tmp211 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 undef, double* elementtype(double) nonnull %tmp, i64 1) #2
  br label %bb212

bb212:                                            ; preds = %bb269, %bb209
  br i1 undef, label %bb237, label %bb213

bb213:                                            ; preds = %bb212
  %tmp214 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 undef, i64 32, double* elementtype(double) nonnull %tmp210, i64 undef) #2
  br label %bb215

bb215:                                            ; preds = %bb215, %bb213
  %tmp216 = phi i64 [ 1, %bb213 ], [ %tmp219, %bb215 ]
  %tmp217 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp214, i64 %tmp216) #2
  %tmp218 = bitcast double* %tmp217 to i64*
  store i64 undef, i64* %tmp218, align 1
  %tmp219 = add nuw nsw i64 %tmp216, 1
  %tmp220 = icmp eq i64 %tmp219, undef
  br i1 %tmp220, label %bb221, label %bb215

bb221:                                            ; preds = %bb215
  %tmp222 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 undef, i64 32, double* elementtype(double) nonnull %tmp211, i64 undef) #2
  br label %bb223

bb223:                                            ; preds = %bb223, %bb221
  %tmp224 = phi i64 [ 1, %bb221 ], [ undef, %bb223 ]
  %tmp225 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp222, i64 %tmp224) #2
  %tmp226 = bitcast double* %tmp225 to i64*
  store i64 undef, i64* %tmp226, align 1
  %tmp227 = icmp eq i64 undef, undef
  br i1 %tmp227, label %bb228, label %bb223

bb228:                                            ; preds = %bb223
  br label %bb229

bb229:                                            ; preds = %bb229, %bb228
  %tmp230 = phi double [ %tmp234, %bb229 ], [ 0xFFF0000000000000, %bb228 ]
  %tmp231 = phi i64 [ %tmp235, %bb229 ], [ 1, %bb228 ]
  %tmp232 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp214, i64 %tmp231) #2
  %tmp233 = load double, double* %tmp232, align 1
  %tmp234 = select i1 undef, double %tmp233, double %tmp230
  %tmp235 = add nuw nsw i64 %tmp231, 1
  %tmp236 = icmp eq i64 %tmp235, undef
  br i1 %tmp236, label %bb238, label %bb229

bb237:                                            ; preds = %bb212
  br label %bb269

bb238:                                            ; preds = %bb229
  %tmp239 = phi double [ %tmp234, %bb229 ]
  store double %tmp239, double* undef, align 1
  br label %bb240

bb240:                                            ; preds = %bb240, %bb238
  %tmp241 = phi double [ 0x7FF0000000000000, %bb238 ], [ %tmp245, %bb240 ]
  %tmp242 = phi i64 [ 1, %bb238 ], [ %tmp246, %bb240 ]
  %tmp243 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp214, i64 %tmp242) #2
  %tmp244 = load double, double* %tmp243, align 1
  %tmp245 = select i1 undef, double %tmp244, double %tmp241
  %tmp246 = add nuw nsw i64 %tmp242, 1
  %tmp247 = icmp eq i64 %tmp246, undef
  br i1 %tmp247, label %bb248, label %bb240

bb248:                                            ; preds = %bb240
  %tmp249 = phi double [ %tmp245, %bb240 ]
  store double %tmp249, double* undef, align 1
  br label %bb250

bb250:                                            ; preds = %bb250, %bb248
  %tmp251 = phi double [ 0xFFF0000000000000, %bb248 ], [ %tmp255, %bb250 ]
  %tmp252 = phi i64 [ 1, %bb248 ], [ %tmp256, %bb250 ]
  %tmp253 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp222, i64 %tmp252) #2
  %tmp254 = load double, double* %tmp253, align 1
  %tmp255 = select i1 undef, double %tmp254, double %tmp251
  %tmp256 = add nuw nsw i64 %tmp252, 1
  %tmp257 = icmp eq i64 %tmp256, undef
  br i1 %tmp257, label %bb258, label %bb250

bb258:                                            ; preds = %bb250
  %tmp259 = phi double [ %tmp255, %bb250 ]
  store double %tmp259, double* undef, align 1
  br label %bb260

bb260:                                            ; preds = %bb260, %bb258
  %tmp261 = phi double [ 0x7FF0000000000000, %bb258 ], [ %tmp265, %bb260 ]
  %tmp262 = phi i64 [ 1, %bb258 ], [ %tmp266, %bb260 ]
  %tmp263 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %tmp222, i64 %tmp262) #2
  %tmp264 = load double, double* %tmp263, align 1
  %tmp265 = select i1 undef, double %tmp264, double %tmp261
  %tmp266 = add nuw nsw i64 %tmp262, 1
  %tmp267 = icmp eq i64 %tmp266, undef
  br i1 %tmp267, label %bb268, label %bb260

bb268:                                            ; preds = %bb260
  br label %bb269

bb269:                                            ; preds = %bb268, %bb237
  %tmp270 = icmp ne i32 undef, undef
  br i1 %tmp270, label %bb212, label %bb271

bb271:                                            ; preds = %bb269
  br label %bb272

bb272:                                            ; preds = %bb271, %bb208
  br i1 undef, label %bb273, label %bb274

bb273:                                            ; preds = %bb272
  br label %bb274

bb274:                                            ; preds = %bb273, %bb272, %bb207
  br i1 undef, label %bb22, label %bb34

bb275:                                            ; preds = %bb101
  br i1 undef, label %bb287, label %bb276

bb276:                                            ; preds = %bb275
  br label %bb284

bb277:                                            ; preds = %bb281, %bb277
  br i1 undef, label %bb278, label %bb277

bb278:                                            ; preds = %bb277
  br label %bb279

bb279:                                            ; preds = %bb280, %bb278
  br i1 undef, label %bb282, label %bb280

bb280:                                            ; preds = %bb285, %bb279
  br i1 undef, label %bb279, label %bb281

bb281:                                            ; preds = %bb280
  br label %bb277

bb282:                                            ; preds = %bb279
  br label %bb283

bb283:                                            ; preds = %bb284, %bb282
  br i1 undef, label %bb286, label %bb284

bb284:                                            ; preds = %bb283, %bb276
  br i1 undef, label %bb283, label %bb285

bb285:                                            ; preds = %bb284
  br label %bb280

bb286:                                            ; preds = %bb283
  br label %bb287

bb287:                                            ; preds = %bb286, %bb275, %bb101
  br i1 undef, label %bb288, label %bb296

bb288:                                            ; preds = %bb287
  br label %bb296

bb289:                                            ; preds = %bb9
  br i1 undef, label %bb291, label %bb292

bb290:                                            ; preds = %bb291
  br label %bb292

bb291:                                            ; preds = %bb289
  br i1 undef, label %bb292, label %bb290

bb292:                                            ; preds = %bb291, %bb290, %bb289
  br i1 undef, label %bb293, label %bb294

bb293:                                            ; preds = %bb292
  br label %bb294

bb294:                                            ; preds = %bb293, %bb292
  br i1 undef, label %bb20, label %bb295

bb295:                                            ; preds = %bb294
  br label %bb17

bb296:                                            ; preds = %bb288, %bb287, %bb9
  br i1 undef, label %bb297, label %bb298

bb297:                                            ; preds = %bb296
  br label %bb298

bb298:                                            ; preds = %bb297, %bb296
  br i1 undef, label %bb303, label %bb299

bb299:                                            ; preds = %bb298
  br i1 undef, label %bb300, label %bb301

bb300:                                            ; preds = %bb299
  br label %bb301

bb301:                                            ; preds = %bb300, %bb299
  br i1 undef, label %bb302, label %bb303

bb302:                                            ; preds = %bb301
  br label %bb303

bb303:                                            ; preds = %bb302, %bb301, %bb298
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

