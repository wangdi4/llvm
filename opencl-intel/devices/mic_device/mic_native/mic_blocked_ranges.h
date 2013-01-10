// Copyright (c) 2006-2008 Intel Corporation
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

/////////////////////////////////////////////////////////////
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once

#include <tbb/tbb.h>
#include <tbb/task.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

//
// Represents contiguous range if indices [min,max), including min and excluding max
//
class BlockedRange 
{
public:
    typedef unsigned int BlockedRangeValueType;
    typedef size_t       BlockedRangeSizeType;

    BlockedRange() : m_min(0), m_max(0), m_grain(0) {};
        
    BlockedRange( BlockedRangeValueType min, 
                  BlockedRangeValueType max, 
                  BlockedRangeSizeType grain = 1 ) : 
        m_min(min), m_max(max), m_grain(grain) {};

    BlockedRange( const BlockedRange& o ) : 
        m_min(o.m_min), m_max(o.m_max), m_grain(o.m_grain) {};
    
    BlockedRange(const tbb::blocked_range<int>& tbb_r) :
        m_min(tbb_r.begin()), m_max(tbb_r.end()), m_grain(tbb_r.grainsize()) {};

    BlockedRange(const tbb::blocked_range<int>& tbb_r, BlockedRangeSizeType grain ) :
        m_min(tbb_r.begin()), m_max(tbb_r.end()), m_grain(grain) {};

    BlockedRangeValueType begin() const { return m_min; };
    BlockedRangeValueType end()   const { return m_max; };
    BlockedRangeSizeType  size()  const { return BlockedRangeSizeType( m_max - m_min ); };
    BlockedRangeSizeType  grainsize() const { return m_grain; };

    bool empty()                  const { return !(m_min < m_max); };
    bool is_divisible()           const { return m_grain<size(); };
    
    BlockedRangeValueType min()   const { return m_min; };
    BlockedRangeValueType max()   const { return m_max; };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRange( BlockedRange& o, tbb::split ) : 
        m_min(o.m_min), m_max(o.m_max), m_grain(o.m_grain)
    {
        BlockedRangeValueType middle = m_min + ((m_max - m_min) / 2u);
        m_min = middle;
        o.m_max = middle;
    }

private:
    BlockedRangeValueType m_min;
    BlockedRangeValueType m_max;
    BlockedRangeSizeType  m_grain;
};

//
//   Optimize access by Row (split by row)
//

typedef BlockedRange BlockedRangeByRow1d;

class BlockedRangeByRow2d 
{
public:
    BlockedRangeByRow2d() : m_cols(), m_rows() {};
        
    BlockedRangeByRow2d( BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols)  {};

    BlockedRangeByRow2d( const BlockedRangeByRow2d& o ) : 
        m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByRow2d(const tbb::blocked_range2d<int>& tbb_r) :
        m_rows(tbb_r.rows()), m_cols(tbb_r.cols(),1) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& rows() const { return m_rows; };
    const BlockedRangeByRow1d& cols() const { return m_cols; };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_rows.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow2d( BlockedRangeByRow2d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_rows.size() > 1)
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

class BlockedRangeByRow3d 
{
public:
    BlockedRangeByRow3d() : m_pages(), m_cols(), m_rows() {};
        
    BlockedRangeByRow3d( BlockedRange::BlockedRangeValueType min_pages, 
                         BlockedRange::BlockedRangeValueType max_pages, 
                         BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_pages(min_pages,max_pages), m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols)  {};

    BlockedRangeByRow3d( const BlockedRangeByRow3d& o ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByRow3d(const tbb::blocked_range3d<int>& tbb_r) :
        m_pages(tbb_r.pages(), 1), m_rows(tbb_r.rows()), m_cols(tbb_r.cols(), 1) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_pages.empty() || m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_pages.is_divisible() || m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& pages() const { return m_pages; };
    const BlockedRangeByRow1d& rows()  const { return m_rows;  };
    const BlockedRangeByRow1d& cols()  const { return m_cols;  };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_rows.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow3d( BlockedRangeByRow3d& o, tbb::split ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_pages.size() > 1)
        {
            m_pages = BlockedRange( o.m_pages, tbb::split() );
        } 
        else if (m_rows.size() > 1)
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_pages;
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

//
//   Optimize access by Column (split by column)
//

typedef BlockedRangeByRow1d BlockedRangeByColumn1d;

class BlockedRangeByColumn2d 
{
public:
    BlockedRangeByColumn2d() : m_cols(), m_rows() {};
        
    BlockedRangeByColumn2d( BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_rows(min_rows,max_rows), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByColumn2d( const BlockedRangeByColumn2d& o ) : 
        m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByColumn2d(const tbb::blocked_range2d<int>& tbb_r) :
        m_rows(tbb_r.rows(), 1), m_cols(tbb_r.cols()) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& rows() const { return m_rows; };
    const BlockedRangeByRow1d& cols() const { return m_cols; };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn2d( BlockedRangeByColumn2d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_cols.size() > 1)
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
        else
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

class BlockedRangeByColumn3d 
{
public:
    BlockedRangeByColumn3d() : m_pages(), m_cols(), m_rows() {};
        
    BlockedRangeByColumn3d( BlockedRange::BlockedRangeValueType min_pages, 
                            BlockedRange::BlockedRangeValueType max_pages, 
                            BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_pages(min_pages,max_pages), m_rows(min_rows,max_rows), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByColumn3d( const BlockedRangeByColumn3d& o ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByColumn3d(const tbb::blocked_range3d<int>& tbb_r) :
        m_pages(tbb_r.pages(), 1), m_rows(tbb_r.rows(), 1), m_cols(tbb_r.cols()) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_pages.empty() || m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_pages.is_divisible() || m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& pages() const { return m_pages; };
    const BlockedRangeByRow1d& rows()  const { return m_rows;  };
    const BlockedRangeByRow1d& cols()  const { return m_cols;  };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn3d( BlockedRangeByColumn3d& o, tbb::split ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_pages.size() > 1)
        {
            m_pages = BlockedRange( o.m_pages, tbb::split() );
        } 
        else if (m_cols.size() > 1)
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
        else
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_pages;
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

//
//   Split By Tile
//

typedef BlockedRangeByRow1d BlockedRangeByTile1d;

class BlockedRangeByTile2d 
{
public:
    BlockedRangeByTile2d() : m_cols(), m_rows() {};
        
    BlockedRangeByTile2d( BlockedRange::BlockedRangeValueType min_rows, 
                          BlockedRange::BlockedRangeValueType max_rows, 
                          BlockedRange::BlockedRangeValueType min_cols, 
                          BlockedRange::BlockedRangeValueType max_cols, 
                          BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByTile2d( const BlockedRangeByTile2d& o ) : 
        m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByTile2d(const tbb::blocked_range2d<int>& tbb_r) :
        m_rows(tbb_r.rows()), m_cols(tbb_r.cols()) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& rows() const { return m_rows; };
    const BlockedRangeByRow1d& cols() const { return m_cols; };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split cols
    BlockedRangeByTile2d( BlockedRangeByTile2d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        if (m_rows.size() > m_cols.size())
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

class BlockedRangeByTile3d 
{
public:
    BlockedRangeByTile3d() : m_pages(), m_cols(), m_rows() {};
        
    BlockedRangeByTile3d( BlockedRange::BlockedRangeValueType min_pages, 
                          BlockedRange::BlockedRangeValueType max_pages, 
                          BlockedRange::BlockedRangeValueType min_rows, 
                          BlockedRange::BlockedRangeValueType max_rows, 
                          BlockedRange::BlockedRangeValueType min_cols, 
                          BlockedRange::BlockedRangeValueType max_cols, 
                          BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_pages(min_pages,max_pages,grain), m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByTile3d( const BlockedRangeByTile3d& o ) : 
        m_pages(o.m_pages), m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByTile3d(const tbb::blocked_range3d<int>& tbb_r) :
        m_pages(tbb_r.pages()), m_rows(tbb_r.rows()), m_cols(tbb_r.cols()) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_pages.empty() || m_rows.empty() || m_cols.empty()); };
    bool is_divisible()           const { return (m_pages.is_divisible() || m_rows.is_divisible() || m_cols.is_divisible()); };

    const BlockedRangeByRow1d& pages() const { return m_pages; };
    const BlockedRangeByRow1d& rows()  const { return m_rows;  };
    const BlockedRangeByRow1d& cols()  const { return m_cols;  };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_cols.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split cols
    BlockedRangeByTile3d( BlockedRangeByTile3d& o, tbb::split ) : 
        m_rows(o.m_rows), m_cols(o.m_cols)
    {
        BlockedRange::BlockedRangeSizeType pages_size = m_pages.size();
        BlockedRange::BlockedRangeSizeType rows_size  = m_rows.size();
        BlockedRange::BlockedRangeSizeType cols_size  = m_cols.size();
        
        if ((pages_size >= rows_size) && (pages_size >= cols_size))
        {
            m_pages = BlockedRange( o.m_pages, tbb::split() );
        } 
        else if (rows_size >= cols_size)
        {
            m_rows = BlockedRange( o.m_rows, tbb::split() );
        }
        else
        {
            m_cols = BlockedRange( o.m_cols, tbb::split() );
        }
    }
    
private:
   BlockedRange  m_pages;
   BlockedRange  m_rows;
   BlockedRange  m_cols;    
};

//
//  Just wrapper above default TBB blocked range
//
class BlockedRangeByDefaultTBB1d : public tbb::blocked_range<int> 
{
public:
    BlockedRangeByDefaultTBB1d() : tbb::blocked_range<int> () {};
    
    BlockedRangeByDefaultTBB1d( unsigned int min, 
                                unsigned int max, 
                                size_t grain = 1 ) : 
        tbb::blocked_range<int> ( min, max, grain ) {};

    BlockedRangeByDefaultTBB1d( const BlockedRangeByDefaultTBB1d& o ) : 
        tbb::blocked_range<int> ( o ) {};
    
    BlockedRangeByDefaultTBB1d(const tbb::blocked_range<int>& tbb_r) :
        tbb::blocked_range<int> ( tbb_r ) {};

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB1d( BlockedRangeByDefaultTBB1d& o, tbb::split ) : 
        tbb::blocked_range<int> ( o, tbb::split() ) {};
};

class BlockedRangeByDefaultTBB2d : public tbb::blocked_range2d<int> 
{
public:
    BlockedRangeByDefaultTBB2d() : tbb::blocked_range2d<int> (0,0,0,0) {};
    
    BlockedRangeByDefaultTBB2d( unsigned int min_rows, 
                                unsigned int max_rows, 
                                unsigned int min_cols, 
                                unsigned int max_cols, 
                                size_t grain = 1 ) : 
        tbb::blocked_range2d<int> ( min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByDefaultTBB2d( const BlockedRangeByDefaultTBB2d& o ) : 
        tbb::blocked_range2d<int> ( o ) {};
    
    BlockedRangeByDefaultTBB2d(const tbb::blocked_range2d<int>& tbb_r) :
        tbb::blocked_range2d<int> ( tbb_r ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB2d( BlockedRangeByDefaultTBB2d& o, tbb::split ) : 
        tbb::blocked_range2d<int> ( o, tbb::split() ) {};
};

class BlockedRangeByDefaultTBB3d : public tbb::blocked_range3d<int> 
{
public:
    BlockedRangeByDefaultTBB3d() : tbb::blocked_range3d<int> (0,0,0,0,0,0) {};
    
    BlockedRangeByDefaultTBB3d( unsigned int min_pages, 
                                unsigned int max_pages, 
                                unsigned int min_rows, 
                                unsigned int max_rows, 
                                unsigned int min_cols, 
                                unsigned int max_cols, 
                                size_t grain = 1 ) : 
        tbb::blocked_range3d<int> ( min_pages, max_pages, grain, min_rows, max_rows, grain, min_cols, max_cols, grain ) {};

    BlockedRangeByDefaultTBB3d( const BlockedRangeByDefaultTBB3d& o ) : 
        tbb::blocked_range3d<int> ( o ) {};
    
    BlockedRangeByDefaultTBB3d(const tbb::blocked_range3d<int>& tbb_r) :
        tbb::blocked_range3d<int> ( tbb_r ) {};

    BlockedRange::BlockedRangeSizeType  grainsize() const { return cols().grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    BlockedRangeByDefaultTBB3d( BlockedRangeByDefaultTBB3d& o, tbb::split ) : 
        tbb::blocked_range3d<int> ( o, tbb::split() ) {};
};

}}}

