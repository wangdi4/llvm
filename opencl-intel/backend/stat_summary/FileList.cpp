// Copyright (c) 2014 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <iostream>
#include <string>
#include <vector>

using namespace std;

static bool isIRFileName(const string &name)
{
  if (name.size() < 4)
    return false;

  if (name.compare(name.size()-3, 3, ".ll") == 0)
    return true;

  return false;
}


#if defined (WIN32)

#include <windows.h>
#include <tchar.h>

namespace Intel {

bool getIRFileNames(const char *dirname, vector<string> &fname)
{
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;

   hFind = FindFirstFile(dirname, &FindFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
     cout << "Error: Can't find directory " << dirname << "\n";
     return false;
   }

   do {
     if (isIRFileName(FindFileData.cFileName) &&
         GetFileAttributes(FindFileData.cFileName) == FILE_ATTRIBUTE_ARCHIVE)
       fname.push_back(FindFileData.cFileName);
   } while (FindNextFile(hFind, &FindFileData));

   FindClose(hFind);
   // report if this directory doesn't contain any IR file
   if (fname.size() == 0) {
     cout << "No IR files found in directory " << dirname << "\n";
     return false;
   }

   return true;
}
}
#else // WIN32


#include <dirent.h>
#include <sys/stat.h>

namespace Intel {

bool getIRFileNames(const char *dirname, vector<string> &fname)
{
  DIR *dirp = opendir(dirname);
  // report if the given directory is not found
  if (dirp == NULL) {
    cout << "Error: Can't find directory " << dirname <<"\n";
    return false;
  }

  struct dirent *dp;
  struct stat ir_fstat;
  // traverse all files in a directory.
  // if a file ends with .ll and this is a regular file
  // keep it in the file list
  while ((dp = readdir(dirp)) != NULL) {
    if (isIRFileName(dp->d_name) &&
      stat(dp->d_name, &ir_fstat) == 0 &&
      ir_fstat.st_mode & S_IFREG)
        fname.push_back(dp->d_name);
  }
  closedir(dirp);

  // report if this directory doesn't contain any IR file
  if (fname.size() == 0) {
    cout << "No IR files found in directory " << dirname << "\n";
    return false;
  }

  return true;
}
}
#endif // WIN32






/*
 * the following is portable but veeery slow code
  StringRef p(dirname);

  file_status fs;
  //Twin path(dirname);
  if (status(dirname, fs)) {
    cout << "Error: Can't find directory " << dirname <<"\n";
    return false;
  }
  int count = 0;
  if (fs.type() == file_type::directory_file) {
    error_code ec;
    directory_iterator i(dirname, ec);
    if (ec) return false;
    for (directory_iterator e; i != e; i.increment(ec)) {
      if (ec) return false;
      file_status st;
      if (i->status(st)) return false;
      const char *fileName = i->path().c_str();
      int len = strlen(fileName);
      if (len >= 4 && strcmp(".ll", fileName+len-3) == 0) {
        if (st.type() == file_type::regular_file) {
          fname.push_back(fileName);
          count ++;
        }
      }
    }
  }
 *
 */
