//===-- x86_archive.cpp - OpenCL Archive interface -----------------===//
//
// Copyright:  (c) 2011 by Apple, Inc., All Rights Reserved.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "llvm/Bitcode/Archive.h"
#include "llvm/Support/FormattedStream.h"
#include <stdio.h>
#include <utility>
#include <vector>
#include "x86_archive.h"

using namespace llvm;

// The following is taken from ArchiveInternals.h so we can match the structure
// until we update the Archive to support memory members as well as file
// members.
#define ARFILE_MAGIC "!<arch>\n"                   ///< magic string
#define ARFILE_MAGIC_LEN (sizeof(ARFILE_MAGIC)-1)  ///< length of magic string
#define ARFILE_SVR4_SYMTAB_NAME "/               " ///< SVR4 symtab entry name
#define ARFILE_LLVM_SYMTAB_NAME "#_LLVM_SYM_TAB_#" ///< LLVM symtab entry name
#define ARFILE_BSD4_SYMTAB_NAME "__.SYMDEF SORTED" ///< BSD4 symtab entry name
#define ARFILE_STRTAB_NAME      "//              " ///< Name of string table
#define ARFILE_PAD "\n"                            ///< inter-file align padding
#define ARFILE_MEMBER_MAGIC "`\n"                  ///< fmag field magic #

/// The ArchiveMemberHeader structure is used internally for bitcode
/// archives.
/// The header precedes each file member in the archive. This structure is
/// defined using character arrays for direct and correct interpretation
/// regardless of the endianess of the machine that produced it.
/// @brief Archive File Member Header
class CLArchiveMemberHeader {
  /// @name Data
  /// @{
public:
  char name[16];  ///< Name of the file member.
  char date[12];  ///< File date, decimal seconds since Epoch
  char uid[6];    ///< user id in ASCII decimal
  char gid[6];    ///< group id in ASCII decimal
  char mode[8];   ///< file mode in ASCII octal
  char size[10];  ///< file size in ASCII decimal
  char fmag[2];   ///< Always contains ARFILE_MAGIC_TERMINATOR
  
  /// @}
  /// @name Methods
  /// @{
public:
  CLArchiveMemberHeader(int sz) {
    init();
    char buffer[32];
    if (sz < 0) {
      buffer[0] = '-';
      sprintf(&buffer[1],"%-9u",(unsigned)-sz);
    } else {
      sprintf(buffer, "%-10u", (unsigned)sz);
    }
    memcpy(size,buffer,10);
  }

  void init() {
    memset(name,' ',16);
    memset(date,' ',12);
    memset(uid,' ',6);
    memset(gid,' ',6);
    memset(mode,' ',8);
    memset(size,' ',10);
    fmag[0] = '`';
    fmag[1] = '\n';
  }
  
  bool checkSignature() {
    return 0 == memcmp(fmag, ARFILE_MEMBER_MAGIC,2);
  }
};

bool CLArchive::isArchive(unsigned char *BufPtr, unsigned char *BufEnd) {
  return (ARFILE_MAGIC_LEN <= (BufEnd - BufPtr)) &&
          (memcmp(BufPtr, ARFILE_MAGIC, ARFILE_MAGIC_LEN) == 0);
}

void CLArchive::createArchive(std::vector<SrcLenStruct> &BCVec,
                                          char **log, formatted_raw_ostream &os)
{
  os << ARFILE_MAGIC;
  std::vector<SrcLenStruct>::iterator iter = BCVec.begin();
  std::vector<SrcLenStruct>::iterator end = BCVec.end();
  for (; iter != end; ++iter) {
    CLArchiveMemberHeader Hdr(iter->src_size);
    os.write((char*)&Hdr, sizeof(Hdr));
    os.write((char*)iter->src, iter->src_size);
  }
  os.flush();
}

int CLArchive::readArchive(unsigned char *BufPtr, unsigned char *BufEnd, 
                           char **log,
                           std::vector<SrcLenStruct> &BCVec)
{
  if (!isArchive(BufPtr, BufEnd)) {
    if (log)
      *log = strdup("Cannot read archive library");
      return -1;
  }
  
  unsigned char *CurPtr = BufPtr + 8;
  
  while (CurPtr < BufEnd) {
    size_t HeaderSize = sizeof(CLArchiveMemberHeader);
    if ((CurPtr + HeaderSize) > BufEnd) {
      if (log)
        *log = strdup("Invalid header in archive file");
      return -2;
    }
    CLArchiveMemberHeader *Hdr = (CLArchiveMemberHeader*)CurPtr;
    int MemberSize = atoi(Hdr->size);
    CurPtr += HeaderSize;
    if (CurPtr+MemberSize > BufEnd) {
      if (log)
        *log = strdup("Invalid member length in archive file");
      return -2;
    }
    BCVec.push_back(SrcLenStruct(CurPtr, MemberSize));
    CurPtr += MemberSize;
  }
}
