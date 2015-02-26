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
  const FileEntry *File = LookupFile(
      FilenameLoc, Filename, isAngled, 0, 0, CurDir,
      Callbacks ? &SearchPath : 0, Callbacks ? &RelativePath : 0,
      getLangOpts().Modules? &SuggestedModule : 0);

  if (Callbacks) {
    if (!File) {
      // Give the clients a chance to recover.
      SmallString<128> RecoveryPath;
      if (Callbacks->FileNotFound(Filename, RecoveryPath)) {
        if (const DirectoryEntry *DE = FileMgr.getDirectory(RecoveryPath)) {
          // Add the recovery path to the list of search paths.
          DirectoryLookup DL(DE, SrcMgr::C_User, false);
          HeaderInfo.AddSearchPath(DL, isAngled);
          // Try the lookup again, skipping the cache.
          File = LookupFile(FilenameLoc, Filename, isAngled, 0, 0, CurDir, 0, 0,
                            getLangOpts().Modules? &SuggestedModule : 0,
                            /*SkipCache*/true);
        }
      }
    }

    if (!SuggestedModule || !getLangOpts().Modules) {
      // Notify the callback object that we've seen an inclusion directive.
      Callbacks->InclusionDirective(HashLoc, FilenameTok, Filename, isAngled, 
                                    FilenameRange, File,
                                    SearchPath, RelativePath,
                                    /*ImportedModule=*/0);
    }
  }

  if (File == 0) {
    if (!SuppressIncludeNotFoundError) {
      // If the file could not be located and it was included via angle
      // brackets, we can attempt a lookup as though it were a quoted path to
      // provide the user with a possible fixit.
      if (isAngled) {
        File = LookupFile(FilenameLoc, Filename, false, 0, 0, CurDir,
                          Callbacks ? &SearchPath : 0,
                          Callbacks ? &RelativePath : 0,
                          getLangOpts().Modules ? &SuggestedModule : 0);
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
    if (!File)
      return;
  }

  // If we are supposed to import a module rather than including the header,
  // do so now.
  if (SuggestedModule) {
    // Compute the module access path corresponding to this module.
    // FIXME: Should we have a second loadModule() overload to avoid this
    // extra lookup step?
    llvm::SmallVector<std::pair<IdentifierInfo *, SourceLocation>, 2> Path;
    for (Module *Mod = SuggestedModule.getModule(); Mod; Mod = Mod->Parent)
      Path.push_back(std::make_pair(getIdentifierInfo(Mod->Name),
                                    FilenameTok.getLocation()));
    std::reverse(Path.begin(), Path.end());

    // Warn that we're replacing the include/import with a module import.
    SmallString<128> PathString;
    for (unsigned I = 0, N = Path.size(); I != N; ++I) {
      if (I)
        PathString += '.';
      PathString += Path[I].first->getName();
    }
    int IncludeKind = 0;

    // Determine whether we are actually building the module that this
    // include directive maps to.
    bool BuildingImportedModule
      = Path[0].first->getName() == getLangOpts().CurrentModule;

    if (!BuildingImportedModule && getLangOpts().ObjC2) {
      // If we're not building the imported module, warn that we're going
      // to automatically turn this inclusion directive into a module import.
      // We only do this in Objective-C, where we have a module-import syntax.
      CharSourceRange ReplaceRange(SourceRange(HashLoc, CharEnd),
                                   /*IsTokenRange=*/false);
      Diag(HashLoc, diag::warn_auto_module_import)
        << IncludeKind << PathString
        << FixItHint::CreateReplacement(ReplaceRange,
             "@__experimental_modules_import " + PathString.str().str() + ";");
    }

    // Load the module.
    // If this was an #__include_macros directive, only make macros visible.
    Module::NameVisibilityKind Visibility
      = (IncludeKind == 3)? Module::MacrosVisible : Module::AllVisible;
    Module *Imported
      = TheModuleLoader.loadModule(FilenameTok.getLocation(), Path, Visibility,
                                   /*IsIncludeDirective=*/true);
    assert((Imported == 0 || Imported == SuggestedModule.getModule()) &&
           "the imported module is different than the suggested one");

    // If this header isn't part of the module we're building, we're done.
    if (!BuildingImportedModule && Imported) {
      if (Callbacks) {
        Callbacks->InclusionDirective(HashLoc, FilenameTok, Filename, isAngled,
                                      FilenameRange, File,
                                      SearchPath, RelativePath, Imported);
      }
      return;
    }
  }

  if (Callbacks && SuggestedModule) {
    // We didn't notify the callback object that we've seen an inclusion
    // directive before. Now that we are parsing the include normally and not
    // turning it to a module import, notify the callback object.
    Callbacks->InclusionDirective(HashLoc, FilenameTok, Filename, isAngled,
                                  FilenameRange, File,
                                  SearchPath, RelativePath,
                                  /*ImportedModule=*/0);
  }

  // The #included file will be considered to be a system header if either it is
  // in a system include directory, or if the #includer is a system include
  // header.
  SrcMgr::CharacteristicKind FileCharacter =
    std::max(HeaderInfo.getFileDirFlavor(File),
             SourceMgr.getFileCharacteristic(FilenameTok.getLocation()));

  // Ask HeaderInfo if we should enter this #include file.  If not, #including
  // this file will have no effect.
  if (!HeaderInfo.ShouldEnterIncludeFile(File, false)) {
    if (Callbacks)
      Callbacks->FileSkipped(*File, FilenameTok, FileCharacter);
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

  // Finally, if all is good, enter the new file!
  EnterSourceFile(FID, CurDir, FilenameTok.getLocation());
}

#endif  // INTEL_SPECIFIC_IL0_BACKEND
