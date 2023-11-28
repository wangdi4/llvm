//===----------------------------------------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interface to the XML coverage renderer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_COV_INTEL_COVERAGEREPORTXML_H
#define LLVM_COV_INTEL_COVERAGEREPORTXML_H

#include "CoverageFilters.h"
#include "CoverageSummaryInfo.h"
#include "CoverageViewOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ProfileData/Coverage/CoverageMapping.h"

namespace llvm {

class CoverageReportXML {
public:
  CoverageReportXML(const CoverageViewOptions &Options,
                    const coverage::CoverageMapping &Coverage)
      : Options(Options), Coverage(Coverage) {}

  CoverageReportXML(const CoverageReportXML &) = delete;
  CoverageReportXML(CoverageReportXML &&) = delete;
  CoverageReportXML &operator=(const CoverageReportXML &) = delete;
  CoverageReportXML &operator=(CoverageReportXML &&) = delete;

  // Render file reports for every unique file in the coverage mapping.
  void renderFileReports(raw_ostream &OS,
                         const CoverageFilters &IgnoreFilenameFilters) const;

  // Render file reports for the files specified in \p Files.
  void renderFileReports(raw_ostream &OS, ArrayRef<std::string> Files) const;

  // Render file reports for the files specified in \p Files and the functions
  /// in \p Filters.
  void renderFileReports(raw_ostream &OS, ArrayRef<std::string> Files,
                         const CoverageFiltersMatchAll &Filters) const;

private:
  class XMLTag;

  // These names are used for the XML tags
  const char *ProjectTagName = "PROJECT";
  const char *ModuleTagName = "MODULE";
  const char *FunctionTagName = "FUNCTION";
  const char *BlocksTagName = "BLOCKS";
  const char *BlockTagName = "BLOCK";
  const char *InstanceTagName = "INSTANCE";

  // These names are used for attributes of a tag
  const char *NameAttrName = "name";
  const char *FreqAttrName = "freq";
  const char *TotalBlocksAttrName = "total";
  const char *CoveredBlockCountAttrName = "covered";
  const char *CoveragePercentageAttrName = "coverage";
  const char *LineStartAttrName = "line";
  const char *ColumnStartAttrName = "col";
  const char *LineEndAttrName = "end_line";
  const char *ColumnEndAttrName = "end_col";
  const char *InstIdAttrName = "id";
  const char *MacroExpFileAttrName = "macro_file";
  const char *MacroExpLineStartAttrName = "macro_line";
  const char *MacroExpColStartAttrName = "macro_col";
  const char *MacroExpLineEndAttrName = "end_macro_line";
  const char *MacroExpColEndAttrName = "end_macro_col";

  const CoverageViewOptions &Options;
  const coverage::CoverageMapping &Coverage;

  void
  renderFunction(raw_ostream &OS, XMLTag &ModuleTag,
                 const coverage::FunctionRecord *F,
                 const FunctionCoverageSummary &InstantiationSummary) const;

  std::string DemangleName(StringRef Name) const;

  // Helper struct that is used to hold information about location
  // that will be included in the report.
  struct OutputRecord {
    OutputRecord(coverage::LineColPair StartLoc, coverage::LineColPair EndLoc,
                 uint64_t ExecCount)
        : StartLoc(StartLoc), EndLoc(EndLoc) {
      addInstance(ExecCount);
    }

    OutputRecord(coverage::LineColPair StartLoc, coverage::LineColPair EndLoc,
                 uint64_t ExecCount, uint32_t MacroFile,
                 coverage::LineColPair MacroDefStartLoc,
                 coverage::LineColPair MacroDefEndLoc)
        : StartLoc(StartLoc), EndLoc(EndLoc) {
      addInstance(ExecCount, MacroFile, MacroDefStartLoc, MacroDefEndLoc);
    }

    OutputRecord(const OutputRecord &) = default;
    OutputRecord(OutputRecord &&) = default;
    OutputRecord &operator=(const OutputRecord &) = default;
    OutputRecord &operator=(OutputRecord &&) = default;

    // To handle reporting multiple instances that map to the same location,
    // keep a instance record for each.
    struct OutputInstance {
      OutputInstance(uint64_t ExecCount) : ExecCount(ExecCount) {}
      OutputInstance(uint64_t ExecCount, uint32_t MacroFile,
                     coverage::LineColPair MacroDefStartLoc,
                     coverage::LineColPair MacroDefEndLoc)
          : ExecCount(ExecCount), MacroFile(MacroFile),
            MacroDefStartLoc(MacroDefStartLoc), MacroDefEndLoc(MacroDefEndLoc) {
      }

      OutputInstance(const OutputInstance &) = default;
      OutputInstance(OutputInstance &&) = default;
      OutputInstance &operator=(const OutputInstance &) = default;
      OutputInstance &operator=(OutputInstance &&) = default;

      uint64_t ExecCount;
      std::optional<uint32_t> MacroFile;
      std::optional<coverage::LineColPair> MacroDefStartLoc;
      std::optional<coverage::LineColPair> MacroDefEndLoc;
    };

    void addInstance(uint64_t ExecCount) {
      Instances.push_back(OutputInstance(ExecCount));
    }
    void addInstance(uint64_t ExecCount, uint32_t MacroFile,
                     coverage::LineColPair MacroDefStartLoc,
                     coverage::LineColPair MacroDefEndLoc) {
      Instances.push_back(OutputInstance(ExecCount, MacroFile, MacroDefStartLoc,
                                         MacroDefEndLoc));
    }

    coverage::LineColPair StartLoc;
    coverage::LineColPair EndLoc;
    SmallVector<OutputInstance, 1> Instances;

    // Provide sorting support.
    bool operator<(const OutputRecord &other) const {
      if (StartLoc != other.StartLoc)
        return StartLoc < other.StartLoc;
      return EndLoc < other.EndLoc;
    }
  };

  // Helper class for rendering the XML-like content that only
  // supports the basic functionality needed for this report,
  // which are 'tags' and string/integer 'attributes'.
  class XMLTag {
  public:
    XMLTag(StringRef Tag, XMLTag *Parent = nullptr);
    ~XMLTag();

    XMLTag(const XMLTag &) = delete;
    XMLTag(XMLTag &&) = delete;
    XMLTag &operator=(const XMLTag &) = delete;
    XMLTag &operator=(XMLTag &&) = delete;

    bool isOpen() const { return IsOpen; }
    uint32_t getLevel() const { return Level; }

    void renderOpen(raw_ostream &OS);
    void renderClose(raw_ostream &OS);

    void addAttribute(StringRef Name, StringRef Value);
    void addAttribute(StringRef Name, uint64_t Value);

  private:
    // Return a string that has replaced the special characters '<', '>', '&'
    // with the escape sequences needed for XML.
    std::string replaceSpecialCharacters(StringRef Value);

    StringRef TagName;
    XMLTag *Parent;

    // Key-Value pairs for attributes to emit
    SmallVector<std::pair<StringRef, std::string>> Attributes;

    uint32_t Level = 0;
    bool IsOpen = false;
  };
};

} // namespace llvm

#endif // LLVM_COV_INTEL_COVERAGEREPORTXML_H
