
class IMutex
{
public:
    IMutex() {}
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
    virtual ~IMutex(){}
private:
    //Disallow copying
    IMutex( const IMutex& im ) {}
};


class OclMutex: public IMutex
{
public:
    OclMutex( unsigned int uiSpinCount = 4000 );
    virtual ~OclMutex ();
    void Lock();
    void Unlock();
protected:
    void* m_mutexHndl;
    void* m_mutexAttr;
private:
    unsigned int m_uiSpinCount;
};


class OclAutoMutex
{
public:
    OclAutoMutex( IMutex* mutexObj, bool bAutoLock = true );
    ~OclAutoMutex();

private:
    IMutex* m_mutexObj;
};

