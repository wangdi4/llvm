//===-- LPURegisterInfo.td - LPU Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the LPU register file 
//===----------------------------------------------------------------------===//

// The LPU has no standard registers, though there will likely be registers
// local to individual functional units.
// Currently registers are used to represent LICs as a convenience, because
// that is what the LLVM CG deals in.
// It is TBD whether this will continue, or whether we create a new "LIC" type.
// Eventually, everything will likely be LICs, though it is possible this
// will be used for local register alloc.
// Ideally, LICs would be unbounded / allocated "on demand".


class LPUReg<bits<12> num, string n> : Register<n> {
  field bits<12> Num;
  let Namespace = "LPU";
  let Num = num;
}

// These are TEMPORARY "Global" registers to get things going.

def FP  : LPUReg< 0, "fp">;  // preserved (conventional FP)
def SP  : LPUReg< 1, "sp">;  // stack pointer
def RA  : LPUReg< 2, "ra">;  // return addr

def RCR : RegisterClass<"LPU", [i64], 64,
    (add FP, SP, RA)>;

// LPU Invocation arguments
def ARGS : LPUReg<8,  "args">;  // control sequencing arg
def ARG0 : LPUReg<9,  "arg0">;  // arg 0...
def ARG1 : LPUReg<10, "arg1">;
def ARG2 : LPUReg<11, "arg2">;
def ARG3 : LPUReg<12, "arg3">;
def ARG4 : LPUReg<13, "arg4">;
def ARG5 : LPUReg<14, "arg5">;
def ARG6 : LPUReg<15, "arg6">;
def ARG7 : LPUReg<16, "arg7">;
def ARG8 : LPUReg<17, "arg8">;
def ARG9 : LPUReg<18, "arg9">;
def ARG10 : LPUReg<19, "arg10">;
def ARG11 : LPUReg<20, "arg11">;
def ARG12 : LPUReg<21, "arg12">;
def ARG13 : LPUReg<22, "arg13">;
def ARG14 : LPUReg<23, "arg14">;
def ARG15 : LPUReg<24, "arg15">;

// Channel Class for arguments
def CCARG : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
    (add ARGS,ARG0,ARG1,ARG2,ARG3,ARG4,ARG5,ARG6,ARG7,
              ARG8,ARG9,ARG10,ARG11,ARG12,ARG13,ARG14,ARG15)>;

// LPU Invocation results
def RESS : LPUReg<25, "ress">;  // control sequence result
def RES0 : LPUReg<26, "res0">;  // result 0
def RES1 : LPUReg<27, "res1">;
def RES2 : LPUReg<28, "res2">;
def RES3 : LPUReg<29, "res3">;
def RES4 : LPUReg<30, "res4">;
def RES5 : LPUReg<31, "res5">;
def RES6 : LPUReg<32, "res6">;
def RES7 : LPUReg<33, "res7">;

// Channel Class for results
def CCRES : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
    (add RESS,RES0,RES1,RES2,RES3,RES4,RES5,RES6,RES7)>;

// This is a TEMPORARY work-around.  Ideally, LICs of the appropriate size
// should be allocated on demand, even if they still "look like" registers
// in some way.

// (In theory, a full sized LPU could have 1 LIC for each input, or several
// thousand.)

def C0 : LPUReg<64, "c0">;
def C1 : LPUReg<65, "c1">;
def C2 : LPUReg<66, "c2">;
def C3 : LPUReg<67, "c3">;
def C4 : LPUReg<68, "c4">;
def C5 : LPUReg<69, "c5">;
def C6 : LPUReg<70, "c6">;
def C7 : LPUReg<71, "c7">;
def C8 : LPUReg<72, "c8">;
def C9 : LPUReg<73, "c9">;
def C10 : LPUReg<74, "c10">;
def C11 : LPUReg<75, "c11">;
def C12 : LPUReg<76, "c12">;
def C13 : LPUReg<77, "c13">;
def C14 : LPUReg<78, "c14">;
def C15 : LPUReg<79, "c15">;
def C16 : LPUReg<80, "c16">;
def C17 : LPUReg<81, "c17">;
def C18 : LPUReg<82, "c18">;
def C19 : LPUReg<83, "c19">;
def C20 : LPUReg<84, "c20">;
def C21 : LPUReg<85, "c21">;
def C22 : LPUReg<86, "c22">;
def C23 : LPUReg<87, "c23">;
def C24 : LPUReg<88, "c24">;
def C25 : LPUReg<89, "c25">;
def C26 : LPUReg<90, "c26">;
def C27 : LPUReg<91, "c27">;
def C28 : LPUReg<92, "c28">;
def C29 : LPUReg<93, "c29">;
def C30 : LPUReg<94, "c30">;
def C31 : LPUReg<95, "c31">;
def C32 : LPUReg<96, "c32">;
def C33 : LPUReg<97, "c33">;
def C34 : LPUReg<98, "c34">;
def C35 : LPUReg<99, "c35">;
def C36 : LPUReg<100, "c36">;
def C37 : LPUReg<101, "c37">;
def C38 : LPUReg<102, "c38">;
def C39 : LPUReg<103, "c39">;
def C40 : LPUReg<104, "c40">;
def C41 : LPUReg<105, "c41">;
def C42 : LPUReg<106, "c42">;
def C43 : LPUReg<107, "c43">;
def C44 : LPUReg<108, "c44">;
def C45 : LPUReg<109, "c45">;
def C46 : LPUReg<110, "c46">;
def C47 : LPUReg<111, "c47">;
def C48 : LPUReg<112, "c48">;
def C49 : LPUReg<113, "c49">;
def C50 : LPUReg<114, "c50">;
def C51 : LPUReg<115, "c51">;
def C52 : LPUReg<116, "c52">;
def C53 : LPUReg<117, "c53">;
def C54 : LPUReg<118, "c54">;
def C55 : LPUReg<119, "c55">;
def C56 : LPUReg<120, "c56">;
def C57 : LPUReg<121, "c57">;
def C58 : LPUReg<122, "c58">;
def C59 : LPUReg<123, "c59">;
def C60 : LPUReg<124, "c60">;
def C61 : LPUReg<125, "c61">;
def C62 : LPUReg<126, "c62">;
def C63 : LPUReg<127, "c63">;
def C64 : LPUReg<128, "c64">;
def C65 : LPUReg<129, "c65">;
def C66 : LPUReg<130, "c66">;
def C67 : LPUReg<131, "c67">;
def C68 : LPUReg<132, "c68">;
def C69 : LPUReg<133, "c69">;
def C70 : LPUReg<134, "c70">;
def C71 : LPUReg<135, "c71">;
def C72 : LPUReg<136, "c72">;
def C73 : LPUReg<137, "c73">;
def C74 : LPUReg<138, "c74">;
def C75 : LPUReg<139, "c75">;
def C76 : LPUReg<140, "c76">;
def C77 : LPUReg<141, "c77">;
def C78 : LPUReg<142, "c78">;
def C79 : LPUReg<143, "c79">;
def C80 : LPUReg<144, "c80">;
def C81 : LPUReg<145, "c81">;
def C82 : LPUReg<146, "c82">;
def C83 : LPUReg<147, "c83">;
def C84 : LPUReg<148, "c84">;
def C85 : LPUReg<149, "c85">;
def C86 : LPUReg<150, "c86">;
def C87 : LPUReg<151, "c87">;
def C88 : LPUReg<152, "c88">;
def C89 : LPUReg<153, "c89">;
def C90 : LPUReg<154, "c90">;
def C91 : LPUReg<155, "c91">;
def C92 : LPUReg<156, "c92">;
def C93 : LPUReg<157, "c93">;
def C94 : LPUReg<158, "c94">;
def C95 : LPUReg<159, "c95">;
def C96 : LPUReg<160, "c96">;
def C97 : LPUReg<161, "c97">;
def C98 : LPUReg<162, "c98">;
def C99 : LPUReg<163, "c99">;
def C100 : LPUReg<164, "c100">;
def C101 : LPUReg<165, "c101">;
def C102 : LPUReg<166, "c102">;
def C103 : LPUReg<167, "c103">;
def C104 : LPUReg<168, "c104">;
def C105 : LPUReg<169, "c105">;
def C106 : LPUReg<170, "c106">;
def C107 : LPUReg<171, "c107">;
def C108 : LPUReg<172, "c108">;
def C109 : LPUReg<173, "c109">;
def C110 : LPUReg<174, "c110">;
def C111 : LPUReg<175, "c111">;
def C112 : LPUReg<176, "c112">;
def C113 : LPUReg<177, "c113">;
def C114 : LPUReg<178, "c114">;
def C115 : LPUReg<179, "c115">;
def C116 : LPUReg<180, "c116">;
def C117 : LPUReg<181, "c117">;
def C118 : LPUReg<182, "c118">;
def C119 : LPUReg<183, "c119">;
def C120 : LPUReg<184, "c120">;
def C121 : LPUReg<185, "c121">;
def C122 : LPUReg<186, "c122">;
def C123 : LPUReg<187, "c123">;
def C124 : LPUReg<188, "c124">;
def C125 : LPUReg<189, "c125">;
def C126 : LPUReg<190, "c126">;
def C127 : LPUReg<191, "c127">;
def C128 : LPUReg<192, "c128">;
def C129 : LPUReg<193, "c129">;
def C130 : LPUReg<194, "c130">;
def C131 : LPUReg<195, "c131">;
def C132 : LPUReg<196, "c132">;
def C133 : LPUReg<197, "c133">;
def C134 : LPUReg<198, "c134">;
def C135 : LPUReg<199, "c135">;
def C136 : LPUReg<200, "c136">;
def C137 : LPUReg<201, "c137">;
def C138 : LPUReg<202, "c138">;
def C139 : LPUReg<203, "c139">;
def C140 : LPUReg<204, "c140">;
def C141 : LPUReg<205, "c141">;
def C142 : LPUReg<206, "c142">;
def C143 : LPUReg<207, "c143">;
def C144 : LPUReg<208, "c144">;
def C145 : LPUReg<209, "c145">;
def C146 : LPUReg<210, "c146">;
def C147 : LPUReg<211, "c147">;
def C148 : LPUReg<212, "c148">;
def C149 : LPUReg<213, "c149">;
def C150 : LPUReg<214, "c150">;
def C151 : LPUReg<215, "c151">;
def C152 : LPUReg<216, "c152">;
def C153 : LPUReg<217, "c153">;
def C154 : LPUReg<218, "c154">;
def C155 : LPUReg<219, "c155">;
def C156 : LPUReg<220, "c156">;
def C157 : LPUReg<221, "c157">;
def C158 : LPUReg<222, "c158">;
def C159 : LPUReg<223, "c159">;
def C160 : LPUReg<224, "c160">;
def C161 : LPUReg<225, "c161">;
def C162 : LPUReg<226, "c162">;
def C163 : LPUReg<227, "c163">;
def C164 : LPUReg<228, "c164">;
def C165 : LPUReg<229, "c165">;
def C166 : LPUReg<230, "c166">;
def C167 : LPUReg<231, "c167">;
def C168 : LPUReg<232, "c168">;
def C169 : LPUReg<233, "c169">;
def C170 : LPUReg<234, "c170">;
def C171 : LPUReg<235, "c171">;
def C172 : LPUReg<236, "c172">;
def C173 : LPUReg<237, "c173">;
def C174 : LPUReg<238, "c174">;
def C175 : LPUReg<239, "c175">;
def C176 : LPUReg<240, "c176">;
def C177 : LPUReg<241, "c177">;
def C178 : LPUReg<242, "c178">;
def C179 : LPUReg<243, "c179">;
def C180 : LPUReg<244, "c180">;
def C181 : LPUReg<245, "c181">;
def C182 : LPUReg<246, "c182">;
def C183 : LPUReg<247, "c183">;
def C184 : LPUReg<248, "c184">;
def C185 : LPUReg<249, "c185">;
def C186 : LPUReg<250, "c186">;
def C187 : LPUReg<251, "c187">;
def C188 : LPUReg<252, "c188">;
def C189 : LPUReg<253, "c189">;
def C190 : LPUReg<254, "c190">;
def C191 : LPUReg<255, "c191">;
def C192 : LPUReg<256, "c192">;
def C193 : LPUReg<257, "c193">;
def C194 : LPUReg<258, "c194">;
def C195 : LPUReg<259, "c195">;
def C196 : LPUReg<260, "c196">;
def C197 : LPUReg<261, "c197">;
def C198 : LPUReg<262, "c198">;
def C199 : LPUReg<263, "c199">;
def C200 : LPUReg<264, "c200">;
def C201 : LPUReg<265, "c201">;
def C202 : LPUReg<266, "c202">;
def C203 : LPUReg<267, "c203">;
def C204 : LPUReg<268, "c204">;
def C205 : LPUReg<269, "c205">;
def C206 : LPUReg<270, "c206">;
def C207 : LPUReg<271, "c207">;
def C208 : LPUReg<272, "c208">;
def C209 : LPUReg<273, "c209">;
def C210 : LPUReg<274, "c210">;
def C211 : LPUReg<275, "c211">;
def C212 : LPUReg<276, "c212">;
def C213 : LPUReg<277, "c213">;
def C214 : LPUReg<278, "c214">;
def C215 : LPUReg<279, "c215">;
def C216 : LPUReg<280, "c216">;
def C217 : LPUReg<281, "c217">;
def C218 : LPUReg<282, "c218">;
def C219 : LPUReg<283, "c219">;
def C220 : LPUReg<284, "c220">;
def C221 : LPUReg<285, "c221">;
def C222 : LPUReg<286, "c222">;
def C223 : LPUReg<287, "c223">;
def C224 : LPUReg<288, "c224">;
def C225 : LPUReg<289, "c225">;
def C226 : LPUReg<290, "c226">;
def C227 : LPUReg<291, "c227">;
def C228 : LPUReg<292, "c228">;
def C229 : LPUReg<293, "c229">;
def C230 : LPUReg<294, "c230">;
def C231 : LPUReg<295, "c231">;
def C232 : LPUReg<296, "c232">;
def C233 : LPUReg<297, "c233">;
def C234 : LPUReg<298, "c234">;
def C235 : LPUReg<299, "c235">;
def C236 : LPUReg<300, "c236">;
def C237 : LPUReg<301, "c237">;
def C238 : LPUReg<302, "c238">;
def C239 : LPUReg<303, "c239">;
def C240 : LPUReg<304, "c240">;
def C241 : LPUReg<305, "c241">;
def C242 : LPUReg<306, "c242">;
def C243 : LPUReg<307, "c243">;
def C244 : LPUReg<308, "c244">;
def C245 : LPUReg<309, "c245">;
def C246 : LPUReg<310, "c246">;
def C247 : LPUReg<311, "c247">;
def C248 : LPUReg<312, "c248">;
def C249 : LPUReg<313, "c249">;
def C250 : LPUReg<314, "c250">;
def C251 : LPUReg<315, "c251">;
def C252 : LPUReg<316, "c252">;
def C253 : LPUReg<317, "c253">;
def C254 : LPUReg<318, "c254">;
def C255 : LPUReg<319, "c255">;
def C256 : LPUReg<320, "c256">;
def C257 : LPUReg<321, "c257">;
def C258 : LPUReg<322, "c258">;
def C259 : LPUReg<323, "c259">;
def C260 : LPUReg<324, "c260">;
def C261 : LPUReg<325, "c261">;
def C262 : LPUReg<326, "c262">;
def C263 : LPUReg<327, "c263">;
def C264 : LPUReg<328, "c264">;
def C265 : LPUReg<329, "c265">;
def C266 : LPUReg<330, "c266">;
def C267 : LPUReg<331, "c267">;
def C268 : LPUReg<332, "c268">;
def C269 : LPUReg<333, "c269">;
def C270 : LPUReg<334, "c270">;
def C271 : LPUReg<335, "c271">;
def C272 : LPUReg<336, "c272">;
def C273 : LPUReg<337, "c273">;
def C274 : LPUReg<338, "c274">;
def C275 : LPUReg<339, "c275">;
def C276 : LPUReg<340, "c276">;
def C277 : LPUReg<341, "c277">;
def C278 : LPUReg<342, "c278">;
def C279 : LPUReg<343, "c279">;
def C280 : LPUReg<344, "c280">;
def C281 : LPUReg<345, "c281">;
def C282 : LPUReg<346, "c282">;
def C283 : LPUReg<347, "c283">;
def C284 : LPUReg<348, "c284">;
def C285 : LPUReg<349, "c285">;
def C286 : LPUReg<350, "c286">;
def C287 : LPUReg<351, "c287">;
def C288 : LPUReg<352, "c288">;
def C289 : LPUReg<353, "c289">;
def C290 : LPUReg<354, "c290">;
def C291 : LPUReg<355, "c291">;
def C292 : LPUReg<356, "c292">;
def C293 : LPUReg<357, "c293">;
def C294 : LPUReg<358, "c294">;
def C295 : LPUReg<359, "c295">;
def C296 : LPUReg<360, "c296">;
def C297 : LPUReg<361, "c297">;
def C298 : LPUReg<362, "c298">;
def C299 : LPUReg<363, "c299">;
def C300 : LPUReg<364, "c300">;
def C301 : LPUReg<365, "c301">;
def C302 : LPUReg<366, "c302">;
def C303 : LPUReg<367, "c303">;
def C304 : LPUReg<368, "c304">;
def C305 : LPUReg<369, "c305">;
def C306 : LPUReg<370, "c306">;
def C307 : LPUReg<371, "c307">;
def C308 : LPUReg<372, "c308">;
def C309 : LPUReg<373, "c309">;
def C310 : LPUReg<374, "c310">;
def C311 : LPUReg<375, "c311">;
def C312 : LPUReg<376, "c312">;
def C313 : LPUReg<377, "c313">;
def C314 : LPUReg<378, "c314">;
def C315 : LPUReg<379, "c315">;
def C316 : LPUReg<380, "c316">;
def C317 : LPUReg<381, "c317">;
def C318 : LPUReg<382, "c318">;
def C319 : LPUReg<383, "c319">;
def C320 : LPUReg<384, "c320">;
def C321 : LPUReg<385, "c321">;
def C322 : LPUReg<386, "c322">;
def C323 : LPUReg<387, "c323">;
def C324 : LPUReg<388, "c324">;
def C325 : LPUReg<389, "c325">;
def C326 : LPUReg<390, "c326">;
def C327 : LPUReg<391, "c327">;
def C328 : LPUReg<392, "c328">;
def C329 : LPUReg<393, "c329">;
def C330 : LPUReg<394, "c330">;
def C331 : LPUReg<395, "c331">;
def C332 : LPUReg<396, "c332">;
def C333 : LPUReg<397, "c333">;
def C334 : LPUReg<398, "c334">;
def C335 : LPUReg<399, "c335">;
def C336 : LPUReg<400, "c336">;
def C337 : LPUReg<401, "c337">;
def C338 : LPUReg<402, "c338">;
def C339 : LPUReg<403, "c339">;
def C340 : LPUReg<404, "c340">;
def C341 : LPUReg<405, "c341">;
def C342 : LPUReg<406, "c342">;
def C343 : LPUReg<407, "c343">;
def C344 : LPUReg<408, "c344">;
def C345 : LPUReg<409, "c345">;
def C346 : LPUReg<410, "c346">;
def C347 : LPUReg<411, "c347">;
def C348 : LPUReg<412, "c348">;
def C349 : LPUReg<413, "c349">;
def C350 : LPUReg<414, "c350">;
def C351 : LPUReg<415, "c351">;
def C352 : LPUReg<416, "c352">;
def C353 : LPUReg<417, "c353">;
def C354 : LPUReg<418, "c354">;
def C355 : LPUReg<419, "c355">;
def C356 : LPUReg<420, "c356">;
def C357 : LPUReg<421, "c357">;
def C358 : LPUReg<422, "c358">;
def C359 : LPUReg<423, "c359">;
def C360 : LPUReg<424, "c360">;
def C361 : LPUReg<425, "c361">;
def C362 : LPUReg<426, "c362">;
def C363 : LPUReg<427, "c363">;
def C364 : LPUReg<428, "c364">;
def C365 : LPUReg<429, "c365">;
def C366 : LPUReg<430, "c366">;
def C367 : LPUReg<431, "c367">;
def C368 : LPUReg<432, "c368">;
def C369 : LPUReg<433, "c369">;
def C370 : LPUReg<434, "c370">;
def C371 : LPUReg<435, "c371">;
def C372 : LPUReg<436, "c372">;
def C373 : LPUReg<437, "c373">;
def C374 : LPUReg<438, "c374">;
def C375 : LPUReg<439, "c375">;
def C376 : LPUReg<440, "c376">;
def C377 : LPUReg<441, "c377">;
def C378 : LPUReg<442, "c378">;
def C379 : LPUReg<443, "c379">;
def C380 : LPUReg<444, "c380">;
def C381 : LPUReg<445, "c381">;
def C382 : LPUReg<446, "c382">;
def C383 : LPUReg<447, "c383">;
def C384 : LPUReg<448, "c384">;
def C385 : LPUReg<449, "c385">;
def C386 : LPUReg<450, "c386">;
def C387 : LPUReg<451, "c387">;
def C388 : LPUReg<452, "c388">;
def C389 : LPUReg<453, "c389">;
def C390 : LPUReg<454, "c390">;
def C391 : LPUReg<455, "c391">;
def C392 : LPUReg<456, "c392">;
def C393 : LPUReg<457, "c393">;
def C394 : LPUReg<458, "c394">;
def C395 : LPUReg<459, "c395">;
def C396 : LPUReg<460, "c396">;
def C397 : LPUReg<461, "c397">;
def C398 : LPUReg<462, "c398">;
def C399 : LPUReg<463, "c399">;
def C400 : LPUReg<464, "c400">;
def C401 : LPUReg<465, "c401">;
def C402 : LPUReg<466, "c402">;
def C403 : LPUReg<467, "c403">;
def C404 : LPUReg<468, "c404">;
def C405 : LPUReg<469, "c405">;
def C406 : LPUReg<470, "c406">;
def C407 : LPUReg<471, "c407">;
def C408 : LPUReg<472, "c408">;
def C409 : LPUReg<473, "c409">;
def C410 : LPUReg<474, "c410">;
def C411 : LPUReg<475, "c411">;
def C412 : LPUReg<476, "c412">;
def C413 : LPUReg<477, "c413">;
def C414 : LPUReg<478, "c414">;
def C415 : LPUReg<479, "c415">;
def C416 : LPUReg<480, "c416">;
def C417 : LPUReg<481, "c417">;
def C418 : LPUReg<482, "c418">;
def C419 : LPUReg<483, "c419">;
def C420 : LPUReg<484, "c420">;
def C421 : LPUReg<485, "c421">;
def C422 : LPUReg<486, "c422">;
def C423 : LPUReg<487, "c423">;
def C424 : LPUReg<488, "c424">;
def C425 : LPUReg<489, "c425">;
def C426 : LPUReg<490, "c426">;
def C427 : LPUReg<491, "c427">;
def C428 : LPUReg<492, "c428">;
def C429 : LPUReg<493, "c429">;
def C430 : LPUReg<494, "c430">;
def C431 : LPUReg<495, "c431">;
def C432 : LPUReg<496, "c432">;
def C433 : LPUReg<497, "c433">;
def C434 : LPUReg<498, "c434">;
def C435 : LPUReg<499, "c435">;
def C436 : LPUReg<500, "c436">;
def C437 : LPUReg<501, "c437">;
def C438 : LPUReg<502, "c438">;
def C439 : LPUReg<503, "c439">;
def C440 : LPUReg<504, "c440">;
def C441 : LPUReg<505, "c441">;
def C442 : LPUReg<506, "c442">;
def C443 : LPUReg<507, "c443">;
def C444 : LPUReg<508, "c444">;
def C445 : LPUReg<509, "c445">;
def C446 : LPUReg<510, "c446">;
def C447 : LPUReg<511, "c447">;
def C448 : LPUReg<512, "c448">;
def C449 : LPUReg<513, "c449">;
def C450 : LPUReg<514, "c450">;
def C451 : LPUReg<515, "c451">;
def C452 : LPUReg<516, "c452">;
def C453 : LPUReg<517, "c453">;
def C454 : LPUReg<518, "c454">;
def C455 : LPUReg<519, "c455">;
def C456 : LPUReg<520, "c456">;
def C457 : LPUReg<521, "c457">;
def C458 : LPUReg<522, "c458">;
def C459 : LPUReg<523, "c459">;
def C460 : LPUReg<524, "c460">;
def C461 : LPUReg<525, "c461">;
def C462 : LPUReg<526, "c462">;
def C463 : LPUReg<527, "c463">;
def C464 : LPUReg<528, "c464">;
def C465 : LPUReg<529, "c465">;
def C466 : LPUReg<530, "c466">;
def C467 : LPUReg<531, "c467">;
def C468 : LPUReg<532, "c468">;
def C469 : LPUReg<533, "c469">;
def C470 : LPUReg<534, "c470">;
def C471 : LPUReg<535, "c471">;
def C472 : LPUReg<536, "c472">;
def C473 : LPUReg<537, "c473">;
def C474 : LPUReg<538, "c474">;
def C475 : LPUReg<539, "c475">;
def C476 : LPUReg<540, "c476">;
def C477 : LPUReg<541, "c477">;
def C478 : LPUReg<542, "c478">;
def C479 : LPUReg<543, "c479">;
def C480 : LPUReg<544, "c480">;
def C481 : LPUReg<545, "c481">;
def C482 : LPUReg<546, "c482">;
def C483 : LPUReg<547, "c483">;
def C484 : LPUReg<548, "c484">;
def C485 : LPUReg<549, "c485">;
def C486 : LPUReg<550, "c486">;
def C487 : LPUReg<551, "c487">;
def C488 : LPUReg<552, "c488">;
def C489 : LPUReg<553, "c489">;
def C490 : LPUReg<554, "c490">;
def C491 : LPUReg<555, "c491">;
def C492 : LPUReg<556, "c492">;
def C493 : LPUReg<557, "c493">;
def C494 : LPUReg<558, "c494">;
def C495 : LPUReg<559, "c495">;
def C496 : LPUReg<560, "c496">;
def C497 : LPUReg<561, "c497">;
def C498 : LPUReg<562, "c498">;
def C499 : LPUReg<563, "c499">;
def C500 : LPUReg<564, "c500">;
def C501 : LPUReg<565, "c501">;
def C502 : LPUReg<566, "c502">;
def C503 : LPUReg<567, "c503">;
def C504 : LPUReg<568, "c504">;
def C505 : LPUReg<569, "c505">;
def C506 : LPUReg<570, "c506">;
def C507 : LPUReg<571, "c507">;
def C508 : LPUReg<572, "c508">;
def C509 : LPUReg<573, "c509">;
def C510 : LPUReg<574, "c510">;
def C511 : LPUReg<575, "c511">;
def C512 : LPUReg<576, "c512">;
def C513 : LPUReg<577, "c513">;
def C514 : LPUReg<578, "c514">;
def C515 : LPUReg<579, "c515">;
def C516 : LPUReg<580, "c516">;
def C517 : LPUReg<581, "c517">;
def C518 : LPUReg<582, "c518">;
def C519 : LPUReg<583, "c519">;
def C520 : LPUReg<584, "c520">;
def C521 : LPUReg<585, "c521">;
def C522 : LPUReg<586, "c522">;
def C523 : LPUReg<587, "c523">;
def C524 : LPUReg<588, "c524">;
def C525 : LPUReg<589, "c525">;
def C526 : LPUReg<590, "c526">;
def C527 : LPUReg<591, "c527">;
def C528 : LPUReg<592, "c528">;
def C529 : LPUReg<593, "c529">;
def C530 : LPUReg<594, "c530">;
def C531 : LPUReg<595, "c531">;
def C532 : LPUReg<596, "c532">;
def C533 : LPUReg<597, "c533">;
def C534 : LPUReg<598, "c534">;
def C535 : LPUReg<599, "c535">;
def C536 : LPUReg<600, "c536">;
def C537 : LPUReg<601, "c537">;
def C538 : LPUReg<602, "c538">;
def C539 : LPUReg<603, "c539">;
def C540 : LPUReg<604, "c540">;
def C541 : LPUReg<605, "c541">;
def C542 : LPUReg<606, "c542">;
def C543 : LPUReg<607, "c543">;
def C544 : LPUReg<608, "c544">;
def C545 : LPUReg<609, "c545">;
def C546 : LPUReg<610, "c546">;
def C547 : LPUReg<611, "c547">;
def C548 : LPUReg<612, "c548">;
def C549 : LPUReg<613, "c549">;
def C550 : LPUReg<614, "c550">;
def C551 : LPUReg<615, "c551">;
def C552 : LPUReg<616, "c552">;
def C553 : LPUReg<617, "c553">;
def C554 : LPUReg<618, "c554">;
def C555 : LPUReg<619, "c555">;
def C556 : LPUReg<620, "c556">;
def C557 : LPUReg<621, "c557">;
def C558 : LPUReg<622, "c558">;
def C559 : LPUReg<623, "c559">;
def C560 : LPUReg<624, "c560">;
def C561 : LPUReg<625, "c561">;
def C562 : LPUReg<626, "c562">;
def C563 : LPUReg<627, "c563">;
def C564 : LPUReg<628, "c564">;
def C565 : LPUReg<629, "c565">;
def C566 : LPUReg<630, "c566">;
def C567 : LPUReg<631, "c567">;
def C568 : LPUReg<632, "c568">;
def C569 : LPUReg<633, "c569">;
def C570 : LPUReg<634, "c570">;
def C571 : LPUReg<635, "c571">;
def C572 : LPUReg<636, "c572">;
def C573 : LPUReg<637, "c573">;
def C574 : LPUReg<638, "c574">;
def C575 : LPUReg<639, "c575">;
def C576 : LPUReg<640, "c576">;
def C577 : LPUReg<641, "c577">;
def C578 : LPUReg<642, "c578">;
def C579 : LPUReg<643, "c579">;
def C580 : LPUReg<644, "c580">;
def C581 : LPUReg<645, "c581">;
def C582 : LPUReg<646, "c582">;
def C583 : LPUReg<647, "c583">;
def C584 : LPUReg<648, "c584">;
def C585 : LPUReg<649, "c585">;
def C586 : LPUReg<650, "c586">;
def C587 : LPUReg<651, "c587">;
def C588 : LPUReg<652, "c588">;
def C589 : LPUReg<653, "c589">;
def C590 : LPUReg<654, "c590">;
def C591 : LPUReg<655, "c591">;
def C592 : LPUReg<656, "c592">;
def C593 : LPUReg<657, "c593">;
def C594 : LPUReg<658, "c594">;
def C595 : LPUReg<659, "c595">;
def C596 : LPUReg<660, "c596">;
def C597 : LPUReg<661, "c597">;
def C598 : LPUReg<662, "c598">;
def C599 : LPUReg<663, "c599">;
def C600 : LPUReg<664, "c600">;
def C601 : LPUReg<665, "c601">;
def C602 : LPUReg<666, "c602">;
def C603 : LPUReg<667, "c603">;
def C604 : LPUReg<668, "c604">;
def C605 : LPUReg<669, "c605">;
def C606 : LPUReg<670, "c606">;
def C607 : LPUReg<671, "c607">;
def C608 : LPUReg<672, "c608">;
def C609 : LPUReg<673, "c609">;
def C610 : LPUReg<674, "c610">;
def C611 : LPUReg<675, "c611">;
def C612 : LPUReg<676, "c612">;
def C613 : LPUReg<677, "c613">;
def C614 : LPUReg<678, "c614">;
def C615 : LPUReg<679, "c615">;
def C616 : LPUReg<680, "c616">;
def C617 : LPUReg<681, "c617">;
def C618 : LPUReg<682, "c618">;
def C619 : LPUReg<683, "c619">;
def C620 : LPUReg<684, "c620">;
def C621 : LPUReg<685, "c621">;
def C622 : LPUReg<686, "c622">;
def C623 : LPUReg<687, "c623">;
def C624 : LPUReg<688, "c624">;
def C625 : LPUReg<689, "c625">;
def C626 : LPUReg<690, "c626">;
def C627 : LPUReg<691, "c627">;
def C628 : LPUReg<692, "c628">;
def C629 : LPUReg<693, "c629">;
def C630 : LPUReg<694, "c630">;
def C631 : LPUReg<695, "c631">;
def C632 : LPUReg<696, "c632">;
def C633 : LPUReg<697, "c633">;
def C634 : LPUReg<698, "c634">;
def C635 : LPUReg<699, "c635">;
def C636 : LPUReg<700, "c636">;
def C637 : LPUReg<701, "c637">;
def C638 : LPUReg<702, "c638">;
def C639 : LPUReg<703, "c639">;
def C640 : LPUReg<704, "c640">;
def C641 : LPUReg<705, "c641">;
def C642 : LPUReg<706, "c642">;
def C643 : LPUReg<707, "c643">;
def C644 : LPUReg<708, "c644">;
def C645 : LPUReg<709, "c645">;
def C646 : LPUReg<710, "c646">;
def C647 : LPUReg<711, "c647">;
def C648 : LPUReg<712, "c648">;
def C649 : LPUReg<713, "c649">;
def C650 : LPUReg<714, "c650">;
def C651 : LPUReg<715, "c651">;
def C652 : LPUReg<716, "c652">;
def C653 : LPUReg<717, "c653">;
def C654 : LPUReg<718, "c654">;
def C655 : LPUReg<719, "c655">;
def C656 : LPUReg<720, "c656">;
def C657 : LPUReg<721, "c657">;
def C658 : LPUReg<722, "c658">;
def C659 : LPUReg<723, "c659">;
def C660 : LPUReg<724, "c660">;
def C661 : LPUReg<725, "c661">;
def C662 : LPUReg<726, "c662">;
def C663 : LPUReg<727, "c663">;
def C664 : LPUReg<728, "c664">;
def C665 : LPUReg<729, "c665">;
def C666 : LPUReg<730, "c666">;
def C667 : LPUReg<731, "c667">;
def C668 : LPUReg<732, "c668">;
def C669 : LPUReg<733, "c669">;
def C670 : LPUReg<734, "c670">;
def C671 : LPUReg<735, "c671">;
def C672 : LPUReg<736, "c672">;
def C673 : LPUReg<737, "c673">;
def C674 : LPUReg<738, "c674">;
def C675 : LPUReg<739, "c675">;
def C676 : LPUReg<740, "c676">;
def C677 : LPUReg<741, "c677">;
def C678 : LPUReg<742, "c678">;
def C679 : LPUReg<743, "c679">;
def C680 : LPUReg<744, "c680">;
def C681 : LPUReg<745, "c681">;
def C682 : LPUReg<746, "c682">;
def C683 : LPUReg<747, "c683">;
def C684 : LPUReg<748, "c684">;
def C685 : LPUReg<749, "c685">;
def C686 : LPUReg<750, "c686">;
def C687 : LPUReg<751, "c687">;
def C688 : LPUReg<752, "c688">;
def C689 : LPUReg<753, "c689">;
def C690 : LPUReg<754, "c690">;
def C691 : LPUReg<755, "c691">;
def C692 : LPUReg<756, "c692">;
def C693 : LPUReg<757, "c693">;
def C694 : LPUReg<758, "c694">;
def C695 : LPUReg<759, "c695">;
def C696 : LPUReg<760, "c696">;
def C697 : LPUReg<761, "c697">;
def C698 : LPUReg<762, "c698">;
def C699 : LPUReg<763, "c699">;
def C700 : LPUReg<764, "c700">;
def C701 : LPUReg<765, "c701">;
def C702 : LPUReg<766, "c702">;
def C703 : LPUReg<767, "c703">;
def C704 : LPUReg<768, "c704">;
def C705 : LPUReg<769, "c705">;
def C706 : LPUReg<770, "c706">;
def C707 : LPUReg<771, "c707">;
def C708 : LPUReg<772, "c708">;
def C709 : LPUReg<773, "c709">;
def C710 : LPUReg<774, "c710">;
def C711 : LPUReg<775, "c711">;
def C712 : LPUReg<776, "c712">;
def C713 : LPUReg<777, "c713">;
def C714 : LPUReg<778, "c714">;
def C715 : LPUReg<779, "c715">;
def C716 : LPUReg<780, "c716">;
def C717 : LPUReg<781, "c717">;
def C718 : LPUReg<782, "c718">;
def C719 : LPUReg<783, "c719">;
def C720 : LPUReg<784, "c720">;
def C721 : LPUReg<785, "c721">;
def C722 : LPUReg<786, "c722">;
def C723 : LPUReg<787, "c723">;
def C724 : LPUReg<788, "c724">;
def C725 : LPUReg<789, "c725">;
def C726 : LPUReg<790, "c726">;
def C727 : LPUReg<791, "c727">;
def C728 : LPUReg<792, "c728">;
def C729 : LPUReg<793, "c729">;
def C730 : LPUReg<794, "c730">;
def C731 : LPUReg<795, "c731">;
def C732 : LPUReg<796, "c732">;
def C733 : LPUReg<797, "c733">;
def C734 : LPUReg<798, "c734">;
def C735 : LPUReg<799, "c735">;
def C736 : LPUReg<800, "c736">;
def C737 : LPUReg<801, "c737">;
def C738 : LPUReg<802, "c738">;
def C739 : LPUReg<803, "c739">;
def C740 : LPUReg<804, "c740">;
def C741 : LPUReg<805, "c741">;
def C742 : LPUReg<806, "c742">;
def C743 : LPUReg<807, "c743">;
def C744 : LPUReg<808, "c744">;
def C745 : LPUReg<809, "c745">;
def C746 : LPUReg<810, "c746">;
def C747 : LPUReg<811, "c747">;
def C748 : LPUReg<812, "c748">;
def C749 : LPUReg<813, "c749">;
def C750 : LPUReg<814, "c750">;
def C751 : LPUReg<815, "c751">;
def C752 : LPUReg<816, "c752">;
def C753 : LPUReg<817, "c753">;
def C754 : LPUReg<818, "c754">;
def C755 : LPUReg<819, "c755">;
def C756 : LPUReg<820, "c756">;
def C757 : LPUReg<821, "c757">;
def C758 : LPUReg<822, "c758">;
def C759 : LPUReg<823, "c759">;
def C760 : LPUReg<824, "c760">;
def C761 : LPUReg<825, "c761">;
def C762 : LPUReg<826, "c762">;
def C763 : LPUReg<827, "c763">;
def C764 : LPUReg<828, "c764">;
def C765 : LPUReg<829, "c765">;
def C766 : LPUReg<830, "c766">;
def C767 : LPUReg<831, "c767">;
def C768 : LPUReg<832, "c768">;
def C769 : LPUReg<833, "c769">;
def C770 : LPUReg<834, "c770">;
def C771 : LPUReg<835, "c771">;
def C772 : LPUReg<836, "c772">;
def C773 : LPUReg<837, "c773">;
def C774 : LPUReg<838, "c774">;
def C775 : LPUReg<839, "c775">;
def C776 : LPUReg<840, "c776">;
def C777 : LPUReg<841, "c777">;
def C778 : LPUReg<842, "c778">;
def C779 : LPUReg<843, "c779">;
def C780 : LPUReg<844, "c780">;
def C781 : LPUReg<845, "c781">;
def C782 : LPUReg<846, "c782">;
def C783 : LPUReg<847, "c783">;
def C784 : LPUReg<848, "c784">;
def C785 : LPUReg<849, "c785">;
def C786 : LPUReg<850, "c786">;
def C787 : LPUReg<851, "c787">;
def C788 : LPUReg<852, "c788">;
def C789 : LPUReg<853, "c789">;
def C790 : LPUReg<854, "c790">;
def C791 : LPUReg<855, "c791">;
def C792 : LPUReg<856, "c792">;
def C793 : LPUReg<857, "c793">;
def C794 : LPUReg<858, "c794">;
def C795 : LPUReg<859, "c795">;
def C796 : LPUReg<860, "c796">;
def C797 : LPUReg<861, "c797">;
def C798 : LPUReg<862, "c798">;
def C799 : LPUReg<863, "c799">;
def C800 : LPUReg<864, "c800">;
def C801 : LPUReg<865, "c801">;
def C802 : LPUReg<866, "c802">;
def C803 : LPUReg<867, "c803">;
def C804 : LPUReg<868, "c804">;
def C805 : LPUReg<869, "c805">;
def C806 : LPUReg<870, "c806">;
def C807 : LPUReg<871, "c807">;
def C808 : LPUReg<872, "c808">;
def C809 : LPUReg<873, "c809">;
def C810 : LPUReg<874, "c810">;
def C811 : LPUReg<875, "c811">;
def C812 : LPUReg<876, "c812">;
def C813 : LPUReg<877, "c813">;
def C814 : LPUReg<878, "c814">;
def C815 : LPUReg<879, "c815">;
def C816 : LPUReg<880, "c816">;
def C817 : LPUReg<881, "c817">;
def C818 : LPUReg<882, "c818">;
def C819 : LPUReg<883, "c819">;
def C820 : LPUReg<884, "c820">;
def C821 : LPUReg<885, "c821">;
def C822 : LPUReg<886, "c822">;
def C823 : LPUReg<887, "c823">;
def C824 : LPUReg<888, "c824">;
def C825 : LPUReg<889, "c825">;
def C826 : LPUReg<890, "c826">;
def C827 : LPUReg<891, "c827">;
def C828 : LPUReg<892, "c828">;
def C829 : LPUReg<893, "c829">;
def C830 : LPUReg<894, "c830">;
def C831 : LPUReg<895, "c831">;
def C832 : LPUReg<896, "c832">;
def C833 : LPUReg<897, "c833">;
def C834 : LPUReg<898, "c834">;
def C835 : LPUReg<899, "c835">;
def C836 : LPUReg<900, "c836">;
def C837 : LPUReg<901, "c837">;
def C838 : LPUReg<902, "c838">;
def C839 : LPUReg<903, "c839">;
def C840 : LPUReg<904, "c840">;
def C841 : LPUReg<905, "c841">;
def C842 : LPUReg<906, "c842">;
def C843 : LPUReg<907, "c843">;
def C844 : LPUReg<908, "c844">;
def C845 : LPUReg<909, "c845">;
def C846 : LPUReg<910, "c846">;
def C847 : LPUReg<911, "c847">;
def C848 : LPUReg<912, "c848">;
def C849 : LPUReg<913, "c849">;
def C850 : LPUReg<914, "c850">;
def C851 : LPUReg<915, "c851">;
def C852 : LPUReg<916, "c852">;
def C853 : LPUReg<917, "c853">;
def C854 : LPUReg<918, "c854">;
def C855 : LPUReg<919, "c855">;
def C856 : LPUReg<920, "c856">;
def C857 : LPUReg<921, "c857">;
def C858 : LPUReg<922, "c858">;
def C859 : LPUReg<923, "c859">;
def C860 : LPUReg<924, "c860">;
def C861 : LPUReg<925, "c861">;
def C862 : LPUReg<926, "c862">;
def C863 : LPUReg<927, "c863">;
def C864 : LPUReg<928, "c864">;
def C865 : LPUReg<929, "c865">;
def C866 : LPUReg<930, "c866">;
def C867 : LPUReg<931, "c867">;
def C868 : LPUReg<932, "c868">;
def C869 : LPUReg<933, "c869">;
def C870 : LPUReg<934, "c870">;
def C871 : LPUReg<935, "c871">;
def C872 : LPUReg<936, "c872">;
def C873 : LPUReg<937, "c873">;
def C874 : LPUReg<938, "c874">;
def C875 : LPUReg<939, "c875">;
def C876 : LPUReg<940, "c876">;
def C877 : LPUReg<941, "c877">;
def C878 : LPUReg<942, "c878">;
def C879 : LPUReg<943, "c879">;
def C880 : LPUReg<944, "c880">;
def C881 : LPUReg<945, "c881">;
def C882 : LPUReg<946, "c882">;
def C883 : LPUReg<947, "c883">;
def C884 : LPUReg<948, "c884">;
def C885 : LPUReg<949, "c885">;
def C886 : LPUReg<950, "c886">;
def C887 : LPUReg<951, "c887">;
def C888 : LPUReg<952, "c888">;
def C889 : LPUReg<953, "c889">;
def C890 : LPUReg<954, "c890">;
def C891 : LPUReg<955, "c891">;
def C892 : LPUReg<956, "c892">;
def C893 : LPUReg<957, "c893">;
def C894 : LPUReg<958, "c894">;
def C895 : LPUReg<959, "c895">;
def C896 : LPUReg<960, "c896">;
def C897 : LPUReg<961, "c897">;
def C898 : LPUReg<962, "c898">;
def C899 : LPUReg<963, "c899">;
def C900 : LPUReg<964, "c900">;
def C901 : LPUReg<965, "c901">;
def C902 : LPUReg<966, "c902">;
def C903 : LPUReg<967, "c903">;
def C904 : LPUReg<968, "c904">;
def C905 : LPUReg<969, "c905">;
def C906 : LPUReg<970, "c906">;
def C907 : LPUReg<971, "c907">;
def C908 : LPUReg<972, "c908">;
def C909 : LPUReg<973, "c909">;
def C910 : LPUReg<974, "c910">;
def C911 : LPUReg<975, "c911">;
def C912 : LPUReg<976, "c912">;
def C913 : LPUReg<977, "c913">;
def C914 : LPUReg<978, "c914">;
def C915 : LPUReg<979, "c915">;
def C916 : LPUReg<980, "c916">;
def C917 : LPUReg<981, "c917">;
def C918 : LPUReg<982, "c918">;
def C919 : LPUReg<983, "c919">;
def C920 : LPUReg<984, "c920">;
def C921 : LPUReg<985, "c921">;
def C922 : LPUReg<986, "c922">;
def C923 : LPUReg<987, "c923">;
def C924 : LPUReg<988, "c924">;
def C925 : LPUReg<989, "c925">;
def C926 : LPUReg<990, "c926">;
def C927 : LPUReg<991, "c927">;
def C928 : LPUReg<992, "c928">;
def C929 : LPUReg<993, "c929">;
def C930 : LPUReg<994, "c930">;
def C931 : LPUReg<995, "c931">;
def C932 : LPUReg<996, "c932">;
def C933 : LPUReg<997, "c933">;
def C934 : LPUReg<998, "c934">;
def C935 : LPUReg<999, "c935">;
def C936 : LPUReg<1000, "c936">;
def C937 : LPUReg<1001, "c937">;
def C938 : LPUReg<1002, "c938">;
def C939 : LPUReg<1003, "c939">;
def C940 : LPUReg<1004, "c940">;
def C941 : LPUReg<1005, "c941">;
def C942 : LPUReg<1006, "c942">;
def C943 : LPUReg<1007, "c943">;
def C944 : LPUReg<1008, "c944">;
def C945 : LPUReg<1009, "c945">;
def C946 : LPUReg<1010, "c946">;
def C947 : LPUReg<1011, "c947">;
def C948 : LPUReg<1012, "c948">;
def C949 : LPUReg<1013, "c949">;
def C950 : LPUReg<1014, "c950">;
def C951 : LPUReg<1015, "c951">;
def C952 : LPUReg<1016, "c952">;
def C953 : LPUReg<1017, "c953">;
def C954 : LPUReg<1018, "c954">;
def C955 : LPUReg<1019, "c955">;
def C956 : LPUReg<1020, "c956">;
def C957 : LPUReg<1021, "c957">;
def C958 : LPUReg<1022, "c958">;
def C959 : LPUReg<1023, "c959">;
def C960 : LPUReg<1024, "c960">;
def C961 : LPUReg<1025, "c961">;
def C962 : LPUReg<1026, "c962">;
def C963 : LPUReg<1027, "c963">;
def C964 : LPUReg<1028, "c964">;
def C965 : LPUReg<1029, "c965">;
def C966 : LPUReg<1030, "c966">;
def C967 : LPUReg<1031, "c967">;
def C968 : LPUReg<1032, "c968">;
def C969 : LPUReg<1033, "c969">;
def C970 : LPUReg<1034, "c970">;
def C971 : LPUReg<1035, "c971">;
def C972 : LPUReg<1036, "c972">;
def C973 : LPUReg<1037, "c973">;
def C974 : LPUReg<1038, "c974">;
def C975 : LPUReg<1039, "c975">;
def C976 : LPUReg<1040, "c976">;
def C977 : LPUReg<1041, "c977">;
def C978 : LPUReg<1042, "c978">;
def C979 : LPUReg<1043, "c979">;
def C980 : LPUReg<1044, "c980">;
def C981 : LPUReg<1045, "c981">;
def C982 : LPUReg<1046, "c982">;
def C983 : LPUReg<1047, "c983">;
def C984 : LPUReg<1048, "c984">;
def C985 : LPUReg<1049, "c985">;
def C986 : LPUReg<1050, "c986">;
def C987 : LPUReg<1051, "c987">;
def C988 : LPUReg<1052, "c988">;
def C989 : LPUReg<1053, "c989">;
def C990 : LPUReg<1054, "c990">;
def C991 : LPUReg<1055, "c991">;
def C992 : LPUReg<1056, "c992">;
def C993 : LPUReg<1057, "c993">;
def C994 : LPUReg<1058, "c994">;
def C995 : LPUReg<1059, "c995">;
def C996 : LPUReg<1060, "c996">;
def C997 : LPUReg<1061, "c997">;
def C998 : LPUReg<1062, "c998">;
def C999 : LPUReg<1063, "c999">;
def C1000 : LPUReg<1064, "c1000">;
def C1001 : LPUReg<1065, "c1001">;
def C1002 : LPUReg<1066, "c1002">;
def C1003 : LPUReg<1067, "c1003">;
def C1004 : LPUReg<1068, "c1004">;
def C1005 : LPUReg<1069, "c1005">;
def C1006 : LPUReg<1070, "c1006">;
def C1007 : LPUReg<1071, "c1007">;
def C1008 : LPUReg<1072, "c1008">;
def C1009 : LPUReg<1073, "c1009">;
def C1010 : LPUReg<1074, "c1010">;
def C1011 : LPUReg<1075, "c1011">;
def C1012 : LPUReg<1076, "c1012">;
def C1013 : LPUReg<1077, "c1013">;
def C1014 : LPUReg<1078, "c1014">;
def C1015 : LPUReg<1079, "c1015">;
def C1016 : LPUReg<1080, "c1016">;
def C1017 : LPUReg<1081, "c1017">;
def C1018 : LPUReg<1082, "c1018">;
def C1019 : LPUReg<1083, "c1019">;
def C1020 : LPUReg<1084, "c1020">;
def C1021 : LPUReg<1085, "c1021">;
def C1022 : LPUReg<1086, "c1022">;
def C1023 : LPUReg<1087, "c1023">;
def C1024 : LPUReg<1088, "c1024">;
def C1025 : LPUReg<1089, "c1025">;
def C1026 : LPUReg<1090, "c1026">;
def C1027 : LPUReg<1091, "c1027">;
def C1028 : LPUReg<1092, "c1028">;
def C1029 : LPUReg<1093, "c1029">;
def C1030 : LPUReg<1094, "c1030">;
def C1031 : LPUReg<1095, "c1031">;
def C1032 : LPUReg<1096, "c1032">;
def C1033 : LPUReg<1097, "c1033">;
def C1034 : LPUReg<1098, "c1034">;
def C1035 : LPUReg<1099, "c1035">;
def C1036 : LPUReg<1100, "c1036">;
def C1037 : LPUReg<1101, "c1037">;
def C1038 : LPUReg<1102, "c1038">;
def C1039 : LPUReg<1103, "c1039">;
def C1040 : LPUReg<1104, "c1040">;
def C1041 : LPUReg<1105, "c1041">;
def C1042 : LPUReg<1106, "c1042">;
def C1043 : LPUReg<1107, "c1043">;
def C1044 : LPUReg<1108, "c1044">;
def C1045 : LPUReg<1109, "c1045">;
def C1046 : LPUReg<1110, "c1046">;
def C1047 : LPUReg<1111, "c1047">;
def C1048 : LPUReg<1112, "c1048">;
def C1049 : LPUReg<1113, "c1049">;
def C1050 : LPUReg<1114, "c1050">;
def C1051 : LPUReg<1115, "c1051">;
def C1052 : LPUReg<1116, "c1052">;
def C1053 : LPUReg<1117, "c1053">;
def C1054 : LPUReg<1118, "c1054">;
def C1055 : LPUReg<1119, "c1055">;
def C1056 : LPUReg<1120, "c1056">;
def C1057 : LPUReg<1121, "c1057">;
def C1058 : LPUReg<1122, "c1058">;
def C1059 : LPUReg<1123, "c1059">;
def C1060 : LPUReg<1124, "c1060">;
def C1061 : LPUReg<1125, "c1061">;
def C1062 : LPUReg<1126, "c1062">;
def C1063 : LPUReg<1127, "c1063">;
def C1064 : LPUReg<1128, "c1064">;
def C1065 : LPUReg<1129, "c1065">;
def C1066 : LPUReg<1130, "c1066">;
def C1067 : LPUReg<1131, "c1067">;
def C1068 : LPUReg<1132, "c1068">;
def C1069 : LPUReg<1133, "c1069">;
def C1070 : LPUReg<1134, "c1070">;
def C1071 : LPUReg<1135, "c1071">;
def C1072 : LPUReg<1136, "c1072">;
def C1073 : LPUReg<1137, "c1073">;
def C1074 : LPUReg<1138, "c1074">;
def C1075 : LPUReg<1139, "c1075">;
def C1076 : LPUReg<1140, "c1076">;
def C1077 : LPUReg<1141, "c1077">;
def C1078 : LPUReg<1142, "c1078">;
def C1079 : LPUReg<1143, "c1079">;
def C1080 : LPUReg<1144, "c1080">;
def C1081 : LPUReg<1145, "c1081">;
def C1082 : LPUReg<1146, "c1082">;
def C1083 : LPUReg<1147, "c1083">;
def C1084 : LPUReg<1148, "c1084">;
def C1085 : LPUReg<1149, "c1085">;
def C1086 : LPUReg<1150, "c1086">;
def C1087 : LPUReg<1151, "c1087">;
def C1088 : LPUReg<1152, "c1088">;
def C1089 : LPUReg<1153, "c1089">;
def C1090 : LPUReg<1154, "c1090">;
def C1091 : LPUReg<1155, "c1091">;
def C1092 : LPUReg<1156, "c1092">;
def C1093 : LPUReg<1157, "c1093">;
def C1094 : LPUReg<1158, "c1094">;
def C1095 : LPUReg<1159, "c1095">;
def C1096 : LPUReg<1160, "c1096">;
def C1097 : LPUReg<1161, "c1097">;
def C1098 : LPUReg<1162, "c1098">;
def C1099 : LPUReg<1163, "c1099">;
def C1100 : LPUReg<1164, "c1100">;
def C1101 : LPUReg<1165, "c1101">;
def C1102 : LPUReg<1166, "c1102">;
def C1103 : LPUReg<1167, "c1103">;
def C1104 : LPUReg<1168, "c1104">;
def C1105 : LPUReg<1169, "c1105">;
def C1106 : LPUReg<1170, "c1106">;
def C1107 : LPUReg<1171, "c1107">;
def C1108 : LPUReg<1172, "c1108">;
def C1109 : LPUReg<1173, "c1109">;
def C1110 : LPUReg<1174, "c1110">;
def C1111 : LPUReg<1175, "c1111">;
def C1112 : LPUReg<1176, "c1112">;
def C1113 : LPUReg<1177, "c1113">;
def C1114 : LPUReg<1178, "c1114">;
def C1115 : LPUReg<1179, "c1115">;
def C1116 : LPUReg<1180, "c1116">;
def C1117 : LPUReg<1181, "c1117">;
def C1118 : LPUReg<1182, "c1118">;
def C1119 : LPUReg<1183, "c1119">;
def C1120 : LPUReg<1184, "c1120">;
def C1121 : LPUReg<1185, "c1121">;
def C1122 : LPUReg<1186, "c1122">;
def C1123 : LPUReg<1187, "c1123">;
def C1124 : LPUReg<1188, "c1124">;
def C1125 : LPUReg<1189, "c1125">;
def C1126 : LPUReg<1190, "c1126">;
def C1127 : LPUReg<1191, "c1127">;
def C1128 : LPUReg<1192, "c1128">;
def C1129 : LPUReg<1193, "c1129">;
def C1130 : LPUReg<1194, "c1130">;
def C1131 : LPUReg<1195, "c1131">;
def C1132 : LPUReg<1196, "c1132">;
def C1133 : LPUReg<1197, "c1133">;
def C1134 : LPUReg<1198, "c1134">;
def C1135 : LPUReg<1199, "c1135">;
def C1136 : LPUReg<1200, "c1136">;
def C1137 : LPUReg<1201, "c1137">;
def C1138 : LPUReg<1202, "c1138">;
def C1139 : LPUReg<1203, "c1139">;
def C1140 : LPUReg<1204, "c1140">;
def C1141 : LPUReg<1205, "c1141">;
def C1142 : LPUReg<1206, "c1142">;
def C1143 : LPUReg<1207, "c1143">;
def C1144 : LPUReg<1208, "c1144">;
def C1145 : LPUReg<1209, "c1145">;
def C1146 : LPUReg<1210, "c1146">;
def C1147 : LPUReg<1211, "c1147">;
def C1148 : LPUReg<1212, "c1148">;
def C1149 : LPUReg<1213, "c1149">;
def C1150 : LPUReg<1214, "c1150">;
def C1151 : LPUReg<1215, "c1151">;
def C1152 : LPUReg<1216, "c1152">;
def C1153 : LPUReg<1217, "c1153">;
def C1154 : LPUReg<1218, "c1154">;
def C1155 : LPUReg<1219, "c1155">;
def C1156 : LPUReg<1220, "c1156">;
def C1157 : LPUReg<1221, "c1157">;
def C1158 : LPUReg<1222, "c1158">;
def C1159 : LPUReg<1223, "c1159">;
def C1160 : LPUReg<1224, "c1160">;
def C1161 : LPUReg<1225, "c1161">;
def C1162 : LPUReg<1226, "c1162">;
def C1163 : LPUReg<1227, "c1163">;
def C1164 : LPUReg<1228, "c1164">;
def C1165 : LPUReg<1229, "c1165">;
def C1166 : LPUReg<1230, "c1166">;
def C1167 : LPUReg<1231, "c1167">;
def C1168 : LPUReg<1232, "c1168">;
def C1169 : LPUReg<1233, "c1169">;
def C1170 : LPUReg<1234, "c1170">;
def C1171 : LPUReg<1235, "c1171">;
def C1172 : LPUReg<1236, "c1172">;
def C1173 : LPUReg<1237, "c1173">;
def C1174 : LPUReg<1238, "c1174">;
def C1175 : LPUReg<1239, "c1175">;
def C1176 : LPUReg<1240, "c1176">;
def C1177 : LPUReg<1241, "c1177">;
def C1178 : LPUReg<1242, "c1178">;
def C1179 : LPUReg<1243, "c1179">;
def C1180 : LPUReg<1244, "c1180">;
def C1181 : LPUReg<1245, "c1181">;
def C1182 : LPUReg<1246, "c1182">;
def C1183 : LPUReg<1247, "c1183">;
def C1184 : LPUReg<1248, "c1184">;
def C1185 : LPUReg<1249, "c1185">;
def C1186 : LPUReg<1250, "c1186">;
def C1187 : LPUReg<1251, "c1187">;
def C1188 : LPUReg<1252, "c1188">;
def C1189 : LPUReg<1253, "c1189">;
def C1190 : LPUReg<1254, "c1190">;
def C1191 : LPUReg<1255, "c1191">;
def C1192 : LPUReg<1256, "c1192">;
def C1193 : LPUReg<1257, "c1193">;
def C1194 : LPUReg<1258, "c1194">;
def C1195 : LPUReg<1259, "c1195">;
def C1196 : LPUReg<1260, "c1196">;
def C1197 : LPUReg<1261, "c1197">;
def C1198 : LPUReg<1262, "c1198">;
def C1199 : LPUReg<1263, "c1199">;
def C1200 : LPUReg<1264, "c1200">;
def C1201 : LPUReg<1265, "c1201">;
def C1202 : LPUReg<1266, "c1202">;
def C1203 : LPUReg<1267, "c1203">;
def C1204 : LPUReg<1268, "c1204">;
def C1205 : LPUReg<1269, "c1205">;
def C1206 : LPUReg<1270, "c1206">;
def C1207 : LPUReg<1271, "c1207">;
def C1208 : LPUReg<1272, "c1208">;
def C1209 : LPUReg<1273, "c1209">;
def C1210 : LPUReg<1274, "c1210">;
def C1211 : LPUReg<1275, "c1211">;
def C1212 : LPUReg<1276, "c1212">;
def C1213 : LPUReg<1277, "c1213">;
def C1214 : LPUReg<1278, "c1214">;
def C1215 : LPUReg<1279, "c1215">;
def C1216 : LPUReg<1280, "c1216">;
def C1217 : LPUReg<1281, "c1217">;
def C1218 : LPUReg<1282, "c1218">;
def C1219 : LPUReg<1283, "c1219">;
def C1220 : LPUReg<1284, "c1220">;
def C1221 : LPUReg<1285, "c1221">;
def C1222 : LPUReg<1286, "c1222">;
def C1223 : LPUReg<1287, "c1223">;
def C1224 : LPUReg<1288, "c1224">;
def C1225 : LPUReg<1289, "c1225">;
def C1226 : LPUReg<1290, "c1226">;
def C1227 : LPUReg<1291, "c1227">;
def C1228 : LPUReg<1292, "c1228">;
def C1229 : LPUReg<1293, "c1229">;
def C1230 : LPUReg<1294, "c1230">;
def C1231 : LPUReg<1295, "c1231">;
def C1232 : LPUReg<1296, "c1232">;
def C1233 : LPUReg<1297, "c1233">;
def C1234 : LPUReg<1298, "c1234">;
def C1235 : LPUReg<1299, "c1235">;
def C1236 : LPUReg<1300, "c1236">;
def C1237 : LPUReg<1301, "c1237">;
def C1238 : LPUReg<1302, "c1238">;
def C1239 : LPUReg<1303, "c1239">;
def C1240 : LPUReg<1304, "c1240">;
def C1241 : LPUReg<1305, "c1241">;
def C1242 : LPUReg<1306, "c1242">;
def C1243 : LPUReg<1307, "c1243">;
def C1244 : LPUReg<1308, "c1244">;
def C1245 : LPUReg<1309, "c1245">;
def C1246 : LPUReg<1310, "c1246">;
def C1247 : LPUReg<1311, "c1247">;
def C1248 : LPUReg<1312, "c1248">;
def C1249 : LPUReg<1313, "c1249">;
def C1250 : LPUReg<1314, "c1250">;
def C1251 : LPUReg<1315, "c1251">;
def C1252 : LPUReg<1316, "c1252">;
def C1253 : LPUReg<1317, "c1253">;
def C1254 : LPUReg<1318, "c1254">;
def C1255 : LPUReg<1319, "c1255">;
def C1256 : LPUReg<1320, "c1256">;
def C1257 : LPUReg<1321, "c1257">;
def C1258 : LPUReg<1322, "c1258">;
def C1259 : LPUReg<1323, "c1259">;
def C1260 : LPUReg<1324, "c1260">;
def C1261 : LPUReg<1325, "c1261">;
def C1262 : LPUReg<1326, "c1262">;
def C1263 : LPUReg<1327, "c1263">;
def C1264 : LPUReg<1328, "c1264">;
def C1265 : LPUReg<1329, "c1265">;
def C1266 : LPUReg<1330, "c1266">;
def C1267 : LPUReg<1331, "c1267">;
def C1268 : LPUReg<1332, "c1268">;
def C1269 : LPUReg<1333, "c1269">;
def C1270 : LPUReg<1334, "c1270">;
def C1271 : LPUReg<1335, "c1271">;
def C1272 : LPUReg<1336, "c1272">;
def C1273 : LPUReg<1337, "c1273">;
def C1274 : LPUReg<1338, "c1274">;
def C1275 : LPUReg<1339, "c1275">;
def C1276 : LPUReg<1340, "c1276">;
def C1277 : LPUReg<1341, "c1277">;
def C1278 : LPUReg<1342, "c1278">;
def C1279 : LPUReg<1343, "c1279">;
def C1280 : LPUReg<1344, "c1280">;
def C1281 : LPUReg<1345, "c1281">;
def C1282 : LPUReg<1346, "c1282">;
def C1283 : LPUReg<1347, "c1283">;
def C1284 : LPUReg<1348, "c1284">;
def C1285 : LPUReg<1349, "c1285">;
def C1286 : LPUReg<1350, "c1286">;
def C1287 : LPUReg<1351, "c1287">;
def C1288 : LPUReg<1352, "c1288">;
def C1289 : LPUReg<1353, "c1289">;
def C1290 : LPUReg<1354, "c1290">;
def C1291 : LPUReg<1355, "c1291">;
def C1292 : LPUReg<1356, "c1292">;
def C1293 : LPUReg<1357, "c1293">;
def C1294 : LPUReg<1358, "c1294">;
def C1295 : LPUReg<1359, "c1295">;
def C1296 : LPUReg<1360, "c1296">;
def C1297 : LPUReg<1361, "c1297">;
def C1298 : LPUReg<1362, "c1298">;
def C1299 : LPUReg<1363, "c1299">;
def C1300 : LPUReg<1364, "c1300">;
def C1301 : LPUReg<1365, "c1301">;
def C1302 : LPUReg<1366, "c1302">;
def C1303 : LPUReg<1367, "c1303">;
def C1304 : LPUReg<1368, "c1304">;
def C1305 : LPUReg<1369, "c1305">;
def C1306 : LPUReg<1370, "c1306">;
def C1307 : LPUReg<1371, "c1307">;
def C1308 : LPUReg<1372, "c1308">;
def C1309 : LPUReg<1373, "c1309">;
def C1310 : LPUReg<1374, "c1310">;
def C1311 : LPUReg<1375, "c1311">;
def C1312 : LPUReg<1376, "c1312">;
def C1313 : LPUReg<1377, "c1313">;
def C1314 : LPUReg<1378, "c1314">;
def C1315 : LPUReg<1379, "c1315">;
def C1316 : LPUReg<1380, "c1316">;
def C1317 : LPUReg<1381, "c1317">;
def C1318 : LPUReg<1382, "c1318">;
def C1319 : LPUReg<1383, "c1319">;
def C1320 : LPUReg<1384, "c1320">;
def C1321 : LPUReg<1385, "c1321">;
def C1322 : LPUReg<1386, "c1322">;
def C1323 : LPUReg<1387, "c1323">;
def C1324 : LPUReg<1388, "c1324">;
def C1325 : LPUReg<1389, "c1325">;
def C1326 : LPUReg<1390, "c1326">;
def C1327 : LPUReg<1391, "c1327">;
def C1328 : LPUReg<1392, "c1328">;
def C1329 : LPUReg<1393, "c1329">;
def C1330 : LPUReg<1394, "c1330">;
def C1331 : LPUReg<1395, "c1331">;
def C1332 : LPUReg<1396, "c1332">;
def C1333 : LPUReg<1397, "c1333">;
def C1334 : LPUReg<1398, "c1334">;
def C1335 : LPUReg<1399, "c1335">;
def C1336 : LPUReg<1400, "c1336">;
def C1337 : LPUReg<1401, "c1337">;
def C1338 : LPUReg<1402, "c1338">;
def C1339 : LPUReg<1403, "c1339">;
def C1340 : LPUReg<1404, "c1340">;
def C1341 : LPUReg<1405, "c1341">;
def C1342 : LPUReg<1406, "c1342">;
def C1343 : LPUReg<1407, "c1343">;
def C1344 : LPUReg<1408, "c1344">;
def C1345 : LPUReg<1409, "c1345">;
def C1346 : LPUReg<1410, "c1346">;
def C1347 : LPUReg<1411, "c1347">;
def C1348 : LPUReg<1412, "c1348">;
def C1349 : LPUReg<1413, "c1349">;
def C1350 : LPUReg<1414, "c1350">;
def C1351 : LPUReg<1415, "c1351">;
def C1352 : LPUReg<1416, "c1352">;
def C1353 : LPUReg<1417, "c1353">;
def C1354 : LPUReg<1418, "c1354">;
def C1355 : LPUReg<1419, "c1355">;
def C1356 : LPUReg<1420, "c1356">;
def C1357 : LPUReg<1421, "c1357">;
def C1358 : LPUReg<1422, "c1358">;
def C1359 : LPUReg<1423, "c1359">;
def C1360 : LPUReg<1424, "c1360">;
def C1361 : LPUReg<1425, "c1361">;
def C1362 : LPUReg<1426, "c1362">;
def C1363 : LPUReg<1427, "c1363">;
def C1364 : LPUReg<1428, "c1364">;
def C1365 : LPUReg<1429, "c1365">;
def C1366 : LPUReg<1430, "c1366">;
def C1367 : LPUReg<1431, "c1367">;
def C1368 : LPUReg<1432, "c1368">;
def C1369 : LPUReg<1433, "c1369">;
def C1370 : LPUReg<1434, "c1370">;
def C1371 : LPUReg<1435, "c1371">;
def C1372 : LPUReg<1436, "c1372">;
def C1373 : LPUReg<1437, "c1373">;
def C1374 : LPUReg<1438, "c1374">;
def C1375 : LPUReg<1439, "c1375">;
def C1376 : LPUReg<1440, "c1376">;
def C1377 : LPUReg<1441, "c1377">;
def C1378 : LPUReg<1442, "c1378">;
def C1379 : LPUReg<1443, "c1379">;
def C1380 : LPUReg<1444, "c1380">;
def C1381 : LPUReg<1445, "c1381">;
def C1382 : LPUReg<1446, "c1382">;
def C1383 : LPUReg<1447, "c1383">;
def C1384 : LPUReg<1448, "c1384">;
def C1385 : LPUReg<1449, "c1385">;
def C1386 : LPUReg<1450, "c1386">;
def C1387 : LPUReg<1451, "c1387">;
def C1388 : LPUReg<1452, "c1388">;
def C1389 : LPUReg<1453, "c1389">;
def C1390 : LPUReg<1454, "c1390">;
def C1391 : LPUReg<1455, "c1391">;
def C1392 : LPUReg<1456, "c1392">;
def C1393 : LPUReg<1457, "c1393">;
def C1394 : LPUReg<1458, "c1394">;
def C1395 : LPUReg<1459, "c1395">;
def C1396 : LPUReg<1460, "c1396">;
def C1397 : LPUReg<1461, "c1397">;
def C1398 : LPUReg<1462, "c1398">;
def C1399 : LPUReg<1463, "c1399">;
def C1400 : LPUReg<1464, "c1400">;
def C1401 : LPUReg<1465, "c1401">;
def C1402 : LPUReg<1466, "c1402">;
def C1403 : LPUReg<1467, "c1403">;
def C1404 : LPUReg<1468, "c1404">;
def C1405 : LPUReg<1469, "c1405">;
def C1406 : LPUReg<1470, "c1406">;
def C1407 : LPUReg<1471, "c1407">;
def C1408 : LPUReg<1472, "c1408">;
def C1409 : LPUReg<1473, "c1409">;
def C1410 : LPUReg<1474, "c1410">;
def C1411 : LPUReg<1475, "c1411">;
def C1412 : LPUReg<1476, "c1412">;
def C1413 : LPUReg<1477, "c1413">;
def C1414 : LPUReg<1478, "c1414">;
def C1415 : LPUReg<1479, "c1415">;
def C1416 : LPUReg<1480, "c1416">;
def C1417 : LPUReg<1481, "c1417">;
def C1418 : LPUReg<1482, "c1418">;
def C1419 : LPUReg<1483, "c1419">;
def C1420 : LPUReg<1484, "c1420">;
def C1421 : LPUReg<1485, "c1421">;
def C1422 : LPUReg<1486, "c1422">;
def C1423 : LPUReg<1487, "c1423">;
def C1424 : LPUReg<1488, "c1424">;
def C1425 : LPUReg<1489, "c1425">;
def C1426 : LPUReg<1490, "c1426">;
def C1427 : LPUReg<1491, "c1427">;
def C1428 : LPUReg<1492, "c1428">;
def C1429 : LPUReg<1493, "c1429">;
def C1430 : LPUReg<1494, "c1430">;
def C1431 : LPUReg<1495, "c1431">;
def C1432 : LPUReg<1496, "c1432">;
def C1433 : LPUReg<1497, "c1433">;
def C1434 : LPUReg<1498, "c1434">;
def C1435 : LPUReg<1499, "c1435">;
def C1436 : LPUReg<1500, "c1436">;
def C1437 : LPUReg<1501, "c1437">;
def C1438 : LPUReg<1502, "c1438">;
def C1439 : LPUReg<1503, "c1439">;
def C1440 : LPUReg<1504, "c1440">;
def C1441 : LPUReg<1505, "c1441">;
def C1442 : LPUReg<1506, "c1442">;
def C1443 : LPUReg<1507, "c1443">;
def C1444 : LPUReg<1508, "c1444">;
def C1445 : LPUReg<1509, "c1445">;
def C1446 : LPUReg<1510, "c1446">;
def C1447 : LPUReg<1511, "c1447">;
def C1448 : LPUReg<1512, "c1448">;
def C1449 : LPUReg<1513, "c1449">;
def C1450 : LPUReg<1514, "c1450">;
def C1451 : LPUReg<1515, "c1451">;
def C1452 : LPUReg<1516, "c1452">;
def C1453 : LPUReg<1517, "c1453">;
def C1454 : LPUReg<1518, "c1454">;
def C1455 : LPUReg<1519, "c1455">;
def C1456 : LPUReg<1520, "c1456">;
def C1457 : LPUReg<1521, "c1457">;
def C1458 : LPUReg<1522, "c1458">;
def C1459 : LPUReg<1523, "c1459">;
def C1460 : LPUReg<1524, "c1460">;
def C1461 : LPUReg<1525, "c1461">;
def C1462 : LPUReg<1526, "c1462">;
def C1463 : LPUReg<1527, "c1463">;
def C1464 : LPUReg<1528, "c1464">;
def C1465 : LPUReg<1529, "c1465">;
def C1466 : LPUReg<1530, "c1466">;
def C1467 : LPUReg<1531, "c1467">;
def C1468 : LPUReg<1532, "c1468">;
def C1469 : LPUReg<1533, "c1469">;
def C1470 : LPUReg<1534, "c1470">;
def C1471 : LPUReg<1535, "c1471">;
def C1472 : LPUReg<1536, "c1472">;
def C1473 : LPUReg<1537, "c1473">;
def C1474 : LPUReg<1538, "c1474">;
def C1475 : LPUReg<1539, "c1475">;
def C1476 : LPUReg<1540, "c1476">;
def C1477 : LPUReg<1541, "c1477">;
def C1478 : LPUReg<1542, "c1478">;
def C1479 : LPUReg<1543, "c1479">;
def C1480 : LPUReg<1544, "c1480">;
def C1481 : LPUReg<1545, "c1481">;
def C1482 : LPUReg<1546, "c1482">;
def C1483 : LPUReg<1547, "c1483">;
def C1484 : LPUReg<1548, "c1484">;
def C1485 : LPUReg<1549, "c1485">;
def C1486 : LPUReg<1550, "c1486">;
def C1487 : LPUReg<1551, "c1487">;
def C1488 : LPUReg<1552, "c1488">;
def C1489 : LPUReg<1553, "c1489">;
def C1490 : LPUReg<1554, "c1490">;
def C1491 : LPUReg<1555, "c1491">;
def C1492 : LPUReg<1556, "c1492">;
def C1493 : LPUReg<1557, "c1493">;
def C1494 : LPUReg<1558, "c1494">;
def C1495 : LPUReg<1559, "c1495">;
def C1496 : LPUReg<1560, "c1496">;
def C1497 : LPUReg<1561, "c1497">;
def C1498 : LPUReg<1562, "c1498">;
def C1499 : LPUReg<1563, "c1499">;
def C1500 : LPUReg<1564, "c1500">;
def C1501 : LPUReg<1565, "c1501">;
def C1502 : LPUReg<1566, "c1502">;
def C1503 : LPUReg<1567, "c1503">;
def C1504 : LPUReg<1568, "c1504">;
def C1505 : LPUReg<1569, "c1505">;
def C1506 : LPUReg<1570, "c1506">;
def C1507 : LPUReg<1571, "c1507">;
def C1508 : LPUReg<1572, "c1508">;
def C1509 : LPUReg<1573, "c1509">;
def C1510 : LPUReg<1574, "c1510">;
def C1511 : LPUReg<1575, "c1511">;
def C1512 : LPUReg<1576, "c1512">;
def C1513 : LPUReg<1577, "c1513">;
def C1514 : LPUReg<1578, "c1514">;
def C1515 : LPUReg<1579, "c1515">;
def C1516 : LPUReg<1580, "c1516">;
def C1517 : LPUReg<1581, "c1517">;
def C1518 : LPUReg<1582, "c1518">;
def C1519 : LPUReg<1583, "c1519">;
def C1520 : LPUReg<1584, "c1520">;
def C1521 : LPUReg<1585, "c1521">;
def C1522 : LPUReg<1586, "c1522">;
def C1523 : LPUReg<1587, "c1523">;
def C1524 : LPUReg<1588, "c1524">;
def C1525 : LPUReg<1589, "c1525">;
def C1526 : LPUReg<1590, "c1526">;
def C1527 : LPUReg<1591, "c1527">;
def C1528 : LPUReg<1592, "c1528">;
def C1529 : LPUReg<1593, "c1529">;
def C1530 : LPUReg<1594, "c1530">;
def C1531 : LPUReg<1595, "c1531">;
def C1532 : LPUReg<1596, "c1532">;
def C1533 : LPUReg<1597, "c1533">;
def C1534 : LPUReg<1598, "c1534">;
def C1535 : LPUReg<1599, "c1535">;
def C1536 : LPUReg<1600, "c1536">;
def C1537 : LPUReg<1601, "c1537">;
def C1538 : LPUReg<1602, "c1538">;
def C1539 : LPUReg<1603, "c1539">;
def C1540 : LPUReg<1604, "c1540">;
def C1541 : LPUReg<1605, "c1541">;
def C1542 : LPUReg<1606, "c1542">;
def C1543 : LPUReg<1607, "c1543">;
def C1544 : LPUReg<1608, "c1544">;
def C1545 : LPUReg<1609, "c1545">;
def C1546 : LPUReg<1610, "c1546">;
def C1547 : LPUReg<1611, "c1547">;
def C1548 : LPUReg<1612, "c1548">;
def C1549 : LPUReg<1613, "c1549">;
def C1550 : LPUReg<1614, "c1550">;
def C1551 : LPUReg<1615, "c1551">;
def C1552 : LPUReg<1616, "c1552">;
def C1553 : LPUReg<1617, "c1553">;
def C1554 : LPUReg<1618, "c1554">;
def C1555 : LPUReg<1619, "c1555">;
def C1556 : LPUReg<1620, "c1556">;
def C1557 : LPUReg<1621, "c1557">;
def C1558 : LPUReg<1622, "c1558">;
def C1559 : LPUReg<1623, "c1559">;
def C1560 : LPUReg<1624, "c1560">;
def C1561 : LPUReg<1625, "c1561">;
def C1562 : LPUReg<1626, "c1562">;
def C1563 : LPUReg<1627, "c1563">;
def C1564 : LPUReg<1628, "c1564">;
def C1565 : LPUReg<1629, "c1565">;
def C1566 : LPUReg<1630, "c1566">;
def C1567 : LPUReg<1631, "c1567">;
def C1568 : LPUReg<1632, "c1568">;
def C1569 : LPUReg<1633, "c1569">;
def C1570 : LPUReg<1634, "c1570">;
def C1571 : LPUReg<1635, "c1571">;
def C1572 : LPUReg<1636, "c1572">;
def C1573 : LPUReg<1637, "c1573">;
def C1574 : LPUReg<1638, "c1574">;
def C1575 : LPUReg<1639, "c1575">;
def C1576 : LPUReg<1640, "c1576">;
def C1577 : LPUReg<1641, "c1577">;
def C1578 : LPUReg<1642, "c1578">;
def C1579 : LPUReg<1643, "c1579">;
def C1580 : LPUReg<1644, "c1580">;
def C1581 : LPUReg<1645, "c1581">;
def C1582 : LPUReg<1646, "c1582">;
def C1583 : LPUReg<1647, "c1583">;
def C1584 : LPUReg<1648, "c1584">;
def C1585 : LPUReg<1649, "c1585">;
def C1586 : LPUReg<1650, "c1586">;
def C1587 : LPUReg<1651, "c1587">;
def C1588 : LPUReg<1652, "c1588">;
def C1589 : LPUReg<1653, "c1589">;
def C1590 : LPUReg<1654, "c1590">;
def C1591 : LPUReg<1655, "c1591">;
def C1592 : LPUReg<1656, "c1592">;
def C1593 : LPUReg<1657, "c1593">;
def C1594 : LPUReg<1658, "c1594">;
def C1595 : LPUReg<1659, "c1595">;
def C1596 : LPUReg<1660, "c1596">;
def C1597 : LPUReg<1661, "c1597">;
def C1598 : LPUReg<1662, "c1598">;
def C1599 : LPUReg<1663, "c1599">;
def C1600 : LPUReg<1664, "c1600">;
def C1601 : LPUReg<1665, "c1601">;
def C1602 : LPUReg<1666, "c1602">;
def C1603 : LPUReg<1667, "c1603">;
def C1604 : LPUReg<1668, "c1604">;
def C1605 : LPUReg<1669, "c1605">;
def C1606 : LPUReg<1670, "c1606">;
def C1607 : LPUReg<1671, "c1607">;
def C1608 : LPUReg<1672, "c1608">;
def C1609 : LPUReg<1673, "c1609">;
def C1610 : LPUReg<1674, "c1610">;
def C1611 : LPUReg<1675, "c1611">;
def C1612 : LPUReg<1676, "c1612">;
def C1613 : LPUReg<1677, "c1613">;
def C1614 : LPUReg<1678, "c1614">;
def C1615 : LPUReg<1679, "c1615">;
def C1616 : LPUReg<1680, "c1616">;
def C1617 : LPUReg<1681, "c1617">;
def C1618 : LPUReg<1682, "c1618">;
def C1619 : LPUReg<1683, "c1619">;
def C1620 : LPUReg<1684, "c1620">;
def C1621 : LPUReg<1685, "c1621">;
def C1622 : LPUReg<1686, "c1622">;
def C1623 : LPUReg<1687, "c1623">;
def C1624 : LPUReg<1688, "c1624">;
def C1625 : LPUReg<1689, "c1625">;
def C1626 : LPUReg<1690, "c1626">;
def C1627 : LPUReg<1691, "c1627">;
def C1628 : LPUReg<1692, "c1628">;
def C1629 : LPUReg<1693, "c1629">;
def C1630 : LPUReg<1694, "c1630">;
def C1631 : LPUReg<1695, "c1631">;
def C1632 : LPUReg<1696, "c1632">;
def C1633 : LPUReg<1697, "c1633">;
def C1634 : LPUReg<1698, "c1634">;
def C1635 : LPUReg<1699, "c1635">;
def C1636 : LPUReg<1700, "c1636">;
def C1637 : LPUReg<1701, "c1637">;
def C1638 : LPUReg<1702, "c1638">;
def C1639 : LPUReg<1703, "c1639">;
def C1640 : LPUReg<1704, "c1640">;
def C1641 : LPUReg<1705, "c1641">;
def C1642 : LPUReg<1706, "c1642">;
def C1643 : LPUReg<1707, "c1643">;
def C1644 : LPUReg<1708, "c1644">;
def C1645 : LPUReg<1709, "c1645">;
def C1646 : LPUReg<1710, "c1646">;
def C1647 : LPUReg<1711, "c1647">;
def C1648 : LPUReg<1712, "c1648">;
def C1649 : LPUReg<1713, "c1649">;
def C1650 : LPUReg<1714, "c1650">;
def C1651 : LPUReg<1715, "c1651">;
def C1652 : LPUReg<1716, "c1652">;
def C1653 : LPUReg<1717, "c1653">;
def C1654 : LPUReg<1718, "c1654">;
def C1655 : LPUReg<1719, "c1655">;
def C1656 : LPUReg<1720, "c1656">;
def C1657 : LPUReg<1721, "c1657">;
def C1658 : LPUReg<1722, "c1658">;
def C1659 : LPUReg<1723, "c1659">;
def C1660 : LPUReg<1724, "c1660">;
def C1661 : LPUReg<1725, "c1661">;
def C1662 : LPUReg<1726, "c1662">;
def C1663 : LPUReg<1727, "c1663">;
def C1664 : LPUReg<1728, "c1664">;
def C1665 : LPUReg<1729, "c1665">;
def C1666 : LPUReg<1730, "c1666">;
def C1667 : LPUReg<1731, "c1667">;
def C1668 : LPUReg<1732, "c1668">;
def C1669 : LPUReg<1733, "c1669">;
def C1670 : LPUReg<1734, "c1670">;
def C1671 : LPUReg<1735, "c1671">;
def C1672 : LPUReg<1736, "c1672">;
def C1673 : LPUReg<1737, "c1673">;
def C1674 : LPUReg<1738, "c1674">;
def C1675 : LPUReg<1739, "c1675">;
def C1676 : LPUReg<1740, "c1676">;
def C1677 : LPUReg<1741, "c1677">;
def C1678 : LPUReg<1742, "c1678">;
def C1679 : LPUReg<1743, "c1679">;
def C1680 : LPUReg<1744, "c1680">;
def C1681 : LPUReg<1745, "c1681">;
def C1682 : LPUReg<1746, "c1682">;
def C1683 : LPUReg<1747, "c1683">;
def C1684 : LPUReg<1748, "c1684">;
def C1685 : LPUReg<1749, "c1685">;
def C1686 : LPUReg<1750, "c1686">;
def C1687 : LPUReg<1751, "c1687">;
def C1688 : LPUReg<1752, "c1688">;
def C1689 : LPUReg<1753, "c1689">;
def C1690 : LPUReg<1754, "c1690">;
def C1691 : LPUReg<1755, "c1691">;
def C1692 : LPUReg<1756, "c1692">;
def C1693 : LPUReg<1757, "c1693">;
def C1694 : LPUReg<1758, "c1694">;
def C1695 : LPUReg<1759, "c1695">;
def C1696 : LPUReg<1760, "c1696">;
def C1697 : LPUReg<1761, "c1697">;
def C1698 : LPUReg<1762, "c1698">;
def C1699 : LPUReg<1763, "c1699">;
def C1700 : LPUReg<1764, "c1700">;
def C1701 : LPUReg<1765, "c1701">;
def C1702 : LPUReg<1766, "c1702">;
def C1703 : LPUReg<1767, "c1703">;
def C1704 : LPUReg<1768, "c1704">;
def C1705 : LPUReg<1769, "c1705">;
def C1706 : LPUReg<1770, "c1706">;
def C1707 : LPUReg<1771, "c1707">;
def C1708 : LPUReg<1772, "c1708">;
def C1709 : LPUReg<1773, "c1709">;
def C1710 : LPUReg<1774, "c1710">;
def C1711 : LPUReg<1775, "c1711">;
def C1712 : LPUReg<1776, "c1712">;
def C1713 : LPUReg<1777, "c1713">;
def C1714 : LPUReg<1778, "c1714">;
def C1715 : LPUReg<1779, "c1715">;
def C1716 : LPUReg<1780, "c1716">;
def C1717 : LPUReg<1781, "c1717">;
def C1718 : LPUReg<1782, "c1718">;
def C1719 : LPUReg<1783, "c1719">;
def C1720 : LPUReg<1784, "c1720">;
def C1721 : LPUReg<1785, "c1721">;
def C1722 : LPUReg<1786, "c1722">;
def C1723 : LPUReg<1787, "c1723">;
def C1724 : LPUReg<1788, "c1724">;
def C1725 : LPUReg<1789, "c1725">;
def C1726 : LPUReg<1790, "c1726">;
def C1727 : LPUReg<1791, "c1727">;
def C1728 : LPUReg<1792, "c1728">;
def C1729 : LPUReg<1793, "c1729">;
def C1730 : LPUReg<1794, "c1730">;
def C1731 : LPUReg<1795, "c1731">;
def C1732 : LPUReg<1796, "c1732">;
def C1733 : LPUReg<1797, "c1733">;
def C1734 : LPUReg<1798, "c1734">;
def C1735 : LPUReg<1799, "c1735">;
def C1736 : LPUReg<1800, "c1736">;
def C1737 : LPUReg<1801, "c1737">;
def C1738 : LPUReg<1802, "c1738">;
def C1739 : LPUReg<1803, "c1739">;
def C1740 : LPUReg<1804, "c1740">;
def C1741 : LPUReg<1805, "c1741">;
def C1742 : LPUReg<1806, "c1742">;
def C1743 : LPUReg<1807, "c1743">;
def C1744 : LPUReg<1808, "c1744">;
def C1745 : LPUReg<1809, "c1745">;
def C1746 : LPUReg<1810, "c1746">;
def C1747 : LPUReg<1811, "c1747">;
def C1748 : LPUReg<1812, "c1748">;
def C1749 : LPUReg<1813, "c1749">;
def C1750 : LPUReg<1814, "c1750">;
def C1751 : LPUReg<1815, "c1751">;
def C1752 : LPUReg<1816, "c1752">;
def C1753 : LPUReg<1817, "c1753">;
def C1754 : LPUReg<1818, "c1754">;
def C1755 : LPUReg<1819, "c1755">;
def C1756 : LPUReg<1820, "c1756">;
def C1757 : LPUReg<1821, "c1757">;
def C1758 : LPUReg<1822, "c1758">;
def C1759 : LPUReg<1823, "c1759">;
def C1760 : LPUReg<1824, "c1760">;
def C1761 : LPUReg<1825, "c1761">;
def C1762 : LPUReg<1826, "c1762">;
def C1763 : LPUReg<1827, "c1763">;
def C1764 : LPUReg<1828, "c1764">;
def C1765 : LPUReg<1829, "c1765">;
def C1766 : LPUReg<1830, "c1766">;
def C1767 : LPUReg<1831, "c1767">;
def C1768 : LPUReg<1832, "c1768">;
def C1769 : LPUReg<1833, "c1769">;
def C1770 : LPUReg<1834, "c1770">;
def C1771 : LPUReg<1835, "c1771">;
def C1772 : LPUReg<1836, "c1772">;
def C1773 : LPUReg<1837, "c1773">;
def C1774 : LPUReg<1838, "c1774">;
def C1775 : LPUReg<1839, "c1775">;
def C1776 : LPUReg<1840, "c1776">;
def C1777 : LPUReg<1841, "c1777">;
def C1778 : LPUReg<1842, "c1778">;
def C1779 : LPUReg<1843, "c1779">;
def C1780 : LPUReg<1844, "c1780">;
def C1781 : LPUReg<1845, "c1781">;
def C1782 : LPUReg<1846, "c1782">;
def C1783 : LPUReg<1847, "c1783">;
def C1784 : LPUReg<1848, "c1784">;
def C1785 : LPUReg<1849, "c1785">;
def C1786 : LPUReg<1850, "c1786">;
def C1787 : LPUReg<1851, "c1787">;
def C1788 : LPUReg<1852, "c1788">;
def C1789 : LPUReg<1853, "c1789">;
def C1790 : LPUReg<1854, "c1790">;
def C1791 : LPUReg<1855, "c1791">;
def C1792 : LPUReg<1856, "c1792">;
def C1793 : LPUReg<1857, "c1793">;
def C1794 : LPUReg<1858, "c1794">;
def C1795 : LPUReg<1859, "c1795">;
def C1796 : LPUReg<1860, "c1796">;
def C1797 : LPUReg<1861, "c1797">;
def C1798 : LPUReg<1862, "c1798">;
def C1799 : LPUReg<1863, "c1799">;
def C1800 : LPUReg<1864, "c1800">;
def C1801 : LPUReg<1865, "c1801">;
def C1802 : LPUReg<1866, "c1802">;
def C1803 : LPUReg<1867, "c1803">;
def C1804 : LPUReg<1868, "c1804">;
def C1805 : LPUReg<1869, "c1805">;
def C1806 : LPUReg<1870, "c1806">;
def C1807 : LPUReg<1871, "c1807">;
def C1808 : LPUReg<1872, "c1808">;
def C1809 : LPUReg<1873, "c1809">;
def C1810 : LPUReg<1874, "c1810">;
def C1811 : LPUReg<1875, "c1811">;
def C1812 : LPUReg<1876, "c1812">;
def C1813 : LPUReg<1877, "c1813">;
def C1814 : LPUReg<1878, "c1814">;
def C1815 : LPUReg<1879, "c1815">;
def C1816 : LPUReg<1880, "c1816">;
def C1817 : LPUReg<1881, "c1817">;
def C1818 : LPUReg<1882, "c1818">;
def C1819 : LPUReg<1883, "c1819">;
def C1820 : LPUReg<1884, "c1820">;
def C1821 : LPUReg<1885, "c1821">;
def C1822 : LPUReg<1886, "c1822">;
def C1823 : LPUReg<1887, "c1823">;
def C1824 : LPUReg<1888, "c1824">;
def C1825 : LPUReg<1889, "c1825">;
def C1826 : LPUReg<1890, "c1826">;
def C1827 : LPUReg<1891, "c1827">;
def C1828 : LPUReg<1892, "c1828">;
def C1829 : LPUReg<1893, "c1829">;
def C1830 : LPUReg<1894, "c1830">;
def C1831 : LPUReg<1895, "c1831">;
def C1832 : LPUReg<1896, "c1832">;
def C1833 : LPUReg<1897, "c1833">;
def C1834 : LPUReg<1898, "c1834">;
def C1835 : LPUReg<1899, "c1835">;
def C1836 : LPUReg<1900, "c1836">;
def C1837 : LPUReg<1901, "c1837">;
def C1838 : LPUReg<1902, "c1838">;
def C1839 : LPUReg<1903, "c1839">;
def C1840 : LPUReg<1904, "c1840">;
def C1841 : LPUReg<1905, "c1841">;
def C1842 : LPUReg<1906, "c1842">;
def C1843 : LPUReg<1907, "c1843">;
def C1844 : LPUReg<1908, "c1844">;
def C1845 : LPUReg<1909, "c1845">;
def C1846 : LPUReg<1910, "c1846">;
def C1847 : LPUReg<1911, "c1847">;
def C1848 : LPUReg<1912, "c1848">;
def C1849 : LPUReg<1913, "c1849">;
def C1850 : LPUReg<1914, "c1850">;
def C1851 : LPUReg<1915, "c1851">;
def C1852 : LPUReg<1916, "c1852">;
def C1853 : LPUReg<1917, "c1853">;
def C1854 : LPUReg<1918, "c1854">;
def C1855 : LPUReg<1919, "c1855">;
def C1856 : LPUReg<1920, "c1856">;
def C1857 : LPUReg<1921, "c1857">;
def C1858 : LPUReg<1922, "c1858">;
def C1859 : LPUReg<1923, "c1859">;
def C1860 : LPUReg<1924, "c1860">;
def C1861 : LPUReg<1925, "c1861">;
def C1862 : LPUReg<1926, "c1862">;
def C1863 : LPUReg<1927, "c1863">;
def C1864 : LPUReg<1928, "c1864">;
def C1865 : LPUReg<1929, "c1865">;
def C1866 : LPUReg<1930, "c1866">;
def C1867 : LPUReg<1931, "c1867">;
def C1868 : LPUReg<1932, "c1868">;
def C1869 : LPUReg<1933, "c1869">;
def C1870 : LPUReg<1934, "c1870">;
def C1871 : LPUReg<1935, "c1871">;
def C1872 : LPUReg<1936, "c1872">;
def C1873 : LPUReg<1937, "c1873">;
def C1874 : LPUReg<1938, "c1874">;
def C1875 : LPUReg<1939, "c1875">;
def C1876 : LPUReg<1940, "c1876">;
def C1877 : LPUReg<1941, "c1877">;
def C1878 : LPUReg<1942, "c1878">;
def C1879 : LPUReg<1943, "c1879">;
def C1880 : LPUReg<1944, "c1880">;
def C1881 : LPUReg<1945, "c1881">;
def C1882 : LPUReg<1946, "c1882">;
def C1883 : LPUReg<1947, "c1883">;
def C1884 : LPUReg<1948, "c1884">;
def C1885 : LPUReg<1949, "c1885">;
def C1886 : LPUReg<1950, "c1886">;
def C1887 : LPUReg<1951, "c1887">;
def C1888 : LPUReg<1952, "c1888">;
def C1889 : LPUReg<1953, "c1889">;
def C1890 : LPUReg<1954, "c1890">;
def C1891 : LPUReg<1955, "c1891">;
def C1892 : LPUReg<1956, "c1892">;
def C1893 : LPUReg<1957, "c1893">;
def C1894 : LPUReg<1958, "c1894">;
def C1895 : LPUReg<1959, "c1895">;
def C1896 : LPUReg<1960, "c1896">;
def C1897 : LPUReg<1961, "c1897">;
def C1898 : LPUReg<1962, "c1898">;
def C1899 : LPUReg<1963, "c1899">;
def C1900 : LPUReg<1964, "c1900">;
def C1901 : LPUReg<1965, "c1901">;
def C1902 : LPUReg<1966, "c1902">;
def C1903 : LPUReg<1967, "c1903">;
def C1904 : LPUReg<1968, "c1904">;
def C1905 : LPUReg<1969, "c1905">;
def C1906 : LPUReg<1970, "c1906">;
def C1907 : LPUReg<1971, "c1907">;
def C1908 : LPUReg<1972, "c1908">;
def C1909 : LPUReg<1973, "c1909">;
def C1910 : LPUReg<1974, "c1910">;
def C1911 : LPUReg<1975, "c1911">;
def C1912 : LPUReg<1976, "c1912">;
def C1913 : LPUReg<1977, "c1913">;
def C1914 : LPUReg<1978, "c1914">;
def C1915 : LPUReg<1979, "c1915">;
def C1916 : LPUReg<1980, "c1916">;
def C1917 : LPUReg<1981, "c1917">;
def C1918 : LPUReg<1982, "c1918">;
def C1919 : LPUReg<1983, "c1919">;
def C1920 : LPUReg<1984, "c1920">;
def C1921 : LPUReg<1985, "c1921">;
def C1922 : LPUReg<1986, "c1922">;
def C1923 : LPUReg<1987, "c1923">;
def C1924 : LPUReg<1988, "c1924">;
def C1925 : LPUReg<1989, "c1925">;
def C1926 : LPUReg<1990, "c1926">;
def C1927 : LPUReg<1991, "c1927">;
def C1928 : LPUReg<1992, "c1928">;
def C1929 : LPUReg<1993, "c1929">;
def C1930 : LPUReg<1994, "c1930">;
def C1931 : LPUReg<1995, "c1931">;
def C1932 : LPUReg<1996, "c1932">;
def C1933 : LPUReg<1997, "c1933">;
def C1934 : LPUReg<1998, "c1934">;
def C1935 : LPUReg<1999, "c1935">;
def C1936 : LPUReg<2000, "c1936">;
def C1937 : LPUReg<2001, "c1937">;
def C1938 : LPUReg<2002, "c1938">;
def C1939 : LPUReg<2003, "c1939">;
def C1940 : LPUReg<2004, "c1940">;
def C1941 : LPUReg<2005, "c1941">;
def C1942 : LPUReg<2006, "c1942">;
def C1943 : LPUReg<2007, "c1943">;
def C1944 : LPUReg<2008, "c1944">;
def C1945 : LPUReg<2009, "c1945">;
def C1946 : LPUReg<2010, "c1946">;
def C1947 : LPUReg<2011, "c1947">;
def C1948 : LPUReg<2012, "c1948">;
def C1949 : LPUReg<2013, "c1949">;
def C1950 : LPUReg<2014, "c1950">;
def C1951 : LPUReg<2015, "c1951">;
def C1952 : LPUReg<2016, "c1952">;
def C1953 : LPUReg<2017, "c1953">;
def C1954 : LPUReg<2018, "c1954">;
def C1955 : LPUReg<2019, "c1955">;
def C1956 : LPUReg<2020, "c1956">;
def C1957 : LPUReg<2021, "c1957">;
def C1958 : LPUReg<2022, "c1958">;
def C1959 : LPUReg<2023, "c1959">;
def C1960 : LPUReg<2024, "c1960">;
def C1961 : LPUReg<2025, "c1961">;
def C1962 : LPUReg<2026, "c1962">;
def C1963 : LPUReg<2027, "c1963">;
def C1964 : LPUReg<2028, "c1964">;
def C1965 : LPUReg<2029, "c1965">;
def C1966 : LPUReg<2030, "c1966">;
def C1967 : LPUReg<2031, "c1967">;
def C1968 : LPUReg<2032, "c1968">;
def C1969 : LPUReg<2033, "c1969">;
def C1970 : LPUReg<2034, "c1970">;
def C1971 : LPUReg<2035, "c1971">;
def C1972 : LPUReg<2036, "c1972">;
def C1973 : LPUReg<2037, "c1973">;
def C1974 : LPUReg<2038, "c1974">;
def C1975 : LPUReg<2039, "c1975">;
def C1976 : LPUReg<2040, "c1976">;
def C1977 : LPUReg<2041, "c1977">;
def C1978 : LPUReg<2042, "c1978">;
def C1979 : LPUReg<2043, "c1979">;
def C1980 : LPUReg<2044, "c1980">;
def C1981 : LPUReg<2045, "c1981">;
def C1982 : LPUReg<2046, "c1982">;
def C1983 : LPUReg<2047, "c1983">;
def C1984 : LPUReg<2048, "c1984">;
def C1985 : LPUReg<2049, "c1985">;
def C1986 : LPUReg<2050, "c1986">;
def C1987 : LPUReg<2051, "c1987">;
def C1988 : LPUReg<2052, "c1988">;
def C1989 : LPUReg<2053, "c1989">;
def C1990 : LPUReg<2054, "c1990">;
def C1991 : LPUReg<2055, "c1991">;
def C1992 : LPUReg<2056, "c1992">;
def C1993 : LPUReg<2057, "c1993">;
def C1994 : LPUReg<2058, "c1994">;
def C1995 : LPUReg<2059, "c1995">;
def C1996 : LPUReg<2060, "c1996">;
def C1997 : LPUReg<2061, "c1997">;
def C1998 : LPUReg<2062, "c1998">;
def C1999 : LPUReg<2063, "c1999">;
def C2000 : LPUReg<2064, "c2000">;
def C2001 : LPUReg<2065, "c2001">;
def C2002 : LPUReg<2066, "c2002">;
def C2003 : LPUReg<2067, "c2003">;
def C2004 : LPUReg<2068, "c2004">;
def C2005 : LPUReg<2069, "c2005">;
def C2006 : LPUReg<2070, "c2006">;
def C2007 : LPUReg<2071, "c2007">;
def C2008 : LPUReg<2072, "c2008">;
def C2009 : LPUReg<2073, "c2009">;
def C2010 : LPUReg<2074, "c2010">;
def C2011 : LPUReg<2075, "c2011">;
def C2012 : LPUReg<2076, "c2012">;
def C2013 : LPUReg<2077, "c2013">;
def C2014 : LPUReg<2078, "c2014">;
def C2015 : LPUReg<2079, "c2015">;
def C2016 : LPUReg<2080, "c2016">;
def C2017 : LPUReg<2081, "c2017">;
def C2018 : LPUReg<2082, "c2018">;
def C2019 : LPUReg<2083, "c2019">;
def C2020 : LPUReg<2084, "c2020">;
def C2021 : LPUReg<2085, "c2021">;
def C2022 : LPUReg<2086, "c2022">;
def C2023 : LPUReg<2087, "c2023">;
def C2024 : LPUReg<2088, "c2024">;
def C2025 : LPUReg<2089, "c2025">;
def C2026 : LPUReg<2090, "c2026">;
def C2027 : LPUReg<2091, "c2027">;
def C2028 : LPUReg<2092, "c2028">;
def C2029 : LPUReg<2093, "c2029">;
def C2030 : LPUReg<2094, "c2030">;
def C2031 : LPUReg<2095, "c2031">;
def C2032 : LPUReg<2096, "c2032">;
def C2033 : LPUReg<2097, "c2033">;
def C2034 : LPUReg<2098, "c2034">;
def C2035 : LPUReg<2099, "c2035">;
def C2036 : LPUReg<2100, "c2036">;
def C2037 : LPUReg<2101, "c2037">;
def C2038 : LPUReg<2102, "c2038">;
def C2039 : LPUReg<2103, "c2039">;
def C2040 : LPUReg<2104, "c2040">;
def C2041 : LPUReg<2105, "c2041">;
def C2042 : LPUReg<2106, "c2042">;
def C2043 : LPUReg<2107, "c2043">;
def C2044 : LPUReg<2108, "c2044">;
def C2045 : LPUReg<2109, "c2045">;
def C2046 : LPUReg<2110, "c2046">;
def C2047 : LPUReg<2111, "c2047">;

// generic channel class
def CC : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add   C0,   C1,   C2,   C3,   C4,   C5,   C6,   C7,   C8,   C9,
         C10,  C11,  C12,  C13,  C14,  C15,  C16,  C17,  C18,  C19,
         C20,  C21,  C22,  C23,  C24,  C25,  C26,  C27,  C28,  C29,
         C30,  C31,  C32,  C33,  C34,  C35,  C36,  C37,  C38,  C39,
         C40,  C41,  C42,  C43,  C44,  C45,  C46,  C47,  C48,  C49,
         C50,  C51,  C52,  C53,  C54,  C55,  C56,  C57,  C58,  C59,
         C60,  C61,  C62,  C63,  C64,  C65,  C66,  C67,  C68,  C69,
         C70,  C71,  C72,  C73,  C74,  C75,  C76,  C77,  C78,  C79,
         C80,  C81,  C82,  C83,  C84,  C85,  C86,  C87,  C88,  C89,
         C90,  C91,  C92,  C93,  C94,  C95,  C96,  C97,  C98,  C99,
        C100, C101, C102, C103, C104, C105, C106, C107, C108, C109,
        C110, C111, C112, C113, C114, C115, C116, C117, C118, C119,
        C120, C121, C122, C123, C124, C125, C126, C127, C128, C129,
        C130, C131, C132, C133, C134, C135, C136, C137, C138, C139,
        C140, C141, C142, C143, C144, C145, C146, C147, C148, C149,
        C150, C151, C152, C153, C154, C155, C156, C157, C158, C159,
        C160, C161, C162, C163, C164, C165, C166, C167, C168, C169,
        C170, C171, C172, C173, C174, C175, C176, C177, C178, C179,
        C180, C181, C182, C183, C184, C185, C186, C187, C188, C189,
        C190, C191, C192, C193, C194, C195, C196, C197, C198, C199,
        C200, C201, C202, C203, C204, C205, C206, C207, C208, C209,
        C210, C211, C212, C213, C214, C215, C216, C217, C218, C219,
        C220, C221, C222, C223, C224, C225, C226, C227, C228, C229,
        C230, C231, C232, C233, C234, C235, C236, C237, C238, C239,
        C240, C241, C242, C243, C244, C245, C246, C247, C248, C249,
        C250, C251, C252, C253, C254, C255, C256, C257, C258, C259,
        C260, C261, C262, C263, C264, C265, C266, C267, C268, C269,
        C270, C271, C272, C273, C274, C275, C276, C277, C278, C279,
        C280, C281, C282, C283, C284, C285, C286, C287, C288, C289,
        C290, C291, C292, C293, C294, C295, C296, C297, C298, C299,
        C300, C301, C302, C303, C304, C305, C306, C307, C308, C309,
        C310, C311, C312, C313, C314, C315, C316, C317, C318, C319,
        C320, C321, C322, C323, C324, C325, C326, C327, C328, C329,
        C330, C331, C332, C333, C334, C335, C336, C337, C338, C339,
        C340, C341, C342, C343, C344, C345, C346, C347, C348, C349,
        C350, C351, C352, C353, C354, C355, C356, C357, C358, C359,
        C360, C361, C362, C363, C364, C365, C366, C367, C368, C369,
        C370, C371, C372, C373, C374, C375, C376, C377, C378, C379,
        C380, C381, C382, C383, C384, C385, C386, C387, C388, C389,
        C390, C391, C392, C393, C394, C395, C396, C397, C398, C399,
        C400, C401, C402, C403, C404, C405, C406, C407, C408, C409,
        C410, C411, C412, C413, C414, C415, C416, C417, C418, C419,
        C420, C421, C422, C423, C424, C425, C426, C427, C428, C429,
        C430, C431, C432, C433, C434, C435, C436, C437, C438, C439,
        C440, C441, C442, C443, C444, C445, C446, C447, C448, C449,
        C450, C451, C452, C453, C454, C455, C456, C457, C458, C459,
        C460, C461, C462, C463, C464, C465, C466, C467, C468, C469,
        C470, C471, C472, C473, C474, C475, C476, C477, C478, C479,
        C480, C481, C482, C483, C484, C485, C486, C487, C488, C489,
        C490, C491, C492, C493, C494, C495, C496, C497, C498, C499,
        C500, C501, C502, C503, C504, C505, C506, C507, C508, C509,
        C510, C511, C512, C513, C514, C515, C516, C517, C518, C519,
        C520, C521, C522, C523, C524, C525, C526, C527, C528, C529,
        C530, C531, C532, C533, C534, C535, C536, C537, C538, C539,
        C540, C541, C542, C543, C544, C545, C546, C547, C548, C549,
        C550, C551, C552, C553, C554, C555, C556, C557, C558, C559,
        C560, C561, C562, C563, C564, C565, C566, C567, C568, C569,
        C570, C571, C572, C573, C574, C575, C576, C577, C578, C579,
        C580, C581, C582, C583, C584, C585, C586, C587, C588, C589,
        C590, C591, C592, C593, C594, C595, C596, C597, C598, C599,
        C600, C601, C602, C603, C604, C605, C606, C607, C608, C609,
        C610, C611, C612, C613, C614, C615, C616, C617, C618, C619,
        C620, C621, C622, C623, C624, C625, C626, C627, C628, C629,
        C630, C631, C632, C633, C634, C635, C636, C637, C638, C639,
        C640, C641, C642, C643, C644, C645, C646, C647, C648, C649,
        C650, C651, C652, C653, C654, C655, C656, C657, C658, C659,
        C660, C661, C662, C663, C664, C665, C666, C667, C668, C669,
        C670, C671, C672, C673, C674, C675, C676, C677, C678, C679,
        C680, C681, C682, C683, C684, C685, C686, C687, C688, C689,
        C690, C691, C692, C693, C694, C695, C696, C697, C698, C699,
        C700, C701, C702, C703, C704, C705, C706, C707, C708, C709,
        C710, C711, C712, C713, C714, C715, C716, C717, C718, C719,
        C720, C721, C722, C723, C724, C725, C726, C727, C728, C729,
        C730, C731, C732, C733, C734, C735, C736, C737, C738, C739,
        C740, C741, C742, C743, C744, C745, C746, C747, C748, C749,
        C750, C751, C752, C753, C754, C755, C756, C757, C758, C759,
        C760, C761, C762, C763, C764, C765, C766, C767, C768, C769,
        C770, C771, C772, C773, C774, C775, C776, C777, C778, C779,
        C780, C781, C782, C783, C784, C785, C786, C787, C788, C789,
        C790, C791, C792, C793, C794, C795, C796, C797, C798, C799,
        C800, C801, C802, C803, C804, C805, C806, C807, C808, C809,
        C810, C811, C812, C813, C814, C815, C816, C817, C818, C819,
        C820, C821, C822, C823, C824, C825, C826, C827, C828, C829,
        C830, C831, C832, C833, C834, C835, C836, C837, C838, C839,
        C840, C841, C842, C843, C844, C845, C846, C847, C848, C849,
        C850, C851, C852, C853, C854, C855, C856, C857, C858, C859,
        C860, C861, C862, C863, C864, C865, C866, C867, C868, C869,
        C870, C871, C872, C873, C874, C875, C876, C877, C878, C879,
        C880, C881, C882, C883, C884, C885, C886, C887, C888, C889,
        C890, C891, C892, C893, C894, C895, C896, C897, C898, C899,
        C900, C901, C902, C903, C904, C905, C906, C907, C908, C909,
        C910, C911, C912, C913, C914, C915, C916, C917, C918, C919,
        C920, C921, C922, C923, C924, C925, C926, C927, C928, C929,
        C930, C931, C932, C933, C934, C935, C936, C937, C938, C939,
        C940, C941, C942, C943, C944, C945, C946, C947, C948, C949,
        C950, C951, C952, C953, C954, C955, C956, C957, C958, C959,
        C960, C961, C962, C963, C964, C965, C966, C967, C968, C969,
        C970, C971, C972, C973, C974, C975, C976, C977, C978, C979,
        C980, C981, C982, C983, C984, C985, C986, C987, C988, C989,
        C990, C991, C992, C993, C994, C995, C996, C997, C998, C999,
        C1000, C1001, C1002, C1003, C1004, C1005, C1006, C1007, C1008, C1009,
        C1010, C1011, C1012, C1013, C1014, C1015, C1016, C1017, C1018, C1019,
        C1020, C1021, C1022, C1023, C1024, C1025, C1026, C1027, C1028, C1029,
        C1030, C1031, C1032, C1033, C1034, C1035, C1036, C1037, C1038, C1039,
        C1040, C1041, C1042, C1043, C1044, C1045, C1046, C1047, C1048, C1049,
        C1050, C1051, C1052, C1053, C1054, C1055, C1056, C1057, C1058, C1059,
        C1060, C1061, C1062, C1063, C1064, C1065, C1066, C1067, C1068, C1069,
        C1070, C1071, C1072, C1073, C1074, C1075, C1076, C1077, C1078, C1079,
        C1080, C1081, C1082, C1083, C1084, C1085, C1086, C1087, C1088, C1089,
        C1090, C1091, C1092, C1093, C1094, C1095, C1096, C1097, C1098, C1099,
        C1100, C1101, C1102, C1103, C1104, C1105, C1106, C1107, C1108, C1109,
        C1110, C1111, C1112, C1113, C1114, C1115, C1116, C1117, C1118, C1119,
        C1120, C1121, C1122, C1123, C1124, C1125, C1126, C1127, C1128, C1129,
        C1130, C1131, C1132, C1133, C1134, C1135, C1136, C1137, C1138, C1139,
        C1140, C1141, C1142, C1143, C1144, C1145, C1146, C1147, C1148, C1149,
        C1150, C1151, C1152, C1153, C1154, C1155, C1156, C1157, C1158, C1159,
        C1160, C1161, C1162, C1163, C1164, C1165, C1166, C1167, C1168, C1169,
        C1170, C1171, C1172, C1173, C1174, C1175, C1176, C1177, C1178, C1179,
        C1180, C1181, C1182, C1183, C1184, C1185, C1186, C1187, C1188, C1189,
        C1190, C1191, C1192, C1193, C1194, C1195, C1196, C1197, C1198, C1199,
        C1200, C1201, C1202, C1203, C1204, C1205, C1206, C1207, C1208, C1209,
        C1210, C1211, C1212, C1213, C1214, C1215, C1216, C1217, C1218, C1219,
        C1220, C1221, C1222, C1223, C1224, C1225, C1226, C1227, C1228, C1229,
        C1230, C1231, C1232, C1233, C1234, C1235, C1236, C1237, C1238, C1239,
        C1240, C1241, C1242, C1243, C1244, C1245, C1246, C1247, C1248, C1249,
        C1250, C1251, C1252, C1253, C1254, C1255, C1256, C1257, C1258, C1259,
        C1260, C1261, C1262, C1263, C1264, C1265, C1266, C1267, C1268, C1269,
        C1270, C1271, C1272, C1273, C1274, C1275, C1276, C1277, C1278, C1279,
        C1280, C1281, C1282, C1283, C1284, C1285, C1286, C1287, C1288, C1289,
        C1290, C1291, C1292, C1293, C1294, C1295, C1296, C1297, C1298, C1299,
        C1300, C1301, C1302, C1303, C1304, C1305, C1306, C1307, C1308, C1309,
        C1310, C1311, C1312, C1313, C1314, C1315, C1316, C1317, C1318, C1319,
        C1320, C1321, C1322, C1323, C1324, C1325, C1326, C1327, C1328, C1329,
        C1330, C1331, C1332, C1333, C1334, C1335, C1336, C1337, C1338, C1339,
        C1340, C1341, C1342, C1343, C1344, C1345, C1346, C1347, C1348, C1349,
        C1350, C1351, C1352, C1353, C1354, C1355, C1356, C1357, C1358, C1359,
        C1360, C1361, C1362, C1363, C1364, C1365, C1366, C1367, C1368, C1369,
        C1370, C1371, C1372, C1373, C1374, C1375, C1376, C1377, C1378, C1379,
        C1380, C1381, C1382, C1383, C1384, C1385, C1386, C1387, C1388, C1389,
        C1390, C1391, C1392, C1393, C1394, C1395, C1396, C1397, C1398, C1399,
        C1400, C1401, C1402, C1403, C1404, C1405, C1406, C1407, C1408, C1409,
        C1410, C1411, C1412, C1413, C1414, C1415, C1416, C1417, C1418, C1419,
        C1420, C1421, C1422, C1423, C1424, C1425, C1426, C1427, C1428, C1429,
        C1430, C1431, C1432, C1433, C1434, C1435, C1436, C1437, C1438, C1439,
        C1440, C1441, C1442, C1443, C1444, C1445, C1446, C1447, C1448, C1449,
        C1450, C1451, C1452, C1453, C1454, C1455, C1456, C1457, C1458, C1459,
        C1460, C1461, C1462, C1463, C1464, C1465, C1466, C1467, C1468, C1469,
        C1470, C1471, C1472, C1473, C1474, C1475, C1476, C1477, C1478, C1479,
        C1480, C1481, C1482, C1483, C1484, C1485, C1486, C1487, C1488, C1489,
        C1490, C1491, C1492, C1493, C1494, C1495, C1496, C1497, C1498, C1499,
        C1500, C1501, C1502, C1503, C1504, C1505, C1506, C1507, C1508, C1509,
        C1510, C1511, C1512, C1513, C1514, C1515, C1516, C1517, C1518, C1519,
        C1520, C1521, C1522, C1523, C1524, C1525, C1526, C1527, C1528, C1529,
        C1530, C1531, C1532, C1533, C1534, C1535, C1536, C1537, C1538, C1539,
        C1540, C1541, C1542, C1543, C1544, C1545, C1546, C1547, C1548, C1549,
        C1550, C1551, C1552, C1553, C1554, C1555, C1556, C1557, C1558, C1559,
        C1560, C1561, C1562, C1563, C1564, C1565, C1566, C1567, C1568, C1569,
        C1570, C1571, C1572, C1573, C1574, C1575, C1576, C1577, C1578, C1579,
        C1580, C1581, C1582, C1583, C1584, C1585, C1586, C1587, C1588, C1589,
        C1590, C1591, C1592, C1593, C1594, C1595, C1596, C1597, C1598, C1599,
        C1600, C1601, C1602, C1603, C1604, C1605, C1606, C1607, C1608, C1609,
        C1610, C1611, C1612, C1613, C1614, C1615, C1616, C1617, C1618, C1619,
        C1620, C1621, C1622, C1623, C1624, C1625, C1626, C1627, C1628, C1629,
        C1630, C1631, C1632, C1633, C1634, C1635, C1636, C1637, C1638, C1639,
        C1640, C1641, C1642, C1643, C1644, C1645, C1646, C1647, C1648, C1649,
        C1650, C1651, C1652, C1653, C1654, C1655, C1656, C1657, C1658, C1659,
        C1660, C1661, C1662, C1663, C1664, C1665, C1666, C1667, C1668, C1669,
        C1670, C1671, C1672, C1673, C1674, C1675, C1676, C1677, C1678, C1679,
        C1680, C1681, C1682, C1683, C1684, C1685, C1686, C1687, C1688, C1689,
        C1690, C1691, C1692, C1693, C1694, C1695, C1696, C1697, C1698, C1699,
        C1700, C1701, C1702, C1703, C1704, C1705, C1706, C1707, C1708, C1709,
        C1710, C1711, C1712, C1713, C1714, C1715, C1716, C1717, C1718, C1719,
        C1720, C1721, C1722, C1723, C1724, C1725, C1726, C1727, C1728, C1729,
        C1730, C1731, C1732, C1733, C1734, C1735, C1736, C1737, C1738, C1739,
        C1740, C1741, C1742, C1743, C1744, C1745, C1746, C1747, C1748, C1749,
        C1750, C1751, C1752, C1753, C1754, C1755, C1756, C1757, C1758, C1759,
        C1760, C1761, C1762, C1763, C1764, C1765, C1766, C1767, C1768, C1769,
        C1770, C1771, C1772, C1773, C1774, C1775, C1776, C1777, C1778, C1779,
        C1780, C1781, C1782, C1783, C1784, C1785, C1786, C1787, C1788, C1789,
        C1790, C1791, C1792, C1793, C1794, C1795, C1796, C1797, C1798, C1799,
        C1800, C1801, C1802, C1803, C1804, C1805, C1806, C1807, C1808, C1809,
        C1810, C1811, C1812, C1813, C1814, C1815, C1816, C1817, C1818, C1819,
        C1820, C1821, C1822, C1823, C1824, C1825, C1826, C1827, C1828, C1829,
        C1830, C1831, C1832, C1833, C1834, C1835, C1836, C1837, C1838, C1839,
        C1840, C1841, C1842, C1843, C1844, C1845, C1846, C1847, C1848, C1849,
        C1850, C1851, C1852, C1853, C1854, C1855, C1856, C1857, C1858, C1859,
        C1860, C1861, C1862, C1863, C1864, C1865, C1866, C1867, C1868, C1869,
        C1870, C1871, C1872, C1873, C1874, C1875, C1876, C1877, C1878, C1879,
        C1880, C1881, C1882, C1883, C1884, C1885, C1886, C1887, C1888, C1889,
        C1890, C1891, C1892, C1893, C1894, C1895, C1896, C1897, C1898, C1899,
        C1900, C1901, C1902, C1903, C1904, C1905, C1906, C1907, C1908, C1909,
        C1910, C1911, C1912, C1913, C1914, C1915, C1916, C1917, C1918, C1919,
        C1920, C1921, C1922, C1923, C1924, C1925, C1926, C1927, C1928, C1929,
        C1930, C1931, C1932, C1933, C1934, C1935, C1936, C1937, C1938, C1939,
        C1940, C1941, C1942, C1943, C1944, C1945, C1946, C1947, C1948, C1949,
        C1950, C1951, C1952, C1953, C1954, C1955, C1956, C1957, C1958, C1959,
        C1960, C1961, C1962, C1963, C1964, C1965, C1966, C1967, C1968, C1969,
        C1970, C1971, C1972, C1973, C1974, C1975, C1976, C1977, C1978, C1979,
        C1980, C1981, C1982, C1983, C1984, C1985, C1986, C1987, C1988, C1989,
        C1990, C1991, C1992, C1993, C1994, C1995, C1996, C1997, C1998, C1999,
        C2000, C2001, C2002, C2003, C2004, C2005, C2006, C2007, C2008, C2009,
        C2010, C2011, C2012, C2013, C2014, C2015, C2016, C2017, C2018, C2019,
        C2020, C2021, C2022, C2023, C2024, C2025, C2026, C2027, C2028, C2029,
        C2030, C2031, C2032, C2033, C2034, C2035, C2036, C2037, C2038, C2039,
        C2040, C2041, C2042, C2043, C2044, C2045, C2046, C2047)>;
def CCI1  : RegisterClass<"LPU",  [i1], 64, (add CC)>;
def CCI8  : RegisterClass<"LPU",  [i8], 64, (add CC)>;
def CCI16 : RegisterClass<"LPU", [i16], 64, (add CC)>;
def CCI32 : RegisterClass<"LPU", [i32], 64, (add CC)>;
def CCI64 : RegisterClass<"LPU", [i64], 64, (add CC)>;
def CCF16 : RegisterClass<"LPU", [f16], 64, (add CC)>;
def CCF32 : RegisterClass<"LPU", [f32], 64, (add CC)>;
def CCF64 : RegisterClass<"LPU", [f64], 64, (add CC)>;
