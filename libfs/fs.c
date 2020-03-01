#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "disk.h"
#include "fs.h"



#define SIGN "ECS150FS"
#define SUPERBLOCKOFFSET 0x00


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
	superBlock =  malloc(sizeof(struct superblock));
	rootDir = malloc(sizeof(uint32_t) * BLOCK_SIZE);


	if(block_disk_open(diskname) == -1){
		return -1;
	} // If we cannot open disk

	if(block_read(SUPERBLOCKOFFSET, superBlock) == -1){
		return -1;
	} // If failed to update superblock

	if(block_read(superBlock->root_block_index, rootDir) == -1){
		return -1;
	} // Load rootDir


	int block_count = block_disk_count(); // Get count for currently opened disk
	if(block_count != superBlock->num_total_blocks){
		return -1;
	} // If the block counts don't match


	// Initialize signCheck and add \0
	char signCheck[9];
	memcpy(signCheck, superBlock->signature, 8);
	signCheck[8] = '\0';
	if(strcmp(SIGN, signCheck) != 0){
		return -1;
	} // If signature doesn't match as per specifications

	fat = malloc(sizeof(uint16_t) * superBlock->num_fat_blocks * BLOCK_SIZE);
	for(int i = 0; i < superBlock->num_fat_blocks; i++){
		if(block_read(i+1, fat + (BLOCK_SIZE*i)) == -1 ){
			return -1;
		}
	}

	mounted = true;
	return 0;
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

	int data_blk_count = block_disk_count() - (superBlock->num_fat_blocks  + 2);
	int fat_free_count = 0;

	for (int i = 0; i < (superBlock->num_data_blocks); i++){
		if(fat[i] == 0){
			fat_free_count++;
		}
	} // Find fat_free_ratio
	int root_free_count = 0;
	for (int i = 0; i <FS_FILE_MAX_COUNT; i++){

		if (rootDir->filename[0] == 0){
			root_free_count++;
		}
	}

	printf("FS Info:\n");
	printf("total_blk_count=%d\n", block_disk_count() );
	printf("fat_blk_count=%d\n", superBlock->num_fat_blocks);
	printf("rdir_blk=%d\n", superBlock->root_block_index);
	printf("data_blk=%d\n", superBlock->root_block_index+1);
	printf("data_blk_count=%d\n", data_blk_count);
	printf("fat_free_ratio=%d/%d\n", fat_free_count,data_blk_count);
	printf("rdir_free_ratio=%d/%d\n", root_free_count,FS_FILE_MAX_COUNT);

	return 0;
}

int fs_create(const char *filename)
{
	if ( strlen(filename)+1 > FS_FILENAME_LEN ||  num_files+1 > FS_FILE_MAX_COUNT){ //return -1 if string @filename is too long or if the root directory already contains* %FS_FILE_MAX_COUNT files
		return -1;
	}

	bool null_terminated = false;
	root_directory *empty_index = NULL;

	for (int x = 0; x < FS_FILENAME_LEN; x++){
		if (filename[x] == '\0'){
			null_terminated = true;
			empty_index = rootDir[x]; //find empty space
			break;
		}
	}

	if(!null_terminated){ //String @filename must be NULL-terminated
		return -1;
	}

	for (int i = 0; i < FS_FILE_MAX_COUNT; i++){ //return -1 if a file named @filename already exists
		if (strcmp((char*)rootDir[i].filename,filename) == 0){//if two strings are same
			return -1;
		}
	}

	if (empty_index == NULL){ //if empty space not found
		return -1;
	}

	//else create a new and empty file named @filename in the root directory of the mounted file system
	strcpy(empty_index->filename, filename);
	empty_index->file_size = 0;
	empty_index->first_data_blk_index = 0xFFFF;
	empty_index->filename[strlen(filename)] = '\0';

	return 0;
}

int fs_delete(const char *filename)
{
	return 0;
}

int fs_ls(void)
{
	if(block_disk_count() == -1){ //return -1 if no underlying virtual disk was opened.
		return -1;
	}
	/* FS Ls:file: %s, size: %d, data_blk: %d*/
	printf("FS Ls:\n");
	for(int i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (rootDir[i].filename[0] != 0)
		{
			printf("file: %s,", (char*)rootDir[i].filename);
			printf(" size: %u,", rootDir[i].file_size);
			printf(" data_blk: %u\n", rootDir[i].first_data_block_index);
		}
	}

	return 0;
}

int fs_open(const char *filename)
{
	return 0;
}

int fs_close(int fd)
{
	return 0;
}

int fs_stat(int fd)
{

	return 0;

}

int fs_lseek(int fd, size_t offset)
{
	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
	return 0;
}
