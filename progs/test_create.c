#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fs.h>


int main(int argc, char **argv)
{
    fs_mount("disk.fs");
    fs_create("myFile1.txt");
    fs_create("myFile.txt");
    //fs_delete("myFile.txt");
     //fs_open("myFile.txt");
    //fs_close(fd);
    //fs_delete("myFile.txt");
    fs_umount();
    return 0;

}
