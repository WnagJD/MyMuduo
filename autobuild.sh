#!/bin/bash

set -e 

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build

fi

rm -rf `pwd`/build/*


cd `pwd`/build/ &&
        cmake ..&&
        make

cd ..

cp `pwd`/lib/libmymuduo.so /usr/lib/

if [ ! -d /usr/include/mymuduo ];then
    mkdir /usr/include/mymuduo

fi

for headers in `ls *.h`
do 
    cp $headers /usr/include/mymuduo

done

#刷新链接库的动态缓存
ldconfig



