// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++11 -triple i386-pc-linux
// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++14 -triple i386-pc-linux
// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++11 -triple x86_64-pc-linux
// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++14 -triple x86_64-pc-linux
constexpr char h = '\u5678'; //expected-warning {{implicit conversion from}} 
constexpr auto a = 'Ã¼';  //expected-warning {{extraneous characters in character constant ignored}}
   
void foo() {
  switch (a) {
  case 'Ã¼':  //expected-warning {{extraneous characters in character constant ignored}}
    break;
  }
  static_assert(sizeof(a) >= 2, "sizeof(a)");
  static_assert(a == 'Ã¼', "value of a");//expected-warning {{extraneous characters in character constant ignored}}
  static_assert(sizeof(h) == 1, "sizeof(h)");

#define CS_ISOLatin1_atilde '\341'
#define CS_ISOLatin1_etilde '\351'
#define CS_ISOLatin1_itilde '\355'
#define CS_ISOLatin1_otilde '\363'
#define CS_ISOLatin1_utilde '\372'
#define CS_ISOLatin1_uuml   '\374'
#define CS_ISOLatin1_ntilde '\361'
#define CS_ISOLatin1_Atilde '\301'
#define CS_ISOLatin1_Etilde '\311'
#define CS_ISOLatin1_Itilde '\315'
#define CS_ISOLatin1_Otilde '\323'
#define CS_ISOLatin1_Utilde '\332'
#define CS_ISOLatin1_Uuml   '\334'
#define CS_ISOLatin1_Ntilde '\321'
#define CS_ISOLatin1_iexcl  '\241'
#define CS_ISOLatin1_iquest '\277'
#define CS_ISOLatin1_ordf   '\252'
#define CS_ISOLatin1_ordm   '\272'
#define CS_ISOLatin1_deg    '\260'
#define CS_atilde CS_ISOLatin1_atilde
#define CS_etilde CS_ISOLatin1_etilde
#define CS_itilde CS_ISOLatin1_itilde
#define CS_otilde CS_ISOLatin1_otilde
#define CS_utilde CS_ISOLatin1_utilde
#define CS_uuml   CS_ISOLatin1_uuml
#define CS_ntilde CS_ISOLatin1_ntilde
#define CS_Atilde CS_ISOLatin1_Atilde
#define CS_Etilde CS_ISOLatin1_Etilde
#define CS_Itilde CS_ISOLatin1_Itilde
#define CS_Otilde CS_ISOLatin1_Otilde
#define CS_Utilde CS_ISOLatin1_Utilde
#define CS_Uuml   CS_ISOLatin1_Uuml
#define CS_Ntilde CS_ISOLatin1_Ntilde
#define CS_iexcl  CS_ISOLatin1_iexcl
#define CS_iquest CS_ISOLatin1_iquest
#define CS_ordf   CS_ISOLatin1_ordf
#define CS_ordm   CS_ISOLatin1_ordm
#define CS_deg    CS_ISOLatin1_deg


  switch (a) {
  case 'a': 
  case 'e': 
  case 'i': 
  case 'o': 
  case 'u': 
  case 'Ã¼'://expected-warning {{extraneous characters in character constant ignored}}
  case CS_otilde:
  case CS_utilde:
  case CS_uuml: 
  case 'v':
  case 'b':
  case 'c':
  case 'd':
  case 'f':
  case 'g':
  case 'h':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
  case CS_ISOLatin1_atilde:
  case CS_ISOLatin1_etilde:
  case CS_ISOLatin1_itilde:
  case CS_ISOLatin1_ntilde:
  case CS_ISOLatin1_Atilde:
  case CS_ISOLatin1_Etilde:
  case CS_ISOLatin1_Itilde:
  case CS_ISOLatin1_Otilde:
  case CS_ISOLatin1_Utilde:
  case CS_ISOLatin1_Uuml  :
  case CS_ISOLatin1_Ntilde:
  case CS_ISOLatin1_iexcl :
  case CS_ISOLatin1_iquest:
  case CS_ISOLatin1_ordf  :
  case CS_ISOLatin1_ordm  :
  case CS_ISOLatin1_deg   :
    break;
  }
}
