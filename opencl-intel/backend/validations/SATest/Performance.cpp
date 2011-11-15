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

File Name:  Performence.cpp

\*****************************************************************************/
#include "Performance.h"
#include "SATestException.h"
#include <assert.h>
#include <iostream>
#include <numeric>
#include <cmath>
#include <algorithm>

#if (defined(_WIN32))
    #if !defined(_M_X64)
        #define _WIN32_VTUNE
    #endif
    #include <tchar.h>
    #include <Windows.h>
    #include <intrin.h>
#else
    #include <time.h>
    #include <sched.h>
#endif

using namespace Validation;

PriorityBooster::PriorityBooster(bool dummy):
    m_dummy(dummy)
{
    if( m_dummy )
        return;

#if defined(_WIN32)
    bool result = FALSE;
    result = SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    if( !result )
        throw Exception::GeneralException("OS API error:Can't set program priority class");
    result = SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
    if( !result )
        throw Exception::GeneralException("OS API error:Can't set thread priority");
    result = SetProcessAffinityMask(GetCurrentProcess(), 2);
    if( !result )
        throw Exception::GeneralException("OS API error:Can't set process affinity");
    
    Sleep(0);
#else
    // Increase the thread priority to real time
    struct sched_param param;
    int policy = SCHED_RR;
    pthread_t thread_id = pthread_self();
    param.sched_priority = 90;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1,&mask);
    pthread_setschedparam(thread_id, policy, &param);
    int result = sched_setaffinity(0, sizeof(mask), &mask);
    if( 0 != result )
        throw Exception::GeneralException("OS API error:Can't set process affinity");
#endif
}

PriorityBooster::~PriorityBooster()
{
    if( m_dummy )
        return;

#if defined(_WIN32)
    SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
    Sleep(0);
#else
    struct sched_param param;
    pthread_t thread_id = pthread_self();
    param.sched_priority = 0;
    int policy = 0;             // for normal threads
    pthread_setschedparam(thread_id, policy, &param);
#endif
}

Sample::Sample():
    m_ulStart(0),
    m_ulStop(0),
    m_ulRDTSCStart(0),
    m_ulTotalTime(0),
    m_ulTotalRDTSC(0),
    m_ulSamplesCount(0)
{
}

inline cl_long RDTSC()
{
#ifdef _WIN32
    return __rdtsc();
#else
    if (sizeof(void *) < 8)
    {
        //	32 bits
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));
        return (cl_long) ((uint64_t)hi << 32 | lo);
    }
    else
    {
        // 64 bits
        uint64_t lo, hi;
        __asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));
        return (cl_long) (hi << 32 | lo);
    }
    return 0;

#endif
}

cl_ulong Sample::Start()
{
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&m_ulStart);
#else
    m_ulStart = clock();
#endif
    m_ulRDTSCStart = RDTSC();
    return m_ulStart;
}

cl_ulong Sample::Stop()
{
#ifdef _WIN32
    QueryPerformanceCounter((LARGE_INTEGER*)&m_ulStop);
#else
    m_ulStop = clock();
#endif
    m_ulRDTSCEnd = RDTSC();

    m_ulTotalTime += (m_ulStop - m_ulStart);
    m_ulTotalRDTSC += (m_ulRDTSCEnd - m_ulRDTSCStart);
    ++m_ulSamplesCount;
    return m_ulStop;
}

cl_ulong Sample::SamplesCount() const
{
    return m_ulSamplesCount;
}

cl_ulong Sample::TotalTicks() const
{
    return m_ulTotalRDTSC;
}

cl_ulong Sample::TotalTime() const
{
#ifdef _WIN32
    LARGE_INTEGER freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    double ticksPerSecond = (double)freq.QuadPart / 1000000.;
    return (cl_ulong)((double)m_ulTotalTime / ticksPerSecond);
#else
    return (m_ulTotalTime / CLOCKS_PER_SEC );
#endif
}

bool Sample::operator < ( const Sample& rhs) const
{
    return TotalTicks() < rhs.TotalTicks();
}


void SampleVector::AddSample(const Sample& sample)
{
    m_samples.push_back(sample);
}

Sample SampleVector::MinimalSample() const
{
    std::vector<Sample>::const_iterator it = std::min_element( m_samples.begin(), m_samples.end());
    return it == m_samples.end() ? Sample() : *it;
}

void SampleVector::Print() const
{
    for( std::vector<Sample>::const_iterator it = m_samples.begin();
         it != m_samples.end();
         ++it)
    {
        std::cout << it->TotalTicks() << ",";
    }
    std::cout << "\n";
}

cl_ulong add_ticks( cl_ulong v, const Sample& sample)
{
    return sample.TotalTicks() + v;
}

struct pow_mean
{
    pow_mean( double mean): m_mean(mean){}
    
    double operator()( double v, const Sample& rhs)
    {
        return v + std::pow((double)rhs.TotalTicks()-m_mean, 2);
    }
private:
    double m_mean;
};

double  SampleVector::Mean() const
{
    if( m_samples.empty() )
    {
        return 0.;
    }

    cl_ulong sum = std::accumulate(m_samples.begin(), m_samples.end(), (cl_ulong)0, add_ticks);
    return sum / (double)m_samples.size();
}

double  SampleVector::StandardDeviation() const
{
    if( m_samples.empty() )
    {
        return 0.;
    }
    double deviation = std::accumulate(m_samples.begin(), m_samples.end(), (double)0, pow_mean(Mean()));
    return std::sqrt( deviation / m_samples.size() );
}

void Performance::SetBuildTime(const Sample& sample)
{
    m_buildSample.AddSample(sample);
}

void Performance::SetSerializationTime(const Sample& sample)
{
    m_serializationSample.AddSample(sample);
}

void Performance::SetDeserializationTime(const Sample& sample)
{
    m_deserializationSample.AddSample(sample);
}

void Performance::SetExecutionTime(const std::string& name, const Sample& sample)
{
    m_executionSamples[name].AddSample(sample);
}

cl_long Performance::GetBuildTime() const
{
    return m_buildSample.MinimalSample().TotalTicks();
}

cl_long Performance::GetExecutionTime(const std::string& name) const
{
    Samples::const_iterator sample = m_executionSamples.find(name);
    assert( sample != m_executionSamples.end() );
    
    return sample->second.MinimalSample().TotalTicks();
}

void Performance::Print(const std::string& programName) const
{
    cl_long buildTicks = m_buildSample.MinimalSample().TotalTicks();
    double  buildMean  = m_buildSample.Mean();
    double  buildSD    = m_buildSample.StandardDeviation();
    double  buildSDMean= buildSD / buildMean;

#ifdef MIC_ENABLE
    cl_long serializationTicks = m_serializationSample.MinimalSample().TotalTicks();
    double  serializationMean  = m_serializationSample.Mean();
    double  serializationSD    = m_serializationSample.StandardDeviation();
    double  serializationSDMean= serializationSD / serializationMean;

    cl_long deserializationTicks = m_deserializationSample.MinimalSample().TotalTicks();
    double  deserializationMean  = m_deserializationSample.Mean();
    double  deserializationSD    = m_deserializationSample.StandardDeviation();
    double  deserializationSDMean= deserializationSD / deserializationMean;
#endif // MIC_ENABLE

    for( Samples::const_iterator it = m_executionSamples.begin();
         it != m_executionSamples.end();
         ++it)
    {
        double mean   = it->second.Mean();
        double sd     = it->second.StandardDeviation();
        double sdmean = sd / mean;

        //it->second.Print();

        std::cout << programName << "."
                  << it->first << ","
                  << buildTicks << ","
                  << buildSDMean << ","
#ifdef MIC_ENABLE
                  << serializationTicks << ","
                  << serializationSDMean << ","
                  << deserializationTicks << ","
                  << deserializationSDMean << ","
#endif //MIC_ENABLE
                  << it->second.MinimalSample().TotalTicks() << ","
                  << sdmean
                  << std::endl;
    }
}
