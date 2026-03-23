#!/bin/bash

#
# apt-get install cmake 
# apt-get install libssl-dev
# 

echo "--- Build pcre ---"
if [ ! -d "libs/pcre-8.45.tar.bz2" ]; then 
 tar -xpjf libs/pcre-8.45.tar.bz2 -C ./libs/ && rm -f libs/pcre && ln -s ../libs/pcre-8.45 libs/pcre
fi
cd libs/pcre/ && ./configure --enable-static=yes && make
if [ $? -ne 0 ]; then echo "ERROR: build failed!";exit 1; fi
cd ../..


echo "--- Build libre ---"
if [ ! -d "libs/re-4.5.0" ]; then 
 tar -xpzf libs/re-4.5.0.tar.gz -C libs/ && rm -f libs/re && ln -s ../libs/re-4.5.0 libs/re
fi
cd libs/re-4.5.0 && make clean && make dist
if [ $? -ne 0 ]; then echo "ERROR: build failed!"; exit 1; fi
cd ../..


echo "--- Build baresip ---"
if [ ! -d "libs/baresip-4.5.0" ]; then
 tar -xpzf libs/baresip-4.5.0.tar.gz -C libs/ && rm -f libs/baresip && ln -s ../libs/baresip-4.5.0 libs/baresip
 patch -d libs -p0 < libs/baresip.patch
fi
cd libs/baresip-4.5.0 && cmake -B build -DSTATIC=ON -DMODULES="account;contact;aubridge;auconv;mixausrc;mixminus;auresamp;stun;turn;ice;natpmp;pcp;srtp;mwi;g711;l16" && cmake --build build && cmake --install build --prefix dist
if [ $? -ne 0 ]; then echo "ERROR: build failed!"; exit 1; fi
cd ../..


echo "--- Build embpx ---"
cd empbx && make clean all install
if [ $? -ne 0 ]; then echo "ERROR: build failed!"; exit 1; fi
cd ..



