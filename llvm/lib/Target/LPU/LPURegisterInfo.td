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

def FP  : LPUReg<4093, "%fp">;  // preserved (conventional FP)
def SP  : LPUReg<4094, "%sp">;  // stack pointer
def RA  : LPUReg<4095, "%ra">;  // return addr

def GLOBALREG : RegisterClass<"LPU", [i64], 64,
    (add FP, SP, RA)>;

// This is a TEMPORARY work-around.  Ideally, LICs of the appropriate size
// should be allocated on demand, even if they still "look like" registers
// in some way.

// One question is if LICs could be handled similarly to VREGs (i.e. simply
// a number...)  That seems like it might be desirable on several levels.

// (In theory, a full sized LPU could have 1 LIC for each input, or a total
// of several thousand.)

def IGN : LPUReg<4092, "%ign">; // Ignored (always writeable, never readable)

def C0 : LPUReg<0, "%c0">;
def C1 : LPUReg<1, "%c1">;
def C2 : LPUReg<2, "%c2">;
def C3 : LPUReg<3, "%c3">;
def C4 : LPUReg<4, "%c4">;
def C5 : LPUReg<5, "%c5">;
def C6 : LPUReg<6, "%c6">;
def C7 : LPUReg<7, "%c7">;
def C8 : LPUReg<8, "%c8">;
def C9 : LPUReg<9, "%c9">;
def C10 : LPUReg<10, "%c10">;
def C11 : LPUReg<11, "%c11">;
def C12 : LPUReg<12, "%c12">;
def C13 : LPUReg<13, "%c13">;
def C14 : LPUReg<14, "%c14">;
def C15 : LPUReg<15, "%c15">;
def C16 : LPUReg<16, "%c16">;
def C17 : LPUReg<17, "%c17">;
def C18 : LPUReg<18, "%c18">;
def C19 : LPUReg<19, "%c19">;
def C20 : LPUReg<20, "%c20">;
def C21 : LPUReg<21, "%c21">;
def C22 : LPUReg<22, "%c22">;
def C23 : LPUReg<23, "%c23">;
def C24 : LPUReg<24, "%c24">;
def C25 : LPUReg<25, "%c25">;
def C26 : LPUReg<26, "%c26">;
def C27 : LPUReg<27, "%c27">;
def C28 : LPUReg<28, "%c28">;
def C29 : LPUReg<29, "%c29">;
def C30 : LPUReg<30, "%c30">;
def C31 : LPUReg<31, "%c31">;
def C32 : LPUReg<32, "%c32">;
def C33 : LPUReg<33, "%c33">;
def C34 : LPUReg<34, "%c34">;
def C35 : LPUReg<35, "%c35">;
def C36 : LPUReg<36, "%c36">;
def C37 : LPUReg<37, "%c37">;
def C38 : LPUReg<38, "%c38">;
def C39 : LPUReg<39, "%c39">;
def C40 : LPUReg<40, "%c40">;
def C41 : LPUReg<41, "%c41">;
def C42 : LPUReg<42, "%c42">;
def C43 : LPUReg<43, "%c43">;
def C44 : LPUReg<44, "%c44">;
def C45 : LPUReg<45, "%c45">;
def C46 : LPUReg<46, "%c46">;
def C47 : LPUReg<47, "%c47">;
def C48 : LPUReg<48, "%c48">;
def C49 : LPUReg<49, "%c49">;
def C50 : LPUReg<50, "%c50">;
def C51 : LPUReg<51, "%c51">;
def C52 : LPUReg<52, "%c52">;
def C53 : LPUReg<53, "%c53">;
def C54 : LPUReg<54, "%c54">;
def C55 : LPUReg<55, "%c55">;
def C56 : LPUReg<56, "%c56">;
def C57 : LPUReg<57, "%c57">;
def C58 : LPUReg<58, "%c58">;
def C59 : LPUReg<59, "%c59">;
def C60 : LPUReg<60, "%c60">;
def C61 : LPUReg<61, "%c61">;
def C62 : LPUReg<62, "%c62">;
def C63 : LPUReg<63, "%c63">;
def C64 : LPUReg<64, "%c64">;
def C65 : LPUReg<65, "%c65">;
def C66 : LPUReg<66, "%c66">;
def C67 : LPUReg<67, "%c67">;
def C68 : LPUReg<68, "%c68">;
def C69 : LPUReg<69, "%c69">;
def C70 : LPUReg<70, "%c70">;
def C71 : LPUReg<71, "%c71">;
def C72 : LPUReg<72, "%c72">;
def C73 : LPUReg<73, "%c73">;
def C74 : LPUReg<74, "%c74">;
def C75 : LPUReg<75, "%c75">;
def C76 : LPUReg<76, "%c76">;
def C77 : LPUReg<77, "%c77">;
def C78 : LPUReg<78, "%c78">;
def C79 : LPUReg<79, "%c79">;
def C80 : LPUReg<80, "%c80">;
def C81 : LPUReg<81, "%c81">;
def C82 : LPUReg<82, "%c82">;
def C83 : LPUReg<83, "%c83">;
def C84 : LPUReg<84, "%c84">;
def C85 : LPUReg<85, "%c85">;
def C86 : LPUReg<86, "%c86">;
def C87 : LPUReg<87, "%c87">;
def C88 : LPUReg<88, "%c88">;
def C89 : LPUReg<89, "%c89">;
def C90 : LPUReg<90, "%c90">;
def C91 : LPUReg<91, "%c91">;
def C92 : LPUReg<92, "%c92">;
def C93 : LPUReg<93, "%c93">;
def C94 : LPUReg<94, "%c94">;
def C95 : LPUReg<95, "%c95">;
def C96 : LPUReg<96, "%c96">;
def C97 : LPUReg<97, "%c97">;
def C98 : LPUReg<98, "%c98">;
def C99 : LPUReg<99, "%c99">;
def C100 : LPUReg<100, "%c100">;
def C101 : LPUReg<101, "%c101">;
def C102 : LPUReg<102, "%c102">;
def C103 : LPUReg<103, "%c103">;
def C104 : LPUReg<104, "%c104">;
def C105 : LPUReg<105, "%c105">;
def C106 : LPUReg<106, "%c106">;
def C107 : LPUReg<107, "%c107">;
def C108 : LPUReg<108, "%c108">;
def C109 : LPUReg<109, "%c109">;
def C110 : LPUReg<110, "%c110">;
def C111 : LPUReg<111, "%c111">;
def C112 : LPUReg<112, "%c112">;
def C113 : LPUReg<113, "%c113">;
def C114 : LPUReg<114, "%c114">;
def C115 : LPUReg<115, "%c115">;
def C116 : LPUReg<116, "%c116">;
def C117 : LPUReg<117, "%c117">;
def C118 : LPUReg<118, "%c118">;
def C119 : LPUReg<119, "%c119">;
def C120 : LPUReg<120, "%c120">;
def C121 : LPUReg<121, "%c121">;
def C122 : LPUReg<122, "%c122">;
def C123 : LPUReg<123, "%c123">;
def C124 : LPUReg<124, "%c124">;
def C125 : LPUReg<125, "%c125">;
def C126 : LPUReg<126, "%c126">;
def C127 : LPUReg<127, "%c127">;
def C128 : LPUReg<128, "%c128">;
def C129 : LPUReg<129, "%c129">;
def C130 : LPUReg<130, "%c130">;
def C131 : LPUReg<131, "%c131">;
def C132 : LPUReg<132, "%c132">;
def C133 : LPUReg<133, "%c133">;
def C134 : LPUReg<134, "%c134">;
def C135 : LPUReg<135, "%c135">;
def C136 : LPUReg<136, "%c136">;
def C137 : LPUReg<137, "%c137">;
def C138 : LPUReg<138, "%c138">;
def C139 : LPUReg<139, "%c139">;
def C140 : LPUReg<140, "%c140">;
def C141 : LPUReg<141, "%c141">;
def C142 : LPUReg<142, "%c142">;
def C143 : LPUReg<143, "%c143">;
def C144 : LPUReg<144, "%c144">;
def C145 : LPUReg<145, "%c145">;
def C146 : LPUReg<146, "%c146">;
def C147 : LPUReg<147, "%c147">;
def C148 : LPUReg<148, "%c148">;
def C149 : LPUReg<149, "%c149">;
def C150 : LPUReg<150, "%c150">;
def C151 : LPUReg<151, "%c151">;
def C152 : LPUReg<152, "%c152">;
def C153 : LPUReg<153, "%c153">;
def C154 : LPUReg<154, "%c154">;
def C155 : LPUReg<155, "%c155">;
def C156 : LPUReg<156, "%c156">;
def C157 : LPUReg<157, "%c157">;
def C158 : LPUReg<158, "%c158">;
def C159 : LPUReg<159, "%c159">;
def C160 : LPUReg<160, "%c160">;
def C161 : LPUReg<161, "%c161">;
def C162 : LPUReg<162, "%c162">;
def C163 : LPUReg<163, "%c163">;
def C164 : LPUReg<164, "%c164">;
def C165 : LPUReg<165, "%c165">;
def C166 : LPUReg<166, "%c166">;
def C167 : LPUReg<167, "%c167">;
def C168 : LPUReg<168, "%c168">;
def C169 : LPUReg<169, "%c169">;
def C170 : LPUReg<170, "%c170">;
def C171 : LPUReg<171, "%c171">;
def C172 : LPUReg<172, "%c172">;
def C173 : LPUReg<173, "%c173">;
def C174 : LPUReg<174, "%c174">;
def C175 : LPUReg<175, "%c175">;
def C176 : LPUReg<176, "%c176">;
def C177 : LPUReg<177, "%c177">;
def C178 : LPUReg<178, "%c178">;
def C179 : LPUReg<179, "%c179">;
def C180 : LPUReg<180, "%c180">;
def C181 : LPUReg<181, "%c181">;
def C182 : LPUReg<182, "%c182">;
def C183 : LPUReg<183, "%c183">;
def C184 : LPUReg<184, "%c184">;
def C185 : LPUReg<185, "%c185">;
def C186 : LPUReg<186, "%c186">;
def C187 : LPUReg<187, "%c187">;
def C188 : LPUReg<188, "%c188">;
def C189 : LPUReg<189, "%c189">;
def C190 : LPUReg<190, "%c190">;
def C191 : LPUReg<191, "%c191">;
def C192 : LPUReg<192, "%c192">;
def C193 : LPUReg<193, "%c193">;
def C194 : LPUReg<194, "%c194">;
def C195 : LPUReg<195, "%c195">;
def C196 : LPUReg<196, "%c196">;
def C197 : LPUReg<197, "%c197">;
def C198 : LPUReg<198, "%c198">;
def C199 : LPUReg<199, "%c199">;
def C200 : LPUReg<200, "%c200">;
def C201 : LPUReg<201, "%c201">;
def C202 : LPUReg<202, "%c202">;
def C203 : LPUReg<203, "%c203">;
def C204 : LPUReg<204, "%c204">;
def C205 : LPUReg<205, "%c205">;
def C206 : LPUReg<206, "%c206">;
def C207 : LPUReg<207, "%c207">;
def C208 : LPUReg<208, "%c208">;
def C209 : LPUReg<209, "%c209">;
def C210 : LPUReg<210, "%c210">;
def C211 : LPUReg<211, "%c211">;
def C212 : LPUReg<212, "%c212">;
def C213 : LPUReg<213, "%c213">;
def C214 : LPUReg<214, "%c214">;
def C215 : LPUReg<215, "%c215">;
def C216 : LPUReg<216, "%c216">;
def C217 : LPUReg<217, "%c217">;
def C218 : LPUReg<218, "%c218">;
def C219 : LPUReg<219, "%c219">;
def C220 : LPUReg<220, "%c220">;
def C221 : LPUReg<221, "%c221">;
def C222 : LPUReg<222, "%c222">;
def C223 : LPUReg<223, "%c223">;
def C224 : LPUReg<224, "%c224">;
def C225 : LPUReg<225, "%c225">;
def C226 : LPUReg<226, "%c226">;
def C227 : LPUReg<227, "%c227">;
def C228 : LPUReg<228, "%c228">;
def C229 : LPUReg<229, "%c229">;
def C230 : LPUReg<230, "%c230">;
def C231 : LPUReg<231, "%c231">;
def C232 : LPUReg<232, "%c232">;
def C233 : LPUReg<233, "%c233">;
def C234 : LPUReg<234, "%c234">;
def C235 : LPUReg<235, "%c235">;
def C236 : LPUReg<236, "%c236">;
def C237 : LPUReg<237, "%c237">;
def C238 : LPUReg<238, "%c238">;
def C239 : LPUReg<239, "%c239">;
def C240 : LPUReg<240, "%c240">;
def C241 : LPUReg<241, "%c241">;
def C242 : LPUReg<242, "%c242">;
def C243 : LPUReg<243, "%c243">;
def C244 : LPUReg<244, "%c244">;
def C245 : LPUReg<245, "%c245">;
def C246 : LPUReg<246, "%c246">;
def C247 : LPUReg<247, "%c247">;
def C248 : LPUReg<248, "%c248">;
def C249 : LPUReg<249, "%c249">;
def C250 : LPUReg<250, "%c250">;
def C251 : LPUReg<251, "%c251">;
def C252 : LPUReg<252, "%c252">;
def C253 : LPUReg<253, "%c253">;
def C254 : LPUReg<254, "%c254">;
def C255 : LPUReg<255, "%c255">;
def C256 : LPUReg<256, "%c256">;
def C257 : LPUReg<257, "%c257">;
def C258 : LPUReg<258, "%c258">;
def C259 : LPUReg<259, "%c259">;
def C260 : LPUReg<260, "%c260">;
def C261 : LPUReg<261, "%c261">;
def C262 : LPUReg<262, "%c262">;
def C263 : LPUReg<263, "%c263">;
def C264 : LPUReg<264, "%c264">;
def C265 : LPUReg<265, "%c265">;
def C266 : LPUReg<266, "%c266">;
def C267 : LPUReg<267, "%c267">;
def C268 : LPUReg<268, "%c268">;
def C269 : LPUReg<269, "%c269">;
def C270 : LPUReg<270, "%c270">;
def C271 : LPUReg<271, "%c271">;
def C272 : LPUReg<272, "%c272">;
def C273 : LPUReg<273, "%c273">;
def C274 : LPUReg<274, "%c274">;
def C275 : LPUReg<275, "%c275">;
def C276 : LPUReg<276, "%c276">;
def C277 : LPUReg<277, "%c277">;
def C278 : LPUReg<278, "%c278">;
def C279 : LPUReg<279, "%c279">;
def C280 : LPUReg<280, "%c280">;
def C281 : LPUReg<281, "%c281">;
def C282 : LPUReg<282, "%c282">;
def C283 : LPUReg<283, "%c283">;
def C284 : LPUReg<284, "%c284">;
def C285 : LPUReg<285, "%c285">;
def C286 : LPUReg<286, "%c286">;
def C287 : LPUReg<287, "%c287">;
def C288 : LPUReg<288, "%c288">;
def C289 : LPUReg<289, "%c289">;
def C290 : LPUReg<290, "%c290">;
def C291 : LPUReg<291, "%c291">;
def C292 : LPUReg<292, "%c292">;
def C293 : LPUReg<293, "%c293">;
def C294 : LPUReg<294, "%c294">;
def C295 : LPUReg<295, "%c295">;
def C296 : LPUReg<296, "%c296">;
def C297 : LPUReg<297, "%c297">;
def C298 : LPUReg<298, "%c298">;
def C299 : LPUReg<299, "%c299">;
def C300 : LPUReg<300, "%c300">;
def C301 : LPUReg<301, "%c301">;
def C302 : LPUReg<302, "%c302">;
def C303 : LPUReg<303, "%c303">;
def C304 : LPUReg<304, "%c304">;
def C305 : LPUReg<305, "%c305">;
def C306 : LPUReg<306, "%c306">;
def C307 : LPUReg<307, "%c307">;
def C308 : LPUReg<308, "%c308">;
def C309 : LPUReg<309, "%c309">;
def C310 : LPUReg<310, "%c310">;
def C311 : LPUReg<311, "%c311">;
def C312 : LPUReg<312, "%c312">;
def C313 : LPUReg<313, "%c313">;
def C314 : LPUReg<314, "%c314">;
def C315 : LPUReg<315, "%c315">;
def C316 : LPUReg<316, "%c316">;
def C317 : LPUReg<317, "%c317">;
def C318 : LPUReg<318, "%c318">;
def C319 : LPUReg<319, "%c319">;
def C320 : LPUReg<320, "%c320">;
def C321 : LPUReg<321, "%c321">;
def C322 : LPUReg<322, "%c322">;
def C323 : LPUReg<323, "%c323">;
def C324 : LPUReg<324, "%c324">;
def C325 : LPUReg<325, "%c325">;
def C326 : LPUReg<326, "%c326">;
def C327 : LPUReg<327, "%c327">;
def C328 : LPUReg<328, "%c328">;
def C329 : LPUReg<329, "%c329">;
def C330 : LPUReg<330, "%c330">;
def C331 : LPUReg<331, "%c331">;
def C332 : LPUReg<332, "%c332">;
def C333 : LPUReg<333, "%c333">;
def C334 : LPUReg<334, "%c334">;
def C335 : LPUReg<335, "%c335">;
def C336 : LPUReg<336, "%c336">;
def C337 : LPUReg<337, "%c337">;
def C338 : LPUReg<338, "%c338">;
def C339 : LPUReg<339, "%c339">;
def C340 : LPUReg<340, "%c340">;
def C341 : LPUReg<341, "%c341">;
def C342 : LPUReg<342, "%c342">;
def C343 : LPUReg<343, "%c343">;
def C344 : LPUReg<344, "%c344">;
def C345 : LPUReg<345, "%c345">;
def C346 : LPUReg<346, "%c346">;
def C347 : LPUReg<347, "%c347">;
def C348 : LPUReg<348, "%c348">;
def C349 : LPUReg<349, "%c349">;
def C350 : LPUReg<350, "%c350">;
def C351 : LPUReg<351, "%c351">;
def C352 : LPUReg<352, "%c352">;
def C353 : LPUReg<353, "%c353">;
def C354 : LPUReg<354, "%c354">;
def C355 : LPUReg<355, "%c355">;
def C356 : LPUReg<356, "%c356">;
def C357 : LPUReg<357, "%c357">;
def C358 : LPUReg<358, "%c358">;
def C359 : LPUReg<359, "%c359">;
def C360 : LPUReg<360, "%c360">;
def C361 : LPUReg<361, "%c361">;
def C362 : LPUReg<362, "%c362">;
def C363 : LPUReg<363, "%c363">;
def C364 : LPUReg<364, "%c364">;
def C365 : LPUReg<365, "%c365">;
def C366 : LPUReg<366, "%c366">;
def C367 : LPUReg<367, "%c367">;
def C368 : LPUReg<368, "%c368">;
def C369 : LPUReg<369, "%c369">;
def C370 : LPUReg<370, "%c370">;
def C371 : LPUReg<371, "%c371">;
def C372 : LPUReg<372, "%c372">;
def C373 : LPUReg<373, "%c373">;
def C374 : LPUReg<374, "%c374">;
def C375 : LPUReg<375, "%c375">;
def C376 : LPUReg<376, "%c376">;
def C377 : LPUReg<377, "%c377">;
def C378 : LPUReg<378, "%c378">;
def C379 : LPUReg<379, "%c379">;
def C380 : LPUReg<380, "%c380">;
def C381 : LPUReg<381, "%c381">;
def C382 : LPUReg<382, "%c382">;
def C383 : LPUReg<383, "%c383">;
def C384 : LPUReg<384, "%c384">;
def C385 : LPUReg<385, "%c385">;
def C386 : LPUReg<386, "%c386">;
def C387 : LPUReg<387, "%c387">;
def C388 : LPUReg<388, "%c388">;
def C389 : LPUReg<389, "%c389">;
def C390 : LPUReg<390, "%c390">;
def C391 : LPUReg<391, "%c391">;
def C392 : LPUReg<392, "%c392">;
def C393 : LPUReg<393, "%c393">;
def C394 : LPUReg<394, "%c394">;
def C395 : LPUReg<395, "%c395">;
def C396 : LPUReg<396, "%c396">;
def C397 : LPUReg<397, "%c397">;
def C398 : LPUReg<398, "%c398">;
def C399 : LPUReg<399, "%c399">;
def C400 : LPUReg<400, "%c400">;
def C401 : LPUReg<401, "%c401">;
def C402 : LPUReg<402, "%c402">;
def C403 : LPUReg<403, "%c403">;
def C404 : LPUReg<404, "%c404">;
def C405 : LPUReg<405, "%c405">;
def C406 : LPUReg<406, "%c406">;
def C407 : LPUReg<407, "%c407">;
def C408 : LPUReg<408, "%c408">;
def C409 : LPUReg<409, "%c409">;
def C410 : LPUReg<410, "%c410">;
def C411 : LPUReg<411, "%c411">;
def C412 : LPUReg<412, "%c412">;
def C413 : LPUReg<413, "%c413">;
def C414 : LPUReg<414, "%c414">;
def C415 : LPUReg<415, "%c415">;
def C416 : LPUReg<416, "%c416">;
def C417 : LPUReg<417, "%c417">;
def C418 : LPUReg<418, "%c418">;
def C419 : LPUReg<419, "%c419">;
def C420 : LPUReg<420, "%c420">;
def C421 : LPUReg<421, "%c421">;
def C422 : LPUReg<422, "%c422">;
def C423 : LPUReg<423, "%c423">;
def C424 : LPUReg<424, "%c424">;
def C425 : LPUReg<425, "%c425">;
def C426 : LPUReg<426, "%c426">;
def C427 : LPUReg<427, "%c427">;
def C428 : LPUReg<428, "%c428">;
def C429 : LPUReg<429, "%c429">;
def C430 : LPUReg<430, "%c430">;
def C431 : LPUReg<431, "%c431">;
def C432 : LPUReg<432, "%c432">;
def C433 : LPUReg<433, "%c433">;
def C434 : LPUReg<434, "%c434">;
def C435 : LPUReg<435, "%c435">;
def C436 : LPUReg<436, "%c436">;
def C437 : LPUReg<437, "%c437">;
def C438 : LPUReg<438, "%c438">;
def C439 : LPUReg<439, "%c439">;
def C440 : LPUReg<440, "%c440">;
def C441 : LPUReg<441, "%c441">;
def C442 : LPUReg<442, "%c442">;
def C443 : LPUReg<443, "%c443">;
def C444 : LPUReg<444, "%c444">;
def C445 : LPUReg<445, "%c445">;
def C446 : LPUReg<446, "%c446">;
def C447 : LPUReg<447, "%c447">;
def C448 : LPUReg<448, "%c448">;
def C449 : LPUReg<449, "%c449">;
def C450 : LPUReg<450, "%c450">;
def C451 : LPUReg<451, "%c451">;
def C452 : LPUReg<452, "%c452">;
def C453 : LPUReg<453, "%c453">;
def C454 : LPUReg<454, "%c454">;
def C455 : LPUReg<455, "%c455">;
def C456 : LPUReg<456, "%c456">;
def C457 : LPUReg<457, "%c457">;
def C458 : LPUReg<458, "%c458">;
def C459 : LPUReg<459, "%c459">;
def C460 : LPUReg<460, "%c460">;
def C461 : LPUReg<461, "%c461">;
def C462 : LPUReg<462, "%c462">;
def C463 : LPUReg<463, "%c463">;
def C464 : LPUReg<464, "%c464">;
def C465 : LPUReg<465, "%c465">;
def C466 : LPUReg<466, "%c466">;
def C467 : LPUReg<467, "%c467">;
def C468 : LPUReg<468, "%c468">;
def C469 : LPUReg<469, "%c469">;
def C470 : LPUReg<470, "%c470">;
def C471 : LPUReg<471, "%c471">;
def C472 : LPUReg<472, "%c472">;
def C473 : LPUReg<473, "%c473">;
def C474 : LPUReg<474, "%c474">;
def C475 : LPUReg<475, "%c475">;
def C476 : LPUReg<476, "%c476">;
def C477 : LPUReg<477, "%c477">;
def C478 : LPUReg<478, "%c478">;
def C479 : LPUReg<479, "%c479">;
def C480 : LPUReg<480, "%c480">;
def C481 : LPUReg<481, "%c481">;
def C482 : LPUReg<482, "%c482">;
def C483 : LPUReg<483, "%c483">;
def C484 : LPUReg<484, "%c484">;
def C485 : LPUReg<485, "%c485">;
def C486 : LPUReg<486, "%c486">;
def C487 : LPUReg<487, "%c487">;
def C488 : LPUReg<488, "%c488">;
def C489 : LPUReg<489, "%c489">;
def C490 : LPUReg<490, "%c490">;
def C491 : LPUReg<491, "%c491">;
def C492 : LPUReg<492, "%c492">;
def C493 : LPUReg<493, "%c493">;
def C494 : LPUReg<494, "%c494">;
def C495 : LPUReg<495, "%c495">;
def C496 : LPUReg<496, "%c496">;
def C497 : LPUReg<497, "%c497">;
def C498 : LPUReg<498, "%c498">;
def C499 : LPUReg<499, "%c499">;
def C500 : LPUReg<500, "%c500">;
def C501 : LPUReg<501, "%c501">;
def C502 : LPUReg<502, "%c502">;
def C503 : LPUReg<503, "%c503">;
def C504 : LPUReg<504, "%c504">;
def C505 : LPUReg<505, "%c505">;
def C506 : LPUReg<506, "%c506">;
def C507 : LPUReg<507, "%c507">;
def C508 : LPUReg<508, "%c508">;
def C509 : LPUReg<509, "%c509">;
def C510 : LPUReg<510, "%c510">;
def C511 : LPUReg<511, "%c511">;
def C512 : LPUReg<512, "%c512">;
def C513 : LPUReg<513, "%c513">;
def C514 : LPUReg<514, "%c514">;
def C515 : LPUReg<515, "%c515">;
def C516 : LPUReg<516, "%c516">;
def C517 : LPUReg<517, "%c517">;
def C518 : LPUReg<518, "%c518">;
def C519 : LPUReg<519, "%c519">;
def C520 : LPUReg<520, "%c520">;
def C521 : LPUReg<521, "%c521">;
def C522 : LPUReg<522, "%c522">;
def C523 : LPUReg<523, "%c523">;
def C524 : LPUReg<524, "%c524">;
def C525 : LPUReg<525, "%c525">;
def C526 : LPUReg<526, "%c526">;
def C527 : LPUReg<527, "%c527">;
def C528 : LPUReg<528, "%c528">;
def C529 : LPUReg<529, "%c529">;
def C530 : LPUReg<530, "%c530">;
def C531 : LPUReg<531, "%c531">;
def C532 : LPUReg<532, "%c532">;
def C533 : LPUReg<533, "%c533">;
def C534 : LPUReg<534, "%c534">;
def C535 : LPUReg<535, "%c535">;
def C536 : LPUReg<536, "%c536">;
def C537 : LPUReg<537, "%c537">;
def C538 : LPUReg<538, "%c538">;
def C539 : LPUReg<539, "%c539">;
def C540 : LPUReg<540, "%c540">;
def C541 : LPUReg<541, "%c541">;
def C542 : LPUReg<542, "%c542">;
def C543 : LPUReg<543, "%c543">;
def C544 : LPUReg<544, "%c544">;
def C545 : LPUReg<545, "%c545">;
def C546 : LPUReg<546, "%c546">;
def C547 : LPUReg<547, "%c547">;
def C548 : LPUReg<548, "%c548">;
def C549 : LPUReg<549, "%c549">;
def C550 : LPUReg<550, "%c550">;
def C551 : LPUReg<551, "%c551">;
def C552 : LPUReg<552, "%c552">;
def C553 : LPUReg<553, "%c553">;
def C554 : LPUReg<554, "%c554">;
def C555 : LPUReg<555, "%c555">;
def C556 : LPUReg<556, "%c556">;
def C557 : LPUReg<557, "%c557">;
def C558 : LPUReg<558, "%c558">;
def C559 : LPUReg<559, "%c559">;
def C560 : LPUReg<560, "%c560">;
def C561 : LPUReg<561, "%c561">;
def C562 : LPUReg<562, "%c562">;
def C563 : LPUReg<563, "%c563">;
def C564 : LPUReg<564, "%c564">;
def C565 : LPUReg<565, "%c565">;
def C566 : LPUReg<566, "%c566">;
def C567 : LPUReg<567, "%c567">;
def C568 : LPUReg<568, "%c568">;
def C569 : LPUReg<569, "%c569">;
def C570 : LPUReg<570, "%c570">;
def C571 : LPUReg<571, "%c571">;
def C572 : LPUReg<572, "%c572">;
def C573 : LPUReg<573, "%c573">;
def C574 : LPUReg<574, "%c574">;
def C575 : LPUReg<575, "%c575">;
def C576 : LPUReg<576, "%c576">;
def C577 : LPUReg<577, "%c577">;
def C578 : LPUReg<578, "%c578">;
def C579 : LPUReg<579, "%c579">;
def C580 : LPUReg<580, "%c580">;
def C581 : LPUReg<581, "%c581">;
def C582 : LPUReg<582, "%c582">;
def C583 : LPUReg<583, "%c583">;
def C584 : LPUReg<584, "%c584">;
def C585 : LPUReg<585, "%c585">;
def C586 : LPUReg<586, "%c586">;
def C587 : LPUReg<587, "%c587">;
def C588 : LPUReg<588, "%c588">;
def C589 : LPUReg<589, "%c589">;
def C590 : LPUReg<590, "%c590">;
def C591 : LPUReg<591, "%c591">;
def C592 : LPUReg<592, "%c592">;
def C593 : LPUReg<593, "%c593">;
def C594 : LPUReg<594, "%c594">;
def C595 : LPUReg<595, "%c595">;
def C596 : LPUReg<596, "%c596">;
def C597 : LPUReg<597, "%c597">;
def C598 : LPUReg<598, "%c598">;
def C599 : LPUReg<599, "%c599">;
def C600 : LPUReg<600, "%c600">;
def C601 : LPUReg<601, "%c601">;
def C602 : LPUReg<602, "%c602">;
def C603 : LPUReg<603, "%c603">;
def C604 : LPUReg<604, "%c604">;
def C605 : LPUReg<605, "%c605">;
def C606 : LPUReg<606, "%c606">;
def C607 : LPUReg<607, "%c607">;
def C608 : LPUReg<608, "%c608">;
def C609 : LPUReg<609, "%c609">;
def C610 : LPUReg<610, "%c610">;
def C611 : LPUReg<611, "%c611">;
def C612 : LPUReg<612, "%c612">;
def C613 : LPUReg<613, "%c613">;
def C614 : LPUReg<614, "%c614">;
def C615 : LPUReg<615, "%c615">;
def C616 : LPUReg<616, "%c616">;
def C617 : LPUReg<617, "%c617">;
def C618 : LPUReg<618, "%c618">;
def C619 : LPUReg<619, "%c619">;
def C620 : LPUReg<620, "%c620">;
def C621 : LPUReg<621, "%c621">;
def C622 : LPUReg<622, "%c622">;
def C623 : LPUReg<623, "%c623">;
def C624 : LPUReg<624, "%c624">;
def C625 : LPUReg<625, "%c625">;
def C626 : LPUReg<626, "%c626">;
def C627 : LPUReg<627, "%c627">;
def C628 : LPUReg<628, "%c628">;
def C629 : LPUReg<629, "%c629">;
def C630 : LPUReg<630, "%c630">;
def C631 : LPUReg<631, "%c631">;
def C632 : LPUReg<632, "%c632">;
def C633 : LPUReg<633, "%c633">;
def C634 : LPUReg<634, "%c634">;
def C635 : LPUReg<635, "%c635">;
def C636 : LPUReg<636, "%c636">;
def C637 : LPUReg<637, "%c637">;
def C638 : LPUReg<638, "%c638">;
def C639 : LPUReg<639, "%c639">;
def C640 : LPUReg<640, "%c640">;
def C641 : LPUReg<641, "%c641">;
def C642 : LPUReg<642, "%c642">;
def C643 : LPUReg<643, "%c643">;
def C644 : LPUReg<644, "%c644">;
def C645 : LPUReg<645, "%c645">;
def C646 : LPUReg<646, "%c646">;
def C647 : LPUReg<647, "%c647">;
def C648 : LPUReg<648, "%c648">;
def C649 : LPUReg<649, "%c649">;
def C650 : LPUReg<650, "%c650">;
def C651 : LPUReg<651, "%c651">;
def C652 : LPUReg<652, "%c652">;
def C653 : LPUReg<653, "%c653">;
def C654 : LPUReg<654, "%c654">;
def C655 : LPUReg<655, "%c655">;
def C656 : LPUReg<656, "%c656">;
def C657 : LPUReg<657, "%c657">;
def C658 : LPUReg<658, "%c658">;
def C659 : LPUReg<659, "%c659">;
def C660 : LPUReg<660, "%c660">;
def C661 : LPUReg<661, "%c661">;
def C662 : LPUReg<662, "%c662">;
def C663 : LPUReg<663, "%c663">;
def C664 : LPUReg<664, "%c664">;
def C665 : LPUReg<665, "%c665">;
def C666 : LPUReg<666, "%c666">;
def C667 : LPUReg<667, "%c667">;
def C668 : LPUReg<668, "%c668">;
def C669 : LPUReg<669, "%c669">;
def C670 : LPUReg<670, "%c670">;
def C671 : LPUReg<671, "%c671">;
def C672 : LPUReg<672, "%c672">;
def C673 : LPUReg<673, "%c673">;
def C674 : LPUReg<674, "%c674">;
def C675 : LPUReg<675, "%c675">;
def C676 : LPUReg<676, "%c676">;
def C677 : LPUReg<677, "%c677">;
def C678 : LPUReg<678, "%c678">;
def C679 : LPUReg<679, "%c679">;
def C680 : LPUReg<680, "%c680">;
def C681 : LPUReg<681, "%c681">;
def C682 : LPUReg<682, "%c682">;
def C683 : LPUReg<683, "%c683">;
def C684 : LPUReg<684, "%c684">;
def C685 : LPUReg<685, "%c685">;
def C686 : LPUReg<686, "%c686">;
def C687 : LPUReg<687, "%c687">;
def C688 : LPUReg<688, "%c688">;
def C689 : LPUReg<689, "%c689">;
def C690 : LPUReg<690, "%c690">;
def C691 : LPUReg<691, "%c691">;
def C692 : LPUReg<692, "%c692">;
def C693 : LPUReg<693, "%c693">;
def C694 : LPUReg<694, "%c694">;
def C695 : LPUReg<695, "%c695">;
def C696 : LPUReg<696, "%c696">;
def C697 : LPUReg<697, "%c697">;
def C698 : LPUReg<698, "%c698">;
def C699 : LPUReg<699, "%c699">;
def C700 : LPUReg<700, "%c700">;
def C701 : LPUReg<701, "%c701">;
def C702 : LPUReg<702, "%c702">;
def C703 : LPUReg<703, "%c703">;
def C704 : LPUReg<704, "%c704">;
def C705 : LPUReg<705, "%c705">;
def C706 : LPUReg<706, "%c706">;
def C707 : LPUReg<707, "%c707">;
def C708 : LPUReg<708, "%c708">;
def C709 : LPUReg<709, "%c709">;
def C710 : LPUReg<710, "%c710">;
def C711 : LPUReg<711, "%c711">;
def C712 : LPUReg<712, "%c712">;
def C713 : LPUReg<713, "%c713">;
def C714 : LPUReg<714, "%c714">;
def C715 : LPUReg<715, "%c715">;
def C716 : LPUReg<716, "%c716">;
def C717 : LPUReg<717, "%c717">;
def C718 : LPUReg<718, "%c718">;
def C719 : LPUReg<719, "%c719">;
def C720 : LPUReg<720, "%c720">;
def C721 : LPUReg<721, "%c721">;
def C722 : LPUReg<722, "%c722">;
def C723 : LPUReg<723, "%c723">;
def C724 : LPUReg<724, "%c724">;
def C725 : LPUReg<725, "%c725">;
def C726 : LPUReg<726, "%c726">;
def C727 : LPUReg<727, "%c727">;
def C728 : LPUReg<728, "%c728">;
def C729 : LPUReg<729, "%c729">;
def C730 : LPUReg<730, "%c730">;
def C731 : LPUReg<731, "%c731">;
def C732 : LPUReg<732, "%c732">;
def C733 : LPUReg<733, "%c733">;
def C734 : LPUReg<734, "%c734">;
def C735 : LPUReg<735, "%c735">;
def C736 : LPUReg<736, "%c736">;
def C737 : LPUReg<737, "%c737">;
def C738 : LPUReg<738, "%c738">;
def C739 : LPUReg<739, "%c739">;
def C740 : LPUReg<740, "%c740">;
def C741 : LPUReg<741, "%c741">;
def C742 : LPUReg<742, "%c742">;
def C743 : LPUReg<743, "%c743">;
def C744 : LPUReg<744, "%c744">;
def C745 : LPUReg<745, "%c745">;
def C746 : LPUReg<746, "%c746">;
def C747 : LPUReg<747, "%c747">;
def C748 : LPUReg<748, "%c748">;
def C749 : LPUReg<749, "%c749">;
def C750 : LPUReg<750, "%c750">;
def C751 : LPUReg<751, "%c751">;
def C752 : LPUReg<752, "%c752">;
def C753 : LPUReg<753, "%c753">;
def C754 : LPUReg<754, "%c754">;
def C755 : LPUReg<755, "%c755">;
def C756 : LPUReg<756, "%c756">;
def C757 : LPUReg<757, "%c757">;
def C758 : LPUReg<758, "%c758">;
def C759 : LPUReg<759, "%c759">;
def C760 : LPUReg<760, "%c760">;
def C761 : LPUReg<761, "%c761">;
def C762 : LPUReg<762, "%c762">;
def C763 : LPUReg<763, "%c763">;
def C764 : LPUReg<764, "%c764">;
def C765 : LPUReg<765, "%c765">;
def C766 : LPUReg<766, "%c766">;
def C767 : LPUReg<767, "%c767">;
def C768 : LPUReg<768, "%c768">;
def C769 : LPUReg<769, "%c769">;
def C770 : LPUReg<770, "%c770">;
def C771 : LPUReg<771, "%c771">;
def C772 : LPUReg<772, "%c772">;
def C773 : LPUReg<773, "%c773">;
def C774 : LPUReg<774, "%c774">;
def C775 : LPUReg<775, "%c775">;
def C776 : LPUReg<776, "%c776">;
def C777 : LPUReg<777, "%c777">;
def C778 : LPUReg<778, "%c778">;
def C779 : LPUReg<779, "%c779">;
def C780 : LPUReg<780, "%c780">;
def C781 : LPUReg<781, "%c781">;
def C782 : LPUReg<782, "%c782">;
def C783 : LPUReg<783, "%c783">;
def C784 : LPUReg<784, "%c784">;
def C785 : LPUReg<785, "%c785">;
def C786 : LPUReg<786, "%c786">;
def C787 : LPUReg<787, "%c787">;
def C788 : LPUReg<788, "%c788">;
def C789 : LPUReg<789, "%c789">;
def C790 : LPUReg<790, "%c790">;
def C791 : LPUReg<791, "%c791">;
def C792 : LPUReg<792, "%c792">;
def C793 : LPUReg<793, "%c793">;
def C794 : LPUReg<794, "%c794">;
def C795 : LPUReg<795, "%c795">;
def C796 : LPUReg<796, "%c796">;
def C797 : LPUReg<797, "%c797">;
def C798 : LPUReg<798, "%c798">;
def C799 : LPUReg<799, "%c799">;
def C800 : LPUReg<800, "%c800">;
def C801 : LPUReg<801, "%c801">;
def C802 : LPUReg<802, "%c802">;
def C803 : LPUReg<803, "%c803">;
def C804 : LPUReg<804, "%c804">;
def C805 : LPUReg<805, "%c805">;
def C806 : LPUReg<806, "%c806">;
def C807 : LPUReg<807, "%c807">;
def C808 : LPUReg<808, "%c808">;
def C809 : LPUReg<809, "%c809">;
def C810 : LPUReg<810, "%c810">;
def C811 : LPUReg<811, "%c811">;
def C812 : LPUReg<812, "%c812">;
def C813 : LPUReg<813, "%c813">;
def C814 : LPUReg<814, "%c814">;
def C815 : LPUReg<815, "%c815">;
def C816 : LPUReg<816, "%c816">;
def C817 : LPUReg<817, "%c817">;
def C818 : LPUReg<818, "%c818">;
def C819 : LPUReg<819, "%c819">;
def C820 : LPUReg<820, "%c820">;
def C821 : LPUReg<821, "%c821">;
def C822 : LPUReg<822, "%c822">;
def C823 : LPUReg<823, "%c823">;
def C824 : LPUReg<824, "%c824">;
def C825 : LPUReg<825, "%c825">;
def C826 : LPUReg<826, "%c826">;
def C827 : LPUReg<827, "%c827">;
def C828 : LPUReg<828, "%c828">;
def C829 : LPUReg<829, "%c829">;
def C830 : LPUReg<830, "%c830">;
def C831 : LPUReg<831, "%c831">;
def C832 : LPUReg<832, "%c832">;
def C833 : LPUReg<833, "%c833">;
def C834 : LPUReg<834, "%c834">;
def C835 : LPUReg<835, "%c835">;
def C836 : LPUReg<836, "%c836">;
def C837 : LPUReg<837, "%c837">;
def C838 : LPUReg<838, "%c838">;
def C839 : LPUReg<839, "%c839">;
def C840 : LPUReg<840, "%c840">;
def C841 : LPUReg<841, "%c841">;
def C842 : LPUReg<842, "%c842">;
def C843 : LPUReg<843, "%c843">;
def C844 : LPUReg<844, "%c844">;
def C845 : LPUReg<845, "%c845">;
def C846 : LPUReg<846, "%c846">;
def C847 : LPUReg<847, "%c847">;
def C848 : LPUReg<848, "%c848">;
def C849 : LPUReg<849, "%c849">;
def C850 : LPUReg<850, "%c850">;
def C851 : LPUReg<851, "%c851">;
def C852 : LPUReg<852, "%c852">;
def C853 : LPUReg<853, "%c853">;
def C854 : LPUReg<854, "%c854">;
def C855 : LPUReg<855, "%c855">;
def C856 : LPUReg<856, "%c856">;
def C857 : LPUReg<857, "%c857">;
def C858 : LPUReg<858, "%c858">;
def C859 : LPUReg<859, "%c859">;
def C860 : LPUReg<860, "%c860">;
def C861 : LPUReg<861, "%c861">;
def C862 : LPUReg<862, "%c862">;
def C863 : LPUReg<863, "%c863">;
def C864 : LPUReg<864, "%c864">;
def C865 : LPUReg<865, "%c865">;
def C866 : LPUReg<866, "%c866">;
def C867 : LPUReg<867, "%c867">;
def C868 : LPUReg<868, "%c868">;
def C869 : LPUReg<869, "%c869">;
def C870 : LPUReg<870, "%c870">;
def C871 : LPUReg<871, "%c871">;
def C872 : LPUReg<872, "%c872">;
def C873 : LPUReg<873, "%c873">;
def C874 : LPUReg<874, "%c874">;
def C875 : LPUReg<875, "%c875">;
def C876 : LPUReg<876, "%c876">;
def C877 : LPUReg<877, "%c877">;
def C878 : LPUReg<878, "%c878">;
def C879 : LPUReg<879, "%c879">;
def C880 : LPUReg<880, "%c880">;
def C881 : LPUReg<881, "%c881">;
def C882 : LPUReg<882, "%c882">;
def C883 : LPUReg<883, "%c883">;
def C884 : LPUReg<884, "%c884">;
def C885 : LPUReg<885, "%c885">;
def C886 : LPUReg<886, "%c886">;
def C887 : LPUReg<887, "%c887">;
def C888 : LPUReg<888, "%c888">;
def C889 : LPUReg<889, "%c889">;
def C890 : LPUReg<890, "%c890">;
def C891 : LPUReg<891, "%c891">;
def C892 : LPUReg<892, "%c892">;
def C893 : LPUReg<893, "%c893">;
def C894 : LPUReg<894, "%c894">;
def C895 : LPUReg<895, "%c895">;
def C896 : LPUReg<896, "%c896">;
def C897 : LPUReg<897, "%c897">;
def C898 : LPUReg<898, "%c898">;
def C899 : LPUReg<899, "%c899">;
def C900 : LPUReg<900, "%c900">;
def C901 : LPUReg<901, "%c901">;
def C902 : LPUReg<902, "%c902">;
def C903 : LPUReg<903, "%c903">;
def C904 : LPUReg<904, "%c904">;
def C905 : LPUReg<905, "%c905">;
def C906 : LPUReg<906, "%c906">;
def C907 : LPUReg<907, "%c907">;
def C908 : LPUReg<908, "%c908">;
def C909 : LPUReg<909, "%c909">;
def C910 : LPUReg<910, "%c910">;
def C911 : LPUReg<911, "%c911">;
def C912 : LPUReg<912, "%c912">;
def C913 : LPUReg<913, "%c913">;
def C914 : LPUReg<914, "%c914">;
def C915 : LPUReg<915, "%c915">;
def C916 : LPUReg<916, "%c916">;
def C917 : LPUReg<917, "%c917">;
def C918 : LPUReg<918, "%c918">;
def C919 : LPUReg<919, "%c919">;
def C920 : LPUReg<920, "%c920">;
def C921 : LPUReg<921, "%c921">;
def C922 : LPUReg<922, "%c922">;
def C923 : LPUReg<923, "%c923">;
def C924 : LPUReg<924, "%c924">;
def C925 : LPUReg<925, "%c925">;
def C926 : LPUReg<926, "%c926">;
def C927 : LPUReg<927, "%c927">;
def C928 : LPUReg<928, "%c928">;
def C929 : LPUReg<929, "%c929">;
def C930 : LPUReg<930, "%c930">;
def C931 : LPUReg<931, "%c931">;
def C932 : LPUReg<932, "%c932">;
def C933 : LPUReg<933, "%c933">;
def C934 : LPUReg<934, "%c934">;
def C935 : LPUReg<935, "%c935">;
def C936 : LPUReg<936, "%c936">;
def C937 : LPUReg<937, "%c937">;
def C938 : LPUReg<938, "%c938">;
def C939 : LPUReg<939, "%c939">;
def C940 : LPUReg<940, "%c940">;
def C941 : LPUReg<941, "%c941">;
def C942 : LPUReg<942, "%c942">;
def C943 : LPUReg<943, "%c943">;
def C944 : LPUReg<944, "%c944">;
def C945 : LPUReg<945, "%c945">;
def C946 : LPUReg<946, "%c946">;
def C947 : LPUReg<947, "%c947">;
def C948 : LPUReg<948, "%c948">;
def C949 : LPUReg<949, "%c949">;
def C950 : LPUReg<950, "%c950">;
def C951 : LPUReg<951, "%c951">;
def C952 : LPUReg<952, "%c952">;
def C953 : LPUReg<953, "%c953">;
def C954 : LPUReg<954, "%c954">;
def C955 : LPUReg<955, "%c955">;
def C956 : LPUReg<956, "%c956">;
def C957 : LPUReg<957, "%c957">;
def C958 : LPUReg<958, "%c958">;
def C959 : LPUReg<959, "%c959">;
def C960 : LPUReg<960, "%c960">;
def C961 : LPUReg<961, "%c961">;
def C962 : LPUReg<962, "%c962">;
def C963 : LPUReg<963, "%c963">;
def C964 : LPUReg<964, "%c964">;
def C965 : LPUReg<965, "%c965">;
def C966 : LPUReg<966, "%c966">;
def C967 : LPUReg<967, "%c967">;
def C968 : LPUReg<968, "%c968">;
def C969 : LPUReg<969, "%c969">;
def C970 : LPUReg<970, "%c970">;
def C971 : LPUReg<971, "%c971">;
def C972 : LPUReg<972, "%c972">;
def C973 : LPUReg<973, "%c973">;
def C974 : LPUReg<974, "%c974">;
def C975 : LPUReg<975, "%c975">;
def C976 : LPUReg<976, "%c976">;
def C977 : LPUReg<977, "%c977">;
def C978 : LPUReg<978, "%c978">;
def C979 : LPUReg<979, "%c979">;
def C980 : LPUReg<980, "%c980">;
def C981 : LPUReg<981, "%c981">;
def C982 : LPUReg<982, "%c982">;
def C983 : LPUReg<983, "%c983">;
def C984 : LPUReg<984, "%c984">;
def C985 : LPUReg<985, "%c985">;
def C986 : LPUReg<986, "%c986">;
def C987 : LPUReg<987, "%c987">;
def C988 : LPUReg<988, "%c988">;
def C989 : LPUReg<989, "%c989">;
def C990 : LPUReg<990, "%c990">;
def C991 : LPUReg<991, "%c991">;
def C992 : LPUReg<992, "%c992">;
def C993 : LPUReg<993, "%c993">;
def C994 : LPUReg<994, "%c994">;
def C995 : LPUReg<995, "%c995">;
def C996 : LPUReg<996, "%c996">;
def C997 : LPUReg<997, "%c997">;
def C998 : LPUReg<998, "%c998">;
def C999 : LPUReg<999, "%c999">;
def C1000 : LPUReg<1000, "%c1000">;
def C1001 : LPUReg<1001, "%c1001">;
def C1002 : LPUReg<1002, "%c1002">;
def C1003 : LPUReg<1003, "%c1003">;
def C1004 : LPUReg<1004, "%c1004">;
def C1005 : LPUReg<1005, "%c1005">;
def C1006 : LPUReg<1006, "%c1006">;
def C1007 : LPUReg<1007, "%c1007">;
def C1008 : LPUReg<1008, "%c1008">;
def C1009 : LPUReg<1009, "%c1009">;
def C1010 : LPUReg<1010, "%c1010">;
def C1011 : LPUReg<1011, "%c1011">;
def C1012 : LPUReg<1012, "%c1012">;
def C1013 : LPUReg<1013, "%c1013">;
def C1014 : LPUReg<1014, "%c1014">;
def C1015 : LPUReg<1015, "%c1015">;
def C1016 : LPUReg<1016, "%c1016">;
def C1017 : LPUReg<1017, "%c1017">;
def C1018 : LPUReg<1018, "%c1018">;
def C1019 : LPUReg<1019, "%c1019">;
def C1020 : LPUReg<1020, "%c1020">;
def C1021 : LPUReg<1021, "%c1021">;
def C1022 : LPUReg<1022, "%c1022">;
def C1023 : LPUReg<1023, "%c1023">;
def C1024 : LPUReg<1024, "%c1024">;
def C1025 : LPUReg<1025, "%c1025">;
def C1026 : LPUReg<1026, "%c1026">;
def C1027 : LPUReg<1027, "%c1027">;
def C1028 : LPUReg<1028, "%c1028">;
def C1029 : LPUReg<1029, "%c1029">;
def C1030 : LPUReg<1030, "%c1030">;
def C1031 : LPUReg<1031, "%c1031">;
def C1032 : LPUReg<1032, "%c1032">;
def C1033 : LPUReg<1033, "%c1033">;
def C1034 : LPUReg<1034, "%c1034">;
def C1035 : LPUReg<1035, "%c1035">;
def C1036 : LPUReg<1036, "%c1036">;
def C1037 : LPUReg<1037, "%c1037">;
def C1038 : LPUReg<1038, "%c1038">;
def C1039 : LPUReg<1039, "%c1039">;
def C1040 : LPUReg<1040, "%c1040">;
def C1041 : LPUReg<1041, "%c1041">;
def C1042 : LPUReg<1042, "%c1042">;
def C1043 : LPUReg<1043, "%c1043">;
def C1044 : LPUReg<1044, "%c1044">;
def C1045 : LPUReg<1045, "%c1045">;
def C1046 : LPUReg<1046, "%c1046">;
def C1047 : LPUReg<1047, "%c1047">;
def C1048 : LPUReg<1048, "%c1048">;
def C1049 : LPUReg<1049, "%c1049">;
def C1050 : LPUReg<1050, "%c1050">;
def C1051 : LPUReg<1051, "%c1051">;
def C1052 : LPUReg<1052, "%c1052">;
def C1053 : LPUReg<1053, "%c1053">;
def C1054 : LPUReg<1054, "%c1054">;
def C1055 : LPUReg<1055, "%c1055">;
def C1056 : LPUReg<1056, "%c1056">;
def C1057 : LPUReg<1057, "%c1057">;
def C1058 : LPUReg<1058, "%c1058">;
def C1059 : LPUReg<1059, "%c1059">;
def C1060 : LPUReg<1060, "%c1060">;
def C1061 : LPUReg<1061, "%c1061">;
def C1062 : LPUReg<1062, "%c1062">;
def C1063 : LPUReg<1063, "%c1063">;
def C1064 : LPUReg<1064, "%c1064">;
def C1065 : LPUReg<1065, "%c1065">;
def C1066 : LPUReg<1066, "%c1066">;
def C1067 : LPUReg<1067, "%c1067">;
def C1068 : LPUReg<1068, "%c1068">;
def C1069 : LPUReg<1069, "%c1069">;
def C1070 : LPUReg<1070, "%c1070">;
def C1071 : LPUReg<1071, "%c1071">;
def C1072 : LPUReg<1072, "%c1072">;
def C1073 : LPUReg<1073, "%c1073">;
def C1074 : LPUReg<1074, "%c1074">;
def C1075 : LPUReg<1075, "%c1075">;
def C1076 : LPUReg<1076, "%c1076">;
def C1077 : LPUReg<1077, "%c1077">;
def C1078 : LPUReg<1078, "%c1078">;
def C1079 : LPUReg<1079, "%c1079">;
def C1080 : LPUReg<1080, "%c1080">;
def C1081 : LPUReg<1081, "%c1081">;
def C1082 : LPUReg<1082, "%c1082">;
def C1083 : LPUReg<1083, "%c1083">;
def C1084 : LPUReg<1084, "%c1084">;
def C1085 : LPUReg<1085, "%c1085">;
def C1086 : LPUReg<1086, "%c1086">;
def C1087 : LPUReg<1087, "%c1087">;
def C1088 : LPUReg<1088, "%c1088">;
def C1089 : LPUReg<1089, "%c1089">;
def C1090 : LPUReg<1090, "%c1090">;
def C1091 : LPUReg<1091, "%c1091">;
def C1092 : LPUReg<1092, "%c1092">;
def C1093 : LPUReg<1093, "%c1093">;
def C1094 : LPUReg<1094, "%c1094">;
def C1095 : LPUReg<1095, "%c1095">;
def C1096 : LPUReg<1096, "%c1096">;
def C1097 : LPUReg<1097, "%c1097">;
def C1098 : LPUReg<1098, "%c1098">;
def C1099 : LPUReg<1099, "%c1099">;
def C1100 : LPUReg<1100, "%c1100">;
def C1101 : LPUReg<1101, "%c1101">;
def C1102 : LPUReg<1102, "%c1102">;
def C1103 : LPUReg<1103, "%c1103">;
def C1104 : LPUReg<1104, "%c1104">;
def C1105 : LPUReg<1105, "%c1105">;
def C1106 : LPUReg<1106, "%c1106">;
def C1107 : LPUReg<1107, "%c1107">;
def C1108 : LPUReg<1108, "%c1108">;
def C1109 : LPUReg<1109, "%c1109">;
def C1110 : LPUReg<1110, "%c1110">;
def C1111 : LPUReg<1111, "%c1111">;
def C1112 : LPUReg<1112, "%c1112">;
def C1113 : LPUReg<1113, "%c1113">;
def C1114 : LPUReg<1114, "%c1114">;
def C1115 : LPUReg<1115, "%c1115">;
def C1116 : LPUReg<1116, "%c1116">;
def C1117 : LPUReg<1117, "%c1117">;
def C1118 : LPUReg<1118, "%c1118">;
def C1119 : LPUReg<1119, "%c1119">;
def C1120 : LPUReg<1120, "%c1120">;
def C1121 : LPUReg<1121, "%c1121">;
def C1122 : LPUReg<1122, "%c1122">;
def C1123 : LPUReg<1123, "%c1123">;
def C1124 : LPUReg<1124, "%c1124">;
def C1125 : LPUReg<1125, "%c1125">;
def C1126 : LPUReg<1126, "%c1126">;
def C1127 : LPUReg<1127, "%c1127">;
def C1128 : LPUReg<1128, "%c1128">;
def C1129 : LPUReg<1129, "%c1129">;
def C1130 : LPUReg<1130, "%c1130">;
def C1131 : LPUReg<1131, "%c1131">;
def C1132 : LPUReg<1132, "%c1132">;
def C1133 : LPUReg<1133, "%c1133">;
def C1134 : LPUReg<1134, "%c1134">;
def C1135 : LPUReg<1135, "%c1135">;
def C1136 : LPUReg<1136, "%c1136">;
def C1137 : LPUReg<1137, "%c1137">;
def C1138 : LPUReg<1138, "%c1138">;
def C1139 : LPUReg<1139, "%c1139">;
def C1140 : LPUReg<1140, "%c1140">;
def C1141 : LPUReg<1141, "%c1141">;
def C1142 : LPUReg<1142, "%c1142">;
def C1143 : LPUReg<1143, "%c1143">;
def C1144 : LPUReg<1144, "%c1144">;
def C1145 : LPUReg<1145, "%c1145">;
def C1146 : LPUReg<1146, "%c1146">;
def C1147 : LPUReg<1147, "%c1147">;
def C1148 : LPUReg<1148, "%c1148">;
def C1149 : LPUReg<1149, "%c1149">;
def C1150 : LPUReg<1150, "%c1150">;
def C1151 : LPUReg<1151, "%c1151">;
def C1152 : LPUReg<1152, "%c1152">;
def C1153 : LPUReg<1153, "%c1153">;
def C1154 : LPUReg<1154, "%c1154">;
def C1155 : LPUReg<1155, "%c1155">;
def C1156 : LPUReg<1156, "%c1156">;
def C1157 : LPUReg<1157, "%c1157">;
def C1158 : LPUReg<1158, "%c1158">;
def C1159 : LPUReg<1159, "%c1159">;
def C1160 : LPUReg<1160, "%c1160">;
def C1161 : LPUReg<1161, "%c1161">;
def C1162 : LPUReg<1162, "%c1162">;
def C1163 : LPUReg<1163, "%c1163">;
def C1164 : LPUReg<1164, "%c1164">;
def C1165 : LPUReg<1165, "%c1165">;
def C1166 : LPUReg<1166, "%c1166">;
def C1167 : LPUReg<1167, "%c1167">;
def C1168 : LPUReg<1168, "%c1168">;
def C1169 : LPUReg<1169, "%c1169">;
def C1170 : LPUReg<1170, "%c1170">;
def C1171 : LPUReg<1171, "%c1171">;
def C1172 : LPUReg<1172, "%c1172">;
def C1173 : LPUReg<1173, "%c1173">;
def C1174 : LPUReg<1174, "%c1174">;
def C1175 : LPUReg<1175, "%c1175">;
def C1176 : LPUReg<1176, "%c1176">;
def C1177 : LPUReg<1177, "%c1177">;
def C1178 : LPUReg<1178, "%c1178">;
def C1179 : LPUReg<1179, "%c1179">;
def C1180 : LPUReg<1180, "%c1180">;
def C1181 : LPUReg<1181, "%c1181">;
def C1182 : LPUReg<1182, "%c1182">;
def C1183 : LPUReg<1183, "%c1183">;
def C1184 : LPUReg<1184, "%c1184">;
def C1185 : LPUReg<1185, "%c1185">;
def C1186 : LPUReg<1186, "%c1186">;
def C1187 : LPUReg<1187, "%c1187">;
def C1188 : LPUReg<1188, "%c1188">;
def C1189 : LPUReg<1189, "%c1189">;
def C1190 : LPUReg<1190, "%c1190">;
def C1191 : LPUReg<1191, "%c1191">;
def C1192 : LPUReg<1192, "%c1192">;
def C1193 : LPUReg<1193, "%c1193">;
def C1194 : LPUReg<1194, "%c1194">;
def C1195 : LPUReg<1195, "%c1195">;
def C1196 : LPUReg<1196, "%c1196">;
def C1197 : LPUReg<1197, "%c1197">;
def C1198 : LPUReg<1198, "%c1198">;
def C1199 : LPUReg<1199, "%c1199">;
def C1200 : LPUReg<1200, "%c1200">;
def C1201 : LPUReg<1201, "%c1201">;
def C1202 : LPUReg<1202, "%c1202">;
def C1203 : LPUReg<1203, "%c1203">;
def C1204 : LPUReg<1204, "%c1204">;
def C1205 : LPUReg<1205, "%c1205">;
def C1206 : LPUReg<1206, "%c1206">;
def C1207 : LPUReg<1207, "%c1207">;
def C1208 : LPUReg<1208, "%c1208">;
def C1209 : LPUReg<1209, "%c1209">;
def C1210 : LPUReg<1210, "%c1210">;
def C1211 : LPUReg<1211, "%c1211">;
def C1212 : LPUReg<1212, "%c1212">;
def C1213 : LPUReg<1213, "%c1213">;
def C1214 : LPUReg<1214, "%c1214">;
def C1215 : LPUReg<1215, "%c1215">;
def C1216 : LPUReg<1216, "%c1216">;
def C1217 : LPUReg<1217, "%c1217">;
def C1218 : LPUReg<1218, "%c1218">;
def C1219 : LPUReg<1219, "%c1219">;
def C1220 : LPUReg<1220, "%c1220">;
def C1221 : LPUReg<1221, "%c1221">;
def C1222 : LPUReg<1222, "%c1222">;
def C1223 : LPUReg<1223, "%c1223">;
def C1224 : LPUReg<1224, "%c1224">;
def C1225 : LPUReg<1225, "%c1225">;
def C1226 : LPUReg<1226, "%c1226">;
def C1227 : LPUReg<1227, "%c1227">;
def C1228 : LPUReg<1228, "%c1228">;
def C1229 : LPUReg<1229, "%c1229">;
def C1230 : LPUReg<1230, "%c1230">;
def C1231 : LPUReg<1231, "%c1231">;
def C1232 : LPUReg<1232, "%c1232">;
def C1233 : LPUReg<1233, "%c1233">;
def C1234 : LPUReg<1234, "%c1234">;
def C1235 : LPUReg<1235, "%c1235">;
def C1236 : LPUReg<1236, "%c1236">;
def C1237 : LPUReg<1237, "%c1237">;
def C1238 : LPUReg<1238, "%c1238">;
def C1239 : LPUReg<1239, "%c1239">;
def C1240 : LPUReg<1240, "%c1240">;
def C1241 : LPUReg<1241, "%c1241">;
def C1242 : LPUReg<1242, "%c1242">;
def C1243 : LPUReg<1243, "%c1243">;
def C1244 : LPUReg<1244, "%c1244">;
def C1245 : LPUReg<1245, "%c1245">;
def C1246 : LPUReg<1246, "%c1246">;
def C1247 : LPUReg<1247, "%c1247">;
def C1248 : LPUReg<1248, "%c1248">;
def C1249 : LPUReg<1249, "%c1249">;
def C1250 : LPUReg<1250, "%c1250">;
def C1251 : LPUReg<1251, "%c1251">;
def C1252 : LPUReg<1252, "%c1252">;
def C1253 : LPUReg<1253, "%c1253">;
def C1254 : LPUReg<1254, "%c1254">;
def C1255 : LPUReg<1255, "%c1255">;
def C1256 : LPUReg<1256, "%c1256">;
def C1257 : LPUReg<1257, "%c1257">;
def C1258 : LPUReg<1258, "%c1258">;
def C1259 : LPUReg<1259, "%c1259">;
def C1260 : LPUReg<1260, "%c1260">;
def C1261 : LPUReg<1261, "%c1261">;
def C1262 : LPUReg<1262, "%c1262">;
def C1263 : LPUReg<1263, "%c1263">;
def C1264 : LPUReg<1264, "%c1264">;
def C1265 : LPUReg<1265, "%c1265">;
def C1266 : LPUReg<1266, "%c1266">;
def C1267 : LPUReg<1267, "%c1267">;
def C1268 : LPUReg<1268, "%c1268">;
def C1269 : LPUReg<1269, "%c1269">;
def C1270 : LPUReg<1270, "%c1270">;
def C1271 : LPUReg<1271, "%c1271">;
def C1272 : LPUReg<1272, "%c1272">;
def C1273 : LPUReg<1273, "%c1273">;
def C1274 : LPUReg<1274, "%c1274">;
def C1275 : LPUReg<1275, "%c1275">;
def C1276 : LPUReg<1276, "%c1276">;
def C1277 : LPUReg<1277, "%c1277">;
def C1278 : LPUReg<1278, "%c1278">;
def C1279 : LPUReg<1279, "%c1279">;
def C1280 : LPUReg<1280, "%c1280">;
def C1281 : LPUReg<1281, "%c1281">;
def C1282 : LPUReg<1282, "%c1282">;
def C1283 : LPUReg<1283, "%c1283">;
def C1284 : LPUReg<1284, "%c1284">;
def C1285 : LPUReg<1285, "%c1285">;
def C1286 : LPUReg<1286, "%c1286">;
def C1287 : LPUReg<1287, "%c1287">;
def C1288 : LPUReg<1288, "%c1288">;
def C1289 : LPUReg<1289, "%c1289">;
def C1290 : LPUReg<1290, "%c1290">;
def C1291 : LPUReg<1291, "%c1291">;
def C1292 : LPUReg<1292, "%c1292">;
def C1293 : LPUReg<1293, "%c1293">;
def C1294 : LPUReg<1294, "%c1294">;
def C1295 : LPUReg<1295, "%c1295">;
def C1296 : LPUReg<1296, "%c1296">;
def C1297 : LPUReg<1297, "%c1297">;
def C1298 : LPUReg<1298, "%c1298">;
def C1299 : LPUReg<1299, "%c1299">;
def C1300 : LPUReg<1300, "%c1300">;
def C1301 : LPUReg<1301, "%c1301">;
def C1302 : LPUReg<1302, "%c1302">;
def C1303 : LPUReg<1303, "%c1303">;
def C1304 : LPUReg<1304, "%c1304">;
def C1305 : LPUReg<1305, "%c1305">;
def C1306 : LPUReg<1306, "%c1306">;
def C1307 : LPUReg<1307, "%c1307">;
def C1308 : LPUReg<1308, "%c1308">;
def C1309 : LPUReg<1309, "%c1309">;
def C1310 : LPUReg<1310, "%c1310">;
def C1311 : LPUReg<1311, "%c1311">;
def C1312 : LPUReg<1312, "%c1312">;
def C1313 : LPUReg<1313, "%c1313">;
def C1314 : LPUReg<1314, "%c1314">;
def C1315 : LPUReg<1315, "%c1315">;
def C1316 : LPUReg<1316, "%c1316">;
def C1317 : LPUReg<1317, "%c1317">;
def C1318 : LPUReg<1318, "%c1318">;
def C1319 : LPUReg<1319, "%c1319">;
def C1320 : LPUReg<1320, "%c1320">;
def C1321 : LPUReg<1321, "%c1321">;
def C1322 : LPUReg<1322, "%c1322">;
def C1323 : LPUReg<1323, "%c1323">;
def C1324 : LPUReg<1324, "%c1324">;
def C1325 : LPUReg<1325, "%c1325">;
def C1326 : LPUReg<1326, "%c1326">;
def C1327 : LPUReg<1327, "%c1327">;
def C1328 : LPUReg<1328, "%c1328">;
def C1329 : LPUReg<1329, "%c1329">;
def C1330 : LPUReg<1330, "%c1330">;
def C1331 : LPUReg<1331, "%c1331">;
def C1332 : LPUReg<1332, "%c1332">;
def C1333 : LPUReg<1333, "%c1333">;
def C1334 : LPUReg<1334, "%c1334">;
def C1335 : LPUReg<1335, "%c1335">;
def C1336 : LPUReg<1336, "%c1336">;
def C1337 : LPUReg<1337, "%c1337">;
def C1338 : LPUReg<1338, "%c1338">;
def C1339 : LPUReg<1339, "%c1339">;
def C1340 : LPUReg<1340, "%c1340">;
def C1341 : LPUReg<1341, "%c1341">;
def C1342 : LPUReg<1342, "%c1342">;
def C1343 : LPUReg<1343, "%c1343">;
def C1344 : LPUReg<1344, "%c1344">;
def C1345 : LPUReg<1345, "%c1345">;
def C1346 : LPUReg<1346, "%c1346">;
def C1347 : LPUReg<1347, "%c1347">;
def C1348 : LPUReg<1348, "%c1348">;
def C1349 : LPUReg<1349, "%c1349">;
def C1350 : LPUReg<1350, "%c1350">;
def C1351 : LPUReg<1351, "%c1351">;
def C1352 : LPUReg<1352, "%c1352">;
def C1353 : LPUReg<1353, "%c1353">;
def C1354 : LPUReg<1354, "%c1354">;
def C1355 : LPUReg<1355, "%c1355">;
def C1356 : LPUReg<1356, "%c1356">;
def C1357 : LPUReg<1357, "%c1357">;
def C1358 : LPUReg<1358, "%c1358">;
def C1359 : LPUReg<1359, "%c1359">;
def C1360 : LPUReg<1360, "%c1360">;
def C1361 : LPUReg<1361, "%c1361">;
def C1362 : LPUReg<1362, "%c1362">;
def C1363 : LPUReg<1363, "%c1363">;
def C1364 : LPUReg<1364, "%c1364">;
def C1365 : LPUReg<1365, "%c1365">;
def C1366 : LPUReg<1366, "%c1366">;
def C1367 : LPUReg<1367, "%c1367">;
def C1368 : LPUReg<1368, "%c1368">;
def C1369 : LPUReg<1369, "%c1369">;
def C1370 : LPUReg<1370, "%c1370">;
def C1371 : LPUReg<1371, "%c1371">;
def C1372 : LPUReg<1372, "%c1372">;
def C1373 : LPUReg<1373, "%c1373">;
def C1374 : LPUReg<1374, "%c1374">;
def C1375 : LPUReg<1375, "%c1375">;
def C1376 : LPUReg<1376, "%c1376">;
def C1377 : LPUReg<1377, "%c1377">;
def C1378 : LPUReg<1378, "%c1378">;
def C1379 : LPUReg<1379, "%c1379">;
def C1380 : LPUReg<1380, "%c1380">;
def C1381 : LPUReg<1381, "%c1381">;
def C1382 : LPUReg<1382, "%c1382">;
def C1383 : LPUReg<1383, "%c1383">;
def C1384 : LPUReg<1384, "%c1384">;
def C1385 : LPUReg<1385, "%c1385">;
def C1386 : LPUReg<1386, "%c1386">;
def C1387 : LPUReg<1387, "%c1387">;
def C1388 : LPUReg<1388, "%c1388">;
def C1389 : LPUReg<1389, "%c1389">;
def C1390 : LPUReg<1390, "%c1390">;
def C1391 : LPUReg<1391, "%c1391">;
def C1392 : LPUReg<1392, "%c1392">;
def C1393 : LPUReg<1393, "%c1393">;
def C1394 : LPUReg<1394, "%c1394">;
def C1395 : LPUReg<1395, "%c1395">;
def C1396 : LPUReg<1396, "%c1396">;
def C1397 : LPUReg<1397, "%c1397">;
def C1398 : LPUReg<1398, "%c1398">;
def C1399 : LPUReg<1399, "%c1399">;
def C1400 : LPUReg<1400, "%c1400">;
def C1401 : LPUReg<1401, "%c1401">;
def C1402 : LPUReg<1402, "%c1402">;
def C1403 : LPUReg<1403, "%c1403">;
def C1404 : LPUReg<1404, "%c1404">;
def C1405 : LPUReg<1405, "%c1405">;
def C1406 : LPUReg<1406, "%c1406">;
def C1407 : LPUReg<1407, "%c1407">;
def C1408 : LPUReg<1408, "%c1408">;
def C1409 : LPUReg<1409, "%c1409">;
def C1410 : LPUReg<1410, "%c1410">;
def C1411 : LPUReg<1411, "%c1411">;
def C1412 : LPUReg<1412, "%c1412">;
def C1413 : LPUReg<1413, "%c1413">;
def C1414 : LPUReg<1414, "%c1414">;
def C1415 : LPUReg<1415, "%c1415">;
def C1416 : LPUReg<1416, "%c1416">;
def C1417 : LPUReg<1417, "%c1417">;
def C1418 : LPUReg<1418, "%c1418">;
def C1419 : LPUReg<1419, "%c1419">;
def C1420 : LPUReg<1420, "%c1420">;
def C1421 : LPUReg<1421, "%c1421">;
def C1422 : LPUReg<1422, "%c1422">;
def C1423 : LPUReg<1423, "%c1423">;
def C1424 : LPUReg<1424, "%c1424">;
def C1425 : LPUReg<1425, "%c1425">;
def C1426 : LPUReg<1426, "%c1426">;
def C1427 : LPUReg<1427, "%c1427">;
def C1428 : LPUReg<1428, "%c1428">;
def C1429 : LPUReg<1429, "%c1429">;
def C1430 : LPUReg<1430, "%c1430">;
def C1431 : LPUReg<1431, "%c1431">;
def C1432 : LPUReg<1432, "%c1432">;
def C1433 : LPUReg<1433, "%c1433">;
def C1434 : LPUReg<1434, "%c1434">;
def C1435 : LPUReg<1435, "%c1435">;
def C1436 : LPUReg<1436, "%c1436">;
def C1437 : LPUReg<1437, "%c1437">;
def C1438 : LPUReg<1438, "%c1438">;
def C1439 : LPUReg<1439, "%c1439">;
def C1440 : LPUReg<1440, "%c1440">;
def C1441 : LPUReg<1441, "%c1441">;
def C1442 : LPUReg<1442, "%c1442">;
def C1443 : LPUReg<1443, "%c1443">;
def C1444 : LPUReg<1444, "%c1444">;
def C1445 : LPUReg<1445, "%c1445">;
def C1446 : LPUReg<1446, "%c1446">;
def C1447 : LPUReg<1447, "%c1447">;
def C1448 : LPUReg<1448, "%c1448">;
def C1449 : LPUReg<1449, "%c1449">;
def C1450 : LPUReg<1450, "%c1450">;
def C1451 : LPUReg<1451, "%c1451">;
def C1452 : LPUReg<1452, "%c1452">;
def C1453 : LPUReg<1453, "%c1453">;
def C1454 : LPUReg<1454, "%c1454">;
def C1455 : LPUReg<1455, "%c1455">;
def C1456 : LPUReg<1456, "%c1456">;
def C1457 : LPUReg<1457, "%c1457">;
def C1458 : LPUReg<1458, "%c1458">;
def C1459 : LPUReg<1459, "%c1459">;
def C1460 : LPUReg<1460, "%c1460">;
def C1461 : LPUReg<1461, "%c1461">;
def C1462 : LPUReg<1462, "%c1462">;
def C1463 : LPUReg<1463, "%c1463">;
def C1464 : LPUReg<1464, "%c1464">;
def C1465 : LPUReg<1465, "%c1465">;
def C1466 : LPUReg<1466, "%c1466">;
def C1467 : LPUReg<1467, "%c1467">;
def C1468 : LPUReg<1468, "%c1468">;
def C1469 : LPUReg<1469, "%c1469">;
def C1470 : LPUReg<1470, "%c1470">;
def C1471 : LPUReg<1471, "%c1471">;
def C1472 : LPUReg<1472, "%c1472">;
def C1473 : LPUReg<1473, "%c1473">;
def C1474 : LPUReg<1474, "%c1474">;
def C1475 : LPUReg<1475, "%c1475">;
def C1476 : LPUReg<1476, "%c1476">;
def C1477 : LPUReg<1477, "%c1477">;
def C1478 : LPUReg<1478, "%c1478">;
def C1479 : LPUReg<1479, "%c1479">;
def C1480 : LPUReg<1480, "%c1480">;
def C1481 : LPUReg<1481, "%c1481">;
def C1482 : LPUReg<1482, "%c1482">;
def C1483 : LPUReg<1483, "%c1483">;
def C1484 : LPUReg<1484, "%c1484">;
def C1485 : LPUReg<1485, "%c1485">;
def C1486 : LPUReg<1486, "%c1486">;
def C1487 : LPUReg<1487, "%c1487">;
def C1488 : LPUReg<1488, "%c1488">;
def C1489 : LPUReg<1489, "%c1489">;
def C1490 : LPUReg<1490, "%c1490">;
def C1491 : LPUReg<1491, "%c1491">;
def C1492 : LPUReg<1492, "%c1492">;
def C1493 : LPUReg<1493, "%c1493">;
def C1494 : LPUReg<1494, "%c1494">;
def C1495 : LPUReg<1495, "%c1495">;
def C1496 : LPUReg<1496, "%c1496">;
def C1497 : LPUReg<1497, "%c1497">;
def C1498 : LPUReg<1498, "%c1498">;
def C1499 : LPUReg<1499, "%c1499">;
def C1500 : LPUReg<1500, "%c1500">;
def C1501 : LPUReg<1501, "%c1501">;
def C1502 : LPUReg<1502, "%c1502">;
def C1503 : LPUReg<1503, "%c1503">;
def C1504 : LPUReg<1504, "%c1504">;
def C1505 : LPUReg<1505, "%c1505">;
def C1506 : LPUReg<1506, "%c1506">;
def C1507 : LPUReg<1507, "%c1507">;
def C1508 : LPUReg<1508, "%c1508">;
def C1509 : LPUReg<1509, "%c1509">;
def C1510 : LPUReg<1510, "%c1510">;
def C1511 : LPUReg<1511, "%c1511">;
def C1512 : LPUReg<1512, "%c1512">;
def C1513 : LPUReg<1513, "%c1513">;
def C1514 : LPUReg<1514, "%c1514">;
def C1515 : LPUReg<1515, "%c1515">;
def C1516 : LPUReg<1516, "%c1516">;
def C1517 : LPUReg<1517, "%c1517">;
def C1518 : LPUReg<1518, "%c1518">;
def C1519 : LPUReg<1519, "%c1519">;
def C1520 : LPUReg<1520, "%c1520">;
def C1521 : LPUReg<1521, "%c1521">;
def C1522 : LPUReg<1522, "%c1522">;
def C1523 : LPUReg<1523, "%c1523">;
def C1524 : LPUReg<1524, "%c1524">;
def C1525 : LPUReg<1525, "%c1525">;
def C1526 : LPUReg<1526, "%c1526">;
def C1527 : LPUReg<1527, "%c1527">;
def C1528 : LPUReg<1528, "%c1528">;
def C1529 : LPUReg<1529, "%c1529">;
def C1530 : LPUReg<1530, "%c1530">;
def C1531 : LPUReg<1531, "%c1531">;
def C1532 : LPUReg<1532, "%c1532">;
def C1533 : LPUReg<1533, "%c1533">;
def C1534 : LPUReg<1534, "%c1534">;
def C1535 : LPUReg<1535, "%c1535">;
def C1536 : LPUReg<1536, "%c1536">;
def C1537 : LPUReg<1537, "%c1537">;
def C1538 : LPUReg<1538, "%c1538">;
def C1539 : LPUReg<1539, "%c1539">;
def C1540 : LPUReg<1540, "%c1540">;
def C1541 : LPUReg<1541, "%c1541">;
def C1542 : LPUReg<1542, "%c1542">;
def C1543 : LPUReg<1543, "%c1543">;
def C1544 : LPUReg<1544, "%c1544">;
def C1545 : LPUReg<1545, "%c1545">;
def C1546 : LPUReg<1546, "%c1546">;
def C1547 : LPUReg<1547, "%c1547">;
def C1548 : LPUReg<1548, "%c1548">;
def C1549 : LPUReg<1549, "%c1549">;
def C1550 : LPUReg<1550, "%c1550">;
def C1551 : LPUReg<1551, "%c1551">;
def C1552 : LPUReg<1552, "%c1552">;
def C1553 : LPUReg<1553, "%c1553">;
def C1554 : LPUReg<1554, "%c1554">;
def C1555 : LPUReg<1555, "%c1555">;
def C1556 : LPUReg<1556, "%c1556">;
def C1557 : LPUReg<1557, "%c1557">;
def C1558 : LPUReg<1558, "%c1558">;
def C1559 : LPUReg<1559, "%c1559">;
def C1560 : LPUReg<1560, "%c1560">;
def C1561 : LPUReg<1561, "%c1561">;
def C1562 : LPUReg<1562, "%c1562">;
def C1563 : LPUReg<1563, "%c1563">;
def C1564 : LPUReg<1564, "%c1564">;
def C1565 : LPUReg<1565, "%c1565">;
def C1566 : LPUReg<1566, "%c1566">;
def C1567 : LPUReg<1567, "%c1567">;
def C1568 : LPUReg<1568, "%c1568">;
def C1569 : LPUReg<1569, "%c1569">;
def C1570 : LPUReg<1570, "%c1570">;
def C1571 : LPUReg<1571, "%c1571">;
def C1572 : LPUReg<1572, "%c1572">;
def C1573 : LPUReg<1573, "%c1573">;
def C1574 : LPUReg<1574, "%c1574">;
def C1575 : LPUReg<1575, "%c1575">;
def C1576 : LPUReg<1576, "%c1576">;
def C1577 : LPUReg<1577, "%c1577">;
def C1578 : LPUReg<1578, "%c1578">;
def C1579 : LPUReg<1579, "%c1579">;
def C1580 : LPUReg<1580, "%c1580">;
def C1581 : LPUReg<1581, "%c1581">;
def C1582 : LPUReg<1582, "%c1582">;
def C1583 : LPUReg<1583, "%c1583">;
def C1584 : LPUReg<1584, "%c1584">;
def C1585 : LPUReg<1585, "%c1585">;
def C1586 : LPUReg<1586, "%c1586">;
def C1587 : LPUReg<1587, "%c1587">;
def C1588 : LPUReg<1588, "%c1588">;
def C1589 : LPUReg<1589, "%c1589">;
def C1590 : LPUReg<1590, "%c1590">;
def C1591 : LPUReg<1591, "%c1591">;
def C1592 : LPUReg<1592, "%c1592">;
def C1593 : LPUReg<1593, "%c1593">;
def C1594 : LPUReg<1594, "%c1594">;
def C1595 : LPUReg<1595, "%c1595">;
def C1596 : LPUReg<1596, "%c1596">;
def C1597 : LPUReg<1597, "%c1597">;
def C1598 : LPUReg<1598, "%c1598">;
def C1599 : LPUReg<1599, "%c1599">;
def C1600 : LPUReg<1600, "%c1600">;
def C1601 : LPUReg<1601, "%c1601">;
def C1602 : LPUReg<1602, "%c1602">;
def C1603 : LPUReg<1603, "%c1603">;
def C1604 : LPUReg<1604, "%c1604">;
def C1605 : LPUReg<1605, "%c1605">;
def C1606 : LPUReg<1606, "%c1606">;
def C1607 : LPUReg<1607, "%c1607">;
def C1608 : LPUReg<1608, "%c1608">;
def C1609 : LPUReg<1609, "%c1609">;
def C1610 : LPUReg<1610, "%c1610">;
def C1611 : LPUReg<1611, "%c1611">;
def C1612 : LPUReg<1612, "%c1612">;
def C1613 : LPUReg<1613, "%c1613">;
def C1614 : LPUReg<1614, "%c1614">;
def C1615 : LPUReg<1615, "%c1615">;
def C1616 : LPUReg<1616, "%c1616">;
def C1617 : LPUReg<1617, "%c1617">;
def C1618 : LPUReg<1618, "%c1618">;
def C1619 : LPUReg<1619, "%c1619">;
def C1620 : LPUReg<1620, "%c1620">;
def C1621 : LPUReg<1621, "%c1621">;
def C1622 : LPUReg<1622, "%c1622">;
def C1623 : LPUReg<1623, "%c1623">;
def C1624 : LPUReg<1624, "%c1624">;
def C1625 : LPUReg<1625, "%c1625">;
def C1626 : LPUReg<1626, "%c1626">;
def C1627 : LPUReg<1627, "%c1627">;
def C1628 : LPUReg<1628, "%c1628">;
def C1629 : LPUReg<1629, "%c1629">;
def C1630 : LPUReg<1630, "%c1630">;
def C1631 : LPUReg<1631, "%c1631">;
def C1632 : LPUReg<1632, "%c1632">;
def C1633 : LPUReg<1633, "%c1633">;
def C1634 : LPUReg<1634, "%c1634">;
def C1635 : LPUReg<1635, "%c1635">;
def C1636 : LPUReg<1636, "%c1636">;
def C1637 : LPUReg<1637, "%c1637">;
def C1638 : LPUReg<1638, "%c1638">;
def C1639 : LPUReg<1639, "%c1639">;
def C1640 : LPUReg<1640, "%c1640">;
def C1641 : LPUReg<1641, "%c1641">;
def C1642 : LPUReg<1642, "%c1642">;
def C1643 : LPUReg<1643, "%c1643">;
def C1644 : LPUReg<1644, "%c1644">;
def C1645 : LPUReg<1645, "%c1645">;
def C1646 : LPUReg<1646, "%c1646">;
def C1647 : LPUReg<1647, "%c1647">;
def C1648 : LPUReg<1648, "%c1648">;
def C1649 : LPUReg<1649, "%c1649">;
def C1650 : LPUReg<1650, "%c1650">;
def C1651 : LPUReg<1651, "%c1651">;
def C1652 : LPUReg<1652, "%c1652">;
def C1653 : LPUReg<1653, "%c1653">;
def C1654 : LPUReg<1654, "%c1654">;
def C1655 : LPUReg<1655, "%c1655">;
def C1656 : LPUReg<1656, "%c1656">;
def C1657 : LPUReg<1657, "%c1657">;
def C1658 : LPUReg<1658, "%c1658">;
def C1659 : LPUReg<1659, "%c1659">;
def C1660 : LPUReg<1660, "%c1660">;
def C1661 : LPUReg<1661, "%c1661">;
def C1662 : LPUReg<1662, "%c1662">;
def C1663 : LPUReg<1663, "%c1663">;
def C1664 : LPUReg<1664, "%c1664">;
def C1665 : LPUReg<1665, "%c1665">;
def C1666 : LPUReg<1666, "%c1666">;
def C1667 : LPUReg<1667, "%c1667">;
def C1668 : LPUReg<1668, "%c1668">;
def C1669 : LPUReg<1669, "%c1669">;
def C1670 : LPUReg<1670, "%c1670">;
def C1671 : LPUReg<1671, "%c1671">;
def C1672 : LPUReg<1672, "%c1672">;
def C1673 : LPUReg<1673, "%c1673">;
def C1674 : LPUReg<1674, "%c1674">;
def C1675 : LPUReg<1675, "%c1675">;
def C1676 : LPUReg<1676, "%c1676">;
def C1677 : LPUReg<1677, "%c1677">;
def C1678 : LPUReg<1678, "%c1678">;
def C1679 : LPUReg<1679, "%c1679">;
def C1680 : LPUReg<1680, "%c1680">;
def C1681 : LPUReg<1681, "%c1681">;
def C1682 : LPUReg<1682, "%c1682">;
def C1683 : LPUReg<1683, "%c1683">;
def C1684 : LPUReg<1684, "%c1684">;
def C1685 : LPUReg<1685, "%c1685">;
def C1686 : LPUReg<1686, "%c1686">;
def C1687 : LPUReg<1687, "%c1687">;
def C1688 : LPUReg<1688, "%c1688">;
def C1689 : LPUReg<1689, "%c1689">;
def C1690 : LPUReg<1690, "%c1690">;
def C1691 : LPUReg<1691, "%c1691">;
def C1692 : LPUReg<1692, "%c1692">;
def C1693 : LPUReg<1693, "%c1693">;
def C1694 : LPUReg<1694, "%c1694">;
def C1695 : LPUReg<1695, "%c1695">;
def C1696 : LPUReg<1696, "%c1696">;
def C1697 : LPUReg<1697, "%c1697">;
def C1698 : LPUReg<1698, "%c1698">;
def C1699 : LPUReg<1699, "%c1699">;
def C1700 : LPUReg<1700, "%c1700">;
def C1701 : LPUReg<1701, "%c1701">;
def C1702 : LPUReg<1702, "%c1702">;
def C1703 : LPUReg<1703, "%c1703">;
def C1704 : LPUReg<1704, "%c1704">;
def C1705 : LPUReg<1705, "%c1705">;
def C1706 : LPUReg<1706, "%c1706">;
def C1707 : LPUReg<1707, "%c1707">;
def C1708 : LPUReg<1708, "%c1708">;
def C1709 : LPUReg<1709, "%c1709">;
def C1710 : LPUReg<1710, "%c1710">;
def C1711 : LPUReg<1711, "%c1711">;
def C1712 : LPUReg<1712, "%c1712">;
def C1713 : LPUReg<1713, "%c1713">;
def C1714 : LPUReg<1714, "%c1714">;
def C1715 : LPUReg<1715, "%c1715">;
def C1716 : LPUReg<1716, "%c1716">;
def C1717 : LPUReg<1717, "%c1717">;
def C1718 : LPUReg<1718, "%c1718">;
def C1719 : LPUReg<1719, "%c1719">;
def C1720 : LPUReg<1720, "%c1720">;
def C1721 : LPUReg<1721, "%c1721">;
def C1722 : LPUReg<1722, "%c1722">;
def C1723 : LPUReg<1723, "%c1723">;
def C1724 : LPUReg<1724, "%c1724">;
def C1725 : LPUReg<1725, "%c1725">;
def C1726 : LPUReg<1726, "%c1726">;
def C1727 : LPUReg<1727, "%c1727">;
def C1728 : LPUReg<1728, "%c1728">;
def C1729 : LPUReg<1729, "%c1729">;
def C1730 : LPUReg<1730, "%c1730">;
def C1731 : LPUReg<1731, "%c1731">;
def C1732 : LPUReg<1732, "%c1732">;
def C1733 : LPUReg<1733, "%c1733">;
def C1734 : LPUReg<1734, "%c1734">;
def C1735 : LPUReg<1735, "%c1735">;
def C1736 : LPUReg<1736, "%c1736">;
def C1737 : LPUReg<1737, "%c1737">;
def C1738 : LPUReg<1738, "%c1738">;
def C1739 : LPUReg<1739, "%c1739">;
def C1740 : LPUReg<1740, "%c1740">;
def C1741 : LPUReg<1741, "%c1741">;
def C1742 : LPUReg<1742, "%c1742">;
def C1743 : LPUReg<1743, "%c1743">;
def C1744 : LPUReg<1744, "%c1744">;
def C1745 : LPUReg<1745, "%c1745">;
def C1746 : LPUReg<1746, "%c1746">;
def C1747 : LPUReg<1747, "%c1747">;
def C1748 : LPUReg<1748, "%c1748">;
def C1749 : LPUReg<1749, "%c1749">;
def C1750 : LPUReg<1750, "%c1750">;
def C1751 : LPUReg<1751, "%c1751">;
def C1752 : LPUReg<1752, "%c1752">;
def C1753 : LPUReg<1753, "%c1753">;
def C1754 : LPUReg<1754, "%c1754">;
def C1755 : LPUReg<1755, "%c1755">;
def C1756 : LPUReg<1756, "%c1756">;
def C1757 : LPUReg<1757, "%c1757">;
def C1758 : LPUReg<1758, "%c1758">;
def C1759 : LPUReg<1759, "%c1759">;
def C1760 : LPUReg<1760, "%c1760">;
def C1761 : LPUReg<1761, "%c1761">;
def C1762 : LPUReg<1762, "%c1762">;
def C1763 : LPUReg<1763, "%c1763">;
def C1764 : LPUReg<1764, "%c1764">;
def C1765 : LPUReg<1765, "%c1765">;
def C1766 : LPUReg<1766, "%c1766">;
def C1767 : LPUReg<1767, "%c1767">;
def C1768 : LPUReg<1768, "%c1768">;
def C1769 : LPUReg<1769, "%c1769">;
def C1770 : LPUReg<1770, "%c1770">;
def C1771 : LPUReg<1771, "%c1771">;
def C1772 : LPUReg<1772, "%c1772">;
def C1773 : LPUReg<1773, "%c1773">;
def C1774 : LPUReg<1774, "%c1774">;
def C1775 : LPUReg<1775, "%c1775">;
def C1776 : LPUReg<1776, "%c1776">;
def C1777 : LPUReg<1777, "%c1777">;
def C1778 : LPUReg<1778, "%c1778">;
def C1779 : LPUReg<1779, "%c1779">;
def C1780 : LPUReg<1780, "%c1780">;
def C1781 : LPUReg<1781, "%c1781">;
def C1782 : LPUReg<1782, "%c1782">;
def C1783 : LPUReg<1783, "%c1783">;
def C1784 : LPUReg<1784, "%c1784">;
def C1785 : LPUReg<1785, "%c1785">;
def C1786 : LPUReg<1786, "%c1786">;
def C1787 : LPUReg<1787, "%c1787">;
def C1788 : LPUReg<1788, "%c1788">;
def C1789 : LPUReg<1789, "%c1789">;
def C1790 : LPUReg<1790, "%c1790">;
def C1791 : LPUReg<1791, "%c1791">;
def C1792 : LPUReg<1792, "%c1792">;
def C1793 : LPUReg<1793, "%c1793">;
def C1794 : LPUReg<1794, "%c1794">;
def C1795 : LPUReg<1795, "%c1795">;
def C1796 : LPUReg<1796, "%c1796">;
def C1797 : LPUReg<1797, "%c1797">;
def C1798 : LPUReg<1798, "%c1798">;
def C1799 : LPUReg<1799, "%c1799">;
def C1800 : LPUReg<1800, "%c1800">;
def C1801 : LPUReg<1801, "%c1801">;
def C1802 : LPUReg<1802, "%c1802">;
def C1803 : LPUReg<1803, "%c1803">;
def C1804 : LPUReg<1804, "%c1804">;
def C1805 : LPUReg<1805, "%c1805">;
def C1806 : LPUReg<1806, "%c1806">;
def C1807 : LPUReg<1807, "%c1807">;
def C1808 : LPUReg<1808, "%c1808">;
def C1809 : LPUReg<1809, "%c1809">;
def C1810 : LPUReg<1810, "%c1810">;
def C1811 : LPUReg<1811, "%c1811">;
def C1812 : LPUReg<1812, "%c1812">;
def C1813 : LPUReg<1813, "%c1813">;
def C1814 : LPUReg<1814, "%c1814">;
def C1815 : LPUReg<1815, "%c1815">;
def C1816 : LPUReg<1816, "%c1816">;
def C1817 : LPUReg<1817, "%c1817">;
def C1818 : LPUReg<1818, "%c1818">;
def C1819 : LPUReg<1819, "%c1819">;
def C1820 : LPUReg<1820, "%c1820">;
def C1821 : LPUReg<1821, "%c1821">;
def C1822 : LPUReg<1822, "%c1822">;
def C1823 : LPUReg<1823, "%c1823">;
def C1824 : LPUReg<1824, "%c1824">;
def C1825 : LPUReg<1825, "%c1825">;
def C1826 : LPUReg<1826, "%c1826">;
def C1827 : LPUReg<1827, "%c1827">;
def C1828 : LPUReg<1828, "%c1828">;
def C1829 : LPUReg<1829, "%c1829">;
def C1830 : LPUReg<1830, "%c1830">;
def C1831 : LPUReg<1831, "%c1831">;
def C1832 : LPUReg<1832, "%c1832">;
def C1833 : LPUReg<1833, "%c1833">;
def C1834 : LPUReg<1834, "%c1834">;
def C1835 : LPUReg<1835, "%c1835">;
def C1836 : LPUReg<1836, "%c1836">;
def C1837 : LPUReg<1837, "%c1837">;
def C1838 : LPUReg<1838, "%c1838">;
def C1839 : LPUReg<1839, "%c1839">;
def C1840 : LPUReg<1840, "%c1840">;
def C1841 : LPUReg<1841, "%c1841">;
def C1842 : LPUReg<1842, "%c1842">;
def C1843 : LPUReg<1843, "%c1843">;
def C1844 : LPUReg<1844, "%c1844">;
def C1845 : LPUReg<1845, "%c1845">;
def C1846 : LPUReg<1846, "%c1846">;
def C1847 : LPUReg<1847, "%c1847">;
def C1848 : LPUReg<1848, "%c1848">;
def C1849 : LPUReg<1849, "%c1849">;
def C1850 : LPUReg<1850, "%c1850">;
def C1851 : LPUReg<1851, "%c1851">;
def C1852 : LPUReg<1852, "%c1852">;
def C1853 : LPUReg<1853, "%c1853">;
def C1854 : LPUReg<1854, "%c1854">;
def C1855 : LPUReg<1855, "%c1855">;
def C1856 : LPUReg<1856, "%c1856">;
def C1857 : LPUReg<1857, "%c1857">;
def C1858 : LPUReg<1858, "%c1858">;
def C1859 : LPUReg<1859, "%c1859">;
def C1860 : LPUReg<1860, "%c1860">;
def C1861 : LPUReg<1861, "%c1861">;
def C1862 : LPUReg<1862, "%c1862">;
def C1863 : LPUReg<1863, "%c1863">;
def C1864 : LPUReg<1864, "%c1864">;
def C1865 : LPUReg<1865, "%c1865">;
def C1866 : LPUReg<1866, "%c1866">;
def C1867 : LPUReg<1867, "%c1867">;
def C1868 : LPUReg<1868, "%c1868">;
def C1869 : LPUReg<1869, "%c1869">;
def C1870 : LPUReg<1870, "%c1870">;
def C1871 : LPUReg<1871, "%c1871">;
def C1872 : LPUReg<1872, "%c1872">;
def C1873 : LPUReg<1873, "%c1873">;
def C1874 : LPUReg<1874, "%c1874">;
def C1875 : LPUReg<1875, "%c1875">;
def C1876 : LPUReg<1876, "%c1876">;
def C1877 : LPUReg<1877, "%c1877">;
def C1878 : LPUReg<1878, "%c1878">;
def C1879 : LPUReg<1879, "%c1879">;
def C1880 : LPUReg<1880, "%c1880">;
def C1881 : LPUReg<1881, "%c1881">;
def C1882 : LPUReg<1882, "%c1882">;
def C1883 : LPUReg<1883, "%c1883">;
def C1884 : LPUReg<1884, "%c1884">;
def C1885 : LPUReg<1885, "%c1885">;
def C1886 : LPUReg<1886, "%c1886">;
def C1887 : LPUReg<1887, "%c1887">;
def C1888 : LPUReg<1888, "%c1888">;
def C1889 : LPUReg<1889, "%c1889">;
def C1890 : LPUReg<1890, "%c1890">;
def C1891 : LPUReg<1891, "%c1891">;
def C1892 : LPUReg<1892, "%c1892">;
def C1893 : LPUReg<1893, "%c1893">;
def C1894 : LPUReg<1894, "%c1894">;
def C1895 : LPUReg<1895, "%c1895">;
def C1896 : LPUReg<1896, "%c1896">;
def C1897 : LPUReg<1897, "%c1897">;
def C1898 : LPUReg<1898, "%c1898">;
def C1899 : LPUReg<1899, "%c1899">;
def C1900 : LPUReg<1900, "%c1900">;
def C1901 : LPUReg<1901, "%c1901">;
def C1902 : LPUReg<1902, "%c1902">;
def C1903 : LPUReg<1903, "%c1903">;
def C1904 : LPUReg<1904, "%c1904">;
def C1905 : LPUReg<1905, "%c1905">;
def C1906 : LPUReg<1906, "%c1906">;
def C1907 : LPUReg<1907, "%c1907">;
def C1908 : LPUReg<1908, "%c1908">;
def C1909 : LPUReg<1909, "%c1909">;
def C1910 : LPUReg<1910, "%c1910">;
def C1911 : LPUReg<1911, "%c1911">;
def C1912 : LPUReg<1912, "%c1912">;
def C1913 : LPUReg<1913, "%c1913">;
def C1914 : LPUReg<1914, "%c1914">;
def C1915 : LPUReg<1915, "%c1915">;
def C1916 : LPUReg<1916, "%c1916">;
def C1917 : LPUReg<1917, "%c1917">;
def C1918 : LPUReg<1918, "%c1918">;
def C1919 : LPUReg<1919, "%c1919">;
def C1920 : LPUReg<1920, "%c1920">;
def C1921 : LPUReg<1921, "%c1921">;
def C1922 : LPUReg<1922, "%c1922">;
def C1923 : LPUReg<1923, "%c1923">;
def C1924 : LPUReg<1924, "%c1924">;
def C1925 : LPUReg<1925, "%c1925">;
def C1926 : LPUReg<1926, "%c1926">;
def C1927 : LPUReg<1927, "%c1927">;
def C1928 : LPUReg<1928, "%c1928">;
def C1929 : LPUReg<1929, "%c1929">;
def C1930 : LPUReg<1930, "%c1930">;
def C1931 : LPUReg<1931, "%c1931">;
def C1932 : LPUReg<1932, "%c1932">;
def C1933 : LPUReg<1933, "%c1933">;
def C1934 : LPUReg<1934, "%c1934">;
def C1935 : LPUReg<1935, "%c1935">;
def C1936 : LPUReg<1936, "%c1936">;
def C1937 : LPUReg<1937, "%c1937">;
def C1938 : LPUReg<1938, "%c1938">;
def C1939 : LPUReg<1939, "%c1939">;
def C1940 : LPUReg<1940, "%c1940">;
def C1941 : LPUReg<1941, "%c1941">;
def C1942 : LPUReg<1942, "%c1942">;
def C1943 : LPUReg<1943, "%c1943">;
def C1944 : LPUReg<1944, "%c1944">;
def C1945 : LPUReg<1945, "%c1945">;
def C1946 : LPUReg<1946, "%c1946">;
def C1947 : LPUReg<1947, "%c1947">;
def C1948 : LPUReg<1948, "%c1948">;
def C1949 : LPUReg<1949, "%c1949">;
def C1950 : LPUReg<1950, "%c1950">;
def C1951 : LPUReg<1951, "%c1951">;
def C1952 : LPUReg<1952, "%c1952">;
def C1953 : LPUReg<1953, "%c1953">;
def C1954 : LPUReg<1954, "%c1954">;
def C1955 : LPUReg<1955, "%c1955">;
def C1956 : LPUReg<1956, "%c1956">;
def C1957 : LPUReg<1957, "%c1957">;
def C1958 : LPUReg<1958, "%c1958">;
def C1959 : LPUReg<1959, "%c1959">;
def C1960 : LPUReg<1960, "%c1960">;
def C1961 : LPUReg<1961, "%c1961">;
def C1962 : LPUReg<1962, "%c1962">;
def C1963 : LPUReg<1963, "%c1963">;
def C1964 : LPUReg<1964, "%c1964">;
def C1965 : LPUReg<1965, "%c1965">;
def C1966 : LPUReg<1966, "%c1966">;
def C1967 : LPUReg<1967, "%c1967">;
def C1968 : LPUReg<1968, "%c1968">;
def C1969 : LPUReg<1969, "%c1969">;
def C1970 : LPUReg<1970, "%c1970">;
def C1971 : LPUReg<1971, "%c1971">;
def C1972 : LPUReg<1972, "%c1972">;
def C1973 : LPUReg<1973, "%c1973">;
def C1974 : LPUReg<1974, "%c1974">;
def C1975 : LPUReg<1975, "%c1975">;
def C1976 : LPUReg<1976, "%c1976">;
def C1977 : LPUReg<1977, "%c1977">;
def C1978 : LPUReg<1978, "%c1978">;
def C1979 : LPUReg<1979, "%c1979">;
def C1980 : LPUReg<1980, "%c1980">;
def C1981 : LPUReg<1981, "%c1981">;
def C1982 : LPUReg<1982, "%c1982">;
def C1983 : LPUReg<1983, "%c1983">;
def C1984 : LPUReg<1984, "%c1984">;
def C1985 : LPUReg<1985, "%c1985">;
def C1986 : LPUReg<1986, "%c1986">;
def C1987 : LPUReg<1987, "%c1987">;
def C1988 : LPUReg<1988, "%c1988">;
def C1989 : LPUReg<1989, "%c1989">;
def C1990 : LPUReg<1990, "%c1990">;
def C1991 : LPUReg<1991, "%c1991">;
def C1992 : LPUReg<1992, "%c1992">;
def C1993 : LPUReg<1993, "%c1993">;
def C1994 : LPUReg<1994, "%c1994">;
def C1995 : LPUReg<1995, "%c1995">;
def C1996 : LPUReg<1996, "%c1996">;
def C1997 : LPUReg<1997, "%c1997">;
def C1998 : LPUReg<1998, "%c1998">;
def C1999 : LPUReg<1999, "%c1999">;
def C2000 : LPUReg<2000, "%c2000">;
def C2001 : LPUReg<2001, "%c2001">;
def C2002 : LPUReg<2002, "%c2002">;
def C2003 : LPUReg<2003, "%c2003">;
def C2004 : LPUReg<2004, "%c2004">;
def C2005 : LPUReg<2005, "%c2005">;
def C2006 : LPUReg<2006, "%c2006">;
def C2007 : LPUReg<2007, "%c2007">;
def C2008 : LPUReg<2008, "%c2008">;
def C2009 : LPUReg<2009, "%c2009">;
def C2010 : LPUReg<2010, "%c2010">;
def C2011 : LPUReg<2011, "%c2011">;
def C2012 : LPUReg<2012, "%c2012">;
def C2013 : LPUReg<2013, "%c2013">;
def C2014 : LPUReg<2014, "%c2014">;
def C2015 : LPUReg<2015, "%c2015">;
def C2016 : LPUReg<2016, "%c2016">;
def C2017 : LPUReg<2017, "%c2017">;
def C2018 : LPUReg<2018, "%c2018">;
def C2019 : LPUReg<2019, "%c2019">;
def C2020 : LPUReg<2020, "%c2020">;
def C2021 : LPUReg<2021, "%c2021">;
def C2022 : LPUReg<2022, "%c2022">;
def C2023 : LPUReg<2023, "%c2023">;
def C2024 : LPUReg<2024, "%c2024">;
def C2025 : LPUReg<2025, "%c2025">;
def C2026 : LPUReg<2026, "%c2026">;
def C2027 : LPUReg<2027, "%c2027">;
def C2028 : LPUReg<2028, "%c2028">;
def C2029 : LPUReg<2029, "%c2029">;
def C2030 : LPUReg<2030, "%c2030">;
def C2031 : LPUReg<2031, "%c2031">;
def C2032 : LPUReg<2032, "%c2032">;
def C2033 : LPUReg<2033, "%c2033">;
def C2034 : LPUReg<2034, "%c2034">;
def C2035 : LPUReg<2035, "%c2035">;
def C2036 : LPUReg<2036, "%c2036">;
def C2037 : LPUReg<2037, "%c2037">;
def C2038 : LPUReg<2038, "%c2038">;
def C2039 : LPUReg<2039, "%c2039">;
def C2040 : LPUReg<2040, "%c2040">;
def C2041 : LPUReg<2041, "%c2041">;
def C2042 : LPUReg<2042, "%c2042">;
def C2043 : LPUReg<2043, "%c2043">;
def C2044 : LPUReg<2044, "%c2044">;
def C2045 : LPUReg<2045, "%c2045">;
def C2046 : LPUReg<2046, "%c2046">;
def C2047 : LPUReg<2047, "%c2047">;
def C2048 : LPUReg<2048, "%c2048">;
def C2049 : LPUReg<2049, "%c2049">;
def C2050 : LPUReg<2050, "%c2050">;
def C2051 : LPUReg<2051, "%c2051">;
def C2052 : LPUReg<2052, "%c2052">;
def C2053 : LPUReg<2053, "%c2053">;
def C2054 : LPUReg<2054, "%c2054">;
def C2055 : LPUReg<2055, "%c2055">;
def C2056 : LPUReg<2056, "%c2056">;
def C2057 : LPUReg<2057, "%c2057">;
def C2058 : LPUReg<2058, "%c2058">;
def C2059 : LPUReg<2059, "%c2059">;
def C2060 : LPUReg<2060, "%c2060">;
def C2061 : LPUReg<2061, "%c2061">;
def C2062 : LPUReg<2062, "%c2062">;
def C2063 : LPUReg<2063, "%c2063">;
def C2064 : LPUReg<2064, "%c2064">;
def C2065 : LPUReg<2065, "%c2065">;
def C2066 : LPUReg<2066, "%c2066">;
def C2067 : LPUReg<2067, "%c2067">;
def C2068 : LPUReg<2068, "%c2068">;
def C2069 : LPUReg<2069, "%c2069">;
def C2070 : LPUReg<2070, "%c2070">;
def C2071 : LPUReg<2071, "%c2071">;
def C2072 : LPUReg<2072, "%c2072">;
def C2073 : LPUReg<2073, "%c2073">;
def C2074 : LPUReg<2074, "%c2074">;
def C2075 : LPUReg<2075, "%c2075">;
def C2076 : LPUReg<2076, "%c2076">;
def C2077 : LPUReg<2077, "%c2077">;
def C2078 : LPUReg<2078, "%c2078">;
def C2079 : LPUReg<2079, "%c2079">;
def C2080 : LPUReg<2080, "%c2080">;
def C2081 : LPUReg<2081, "%c2081">;
def C2082 : LPUReg<2082, "%c2082">;
def C2083 : LPUReg<2083, "%c2083">;
def C2084 : LPUReg<2084, "%c2084">;
def C2085 : LPUReg<2085, "%c2085">;
def C2086 : LPUReg<2086, "%c2086">;
def C2087 : LPUReg<2087, "%c2087">;
def C2088 : LPUReg<2088, "%c2088">;
def C2089 : LPUReg<2089, "%c2089">;
def C2090 : LPUReg<2090, "%c2090">;
def C2091 : LPUReg<2091, "%c2091">;
def C2092 : LPUReg<2092, "%c2092">;
def C2093 : LPUReg<2093, "%c2093">;
def C2094 : LPUReg<2094, "%c2094">;
def C2095 : LPUReg<2095, "%c2095">;
def C2096 : LPUReg<2096, "%c2096">;
def C2097 : LPUReg<2097, "%c2097">;
def C2098 : LPUReg<2098, "%c2098">;
def C2099 : LPUReg<2099, "%c2099">;
def C2100 : LPUReg<2100, "%c2100">;
def C2101 : LPUReg<2101, "%c2101">;
def C2102 : LPUReg<2102, "%c2102">;
def C2103 : LPUReg<2103, "%c2103">;
def C2104 : LPUReg<2104, "%c2104">;
def C2105 : LPUReg<2105, "%c2105">;
def C2106 : LPUReg<2106, "%c2106">;
def C2107 : LPUReg<2107, "%c2107">;
def C2108 : LPUReg<2108, "%c2108">;
def C2109 : LPUReg<2109, "%c2109">;
def C2110 : LPUReg<2110, "%c2110">;
def C2111 : LPUReg<2111, "%c2111">;
def C2112 : LPUReg<2112, "%c2112">;
def C2113 : LPUReg<2113, "%c2113">;
def C2114 : LPUReg<2114, "%c2114">;
def C2115 : LPUReg<2115, "%c2115">;
def C2116 : LPUReg<2116, "%c2116">;
def C2117 : LPUReg<2117, "%c2117">;
def C2118 : LPUReg<2118, "%c2118">;
def C2119 : LPUReg<2119, "%c2119">;
def C2120 : LPUReg<2120, "%c2120">;
def C2121 : LPUReg<2121, "%c2121">;
def C2122 : LPUReg<2122, "%c2122">;
def C2123 : LPUReg<2123, "%c2123">;
def C2124 : LPUReg<2124, "%c2124">;
def C2125 : LPUReg<2125, "%c2125">;
def C2126 : LPUReg<2126, "%c2126">;
def C2127 : LPUReg<2127, "%c2127">;
def C2128 : LPUReg<2128, "%c2128">;
def C2129 : LPUReg<2129, "%c2129">;
def C2130 : LPUReg<2130, "%c2130">;
def C2131 : LPUReg<2131, "%c2131">;
def C2132 : LPUReg<2132, "%c2132">;
def C2133 : LPUReg<2133, "%c2133">;
def C2134 : LPUReg<2134, "%c2134">;
def C2135 : LPUReg<2135, "%c2135">;
def C2136 : LPUReg<2136, "%c2136">;
def C2137 : LPUReg<2137, "%c2137">;
def C2138 : LPUReg<2138, "%c2138">;
def C2139 : LPUReg<2139, "%c2139">;
def C2140 : LPUReg<2140, "%c2140">;
def C2141 : LPUReg<2141, "%c2141">;
def C2142 : LPUReg<2142, "%c2142">;
def C2143 : LPUReg<2143, "%c2143">;
def C2144 : LPUReg<2144, "%c2144">;
def C2145 : LPUReg<2145, "%c2145">;
def C2146 : LPUReg<2146, "%c2146">;
def C2147 : LPUReg<2147, "%c2147">;
def C2148 : LPUReg<2148, "%c2148">;
def C2149 : LPUReg<2149, "%c2149">;
def C2150 : LPUReg<2150, "%c2150">;
def C2151 : LPUReg<2151, "%c2151">;
def C2152 : LPUReg<2152, "%c2152">;
def C2153 : LPUReg<2153, "%c2153">;
def C2154 : LPUReg<2154, "%c2154">;
def C2155 : LPUReg<2155, "%c2155">;
def C2156 : LPUReg<2156, "%c2156">;
def C2157 : LPUReg<2157, "%c2157">;
def C2158 : LPUReg<2158, "%c2158">;
def C2159 : LPUReg<2159, "%c2159">;
def C2160 : LPUReg<2160, "%c2160">;
def C2161 : LPUReg<2161, "%c2161">;
def C2162 : LPUReg<2162, "%c2162">;
def C2163 : LPUReg<2163, "%c2163">;
def C2164 : LPUReg<2164, "%c2164">;
def C2165 : LPUReg<2165, "%c2165">;
def C2166 : LPUReg<2166, "%c2166">;
def C2167 : LPUReg<2167, "%c2167">;
def C2168 : LPUReg<2168, "%c2168">;
def C2169 : LPUReg<2169, "%c2169">;
def C2170 : LPUReg<2170, "%c2170">;
def C2171 : LPUReg<2171, "%c2171">;
def C2172 : LPUReg<2172, "%c2172">;
def C2173 : LPUReg<2173, "%c2173">;
def C2174 : LPUReg<2174, "%c2174">;
def C2175 : LPUReg<2175, "%c2175">;
def C2176 : LPUReg<2176, "%c2176">;
def C2177 : LPUReg<2177, "%c2177">;
def C2178 : LPUReg<2178, "%c2178">;
def C2179 : LPUReg<2179, "%c2179">;
def C2180 : LPUReg<2180, "%c2180">;
def C2181 : LPUReg<2181, "%c2181">;
def C2182 : LPUReg<2182, "%c2182">;
def C2183 : LPUReg<2183, "%c2183">;
def C2184 : LPUReg<2184, "%c2184">;
def C2185 : LPUReg<2185, "%c2185">;
def C2186 : LPUReg<2186, "%c2186">;
def C2187 : LPUReg<2187, "%c2187">;
def C2188 : LPUReg<2188, "%c2188">;
def C2189 : LPUReg<2189, "%c2189">;
def C2190 : LPUReg<2190, "%c2190">;
def C2191 : LPUReg<2191, "%c2191">;
def C2192 : LPUReg<2192, "%c2192">;
def C2193 : LPUReg<2193, "%c2193">;
def C2194 : LPUReg<2194, "%c2194">;
def C2195 : LPUReg<2195, "%c2195">;
def C2196 : LPUReg<2196, "%c2196">;
def C2197 : LPUReg<2197, "%c2197">;
def C2198 : LPUReg<2198, "%c2198">;
def C2199 : LPUReg<2199, "%c2199">;
def C2200 : LPUReg<2200, "%c2200">;
def C2201 : LPUReg<2201, "%c2201">;
def C2202 : LPUReg<2202, "%c2202">;
def C2203 : LPUReg<2203, "%c2203">;
def C2204 : LPUReg<2204, "%c2204">;
def C2205 : LPUReg<2205, "%c2205">;
def C2206 : LPUReg<2206, "%c2206">;
def C2207 : LPUReg<2207, "%c2207">;
def C2208 : LPUReg<2208, "%c2208">;
def C2209 : LPUReg<2209, "%c2209">;
def C2210 : LPUReg<2210, "%c2210">;
def C2211 : LPUReg<2211, "%c2211">;
def C2212 : LPUReg<2212, "%c2212">;
def C2213 : LPUReg<2213, "%c2213">;
def C2214 : LPUReg<2214, "%c2214">;
def C2215 : LPUReg<2215, "%c2215">;
def C2216 : LPUReg<2216, "%c2216">;
def C2217 : LPUReg<2217, "%c2217">;
def C2218 : LPUReg<2218, "%c2218">;
def C2219 : LPUReg<2219, "%c2219">;
def C2220 : LPUReg<2220, "%c2220">;
def C2221 : LPUReg<2221, "%c2221">;
def C2222 : LPUReg<2222, "%c2222">;
def C2223 : LPUReg<2223, "%c2223">;
def C2224 : LPUReg<2224, "%c2224">;
def C2225 : LPUReg<2225, "%c2225">;
def C2226 : LPUReg<2226, "%c2226">;
def C2227 : LPUReg<2227, "%c2227">;
def C2228 : LPUReg<2228, "%c2228">;
def C2229 : LPUReg<2229, "%c2229">;
def C2230 : LPUReg<2230, "%c2230">;
def C2231 : LPUReg<2231, "%c2231">;
def C2232 : LPUReg<2232, "%c2232">;
def C2233 : LPUReg<2233, "%c2233">;
def C2234 : LPUReg<2234, "%c2234">;
def C2235 : LPUReg<2235, "%c2235">;
def C2236 : LPUReg<2236, "%c2236">;
def C2237 : LPUReg<2237, "%c2237">;
def C2238 : LPUReg<2238, "%c2238">;
def C2239 : LPUReg<2239, "%c2239">;
def C2240 : LPUReg<2240, "%c2240">;
def C2241 : LPUReg<2241, "%c2241">;
def C2242 : LPUReg<2242, "%c2242">;
def C2243 : LPUReg<2243, "%c2243">;
def C2244 : LPUReg<2244, "%c2244">;
def C2245 : LPUReg<2245, "%c2245">;
def C2246 : LPUReg<2246, "%c2246">;
def C2247 : LPUReg<2247, "%c2247">;
def C2248 : LPUReg<2248, "%c2248">;
def C2249 : LPUReg<2249, "%c2249">;
def C2250 : LPUReg<2250, "%c2250">;
def C2251 : LPUReg<2251, "%c2251">;
def C2252 : LPUReg<2252, "%c2252">;
def C2253 : LPUReg<2253, "%c2253">;
def C2254 : LPUReg<2254, "%c2254">;
def C2255 : LPUReg<2255, "%c2255">;
def C2256 : LPUReg<2256, "%c2256">;
def C2257 : LPUReg<2257, "%c2257">;
def C2258 : LPUReg<2258, "%c2258">;
def C2259 : LPUReg<2259, "%c2259">;
def C2260 : LPUReg<2260, "%c2260">;
def C2261 : LPUReg<2261, "%c2261">;
def C2262 : LPUReg<2262, "%c2262">;
def C2263 : LPUReg<2263, "%c2263">;
def C2264 : LPUReg<2264, "%c2264">;
def C2265 : LPUReg<2265, "%c2265">;
def C2266 : LPUReg<2266, "%c2266">;
def C2267 : LPUReg<2267, "%c2267">;
def C2268 : LPUReg<2268, "%c2268">;
def C2269 : LPUReg<2269, "%c2269">;
def C2270 : LPUReg<2270, "%c2270">;
def C2271 : LPUReg<2271, "%c2271">;
def C2272 : LPUReg<2272, "%c2272">;
def C2273 : LPUReg<2273, "%c2273">;
def C2274 : LPUReg<2274, "%c2274">;
def C2275 : LPUReg<2275, "%c2275">;
def C2276 : LPUReg<2276, "%c2276">;
def C2277 : LPUReg<2277, "%c2277">;
def C2278 : LPUReg<2278, "%c2278">;
def C2279 : LPUReg<2279, "%c2279">;
def C2280 : LPUReg<2280, "%c2280">;
def C2281 : LPUReg<2281, "%c2281">;
def C2282 : LPUReg<2282, "%c2282">;
def C2283 : LPUReg<2283, "%c2283">;
def C2284 : LPUReg<2284, "%c2284">;
def C2285 : LPUReg<2285, "%c2285">;
def C2286 : LPUReg<2286, "%c2286">;
def C2287 : LPUReg<2287, "%c2287">;
def C2288 : LPUReg<2288, "%c2288">;
def C2289 : LPUReg<2289, "%c2289">;
def C2290 : LPUReg<2290, "%c2290">;
def C2291 : LPUReg<2291, "%c2291">;
def C2292 : LPUReg<2292, "%c2292">;
def C2293 : LPUReg<2293, "%c2293">;
def C2294 : LPUReg<2294, "%c2294">;
def C2295 : LPUReg<2295, "%c2295">;
def C2296 : LPUReg<2296, "%c2296">;
def C2297 : LPUReg<2297, "%c2297">;
def C2298 : LPUReg<2298, "%c2298">;
def C2299 : LPUReg<2299, "%c2299">;
def C2300 : LPUReg<2300, "%c2300">;
def C2301 : LPUReg<2301, "%c2301">;
def C2302 : LPUReg<2302, "%c2302">;
def C2303 : LPUReg<2303, "%c2303">;
def C2304 : LPUReg<2304, "%c2304">;
def C2305 : LPUReg<2305, "%c2305">;
def C2306 : LPUReg<2306, "%c2306">;
def C2307 : LPUReg<2307, "%c2307">;
def C2308 : LPUReg<2308, "%c2308">;
def C2309 : LPUReg<2309, "%c2309">;
def C2310 : LPUReg<2310, "%c2310">;
def C2311 : LPUReg<2311, "%c2311">;
def C2312 : LPUReg<2312, "%c2312">;
def C2313 : LPUReg<2313, "%c2313">;
def C2314 : LPUReg<2314, "%c2314">;
def C2315 : LPUReg<2315, "%c2315">;
def C2316 : LPUReg<2316, "%c2316">;
def C2317 : LPUReg<2317, "%c2317">;
def C2318 : LPUReg<2318, "%c2318">;
def C2319 : LPUReg<2319, "%c2319">;
def C2320 : LPUReg<2320, "%c2320">;
def C2321 : LPUReg<2321, "%c2321">;
def C2322 : LPUReg<2322, "%c2322">;
def C2323 : LPUReg<2323, "%c2323">;
def C2324 : LPUReg<2324, "%c2324">;
def C2325 : LPUReg<2325, "%c2325">;
def C2326 : LPUReg<2326, "%c2326">;
def C2327 : LPUReg<2327, "%c2327">;
def C2328 : LPUReg<2328, "%c2328">;
def C2329 : LPUReg<2329, "%c2329">;
def C2330 : LPUReg<2330, "%c2330">;
def C2331 : LPUReg<2331, "%c2331">;
def C2332 : LPUReg<2332, "%c2332">;
def C2333 : LPUReg<2333, "%c2333">;
def C2334 : LPUReg<2334, "%c2334">;
def C2335 : LPUReg<2335, "%c2335">;
def C2336 : LPUReg<2336, "%c2336">;
def C2337 : LPUReg<2337, "%c2337">;
def C2338 : LPUReg<2338, "%c2338">;
def C2339 : LPUReg<2339, "%c2339">;
def C2340 : LPUReg<2340, "%c2340">;
def C2341 : LPUReg<2341, "%c2341">;
def C2342 : LPUReg<2342, "%c2342">;
def C2343 : LPUReg<2343, "%c2343">;
def C2344 : LPUReg<2344, "%c2344">;
def C2345 : LPUReg<2345, "%c2345">;
def C2346 : LPUReg<2346, "%c2346">;
def C2347 : LPUReg<2347, "%c2347">;
def C2348 : LPUReg<2348, "%c2348">;
def C2349 : LPUReg<2349, "%c2349">;
def C2350 : LPUReg<2350, "%c2350">;
def C2351 : LPUReg<2351, "%c2351">;
def C2352 : LPUReg<2352, "%c2352">;
def C2353 : LPUReg<2353, "%c2353">;
def C2354 : LPUReg<2354, "%c2354">;
def C2355 : LPUReg<2355, "%c2355">;
def C2356 : LPUReg<2356, "%c2356">;
def C2357 : LPUReg<2357, "%c2357">;
def C2358 : LPUReg<2358, "%c2358">;
def C2359 : LPUReg<2359, "%c2359">;
def C2360 : LPUReg<2360, "%c2360">;
def C2361 : LPUReg<2361, "%c2361">;
def C2362 : LPUReg<2362, "%c2362">;
def C2363 : LPUReg<2363, "%c2363">;
def C2364 : LPUReg<2364, "%c2364">;
def C2365 : LPUReg<2365, "%c2365">;
def C2366 : LPUReg<2366, "%c2366">;
def C2367 : LPUReg<2367, "%c2367">;
def C2368 : LPUReg<2368, "%c2368">;
def C2369 : LPUReg<2369, "%c2369">;
def C2370 : LPUReg<2370, "%c2370">;
def C2371 : LPUReg<2371, "%c2371">;
def C2372 : LPUReg<2372, "%c2372">;
def C2373 : LPUReg<2373, "%c2373">;
def C2374 : LPUReg<2374, "%c2374">;
def C2375 : LPUReg<2375, "%c2375">;
def C2376 : LPUReg<2376, "%c2376">;
def C2377 : LPUReg<2377, "%c2377">;
def C2378 : LPUReg<2378, "%c2378">;
def C2379 : LPUReg<2379, "%c2379">;
def C2380 : LPUReg<2380, "%c2380">;
def C2381 : LPUReg<2381, "%c2381">;
def C2382 : LPUReg<2382, "%c2382">;
def C2383 : LPUReg<2383, "%c2383">;
def C2384 : LPUReg<2384, "%c2384">;
def C2385 : LPUReg<2385, "%c2385">;
def C2386 : LPUReg<2386, "%c2386">;
def C2387 : LPUReg<2387, "%c2387">;
def C2388 : LPUReg<2388, "%c2388">;
def C2389 : LPUReg<2389, "%c2389">;
def C2390 : LPUReg<2390, "%c2390">;
def C2391 : LPUReg<2391, "%c2391">;
def C2392 : LPUReg<2392, "%c2392">;
def C2393 : LPUReg<2393, "%c2393">;
def C2394 : LPUReg<2394, "%c2394">;
def C2395 : LPUReg<2395, "%c2395">;
def C2396 : LPUReg<2396, "%c2396">;
def C2397 : LPUReg<2397, "%c2397">;
def C2398 : LPUReg<2398, "%c2398">;
def C2399 : LPUReg<2399, "%c2399">;
def C2400 : LPUReg<2400, "%c2400">;
def C2401 : LPUReg<2401, "%c2401">;
def C2402 : LPUReg<2402, "%c2402">;
def C2403 : LPUReg<2403, "%c2403">;
def C2404 : LPUReg<2404, "%c2404">;
def C2405 : LPUReg<2405, "%c2405">;
def C2406 : LPUReg<2406, "%c2406">;
def C2407 : LPUReg<2407, "%c2407">;
def C2408 : LPUReg<2408, "%c2408">;
def C2409 : LPUReg<2409, "%c2409">;
def C2410 : LPUReg<2410, "%c2410">;
def C2411 : LPUReg<2411, "%c2411">;
def C2412 : LPUReg<2412, "%c2412">;
def C2413 : LPUReg<2413, "%c2413">;
def C2414 : LPUReg<2414, "%c2414">;
def C2415 : LPUReg<2415, "%c2415">;
def C2416 : LPUReg<2416, "%c2416">;
def C2417 : LPUReg<2417, "%c2417">;
def C2418 : LPUReg<2418, "%c2418">;
def C2419 : LPUReg<2419, "%c2419">;
def C2420 : LPUReg<2420, "%c2420">;
def C2421 : LPUReg<2421, "%c2421">;
def C2422 : LPUReg<2422, "%c2422">;
def C2423 : LPUReg<2423, "%c2423">;
def C2424 : LPUReg<2424, "%c2424">;
def C2425 : LPUReg<2425, "%c2425">;
def C2426 : LPUReg<2426, "%c2426">;
def C2427 : LPUReg<2427, "%c2427">;
def C2428 : LPUReg<2428, "%c2428">;
def C2429 : LPUReg<2429, "%c2429">;
def C2430 : LPUReg<2430, "%c2430">;
def C2431 : LPUReg<2431, "%c2431">;
def C2432 : LPUReg<2432, "%c2432">;
def C2433 : LPUReg<2433, "%c2433">;
def C2434 : LPUReg<2434, "%c2434">;
def C2435 : LPUReg<2435, "%c2435">;
def C2436 : LPUReg<2436, "%c2436">;
def C2437 : LPUReg<2437, "%c2437">;
def C2438 : LPUReg<2438, "%c2438">;
def C2439 : LPUReg<2439, "%c2439">;
def C2440 : LPUReg<2440, "%c2440">;
def C2441 : LPUReg<2441, "%c2441">;
def C2442 : LPUReg<2442, "%c2442">;
def C2443 : LPUReg<2443, "%c2443">;
def C2444 : LPUReg<2444, "%c2444">;
def C2445 : LPUReg<2445, "%c2445">;
def C2446 : LPUReg<2446, "%c2446">;
def C2447 : LPUReg<2447, "%c2447">;
def C2448 : LPUReg<2448, "%c2448">;
def C2449 : LPUReg<2449, "%c2449">;
def C2450 : LPUReg<2450, "%c2450">;
def C2451 : LPUReg<2451, "%c2451">;
def C2452 : LPUReg<2452, "%c2452">;
def C2453 : LPUReg<2453, "%c2453">;
def C2454 : LPUReg<2454, "%c2454">;
def C2455 : LPUReg<2455, "%c2455">;
def C2456 : LPUReg<2456, "%c2456">;
def C2457 : LPUReg<2457, "%c2457">;
def C2458 : LPUReg<2458, "%c2458">;
def C2459 : LPUReg<2459, "%c2459">;
def C2460 : LPUReg<2460, "%c2460">;
def C2461 : LPUReg<2461, "%c2461">;
def C2462 : LPUReg<2462, "%c2462">;
def C2463 : LPUReg<2463, "%c2463">;
def C2464 : LPUReg<2464, "%c2464">;
def C2465 : LPUReg<2465, "%c2465">;
def C2466 : LPUReg<2466, "%c2466">;
def C2467 : LPUReg<2467, "%c2467">;
def C2468 : LPUReg<2468, "%c2468">;
def C2469 : LPUReg<2469, "%c2469">;
def C2470 : LPUReg<2470, "%c2470">;
def C2471 : LPUReg<2471, "%c2471">;
def C2472 : LPUReg<2472, "%c2472">;
def C2473 : LPUReg<2473, "%c2473">;
def C2474 : LPUReg<2474, "%c2474">;
def C2475 : LPUReg<2475, "%c2475">;
def C2476 : LPUReg<2476, "%c2476">;
def C2477 : LPUReg<2477, "%c2477">;
def C2478 : LPUReg<2478, "%c2478">;
def C2479 : LPUReg<2479, "%c2479">;
def C2480 : LPUReg<2480, "%c2480">;
def C2481 : LPUReg<2481, "%c2481">;
def C2482 : LPUReg<2482, "%c2482">;
def C2483 : LPUReg<2483, "%c2483">;
def C2484 : LPUReg<2484, "%c2484">;
def C2485 : LPUReg<2485, "%c2485">;
def C2486 : LPUReg<2486, "%c2486">;
def C2487 : LPUReg<2487, "%c2487">;
def C2488 : LPUReg<2488, "%c2488">;
def C2489 : LPUReg<2489, "%c2489">;
def C2490 : LPUReg<2490, "%c2490">;
def C2491 : LPUReg<2491, "%c2491">;
def C2492 : LPUReg<2492, "%c2492">;
def C2493 : LPUReg<2493, "%c2493">;
def C2494 : LPUReg<2494, "%c2494">;
def C2495 : LPUReg<2495, "%c2495">;
def C2496 : LPUReg<2496, "%c2496">;
def C2497 : LPUReg<2497, "%c2497">;
def C2498 : LPUReg<2498, "%c2498">;
def C2499 : LPUReg<2499, "%c2499">;
def C2500 : LPUReg<2500, "%c2500">;
def C2501 : LPUReg<2501, "%c2501">;
def C2502 : LPUReg<2502, "%c2502">;
def C2503 : LPUReg<2503, "%c2503">;
def C2504 : LPUReg<2504, "%c2504">;
def C2505 : LPUReg<2505, "%c2505">;
def C2506 : LPUReg<2506, "%c2506">;
def C2507 : LPUReg<2507, "%c2507">;
def C2508 : LPUReg<2508, "%c2508">;
def C2509 : LPUReg<2509, "%c2509">;
def C2510 : LPUReg<2510, "%c2510">;
def C2511 : LPUReg<2511, "%c2511">;
def C2512 : LPUReg<2512, "%c2512">;
def C2513 : LPUReg<2513, "%c2513">;
def C2514 : LPUReg<2514, "%c2514">;
def C2515 : LPUReg<2515, "%c2515">;
def C2516 : LPUReg<2516, "%c2516">;
def C2517 : LPUReg<2517, "%c2517">;
def C2518 : LPUReg<2518, "%c2518">;
def C2519 : LPUReg<2519, "%c2519">;
def C2520 : LPUReg<2520, "%c2520">;
def C2521 : LPUReg<2521, "%c2521">;
def C2522 : LPUReg<2522, "%c2522">;
def C2523 : LPUReg<2523, "%c2523">;
def C2524 : LPUReg<2524, "%c2524">;
def C2525 : LPUReg<2525, "%c2525">;
def C2526 : LPUReg<2526, "%c2526">;
def C2527 : LPUReg<2527, "%c2527">;
def C2528 : LPUReg<2528, "%c2528">;
def C2529 : LPUReg<2529, "%c2529">;
def C2530 : LPUReg<2530, "%c2530">;
def C2531 : LPUReg<2531, "%c2531">;
def C2532 : LPUReg<2532, "%c2532">;
def C2533 : LPUReg<2533, "%c2533">;
def C2534 : LPUReg<2534, "%c2534">;
def C2535 : LPUReg<2535, "%c2535">;
def C2536 : LPUReg<2536, "%c2536">;
def C2537 : LPUReg<2537, "%c2537">;
def C2538 : LPUReg<2538, "%c2538">;
def C2539 : LPUReg<2539, "%c2539">;
def C2540 : LPUReg<2540, "%c2540">;
def C2541 : LPUReg<2541, "%c2541">;
def C2542 : LPUReg<2542, "%c2542">;
def C2543 : LPUReg<2543, "%c2543">;
def C2544 : LPUReg<2544, "%c2544">;
def C2545 : LPUReg<2545, "%c2545">;
def C2546 : LPUReg<2546, "%c2546">;
def C2547 : LPUReg<2547, "%c2547">;
def C2548 : LPUReg<2548, "%c2548">;
def C2549 : LPUReg<2549, "%c2549">;
def C2550 : LPUReg<2550, "%c2550">;
def C2551 : LPUReg<2551, "%c2551">;
def C2552 : LPUReg<2552, "%c2552">;
def C2553 : LPUReg<2553, "%c2553">;
def C2554 : LPUReg<2554, "%c2554">;
def C2555 : LPUReg<2555, "%c2555">;
def C2556 : LPUReg<2556, "%c2556">;
def C2557 : LPUReg<2557, "%c2557">;
def C2558 : LPUReg<2558, "%c2558">;
def C2559 : LPUReg<2559, "%c2559">;
def C2560 : LPUReg<2560, "%c2560">;
def C2561 : LPUReg<2561, "%c2561">;
def C2562 : LPUReg<2562, "%c2562">;
def C2563 : LPUReg<2563, "%c2563">;
def C2564 : LPUReg<2564, "%c2564">;
def C2565 : LPUReg<2565, "%c2565">;
def C2566 : LPUReg<2566, "%c2566">;
def C2567 : LPUReg<2567, "%c2567">;
def C2568 : LPUReg<2568, "%c2568">;
def C2569 : LPUReg<2569, "%c2569">;
def C2570 : LPUReg<2570, "%c2570">;
def C2571 : LPUReg<2571, "%c2571">;
def C2572 : LPUReg<2572, "%c2572">;
def C2573 : LPUReg<2573, "%c2573">;
def C2574 : LPUReg<2574, "%c2574">;
def C2575 : LPUReg<2575, "%c2575">;
def C2576 : LPUReg<2576, "%c2576">;
def C2577 : LPUReg<2577, "%c2577">;
def C2578 : LPUReg<2578, "%c2578">;
def C2579 : LPUReg<2579, "%c2579">;
def C2580 : LPUReg<2580, "%c2580">;
def C2581 : LPUReg<2581, "%c2581">;
def C2582 : LPUReg<2582, "%c2582">;
def C2583 : LPUReg<2583, "%c2583">;
def C2584 : LPUReg<2584, "%c2584">;
def C2585 : LPUReg<2585, "%c2585">;
def C2586 : LPUReg<2586, "%c2586">;
def C2587 : LPUReg<2587, "%c2587">;
def C2588 : LPUReg<2588, "%c2588">;
def C2589 : LPUReg<2589, "%c2589">;
def C2590 : LPUReg<2590, "%c2590">;
def C2591 : LPUReg<2591, "%c2591">;
def C2592 : LPUReg<2592, "%c2592">;
def C2593 : LPUReg<2593, "%c2593">;
def C2594 : LPUReg<2594, "%c2594">;
def C2595 : LPUReg<2595, "%c2595">;
def C2596 : LPUReg<2596, "%c2596">;
def C2597 : LPUReg<2597, "%c2597">;
def C2598 : LPUReg<2598, "%c2598">;
def C2599 : LPUReg<2599, "%c2599">;
def C2600 : LPUReg<2600, "%c2600">;
def C2601 : LPUReg<2601, "%c2601">;
def C2602 : LPUReg<2602, "%c2602">;
def C2603 : LPUReg<2603, "%c2603">;
def C2604 : LPUReg<2604, "%c2604">;
def C2605 : LPUReg<2605, "%c2605">;
def C2606 : LPUReg<2606, "%c2606">;
def C2607 : LPUReg<2607, "%c2607">;
def C2608 : LPUReg<2608, "%c2608">;
def C2609 : LPUReg<2609, "%c2609">;
def C2610 : LPUReg<2610, "%c2610">;
def C2611 : LPUReg<2611, "%c2611">;
def C2612 : LPUReg<2612, "%c2612">;
def C2613 : LPUReg<2613, "%c2613">;
def C2614 : LPUReg<2614, "%c2614">;
def C2615 : LPUReg<2615, "%c2615">;
def C2616 : LPUReg<2616, "%c2616">;
def C2617 : LPUReg<2617, "%c2617">;
def C2618 : LPUReg<2618, "%c2618">;
def C2619 : LPUReg<2619, "%c2619">;
def C2620 : LPUReg<2620, "%c2620">;
def C2621 : LPUReg<2621, "%c2621">;
def C2622 : LPUReg<2622, "%c2622">;
def C2623 : LPUReg<2623, "%c2623">;
def C2624 : LPUReg<2624, "%c2624">;
def C2625 : LPUReg<2625, "%c2625">;
def C2626 : LPUReg<2626, "%c2626">;
def C2627 : LPUReg<2627, "%c2627">;
def C2628 : LPUReg<2628, "%c2628">;
def C2629 : LPUReg<2629, "%c2629">;
def C2630 : LPUReg<2630, "%c2630">;
def C2631 : LPUReg<2631, "%c2631">;
def C2632 : LPUReg<2632, "%c2632">;
def C2633 : LPUReg<2633, "%c2633">;
def C2634 : LPUReg<2634, "%c2634">;
def C2635 : LPUReg<2635, "%c2635">;
def C2636 : LPUReg<2636, "%c2636">;
def C2637 : LPUReg<2637, "%c2637">;
def C2638 : LPUReg<2638, "%c2638">;
def C2639 : LPUReg<2639, "%c2639">;
def C2640 : LPUReg<2640, "%c2640">;
def C2641 : LPUReg<2641, "%c2641">;
def C2642 : LPUReg<2642, "%c2642">;
def C2643 : LPUReg<2643, "%c2643">;
def C2644 : LPUReg<2644, "%c2644">;
def C2645 : LPUReg<2645, "%c2645">;
def C2646 : LPUReg<2646, "%c2646">;
def C2647 : LPUReg<2647, "%c2647">;
def C2648 : LPUReg<2648, "%c2648">;
def C2649 : LPUReg<2649, "%c2649">;
def C2650 : LPUReg<2650, "%c2650">;
def C2651 : LPUReg<2651, "%c2651">;
def C2652 : LPUReg<2652, "%c2652">;
def C2653 : LPUReg<2653, "%c2653">;
def C2654 : LPUReg<2654, "%c2654">;
def C2655 : LPUReg<2655, "%c2655">;
def C2656 : LPUReg<2656, "%c2656">;
def C2657 : LPUReg<2657, "%c2657">;
def C2658 : LPUReg<2658, "%c2658">;
def C2659 : LPUReg<2659, "%c2659">;
def C2660 : LPUReg<2660, "%c2660">;
def C2661 : LPUReg<2661, "%c2661">;
def C2662 : LPUReg<2662, "%c2662">;
def C2663 : LPUReg<2663, "%c2663">;
def C2664 : LPUReg<2664, "%c2664">;
def C2665 : LPUReg<2665, "%c2665">;
def C2666 : LPUReg<2666, "%c2666">;
def C2667 : LPUReg<2667, "%c2667">;
def C2668 : LPUReg<2668, "%c2668">;
def C2669 : LPUReg<2669, "%c2669">;
def C2670 : LPUReg<2670, "%c2670">;
def C2671 : LPUReg<2671, "%c2671">;
def C2672 : LPUReg<2672, "%c2672">;
def C2673 : LPUReg<2673, "%c2673">;
def C2674 : LPUReg<2674, "%c2674">;
def C2675 : LPUReg<2675, "%c2675">;
def C2676 : LPUReg<2676, "%c2676">;
def C2677 : LPUReg<2677, "%c2677">;
def C2678 : LPUReg<2678, "%c2678">;
def C2679 : LPUReg<2679, "%c2679">;
def C2680 : LPUReg<2680, "%c2680">;
def C2681 : LPUReg<2681, "%c2681">;
def C2682 : LPUReg<2682, "%c2682">;
def C2683 : LPUReg<2683, "%c2683">;
def C2684 : LPUReg<2684, "%c2684">;
def C2685 : LPUReg<2685, "%c2685">;
def C2686 : LPUReg<2686, "%c2686">;
def C2687 : LPUReg<2687, "%c2687">;
def C2688 : LPUReg<2688, "%c2688">;
def C2689 : LPUReg<2689, "%c2689">;
def C2690 : LPUReg<2690, "%c2690">;
def C2691 : LPUReg<2691, "%c2691">;
def C2692 : LPUReg<2692, "%c2692">;
def C2693 : LPUReg<2693, "%c2693">;
def C2694 : LPUReg<2694, "%c2694">;
def C2695 : LPUReg<2695, "%c2695">;
def C2696 : LPUReg<2696, "%c2696">;
def C2697 : LPUReg<2697, "%c2697">;
def C2698 : LPUReg<2698, "%c2698">;
def C2699 : LPUReg<2699, "%c2699">;
def C2700 : LPUReg<2700, "%c2700">;
def C2701 : LPUReg<2701, "%c2701">;
def C2702 : LPUReg<2702, "%c2702">;
def C2703 : LPUReg<2703, "%c2703">;
def C2704 : LPUReg<2704, "%c2704">;
def C2705 : LPUReg<2705, "%c2705">;
def C2706 : LPUReg<2706, "%c2706">;
def C2707 : LPUReg<2707, "%c2707">;
def C2708 : LPUReg<2708, "%c2708">;
def C2709 : LPUReg<2709, "%c2709">;
def C2710 : LPUReg<2710, "%c2710">;
def C2711 : LPUReg<2711, "%c2711">;
def C2712 : LPUReg<2712, "%c2712">;
def C2713 : LPUReg<2713, "%c2713">;
def C2714 : LPUReg<2714, "%c2714">;
def C2715 : LPUReg<2715, "%c2715">;
def C2716 : LPUReg<2716, "%c2716">;
def C2717 : LPUReg<2717, "%c2717">;
def C2718 : LPUReg<2718, "%c2718">;
def C2719 : LPUReg<2719, "%c2719">;
def C2720 : LPUReg<2720, "%c2720">;
def C2721 : LPUReg<2721, "%c2721">;
def C2722 : LPUReg<2722, "%c2722">;
def C2723 : LPUReg<2723, "%c2723">;
def C2724 : LPUReg<2724, "%c2724">;
def C2725 : LPUReg<2725, "%c2725">;
def C2726 : LPUReg<2726, "%c2726">;
def C2727 : LPUReg<2727, "%c2727">;
def C2728 : LPUReg<2728, "%c2728">;
def C2729 : LPUReg<2729, "%c2729">;
def C2730 : LPUReg<2730, "%c2730">;
def C2731 : LPUReg<2731, "%c2731">;
def C2732 : LPUReg<2732, "%c2732">;
def C2733 : LPUReg<2733, "%c2733">;
def C2734 : LPUReg<2734, "%c2734">;
def C2735 : LPUReg<2735, "%c2735">;
def C2736 : LPUReg<2736, "%c2736">;
def C2737 : LPUReg<2737, "%c2737">;
def C2738 : LPUReg<2738, "%c2738">;
def C2739 : LPUReg<2739, "%c2739">;
def C2740 : LPUReg<2740, "%c2740">;
def C2741 : LPUReg<2741, "%c2741">;
def C2742 : LPUReg<2742, "%c2742">;
def C2743 : LPUReg<2743, "%c2743">;
def C2744 : LPUReg<2744, "%c2744">;
def C2745 : LPUReg<2745, "%c2745">;
def C2746 : LPUReg<2746, "%c2746">;
def C2747 : LPUReg<2747, "%c2747">;
def C2748 : LPUReg<2748, "%c2748">;
def C2749 : LPUReg<2749, "%c2749">;
def C2750 : LPUReg<2750, "%c2750">;
def C2751 : LPUReg<2751, "%c2751">;
def C2752 : LPUReg<2752, "%c2752">;
def C2753 : LPUReg<2753, "%c2753">;
def C2754 : LPUReg<2754, "%c2754">;
def C2755 : LPUReg<2755, "%c2755">;
def C2756 : LPUReg<2756, "%c2756">;
def C2757 : LPUReg<2757, "%c2757">;
def C2758 : LPUReg<2758, "%c2758">;
def C2759 : LPUReg<2759, "%c2759">;
def C2760 : LPUReg<2760, "%c2760">;
def C2761 : LPUReg<2761, "%c2761">;
def C2762 : LPUReg<2762, "%c2762">;
def C2763 : LPUReg<2763, "%c2763">;
def C2764 : LPUReg<2764, "%c2764">;
def C2765 : LPUReg<2765, "%c2765">;
def C2766 : LPUReg<2766, "%c2766">;
def C2767 : LPUReg<2767, "%c2767">;
def C2768 : LPUReg<2768, "%c2768">;
def C2769 : LPUReg<2769, "%c2769">;
def C2770 : LPUReg<2770, "%c2770">;
def C2771 : LPUReg<2771, "%c2771">;
def C2772 : LPUReg<2772, "%c2772">;
def C2773 : LPUReg<2773, "%c2773">;
def C2774 : LPUReg<2774, "%c2774">;
def C2775 : LPUReg<2775, "%c2775">;
def C2776 : LPUReg<2776, "%c2776">;
def C2777 : LPUReg<2777, "%c2777">;
def C2778 : LPUReg<2778, "%c2778">;
def C2779 : LPUReg<2779, "%c2779">;
def C2780 : LPUReg<2780, "%c2780">;
def C2781 : LPUReg<2781, "%c2781">;
def C2782 : LPUReg<2782, "%c2782">;
def C2783 : LPUReg<2783, "%c2783">;
def C2784 : LPUReg<2784, "%c2784">;
def C2785 : LPUReg<2785, "%c2785">;
def C2786 : LPUReg<2786, "%c2786">;
def C2787 : LPUReg<2787, "%c2787">;
def C2788 : LPUReg<2788, "%c2788">;
def C2789 : LPUReg<2789, "%c2789">;
def C2790 : LPUReg<2790, "%c2790">;
def C2791 : LPUReg<2791, "%c2791">;
def C2792 : LPUReg<2792, "%c2792">;
def C2793 : LPUReg<2793, "%c2793">;
def C2794 : LPUReg<2794, "%c2794">;
def C2795 : LPUReg<2795, "%c2795">;
def C2796 : LPUReg<2796, "%c2796">;
def C2797 : LPUReg<2797, "%c2797">;
def C2798 : LPUReg<2798, "%c2798">;
def C2799 : LPUReg<2799, "%c2799">;
def C2800 : LPUReg<2800, "%c2800">;
def C2801 : LPUReg<2801, "%c2801">;
def C2802 : LPUReg<2802, "%c2802">;
def C2803 : LPUReg<2803, "%c2803">;
def C2804 : LPUReg<2804, "%c2804">;
def C2805 : LPUReg<2805, "%c2805">;
def C2806 : LPUReg<2806, "%c2806">;
def C2807 : LPUReg<2807, "%c2807">;
def C2808 : LPUReg<2808, "%c2808">;
def C2809 : LPUReg<2809, "%c2809">;
def C2810 : LPUReg<2810, "%c2810">;
def C2811 : LPUReg<2811, "%c2811">;
def C2812 : LPUReg<2812, "%c2812">;
def C2813 : LPUReg<2813, "%c2813">;
def C2814 : LPUReg<2814, "%c2814">;
def C2815 : LPUReg<2815, "%c2815">;
def C2816 : LPUReg<2816, "%c2816">;
def C2817 : LPUReg<2817, "%c2817">;
def C2818 : LPUReg<2818, "%c2818">;
def C2819 : LPUReg<2819, "%c2819">;
def C2820 : LPUReg<2820, "%c2820">;
def C2821 : LPUReg<2821, "%c2821">;
def C2822 : LPUReg<2822, "%c2822">;
def C2823 : LPUReg<2823, "%c2823">;
def C2824 : LPUReg<2824, "%c2824">;
def C2825 : LPUReg<2825, "%c2825">;
def C2826 : LPUReg<2826, "%c2826">;
def C2827 : LPUReg<2827, "%c2827">;
def C2828 : LPUReg<2828, "%c2828">;
def C2829 : LPUReg<2829, "%c2829">;
def C2830 : LPUReg<2830, "%c2830">;
def C2831 : LPUReg<2831, "%c2831">;
def C2832 : LPUReg<2832, "%c2832">;
def C2833 : LPUReg<2833, "%c2833">;
def C2834 : LPUReg<2834, "%c2834">;
def C2835 : LPUReg<2835, "%c2835">;
def C2836 : LPUReg<2836, "%c2836">;
def C2837 : LPUReg<2837, "%c2837">;
def C2838 : LPUReg<2838, "%c2838">;
def C2839 : LPUReg<2839, "%c2839">;
def C2840 : LPUReg<2840, "%c2840">;
def C2841 : LPUReg<2841, "%c2841">;
def C2842 : LPUReg<2842, "%c2842">;
def C2843 : LPUReg<2843, "%c2843">;
def C2844 : LPUReg<2844, "%c2844">;
def C2845 : LPUReg<2845, "%c2845">;
def C2846 : LPUReg<2846, "%c2846">;
def C2847 : LPUReg<2847, "%c2847">;
def C2848 : LPUReg<2848, "%c2848">;
def C2849 : LPUReg<2849, "%c2849">;
def C2850 : LPUReg<2850, "%c2850">;
def C2851 : LPUReg<2851, "%c2851">;
def C2852 : LPUReg<2852, "%c2852">;
def C2853 : LPUReg<2853, "%c2853">;
def C2854 : LPUReg<2854, "%c2854">;
def C2855 : LPUReg<2855, "%c2855">;
def C2856 : LPUReg<2856, "%c2856">;
def C2857 : LPUReg<2857, "%c2857">;
def C2858 : LPUReg<2858, "%c2858">;
def C2859 : LPUReg<2859, "%c2859">;
def C2860 : LPUReg<2860, "%c2860">;
def C2861 : LPUReg<2861, "%c2861">;
def C2862 : LPUReg<2862, "%c2862">;
def C2863 : LPUReg<2863, "%c2863">;
def C2864 : LPUReg<2864, "%c2864">;
def C2865 : LPUReg<2865, "%c2865">;
def C2866 : LPUReg<2866, "%c2866">;
def C2867 : LPUReg<2867, "%c2867">;
def C2868 : LPUReg<2868, "%c2868">;
def C2869 : LPUReg<2869, "%c2869">;
def C2870 : LPUReg<2870, "%c2870">;
def C2871 : LPUReg<2871, "%c2871">;
def C2872 : LPUReg<2872, "%c2872">;
def C2873 : LPUReg<2873, "%c2873">;
def C2874 : LPUReg<2874, "%c2874">;
def C2875 : LPUReg<2875, "%c2875">;
def C2876 : LPUReg<2876, "%c2876">;
def C2877 : LPUReg<2877, "%c2877">;
def C2878 : LPUReg<2878, "%c2878">;
def C2879 : LPUReg<2879, "%c2879">;
def C2880 : LPUReg<2880, "%c2880">;
def C2881 : LPUReg<2881, "%c2881">;
def C2882 : LPUReg<2882, "%c2882">;
def C2883 : LPUReg<2883, "%c2883">;
def C2884 : LPUReg<2884, "%c2884">;
def C2885 : LPUReg<2885, "%c2885">;
def C2886 : LPUReg<2886, "%c2886">;
def C2887 : LPUReg<2887, "%c2887">;
def C2888 : LPUReg<2888, "%c2888">;
def C2889 : LPUReg<2889, "%c2889">;
def C2890 : LPUReg<2890, "%c2890">;
def C2891 : LPUReg<2891, "%c2891">;
def C2892 : LPUReg<2892, "%c2892">;
def C2893 : LPUReg<2893, "%c2893">;
def C2894 : LPUReg<2894, "%c2894">;
def C2895 : LPUReg<2895, "%c2895">;
def C2896 : LPUReg<2896, "%c2896">;
def C2897 : LPUReg<2897, "%c2897">;
def C2898 : LPUReg<2898, "%c2898">;
def C2899 : LPUReg<2899, "%c2899">;
def C2900 : LPUReg<2900, "%c2900">;
def C2901 : LPUReg<2901, "%c2901">;
def C2902 : LPUReg<2902, "%c2902">;
def C2903 : LPUReg<2903, "%c2903">;
def C2904 : LPUReg<2904, "%c2904">;
def C2905 : LPUReg<2905, "%c2905">;
def C2906 : LPUReg<2906, "%c2906">;
def C2907 : LPUReg<2907, "%c2907">;
def C2908 : LPUReg<2908, "%c2908">;
def C2909 : LPUReg<2909, "%c2909">;
def C2910 : LPUReg<2910, "%c2910">;
def C2911 : LPUReg<2911, "%c2911">;
def C2912 : LPUReg<2912, "%c2912">;
def C2913 : LPUReg<2913, "%c2913">;
def C2914 : LPUReg<2914, "%c2914">;
def C2915 : LPUReg<2915, "%c2915">;
def C2916 : LPUReg<2916, "%c2916">;
def C2917 : LPUReg<2917, "%c2917">;
def C2918 : LPUReg<2918, "%c2918">;
def C2919 : LPUReg<2919, "%c2919">;
def C2920 : LPUReg<2920, "%c2920">;
def C2921 : LPUReg<2921, "%c2921">;
def C2922 : LPUReg<2922, "%c2922">;
def C2923 : LPUReg<2923, "%c2923">;
def C2924 : LPUReg<2924, "%c2924">;
def C2925 : LPUReg<2925, "%c2925">;
def C2926 : LPUReg<2926, "%c2926">;
def C2927 : LPUReg<2927, "%c2927">;
def C2928 : LPUReg<2928, "%c2928">;
def C2929 : LPUReg<2929, "%c2929">;
def C2930 : LPUReg<2930, "%c2930">;
def C2931 : LPUReg<2931, "%c2931">;
def C2932 : LPUReg<2932, "%c2932">;
def C2933 : LPUReg<2933, "%c2933">;
def C2934 : LPUReg<2934, "%c2934">;
def C2935 : LPUReg<2935, "%c2935">;
def C2936 : LPUReg<2936, "%c2936">;
def C2937 : LPUReg<2937, "%c2937">;
def C2938 : LPUReg<2938, "%c2938">;
def C2939 : LPUReg<2939, "%c2939">;
def C2940 : LPUReg<2940, "%c2940">;
def C2941 : LPUReg<2941, "%c2941">;
def C2942 : LPUReg<2942, "%c2942">;
def C2943 : LPUReg<2943, "%c2943">;
def C2944 : LPUReg<2944, "%c2944">;
def C2945 : LPUReg<2945, "%c2945">;
def C2946 : LPUReg<2946, "%c2946">;
def C2947 : LPUReg<2947, "%c2947">;
def C2948 : LPUReg<2948, "%c2948">;
def C2949 : LPUReg<2949, "%c2949">;
def C2950 : LPUReg<2950, "%c2950">;
def C2951 : LPUReg<2951, "%c2951">;
def C2952 : LPUReg<2952, "%c2952">;
def C2953 : LPUReg<2953, "%c2953">;
def C2954 : LPUReg<2954, "%c2954">;
def C2955 : LPUReg<2955, "%c2955">;
def C2956 : LPUReg<2956, "%c2956">;
def C2957 : LPUReg<2957, "%c2957">;
def C2958 : LPUReg<2958, "%c2958">;
def C2959 : LPUReg<2959, "%c2959">;
def C2960 : LPUReg<2960, "%c2960">;
def C2961 : LPUReg<2961, "%c2961">;
def C2962 : LPUReg<2962, "%c2962">;
def C2963 : LPUReg<2963, "%c2963">;
def C2964 : LPUReg<2964, "%c2964">;
def C2965 : LPUReg<2965, "%c2965">;
def C2966 : LPUReg<2966, "%c2966">;
def C2967 : LPUReg<2967, "%c2967">;
def C2968 : LPUReg<2968, "%c2968">;
def C2969 : LPUReg<2969, "%c2969">;
def C2970 : LPUReg<2970, "%c2970">;
def C2971 : LPUReg<2971, "%c2971">;
def C2972 : LPUReg<2972, "%c2972">;
def C2973 : LPUReg<2973, "%c2973">;
def C2974 : LPUReg<2974, "%c2974">;
def C2975 : LPUReg<2975, "%c2975">;
def C2976 : LPUReg<2976, "%c2976">;
def C2977 : LPUReg<2977, "%c2977">;
def C2978 : LPUReg<2978, "%c2978">;
def C2979 : LPUReg<2979, "%c2979">;
def C2980 : LPUReg<2980, "%c2980">;
def C2981 : LPUReg<2981, "%c2981">;
def C2982 : LPUReg<2982, "%c2982">;
def C2983 : LPUReg<2983, "%c2983">;
def C2984 : LPUReg<2984, "%c2984">;
def C2985 : LPUReg<2985, "%c2985">;
def C2986 : LPUReg<2986, "%c2986">;
def C2987 : LPUReg<2987, "%c2987">;
def C2988 : LPUReg<2988, "%c2988">;
def C2989 : LPUReg<2989, "%c2989">;
def C2990 : LPUReg<2990, "%c2990">;
def C2991 : LPUReg<2991, "%c2991">;
def C2992 : LPUReg<2992, "%c2992">;
def C2993 : LPUReg<2993, "%c2993">;
def C2994 : LPUReg<2994, "%c2994">;
def C2995 : LPUReg<2995, "%c2995">;
def C2996 : LPUReg<2996, "%c2996">;
def C2997 : LPUReg<2997, "%c2997">;
def C2998 : LPUReg<2998, "%c2998">;
def C2999 : LPUReg<2999, "%c2999">;
def C3000 : LPUReg<3000, "%c3000">;
def C3001 : LPUReg<3001, "%c3001">;
def C3002 : LPUReg<3002, "%c3002">;
def C3003 : LPUReg<3003, "%c3003">;
def C3004 : LPUReg<3004, "%c3004">;
def C3005 : LPUReg<3005, "%c3005">;
def C3006 : LPUReg<3006, "%c3006">;
def C3007 : LPUReg<3007, "%c3007">;
def C3008 : LPUReg<3008, "%c3008">;
def C3009 : LPUReg<3009, "%c3009">;
def C3010 : LPUReg<3010, "%c3010">;
def C3011 : LPUReg<3011, "%c3011">;
def C3012 : LPUReg<3012, "%c3012">;
def C3013 : LPUReg<3013, "%c3013">;
def C3014 : LPUReg<3014, "%c3014">;
def C3015 : LPUReg<3015, "%c3015">;
def C3016 : LPUReg<3016, "%c3016">;
def C3017 : LPUReg<3017, "%c3017">;
def C3018 : LPUReg<3018, "%c3018">;
def C3019 : LPUReg<3019, "%c3019">;
def C3020 : LPUReg<3020, "%c3020">;
def C3021 : LPUReg<3021, "%c3021">;
def C3022 : LPUReg<3022, "%c3022">;
def C3023 : LPUReg<3023, "%c3023">;
def C3024 : LPUReg<3024, "%c3024">;
def C3025 : LPUReg<3025, "%c3025">;
def C3026 : LPUReg<3026, "%c3026">;
def C3027 : LPUReg<3027, "%c3027">;
def C3028 : LPUReg<3028, "%c3028">;
def C3029 : LPUReg<3029, "%c3029">;
def C3030 : LPUReg<3030, "%c3030">;
def C3031 : LPUReg<3031, "%c3031">;
def C3032 : LPUReg<3032, "%c3032">;
def C3033 : LPUReg<3033, "%c3033">;
def C3034 : LPUReg<3034, "%c3034">;
def C3035 : LPUReg<3035, "%c3035">;
def C3036 : LPUReg<3036, "%c3036">;
def C3037 : LPUReg<3037, "%c3037">;
def C3038 : LPUReg<3038, "%c3038">;
def C3039 : LPUReg<3039, "%c3039">;
def C3040 : LPUReg<3040, "%c3040">;
def C3041 : LPUReg<3041, "%c3041">;
def C3042 : LPUReg<3042, "%c3042">;
def C3043 : LPUReg<3043, "%c3043">;
def C3044 : LPUReg<3044, "%c3044">;
def C3045 : LPUReg<3045, "%c3045">;
def C3046 : LPUReg<3046, "%c3046">;
def C3047 : LPUReg<3047, "%c3047">;
def C3048 : LPUReg<3048, "%c3048">;
def C3049 : LPUReg<3049, "%c3049">;
def C3050 : LPUReg<3050, "%c3050">;
def C3051 : LPUReg<3051, "%c3051">;
def C3052 : LPUReg<3052, "%c3052">;
def C3053 : LPUReg<3053, "%c3053">;
def C3054 : LPUReg<3054, "%c3054">;
def C3055 : LPUReg<3055, "%c3055">;
def C3056 : LPUReg<3056, "%c3056">;
def C3057 : LPUReg<3057, "%c3057">;
def C3058 : LPUReg<3058, "%c3058">;
def C3059 : LPUReg<3059, "%c3059">;
def C3060 : LPUReg<3060, "%c3060">;
def C3061 : LPUReg<3061, "%c3061">;
def C3062 : LPUReg<3062, "%c3062">;
def C3063 : LPUReg<3063, "%c3063">;
def C3064 : LPUReg<3064, "%c3064">;
def C3065 : LPUReg<3065, "%c3065">;
def C3066 : LPUReg<3066, "%c3066">;
def C3067 : LPUReg<3067, "%c3067">;
def C3068 : LPUReg<3068, "%c3068">;
def C3069 : LPUReg<3069, "%c3069">;
def C3070 : LPUReg<3070, "%c3070">;
def C3071 : LPUReg<3071, "%c3071">;
def C3072 : LPUReg<3072, "%c3072">;
def C3073 : LPUReg<3073, "%c3073">;
def C3074 : LPUReg<3074, "%c3074">;
def C3075 : LPUReg<3075, "%c3075">;
def C3076 : LPUReg<3076, "%c3076">;
def C3077 : LPUReg<3077, "%c3077">;
def C3078 : LPUReg<3078, "%c3078">;
def C3079 : LPUReg<3079, "%c3079">;
def C3080 : LPUReg<3080, "%c3080">;
def C3081 : LPUReg<3081, "%c3081">;
def C3082 : LPUReg<3082, "%c3082">;
def C3083 : LPUReg<3083, "%c3083">;
def C3084 : LPUReg<3084, "%c3084">;
def C3085 : LPUReg<3085, "%c3085">;
def C3086 : LPUReg<3086, "%c3086">;
def C3087 : LPUReg<3087, "%c3087">;
def C3088 : LPUReg<3088, "%c3088">;
def C3089 : LPUReg<3089, "%c3089">;
def C3090 : LPUReg<3090, "%c3090">;
def C3091 : LPUReg<3091, "%c3091">;
def C3092 : LPUReg<3092, "%c3092">;
def C3093 : LPUReg<3093, "%c3093">;
def C3094 : LPUReg<3094, "%c3094">;
def C3095 : LPUReg<3095, "%c3095">;
def C3096 : LPUReg<3096, "%c3096">;
def C3097 : LPUReg<3097, "%c3097">;
def C3098 : LPUReg<3098, "%c3098">;
def C3099 : LPUReg<3099, "%c3099">;
def C3100 : LPUReg<3100, "%c3100">;
def C3101 : LPUReg<3101, "%c3101">;
def C3102 : LPUReg<3102, "%c3102">;
def C3103 : LPUReg<3103, "%c3103">;
def C3104 : LPUReg<3104, "%c3104">;
def C3105 : LPUReg<3105, "%c3105">;
def C3106 : LPUReg<3106, "%c3106">;
def C3107 : LPUReg<3107, "%c3107">;
def C3108 : LPUReg<3108, "%c3108">;
def C3109 : LPUReg<3109, "%c3109">;
def C3110 : LPUReg<3110, "%c3110">;
def C3111 : LPUReg<3111, "%c3111">;
def C3112 : LPUReg<3112, "%c3112">;
def C3113 : LPUReg<3113, "%c3113">;
def C3114 : LPUReg<3114, "%c3114">;
def C3115 : LPUReg<3115, "%c3115">;
def C3116 : LPUReg<3116, "%c3116">;
def C3117 : LPUReg<3117, "%c3117">;
def C3118 : LPUReg<3118, "%c3118">;
def C3119 : LPUReg<3119, "%c3119">;
def C3120 : LPUReg<3120, "%c3120">;
def C3121 : LPUReg<3121, "%c3121">;
def C3122 : LPUReg<3122, "%c3122">;
def C3123 : LPUReg<3123, "%c3123">;
def C3124 : LPUReg<3124, "%c3124">;
def C3125 : LPUReg<3125, "%c3125">;
def C3126 : LPUReg<3126, "%c3126">;
def C3127 : LPUReg<3127, "%c3127">;
def C3128 : LPUReg<3128, "%c3128">;
def C3129 : LPUReg<3129, "%c3129">;
def C3130 : LPUReg<3130, "%c3130">;
def C3131 : LPUReg<3131, "%c3131">;
def C3132 : LPUReg<3132, "%c3132">;
def C3133 : LPUReg<3133, "%c3133">;
def C3134 : LPUReg<3134, "%c3134">;
def C3135 : LPUReg<3135, "%c3135">;
def C3136 : LPUReg<3136, "%c3136">;
def C3137 : LPUReg<3137, "%c3137">;
def C3138 : LPUReg<3138, "%c3138">;
def C3139 : LPUReg<3139, "%c3139">;
def C3140 : LPUReg<3140, "%c3140">;
def C3141 : LPUReg<3141, "%c3141">;
def C3142 : LPUReg<3142, "%c3142">;
def C3143 : LPUReg<3143, "%c3143">;
def C3144 : LPUReg<3144, "%c3144">;
def C3145 : LPUReg<3145, "%c3145">;
def C3146 : LPUReg<3146, "%c3146">;
def C3147 : LPUReg<3147, "%c3147">;
def C3148 : LPUReg<3148, "%c3148">;
def C3149 : LPUReg<3149, "%c3149">;
def C3150 : LPUReg<3150, "%c3150">;
def C3151 : LPUReg<3151, "%c3151">;
def C3152 : LPUReg<3152, "%c3152">;
def C3153 : LPUReg<3153, "%c3153">;
def C3154 : LPUReg<3154, "%c3154">;
def C3155 : LPUReg<3155, "%c3155">;
def C3156 : LPUReg<3156, "%c3156">;
def C3157 : LPUReg<3157, "%c3157">;
def C3158 : LPUReg<3158, "%c3158">;
def C3159 : LPUReg<3159, "%c3159">;
def C3160 : LPUReg<3160, "%c3160">;
def C3161 : LPUReg<3161, "%c3161">;
def C3162 : LPUReg<3162, "%c3162">;
def C3163 : LPUReg<3163, "%c3163">;
def C3164 : LPUReg<3164, "%c3164">;
def C3165 : LPUReg<3165, "%c3165">;
def C3166 : LPUReg<3166, "%c3166">;
def C3167 : LPUReg<3167, "%c3167">;
def C3168 : LPUReg<3168, "%c3168">;
def C3169 : LPUReg<3169, "%c3169">;
def C3170 : LPUReg<3170, "%c3170">;
def C3171 : LPUReg<3171, "%c3171">;
def C3172 : LPUReg<3172, "%c3172">;
def C3173 : LPUReg<3173, "%c3173">;
def C3174 : LPUReg<3174, "%c3174">;
def C3175 : LPUReg<3175, "%c3175">;
def C3176 : LPUReg<3176, "%c3176">;
def C3177 : LPUReg<3177, "%c3177">;
def C3178 : LPUReg<3178, "%c3178">;
def C3179 : LPUReg<3179, "%c3179">;
def C3180 : LPUReg<3180, "%c3180">;
def C3181 : LPUReg<3181, "%c3181">;
def C3182 : LPUReg<3182, "%c3182">;
def C3183 : LPUReg<3183, "%c3183">;
def C3184 : LPUReg<3184, "%c3184">;
def C3185 : LPUReg<3185, "%c3185">;
def C3186 : LPUReg<3186, "%c3186">;
def C3187 : LPUReg<3187, "%c3187">;
def C3188 : LPUReg<3188, "%c3188">;
def C3189 : LPUReg<3189, "%c3189">;
def C3190 : LPUReg<3190, "%c3190">;
def C3191 : LPUReg<3191, "%c3191">;
def C3192 : LPUReg<3192, "%c3192">;
def C3193 : LPUReg<3193, "%c3193">;
def C3194 : LPUReg<3194, "%c3194">;
def C3195 : LPUReg<3195, "%c3195">;
def C3196 : LPUReg<3196, "%c3196">;
def C3197 : LPUReg<3197, "%c3197">;
def C3198 : LPUReg<3198, "%c3198">;
def C3199 : LPUReg<3199, "%c3199">;
def C3200 : LPUReg<3200, "%c3200">;
def C3201 : LPUReg<3201, "%c3201">;
def C3202 : LPUReg<3202, "%c3202">;
def C3203 : LPUReg<3203, "%c3203">;
def C3204 : LPUReg<3204, "%c3204">;
def C3205 : LPUReg<3205, "%c3205">;
def C3206 : LPUReg<3206, "%c3206">;
def C3207 : LPUReg<3207, "%c3207">;
def C3208 : LPUReg<3208, "%c3208">;
def C3209 : LPUReg<3209, "%c3209">;
def C3210 : LPUReg<3210, "%c3210">;
def C3211 : LPUReg<3211, "%c3211">;
def C3212 : LPUReg<3212, "%c3212">;
def C3213 : LPUReg<3213, "%c3213">;
def C3214 : LPUReg<3214, "%c3214">;
def C3215 : LPUReg<3215, "%c3215">;
def C3216 : LPUReg<3216, "%c3216">;
def C3217 : LPUReg<3217, "%c3217">;
def C3218 : LPUReg<3218, "%c3218">;
def C3219 : LPUReg<3219, "%c3219">;
def C3220 : LPUReg<3220, "%c3220">;
def C3221 : LPUReg<3221, "%c3221">;
def C3222 : LPUReg<3222, "%c3222">;
def C3223 : LPUReg<3223, "%c3223">;
def C3224 : LPUReg<3224, "%c3224">;
def C3225 : LPUReg<3225, "%c3225">;
def C3226 : LPUReg<3226, "%c3226">;
def C3227 : LPUReg<3227, "%c3227">;
def C3228 : LPUReg<3228, "%c3228">;
def C3229 : LPUReg<3229, "%c3229">;
def C3230 : LPUReg<3230, "%c3230">;
def C3231 : LPUReg<3231, "%c3231">;
def C3232 : LPUReg<3232, "%c3232">;
def C3233 : LPUReg<3233, "%c3233">;
def C3234 : LPUReg<3234, "%c3234">;
def C3235 : LPUReg<3235, "%c3235">;
def C3236 : LPUReg<3236, "%c3236">;
def C3237 : LPUReg<3237, "%c3237">;
def C3238 : LPUReg<3238, "%c3238">;
def C3239 : LPUReg<3239, "%c3239">;
def C3240 : LPUReg<3240, "%c3240">;
def C3241 : LPUReg<3241, "%c3241">;
def C3242 : LPUReg<3242, "%c3242">;
def C3243 : LPUReg<3243, "%c3243">;
def C3244 : LPUReg<3244, "%c3244">;
def C3245 : LPUReg<3245, "%c3245">;
def C3246 : LPUReg<3246, "%c3246">;
def C3247 : LPUReg<3247, "%c3247">;
def C3248 : LPUReg<3248, "%c3248">;
def C3249 : LPUReg<3249, "%c3249">;
def C3250 : LPUReg<3250, "%c3250">;
def C3251 : LPUReg<3251, "%c3251">;
def C3252 : LPUReg<3252, "%c3252">;
def C3253 : LPUReg<3253, "%c3253">;
def C3254 : LPUReg<3254, "%c3254">;
def C3255 : LPUReg<3255, "%c3255">;
def C3256 : LPUReg<3256, "%c3256">;
def C3257 : LPUReg<3257, "%c3257">;
def C3258 : LPUReg<3258, "%c3258">;
def C3259 : LPUReg<3259, "%c3259">;
def C3260 : LPUReg<3260, "%c3260">;
def C3261 : LPUReg<3261, "%c3261">;
def C3262 : LPUReg<3262, "%c3262">;
def C3263 : LPUReg<3263, "%c3263">;
def C3264 : LPUReg<3264, "%c3264">;
def C3265 : LPUReg<3265, "%c3265">;
def C3266 : LPUReg<3266, "%c3266">;
def C3267 : LPUReg<3267, "%c3267">;
def C3268 : LPUReg<3268, "%c3268">;
def C3269 : LPUReg<3269, "%c3269">;
def C3270 : LPUReg<3270, "%c3270">;
def C3271 : LPUReg<3271, "%c3271">;
def C3272 : LPUReg<3272, "%c3272">;
def C3273 : LPUReg<3273, "%c3273">;
def C3274 : LPUReg<3274, "%c3274">;
def C3275 : LPUReg<3275, "%c3275">;
def C3276 : LPUReg<3276, "%c3276">;
def C3277 : LPUReg<3277, "%c3277">;
def C3278 : LPUReg<3278, "%c3278">;
def C3279 : LPUReg<3279, "%c3279">;
def C3280 : LPUReg<3280, "%c3280">;
def C3281 : LPUReg<3281, "%c3281">;
def C3282 : LPUReg<3282, "%c3282">;
def C3283 : LPUReg<3283, "%c3283">;
def C3284 : LPUReg<3284, "%c3284">;
def C3285 : LPUReg<3285, "%c3285">;
def C3286 : LPUReg<3286, "%c3286">;
def C3287 : LPUReg<3287, "%c3287">;
def C3288 : LPUReg<3288, "%c3288">;
def C3289 : LPUReg<3289, "%c3289">;
def C3290 : LPUReg<3290, "%c3290">;
def C3291 : LPUReg<3291, "%c3291">;
def C3292 : LPUReg<3292, "%c3292">;
def C3293 : LPUReg<3293, "%c3293">;
def C3294 : LPUReg<3294, "%c3294">;
def C3295 : LPUReg<3295, "%c3295">;
def C3296 : LPUReg<3296, "%c3296">;
def C3297 : LPUReg<3297, "%c3297">;
def C3298 : LPUReg<3298, "%c3298">;
def C3299 : LPUReg<3299, "%c3299">;
def C3300 : LPUReg<3300, "%c3300">;
def C3301 : LPUReg<3301, "%c3301">;
def C3302 : LPUReg<3302, "%c3302">;
def C3303 : LPUReg<3303, "%c3303">;
def C3304 : LPUReg<3304, "%c3304">;
def C3305 : LPUReg<3305, "%c3305">;
def C3306 : LPUReg<3306, "%c3306">;
def C3307 : LPUReg<3307, "%c3307">;
def C3308 : LPUReg<3308, "%c3308">;
def C3309 : LPUReg<3309, "%c3309">;
def C3310 : LPUReg<3310, "%c3310">;
def C3311 : LPUReg<3311, "%c3311">;
def C3312 : LPUReg<3312, "%c3312">;
def C3313 : LPUReg<3313, "%c3313">;
def C3314 : LPUReg<3314, "%c3314">;
def C3315 : LPUReg<3315, "%c3315">;
def C3316 : LPUReg<3316, "%c3316">;
def C3317 : LPUReg<3317, "%c3317">;
def C3318 : LPUReg<3318, "%c3318">;
def C3319 : LPUReg<3319, "%c3319">;
def C3320 : LPUReg<3320, "%c3320">;
def C3321 : LPUReg<3321, "%c3321">;
def C3322 : LPUReg<3322, "%c3322">;
def C3323 : LPUReg<3323, "%c3323">;
def C3324 : LPUReg<3324, "%c3324">;
def C3325 : LPUReg<3325, "%c3325">;
def C3326 : LPUReg<3326, "%c3326">;
def C3327 : LPUReg<3327, "%c3327">;
def C3328 : LPUReg<3328, "%c3328">;
def C3329 : LPUReg<3329, "%c3329">;
def C3330 : LPUReg<3330, "%c3330">;
def C3331 : LPUReg<3331, "%c3331">;
def C3332 : LPUReg<3332, "%c3332">;
def C3333 : LPUReg<3333, "%c3333">;
def C3334 : LPUReg<3334, "%c3334">;
def C3335 : LPUReg<3335, "%c3335">;
def C3336 : LPUReg<3336, "%c3336">;
def C3337 : LPUReg<3337, "%c3337">;
def C3338 : LPUReg<3338, "%c3338">;
def C3339 : LPUReg<3339, "%c3339">;
def C3340 : LPUReg<3340, "%c3340">;
def C3341 : LPUReg<3341, "%c3341">;
def C3342 : LPUReg<3342, "%c3342">;
def C3343 : LPUReg<3343, "%c3343">;
def C3344 : LPUReg<3344, "%c3344">;
def C3345 : LPUReg<3345, "%c3345">;
def C3346 : LPUReg<3346, "%c3346">;
def C3347 : LPUReg<3347, "%c3347">;
def C3348 : LPUReg<3348, "%c3348">;
def C3349 : LPUReg<3349, "%c3349">;
def C3350 : LPUReg<3350, "%c3350">;
def C3351 : LPUReg<3351, "%c3351">;
def C3352 : LPUReg<3352, "%c3352">;
def C3353 : LPUReg<3353, "%c3353">;
def C3354 : LPUReg<3354, "%c3354">;
def C3355 : LPUReg<3355, "%c3355">;
def C3356 : LPUReg<3356, "%c3356">;
def C3357 : LPUReg<3357, "%c3357">;
def C3358 : LPUReg<3358, "%c3358">;
def C3359 : LPUReg<3359, "%c3359">;
def C3360 : LPUReg<3360, "%c3360">;
def C3361 : LPUReg<3361, "%c3361">;
def C3362 : LPUReg<3362, "%c3362">;
def C3363 : LPUReg<3363, "%c3363">;
def C3364 : LPUReg<3364, "%c3364">;
def C3365 : LPUReg<3365, "%c3365">;
def C3366 : LPUReg<3366, "%c3366">;
def C3367 : LPUReg<3367, "%c3367">;
def C3368 : LPUReg<3368, "%c3368">;
def C3369 : LPUReg<3369, "%c3369">;
def C3370 : LPUReg<3370, "%c3370">;
def C3371 : LPUReg<3371, "%c3371">;
def C3372 : LPUReg<3372, "%c3372">;
def C3373 : LPUReg<3373, "%c3373">;
def C3374 : LPUReg<3374, "%c3374">;
def C3375 : LPUReg<3375, "%c3375">;
def C3376 : LPUReg<3376, "%c3376">;
def C3377 : LPUReg<3377, "%c3377">;
def C3378 : LPUReg<3378, "%c3378">;
def C3379 : LPUReg<3379, "%c3379">;
def C3380 : LPUReg<3380, "%c3380">;
def C3381 : LPUReg<3381, "%c3381">;
def C3382 : LPUReg<3382, "%c3382">;
def C3383 : LPUReg<3383, "%c3383">;
def C3384 : LPUReg<3384, "%c3384">;
def C3385 : LPUReg<3385, "%c3385">;
def C3386 : LPUReg<3386, "%c3386">;
def C3387 : LPUReg<3387, "%c3387">;
def C3388 : LPUReg<3388, "%c3388">;
def C3389 : LPUReg<3389, "%c3389">;
def C3390 : LPUReg<3390, "%c3390">;
def C3391 : LPUReg<3391, "%c3391">;
def C3392 : LPUReg<3392, "%c3392">;
def C3393 : LPUReg<3393, "%c3393">;
def C3394 : LPUReg<3394, "%c3394">;
def C3395 : LPUReg<3395, "%c3395">;
def C3396 : LPUReg<3396, "%c3396">;
def C3397 : LPUReg<3397, "%c3397">;
def C3398 : LPUReg<3398, "%c3398">;
def C3399 : LPUReg<3399, "%c3399">;
def C3400 : LPUReg<3400, "%c3400">;
def C3401 : LPUReg<3401, "%c3401">;
def C3402 : LPUReg<3402, "%c3402">;
def C3403 : LPUReg<3403, "%c3403">;
def C3404 : LPUReg<3404, "%c3404">;
def C3405 : LPUReg<3405, "%c3405">;
def C3406 : LPUReg<3406, "%c3406">;
def C3407 : LPUReg<3407, "%c3407">;
def C3408 : LPUReg<3408, "%c3408">;
def C3409 : LPUReg<3409, "%c3409">;
def C3410 : LPUReg<3410, "%c3410">;
def C3411 : LPUReg<3411, "%c3411">;
def C3412 : LPUReg<3412, "%c3412">;
def C3413 : LPUReg<3413, "%c3413">;
def C3414 : LPUReg<3414, "%c3414">;
def C3415 : LPUReg<3415, "%c3415">;
def C3416 : LPUReg<3416, "%c3416">;
def C3417 : LPUReg<3417, "%c3417">;
def C3418 : LPUReg<3418, "%c3418">;
def C3419 : LPUReg<3419, "%c3419">;
def C3420 : LPUReg<3420, "%c3420">;
def C3421 : LPUReg<3421, "%c3421">;
def C3422 : LPUReg<3422, "%c3422">;
def C3423 : LPUReg<3423, "%c3423">;
def C3424 : LPUReg<3424, "%c3424">;
def C3425 : LPUReg<3425, "%c3425">;
def C3426 : LPUReg<3426, "%c3426">;
def C3427 : LPUReg<3427, "%c3427">;
def C3428 : LPUReg<3428, "%c3428">;
def C3429 : LPUReg<3429, "%c3429">;
def C3430 : LPUReg<3430, "%c3430">;
def C3431 : LPUReg<3431, "%c3431">;
def C3432 : LPUReg<3432, "%c3432">;
def C3433 : LPUReg<3433, "%c3433">;
def C3434 : LPUReg<3434, "%c3434">;
def C3435 : LPUReg<3435, "%c3435">;
def C3436 : LPUReg<3436, "%c3436">;
def C3437 : LPUReg<3437, "%c3437">;
def C3438 : LPUReg<3438, "%c3438">;
def C3439 : LPUReg<3439, "%c3439">;
def C3440 : LPUReg<3440, "%c3440">;
def C3441 : LPUReg<3441, "%c3441">;
def C3442 : LPUReg<3442, "%c3442">;
def C3443 : LPUReg<3443, "%c3443">;
def C3444 : LPUReg<3444, "%c3444">;
def C3445 : LPUReg<3445, "%c3445">;
def C3446 : LPUReg<3446, "%c3446">;
def C3447 : LPUReg<3447, "%c3447">;
def C3448 : LPUReg<3448, "%c3448">;
def C3449 : LPUReg<3449, "%c3449">;
def C3450 : LPUReg<3450, "%c3450">;
def C3451 : LPUReg<3451, "%c3451">;
def C3452 : LPUReg<3452, "%c3452">;
def C3453 : LPUReg<3453, "%c3453">;
def C3454 : LPUReg<3454, "%c3454">;
def C3455 : LPUReg<3455, "%c3455">;
def C3456 : LPUReg<3456, "%c3456">;
def C3457 : LPUReg<3457, "%c3457">;
def C3458 : LPUReg<3458, "%c3458">;
def C3459 : LPUReg<3459, "%c3459">;
def C3460 : LPUReg<3460, "%c3460">;
def C3461 : LPUReg<3461, "%c3461">;
def C3462 : LPUReg<3462, "%c3462">;
def C3463 : LPUReg<3463, "%c3463">;
def C3464 : LPUReg<3464, "%c3464">;
def C3465 : LPUReg<3465, "%c3465">;
def C3466 : LPUReg<3466, "%c3466">;
def C3467 : LPUReg<3467, "%c3467">;
def C3468 : LPUReg<3468, "%c3468">;
def C3469 : LPUReg<3469, "%c3469">;
def C3470 : LPUReg<3470, "%c3470">;
def C3471 : LPUReg<3471, "%c3471">;
def C3472 : LPUReg<3472, "%c3472">;
def C3473 : LPUReg<3473, "%c3473">;
def C3474 : LPUReg<3474, "%c3474">;
def C3475 : LPUReg<3475, "%c3475">;
def C3476 : LPUReg<3476, "%c3476">;
def C3477 : LPUReg<3477, "%c3477">;
def C3478 : LPUReg<3478, "%c3478">;
def C3479 : LPUReg<3479, "%c3479">;
def C3480 : LPUReg<3480, "%c3480">;
def C3481 : LPUReg<3481, "%c3481">;
def C3482 : LPUReg<3482, "%c3482">;
def C3483 : LPUReg<3483, "%c3483">;
def C3484 : LPUReg<3484, "%c3484">;
def C3485 : LPUReg<3485, "%c3485">;
def C3486 : LPUReg<3486, "%c3486">;
def C3487 : LPUReg<3487, "%c3487">;
def C3488 : LPUReg<3488, "%c3488">;
def C3489 : LPUReg<3489, "%c3489">;
def C3490 : LPUReg<3490, "%c3490">;
def C3491 : LPUReg<3491, "%c3491">;
def C3492 : LPUReg<3492, "%c3492">;
def C3493 : LPUReg<3493, "%c3493">;
def C3494 : LPUReg<3494, "%c3494">;
def C3495 : LPUReg<3495, "%c3495">;
def C3496 : LPUReg<3496, "%c3496">;
def C3497 : LPUReg<3497, "%c3497">;
def C3498 : LPUReg<3498, "%c3498">;
def C3499 : LPUReg<3499, "%c3499">;
def C3500 : LPUReg<3500, "%c3500">;
def C3501 : LPUReg<3501, "%c3501">;
def C3502 : LPUReg<3502, "%c3502">;
def C3503 : LPUReg<3503, "%c3503">;
def C3504 : LPUReg<3504, "%c3504">;
def C3505 : LPUReg<3505, "%c3505">;
def C3506 : LPUReg<3506, "%c3506">;
def C3507 : LPUReg<3507, "%c3507">;
def C3508 : LPUReg<3508, "%c3508">;
def C3509 : LPUReg<3509, "%c3509">;
def C3510 : LPUReg<3510, "%c3510">;
def C3511 : LPUReg<3511, "%c3511">;
def C3512 : LPUReg<3512, "%c3512">;
def C3513 : LPUReg<3513, "%c3513">;
def C3514 : LPUReg<3514, "%c3514">;
def C3515 : LPUReg<3515, "%c3515">;
def C3516 : LPUReg<3516, "%c3516">;
def C3517 : LPUReg<3517, "%c3517">;
def C3518 : LPUReg<3518, "%c3518">;
def C3519 : LPUReg<3519, "%c3519">;
def C3520 : LPUReg<3520, "%c3520">;
def C3521 : LPUReg<3521, "%c3521">;
def C3522 : LPUReg<3522, "%c3522">;
def C3523 : LPUReg<3523, "%c3523">;
def C3524 : LPUReg<3524, "%c3524">;
def C3525 : LPUReg<3525, "%c3525">;
def C3526 : LPUReg<3526, "%c3526">;
def C3527 : LPUReg<3527, "%c3527">;
def C3528 : LPUReg<3528, "%c3528">;
def C3529 : LPUReg<3529, "%c3529">;
def C3530 : LPUReg<3530, "%c3530">;
def C3531 : LPUReg<3531, "%c3531">;
def C3532 : LPUReg<3532, "%c3532">;
def C3533 : LPUReg<3533, "%c3533">;
def C3534 : LPUReg<3534, "%c3534">;
def C3535 : LPUReg<3535, "%c3535">;
def C3536 : LPUReg<3536, "%c3536">;
def C3537 : LPUReg<3537, "%c3537">;
def C3538 : LPUReg<3538, "%c3538">;
def C3539 : LPUReg<3539, "%c3539">;
def C3540 : LPUReg<3540, "%c3540">;
def C3541 : LPUReg<3541, "%c3541">;
def C3542 : LPUReg<3542, "%c3542">;
def C3543 : LPUReg<3543, "%c3543">;
def C3544 : LPUReg<3544, "%c3544">;
def C3545 : LPUReg<3545, "%c3545">;
def C3546 : LPUReg<3546, "%c3546">;
def C3547 : LPUReg<3547, "%c3547">;
def C3548 : LPUReg<3548, "%c3548">;
def C3549 : LPUReg<3549, "%c3549">;
def C3550 : LPUReg<3550, "%c3550">;
def C3551 : LPUReg<3551, "%c3551">;
def C3552 : LPUReg<3552, "%c3552">;
def C3553 : LPUReg<3553, "%c3553">;
def C3554 : LPUReg<3554, "%c3554">;
def C3555 : LPUReg<3555, "%c3555">;
def C3556 : LPUReg<3556, "%c3556">;
def C3557 : LPUReg<3557, "%c3557">;
def C3558 : LPUReg<3558, "%c3558">;
def C3559 : LPUReg<3559, "%c3559">;
def C3560 : LPUReg<3560, "%c3560">;
def C3561 : LPUReg<3561, "%c3561">;
def C3562 : LPUReg<3562, "%c3562">;
def C3563 : LPUReg<3563, "%c3563">;
def C3564 : LPUReg<3564, "%c3564">;
def C3565 : LPUReg<3565, "%c3565">;
def C3566 : LPUReg<3566, "%c3566">;
def C3567 : LPUReg<3567, "%c3567">;
def C3568 : LPUReg<3568, "%c3568">;
def C3569 : LPUReg<3569, "%c3569">;
def C3570 : LPUReg<3570, "%c3570">;
def C3571 : LPUReg<3571, "%c3571">;
def C3572 : LPUReg<3572, "%c3572">;
def C3573 : LPUReg<3573, "%c3573">;
def C3574 : LPUReg<3574, "%c3574">;
def C3575 : LPUReg<3575, "%c3575">;
def C3576 : LPUReg<3576, "%c3576">;
def C3577 : LPUReg<3577, "%c3577">;
def C3578 : LPUReg<3578, "%c3578">;
def C3579 : LPUReg<3579, "%c3579">;
def C3580 : LPUReg<3580, "%c3580">;
def C3581 : LPUReg<3581, "%c3581">;
def C3582 : LPUReg<3582, "%c3582">;
def C3583 : LPUReg<3583, "%c3583">;
def C3584 : LPUReg<3584, "%c3584">;
def C3585 : LPUReg<3585, "%c3585">;
def C3586 : LPUReg<3586, "%c3586">;
def C3587 : LPUReg<3587, "%c3587">;
def C3588 : LPUReg<3588, "%c3588">;
def C3589 : LPUReg<3589, "%c3589">;
def C3590 : LPUReg<3590, "%c3590">;
def C3591 : LPUReg<3591, "%c3591">;
def C3592 : LPUReg<3592, "%c3592">;
def C3593 : LPUReg<3593, "%c3593">;
def C3594 : LPUReg<3594, "%c3594">;
def C3595 : LPUReg<3595, "%c3595">;
def C3596 : LPUReg<3596, "%c3596">;
def C3597 : LPUReg<3597, "%c3597">;
def C3598 : LPUReg<3598, "%c3598">;
def C3599 : LPUReg<3599, "%c3599">;
def C3600 : LPUReg<3600, "%c3600">;
def C3601 : LPUReg<3601, "%c3601">;
def C3602 : LPUReg<3602, "%c3602">;
def C3603 : LPUReg<3603, "%c3603">;
def C3604 : LPUReg<3604, "%c3604">;
def C3605 : LPUReg<3605, "%c3605">;
def C3606 : LPUReg<3606, "%c3606">;
def C3607 : LPUReg<3607, "%c3607">;
def C3608 : LPUReg<3608, "%c3608">;
def C3609 : LPUReg<3609, "%c3609">;
def C3610 : LPUReg<3610, "%c3610">;
def C3611 : LPUReg<3611, "%c3611">;
def C3612 : LPUReg<3612, "%c3612">;
def C3613 : LPUReg<3613, "%c3613">;
def C3614 : LPUReg<3614, "%c3614">;
def C3615 : LPUReg<3615, "%c3615">;
def C3616 : LPUReg<3616, "%c3616">;
def C3617 : LPUReg<3617, "%c3617">;
def C3618 : LPUReg<3618, "%c3618">;
def C3619 : LPUReg<3619, "%c3619">;
def C3620 : LPUReg<3620, "%c3620">;
def C3621 : LPUReg<3621, "%c3621">;
def C3622 : LPUReg<3622, "%c3622">;
def C3623 : LPUReg<3623, "%c3623">;
def C3624 : LPUReg<3624, "%c3624">;
def C3625 : LPUReg<3625, "%c3625">;
def C3626 : LPUReg<3626, "%c3626">;
def C3627 : LPUReg<3627, "%c3627">;
def C3628 : LPUReg<3628, "%c3628">;
def C3629 : LPUReg<3629, "%c3629">;
def C3630 : LPUReg<3630, "%c3630">;
def C3631 : LPUReg<3631, "%c3631">;
def C3632 : LPUReg<3632, "%c3632">;
def C3633 : LPUReg<3633, "%c3633">;
def C3634 : LPUReg<3634, "%c3634">;
def C3635 : LPUReg<3635, "%c3635">;
def C3636 : LPUReg<3636, "%c3636">;
def C3637 : LPUReg<3637, "%c3637">;
def C3638 : LPUReg<3638, "%c3638">;
def C3639 : LPUReg<3639, "%c3639">;
def C3640 : LPUReg<3640, "%c3640">;
def C3641 : LPUReg<3641, "%c3641">;
def C3642 : LPUReg<3642, "%c3642">;
def C3643 : LPUReg<3643, "%c3643">;
def C3644 : LPUReg<3644, "%c3644">;
def C3645 : LPUReg<3645, "%c3645">;
def C3646 : LPUReg<3646, "%c3646">;
def C3647 : LPUReg<3647, "%c3647">;
def C3648 : LPUReg<3648, "%c3648">;
def C3649 : LPUReg<3649, "%c3649">;
def C3650 : LPUReg<3650, "%c3650">;
def C3651 : LPUReg<3651, "%c3651">;
def C3652 : LPUReg<3652, "%c3652">;
def C3653 : LPUReg<3653, "%c3653">;
def C3654 : LPUReg<3654, "%c3654">;
def C3655 : LPUReg<3655, "%c3655">;
def C3656 : LPUReg<3656, "%c3656">;
def C3657 : LPUReg<3657, "%c3657">;
def C3658 : LPUReg<3658, "%c3658">;
def C3659 : LPUReg<3659, "%c3659">;
def C3660 : LPUReg<3660, "%c3660">;
def C3661 : LPUReg<3661, "%c3661">;
def C3662 : LPUReg<3662, "%c3662">;
def C3663 : LPUReg<3663, "%c3663">;
def C3664 : LPUReg<3664, "%c3664">;
def C3665 : LPUReg<3665, "%c3665">;
def C3666 : LPUReg<3666, "%c3666">;
def C3667 : LPUReg<3667, "%c3667">;
def C3668 : LPUReg<3668, "%c3668">;
def C3669 : LPUReg<3669, "%c3669">;
def C3670 : LPUReg<3670, "%c3670">;
def C3671 : LPUReg<3671, "%c3671">;
def C3672 : LPUReg<3672, "%c3672">;
def C3673 : LPUReg<3673, "%c3673">;
def C3674 : LPUReg<3674, "%c3674">;
def C3675 : LPUReg<3675, "%c3675">;
def C3676 : LPUReg<3676, "%c3676">;
def C3677 : LPUReg<3677, "%c3677">;
def C3678 : LPUReg<3678, "%c3678">;
def C3679 : LPUReg<3679, "%c3679">;
def C3680 : LPUReg<3680, "%c3680">;
def C3681 : LPUReg<3681, "%c3681">;
def C3682 : LPUReg<3682, "%c3682">;
def C3683 : LPUReg<3683, "%c3683">;
def C3684 : LPUReg<3684, "%c3684">;
def C3685 : LPUReg<3685, "%c3685">;
def C3686 : LPUReg<3686, "%c3686">;
def C3687 : LPUReg<3687, "%c3687">;
def C3688 : LPUReg<3688, "%c3688">;
def C3689 : LPUReg<3689, "%c3689">;
def C3690 : LPUReg<3690, "%c3690">;
def C3691 : LPUReg<3691, "%c3691">;
def C3692 : LPUReg<3692, "%c3692">;
def C3693 : LPUReg<3693, "%c3693">;
def C3694 : LPUReg<3694, "%c3694">;
def C3695 : LPUReg<3695, "%c3695">;
def C3696 : LPUReg<3696, "%c3696">;
def C3697 : LPUReg<3697, "%c3697">;
def C3698 : LPUReg<3698, "%c3698">;
def C3699 : LPUReg<3699, "%c3699">;
def C3700 : LPUReg<3700, "%c3700">;
def C3701 : LPUReg<3701, "%c3701">;
def C3702 : LPUReg<3702, "%c3702">;
def C3703 : LPUReg<3703, "%c3703">;
def C3704 : LPUReg<3704, "%c3704">;
def C3705 : LPUReg<3705, "%c3705">;
def C3706 : LPUReg<3706, "%c3706">;
def C3707 : LPUReg<3707, "%c3707">;
def C3708 : LPUReg<3708, "%c3708">;
def C3709 : LPUReg<3709, "%c3709">;
def C3710 : LPUReg<3710, "%c3710">;
def C3711 : LPUReg<3711, "%c3711">;
def C3712 : LPUReg<3712, "%c3712">;
def C3713 : LPUReg<3713, "%c3713">;
def C3714 : LPUReg<3714, "%c3714">;
def C3715 : LPUReg<3715, "%c3715">;
def C3716 : LPUReg<3716, "%c3716">;
def C3717 : LPUReg<3717, "%c3717">;
def C3718 : LPUReg<3718, "%c3718">;
def C3719 : LPUReg<3719, "%c3719">;
def C3720 : LPUReg<3720, "%c3720">;
def C3721 : LPUReg<3721, "%c3721">;
def C3722 : LPUReg<3722, "%c3722">;
def C3723 : LPUReg<3723, "%c3723">;
def C3724 : LPUReg<3724, "%c3724">;
def C3725 : LPUReg<3725, "%c3725">;
def C3726 : LPUReg<3726, "%c3726">;
def C3727 : LPUReg<3727, "%c3727">;
def C3728 : LPUReg<3728, "%c3728">;
def C3729 : LPUReg<3729, "%c3729">;
def C3730 : LPUReg<3730, "%c3730">;
def C3731 : LPUReg<3731, "%c3731">;
def C3732 : LPUReg<3732, "%c3732">;
def C3733 : LPUReg<3733, "%c3733">;
def C3734 : LPUReg<3734, "%c3734">;
def C3735 : LPUReg<3735, "%c3735">;
def C3736 : LPUReg<3736, "%c3736">;
def C3737 : LPUReg<3737, "%c3737">;
def C3738 : LPUReg<3738, "%c3738">;
def C3739 : LPUReg<3739, "%c3739">;
def C3740 : LPUReg<3740, "%c3740">;
def C3741 : LPUReg<3741, "%c3741">;
def C3742 : LPUReg<3742, "%c3742">;
def C3743 : LPUReg<3743, "%c3743">;
def C3744 : LPUReg<3744, "%c3744">;
def C3745 : LPUReg<3745, "%c3745">;
def C3746 : LPUReg<3746, "%c3746">;
def C3747 : LPUReg<3747, "%c3747">;
def C3748 : LPUReg<3748, "%c3748">;
def C3749 : LPUReg<3749, "%c3749">;
def C3750 : LPUReg<3750, "%c3750">;
def C3751 : LPUReg<3751, "%c3751">;
def C3752 : LPUReg<3752, "%c3752">;
def C3753 : LPUReg<3753, "%c3753">;
def C3754 : LPUReg<3754, "%c3754">;
def C3755 : LPUReg<3755, "%c3755">;
def C3756 : LPUReg<3756, "%c3756">;
def C3757 : LPUReg<3757, "%c3757">;
def C3758 : LPUReg<3758, "%c3758">;
def C3759 : LPUReg<3759, "%c3759">;
def C3760 : LPUReg<3760, "%c3760">;
def C3761 : LPUReg<3761, "%c3761">;
def C3762 : LPUReg<3762, "%c3762">;
def C3763 : LPUReg<3763, "%c3763">;
def C3764 : LPUReg<3764, "%c3764">;
def C3765 : LPUReg<3765, "%c3765">;
def C3766 : LPUReg<3766, "%c3766">;
def C3767 : LPUReg<3767, "%c3767">;
def C3768 : LPUReg<3768, "%c3768">;
def C3769 : LPUReg<3769, "%c3769">;
def C3770 : LPUReg<3770, "%c3770">;
def C3771 : LPUReg<3771, "%c3771">;
def C3772 : LPUReg<3772, "%c3772">;
def C3773 : LPUReg<3773, "%c3773">;
def C3774 : LPUReg<3774, "%c3774">;
def C3775 : LPUReg<3775, "%c3775">;
def C3776 : LPUReg<3776, "%c3776">;
def C3777 : LPUReg<3777, "%c3777">;
def C3778 : LPUReg<3778, "%c3778">;
def C3779 : LPUReg<3779, "%c3779">;
def C3780 : LPUReg<3780, "%c3780">;
def C3781 : LPUReg<3781, "%c3781">;
def C3782 : LPUReg<3782, "%c3782">;
def C3783 : LPUReg<3783, "%c3783">;
def C3784 : LPUReg<3784, "%c3784">;
def C3785 : LPUReg<3785, "%c3785">;
def C3786 : LPUReg<3786, "%c3786">;
def C3787 : LPUReg<3787, "%c3787">;
def C3788 : LPUReg<3788, "%c3788">;
def C3789 : LPUReg<3789, "%c3789">;
def C3790 : LPUReg<3790, "%c3790">;
def C3791 : LPUReg<3791, "%c3791">;
def C3792 : LPUReg<3792, "%c3792">;
def C3793 : LPUReg<3793, "%c3793">;
def C3794 : LPUReg<3794, "%c3794">;
def C3795 : LPUReg<3795, "%c3795">;
def C3796 : LPUReg<3796, "%c3796">;
def C3797 : LPUReg<3797, "%c3797">;
def C3798 : LPUReg<3798, "%c3798">;
def C3799 : LPUReg<3799, "%c3799">;
def C3800 : LPUReg<3800, "%c3800">;
def C3801 : LPUReg<3801, "%c3801">;
def C3802 : LPUReg<3802, "%c3802">;
def C3803 : LPUReg<3803, "%c3803">;
def C3804 : LPUReg<3804, "%c3804">;
def C3805 : LPUReg<3805, "%c3805">;
def C3806 : LPUReg<3806, "%c3806">;
def C3807 : LPUReg<3807, "%c3807">;
def C3808 : LPUReg<3808, "%c3808">;
def C3809 : LPUReg<3809, "%c3809">;
def C3810 : LPUReg<3810, "%c3810">;
def C3811 : LPUReg<3811, "%c3811">;
def C3812 : LPUReg<3812, "%c3812">;
def C3813 : LPUReg<3813, "%c3813">;
def C3814 : LPUReg<3814, "%c3814">;
def C3815 : LPUReg<3815, "%c3815">;
def C3816 : LPUReg<3816, "%c3816">;
def C3817 : LPUReg<3817, "%c3817">;
def C3818 : LPUReg<3818, "%c3818">;
def C3819 : LPUReg<3819, "%c3819">;
def C3820 : LPUReg<3820, "%c3820">;
def C3821 : LPUReg<3821, "%c3821">;
def C3822 : LPUReg<3822, "%c3822">;
def C3823 : LPUReg<3823, "%c3823">;
def C3824 : LPUReg<3824, "%c3824">;
def C3825 : LPUReg<3825, "%c3825">;
def C3826 : LPUReg<3826, "%c3826">;
def C3827 : LPUReg<3827, "%c3827">;
def C3828 : LPUReg<3828, "%c3828">;
def C3829 : LPUReg<3829, "%c3829">;
def C3830 : LPUReg<3830, "%c3830">;
def C3831 : LPUReg<3831, "%c3831">;
def C3832 : LPUReg<3832, "%c3832">;
def C3833 : LPUReg<3833, "%c3833">;
def C3834 : LPUReg<3834, "%c3834">;
def C3835 : LPUReg<3835, "%c3835">;
def C3836 : LPUReg<3836, "%c3836">;
def C3837 : LPUReg<3837, "%c3837">;
def C3838 : LPUReg<3838, "%c3838">;
def C3839 : LPUReg<3839, "%c3839">;
def C3840 : LPUReg<3840, "%c3840">;
def C3841 : LPUReg<3841, "%c3841">;
def C3842 : LPUReg<3842, "%c3842">;
def C3843 : LPUReg<3843, "%c3843">;
def C3844 : LPUReg<3844, "%c3844">;
def C3845 : LPUReg<3845, "%c3845">;
def C3846 : LPUReg<3846, "%c3846">;
def C3847 : LPUReg<3847, "%c3847">;
def C3848 : LPUReg<3848, "%c3848">;
def C3849 : LPUReg<3849, "%c3849">;
def C3850 : LPUReg<3850, "%c3850">;
def C3851 : LPUReg<3851, "%c3851">;
def C3852 : LPUReg<3852, "%c3852">;
def C3853 : LPUReg<3853, "%c3853">;
def C3854 : LPUReg<3854, "%c3854">;
def C3855 : LPUReg<3855, "%c3855">;
def C3856 : LPUReg<3856, "%c3856">;
def C3857 : LPUReg<3857, "%c3857">;
def C3858 : LPUReg<3858, "%c3858">;
def C3859 : LPUReg<3859, "%c3859">;
def C3860 : LPUReg<3860, "%c3860">;
def C3861 : LPUReg<3861, "%c3861">;
def C3862 : LPUReg<3862, "%c3862">;
def C3863 : LPUReg<3863, "%c3863">;
def C3864 : LPUReg<3864, "%c3864">;
def C3865 : LPUReg<3865, "%c3865">;
def C3866 : LPUReg<3866, "%c3866">;
def C3867 : LPUReg<3867, "%c3867">;
def C3868 : LPUReg<3868, "%c3868">;
def C3869 : LPUReg<3869, "%c3869">;
def C3870 : LPUReg<3870, "%c3870">;
def C3871 : LPUReg<3871, "%c3871">;
def C3872 : LPUReg<3872, "%c3872">;
def C3873 : LPUReg<3873, "%c3873">;
def C3874 : LPUReg<3874, "%c3874">;
def C3875 : LPUReg<3875, "%c3875">;
def C3876 : LPUReg<3876, "%c3876">;
def C3877 : LPUReg<3877, "%c3877">;
def C3878 : LPUReg<3878, "%c3878">;
def C3879 : LPUReg<3879, "%c3879">;
def C3880 : LPUReg<3880, "%c3880">;
def C3881 : LPUReg<3881, "%c3881">;
def C3882 : LPUReg<3882, "%c3882">;
def C3883 : LPUReg<3883, "%c3883">;
def C3884 : LPUReg<3884, "%c3884">;
def C3885 : LPUReg<3885, "%c3885">;
def C3886 : LPUReg<3886, "%c3886">;
def C3887 : LPUReg<3887, "%c3887">;
def C3888 : LPUReg<3888, "%c3888">;
def C3889 : LPUReg<3889, "%c3889">;
def C3890 : LPUReg<3890, "%c3890">;
def C3891 : LPUReg<3891, "%c3891">;
def C3892 : LPUReg<3892, "%c3892">;
def C3893 : LPUReg<3893, "%c3893">;
def C3894 : LPUReg<3894, "%c3894">;
def C3895 : LPUReg<3895, "%c3895">;
def C3896 : LPUReg<3896, "%c3896">;
def C3897 : LPUReg<3897, "%c3897">;
def C3898 : LPUReg<3898, "%c3898">;
def C3899 : LPUReg<3899, "%c3899">;
def C3900 : LPUReg<3900, "%c3900">;
def C3901 : LPUReg<3901, "%c3901">;
def C3902 : LPUReg<3902, "%c3902">;
def C3903 : LPUReg<3903, "%c3903">;
def C3904 : LPUReg<3904, "%c3904">;
def C3905 : LPUReg<3905, "%c3905">;
def C3906 : LPUReg<3906, "%c3906">;
def C3907 : LPUReg<3907, "%c3907">;
def C3908 : LPUReg<3908, "%c3908">;
def C3909 : LPUReg<3909, "%c3909">;
def C3910 : LPUReg<3910, "%c3910">;
def C3911 : LPUReg<3911, "%c3911">;
def C3912 : LPUReg<3912, "%c3912">;
def C3913 : LPUReg<3913, "%c3913">;
def C3914 : LPUReg<3914, "%c3914">;
def C3915 : LPUReg<3915, "%c3915">;
def C3916 : LPUReg<3916, "%c3916">;
def C3917 : LPUReg<3917, "%c3917">;
def C3918 : LPUReg<3918, "%c3918">;
def C3919 : LPUReg<3919, "%c3919">;
def C3920 : LPUReg<3920, "%c3920">;
def C3921 : LPUReg<3921, "%c3921">;
def C3922 : LPUReg<3922, "%c3922">;
def C3923 : LPUReg<3923, "%c3923">;
def C3924 : LPUReg<3924, "%c3924">;
def C3925 : LPUReg<3925, "%c3925">;
def C3926 : LPUReg<3926, "%c3926">;
def C3927 : LPUReg<3927, "%c3927">;
def C3928 : LPUReg<3928, "%c3928">;
def C3929 : LPUReg<3929, "%c3929">;
def C3930 : LPUReg<3930, "%c3930">;
def C3931 : LPUReg<3931, "%c3931">;
def C3932 : LPUReg<3932, "%c3932">;
def C3933 : LPUReg<3933, "%c3933">;
def C3934 : LPUReg<3934, "%c3934">;
def C3935 : LPUReg<3935, "%c3935">;
def C3936 : LPUReg<3936, "%c3936">;
def C3937 : LPUReg<3937, "%c3937">;
def C3938 : LPUReg<3938, "%c3938">;
def C3939 : LPUReg<3939, "%c3939">;
def C3940 : LPUReg<3940, "%c3940">;
def C3941 : LPUReg<3941, "%c3941">;
def C3942 : LPUReg<3942, "%c3942">;
def C3943 : LPUReg<3943, "%c3943">;
def C3944 : LPUReg<3944, "%c3944">;
def C3945 : LPUReg<3945, "%c3945">;
def C3946 : LPUReg<3946, "%c3946">;
def C3947 : LPUReg<3947, "%c3947">;
def C3948 : LPUReg<3948, "%c3948">;
def C3949 : LPUReg<3949, "%c3949">;
def C3950 : LPUReg<3950, "%c3950">;
def C3951 : LPUReg<3951, "%c3951">;
def C3952 : LPUReg<3952, "%c3952">;
def C3953 : LPUReg<3953, "%c3953">;
def C3954 : LPUReg<3954, "%c3954">;
def C3955 : LPUReg<3955, "%c3955">;
def C3956 : LPUReg<3956, "%c3956">;
def C3957 : LPUReg<3957, "%c3957">;
def C3958 : LPUReg<3958, "%c3958">;
def C3959 : LPUReg<3959, "%c3959">;
def C3960 : LPUReg<3960, "%c3960">;
def C3961 : LPUReg<3961, "%c3961">;
def C3962 : LPUReg<3962, "%c3962">;
def C3963 : LPUReg<3963, "%c3963">;
def C3964 : LPUReg<3964, "%c3964">;
def C3965 : LPUReg<3965, "%c3965">;
def C3966 : LPUReg<3966, "%c3966">;
def C3967 : LPUReg<3967, "%c3967">;
def C3968 : LPUReg<3968, "%c3968">;
def C3969 : LPUReg<3969, "%c3969">;
def C3970 : LPUReg<3970, "%c3970">;
def C3971 : LPUReg<3971, "%c3971">;
def C3972 : LPUReg<3972, "%c3972">;
def C3973 : LPUReg<3973, "%c3973">;
def C3974 : LPUReg<3974, "%c3974">;
def C3975 : LPUReg<3975, "%c3975">;
def C3976 : LPUReg<3976, "%c3976">;
def C3977 : LPUReg<3977, "%c3977">;
def C3978 : LPUReg<3978, "%c3978">;
def C3979 : LPUReg<3979, "%c3979">;
def C3980 : LPUReg<3980, "%c3980">;
def C3981 : LPUReg<3981, "%c3981">;
def C3982 : LPUReg<3982, "%c3982">;
def C3983 : LPUReg<3983, "%c3983">;
def C3984 : LPUReg<3984, "%c3984">;
def C3985 : LPUReg<3985, "%c3985">;
def C3986 : LPUReg<3986, "%c3986">;
def C3987 : LPUReg<3987, "%c3987">;
def C3988 : LPUReg<3988, "%c3988">;
def C3989 : LPUReg<3989, "%c3989">;
def C3990 : LPUReg<3990, "%c3990">;
def C3991 : LPUReg<3991, "%c3991">;
def C3992 : LPUReg<3992, "%c3992">;
def C3993 : LPUReg<3993, "%c3993">;
def C3994 : LPUReg<3994, "%c3994">;
def C3995 : LPUReg<3995, "%c3995">;
def C3996 : LPUReg<3996, "%c3996">;
def C3997 : LPUReg<3997, "%c3997">;
def C3998 : LPUReg<3998, "%c3998">;
def C3999 : LPUReg<3999, "%c3999">;
def C4000 : LPUReg<4000, "%c4000">;
def C4001 : LPUReg<4001, "%c4001">;
def C4002 : LPUReg<4002, "%c4002">;
def C4003 : LPUReg<4003, "%c4003">;
def C4004 : LPUReg<4004, "%c4004">;
def C4005 : LPUReg<4005, "%c4005">;
def C4006 : LPUReg<4006, "%c4006">;
def C4007 : LPUReg<4007, "%c4007">;
def C4008 : LPUReg<4008, "%c4008">;
def C4009 : LPUReg<4009, "%c4009">;
def C4010 : LPUReg<4010, "%c4010">;
def C4011 : LPUReg<4011, "%c4011">;
def C4012 : LPUReg<4012, "%c4012">;
def C4013 : LPUReg<4013, "%c4013">;
def C4014 : LPUReg<4014, "%c4014">;
def C4015 : LPUReg<4015, "%c4015">;
def C4016 : LPUReg<4016, "%c4016">;
def C4017 : LPUReg<4017, "%c4017">;
def C4018 : LPUReg<4018, "%c4018">;
def C4019 : LPUReg<4019, "%c4019">;
def C4020 : LPUReg<4020, "%c4020">;
def C4021 : LPUReg<4021, "%c4021">;
def C4022 : LPUReg<4022, "%c4022">;
def C4023 : LPUReg<4023, "%c4023">;
def C4024 : LPUReg<4024, "%c4024">;
def C4025 : LPUReg<4025, "%c4025">;
def C4026 : LPUReg<4026, "%c4026">;
def C4027 : LPUReg<4027, "%c4027">;
def C4028 : LPUReg<4028, "%c4028">;
def C4029 : LPUReg<4029, "%c4029">;
def C4030 : LPUReg<4030, "%c4030">;
def C4031 : LPUReg<4031, "%c4031">;
def C4032 : LPUReg<4032, "%c4032">;
def C4033 : LPUReg<4033, "%c4033">;
def C4034 : LPUReg<4034, "%c4034">;
def C4035 : LPUReg<4035, "%c4035">;
def C4036 : LPUReg<4036, "%c4036">;
def C4037 : LPUReg<4037, "%c4037">;
def C4038 : LPUReg<4038, "%c4038">;
def C4039 : LPUReg<4039, "%c4039">;
def C4040 : LPUReg<4040, "%c4040">;
def C4041 : LPUReg<4041, "%c4041">;
def C4042 : LPUReg<4042, "%c4042">;
def C4043 : LPUReg<4043, "%c4043">;
def C4044 : LPUReg<4044, "%c4044">;
def C4045 : LPUReg<4045, "%c4045">;
def C4046 : LPUReg<4046, "%c4046">;
def C4047 : LPUReg<4047, "%c4047">;
def C4048 : LPUReg<4048, "%c4048">;
def C4049 : LPUReg<4049, "%c4049">;
def C4050 : LPUReg<4050, "%c4050">;
def C4051 : LPUReg<4051, "%c4051">;
def C4052 : LPUReg<4052, "%c4052">;
def C4053 : LPUReg<4053, "%c4053">;
def C4054 : LPUReg<4054, "%c4054">;
def C4055 : LPUReg<4055, "%c4055">;
def C4056 : LPUReg<4056, "%c4056">;
def C4057 : LPUReg<4057, "%c4057">;
def C4058 : LPUReg<4058, "%c4058">;
def C4059 : LPUReg<4059, "%c4059">;
def C4060 : LPUReg<4060, "%c4060">;
def C4061 : LPUReg<4061, "%c4061">;
def C4062 : LPUReg<4062, "%c4062">;
def C4063 : LPUReg<4063, "%c4063">;
def C4064 : LPUReg<4064, "%c4064">;
def C4065 : LPUReg<4065, "%c4065">;
def C4066 : LPUReg<4066, "%c4066">;
def C4067 : LPUReg<4067, "%c4067">;
def C4068 : LPUReg<4068, "%c4068">;
def C4069 : LPUReg<4069, "%c4069">;
def C4070 : LPUReg<4070, "%c4070">;
def C4071 : LPUReg<4071, "%c4071">;
def C4072 : LPUReg<4072, "%c4072">;
def C4073 : LPUReg<4073, "%c4073">;
def C4074 : LPUReg<4074, "%c4074">;
def C4075 : LPUReg<4075, "%c4075">;
def C4076 : LPUReg<4076, "%c4076">;
def C4077 : LPUReg<4077, "%c4077">;
def C4078 : LPUReg<4078, "%c4078">;
def C4079 : LPUReg<4079, "%c4079">;
def C4080 : LPUReg<4080, "%c4080">;
def C4081 : LPUReg<4081, "%c4081">;
def C4082 : LPUReg<4082, "%c4082">;
def C4083 : LPUReg<4083, "%c4083">;
def C4084 : LPUReg<4084, "%c4084">;
def C4085 : LPUReg<4085, "%c4085">;
def C4086 : LPUReg<4086, "%c4086">;
def C4087 : LPUReg<4087, "%c4087">;
def C4088 : LPUReg<4088, "%c4088">;
def C4089 : LPUReg<4089, "%c4089">;
def C4090 : LPUReg<4090, "%c4090">;
def C4091 : LPUReg<4091, "%c4091">;
// 4092..4095 reserved for SP, RA, etc.

// generic channel class
def ANYC : RegisterClass<"LPU", [i1,i8,i16,i32,i64,f16,f32,f64], 64,
   (add   
         IGN,   // IGN (ignore) part of all regs, but not allocatable...

          C0,   C1,   C2,   C3,   C4,   C5,   C6,   C7,   C8,   C9,
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
        C2040, C2041, C2042, C2043, C2044, C2045, C2046, C2047, C2048, C2049,
        C2050, C2051, C2052, C2053, C2054, C2055, C2056, C2057, C2058, C2059,
        C2060, C2061, C2062, C2063, C2064, C2065, C2066, C2067, C2068, C2069,
        C2070, C2071, C2072, C2073, C2074, C2075, C2076, C2077, C2078, C2079,
        C2080, C2081, C2082, C2083, C2084, C2085, C2086, C2087, C2088, C2089,
        C2090, C2091, C2092, C2093, C2094, C2095, C2096, C2097, C2098, C2099,
        C2100, C2101, C2102, C2103, C2104, C2105, C2106, C2107, C2108, C2109,
        C2110, C2111, C2112, C2113, C2114, C2115, C2116, C2117, C2118, C2119,
        C2120, C2121, C2122, C2123, C2124, C2125, C2126, C2127, C2128, C2129,
        C2130, C2131, C2132, C2133, C2134, C2135, C2136, C2137, C2138, C2139,
        C2140, C2141, C2142, C2143, C2144, C2145, C2146, C2147, C2148, C2149,
        C2150, C2151, C2152, C2153, C2154, C2155, C2156, C2157, C2158, C2159,
        C2160, C2161, C2162, C2163, C2164, C2165, C2166, C2167, C2168, C2169,
        C2170, C2171, C2172, C2173, C2174, C2175, C2176, C2177, C2178, C2179,
        C2180, C2181, C2182, C2183, C2184, C2185, C2186, C2187, C2188, C2189,
        C2190, C2191, C2192, C2193, C2194, C2195, C2196, C2197, C2198, C2199,
        C2200, C2201, C2202, C2203, C2204, C2205, C2206, C2207, C2208, C2209,
        C2210, C2211, C2212, C2213, C2214, C2215, C2216, C2217, C2218, C2219,
        C2220, C2221, C2222, C2223, C2224, C2225, C2226, C2227, C2228, C2229,
        C2230, C2231, C2232, C2233, C2234, C2235, C2236, C2237, C2238, C2239,
        C2240, C2241, C2242, C2243, C2244, C2245, C2246, C2247, C2248, C2249,
        C2250, C2251, C2252, C2253, C2254, C2255, C2256, C2257, C2258, C2259,
        C2260, C2261, C2262, C2263, C2264, C2265, C2266, C2267, C2268, C2269,
        C2270, C2271, C2272, C2273, C2274, C2275, C2276, C2277, C2278, C2279,
        C2280, C2281, C2282, C2283, C2284, C2285, C2286, C2287, C2288, C2289,
        C2290, C2291, C2292, C2293, C2294, C2295, C2296, C2297, C2298, C2299,
        C2300, C2301, C2302, C2303, C2304, C2305, C2306, C2307, C2308, C2309,
        C2310, C2311, C2312, C2313, C2314, C2315, C2316, C2317, C2318, C2319,
        C2320, C2321, C2322, C2323, C2324, C2325, C2326, C2327, C2328, C2329,
        C2330, C2331, C2332, C2333, C2334, C2335, C2336, C2337, C2338, C2339,
        C2340, C2341, C2342, C2343, C2344, C2345, C2346, C2347, C2348, C2349,
        C2350, C2351, C2352, C2353, C2354, C2355, C2356, C2357, C2358, C2359,
        C2360, C2361, C2362, C2363, C2364, C2365, C2366, C2367, C2368, C2369,
        C2370, C2371, C2372, C2373, C2374, C2375, C2376, C2377, C2378, C2379,
        C2380, C2381, C2382, C2383, C2384, C2385, C2386, C2387, C2388, C2389,
        C2390, C2391, C2392, C2393, C2394, C2395, C2396, C2397, C2398, C2399,
        C2400, C2401, C2402, C2403, C2404, C2405, C2406, C2407, C2408, C2409,
        C2410, C2411, C2412, C2413, C2414, C2415, C2416, C2417, C2418, C2419,
        C2420, C2421, C2422, C2423, C2424, C2425, C2426, C2427, C2428, C2429,
        C2430, C2431, C2432, C2433, C2434, C2435, C2436, C2437, C2438, C2439,
        C2440, C2441, C2442, C2443, C2444, C2445, C2446, C2447, C2448, C2449,
        C2450, C2451, C2452, C2453, C2454, C2455, C2456, C2457, C2458, C2459,
        C2460, C2461, C2462, C2463, C2464, C2465, C2466, C2467, C2468, C2469,
        C2470, C2471, C2472, C2473, C2474, C2475, C2476, C2477, C2478, C2479,
        C2480, C2481, C2482, C2483, C2484, C2485, C2486, C2487, C2488, C2489,
        C2490, C2491, C2492, C2493, C2494, C2495, C2496, C2497, C2498, C2499,
        C2500, C2501, C2502, C2503, C2504, C2505, C2506, C2507, C2508, C2509,
        C2510, C2511, C2512, C2513, C2514, C2515, C2516, C2517, C2518, C2519,
        C2520, C2521, C2522, C2523, C2524, C2525, C2526, C2527, C2528, C2529,
        C2530, C2531, C2532, C2533, C2534, C2535, C2536, C2537, C2538, C2539,
        C2540, C2541, C2542, C2543, C2544, C2545, C2546, C2547, C2548, C2549,
        C2550, C2551, C2552, C2553, C2554, C2555, C2556, C2557, C2558, C2559,
        C2560, C2561, C2562, C2563, C2564, C2565, C2566, C2567, C2568, C2569,
        C2570, C2571, C2572, C2573, C2574, C2575, C2576, C2577, C2578, C2579,
        C2580, C2581, C2582, C2583, C2584, C2585, C2586, C2587, C2588, C2589,
        C2590, C2591, C2592, C2593, C2594, C2595, C2596, C2597, C2598, C2599,
        C2600, C2601, C2602, C2603, C2604, C2605, C2606, C2607, C2608, C2609,
        C2610, C2611, C2612, C2613, C2614, C2615, C2616, C2617, C2618, C2619,
        C2620, C2621, C2622, C2623, C2624, C2625, C2626, C2627, C2628, C2629,
        C2630, C2631, C2632, C2633, C2634, C2635, C2636, C2637, C2638, C2639,
        C2640, C2641, C2642, C2643, C2644, C2645, C2646, C2647, C2648, C2649,
        C2650, C2651, C2652, C2653, C2654, C2655, C2656, C2657, C2658, C2659,
        C2660, C2661, C2662, C2663, C2664, C2665, C2666, C2667, C2668, C2669,
        C2670, C2671, C2672, C2673, C2674, C2675, C2676, C2677, C2678, C2679,
        C2680, C2681, C2682, C2683, C2684, C2685, C2686, C2687, C2688, C2689,
        C2690, C2691, C2692, C2693, C2694, C2695, C2696, C2697, C2698, C2699,
        C2700, C2701, C2702, C2703, C2704, C2705, C2706, C2707, C2708, C2709,
        C2710, C2711, C2712, C2713, C2714, C2715, C2716, C2717, C2718, C2719,
        C2720, C2721, C2722, C2723, C2724, C2725, C2726, C2727, C2728, C2729,
        C2730, C2731, C2732, C2733, C2734, C2735, C2736, C2737, C2738, C2739,
        C2740, C2741, C2742, C2743, C2744, C2745, C2746, C2747, C2748, C2749,
        C2750, C2751, C2752, C2753, C2754, C2755, C2756, C2757, C2758, C2759,
        C2760, C2761, C2762, C2763, C2764, C2765, C2766, C2767, C2768, C2769,
        C2770, C2771, C2772, C2773, C2774, C2775, C2776, C2777, C2778, C2779,
        C2780, C2781, C2782, C2783, C2784, C2785, C2786, C2787, C2788, C2789,
        C2790, C2791, C2792, C2793, C2794, C2795, C2796, C2797, C2798, C2799,
        C2800, C2801, C2802, C2803, C2804, C2805, C2806, C2807, C2808, C2809,
        C2810, C2811, C2812, C2813, C2814, C2815, C2816, C2817, C2818, C2819,
        C2820, C2821, C2822, C2823, C2824, C2825, C2826, C2827, C2828, C2829,
        C2830, C2831, C2832, C2833, C2834, C2835, C2836, C2837, C2838, C2839,
        C2840, C2841, C2842, C2843, C2844, C2845, C2846, C2847, C2848, C2849,
        C2850, C2851, C2852, C2853, C2854, C2855, C2856, C2857, C2858, C2859,
        C2860, C2861, C2862, C2863, C2864, C2865, C2866, C2867, C2868, C2869,
        C2870, C2871, C2872, C2873, C2874, C2875, C2876, C2877, C2878, C2879,
        C2880, C2881, C2882, C2883, C2884, C2885, C2886, C2887, C2888, C2889,
        C2890, C2891, C2892, C2893, C2894, C2895, C2896, C2897, C2898, C2899,
        C2900, C2901, C2902, C2903, C2904, C2905, C2906, C2907, C2908, C2909,
        C2910, C2911, C2912, C2913, C2914, C2915, C2916, C2917, C2918, C2919,
        C2920, C2921, C2922, C2923, C2924, C2925, C2926, C2927, C2928, C2929,
        C2930, C2931, C2932, C2933, C2934, C2935, C2936, C2937, C2938, C2939,
        C2940, C2941, C2942, C2943, C2944, C2945, C2946, C2947, C2948, C2949,
        C2950, C2951, C2952, C2953, C2954, C2955, C2956, C2957, C2958, C2959,
        C2960, C2961, C2962, C2963, C2964, C2965, C2966, C2967, C2968, C2969,
        C2970, C2971, C2972, C2973, C2974, C2975, C2976, C2977, C2978, C2979,
        C2980, C2981, C2982, C2983, C2984, C2985, C2986, C2987, C2988, C2989,
        C2990, C2991, C2992, C2993, C2994, C2995, C2996, C2997, C2998, C2999,
        C3000, C3001, C3002, C3003, C3004, C3005, C3006, C3007, C3008, C3009,
        C3010, C3011, C3012, C3013, C3014, C3015, C3016, C3017, C3018, C3019,
        C3020, C3021, C3022, C3023, C3024, C3025, C3026, C3027, C3028, C3029,
        C3030, C3031, C3032, C3033, C3034, C3035, C3036, C3037, C3038, C3039,
        C3040, C3041, C3042, C3043, C3044, C3045, C3046, C3047, C3048, C3049,
        C3050, C3051, C3052, C3053, C3054, C3055, C3056, C3057, C3058, C3059,
        C3060, C3061, C3062, C3063, C3064, C3065, C3066, C3067, C3068, C3069,
        C3070, C3071, C3072, C3073, C3074, C3075, C3076, C3077, C3078, C3079,
        C3080, C3081, C3082, C3083, C3084, C3085, C3086, C3087, C3088, C3089,
        C3090, C3091, C3092, C3093, C3094, C3095, C3096, C3097, C3098, C3099,
        C3100, C3101, C3102, C3103, C3104, C3105, C3106, C3107, C3108, C3109,
        C3110, C3111, C3112, C3113, C3114, C3115, C3116, C3117, C3118, C3119,
        C3120, C3121, C3122, C3123, C3124, C3125, C3126, C3127, C3128, C3129,
        C3130, C3131, C3132, C3133, C3134, C3135, C3136, C3137, C3138, C3139,
        C3140, C3141, C3142, C3143, C3144, C3145, C3146, C3147, C3148, C3149,
        C3150, C3151, C3152, C3153, C3154, C3155, C3156, C3157, C3158, C3159,
        C3160, C3161, C3162, C3163, C3164, C3165, C3166, C3167, C3168, C3169,
        C3170, C3171, C3172, C3173, C3174, C3175, C3176, C3177, C3178, C3179,
        C3180, C3181, C3182, C3183, C3184, C3185, C3186, C3187, C3188, C3189,
        C3190, C3191, C3192, C3193, C3194, C3195, C3196, C3197, C3198, C3199,
        C3200, C3201, C3202, C3203, C3204, C3205, C3206, C3207, C3208, C3209,
        C3210, C3211, C3212, C3213, C3214, C3215, C3216, C3217, C3218, C3219,
        C3220, C3221, C3222, C3223, C3224, C3225, C3226, C3227, C3228, C3229,
        C3230, C3231, C3232, C3233, C3234, C3235, C3236, C3237, C3238, C3239,
        C3240, C3241, C3242, C3243, C3244, C3245, C3246, C3247, C3248, C3249,
        C3250, C3251, C3252, C3253, C3254, C3255, C3256, C3257, C3258, C3259,
        C3260, C3261, C3262, C3263, C3264, C3265, C3266, C3267, C3268, C3269,
        C3270, C3271, C3272, C3273, C3274, C3275, C3276, C3277, C3278, C3279,
        C3280, C3281, C3282, C3283, C3284, C3285, C3286, C3287, C3288, C3289,
        C3290, C3291, C3292, C3293, C3294, C3295, C3296, C3297, C3298, C3299,
        C3300, C3301, C3302, C3303, C3304, C3305, C3306, C3307, C3308, C3309,
        C3310, C3311, C3312, C3313, C3314, C3315, C3316, C3317, C3318, C3319,
        C3320, C3321, C3322, C3323, C3324, C3325, C3326, C3327, C3328, C3329,
        C3330, C3331, C3332, C3333, C3334, C3335, C3336, C3337, C3338, C3339,
        C3340, C3341, C3342, C3343, C3344, C3345, C3346, C3347, C3348, C3349,
        C3350, C3351, C3352, C3353, C3354, C3355, C3356, C3357, C3358, C3359,
        C3360, C3361, C3362, C3363, C3364, C3365, C3366, C3367, C3368, C3369,
        C3370, C3371, C3372, C3373, C3374, C3375, C3376, C3377, C3378, C3379,
        C3380, C3381, C3382, C3383, C3384, C3385, C3386, C3387, C3388, C3389,
        C3390, C3391, C3392, C3393, C3394, C3395, C3396, C3397, C3398, C3399,
        C3400, C3401, C3402, C3403, C3404, C3405, C3406, C3407, C3408, C3409,
        C3410, C3411, C3412, C3413, C3414, C3415, C3416, C3417, C3418, C3419,
        C3420, C3421, C3422, C3423, C3424, C3425, C3426, C3427, C3428, C3429,
        C3430, C3431, C3432, C3433, C3434, C3435, C3436, C3437, C3438, C3439,
        C3440, C3441, C3442, C3443, C3444, C3445, C3446, C3447, C3448, C3449,
        C3450, C3451, C3452, C3453, C3454, C3455, C3456, C3457, C3458, C3459,
        C3460, C3461, C3462, C3463, C3464, C3465, C3466, C3467, C3468, C3469,
        C3470, C3471, C3472, C3473, C3474, C3475, C3476, C3477, C3478, C3479,
        C3480, C3481, C3482, C3483, C3484, C3485, C3486, C3487, C3488, C3489,
        C3490, C3491, C3492, C3493, C3494, C3495, C3496, C3497, C3498, C3499,
        C3500, C3501, C3502, C3503, C3504, C3505, C3506, C3507, C3508, C3509,
        C3510, C3511, C3512, C3513, C3514, C3515, C3516, C3517, C3518, C3519,
        C3520, C3521, C3522, C3523, C3524, C3525, C3526, C3527, C3528, C3529,
        C3530, C3531, C3532, C3533, C3534, C3535, C3536, C3537, C3538, C3539,
        C3540, C3541, C3542, C3543, C3544, C3545, C3546, C3547, C3548, C3549,
        C3550, C3551, C3552, C3553, C3554, C3555, C3556, C3557, C3558, C3559,
        C3560, C3561, C3562, C3563, C3564, C3565, C3566, C3567, C3568, C3569,
        C3570, C3571, C3572, C3573, C3574, C3575, C3576, C3577, C3578, C3579,
        C3580, C3581, C3582, C3583, C3584, C3585, C3586, C3587, C3588, C3589,
        C3590, C3591, C3592, C3593, C3594, C3595, C3596, C3597, C3598, C3599,
        C3600, C3601, C3602, C3603, C3604, C3605, C3606, C3607, C3608, C3609,
        C3610, C3611, C3612, C3613, C3614, C3615, C3616, C3617, C3618, C3619,
        C3620, C3621, C3622, C3623, C3624, C3625, C3626, C3627, C3628, C3629,
        C3630, C3631, C3632, C3633, C3634, C3635, C3636, C3637, C3638, C3639,
        C3640, C3641, C3642, C3643, C3644, C3645, C3646, C3647, C3648, C3649,
        C3650, C3651, C3652, C3653, C3654, C3655, C3656, C3657, C3658, C3659,
        C3660, C3661, C3662, C3663, C3664, C3665, C3666, C3667, C3668, C3669,
        C3670, C3671, C3672, C3673, C3674, C3675, C3676, C3677, C3678, C3679,
        C3680, C3681, C3682, C3683, C3684, C3685, C3686, C3687, C3688, C3689,
        C3690, C3691, C3692, C3693, C3694, C3695, C3696, C3697, C3698, C3699,
        C3700, C3701, C3702, C3703, C3704, C3705, C3706, C3707, C3708, C3709,
        C3710, C3711, C3712, C3713, C3714, C3715, C3716, C3717, C3718, C3719,
        C3720, C3721, C3722, C3723, C3724, C3725, C3726, C3727, C3728, C3729,
        C3730, C3731, C3732, C3733, C3734, C3735, C3736, C3737, C3738, C3739,
        C3740, C3741, C3742, C3743, C3744, C3745, C3746, C3747, C3748, C3749,
        C3750, C3751, C3752, C3753, C3754, C3755, C3756, C3757, C3758, C3759,
        C3760, C3761, C3762, C3763, C3764, C3765, C3766, C3767, C3768, C3769,
        C3770, C3771, C3772, C3773, C3774, C3775, C3776, C3777, C3778, C3779,
        C3780, C3781, C3782, C3783, C3784, C3785, C3786, C3787, C3788, C3789,
        C3790, C3791, C3792, C3793, C3794, C3795, C3796, C3797, C3798, C3799,
        C3800, C3801, C3802, C3803, C3804, C3805, C3806, C3807, C3808, C3809,
        C3810, C3811, C3812, C3813, C3814, C3815, C3816, C3817, C3818, C3819,
        C3820, C3821, C3822, C3823, C3824, C3825, C3826, C3827, C3828, C3829,
        C3830, C3831, C3832, C3833, C3834, C3835, C3836, C3837, C3838, C3839,
        C3840, C3841, C3842, C3843, C3844, C3845, C3846, C3847, C3848, C3849,
        C3850, C3851, C3852, C3853, C3854, C3855, C3856, C3857, C3858, C3859,
        C3860, C3861, C3862, C3863, C3864, C3865, C3866, C3867, C3868, C3869,
        C3870, C3871, C3872, C3873, C3874, C3875, C3876, C3877, C3878, C3879,
        C3880, C3881, C3882, C3883, C3884, C3885, C3886, C3887, C3888, C3889,
        C3890, C3891, C3892, C3893, C3894, C3895, C3896, C3897, C3898, C3899,
        C3900, C3901, C3902, C3903, C3904, C3905, C3906, C3907, C3908, C3909,
        C3910, C3911, C3912, C3913, C3914, C3915, C3916, C3917, C3918, C3919,
        C3920, C3921, C3922, C3923, C3924, C3925, C3926, C3927, C3928, C3929,
        C3930, C3931, C3932, C3933, C3934, C3935, C3936, C3937, C3938, C3939,
        C3940, C3941, C3942, C3943, C3944, C3945, C3946, C3947, C3948, C3949,
        C3950, C3951, C3952, C3953, C3954, C3955, C3956, C3957, C3958, C3959,
        C3960, C3961, C3962, C3963, C3964, C3965, C3966, C3967, C3968, C3969,
        C3970, C3971, C3972, C3973, C3974, C3975, C3976, C3977, C3978, C3979,
        C3980, C3981, C3982, C3983, C3984, C3985, C3986, C3987, C3988, C3989,
        C3990, C3991, C3992, C3993, C3994, C3995, C3996, C3997, C3998, C3999,
        C4000, C4001, C4002, C4003, C4004, C4005, C4006, C4007, C4008, C4009,
        C4010, C4011, C4012, C4013, C4014, C4015, C4016, C4017, C4018, C4019,
        C4020, C4021, C4022, C4023, C4024, C4025, C4026, C4027, C4028, C4029,
        C4030, C4031, C4032, C4033, C4034, C4035, C4036, C4037, C4038, C4039,
        C4040, C4041, C4042, C4043, C4044, C4045, C4046, C4047, C4048, C4049,
        C4050, C4051, C4052, C4053, C4054, C4055, C4056, C4057, C4058, C4059,
        C4060, C4061, C4062, C4063, C4064, C4065, C4066, C4067, C4068, C4069,
        C4070, C4071, C4072, C4073, C4074, C4075, C4076, C4077, C4078, C4079,
        C4080, C4081, C4082, C4083, C4084, C4085, C4086, C4087, C4088, C4089,
        C4090, C4091)>;

def I0  : RegisterClass<"LPU", [i1],  64, (add ANYC)>; // 0 bit LIC...
def I1  : RegisterClass<"LPU", [i1],  64, (add ANYC)>;
def I8  : RegisterClass<"LPU", [i8],  64, (add ANYC)>;
def I16 : RegisterClass<"LPU", [i16], 64, (add ANYC)>;
def I32 : RegisterClass<"LPU", [i32], 64, (add ANYC)>;
def I64 : RegisterClass<"LPU", [i64], 64, (add ANYC)>;
def F16 : RegisterClass<"LPU", [f16], 64, (add ANYC)>;
def F32 : RegisterClass<"LPU", [f32], 64, (add ANYC)>;
def F64 : RegisterClass<"LPU", [f64], 64, (add ANYC)>;
