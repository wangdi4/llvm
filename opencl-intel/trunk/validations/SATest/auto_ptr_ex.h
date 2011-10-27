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

File Name:  auto_ptr_ex.h

\*****************************************************************************/
#ifndef AUTO_PTR_EX_H
#define AUTO_PTR_EX_H

#include <memory>

namespace Validation { 

    template<class T> struct DefaultDP
    {
        static void Delete(T* pT) { delete pT; }
    };

    template<class T> struct ArrayDP
    {
        static void Delete(T* pT) { delete[] pT; }
    };

    template<class _Ty, class _DP>
    class auto_ptr_ex;

    template<class _Ty>
    struct auto_ptr_ex_ref
    {   // proxy reference for auto_ptr_ex copying
        explicit auto_ptr_ex_ref(_Ty *_Right)
            : _Ref(_Right)
        {   // construct from generic pointer to auto_ptr_ex ptr
        }

        _Ty *_Ref;  // generic pointer to auto_ptr_ex ptr
    };

    template<class _Ty, class _DP = DefaultDP<_Ty> >
    class auto_ptr_ex
    {   // wrap an object pointer to ensure destruction
    public:
        typedef _Ty element_type;

        explicit auto_ptr_ex(_Ty *_Ptr = 0) throw ()
            : _Myptr(_Ptr)
        {   // construct from object pointer
        }

        auto_ptr_ex(auto_ptr_ex<_Ty, _DP>& _Right) throw ()
            : _Myptr(_Right.release())
        {   // construct by assuming pointer from _Right auto_ptr_ex
        }

        auto_ptr_ex(auto_ptr_ex_ref<_Ty> _Right) throw ()
        {   // construct by assuming pointer from _Right auto_ptr_ex_ref
            _Ty *_Ptr = _Right._Ref;
            _Right._Ref = 0;    // release old
            _Myptr = _Ptr;  // reset this
        }

        template<class _Other, class _OtherDP>
        operator auto_ptr_ex<_Other, _OtherDP>() throw ()
        {   // convert to compatible auto_ptr_ex
            return (auto_ptr_ex<_Other, _OtherDP>(*this));
        }

        template<class _Other, class _OtherDP>
        operator auto_ptr_ex_ref<_Other>() throw ()
        {   // convert to compatible auto_ptr_ex_ref
            _Other *_Cvtptr = _Myptr;   // test implicit conversion
            auto_ptr_ex_ref<_Other> _Ans(_Cvtptr);
            _Myptr = 0; // pass ownership to auto_ptr_ex_ref
            return (_Ans);
        }


        template<class _Other, class _OtherDP>
        auto_ptr_ex<_Ty, _DP>& operator=(auto_ptr_ex<_Other, _OtherDP>& _Right) throw ()
        {   // assign compatible _Right (assume pointer)
            reset(_Right.release());
            return (*this);
        }

        template<class _Other, class _OtherDP>
        auto_ptr_ex(auto_ptr_ex<_Other, _OtherDP>& _Right) throw ()
            : _Myptr(_Right.release())
        {   // construct by assuming pointer from _Right
        }

        auto_ptr_ex<_Ty, _DP>& operator=(auto_ptr_ex<_Ty, _DP>& _Right) throw ()
        {   // assign compatible _Right (assume pointer)
            reset(_Right.release());
            return (*this);
        }

        auto_ptr_ex<_Ty, _DP>& operator=(auto_ptr_ex_ref<_Ty> _Right) throw ()
        {   // assign compatible _Right._Ref (assume pointer)
            _Ty *_Ptr = _Right._Ref;
            _Right._Ref = 0;    // release old
            reset(_Ptr);    // set new
            return (*this);
        }

        ~auto_ptr_ex()
        {
            _DP::Delete(_Myptr);
        }

        _Ty& operator*() const throw ()
        {   // return designated value
            assert(_Myptr != 0);
            return (*get());
        }

        _Ty *operator->() const throw ()
        {   // return pointer to class object
            assert(_Myptr != 0);
            return (get());
        }

        _Ty *get() const throw ()
        {   // return wrapped pointer
            return (_Myptr);
        }

        _Ty** getOutPtr() throw ()
        {   // return wrapped pointer
            return (&_Myptr);
        }

        _Ty *release() throw ()
        {   // return wrapped pointer and give up ownership
            _Ty *_Tmp = _Myptr;
            _Myptr = 0;
            return (_Tmp);
        }

        void reset(_Ty* _Ptr = 0)
        {   // destroy designated object and store new pointer
            if (_Ptr != _Myptr && NULL != _Myptr)
                _DP::Delete(_Myptr);
            _Myptr = _Ptr;
        }

    private:
        _Ty *_Myptr;    // the wrapped object pointer
    };
}
#endif //AUTO_PTR_EX_H