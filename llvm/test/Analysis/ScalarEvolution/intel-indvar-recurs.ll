; This large file tests the recursion limit put into the phi processing
; in ScalarEvolution. We have a series of small loops that feed into each
; other, roughly corresponding to this code:
;  ...
;  for (; i < end; i++)  { }
;  for (; i < end; i++)  { }
;  for (; i < end; i++)  { }
;  ...
; Each "i" is defined by a phi from the previous loop. ScalarEvolution
; will trace back each phi as far as it can, which may be up to 40 levels
; of recursion.

; The new PM will process the loops outside-in, which basically avoids the
; problem by using iteration instead of recursion. Original test may not be
; a problem with the new PM.

; RUN: opt --enable-new-pm=0 -indvars -S < %s | FileCheck %s
; CHECK: lcssa = phi

; Without the limit, indvars can fully remove all the phis. This is good, but
; very expensive.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cond = dso_local global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3fooiPi(i32 %end, i32* %a) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, %end
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc4, %for.end
  %i.1 = phi i32 [ %i.0, %for.end ], [ %inc5, %for.inc4 ]
  %cmp2 = icmp slt i32 %i.1, %end
  br i1 %cmp2, label %for.body3, label %for.end6

for.body3:                                        ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.body3
  %inc5 = add nsw i32 %i.1, 1
  br label %for.cond1

for.end6:                                         ; preds = %for.cond1
  br label %for.cond7

for.cond7:                                        ; preds = %for.inc10, %for.end6
  %i.2 = phi i32 [ %i.1, %for.end6 ], [ %inc11, %for.inc10 ]
  %cmp8 = icmp slt i32 %i.2, %end
  br i1 %cmp8, label %for.body9, label %for.end12

for.body9:                                        ; preds = %for.cond7
  br label %for.inc10

for.inc10:                                        ; preds = %for.body9
  %inc11 = add nsw i32 %i.2, 1
  br label %for.cond7

for.end12:                                        ; preds = %for.cond7
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc16, %for.end12
  %i.3 = phi i32 [ %i.2, %for.end12 ], [ %inc17, %for.inc16 ]
  %cmp14 = icmp slt i32 %i.3, %end
  br i1 %cmp14, label %for.body15, label %for.end18

for.body15:                                       ; preds = %for.cond13
  br label %for.inc16

for.inc16:                                        ; preds = %for.body15
  %inc17 = add nsw i32 %i.3, 1
  br label %for.cond13

for.end18:                                        ; preds = %for.cond13
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc22, %for.end18
  %i.4 = phi i32 [ %i.3, %for.end18 ], [ %inc23, %for.inc22 ]
  %cmp20 = icmp slt i32 %i.4, %end
  br i1 %cmp20, label %for.body21, label %for.end24

for.body21:                                       ; preds = %for.cond19
  br label %for.inc22

for.inc22:                                        ; preds = %for.body21
  %inc23 = add nsw i32 %i.4, 1
  br label %for.cond19

for.end24:                                        ; preds = %for.cond19
  br label %for.cond25

for.cond25:                                       ; preds = %for.inc28, %for.end24
  %i.5 = phi i32 [ %i.4, %for.end24 ], [ %inc29, %for.inc28 ]
  %cmp26 = icmp slt i32 %i.5, %end
  br i1 %cmp26, label %for.body27, label %for.end30

for.body27:                                       ; preds = %for.cond25
  br label %for.inc28

for.inc28:                                        ; preds = %for.body27
  %inc29 = add nsw i32 %i.5, 1
  br label %for.cond25

for.end30:                                        ; preds = %for.cond25
  br label %for.cond31

for.cond31:                                       ; preds = %for.inc34, %for.end30
  %i.6 = phi i32 [ %i.5, %for.end30 ], [ %inc35, %for.inc34 ]
  %cmp32 = icmp slt i32 %i.6, %end
  br i1 %cmp32, label %for.body33, label %for.end36

for.body33:                                       ; preds = %for.cond31
  br label %for.inc34

for.inc34:                                        ; preds = %for.body33
  %inc35 = add nsw i32 %i.6, 1
  br label %for.cond31

for.end36:                                        ; preds = %for.cond31
  br label %for.cond37

for.cond37:                                       ; preds = %for.inc40, %for.end36
  %i.7 = phi i32 [ %i.6, %for.end36 ], [ %inc41, %for.inc40 ]
  %cmp38 = icmp slt i32 %i.7, %end
  br i1 %cmp38, label %for.body39, label %for.end42

for.body39:                                       ; preds = %for.cond37
  br label %for.inc40

for.inc40:                                        ; preds = %for.body39
  %inc41 = add nsw i32 %i.7, 1
  br label %for.cond37

for.end42:                                        ; preds = %for.cond37
  br label %for.cond43

for.cond43:                                       ; preds = %for.inc46, %for.end42
  %i.8 = phi i32 [ %i.7, %for.end42 ], [ %inc47, %for.inc46 ]
  %cmp44 = icmp slt i32 %i.8, %end
  br i1 %cmp44, label %for.body45, label %for.end48

for.body45:                                       ; preds = %for.cond43
  br label %for.inc46

for.inc46:                                        ; preds = %for.body45
  %inc47 = add nsw i32 %i.8, 1
  br label %for.cond43

for.end48:                                        ; preds = %for.cond43
  br label %for.cond49

for.cond49:                                       ; preds = %for.inc52, %for.end48
  %i.9 = phi i32 [ %i.8, %for.end48 ], [ %inc53, %for.inc52 ]
  %cmp50 = icmp slt i32 %i.9, %end
  br i1 %cmp50, label %for.body51, label %for.end54

for.body51:                                       ; preds = %for.cond49
  br label %for.inc52

for.inc52:                                        ; preds = %for.body51
  %inc53 = add nsw i32 %i.9, 1
  br label %for.cond49

for.end54:                                        ; preds = %for.cond49
  br label %for.cond55

for.cond55:                                       ; preds = %for.inc58, %for.end54
  %i.10 = phi i32 [ %i.9, %for.end54 ], [ %inc59, %for.inc58 ]
  %cmp56 = icmp slt i32 %i.10, %end
  br i1 %cmp56, label %for.body57, label %for.end60

for.body57:                                       ; preds = %for.cond55
  br label %for.inc58

for.inc58:                                        ; preds = %for.body57
  %inc59 = add nsw i32 %i.10, 1
  br label %for.cond55

for.end60:                                        ; preds = %for.cond55
  br label %for.cond61

for.cond61:                                       ; preds = %for.inc64, %for.end60
  %i.11 = phi i32 [ %i.10, %for.end60 ], [ %inc65, %for.inc64 ]
  %cmp62 = icmp slt i32 %i.11, %end
  br i1 %cmp62, label %for.body63, label %for.end66

for.body63:                                       ; preds = %for.cond61
  br label %for.inc64

for.inc64:                                        ; preds = %for.body63
  %inc65 = add nsw i32 %i.11, 1
  br label %for.cond61

for.end66:                                        ; preds = %for.cond61
  br label %for.cond67

for.cond67:                                       ; preds = %for.inc70, %for.end66
  %i.12 = phi i32 [ %i.11, %for.end66 ], [ %inc71, %for.inc70 ]
  %cmp68 = icmp slt i32 %i.12, %end
  br i1 %cmp68, label %for.body69, label %for.end72

for.body69:                                       ; preds = %for.cond67
  br label %for.inc70

for.inc70:                                        ; preds = %for.body69
  %inc71 = add nsw i32 %i.12, 1
  br label %for.cond67

for.end72:                                        ; preds = %for.cond67
  br label %for.cond73

for.cond73:                                       ; preds = %for.inc76, %for.end72
  %i.13 = phi i32 [ %i.12, %for.end72 ], [ %inc77, %for.inc76 ]
  %cmp74 = icmp slt i32 %i.13, %end
  br i1 %cmp74, label %for.body75, label %for.end78

for.body75:                                       ; preds = %for.cond73
  br label %for.inc76

for.inc76:                                        ; preds = %for.body75
  %inc77 = add nsw i32 %i.13, 1
  br label %for.cond73

for.end78:                                        ; preds = %for.cond73
  br label %for.cond79

for.cond79:                                       ; preds = %for.inc82, %for.end78
  %i.14 = phi i32 [ %i.13, %for.end78 ], [ %inc83, %for.inc82 ]
  %cmp80 = icmp slt i32 %i.14, %end
  br i1 %cmp80, label %for.body81, label %for.end84

for.body81:                                       ; preds = %for.cond79
  br label %for.inc82

for.inc82:                                        ; preds = %for.body81
  %inc83 = add nsw i32 %i.14, 1
  br label %for.cond79

for.end84:                                        ; preds = %for.cond79
  br label %for.cond85

for.cond85:                                       ; preds = %for.inc88, %for.end84
  %i.15 = phi i32 [ %i.14, %for.end84 ], [ %inc89, %for.inc88 ]
  %cmp86 = icmp slt i32 %i.15, %end
  br i1 %cmp86, label %for.body87, label %for.end90

for.body87:                                       ; preds = %for.cond85
  br label %for.inc88

for.inc88:                                        ; preds = %for.body87
  %inc89 = add nsw i32 %i.15, 1
  br label %for.cond85

for.end90:                                        ; preds = %for.cond85
  br label %for.cond91

for.cond91:                                       ; preds = %for.inc94, %for.end90
  %i.16 = phi i32 [ %i.15, %for.end90 ], [ %inc95, %for.inc94 ]
  %cmp92 = icmp slt i32 %i.16, %end
  br i1 %cmp92, label %for.body93, label %for.end96

for.body93:                                       ; preds = %for.cond91
  br label %for.inc94

for.inc94:                                        ; preds = %for.body93
  %inc95 = add nsw i32 %i.16, 1
  br label %for.cond91

for.end96:                                        ; preds = %for.cond91
  br label %for.cond97

for.cond97:                                       ; preds = %for.inc100, %for.end96
  %i.17 = phi i32 [ %i.16, %for.end96 ], [ %inc101, %for.inc100 ]
  %cmp98 = icmp slt i32 %i.17, %end
  br i1 %cmp98, label %for.body99, label %for.end102

for.body99:                                       ; preds = %for.cond97
  br label %for.inc100

for.inc100:                                       ; preds = %for.body99
  %inc101 = add nsw i32 %i.17, 1
  br label %for.cond97

for.end102:                                       ; preds = %for.cond97
  br label %for.cond103

for.cond103:                                      ; preds = %for.inc106, %for.end102
  %i.18 = phi i32 [ %i.17, %for.end102 ], [ %inc107, %for.inc106 ]
  %cmp104 = icmp slt i32 %i.18, %end
  br i1 %cmp104, label %for.body105, label %for.end108

for.body105:                                      ; preds = %for.cond103
  br label %for.inc106

for.inc106:                                       ; preds = %for.body105
  %inc107 = add nsw i32 %i.18, 1
  br label %for.cond103

for.end108:                                       ; preds = %for.cond103
  br label %for.cond109

for.cond109:                                      ; preds = %for.inc112, %for.end108
  %i.19 = phi i32 [ %i.18, %for.end108 ], [ %inc113, %for.inc112 ]
  %cmp110 = icmp slt i32 %i.19, %end
  br i1 %cmp110, label %for.body111, label %for.end114

for.body111:                                      ; preds = %for.cond109
  br label %for.inc112

for.inc112:                                       ; preds = %for.body111
  %inc113 = add nsw i32 %i.19, 1
  br label %for.cond109

for.end114:                                       ; preds = %for.cond109
  br label %for.cond115

for.cond115:                                      ; preds = %for.inc118, %for.end114
  %i.20 = phi i32 [ %i.19, %for.end114 ], [ %inc119, %for.inc118 ]
  %cmp116 = icmp slt i32 %i.20, %end
  br i1 %cmp116, label %for.body117, label %for.end120

for.body117:                                      ; preds = %for.cond115
  br label %for.inc118

for.inc118:                                       ; preds = %for.body117
  %inc119 = add nsw i32 %i.20, 1
  br label %for.cond115

for.end120:                                       ; preds = %for.cond115
  br label %for.cond121

for.cond121:                                      ; preds = %for.inc124, %for.end120
  %i.21 = phi i32 [ %i.20, %for.end120 ], [ %inc125, %for.inc124 ]
  %cmp122 = icmp slt i32 %i.21, %end
  br i1 %cmp122, label %for.body123, label %for.end126

for.body123:                                      ; preds = %for.cond121
  br label %for.inc124

for.inc124:                                       ; preds = %for.body123
  %inc125 = add nsw i32 %i.21, 1
  br label %for.cond121

for.end126:                                       ; preds = %for.cond121
  br label %for.cond127

for.cond127:                                      ; preds = %for.inc130, %for.end126
  %i.22 = phi i32 [ %i.21, %for.end126 ], [ %inc131, %for.inc130 ]
  %cmp128 = icmp slt i32 %i.22, %end
  br i1 %cmp128, label %for.body129, label %for.end132

for.body129:                                      ; preds = %for.cond127
  br label %for.inc130

for.inc130:                                       ; preds = %for.body129
  %inc131 = add nsw i32 %i.22, 1
  br label %for.cond127

for.end132:                                       ; preds = %for.cond127
  br label %for.cond133

for.cond133:                                      ; preds = %for.inc136, %for.end132
  %i.23 = phi i32 [ %i.22, %for.end132 ], [ %inc137, %for.inc136 ]
  %cmp134 = icmp slt i32 %i.23, %end
  br i1 %cmp134, label %for.body135, label %for.end138

for.body135:                                      ; preds = %for.cond133
  br label %for.inc136

for.inc136:                                       ; preds = %for.body135
  %inc137 = add nsw i32 %i.23, 1
  br label %for.cond133

for.end138:                                       ; preds = %for.cond133
  br label %for.cond139

for.cond139:                                      ; preds = %for.inc142, %for.end138
  %i.24 = phi i32 [ %i.23, %for.end138 ], [ %inc143, %for.inc142 ]
  %cmp140 = icmp slt i32 %i.24, %end
  br i1 %cmp140, label %for.body141, label %for.end144

for.body141:                                      ; preds = %for.cond139
  br label %for.inc142

for.inc142:                                       ; preds = %for.body141
  %inc143 = add nsw i32 %i.24, 1
  br label %for.cond139

for.end144:                                       ; preds = %for.cond139
  br label %for.cond145

for.cond145:                                      ; preds = %for.inc148, %for.end144
  %i.25 = phi i32 [ %i.24, %for.end144 ], [ %inc149, %for.inc148 ]
  %cmp146 = icmp slt i32 %i.25, %end
  br i1 %cmp146, label %for.body147, label %for.end150

for.body147:                                      ; preds = %for.cond145
  br label %for.inc148

for.inc148:                                       ; preds = %for.body147
  %inc149 = add nsw i32 %i.25, 1
  br label %for.cond145

for.end150:                                       ; preds = %for.cond145
  br label %for.cond151

for.cond151:                                      ; preds = %for.inc154, %for.end150
  %i.26 = phi i32 [ %i.25, %for.end150 ], [ %inc155, %for.inc154 ]
  %cmp152 = icmp slt i32 %i.26, %end
  br i1 %cmp152, label %for.body153, label %for.end156

for.body153:                                      ; preds = %for.cond151
  br label %for.inc154

for.inc154:                                       ; preds = %for.body153
  %inc155 = add nsw i32 %i.26, 1
  br label %for.cond151

for.end156:                                       ; preds = %for.cond151
  br label %for.cond157

for.cond157:                                      ; preds = %for.inc160, %for.end156
  %i.27 = phi i32 [ %i.26, %for.end156 ], [ %inc161, %for.inc160 ]
  %cmp158 = icmp slt i32 %i.27, %end
  br i1 %cmp158, label %for.body159, label %for.end162

for.body159:                                      ; preds = %for.cond157
  br label %for.inc160

for.inc160:                                       ; preds = %for.body159
  %inc161 = add nsw i32 %i.27, 1
  br label %for.cond157

for.end162:                                       ; preds = %for.cond157
  br label %for.cond163

for.cond163:                                      ; preds = %for.inc166, %for.end162
  %i.28 = phi i32 [ %i.27, %for.end162 ], [ %inc167, %for.inc166 ]
  %cmp164 = icmp slt i32 %i.28, %end
  br i1 %cmp164, label %for.body165, label %for.end168

for.body165:                                      ; preds = %for.cond163
  br label %for.inc166

for.inc166:                                       ; preds = %for.body165
  %inc167 = add nsw i32 %i.28, 1
  br label %for.cond163

for.end168:                                       ; preds = %for.cond163
  br label %for.cond169

for.cond169:                                      ; preds = %for.inc172, %for.end168
  %i.29 = phi i32 [ %i.28, %for.end168 ], [ %inc173, %for.inc172 ]
  %cmp170 = icmp slt i32 %i.29, %end
  br i1 %cmp170, label %for.body171, label %for.end174

for.body171:                                      ; preds = %for.cond169
  br label %for.inc172

for.inc172:                                       ; preds = %for.body171
  %inc173 = add nsw i32 %i.29, 1
  br label %for.cond169

for.end174:                                       ; preds = %for.cond169
  br label %for.cond175

for.cond175:                                      ; preds = %for.inc178, %for.end174
  %i.30 = phi i32 [ %i.29, %for.end174 ], [ %inc179, %for.inc178 ]
  %cmp176 = icmp slt i32 %i.30, %end
  br i1 %cmp176, label %for.body177, label %for.end180

for.body177:                                      ; preds = %for.cond175
  br label %for.inc178

for.inc178:                                       ; preds = %for.body177
  %inc179 = add nsw i32 %i.30, 1
  br label %for.cond175

for.end180:                                       ; preds = %for.cond175
  br label %for.cond181

for.cond181:                                      ; preds = %for.inc184, %for.end180
  %i.31 = phi i32 [ %i.30, %for.end180 ], [ %inc185, %for.inc184 ]
  %cmp182 = icmp slt i32 %i.31, %end
  br i1 %cmp182, label %for.body183, label %for.end186

for.body183:                                      ; preds = %for.cond181
  br label %for.inc184

for.inc184:                                       ; preds = %for.body183
  %inc185 = add nsw i32 %i.31, 1
  br label %for.cond181

for.end186:                                       ; preds = %for.cond181
  br label %for.cond187

for.cond187:                                      ; preds = %for.inc190, %for.end186
  %i.32 = phi i32 [ %i.31, %for.end186 ], [ %inc191, %for.inc190 ]
  %cmp188 = icmp slt i32 %i.32, %end
  br i1 %cmp188, label %for.body189, label %for.end192

for.body189:                                      ; preds = %for.cond187
  br label %for.inc190

for.inc190:                                       ; preds = %for.body189
  %inc191 = add nsw i32 %i.32, 1
  br label %for.cond187

for.end192:                                       ; preds = %for.cond187
  br label %for.cond193

for.cond193:                                      ; preds = %for.inc196, %for.end192
  %i.33 = phi i32 [ %i.32, %for.end192 ], [ %inc197, %for.inc196 ]
  %cmp194 = icmp slt i32 %i.33, %end
  br i1 %cmp194, label %for.body195, label %for.end198

for.body195:                                      ; preds = %for.cond193
  br label %for.inc196

for.inc196:                                       ; preds = %for.body195
  %inc197 = add nsw i32 %i.33, 1
  br label %for.cond193

for.end198:                                       ; preds = %for.cond193
  br label %for.cond199

for.cond199:                                      ; preds = %for.inc202, %for.end198
  %i.34 = phi i32 [ %i.33, %for.end198 ], [ %inc203, %for.inc202 ]
  %cmp200 = icmp slt i32 %i.34, %end
  br i1 %cmp200, label %for.body201, label %for.end204

for.body201:                                      ; preds = %for.cond199
  br label %for.inc202

for.inc202:                                       ; preds = %for.body201
  %inc203 = add nsw i32 %i.34, 1
  br label %for.cond199

for.end204:                                       ; preds = %for.cond199
  br label %for.cond205

for.cond205:                                      ; preds = %for.inc208, %for.end204
  %i.35 = phi i32 [ %i.34, %for.end204 ], [ %inc209, %for.inc208 ]
  %cmp206 = icmp slt i32 %i.35, %end
  br i1 %cmp206, label %for.body207, label %for.end210

for.body207:                                      ; preds = %for.cond205
  br label %for.inc208

for.inc208:                                       ; preds = %for.body207
  %inc209 = add nsw i32 %i.35, 1
  br label %for.cond205

for.end210:                                       ; preds = %for.cond205
  br label %for.cond211

for.cond211:                                      ; preds = %for.inc214, %for.end210
  %i.36 = phi i32 [ %i.35, %for.end210 ], [ %inc215, %for.inc214 ]
  %cmp212 = icmp slt i32 %i.36, %end
  br i1 %cmp212, label %for.body213, label %for.end216

for.body213:                                      ; preds = %for.cond211
  br label %for.inc214

for.inc214:                                       ; preds = %for.body213
  %inc215 = add nsw i32 %i.36, 1
  br label %for.cond211

for.end216:                                       ; preds = %for.cond211
  br label %for.cond217

for.cond217:                                      ; preds = %for.inc220, %for.end216
  %i.37 = phi i32 [ %i.36, %for.end216 ], [ %inc221, %for.inc220 ]
  %cmp218 = icmp slt i32 %i.37, %end
  br i1 %cmp218, label %for.body219, label %for.end222

for.body219:                                      ; preds = %for.cond217
  br label %for.inc220

for.inc220:                                       ; preds = %for.body219
  %inc221 = add nsw i32 %i.37, 1
  br label %for.cond217

for.end222:                                       ; preds = %for.cond217
  br label %for.cond223

for.cond223:                                      ; preds = %for.inc226, %for.end222
  %i.38 = phi i32 [ %i.37, %for.end222 ], [ %inc227, %for.inc226 ]
  %cmp224 = icmp slt i32 %i.38, %end
  br i1 %cmp224, label %for.body225, label %for.end228

for.body225:                                      ; preds = %for.cond223
  br label %for.inc226

for.inc226:                                       ; preds = %for.body225
  %inc227 = add nsw i32 %i.38, 1
  br label %for.cond223

for.end228:                                       ; preds = %for.cond223
  br label %for.cond229

for.cond229:                                      ; preds = %for.inc232, %for.end228
  %i.39 = phi i32 [ %i.38, %for.end228 ], [ %inc233, %for.inc232 ]
  %cmp230 = icmp slt i32 %i.39, %end
  br i1 %cmp230, label %for.body231, label %for.end234

for.body231:                                      ; preds = %for.cond229
  br label %for.inc232

for.inc232:                                       ; preds = %for.body231
  %inc233 = add nsw i32 %i.39, 1
  br label %for.cond229

for.end234:                                       ; preds = %for.cond229
  %idxprom = sext i32 %i.39 to i64
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, %end
  ret i32 %add
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
