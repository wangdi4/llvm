
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

File Name:  StatisticsCollector.cpp

\*****************************************************************************/

#include "StatisticsCollector.h"

using namespace Validation;

void StatisticsCollector::CountStatistics(std::string stat_name)
{
    UpdateStatistics(COUNT, stat_name, 0.0);
}

void StatisticsCollector::UpdateStatistics(STAT_TYPE in_statType, std::string stat_name, double in_val)
{
    StatMap::iterator mit = m_values.find(stat_name);
    // if not exist add with zero initialized
    if(mit == m_values.end())
    {
        StatValue val;
        val.type = in_statType;
        val.value = 0;
        val.count = 0;
        mit = m_values.insert(std::pair<std::string, StatValue>(stat_name, val)).first;
    }
    
    if((mit->second).type != in_statType)
        throw Exception::InvalidArgument("Statistics type is different from previously set type");

    (mit->second).value+=in_val;
    (mit->second).count++;
}

void StatisticsCollector::Finalize()
{
    for(StatMap::iterator e = m_values.end(),
        it = m_values.begin();
        it!=e;++it)
    {
        switch ((it->second).type)
        {
        case AVG:
            (it->second).res = (double)(it->second).value / (it->second).count;
            break;
        case SUM:
            (it->second).res = (double)(it->second).value;
            break;
        case COUNT:
            (it->second).res = (double)(it->second).count;
            break;
        default:
            throw Exception::InvalidArgument("Type of statistics is invalid in Finalize");
            break;
        }
    }
}

double StatisticsCollector::GetResult(const std::string& in_name)
{
    StatMap::iterator mit = m_values.find(in_name);
    // if not exist add with zero initialized
    if(mit == m_values.end())
        return 0.0;
    Finalize();
    return mit->second.res;
}

std::string StatisticsCollector::ToString()
{
    Finalize();
    std::stringstream ss;
    for(StatMap::iterator e = m_values.end(),
        it = m_values.begin();
        it!=e;++it)
    {
        ss << (it->first) << " : ";
        switch(it->second.type)
        {
        case AVG:
                ss << (it->second).res;
            break;
        case SUM:
                ss << (it->second).value;
            break;
        case COUNT:
                ss << (it->second).count;
            break;
        }
        ss << "\n";
    }
    return ss.str();
}