#!/bin/bash

echo "Have you updated CHANGELOG & debian/changelog?"
read 
echo "Have you updated system/exscalibar.pc,.h?"
read 

set -e
rm -f ../exscalibar_*
debuild -S -sa
cd ..
dput -f ppa:r-launchpad-gavofyork-fastmail-fm/ppa exscalibar_*_source.changes

