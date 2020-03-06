#!/bin/bash

#set -o xtrace


echo CREATING FILES
echo -----------------------
rm -rf disk.fs
fs_make.x disk.fs 4096
fs_make.x disk1.fs 4096

echo Testing add 
echo -----------------------
## Test add 
fs_ref.x add disk.fs hello_test.txt 
test_fs.x add disk1.fs hello_test.txt 

echo Testing ls
echo -----------------------
## Test ls again
test_fs.x ls disk.fs > my_o
fs_ref.x ls disk1.fs > ref_o
diff ref_o my_o
rm -rf ref_o my_o


echo Testing info
echo -----------------------
## Test info
test_fs.x info disk.fs > my_o
fs_ref.x info disk.fs > ref_o
diff ref_o my_o
rm -rf ref_o my_o

echo Testing ls
echo -----------------------
## Test ls 
test_fs.x ls disk.fs > my_o
fs_ref.x ls disk.fs > ref_o
diff ref_o my_o
rm -rf ref_o my_o

echo Testing cat
echo -----------------------
##Test cat 
test_fs.x cat disk.fs hello_test.txt > my_o
fs_ref.x cat disk.fs  hello_test.txt > ref_o
diff ref_o my_o
rm -rf ref_o my_o

echo Testing stat
echo -----------------------
## Test stat
test_fs.x stat disk.fs > my_o
fs_ref.x stat disk.fs > ref_o
diff ref_o my_o
rm -rf ref_o my_o

echo Testing rm
echo -----------------------
## Test rm 
fs_ref.x rm disk.fs hello_test.txt 
fs_ref.x add disk.fs hello_test.txt
test_fs.x rm disk.fs hello_test.txt 

echo Testing ls
echo -----------------------
## Test ls again
test_fs.x ls disk.fs > my_o
fs_ref.x ls disk.fs > ref_o
diff ref_o my_o
rm -rf ref_o my_o

echo DONE
