#ifdef INTEL_SPECIFIC_IL0_BACKEND

#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/CodeCompletionHandler.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Pragma.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SaveAndRestore.h"
using namespace clang;

/// \brief Push a token onto the token stream containing an annotation.
static void EnterAnnotationToken(Preprocessor &PP, SourceLocation Begin,
                                 SourceLocation End, tok::TokenKind Kind,
                                 void *AnnotationVal) {
  // FIXME: Produce this as the current token directly, rather than
  // allocating a new token for it.
  Token Tok;
  Token *T = &Tok;
  Tok.startToken();
  Tok.setKind(Kind);
  Tok.setLocation(Begin);
  Tok.setAnnotationEndLoc(End);
  Tok.setAnnotationValue(AnnotationVal);
  PP.EnterTokenStream(ArrayRef<Token>(T, 1), true);
}

/// \brief Produce a diagnostic informing the user that a #include or similar
/// was implicitly treated as a module import.
static void diagnoseAutoModuleImport(
    Preprocessor &PP, SourceLocation HashLoc, Token &IncludeTok,
    ArrayRef<std::pair<IdentifierInfo *, SourceLocation>> Path,
    SourceLocation PathEnd) {
  assert(PP.getLangOpts().ObjC2 && "no import syntax available");

  SmallString<128> PathString;
  for (unsigned I = 0, N = Path.size(); I != N; ++I) {
    if (I)
      PathString += '.';
    PathString += Path[I].first->getName();
  }
  int IncludeKind = 0;

  switch (IncludeTok.getIdentifierInfo()->getPPKeywordID()) {
  case tok::pp_include:
    IncludeKind = 0;
    break;

  case tok::pp_import:
    IncludeKind = 1;
    break;

  case tok::pp_include_next:
    IncludeKind = 2;
    break;

  case tok::pp___include_macros:
    IncludeKind = 3;
    break;

  default:
    llvm_unreachable("unknown include directive kind");
  }

  CharSourceRange ReplaceRange(SourceRange(HashLoc, PathEnd),
                               /*IsTokenRange=*/false);
  PP.Diag(HashLoc, diag::warn_auto_module_import)
      << IncludeKind << PathString
      << FixItHint::CreateReplacement(ReplaceRange,
                                      ("@import " + PathString + ";").str());
}

void Preprocessor::ParseStartMapRegion(SourceLocation HashLoc, Token &FilenameTok) {
  // Reserve a buffer to get the spelling.
  SmallString<128> FilenameBuffer;
  StringRef Filename;
  SourceLocation End;
  SourceLocation CharEnd; // the end of this directive, in characters

  Filename = getSpelling(FilenameTok, FilenameBuffer);
  End = FilenameTok.getLocation();
  CharEnd = End.getLocWithOffset(Filename.size());
  CharSourceRange FilenameRange =
    CharSourceRange::getCharRange(FilenameTok.getLocation(), CharEnd);
  StringRef OriginalFilename = Filename;
  bool isAngled =
    GetIncludeFilenameSpelling(FilenameTok.getLocation(), Filename);
  // If GetIncludeFilenameSpelling set the start ptr to null, there was an
  // error.
  if (Filename.empty()) {
    DiscardUntilEndOfDirective();
    return;
  }

  // Check that we don't have infinite #include recursion.
  if (IncludeMacroStack.size() == MaxAllowedIncludeStackDepth-1) {
    Diag(FilenameTok, diag::err_pp_include_too_deep);
    return;
  }

  if (HeaderInfo.HasIncludeAliasMap()) {
    // Map the filename with the brackets still attached.  If the name doesn't 
    // map to anything, fall back on the filename we've already gotten the 
    // spelling for.
    StringRef NewName = HeaderInfo.MapHeaderToIncludeAlias(OriginalFilename);
    if (!NewName.empty())
      Filename = NewName;
  }

  // Search include directories.
  const DirectoryLookup *CurDir;
  SmallString<1024> SearchPath;
  SmallString<1024> RelativePath;
  // We get the raw path only if we have 'Callbacks' to which we later pass
  // the path.
  ModuleMap::KnownHeader SuggestedModule;
  SourceLocation FilenameLoc = FilenameTok.getLocation();
  SmallString<128> NormalizedPath;
  if (LangOpts.MSVCCompat) {
    NormalizedPath = Filename.str();
#ifndef LLVM_ON_WIN32
    llvm::sys::path::native(NormalizedPath);
#endif
  }
  const FileEntry *File = LookupFile(
      FilenameLoc, LangOpts.MSVCCompat ? NormalizedPath.c_str() : Filename,
      isAngled, nullptr, nullptr, CurDir, Callbacks ? &SearchPath : nullptr,
      Callbacks ? &RelativePath : nullptr,
      HeaderInfo.getHeaderSearchOpts().ImplicitModuleMaps ? &SuggestedModule
                                                          : nullptr);

  if (!File) {
    if (Callbacks) {
      // Give the clients a chance to recover.
      SmallString<128> RecoveryPath;
      if (Callbacks->FileNotFound(Filename, RecoveryPath)) {
        if (const DirectoryEntry *DE = FileMgr.getDirectory(RecoveryPath)) {
          // Add the recovery path to the list of search paths.
          DirectoryLookup DL(DE, SrcMgr::C_User, false);
          HeaderInfo.AddSearchPath(DL, isAngled);

          // Try the lookup again, skipping the cache.
          File = LookupFile(
              FilenameLoc,
              LangOpts.MSVCCompat ? NormalizedPath.c_str() : Filename, isAngled,
              nullptr, nullptr, CurDir, nullptr, nullptr,
              HeaderInfo.getHeaderSearchOpts().ImplicitModuleMaps
                  ? &SuggestedModule
                  : nullptr,
              /*SkipCache*/ true);
        }
      }
    }

    if (!SuppressIncludeNotFoundError) {
      // If the file could not be located and it was included via angle 
      // brackets, we can attempt a lookup as though it were a quoted path to
      // provide the user with a possible fixit.
      if (isAngled) {
        File = LookupFile(
            FilenameLoc,
            LangOpts.MSVCCompat ? NormalizedPath.c_str() : Filename, false,
            nullptr, nullptr, CurDir, Callbacks ? &SearchPath : nullptr,
            Callbacks ? &RelativePath : nullptr,
            HeaderInfo.getHeaderSearchOpts().ImplicitModuleMaps
                ? &SuggestedModule
                : nullptr);
        if (File) {
          SourceRange Range(FilenameTok.getLocation(), CharEnd);
          Diag(FilenameTok, diag::err_pp_file_not_found_not_fatal) << 
            Filename << 
            FixItHint::CreateReplacement(Range, "\"" + Filename.str() + "\"");
        }
      }

      // If the file is still not found, just go with the vanilla diagnostic
      if (!File)
        Diag(FilenameTok, diag::err_pp_file_not_found) << Filename;
    }
  }

  // Should we enter the source file? Set to false if either the source file is
  // known to have no effect beyond its effect on module visibility -- that is,
  // if it's got an include guard that is already defined or is a modular header
  // we've imported or already built.
  bool ShouldEnter = true;

  // Determine whether we should try to import the module for this #include, if
  // there is one. Don't do so if precompiled module support is disabled or we
  // are processing this module textually (because we're building the module).
  if (File && SuggestedModule && getLangOpts().Modules &&
      SuggestedModule.getModule()->getTopLevelModuleName() !=
          getLangOpts().CurrentModule) {
    // If this include corresponds to a module but that module is
    // unavailable, diagnose the situation and bail out.
    if (!SuggestedModule.getModule()->isAvailable()) {
      clang::Module::Requirement Requirement;
      clang::Module::UnresolvedHeaderDirective MissingHeader;
      Module *M = SuggestedModule.getModule();
      // Identify the cause.
      (void)M->isAvailable(getLangOpts(), getTargetInfo(), Requirement,
                           MissingHeader);
      if (MissingHeader.FileNameLoc.isValid()) {
        Diag(MissingHeader.FileNameLoc, diag::err_module_header_missing)
            << MissingHeader.IsUmbrella << MissingHeader.FileName;
      } else {
        Diag(M->DefinitionLoc, diag::err_module_unavailable)
            << M->getFullModuleName() << Requirement.second
            << Requirement.first;
      }
      Diag(FilenameTok.getLocation(),
           diag::note_implicit_top_level_module_import_here)
          << M->getTopLevelModuleName();
      return;
    }

    // Compute the module access path corresponding to this module.
    // FIXME: Should we have a second loadModule() overload to avoid this
    // extra lookup step?
    SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> Path;
    for (Module *Mod = SuggestedModule.getModule(); Mod; Mod = Mod->Parent)
      Path.push_back(std::make_pair(getIdentifierInfo(Mod->Name),
                                    FilenameTok.getLocation()));
    std::reverse(Path.begin(), Path.end());

    // Warn that we're replacing the include/import with a module import.
    // We only do this in Objective-C, where we have a module-import syntax.
    if (getLangOpts().ObjC2)
      diagnoseAutoModuleImport(*this, HashLoc, FilenameTok, Path, CharEnd);

    // Load the module to import its macros. We'll make the declarations
    // visible when the parser gets here.
    // FIXME: Pass SuggestedModule in here rather than converting it to a path
    // and making the module loader convert it back again.
    ModuleLoadResult Imported = TheModuleLoader.loadModule(
        FilenameTok.getLocation(), Path, Module::Hidden,
        /*IsIncludeDirective=*/true);
    assert((Imported == nullptr || Imported == SuggestedModule.getModule()) &&
           "the imported module is different than the suggested one");

    if (Imported)
      ShouldEnter = false;
    else if (Imported.isMissingExpected()) {
      // We failed to find a submodule that we assumed would exist (because it
      // was in the directory of an umbrella header, for instance), but no
      // actual module exists for it (because the umbrella header is
      // incomplete).  Treat this as a textual inclusion.
      SuggestedModule = ModuleMap::KnownHeader();
    } else {
      // We hit an error processing the import. Bail out.
      if (hadModuleLoaderFatalFailure()) {
        // With a fatal failure in the module loader, we abort parsing.
        Token &Result = FilenameTok;
        if (CurLexer) {
          Result.startToken();
          CurLexer->FormTokenWithChars(Result, CurLexer->BufferEnd, tok::eof);
          CurLexer->cutOffLexing();
        } else {
          assert(CurPTHLexer && "#include but no current lexer set!");
          CurPTHLexer->getEOF(Result);
        }
      }
      return;
    }
  }

  if (Callbacks) {
    // Notify the callback object that we've seen an inclusion directive.
    Callbacks->InclusionDirective(
        HashLoc, FilenameTok,
        LangOpts.MSVCCompat ? NormalizedPath.c_str() : Filename, isAngled,
        FilenameRange, File, SearchPath, RelativePath,
        ShouldEnter ? nullptr : SuggestedModule.getModule());
  }

  if (!File)
    return;
  
  // The #included file will be considered to be a system header if either it is
  // in a system include directory, or if the #includer is a system include
  // header.
  SrcMgr::CharacteristicKind FileCharacter =
    std::max(HeaderInfo.getFileDirFlavor(File),
             SourceMgr.getFileCharacteristic(FilenameTok.getLocation()));

  // FIXME: If we have a suggested module, and we've already visited this file,
  // don't bother entering it again. We know it has no further effect.

  // Ask HeaderInfo if we should enter this #include file.  If not, #including
  // this file will have no effect.
  if (ShouldEnter &&
      !HeaderInfo.ShouldEnterIncludeFile(*this, File, /*IsImport*/ false,
                                         SuggestedModule.getModule())) {
    ShouldEnter = false;
    if (Callbacks)
      Callbacks->FileSkipped(*File, FilenameTok, FileCharacter);
  }

  // If we don't need to enter the file, stop now.
  if (!ShouldEnter) {
    // If this is a module import, make it visible if needed.
    if (auto *M = SuggestedModule.getModule()) {
      makeModuleVisible(M, HashLoc);

      if (FilenameTok.getIdentifierInfo()->getPPKeywordID() !=
          tok::pp___include_macros)
        EnterAnnotationToken(*this, HashLoc, End, tok::annot_module_include, M);
    }
    return;
  }

  // Look up the file, create a File ID for it.
  SourceLocation IncludePos = End;
  // If the filename string was the result of macro expansions, set the include
  // position on the file where it will be included and after the expansions.
  if (IncludePos.isMacroID())
    IncludePos = SourceMgr.getExpansionRange(IncludePos).second;
  FileID FID = SourceMgr.createFileID(File, IncludePos, FileCharacter);
  assert(!FID.isInvalid() && "Expected valid file ID");

  // If all is good, enter the new file!
  if (EnterSourceFile(FID, CurDir, FilenameTok.getLocation()))
    return;

  // Determine if we're switching to building a new submodule, and which one.
  if (auto *M = SuggestedModule.getModule()) {
    assert(!CurSubmodule && "should not have marked this as a module yet");
    CurSubmodule = M;

    // Let the macro handling code know that any future macros are within
    // the new submodule.
    EnterSubmodule(M, HashLoc);

    // Let the parser know that any future declarations are within the new
    // submodule.
    // FIXME: There's no point doing this if we're handling a #__include_macros
    // directive.
    EnterAnnotationToken(*this, HashLoc, End, tok::annot_module_begin, M);
  }
}

#endif  // INTEL_SPECIFIC_IL0_BACKEND
