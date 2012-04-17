#!/bin/sh

OCL_ROOT=/opt/AMD-APP-SDK-v2.4-lnx64

# do the actual configuration
sh ./configure \
    CPPFLAGS="-I$OCL_ROOT/include" \
    LDFLAGS="-L$OCL_ROOT/lib/x86_64" \
    --without-cuda \
    --disable-stability

# other useful options
#    --enable-m64
#    --disable-m64

