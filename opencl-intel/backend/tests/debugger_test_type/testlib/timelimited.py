# timelimited - provide a way to invoke a Python callable with a time limit.
# note that if the function doesn't finish, this will leave a thread running,
# so use with caution.
#
# based on http://code.activestate.com/recipes/576780-timeout-for-nearly-any-callable/
#
from threading import Thread

try:
    _Thread_stop = Thread._Thread__stop
except AttributeError:  # for Python 3.0
    _Thread_stop = Thread._stop


class TimeLimitExpired(Exception):
    """ Exception raised when time limit expires.
    """
    pass


def timelimited(timeout, function, *args, **kwds):
    """ Invoke the given function with the positional and
        keyword arguments under a time constraint.

        The function result is returned if the function
        finishes within the given time limit, otherwise
        a TimeLimitExpired error is raised.

        The timeout value is in seconds and has the same
        resolution as the standard time.time function.  A
        timeout value of None invokes the given function
        without imposing any time limit.

        A TypeError is raised if function is not callable,
        a ValueError is raised for negative timeout values
        and any errors occurring inside the function are
        passed along as-is.
    """
    class _Timelimited(Thread):
        _error_  = TimeLimitExpired  # assume timeout
        _result_ = None

        def run(self):
            try:
                self._result_ = function(*args, **kwds)
                self._error_ = None
            except Exception as e:
                self._error_ = e

        def _stop(self):
            # force the thread to stop by (ab)using the private __stop method
            if self.is_alive():
                _Thread_stop(self)

    if not hasattr(function, '__call__'):
        raise TypeError('function not callable: %s' % repr(function))

    if timeout is None:
        return function(*args, **kwds)

    if timeout < 0:
        raise ValueError('timeout invalid: %s' % repr(timeout))

    t = _Timelimited()
    t.start()
    t.join(timeout)

    if t._error_ is None:
        return t._result_

    if t._error_ is TimeLimitExpired:
        t._stop()
        raise TimeLimitExpired('timeout %r for %s' % (timeout, repr(function)))
    else:
        raise t._error_


#-----------------------------------------------------------------------------
if __name__ == "__main__":
    import time

    def waiter(arg):
        print('got', arg)
        time.sleep(3)
        return arg + 2

    try:
        result = timelimited(2, waiter, 72)
        print('result =', result)
    except TimeLimitExpired as e:
        print('time limit expired')
