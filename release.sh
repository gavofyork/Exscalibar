#!/bin/bash

opwd=$PWD
cd /tmp

echo Checking out...
git clone $opwd

cd exscalibar
version=$(grep EXSCALIBAR_VERSION system/exscalibar.h | sed "s:.define EXSCALIBAR_VERSION ::g" | sed s:\"::g)
perl -i -p -e "s/^Version: .*/Version: $version/g" system/exscalibar.pc
archdir="exscalibar-$version"
archfile="exscalibar-$version.tar.bz2"
echo Version: $version

echo Cleaning documentation...
rm -rf xml html latex exscalibar.tag

echo Cleaning backup files...
find . | grep \~ | xargs rm -f

echo Cleaning others...
rm release.sh

echo Versioning...
rm -rf .git .gitignore

echo Switching to release mode...
perl -i -p -e "s/debug/release/g" exscalibar.pri

echo Renaming directory...
cd ..
mv exscalibar $archdir

echo Creating archive...
tar c $archdir | bzip2 -- > $archfile

echo Packaging...
cd "$archdir"
./package.sh

echo Cleaning up...
cd ..
rm -rf $archdir
mv /tmp/$archfile "$opwd"

echo "Done."
