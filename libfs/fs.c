#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"

#define FS_FILENAME_LEN 16

typedef struct __attribute__((packed)) superblock {
	uint8_t signature[8];
	uint16_t num_total_blocks;
	uint16_t root_block_index;
	uint16_t data_block_index;
	uint16_t num_data_blocks;
	uint8_t num_fat_blocks;
	uint8_t unused_padding[4079];
} superblock;

typedef struct __attribute__((packed)) root_directory {
	uint8_t filename[FS_FILENAME_LEN];
	uint32_t file_size;
	uint16_t first_data_block_index;
	uint8_t unused_padding[10];
} root_directory;

static uint16_t *fat;

//Global variables to be used
superblock *superBlock;
root_directory *rootDir;

static bool mounted = false;
static int num_open_files = 0;
static int num_files = 0;

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
}

int fs_umount(void)
{
	if(!mounted || num_open_files != 0){  //return -1 if no underlying virtual disk was opened or if there are still open file descriptors
		return -1;
	}

	//Unmount the currently mounted file system by writing superblock & root
	if (block_write(0, superBlock) == -1){
		return -1;
	}
	if (block_write((superBlock->data_block_index-1), rootDir) == -1){
		return -1;
	}

	//write fat blocks
	for (int i = 0; i < superBlock->num_fat_blocks; i++){
		if (block_write(1 + i, fat + (i*4096)) == -1){
			return -1;
		}
	}

	//close the underlying virtual disk file.
	if(block_disk_close() != 0){ //return -1 if the virtual disk cannot be closed
		return -1;
	}

	mounted = false;
	return 0;
}

int fs_info(void)
{
	/* TODO: Phase 1 */
}

int fs_create(const char *filename)
{
	if ( strlen(filename)+1 > FS_FILENAME_LEN ||  num_files+1 > FS_FILE_MAX_COUNT){ //return -1 if string @filename is too long or if the root directory already contains* %FS_FILE_MAX_COUNT files
		return -1;
	}

	for (int i = 0; i < FS_FILE_MAX_COUNT; i++){ //return -1 if a file named @filename already exists
		if (strcmp((char*)rootDir[i].filename,filename) == 0){//if two strings are same
			return -1;
		}
	}
	//String @filename must be NULL-terminated
	//Create a new and empty file named @filename in the root directory of the mounted file system

	return 0;
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
}

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
}
