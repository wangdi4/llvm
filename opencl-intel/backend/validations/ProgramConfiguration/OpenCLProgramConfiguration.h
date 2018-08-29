// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "IProgramConfiguration.h"
#include "GeneratorConfig.h"
#include "mem_utils.h"

#include "CL/cl.h"
#define TIXML_USE_STL
#include "tinyxml.h"
#include <list>
#include <cl_device_api.h>
#include <vector>
#include "llvm/Support/DataTypes.h"

#include <iostream>
#include <iomanip>

namespace Validation
{
    enum DataFileType
    {
        Binary,
        Xml,
        Random,
        Config
    };

    /// @brief enum for OpenCL program file type
    enum ProgramFileType
    {
        CL,
        LL,
        BC
    };

    /// @brief This class contain OpenCL kernel specific configuration
    class OpenCLKernelConfiguration: public TiXmlVisitor
    {
    public:
        /// @brief Constructor
        OpenCLKernelConfiguration(const TiXmlElement& root, const std::string& baseDirectory);

        ~OpenCLKernelConfiguration()
        {
            if(m_generatorConfig)
                delete m_generatorConfig;
        }
        /// @brief Returns work dimension (possible values: 1, 2, 3)
        /// @return Work dimension
        // More details about work dimension at end of file
        cl_uint GetWorkDimension() const
        {
            return m_workDimension;
        }

        /// @brief Returns global work offset
        /// @return Global work offset
        // More details about global work offset at end of file
        const size_t * GetGlobalWorkOffset() const
        {
            return m_arrGlobalWorkOffset;
        }

        /// @brief Returns global work size
        /// @return Global work size
        // More details about global work size at end of file
        const size_t * GetGlobalWorkSize() const
        {
            return m_arrGlobalWorkSize;
        }

        /// @brief Returns local work size
        /// @return Local work size
        // More details about local work size at end of file
        const size_t * GetLocalWorkSize() const
        {
            return m_arrLocalWorkSize;
        }

        /// @brief Returns name of kernel to run.
        /// @return Kernel name
        const std::string& GetKernelName() const
        {
            return m_kernelName;
        }

        /// @brief Returns the type of the input file
        DataFileType GetInputFileType() const
        {
            return m_inputFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetInputFilePath() const
        {
            return m_inputFilePath;
        }

        /// @brief Returns the type of the reference file
        DataFileType GetReferenceFileType() const
        {
            return m_referenceFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetReferenceFilePath() const
        {
            return m_referenceFilePath;
        }

        /// @brief Returns the type of the reference file
        DataFileType GetNeatFileType() const
        {
            return m_neatFileType;
        }

        /// @brief Returns the input file path
        const std::string& GetNeatFilePath() const
        {
            return m_neatFilePath;
        }

        /// @brief save stamp, insert it into the reference file name and mark it as "inserted"
        void SetReferenceStamp(const std::vector<uint8_t> stamp)
        {
            m_referenceStamp = stamp;
        }

        /// @brief save stamp, insert it into the neat file name and mark it as "inserted"
        void SetNeatStamp(const std::vector<uint8_t> stamp)
        {
            m_neatStamp = stamp;
        }
        ///  @brief get reference stamp
        const std::vector<uint8_t> GetReferenceStamp() const
        {
            return m_referenceStamp;
        }
        ///  @brief get neat stamp
        const std::vector<uint8_t> GetNeatStamp() const
        {
            return m_neatStamp;
        }

        ///  @brief get reference file name with stamp
        std::string GetStampedPathReference()
        {
#if STAMP_ENABLED
            return GetStampedPath(m_referenceFilePath, m_referenceStamp);
#else
            return GetReferenceFilePath();
#endif
        }
        ///  @brief get neat file name with stamp
        std::string GetStampedPathNeat()
        {
#if STAMP_ENABLED
            return GetStampedPath(m_neatFilePath, m_neatStamp);
#else
            return GetNeatFilePath();
#endif
        }

        const OCLKernelDataGeneratorConfig* GetGeneratorConfig() const
        {
            return this->m_generatorConfig;
        }

    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
        DataFileType GetDataFileType(const std::string& strFileType);


        std::string GetStampedPath( const std::string& path, const std::vector<uint8_t>& stamp)
        {
            std::ostringstream ss;
            ss << path;

            if(stamp.size() > 0) {
                ss << '.';
                ss << std::hex << std::uppercase << std::setfill( '0' );
                for (std::vector<uint8_t>::const_iterator i = stamp.begin(), e = stamp.end(); i != e; ++i)
                ss << std::setw(2) << int(*i);
            }
            return ss.str();        
        }


    private:
        // Global work offset
        // More details about global work offset at end of file
        size_t m_arrGlobalWorkOffset[MAX_WORK_DIM];

        // Global work Size
        // More details about global work size at end of file
        size_t m_arrGlobalWorkSize[MAX_WORK_DIM];

        // Local work size
        // More details about local work size at end of file
        size_t m_arrLocalWorkSize[MAX_WORK_DIM];

        // Work dimension
        // More details about work dimension at end of file
        cl_uint m_workDimension;

        // type of the input file
        DataFileType m_inputFileType;

        // path to the input file
        std::string m_inputFilePath;

        // Name of kernel to run.
        std::string m_kernelName;

        // type of the reference file
        DataFileType m_referenceFileType;

        // path to the reference file
        std::string m_referenceFilePath;

        // type of the neat file
        DataFileType m_neatFileType;

        // path to the neat file
        std::string m_neatFilePath;

        // base directory
        std::string m_baseDirectory;

        // md5 stamp for reference file
        std::vector<uint8_t> m_referenceStamp;
        // md5 stamp for NEAT file
        std::vector<uint8_t> m_neatStamp;

        //config for kernal arguments generator
        OCLKernelDataGeneratorConfig* m_generatorConfig;
    };

    /// @brief This class contain OpenCL Include Directories
    class OpenCLIncludeDirs: public TiXmlVisitor
    {
    public:
        typedef std::list<std::string> IncludeDirsList;

        /// @brief Constructor
        OpenCLIncludeDirs(const TiXmlElement& root, const std::string& baseDirectory);

        /// @brief Return the const iterator to the include directories list start
        /// @return iterator
        IncludeDirsList::const_iterator beginIncldueDirs() const
        {
            return m_includeDirsList.begin();
        }

        /// @brief Return the const iterator to the include directories list end
        /// @return iterator
        IncludeDirsList::const_iterator endIncludeDirs() const
        {
            return m_includeDirsList.end();
        }
    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);

    private:
        // List of include directories for OpenCL
        IncludeDirsList m_includeDirsList;

        // base directory
        std::string m_baseDirectory;
    };

    /// @brief This class contain OpenCL test program information
    class OpenCLProgramConfiguration : public IProgramConfiguration, public TiXmlVisitor
    {
    public:
        typedef std::list<OpenCLKernelConfiguration*> KernelConfigList;

        /// @brief Constructor
        /// @param [IN] configFile Name of OpenCL test run configuration file
        /// @param [IN] baseDir. Base directory used for input/output file lookup
        OpenCLProgramConfiguration(const std::string& configFile, const std::string& baseDir);
        ~OpenCLProgramConfiguration();

        /// @brief Returns the program file path
        std::string GetProgramFilePath() const
        {
            return m_programFilePath;
        }

        /// @brief Returns the type of the program file
        ProgramFileType GetProgramFileType() const
        {
            return m_programFileType;
        }

        std::string GetProgramName() const
        {
            return m_programName;
        }

        /// @brief Returns the compilation flags for the program file
        std::string GetCompilationFlags() const
        {
            return m_compilationFlags;
        }

        /// @brief Returns the injected object file path
        std::string GetInjectedObjectPath() const
        {
            return m_injectedObjectPath;
        }

        /// @brief Returns indicator whether to use vectorizer
        /// @return True if vectorizer should be used, false otherwise.
        bool GetUseVectorizer() const
        {
            return m_useVectorizer;
        }

        /// @brief Return the const iterator to kernel configurations list start
        /// @return iterator
        KernelConfigList::const_iterator beginKernels() const
        {
            return m_kernels.begin();
        }

        /// @brief Return the const iterator to kernel configurations list end
        /// @return iterator
        KernelConfigList::const_iterator endKernels() const
        {
            return m_kernels.end();
        }

        /// @brief Returns OpenCLIncludeDirs
        OpenCLIncludeDirs* GetIncludeDirs() const
        {
            return m_includeDirs.get();
        }

        /// @brief Returns the base directory
        std::string GetBaseDirectory() const
        {
            return m_baseDirectory;
        }

        /// @brief Return number of kernel configurations to run.
        size_t GetNumberOfKernelConfigurations() const
        {
            return m_kernels.size();
        }

    private:
        bool VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
        ProgramFileType GetProgramFileType(const std::string& strFileType);

    private:
        // Indicator whether to use vectorizer
        bool m_useVectorizer;

        // Program file path
        std::string m_programFilePath;

        // Program file type
        ProgramFileType m_programFileType;

        // List of kernel specific configurations
        KernelConfigList m_kernels;

        // Include directories for generating IR from a CL file
        auto_ptr_ex<OpenCLIncludeDirs> m_includeDirs;

        // configuration file absolute path
        std::string m_configFile;

        // base directory path - used to lookup the input/output files if specified as relative path
        std::string m_baseDirectory;

        // program name
        std::string m_programName;
        
        //compilation options
        std::string m_compilationFlags;

        // Injected object file path
        std::string m_injectedObjectPath;

        // program configuration file format
        enum PROGRAM_CONFIG_FILE_FORMAT {
            UNKNOWN,
            BYTECODE_ONLY,  // config file supports only byte code file type for test program
            CL_LL_BC        // config file supports cl, llvm test and llvm byte code file type for test program
        };
        PROGRAM_CONFIG_FILE_FORMAT m_format;
  };

        // For 2 dimensions:
        // If you divide a picture into smaller squares, say 1024 pixels each
        //   ___________________________
        //  |                           |
        //  |                           |
        //  |                           |
        //  |        ________           |
        //  |       |        |          |
        //  |       |        |          |
        //  |       |________|          |
        //  |                           |
        //  |                           |
        //  |___________________________|

        //then:
        //m_workDimension = 2
        //m_arrGlobalWorkOffeset[0] = the id of the square on X radix
        //m_arrGlobalWorkOffeset[1] = the id of the square on Y radix
        //m_arrGlobalWorkSize[0] = total number of squares on X radix
        //m_arrGlobalWorkSize[1] = total number of squares on Y radix
        //m_arrLocalWorkSize[0] = size of square on X radix
        //m_arrLocalWorkSize[1] = size of square on Y radix
}

