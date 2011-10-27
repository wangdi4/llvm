/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Performence.h

\*****************************************************************************/
#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "IPerformance.h"
#include "CL/cl.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"

#include <map>
#include <string>
#include <vector>

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    class PriorityBooster
    {
    public:
        PriorityBooster(bool dummy);
        ~PriorityBooster();
    private:
        bool m_dummy;
    };

    /// @brief Basic sampler. Used to measure the time intervals
    class Sample
    {
    public:
        Sample();

        /// @brief Starts/Restart sampling. Restarting the sampling
        ///        will increment the sample count (upon stop)
        cl_ulong Start();

        /// @brief Stops sampling
        cl_ulong Stop();

        /// @brief Returns the number of seconds in measured interval
        cl_ulong TotalTime() const;

        /// @brief Returns the number of ticks in measured interval
        cl_ulong TotalTicks() const;

        /// @brief Returns the number of samples (actually number of start/stop rounds)
        cl_ulong SamplesCount() const;

        /// @brief Standard comparison operator
        bool operator < ( const Sample& rhs) const;

    private:
        cl_ulong    m_ulStart;
        cl_ulong    m_ulStop;
        cl_ulong    m_ulRDTSCStart;
        cl_ulong    m_ulRDTSCEnd;

        cl_ulong    m_ulTotalTime;
        cl_ulong    m_ulTotalRDTSC;
        cl_ulong    m_ulSamplesCount;
    };


    /// @brief Vector of samples. Provides various statistical information (min, geomean, stdd)
    class SampleVector
    {
    public:
        /// @brief Adds new sample to the vector
        void AddSample(const Sample& sample);

        /// @brief Returns the sample with a minimal TotalTick count or default (zeroed) Sample in case of empty vector
        Sample MinimalSample() const;

        /// @brief Calculates the mean of all samples in the vector
        double  Mean() const;

        /// @brief Calculates the standard deviation of the samples in the vector
        double  StandardDeviation() const;


        void Print() const;
    private:
        std::vector<Sample> m_samples;
    };


    /// @brief This class enables test execution performence measurements.
    /// The measurements consist of build time and execution time.
    class Performance : public IPerformance
    {
    public:
        /// @brief Returns build time
        /// @return Build time in ticks
        virtual cl_long GetBuildTime() const ;

        /// @brief Returns execution time
        /// @return Execution time for specified kernel in ticks
        virtual cl_long GetExecutionTime(const std::string& name) const;

        /// @brief Set the build time
        void SetBuildTime(const Sample& sample);

        /// @brief Add the sample to the sample vector for given kernel
        void SetExecutionTime(const std::string& name, const Sample& sample);

        /// @brief Prints the results
        /// TODO: temporary solution - find more elegant one
        void Print(const std::string& programName) const;

    private:
        typedef std::map<std::string, SampleVector > Samples;

        Samples m_executionSamples;
        SampleVector m_buildSample;
    };
}

#endif // PERFORMANCE_H
