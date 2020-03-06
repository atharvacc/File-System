#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "disk.h"
#include "fs.h"



#define SIGN "ECS150FS"
#define SUPERBLOCKOFFSET 0x00
#define FAT_EOC 0xFFFF


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

typedef struct file {
	root_directory* root_dir;
	size_t offset;
} file;

static int num_files = 0;
static uint16_t *fat;

//Global variables to be used

superblock *superBlock;
root_directory *rootDir;
file *file_descriptor;

static bool mounted = false;
int num_open_files = 0;




int fs_mount(const char *diskname)
{
	// Initialize globals
	file_descriptor = malloc(sizeof(struct file) * FS_OPEN_MAX_COUNT);
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
		if (rootDir[i].filename[0] == '\0'){
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
	if ( strlen(filename)+1 > FS_FILENAME_LEN ||  num_files+1 > FS_FILE_MAX_COUNT){ 
		return -1;
	}//return -1 if string @filename is too long or if the root directory already contains* %FS_FILE_MAX_COUNT files

	for (int i = 0; i < FS_FILE_MAX_COUNT; i++){ 
		if (strcmp((char*)rootDir[i].filename,filename) == 0){//if two strings are same
			return -1;
		}
	}//return -1 if a file named @filename already exists

	int fat_index = 0;
	for (fat_index = 0;fat_index < superBlock->num_data_blocks; fat_index++){
		if(fat[fat_index ] == 0){
			break;
		}
	}// Find the fat index for an empty slot
	
	for (int i =0; i < FS_FILE_MAX_COUNT; i ++){
		if(rootDir[i].filename[0] == '\0'){
			rootDir[i].file_size = 0;
			rootDir[i].first_data_block_index = fat_index;
			fat[fat_index] = FAT_EOC;
			strcpy( (char*)rootDir[i].filename , filename);
			break;
		} // If empty slot then can create
	}// Iterate through every available root dir entry to find an empty slot

	return 0;
}

int fs_delete(const char *filename)
{
	
	if (filename == NULL){
		return -1;
	}// invalid name
	printf("No invalid name \n");
	int file_loc = 0;
	for ( file_loc = 0; file_loc < FS_FILE_MAX_COUNT; file_loc ++){
		if (strcmp((char*)rootDir[file_loc].filename, filename) == 0){
		printf("Matchin file name was found at loc %d \n", file_loc);
			break;
		}// If match file found
	}
	
	
	if (file_loc == FS_FILE_MAX_COUNT){
		return -1;
	}

	for(int i = 0; i <FS_OPEN_MAX_COUNT; i++){
		if(file_descriptor[i].root_dir->filename != NULL){
		if(strcmp((char*)file_descriptor[i].root_dir->filename, filename) == 0){
			return -1;
		}
		}
	} // Check if the file is open

	printf(" NO issues with location being too far away or non existance of file \n");
	uint16_t data_block_index, temp_hold;
	data_block_index = rootDir[file_loc].first_data_block_index;
	while(data_block_index != FAT_EOC){
		temp_hold = fat[data_block_index];
		fat[data_block_index] = 0;
		data_block_index = temp_hold;
	}// Clear out the fat block

	printf("Cleared out FAT BLOCK \n");
	rootDir[file_loc].file_size = 0;
	rootDir[file_loc].filename[0] = '\0';
	rootDir[file_loc].first_data_block_index = 0;

	printf("Cleared out rootDir \n");
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
		if (rootDir[i].filename[0] != '\0')
    	{
			printf("file: %s,", (char*)rootDir[i].filename);
			printf(" size: %d,", rootDir[i].file_size);
			printf(" data_blk: %d\n", rootDir[i].first_data_block_index);
		}
	}
	return 0;
}

int fs_open(const char *filename)
{
	if (filename == NULL){
		return -1;
	} // Invalid filename

	if(num_open_files == FS_OPEN_MAX_COUNT){
		return -1;
	} // Max number of files open

	int rootDir_idx = 0;
	for (rootDir_idx = 0; rootDir_idx < FS_FILE_MAX_COUNT; rootDir_idx++){ 
		if (strcmp((char*)rootDir[rootDir_idx].filename,filename) == 0){//if two strings are same
			break;
		}// 
	} // Check if file exists within the rootDirectory

	if (rootDir_idx == FS_FILE_MAX_COUNT){
		return -1;
	} // File doesn't exist

	int fd_idx = 0;
	for (fd_idx = 0; fd_idx< FS_OPEN_MAX_COUNT; fd_idx++){
		if(file_descriptor[fd_idx].root_dir == NULL ){
			break;
		}
	}// find first empty slot within the file descript

	file_descriptor[fd_idx].root_dir = &(rootDir[rootDir_idx]);
	file_descriptor[fd_idx].offset = 0;
	
	num_open_files++;
	return fd_idx;
}

int fs_close(int fd)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT || file_descriptor[fd].root_dir == NULL){ 
		return -1;
	}//32 is max open count
	file_descriptor[fd].root_dir = NULL;
	file_descriptor[fd].offset = 0;
	num_open_files--;
	return 0;
}

int fs_stat(int fd)
{
	
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT || file_descriptor[fd].root_dir == NULL){ 
		return -1;
	}//32 is max open count
	return (int)file_descriptor[fd].root_dir->file_size;

}

int fs_lseek(int fd, size_t offset)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT || file_descriptor[fd].root_dir == NULL){ 
		return -1;
	}//32 is max open count

	if (offset < 0 || offset > file_descriptor[fd].root_dir->file_size){
		return -1;
	} // offset is negative or greater than file size
	file_descriptor[fd].offset = offset;
	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	return 0;
}


int fs_read(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT || file_descriptor[fd].root_dir == NULL){ 
		return -1;
	}//If fd is invalid or file isn't open
	
	int fat_idx = file_descriptor[fd].root_dir->first_data_block_index;
	int size = file_descriptor[fd].root_dir->file_size;
	int offset = file_descriptor[fd].offset;
	
	if(offset + count > size){
		count = size - offset;
	}
	
	int totBlocks = ceil(count/BLOCK_SIZE) + 1;
	int* bounceBuffer = (int*) malloc( sizeof(int) * totBlocks * BLOCK_SIZE);

	while(BLOCK_SIZE< offset){
		fat_idx = fat[fat_idx];
		offset = offset - BLOCK_SIZE;
	}// find the fat block from where we want to start reading and find how deep into that block we are   

	 for (int i = 0; i <totBlocks; i++){
		 block_read(fat_idx + superBlock->data_block_index , bounceBuffer + (i * BLOCK_SIZE) );
		 fat_idx = fat[fat_idx];
	 }

	 memcpy(buf, bounceBuffer+ offset, count);
	
	file_descriptor[fd].offset += count;
	return count;
}
