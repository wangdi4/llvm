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

// TODO: move this to utils

#include "isp_utils.h"

#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#endif


// Searches the input directory for filesnames with specific extension
// The search doesn't include sub-directories
// Parameters:
//      - dir       : directory path to look in, it can also contains a trailing slash
//      - extension : extension to match to the files in input directory example: "txt"
//                    for all files, use empty string ""
//      - files     : reference to the vector which all matching files will be inserted to with FULL FILE PATH
// Return value:
//      number of matching files found
// NOTE: On Windows, uses ANSI version of winAPIs
int SearchFilesInDirectory(std::string dir, std::string extension, std::vector< std::string >& files)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(dir.c_str());
    if (INVALID_FILE_ATTRIBUTES == attr || !(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        // invalid directory name
        return 0;
    }

    // append trailing back slash if needed
    if ('\\' != dir[ dir.size()-1 ] && '/' != dir[ dir.size()-1 ])
    {
        dir.append("/");
    }

    std::string searchPath = dir;
    // append the pattern string
    if (!extension.empty())
    {
        searchPath = searchPath + "*." + extension;
    }

    int numOfFound = 0;
    DWORD targetAttr = (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_READONLY) | FILE_ATTRIBUTE_NORMAL;

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            std::string fullFileName = dir + findData.cFileName;
            attr = GetFileAttributesA(fullFileName.c_str());
            if (INVALID_FILE_ATTRIBUTES != attr && (attr & targetAttr))
            {
                files.push_back(fullFileName);
                numOfFound++;
            }
        }
        while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }

    return numOfFound;

#else

    // append trailing slash if needed
    if ('\\' != dir[ dir.size()-1 ] && '/' != dir[ dir.size()-1 ])
    {
        dir.append("/");
    }

    DIR* pDirectory = opendir(dir.c_str());
    if (nullptr == pDirectory)
    {
        // invalid directory name or cannot open directory
        return 0;
    }

    if (!extension.empty())
    {
        extension = "." + extension;
    }

    int numOfFound = 0;
    struct dirent* pFile = readdir(pDirectory);
    for ( ; nullptr != pFile; pFile = readdir(pDirectory) )
    {
        if (DT_REG != pFile->d_type)
        {
            continue;
        }

        std::string fileName = pFile->d_name;

        if (!extension.empty())
        {
            if (fileName.size() < extension.size() ||
                fileName.compare(fileName.size() - extension.size(), extension.size(), extension) != 0)
            {
                continue;
            }
        }

        std::string fullFilePath = dir + fileName;

        files.push_back(fullFilePath);
        numOfFound++;
    }

    closedir(pDirectory);

    return numOfFound;
#endif
}

// Calculates the offset pointer based on base pointer and origin
void* CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize)
{
    char* lockedPtr = (char*) pBasePtr;
    if (nullptr != origin)
    {
        lockedPtr += origin[0] * elemSize; //Origin is in Pixels
        for(unsigned i = 1; i < dim_count; ++i)
        {
            lockedPtr += origin[i] * pitch[i-1]; //y * image width pitch
        }
    }

    return lockedPtr;
}
