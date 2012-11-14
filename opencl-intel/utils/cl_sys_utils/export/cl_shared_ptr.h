// Copyright (c) 2006-2012 Intel Corporation
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

#pragma once

// macro that each class T that wish to user SharedPtr<T> must call
#define PREPARE_SHARED_PTR(T)    \
    friend class Intel::OpenCL::Utils::SharedPtr<T >;  \
    friend class Intel::OpenCL::Utils::SharedPtr<const T >; \
    SharedPtr<T > operator&() { return SharedPtr<T >(this); } \
    ConstSharedPtr<T > operator&() const { return ConstSharedPtr<T >(this); }

namespace Intel { namespace OpenCL { namespace Utils {

/**
 * Initialize the shared pointers mechanism (needed for debug mode only)
 */
void InitSharedPtrs();

/**
 * Finalize the shared pointers mechanism (needed for debug mode only)
 */
void FiniSharedPts();

/**
 * Represents an abstract data type that simulates a pointer while providing additional features
 */
template<typename T>
class SmartPtr
{

public:

    /**
     * Destructor
     */
    virtual ~SmartPtr() { }

    /**
     * Cast operator to size_t
     * @return the address of the pointed to object as a size_t type
     */
    operator size_t() const { return (size_t)m_ptr; }

    /**
     * Indirection operator - this method should not be called if GetPtr() returns NULL
     * @return a reference to the pointed to object
     */
    T& operator*() const
    {
        return *m_ptr;
    }

    /**
     * Object derefernce operator - this method should not be called if GetPtr() returns NULL.
     * Note: the intension of this class is to prevent the user from getting a hold of T* directly.
     *  However, using this operator one can do this:
     *
     *      SharedPtr<T> p;
     *      T* pT = p.operator->();
     *
     *  But using this operator this way is an abuse. We provide this operator as a syntactic
     *  sugar instead of "(*p).foo();".
     *
     * @return a pointer to the pointed to object
     */
    T* operator->() const
    {
        return m_ptr;
    }

    /**
     * Note: use this method with caution - you get an unmanaged pointer!
     * @return the pointed to object
     */
    T* GetPtr() const { return m_ptr; }

protected:

    /**
     * Constructor
     * @param ptr a pointer to the object
     */
    SmartPtr(T* ptr = NULL) : m_ptr(ptr) { }

    /**
     * the pointer to the object
     */
    T* m_ptr;

};

// forward declaration
template<typename T> class ConstSharedPtr;
template<typename T> class ConstWeakPtr;

/**
 * This template class represents a shared pointer to an object and offers regular pointer
 * semantics by overriding the dereferencing operators. This class deliberately does not directly
 * expose the object's pointer to the user, so that no accidental uncontrolled deletion of it might
 * occur. It handles reference counting of the object and deletes it upon SharedPtr's destruction
 * if it finds that it holds the last reference to the object.
 * Note that it is the pointed object's responsibility to ensure synchronization of operations on
 * the reference count. SharedPtr class itself cannot avoid a race between the destruction of one
 * SharedPtr and the deletion of its pointed to object and the incrementing of the object's
 * reference count by another SharedPtr in the case where these two SharedPtr objects are the only
 * shared pointers to the object (see documentation of the constructors, assignment operator and
 * destructor for more details).
 *
 * @param T the type of object pointed to by the SharedPtr object (do not use SharedPtr<const T>
 *          directly; @see ConstSharedPtr)
 *          SharedPtr assumes the following methods are defined for T:
 *          void IncRefCnt()
 *          long DecRefCnt()
 *          T* GetPtr()
 */
template <typename T>
class SharedPtr : public SmartPtr<T>
{
public:

    /**
     * Constructor
     * @param ptr a pointer to object of type T to which this SharedPtr will point or NULL. ptr's
     *            reference counter is incremented in this method if it is not NULL.
     */
    SharedPtr(T* ptr = NULL) : SmartPtr<T>(ptr)
    {
        if (NULL != this->m_ptr)
        {
            IncRefCnt();
        }
    }   

    /**
     * Copy constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtr whose object pointer will be copied to this SharedfPtr. The
     *              reference counter of the object is incremented in this method if it is not
     *              NULL.
     */
    SharedPtr(const SharedPtr& other) : SmartPtr<T>(NULL)
    {
        *this = other;
    }

    template<typename S>
    SharedPtr(const SharedPtr<S>& other) : SmartPtr<T>(NULL)
    {       
        *this = other;
    }

#ifdef _HAS_CPP0X		
    /**
     * Move constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtr whose object pointer will be moved to this SharedfPtr.
     */
    SharedPtr(SharedPtr&& other) : SmartPtr<T>(other.GetPtr())
    {
        other.NullifyWithoutDecRefCnt();
    }

    template<typename S>
    SharedPtr(SharedPtr<S>&& other) : SmartPtr<T>(other.GetPtr())
    {
        other.NullifyWithoutDecRefCnt();
    }
#endif

    /**
     * Destructor
     * The reference counter of the currently pointed to object is decremented and it is deleted if
     * it becomes 0 (if it is not NULL).
     */
    ~SharedPtr()
    {
        DecRefCnt();
    }

    /**
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @return a SharedPtr<S> holding the pointed to object of this or holding NULL if this' object
     *         cannot be dynamically cast to S*
     */
    template<typename S>
    SharedPtr<S> DynamicCast() const
    {
        S* const pS = dynamic_cast<S*>(this->m_ptr);
        if (NULL != pS)
        {
            return pS;
        }
        else
        {
            return SharedPtr<S>();
        }
    }

    /**
     * Cast operator to ConstSharedPtr<T>
     * @return a ConstSharedPtr pointing to the object this points to
     */
    operator ConstSharedPtr<T>() const;

    /**
     * l-value Assignment operators
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other another SharedPtr whose object pointer will be copied to this SharedfPtr. The
     *              reference counter of the currently pointed to object is decremented and it is
     *              deleted if it becomes 0 (if it is not NULL). The reference counter of the newly
     *              pointed to object is incremented in this method if it is not NULL.
     * @return a reference to this
     */
    SharedPtr& operator=(const SharedPtr& other)
    {
        return operator=<T>(other);
    }

    template<typename S>
    SharedPtr& operator=(const SharedPtr<S>& other)
    {
        if (other == *this)
        {
            return *this;
        }

        // get hold of the original object address
        T* ptr = this->m_ptr;
        // update m_ptr to the new address
        if (NULL != other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = NULL;
        }
        // increment the reference counter if m_ptr isn't null
        if (NULL != this->m_ptr)
        {
            IncRefCnt();
        }
        /* Only now can we decrement the reference counter of the old address. This is to prevent the case where DecRefCntInt would cause the object containing this SharedPtr to be deleted and
            then accesses to m_ptr would already be invalid, if they had been performed after the call to DecRefCntInt. */
        DecRefCntInt(ptr);
        return *this;
    }

#ifdef _HAS_CPP0X	
    /**
     * r-value Assignment operators
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other another SharedPtr whose object pointer will be moved to this SharedfPtr. The
     *              reference counter of the currently pointed to object is decremented and it is
     *              deleted if it becomes 0 (if it is not NULL).
     * @return a reference to this
     */

    SharedPtr& operator=(SharedPtr&& other)
    {
        // see comments in l-value assignment operator
        T* ptr = this->m_ptr;
        if (other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = NULL;
        }
        DecRefCntInt(ptr);
        other.NullifyWithoutDecRefCnt();
        return *this;
    }

    template<typename S>
    SharedPtr& operator=(SharedPtr<S>&& other)
    {
        // see comments in l-value assignment operator
        T* ptr = this->m_ptr;        
        if (other.GetPtr())
        {
            this->m_ptr = other.GetPtr();
        }
        else
        {
            this->m_ptr = NULL;
        }
        DecRefCntInt(ptr);
        other.NullifyWithoutDecRefCnt();
        return *this;
    }
#endif

    /**
     * nullify the pointer without decrementing the reference counter of the pointered to object.
     * NOTE: this is an internally auxiliary function not to be used outside of this class. It is not declared private, because we need to call it on SharedPtr objects of
     * different type parameters, which are actually not the same class.
     */
    void NullifyWithoutDecRefCnt()
    {
        this->m_ptr = NULL;
    }

    /**
     * manually increment the reference counter of the pointed to object
     */
    void IncRefCnt();

    /**
     * manually decrement the reference counter of the pointed to object
     */
    void DecRefCnt()
    {
        DecRefCntInt(this->m_ptr);
    }

    /**
     * @return the value of the pointed to object's reference count
     */
    long GetRefCnt() const;

private:

    void DecRefCntInt(T* ptr);

    friend class ConstSharedPtr<T>;    

};

/**
 * This template class represents a const shared pointer to an object. It extends
 * SharedPtr<const T> by adding an option to construct a ConstSharedPtr<T> from a
 * SharedPtr<T>, as a regular const pointer can be initialized by a regular non-
 * const pointer.
 * 
 * @param T the type of object pointed to by the ConstSharedPtr (without the 'const' modifier)
 */
template<typename T>
class ConstSharedPtr : public SharedPtr<const T>
{
    
public:
    
    /**
     * Constructor
     * @param ptr a const pointer to object of type T to which this ConstSharedPtr will point.
     *            ptr's reference counter is incremented in this method.
     */
    ConstSharedPtr(const T* ptr = NULL) : SharedPtr<const T>(ptr) { }

    /**
     * Constructors
     * @param S a type that a pointer to T can be cast to a pointer of it
     * @param other SharedPtr whose object pointer will be copied to this ConstSharedfPtr. The
     *              reference counter of the object is incremented in this method if it is not
     *              NULL.
     */
    ConstSharedPtr(const ConstSharedPtr& other) : SharedPtr<const T>(other) { }

    template<typename S>
    ConstSharedPtr(const ConstSharedPtr<S>& other) : SharedPtr<const T>(other) { }
    
};

/**
 * Represents a pointer that does not affect the reference count of the pointed to object
 */
template<typename T>
class WeakPtr : public SmartPtr<T>
{

public:

    /**
     * Constructor
     * @param ptr a pointer to the object
     */
    explicit WeakPtr(T* ptr = NULL) : SmartPtr<T>(ptr) { }

    /**
     * Cast operator to ConstWeakPtr<T>
     * @return a ConstWeakPtr pointing to the object this points to
     */
    operator ConstWeakPtr<T>() const
    {
        return ConstWeakPtr<T>(this->m_ptr);
    }

    /**
     * Cast operator to SharedPtr<T>
     * @return a SharedPtr pointing to the object this points to
     */
    operator SharedPtr<T>() const
    {
        return SharedPtr<T>(this->m_ptr);
    }

};

/**
 * Represents a const weak pointer
 */
template<typename T>
class ConstWeakPtr : public WeakPtr<const T>
{

public:

    /**
     * Constructor
     * @param ptr a pointer to the object
     */
    explicit ConstWeakPtr(const T* ptr = NULL) : WeakPtr<const T>(ptr) { }

    /**
     * Cast operator to ConstSharedPtr<T>
     * @return a ConstSharedPtr pointing to the object this points to
     */
    operator ConstSharedPtr<T>() const
    {
        return ConstSharedPtr<T>(this->m_ptr);
    }

};

}}}
