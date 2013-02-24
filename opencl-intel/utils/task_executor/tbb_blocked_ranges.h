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
//  blocked_ranges.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once

#include <tbb/tbb.h>
#include <tbb/task.h>

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class BlockedRangeBase : public tbb::blocked_range3d<int>
{
public:
    // OpenCL defines dims as (col, row, page), while TBB receives them as (page, row, col)
    BlockedRangeBase( const size_t dims[], size_t grain ) :
        tbb::blocked_range3d<int>( 0, (int)dims[2], grain,
                                   0, (int)dims[1], grain,
                                   0, (int)dims[0], grain ) {};  
};

//
// Represents contiguous range if indices [left,right), including left and excluding right
//
class BlockedRange 
{
public:
    typedef unsigned int BlockedRangeValueType;
    typedef size_t       BlockedRangeSizeType;

    BlockedRange() : m_min(0), m_max(0), m_grain(0) {};
        
    BlockedRange( BlockedRangeValueType left, 
                  BlockedRangeValueType right, 
                  BlockedRangeSizeType grain = 1 ) : 
        m_min(left), m_max(right), m_grain(grain) {};

    BlockedRange( const BlockedRange& o ) : 
        m_min(o.m_min), m_max(o.m_max), m_grain(o.m_grain) {};
    
    BlockedRange(const tbb::blocked_range3d<int>& tbb_r) :
        m_min(tbb_r.cols().begin()), m_max(tbb_r.cols().end()), m_grain(tbb_r.cols().grainsize()) {};

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
    
    BlockedRangeValueType left()   const { return m_min; };
    BlockedRangeValueType right()  const { return m_max; };

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
// Represents contiguous range if 2D indices [left,right), including left and excluding right,
// with priority for one of the for splitting
//
class BlockedRange2d 
{
public:
    BlockedRange2d() : m_coord_split_first(), m_coord_split_second() {};
        
    BlockedRange2d( BlockedRange::BlockedRangeValueType min_coord_split_first, 
                    BlockedRange::BlockedRangeValueType max_coord_split_first, 
                    BlockedRange::BlockedRangeValueType min_coord_split_second, 
                    BlockedRange::BlockedRangeValueType max_coord_split_second, 
                    BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_coord_split_first (min_coord_split_first,max_coord_split_first), 
        m_coord_split_second(min_coord_split_second,max_coord_split_second,grain)  {};

    BlockedRange2d( const BlockedRange2d& o ) : 
        m_coord_split_first(o.m_coord_split_first), m_coord_split_second(o.m_coord_split_second) {};
    
    BlockedRange2d(const tbb::blocked_range<int>&      tbb_coord_split_first, 
                   const tbb::blocked_range<int>&      tbb_coord_split_second,
                   BlockedRange::BlockedRangeSizeType  grain = 1 ) :
        m_coord_split_first(tbb_coord_split_first, 1), m_coord_split_second(tbb_coord_split_second, grain) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const { return (m_coord_split_first.empty() || m_coord_split_second.empty()); };
    bool is_divisible()           const { return (m_coord_split_first.is_divisible() || m_coord_split_second.is_divisible()); };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_coord_split_second.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRange2d( BlockedRange2d& o, tbb::split ) : 
        m_coord_split_first(o.m_coord_split_first), m_coord_split_second(o.m_coord_split_second)
    {
        if (m_coord_split_first.size() > 1)
        {
            m_coord_split_first = BlockedRange( o.m_coord_split_first, tbb::split() );
        }
        else
        {
            m_coord_split_second = BlockedRange( o.m_coord_split_second, tbb::split() );
        }
    }
    
protected:
   BlockedRange  m_coord_split_first;
   BlockedRange  m_coord_split_second;
};

//
// Represents contiguous range if 3D indices [left,right), including left and excluding right,
// with priority for one of the for splitting
//
class BlockedRange3d 
{
public:
    BlockedRange3d() : m_coord_split_first(), m_coord_split_second(), m_coord_split_third() {};
        
    BlockedRange3d( BlockedRange::BlockedRangeValueType min_coord_split_first, 
                    BlockedRange::BlockedRangeValueType max_coord_split_first, 
                    BlockedRange::BlockedRangeValueType min_coord_split_second, 
                    BlockedRange::BlockedRangeValueType max_coord_split_second, 
                    BlockedRange::BlockedRangeValueType min_coord_split_third, 
                    BlockedRange::BlockedRangeValueType max_coord_split_third, 
                    BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_coord_split_first (min_coord_split_first,max_coord_split_first), 
        m_coord_split_second(min_coord_split_second,max_coord_split_second), 
        m_coord_split_third (min_coord_split_third,max_coord_split_third,grain)  {};

    BlockedRange3d( const BlockedRange3d& o ) : 
        m_coord_split_first(o.m_coord_split_first), 
        m_coord_split_second(o.m_coord_split_second), 
        m_coord_split_third(o.m_coord_split_third) {};
    
    BlockedRange3d(const tbb::blocked_range<int>&      tbb_coord_split_first, 
                   const tbb::blocked_range<int>&      tbb_coord_split_second,
                   const tbb::blocked_range<int>&      tbb_coord_split_third,
                   BlockedRange::BlockedRangeSizeType  grain = 1 ) :
        m_coord_split_first (tbb_coord_split_first,  1), 
        m_coord_split_second(tbb_coord_split_second, 1),
        m_coord_split_third (tbb_coord_split_third,  grain) {};
    
    // if any is empty - the whole range is empty
    bool empty()                  const 
                { return (m_coord_split_first.empty() || m_coord_split_second.empty() || m_coord_split_third.empty()); };
    bool is_divisible()           const 
                { return (m_coord_split_first.is_divisible() || m_coord_split_second.is_divisible() || m_coord_split_third.is_divisible()); };

    BlockedRange::BlockedRangeSizeType  grainsize() const { return m_coord_split_third.grainsize(); };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRange3d( BlockedRange3d& o, tbb::split ) : 
        m_coord_split_first(o.m_coord_split_first), 
        m_coord_split_second(o.m_coord_split_second), 
        m_coord_split_third(o.m_coord_split_third)
    {
        if (m_coord_split_first.size() > 1)
        {
            m_coord_split_first = BlockedRange( o.m_coord_split_first, tbb::split() );
        } 
        else if (m_coord_split_second.size() > 1)
        {
            m_coord_split_second = BlockedRange( o.m_coord_split_second, tbb::split() );
        }
        else
        {
            m_coord_split_third = BlockedRange( o.m_coord_split_third, tbb::split() );
        }
    }
    
protected:
   BlockedRange  m_coord_split_first;
   BlockedRange  m_coord_split_second;
   BlockedRange  m_coord_split_third;    
};

//
//   Optimize access by Row (split by row)
//

typedef BlockedRange BlockedRangeByRow1d;

class BlockedRangeByRow2d : public BlockedRange2d
{
public:
    BlockedRangeByRow2d() : BlockedRange2d() {};
        
    BlockedRangeByRow2d( BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange2d( min_rows, max_rows, min_cols, max_cols, grain ) {};

    BlockedRangeByRow2d( const BlockedRangeByRow2d& o ) : BlockedRange2d( o ) {};
    
    BlockedRangeByRow2d(const tbb::blocked_range3d<int>& tbb_r) : 
        BlockedRange2d( tbb_r.rows(), tbb_r.cols(), tbb_r.cols().grainsize() ) {};
    
    const BlockedRangeByRow1d& rows() const { return m_coord_split_first; };
    const BlockedRangeByRow1d& cols() const { return m_coord_split_second; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow2d( BlockedRangeByRow2d& o, tbb::split ) : BlockedRange2d( o, tbb::split() ) {};
};

class BlockedRangeByRow3d  : public BlockedRange3d
{
public:
    BlockedRangeByRow3d() : BlockedRange3d() {};
        
    BlockedRangeByRow3d( BlockedRange::BlockedRangeValueType min_pages, 
                         BlockedRange::BlockedRangeValueType max_pages, 
                         BlockedRange::BlockedRangeValueType min_rows, 
                         BlockedRange::BlockedRangeValueType max_rows, 
                         BlockedRange::BlockedRangeValueType min_cols, 
                         BlockedRange::BlockedRangeValueType max_cols, 
                         BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange3d( min_pages,max_pages, min_rows, max_rows, min_cols, max_cols, grain ) {};

    BlockedRangeByRow3d( const BlockedRangeByRow3d& o ) : BlockedRange3d( o ) {};
    
    BlockedRangeByRow3d(const tbb::blocked_range3d<int>& tbb_r) : 
        BlockedRange3d( tbb_r.pages(), tbb_r.rows(), tbb_r.cols(), tbb_r.cols().grainsize() ) {};
        
    const BlockedRangeByRow1d& pages() const { return m_coord_split_first; };
    const BlockedRangeByRow1d& rows()  const { return m_coord_split_second;  };
    const BlockedRangeByRow1d& cols()  const { return m_coord_split_third;  };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split columns
    BlockedRangeByRow3d( BlockedRangeByRow3d& o, tbb::split ) : BlockedRange3d( o, tbb::split() ) {};
};

//
//   Optimize access by Column (split by column)
//

typedef BlockedRangeByRow1d BlockedRangeByColumn1d;

class BlockedRangeByColumn2d : public BlockedRange2d
{
public:
    BlockedRangeByColumn2d() : BlockedRange2d() {};
        
    BlockedRangeByColumn2d( BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange2d( min_cols, max_cols, min_rows, max_rows, grain ) {};

    BlockedRangeByColumn2d( const BlockedRangeByColumn2d& o ) : BlockedRange2d( o ) {};
    
    BlockedRangeByColumn2d(const tbb::blocked_range3d<int>& tbb_r) : 
        BlockedRange2d( tbb_r.cols(), tbb_r.rows(), tbb_r.rows().grainsize() ) {};
    
    const BlockedRangeByColumn1d& rows() const { return m_coord_split_second; };
    const BlockedRangeByColumn1d& cols() const { return m_coord_split_first; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn2d( BlockedRangeByColumn2d& o, tbb::split ) : BlockedRange2d( o, tbb::split() ) {};
};

class BlockedRangeByColumn3d : public BlockedRange3d
{
public:
    BlockedRangeByColumn3d() : BlockedRange3d() {};
        
    BlockedRangeByColumn3d( BlockedRange::BlockedRangeValueType min_pages, 
                            BlockedRange::BlockedRangeValueType max_pages, 
                            BlockedRange::BlockedRangeValueType min_rows, 
                            BlockedRange::BlockedRangeValueType max_rows, 
                            BlockedRange::BlockedRangeValueType min_cols, 
                            BlockedRange::BlockedRangeValueType max_cols, 
                            BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        BlockedRange3d( min_pages,max_pages, min_cols, max_cols, min_rows, max_rows, grain ) {};

    BlockedRangeByColumn3d( const BlockedRangeByColumn3d& o ) : BlockedRange3d( o ) {};
    
    BlockedRangeByColumn3d(const tbb::blocked_range3d<int>& tbb_r) :
        BlockedRange3d( tbb_r.pages(), tbb_r.cols(), tbb_r.rows(), tbb_r.rows().grainsize() ) {};
    

    const BlockedRangeByColumn1d& pages() const { return m_coord_split_first;  };
    const BlockedRangeByColumn1d& rows()  const { return m_coord_split_third;  };
    const BlockedRangeByColumn1d& cols()  const { return m_coord_split_second; };

    // make me the right side of the range, update other to be the left side of the range
    // first try to split rows
    BlockedRangeByColumn3d( BlockedRangeByColumn3d& o, tbb::split ) : BlockedRange3d( o, tbb::split() ) {};
};

//
//   Split By Tile
//

typedef BlockedRangeByRow1d BlockedRangeByTile1d;

class BlockedRangeByTile2d 
{
public:
    BlockedRangeByTile2d() : m_rows(), m_cols() {};
        
    BlockedRangeByTile2d( BlockedRange::BlockedRangeValueType min_rows, 
                          BlockedRange::BlockedRangeValueType max_rows, 
                          BlockedRange::BlockedRangeValueType min_cols, 
                          BlockedRange::BlockedRangeValueType max_cols, 
                          BlockedRange::BlockedRangeSizeType  grain = 1 ) : 
        m_rows(min_rows,max_rows,grain), m_cols(min_cols,max_cols,grain)  {};

    BlockedRangeByTile2d( const BlockedRangeByTile2d& o ) : 
        m_rows(o.m_rows), m_cols(o.m_cols) {};
    
    BlockedRangeByTile2d(const tbb::blocked_range3d<int>& tbb_r) :
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
    BlockedRangeByTile3d() : m_pages(), m_rows(), m_cols() {};
        
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
    
    BlockedRangeByDefaultTBB1d( unsigned int left, 
                                unsigned int right, 
                                size_t grain = 1 ) : 
        tbb::blocked_range<int> ( left, right, grain ) {};

    BlockedRangeByDefaultTBB1d( const BlockedRangeByDefaultTBB1d& o ) : 
        tbb::blocked_range<int> ( o ) {};
    
    BlockedRangeByDefaultTBB1d(const tbb::blocked_range3d<int>& tbb_r) :
        tbb::blocked_range<int> ( tbb_r.cols() ) {};

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
    
    BlockedRangeByDefaultTBB2d(const tbb::blocked_range3d<int>& tbb_r) :
        tbb::blocked_range2d<int> ( tbb_r.rows().begin(), tbb_r.rows().end(), tbb_r.rows().grainsize(),
                                    tbb_r.cols().begin(), tbb_r.cols().end(), tbb_r.cols().grainsize()
                                  ) {};

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

