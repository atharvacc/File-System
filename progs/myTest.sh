#!/bin/bash

set -o xtrace

rm -rf disk.fs
fs_make.x disk.fs 4096
fs_ref.x add disk.fs hello_test.txt 

test_fs.x ls disk.fs > my_o
fs_ref.x ls disk.fs > ref_o
diff ref_o my_o