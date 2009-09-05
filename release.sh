#!/bin/bash

opwd=$PWD
cd /tmp

echo Checking out...
git clone $opwd

cd exscalibar
version=`grep EXSCALIBAR_VERSION \"$$PWD/system/exscalibar.h\" | sed \"s:.define EXSCALIBAR_VERSION ::g\""`
archdir="exscalibar-$version"
archfile="exscalibar-$version.tar.bz2"

echo Cleaning documentation...
rm -rf xml html latex exscalibar.tag

echo Cleaning backup files...
find . | grep \~ | xargs rm -f

echo Cleaning others...
rm release.sh

echo Switching to release mode...
perl -i -p -e "s/debug/release/g" exscalibar.pri

echo Renaming directory...
cd ..
mv exscalibar $archdir

echo Creating archive...
tar c $archdir | bzip2 -- > $archfile

rm -rf $archdir
cd $opwd
mv /tmp/$archfile .

echo "Done."
