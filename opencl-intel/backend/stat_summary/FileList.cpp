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

namespace Intel {

bool getIRFileNames(const char *dirName, vector<string> &fileList)
{
   LPWIN32_FIND_DATAA FindFileData = 0;

   HANDLE hFind = FindFirstFileA(dirName, FindFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
     cout << "Error: Can't find directory " << dirName << "\n";
     return false;
   }

   do {
     if (isIRFileName(FindFileData->cFileName) &&
         GetFileAttributesA(FindFileData->cFileName) == FILE_ATTRIBUTE_ARCHIVE)
       fileList.push_back(FindFileData->cFileName);
   } while (FindNextFileA(hFind, FindFileData));

   FindClose(hFind);
   // report if this directory doesn't contain any IR file
   if (fileList.size() == 0) {
     cout << "No IR files found in directory " << dirName << "\n";
     return false;
   }

   return true;
}
}
#else // WIN32


#include <dirent.h>
#include <sys/stat.h>

namespace Intel {

bool getIRFileNames(const char *dirName, vector<string> &fileList)
{
  DIR *dirp = opendir(dirName);
  // report if the given directory is not found
  if (dirp == NULL) {
    cout << "Error: Can't find directory " << dirName <<"\n";
    return false;
  }

  struct dirent *dp;
  struct stat ir_fstat;
  string fileName(dirName);
  if (*fileName.rbegin() != '/')
    fileName.append("/");
  int size = fileName.size();
  // traverse all files in a directory.
  // if a file ends with .ll and this is a regular file
  // keep it in the file list
  while ((dp = readdir(dirp)) != NULL) {
    fileName.resize(size);
    fileName.append(dp->d_name);
    if (isIRFileName(fileName.c_str()) &&
      stat(fileName.c_str(), &ir_fstat) == 0 &&
      ir_fstat.st_mode & S_IFREG)
      fileList.push_back(fileName);
  }
  closedir(dirp);

  // report if this directory doesn't contain any IR file
  if (fileList.size() == 0) {
    cout << "No IR files found in directory " << dirName << "\n";
    return false;
  }

  return true;
}
}
#endif // WIN32
