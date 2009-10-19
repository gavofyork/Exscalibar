#!/bin/bash

set -e
rm -f ../exscalibar_*
debuild -S -sa
cd ..
dput -f ppa:r-launchpad-gavofyork-fastmail-fm/ppa exscalibar_*_source.changes

