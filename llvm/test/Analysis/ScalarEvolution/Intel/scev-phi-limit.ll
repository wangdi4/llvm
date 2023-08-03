; CMPLRLLVM-27987:
; Verify that we can set a limit on the recursion depth of
; "IdenticalOperandsPHI" analysis in SCEV.

; Deep recursion through phis can relate %val1 == %val2 == %tmp178.
; %tmp110 = phi i64 [ %val1, %bb108 ], [ %val2, %bb102 ]
; ... 32 levels of phis ...
; %tmp180 = icmp eq i64 %tmp178, %val2

; RUN: opt -scalar-evolution-max-scev-compare-depth=8 -passes=indvars -S %s | FileCheck --check-prefix=NEG %s
; RUN: opt -scalar-evolution-max-scev-compare-depth=64 -passes=indvars -S %s | FileCheck --check-prefix=POS %s

; NEG: br i1 %tmp180, label %bb181, label %bb177
; POS: br i1 true, label %bb181, label %bb177

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@res = external dso_local unnamed_addr global i64

declare dso_local i32 @pluto(...)

declare hidden void @spam() local_unnamed_addr #0

define hidden void @barney() local_unnamed_addr #0 personality ptr @pluto {
bb:
  %init = load i64, ptr @res, align 8
  %val2 = add i64 %init, 1
  invoke void @spam()
          to label %bb1 unwind label %bb37

bb1:                                              ; preds = %bb
  invoke void @spam()
          to label %bb2 unwind label %bb38

bb2:                                              ; preds = %bb1
  invoke void @spam()
          to label %bb3 unwind label %bb40

bb3:                                              ; preds = %bb2
  invoke void @spam()
          to label %bb4 unwind label %bb42

bb4:                                              ; preds = %bb3
  invoke void @spam()
          to label %bb5 unwind label %bb44

bb5:                                              ; preds = %bb4
  invoke void @spam()
          to label %bb6 unwind label %bb46

bb6:                                              ; preds = %bb5
  invoke void @spam()
          to label %bb7 unwind label %bb48

bb7:                                              ; preds = %bb6
  invoke void @spam()
          to label %bb8 unwind label %bb50

bb8:                                              ; preds = %bb7
  invoke void @spam()
          to label %bb9 unwind label %bb52

bb9:                                              ; preds = %bb8
  invoke void @spam()
          to label %bb10 unwind label %bb54

bb10:                                             ; preds = %bb9
  invoke void @spam()
          to label %bb11 unwind label %bb56

bb11:                                             ; preds = %bb10
  invoke void @spam()
          to label %bb12 unwind label %bb58

bb12:                                             ; preds = %bb11
  invoke void @spam()
          to label %bb13 unwind label %bb60

bb13:                                             ; preds = %bb12
  invoke void @spam()
          to label %bb14 unwind label %bb62

bb14:                                             ; preds = %bb13
  invoke void @spam()
          to label %bb15 unwind label %bb64

bb15:                                             ; preds = %bb14
  invoke void @spam()
          to label %bb16 unwind label %bb66

bb16:                                             ; preds = %bb15
  invoke void @spam()
          to label %bb17 unwind label %bb68

bb17:                                             ; preds = %bb16
  invoke void @spam()
          to label %bb18 unwind label %bb70

bb18:                                             ; preds = %bb17
  invoke void @spam()
          to label %bb19 unwind label %bb72

bb19:                                             ; preds = %bb18
  invoke void @spam()
          to label %bb20 unwind label %bb74

bb20:                                             ; preds = %bb19
  invoke void @spam()
          to label %bb21 unwind label %bb76

bb21:                                             ; preds = %bb20
  invoke void @spam()
          to label %bb22 unwind label %bb78

bb22:                                             ; preds = %bb21
  invoke void @spam()
          to label %bb23 unwind label %bb80

bb23:                                             ; preds = %bb22
  invoke void @spam()
          to label %bb24 unwind label %bb82

bb24:                                             ; preds = %bb23
  invoke void @spam()
          to label %bb25 unwind label %bb84

bb25:                                             ; preds = %bb24
  invoke void @spam()
          to label %bb26 unwind label %bb86

bb26:                                             ; preds = %bb25
  invoke void @spam()
          to label %bb27 unwind label %bb88

bb27:                                             ; preds = %bb26
  invoke void @spam()
          to label %bb28 unwind label %bb90

bb28:                                             ; preds = %bb27
  invoke void @spam()
          to label %bb29 unwind label %bb92

bb29:                                             ; preds = %bb28
  invoke void @spam()
          to label %bb30 unwind label %bb94

bb30:                                             ; preds = %bb29
  invoke void @spam()
          to label %bb31 unwind label %bb96

bb31:                                             ; preds = %bb30
  invoke void @spam()
          to label %bb32 unwind label %bb98

bb32:                                             ; preds = %bb31
  invoke void @spam()
          to label %bb33 unwind label %bb100

bb33:                                             ; preds = %bb32
  invoke void @spam()
          to label %bb34 unwind label %bb102

bb34:                                             ; preds = %bb33
  invoke void @spam()
          to label %bb35 unwind label %bb104

bb35:                                             ; preds = %bb34
  invoke void @spam()
          to label %bb36 unwind label %bb106

bb36:                                             ; preds = %bb35
  unreachable

bb37:                                             ; preds = %bb
  %tmp = landingpad { ptr, i32 }
          cleanup
  br label %bb175

bb38:                                             ; preds = %bb1
  %tmp39 = landingpad { ptr, i32 }
          cleanup
  br label %bb173

bb40:                                             ; preds = %bb2
  %tmp41 = landingpad { ptr, i32 }
          cleanup
  br label %bb171

bb42:                                             ; preds = %bb3
  %tmp43 = landingpad { ptr, i32 }
          cleanup
  br label %bb169

bb44:                                             ; preds = %bb4
  %tmp45 = landingpad { ptr, i32 }
          cleanup
  br label %bb167

bb46:                                             ; preds = %bb5
  %tmp47 = landingpad { ptr, i32 }
          cleanup
  br label %bb165

bb48:                                             ; preds = %bb6
  %tmp49 = landingpad { ptr, i32 }
          cleanup
  br label %bb163

bb50:                                             ; preds = %bb7
  %tmp51 = landingpad { ptr, i32 }
          cleanup
  br label %bb161

bb52:                                             ; preds = %bb8
  %tmp53 = landingpad { ptr, i32 }
          cleanup
  br label %bb159

bb54:                                             ; preds = %bb9
  %tmp55 = landingpad { ptr, i32 }
          cleanup
  br label %bb157

bb56:                                             ; preds = %bb10
  %tmp57 = landingpad { ptr, i32 }
          cleanup
  br label %bb155

bb58:                                             ; preds = %bb11
  %tmp59 = landingpad { ptr, i32 }
          cleanup
  br label %bb153

bb60:                                             ; preds = %bb12
  %tmp61 = landingpad { ptr, i32 }
          cleanup
  br label %bb151

bb62:                                             ; preds = %bb13
  %tmp63 = landingpad { ptr, i32 }
          cleanup
  br label %bb149

bb64:                                             ; preds = %bb14
  %tmp65 = landingpad { ptr, i32 }
          cleanup
  br label %bb147

bb66:                                             ; preds = %bb15
  %tmp67 = landingpad { ptr, i32 }
          cleanup
  br label %bb145

bb68:                                             ; preds = %bb16
  %tmp69 = landingpad { ptr, i32 }
          cleanup
  br label %bb143

bb70:                                             ; preds = %bb17
  %tmp71 = landingpad { ptr, i32 }
          cleanup
  br label %bb141

bb72:                                             ; preds = %bb18
  %tmp73 = landingpad { ptr, i32 }
          cleanup
  br label %bb139

bb74:                                             ; preds = %bb19
  %tmp75 = landingpad { ptr, i32 }
          cleanup
  br label %bb137

bb76:                                             ; preds = %bb20
  %tmp77 = landingpad { ptr, i32 }
          cleanup
  br label %bb135

bb78:                                             ; preds = %bb21
  %tmp79 = landingpad { ptr, i32 }
          cleanup
  br label %bb133

bb80:                                             ; preds = %bb22
  %tmp81 = landingpad { ptr, i32 }
          cleanup
  br label %bb131

bb82:                                             ; preds = %bb23
  %tmp83 = landingpad { ptr, i32 }
          cleanup
  br label %bb129

bb84:                                             ; preds = %bb24
  %tmp85 = landingpad { ptr, i32 }
          cleanup
  br label %bb127

bb86:                                             ; preds = %bb25
  %tmp87 = landingpad { ptr, i32 }
          cleanup
  br label %bb125

bb88:                                             ; preds = %bb26
  %tmp89 = landingpad { ptr, i32 }
          cleanup
  br label %bb123

bb90:                                             ; preds = %bb27
  %tmp91 = landingpad { ptr, i32 }
          cleanup
  br label %bb121

bb92:                                             ; preds = %bb28
  %tmp93 = landingpad { ptr, i32 }
          cleanup
  br label %bb119

bb94:                                             ; preds = %bb29
  %tmp95 = landingpad { ptr, i32 }
          cleanup
  br label %bb117

bb96:                                             ; preds = %bb30
  %tmp97 = landingpad { ptr, i32 }
          cleanup
  br label %bb115

bb98:                                             ; preds = %bb31
  %tmp99 = landingpad { ptr, i32 }
          cleanup
  br label %bb113

bb100:                                            ; preds = %bb32
  %tmp101 = landingpad { ptr, i32 }
          cleanup
  br label %bb111

bb102:                                            ; preds = %bb33
  %tmp103 = landingpad { ptr, i32 }
          cleanup
  br label %bb109

bb104:                                            ; preds = %bb34
  %tmp105 = landingpad { ptr, i32 }
          cleanup
  br label %bb108

bb106:                                            ; preds = %bb35
  %tmp107 = landingpad { ptr, i32 }
          cleanup
  br label %bb108

bb108:                                            ; preds = %bb106, %bb104
  %val1 = add i64 %init, 1
  br label %bb109

bb109:                                            ; preds = %bb108, %bb102
  %tmp110 = phi i64 [ %val1, %bb108 ], [ %val2, %bb102 ]
  br label %bb111

bb111:                                            ; preds = %bb109, %bb100
  %tmp112 = phi i64 [ %tmp110, %bb109 ], [ %val2, %bb100 ]
  br label %bb113

bb113:                                            ; preds = %bb111, %bb98
  %tmp114 = phi i64 [ %tmp112, %bb111 ], [ %val2, %bb98 ]
  br label %bb115

bb115:                                            ; preds = %bb113, %bb96
  %tmp116 = phi i64 [ %tmp114, %bb113 ], [ %val2, %bb96 ]
  br label %bb117

bb117:                                            ; preds = %bb115, %bb94
  %tmp118 = phi i64 [ %tmp116, %bb115 ], [ %val2, %bb94 ]
  br label %bb119

bb119:                                            ; preds = %bb117, %bb92
  %tmp120 = phi i64 [ %tmp118, %bb117 ], [ %val2, %bb92 ]
  br label %bb121

bb121:                                            ; preds = %bb119, %bb90
  %tmp122 = phi i64 [ %tmp120, %bb119 ], [ %val2, %bb90 ]
  br label %bb123

bb123:                                            ; preds = %bb121, %bb88
  %tmp124 = phi i64 [ %tmp122, %bb121 ], [ %val2, %bb88 ]
  br label %bb125

bb125:                                            ; preds = %bb123, %bb86
  %tmp126 = phi i64 [ %tmp124, %bb123 ], [ %val2, %bb86 ]
  br label %bb127

bb127:                                            ; preds = %bb125, %bb84
  %tmp128 = phi i64 [ %tmp126, %bb125 ], [ %val2, %bb84 ]
  br label %bb129

bb129:                                            ; preds = %bb127, %bb82
  %tmp130 = phi i64 [ %tmp128, %bb127 ], [ %val2, %bb82 ]
  br label %bb131

bb131:                                            ; preds = %bb129, %bb80
  %tmp132 = phi i64 [ %tmp130, %bb129 ], [ %val2, %bb80 ]
  br label %bb133

bb133:                                            ; preds = %bb131, %bb78
  %tmp134 = phi i64 [ %tmp132, %bb131 ], [ %val2, %bb78 ]
  br label %bb135

bb135:                                            ; preds = %bb133, %bb76
  %tmp136 = phi i64 [ %tmp134, %bb133 ], [ %val2, %bb76 ]
  br label %bb137

bb137:                                            ; preds = %bb135, %bb74
  %tmp138 = phi i64 [ %tmp136, %bb135 ], [ %val2, %bb74 ]
  br label %bb139

bb139:                                            ; preds = %bb137, %bb72
  %tmp140 = phi i64 [ %tmp138, %bb137 ], [ %val2, %bb72 ]
  br label %bb141

bb141:                                            ; preds = %bb139, %bb70
  %tmp142 = phi i64 [ %tmp140, %bb139 ], [ %val2, %bb70 ]
  br label %bb143

bb143:                                            ; preds = %bb141, %bb68
  %tmp144 = phi i64 [ %tmp142, %bb141 ], [ %val2, %bb68 ]
  br label %bb145

bb145:                                            ; preds = %bb143, %bb66
  %tmp146 = phi i64 [ %tmp144, %bb143 ], [ %val2, %bb66 ]
  br label %bb147

bb147:                                            ; preds = %bb145, %bb64
  %tmp148 = phi i64 [ %tmp146, %bb145 ], [ %val2, %bb64 ]
  br label %bb149

bb149:                                            ; preds = %bb147, %bb62
  %tmp150 = phi i64 [ %tmp148, %bb147 ], [ %val2, %bb62 ]
  br label %bb151

bb151:                                            ; preds = %bb149, %bb60
  %tmp152 = phi i64 [ %tmp150, %bb149 ], [ %val2, %bb60 ]
  br label %bb153

bb153:                                            ; preds = %bb151, %bb58
  %tmp154 = phi i64 [ %tmp152, %bb151 ], [ %val2, %bb58 ]
  br label %bb155

bb155:                                            ; preds = %bb153, %bb56
  %tmp156 = phi i64 [ %tmp154, %bb153 ], [ %val2, %bb56 ]
  br label %bb157

bb157:                                            ; preds = %bb155, %bb54
  %tmp158 = phi i64 [ %tmp156, %bb155 ], [ %val2, %bb54 ]
  br label %bb159

bb159:                                            ; preds = %bb157, %bb52
  %tmp160 = phi i64 [ %tmp158, %bb157 ], [ %val2, %bb52 ]
  br label %bb161

bb161:                                            ; preds = %bb159, %bb50
  %tmp162 = phi i64 [ %tmp160, %bb159 ], [ %val2, %bb50 ]
  br label %bb163

bb163:                                            ; preds = %bb161, %bb48
  %tmp164 = phi i64 [ %tmp162, %bb161 ], [ %val2, %bb48 ]
  br label %bb165

bb165:                                            ; preds = %bb163, %bb46
  %tmp166 = phi i64 [ %tmp164, %bb163 ], [ %val2, %bb46 ]
  br label %bb167

bb167:                                            ; preds = %bb165, %bb44
  %tmp168 = phi i64 [ %tmp166, %bb165 ], [ %val2, %bb44 ]
  br label %bb169

bb169:                                            ; preds = %bb167, %bb42
  %tmp170 = phi i64 [ %tmp168, %bb167 ], [ %val2, %bb42 ]
  br label %bb171

bb171:                                            ; preds = %bb169, %bb40
  %tmp172 = phi i64 [ %tmp170, %bb169 ], [ %val2, %bb40 ]
  br label %bb173

bb173:                                            ; preds = %bb171, %bb38
  %tmp174 = phi i64 [ %tmp172, %bb171 ], [ %val2, %bb38 ]
  br label %bb175

bb175:                                            ; preds = %bb173, %bb37
  %tmp176 = phi i64 [ %tmp174, %bb173 ], [ %val2, %bb37 ]
  br label %bb177

bb177:                                            ; preds = %bb177, %bb175
  %tmp178 = phi i64 [ %tmp179, %bb177 ], [ %tmp176, %bb175 ]
  %tmp179 = add i64 %tmp178, 1
  store i64 %tmp179, ptr @res, align 8
  %tmp180 = icmp eq i64 %tmp178, %val2
  br i1 %tmp180, label %bb181, label %bb177

bb181:                                            ; preds = %bb177
  resume { ptr, i32 } undef
}

attributes #0 = { "unsafe-fp-math"="true" }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
