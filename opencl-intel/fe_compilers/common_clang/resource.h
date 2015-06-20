#ifndef __RESOURCE__
#define __RESOURCE__
#include <iostream>


#define IDR_COMMON                      "COMMON"
#define IDR_CPU                         "CPU"
#define IDR_CPU_SPEC_12                 "CPU_SPEC_12"
#define IDR_CPU_SPEC_20                 "CPU_SPEC_20"
#define IDR_HEADER_WITH_DEFS_CPU        "DEFS_CPU"



struct ResourceProp
{
    const char* m_data;
    size_t      m_size;
    std::string m_name;

    ResourceProp()
    {
        m_data = 0;
        m_size = 0;
        m_name = "";
    }
    ResourceProp(const char* data, size_t size, const std::string& name):
            m_data(data), m_size(size), m_name(name)
    {}
};
#endif /* __RESOURCE__ */
