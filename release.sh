#!/bin/bash

opwd=$PWD
cd /tmp

echo Downloading...
cvs -d:ext:emmcee@cvs.sf.net:/cvsroot/exscalibar co exscalibar > /dev/null

cd exscalibar
version=`grep \<version\> exscalibar.kdevelop | grep -E "[0-9]+\.[0-9]+\.[0-9]+" | sed s:\ *\</*version\>::g`
archdir="exscalibar-$version"
archfile="exscalibar-$version.tar.bz2"

echo Cleaning CVS files...
find . | grep CVS | xargs rm -rf
find . | grep .cvsignore | xargs rm -f

echo Cleaning documentation...
rm -rf xml html latex exscalibar.tag

echo Cleaning backup files...
find . | grep "~" | xargs rm -f

echo Cleaning others...
rm release.sh

echo Switching to release mode...
perl -i -p -e "s/debug/release/g" global.tmpl

echo Renaming directory...
cd ..
mv exscalibar $archdir

echo Creating archive...
tar c $archdir | bzip2 -- > $archfile

rm -rf $archdir
cd $opwd
mv /tmp/$archfile .

echo "Done."
