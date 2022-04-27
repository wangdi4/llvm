//===--- COFFModuleDefinition.cpp - Simple DEF parser ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Windows-specific.
// A parser for the module-definition file (.def file).
//
// The format of module-definition files are described in this document:
// https://msdn.microsoft.com/en-us/library/28d6s79h.aspx
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/COFFModuleDefinition.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Object/COFFImportFile.h"
#include "llvm/Object/Error.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/Path.h"

using namespace llvm::COFF;
using namespace llvm;

namespace llvm {
namespace object {

enum Kind {
  Unknown,
  Eof,
  Identifier,
  Comma,
  Equal,
  EqualEqual,
  KwBase,
  KwConstant,
  KwData,
  KwExports,
  KwHeapsize,
  KwLibrary,
  KwName,
  KwNoname,
  KwPrivate,
  KwStacksize,
  KwVersion,
};

struct Token {
  explicit Token(Kind T = Unknown, StringRef S = "") : K(T), Value(S) {}
  Kind K;
  StringRef Value;
};

static bool isDecorated(StringRef Sym, bool MingwDef) {
  // In def files, the symbols can either be listed decorated or undecorated.
  //
  // - For cdecl symbols, only the undecorated form is allowed.
  // - For fastcall and vectorcall symbols, both fully decorated or
  //   undecorated forms can be present.
  // - For stdcall symbols in non-MinGW environments, the decorated form is
  //   fully decorated with leading underscore and trailing stack argument
  //   size - like "_Func@0".
  // - In MinGW def files, a decorated stdcall symbol does not include the
  //   leading underscore though, like "Func@0".

  // This function controls whether a leading underscore should be added to
  // the given symbol name or not. For MinGW, treat a stdcall symbol name such
  // as "Func@0" as undecorated, i.e. a leading underscore must be added.
  // For non-MinGW, look for '@' in the whole string and consider "_Func@0"
  // as decorated, i.e. don't add any more leading underscores.
  // We can't check for a leading underscore here, since function names
  // themselves can start with an underscore, while a second one still needs
  // to be added.
  return Sym.startswith("@") || Sym.contains("@@") || Sym.startswith("?") ||
         (!MingwDef && Sym.contains('@'));
}

class Lexer {
public:
  Lexer(StringRef S) : Buf(S) {}

  Token lex() {
    Buf = Buf.trim();
    if (Buf.empty())
      return Token(Eof);

    switch (Buf[0]) {
    case '\0':
      return Token(Eof);
    case ';': {
      size_t End = Buf.find('\n');
      Buf = (End == Buf.npos) ? "" : Buf.drop_front(End);
      return lex();
    }
    case '=':
      Buf = Buf.drop_front();
      if (Buf.startswith("=")) {
        Buf = Buf.drop_front();
        return Token(EqualEqual, "==");
      }
      return Token(Equal, "=");
    case ',':
      Buf = Buf.drop_front();
      return Token(Comma, ",");
    case '"': {
      StringRef S;
      std::tie(S, Buf) = Buf.substr(1).split('"');
      return Token(Identifier, S);
    }
    default: {
      size_t End = Buf.find_first_of("=,;\r\n \t\v");
      StringRef Word = Buf.substr(0, End);
      Kind K = llvm::StringSwitch<Kind>(Word)
                   .Case("BASE", KwBase)
                   .Case("CONSTANT", KwConstant)
                   .Case("DATA", KwData)
                   .Case("EXPORTS", KwExports)
                   .Case("HEAPSIZE", KwHeapsize)
                   .Case("LIBRARY", KwLibrary)
                   .Case("NAME", KwName)
                   .Case("NONAME", KwNoname)
                   .Case("PRIVATE", KwPrivate)
                   .Case("STACKSIZE", KwStacksize)
                   .Case("VERSION", KwVersion)
                   .Default(Identifier);
      Buf = (End == Buf.npos) ? "" : Buf.drop_front(End);
      return Token(K, Word);
    }
    }
  }

private:
  StringRef Buf;
};

class Parser {
public:
  explicit Parser(StringRef S, MachineTypes M, bool B)
      : Lex(S), Machine(M), MingwDef(B) {}

  Expected<COFFModuleDefinition> parse() {
    do {
      if (Error Err = parseOne())
        return std::move(Err);
    } while (Tok.K != Eof);
    return Info;
  }

private:
  void read() {
    if (Stack.empty()) {
      Tok = Lex.lex();
      return;
    }
    Tok = Stack.back();
    Stack.pop_back();
  }

  Error readAsInt(uint64_t *I) {
    read();
    if (Tok.K != Identifier || Tok.Value.getAsInteger(10, *I))
      return createError("integer expected");
    return Error::success();
  }

  Error expect(Kind Expected, StringRef Msg) {
    read();
    if (Tok.K != Expected)
      return createError(Msg);
    return Error::success();
  }

  void unget() { Stack.push_back(Tok); }

  Error parseOne() {
    read();
    switch (Tok.K) {
    case Eof:
      return Error::success();
    case KwExports:
      for (;;) {
        read();
        if (Tok.K != Identifier) {
          unget();
          return Error::success();
        }
        if (Error Err = parseExport())
          return Err;
      }
    case KwHeapsize:
      return parseNumbers(&Info.HeapReserve, &Info.HeapCommit);
    case KwStacksize:
      return parseNumbers(&Info.StackReserve, &Info.StackCommit);
    case KwLibrary:
    case KwName: {
      bool IsDll = Tok.K == KwLibrary; // Check before parseName.
      std::string Name;
      if (Error Err = parseName(&Name, &Info.ImageBase))
        return Err;

      Info.ImportName = Name;

      // Set the output file, but don't override /out if it was already passed.
      if (Info.OutputFile.empty()) {
        Info.OutputFile = Name;
        // Append the appropriate file extension if not already present.
        if (!sys::path::has_extension(Name))
          Info.OutputFile += IsDll ? ".dll" : ".exe";
      }

      return Error::success();
    }
    case KwVersion:
      return parseVersion(&Info.MajorImageVersion, &Info.MinorImageVersion);
    default:
      return createError("unknown directive: " + Tok.Value);
    }
  }

  Error parseExport() {
    COFFShortExport E;
    E.Name = std::string(Tok.Value);
    read();
    if (Tok.K == Equal) {
      read();
      if (Tok.K != Identifier)
        return createError("identifier expected, but got " + Tok.Value);
      E.ExtName = E.Name;
      E.Name = std::string(Tok.Value);
    } else {
      unget();
    }

    if (Machine == IMAGE_FILE_MACHINE_I386) {
      if (!isDecorated(E.Name, MingwDef))
        E.Name = (std::string("_").append(E.Name));
      if (!E.ExtName.empty() && !isDecorated(E.ExtName, MingwDef))
        E.ExtName = (std::string("_").append(E.ExtName));
    }

    for (;;) {
      read();
      if (Tok.K == Identifier && Tok.Value[0] == '@') {
        if (Tok.Value == "@") {
          // "foo @ 10"
          read();
          Tok.Value.getAsInteger(10, E.Ordinal);
        } else if (Tok.Value.drop_front().getAsInteger(10, E.Ordinal)) {
          // "foo \n @bar" - Not an ordinal modifier at all, but the next
          // export (fastcall decorated) - complete the current one.
          unget();
          Info.Exports.push_back(E);
          return Error::success();
        }
        // "foo @10"
        read();
        if (Tok.K == KwNoname) {
          E.Noname = true;
        } else {
          unget();
        }
        continue;
      }
      if (Tok.K == KwData) {
        E.Data = true;
        continue;
      }
      if (Tok.K == KwConstant) {
        E.Constant = true;
        continue;
      }
      if (Tok.K == KwPrivate) {
        E.Private = true;
        continue;
      }
      if (Tok.K == EqualEqual) {
        read();
        E.AliasTarget = std::string(Tok.Value);
        if (Machine == IMAGE_FILE_MACHINE_I386 && !isDecorated(E.AliasTarget, MingwDef))
          E.AliasTarget = std::string("_").append(E.AliasTarget);
        continue;
      }
      unget();
      Info.Exports.push_back(E);
      return Error::success();
    }
  }

  // HEAPSIZE/STACKSIZE reserve[,commit]
  Error parseNumbers(uint64_t *Reserve, uint64_t *Commit) {
    if (Error Err = readAsInt(Reserve))
      return Err;
    read();
    if (Tok.K != Comma) {
      unget();
      Commit = nullptr;
      return Error::success();
    }
    if (Error Err = readAsInt(Commit))
      return Err;
    return Error::success();
  }

  // NAME outputPath [BASE=address]
  Error parseName(std::string *Out, uint64_t *Baseaddr) {
    read();
    if (Tok.K == Identifier) {
      *Out = std::string(Tok.Value);
    } else {
      *Out = "";
      unget();
      return Error::success();
    }
    read();
    if (Tok.K == KwBase) {
      if (Error Err = expect(Equal, "'=' expected"))
        return Err;
      if (Error Err = readAsInt(Baseaddr))
        return Err;
    } else {
      unget();
      *Baseaddr = 0;
    }
    return Error::success();
  }

  // VERSION major[.minor]
  Error parseVersion(uint32_t *Major, uint32_t *Minor) {
    read();
    if (Tok.K != Identifier)
      return createError("identifier expected, but got " + Tok.Value);
    StringRef V1, V2;
    std::tie(V1, V2) = Tok.Value.split('.');
    if (V1.getAsInteger(10, *Major))
      return createError("integer expected, but got " + Tok.Value);
    if (V2.empty())
      *Minor = 0;
    else if (V2.getAsInteger(10, *Minor))
      return createError("integer expected, but got " + Tok.Value);
    return Error::success();
  }

  Lexer Lex;
  Token Tok;
  std::vector<Token> Stack;
  MachineTypes Machine;
  COFFModuleDefinition Info;
  bool MingwDef;
};

Expected<COFFModuleDefinition> parseCOFFModuleDefinition(MemoryBufferRef MB,
                                                         MachineTypes Machine,
                                                         bool MingwDef) {
#if INTEL_CUSTOMIZATION
  // This enumerator represents which type of UTF byte order mark is being used
  enum UTFBOMType {
    UTF8BOM,                   // UTF-8 BOM
    UTF16BOMLE,                // UTF-16 BOM little endian
    UTF16BOMBE,                // UTF-16 BOM big endian
    UTF32BOMLE,                // UTF-32 BOM little endian
    UTF32BOMBE,                // UTF-32 BOM big endian
    UTFNoBOM,                  // No BOM bytes detected
  };

  // Return the byte order mark identified in the input string. If there is no
  // byte order mark, then return UTFNoBOM
  auto CollectBOMType = [&](StringRef Buf) -> UTFBOMType {
    if (Buf.empty())
      return UTFNoBOM;

    size_t BufferSize = Buf.size();

    // UTF-16 use 2 characters for byte order mark
    if (BufferSize < 2)
      return UTFNoBOM;

    // UTF-16 BOM for little endian use 2 characters at the beginning. The
    // hexadecimal sequence is FF FE
    if (Buf[0] == '\xff' && Buf[1] == '\xfe')
      return UTF16BOMLE;

    // UTF-16 BOM big endian use the sequence FE FF
    if (Buf[0] == '\xfe' && Buf[1] == '\xff')
      return UTF16BOMBE;

    // UTF-8 use 3 characters
    if (BufferSize < 3)
      return UTFNoBOM;

    // UTF-8 BOM hexadecimal sequence is EF BB BF.
    if (Buf[0] == '\xef' && Buf[1] == '\xbb' && Buf[2] == '\xbf')
      return UTF8BOM;

    // UTF-32 use 4 characters
    if (BufferSize < 4)
      return UTFNoBOM;

    // UTF-32 little endian sequence is FF FE 00 00
    if (Buf[0] == '\xff' && Buf[1] == '\xfe' &&
        Buf[2] == '\x00' && Buf[3] == '\x00')
      return UTF32BOMLE;

    // UTF-32 big endian sequence is 00 00 FE FF
    if (Buf[0] == '\x00' && Buf[1] == '\x00' &&
        Buf[2] == '\xfe' && Buf[3] == '\xff')
      return UTF32BOMBE;

    // No byte order mark was identified
    return UTFNoBOM;
  };

  StringRef Buf = MB.getBuffer();

  // Identify if there is work to be done depending on the byte order mark.
  switch (CollectBOMType(MB.getBuffer())) {
  // Handle UTF-8 case, which is just drop the first 3 characters
  case UTF8BOM:
    Buf = MB.getBuffer().drop_front(3);
    break;
  // TODO: Handle the encoding for UTF-16. Perhaps we may want to convert
  // to UTF-8 since it is compatible with ASCII.
  case UTF16BOMLE:
  case UTF16BOMBE:
    Buf = MB.getBuffer().drop_front(2);
    break;
  // TODO: Handle the encoding for UTF-32. Perhaps we may want to convert
  // to UTF-8 since it is compatible with ASCII.
  case UTF32BOMLE:
  case UTF32BOMBE:
    Buf = MB.getBuffer().drop_front(4);
    break;
  case UTFNoBOM:
    break;
  }

  // NOTE: If the file doesn't have byte order mark it doesn't mean is not
  // encoded in UTF. These are the rules from Microsoft when using UTF-X
  // encoding:
  //
  // - UTF-8: file must have BOM
  // - UTF-16:
  //   - Little endian with or without BOM
  //   - Big endian with or without BOM
  // - UTF-32: Not specified
  //
  // In simple words, all files with UTF-8 must have byte order mark, but for
  // UTF-16 there is no requirement. As for UTF-32, there is no specifications
  // in the Microsoft documentation. Also, it is not common to encode files in
  // UTF-32 nor do most of the text editors support it, but CL, clang and
  // MS-LINK support it.

  return Parser(Buf, Machine, MingwDef).parse();
  // return Parser(MB.getBuffer(), Machine, MingwDef).parse();
#endif // INTEL_CUSTOMIZATION
}

} // namespace object
} // namespace llvm
