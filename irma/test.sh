#!/bin/bash
cd backpack
make
cd ..
cd patch
./patch_rok101008_backpack_nodfu.py
cd ..
./irmaload.py flash
