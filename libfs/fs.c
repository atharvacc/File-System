#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"


#define SIGN "ECS150FS"
#define SUPERBLOCKOFFSET 0x00

typedef struct __attribute__((packed)) superblock {
	uint8_t signature[8];
	uint16_t num_total_blocks;
	uint16_t root_block_index;
	uint16_t data_bock_index;
	uint16_t num_data_blocks;
	uint8_t num_fat_blocks;
	uint8_t unused_padding[4079];
} superblock;

typedef struct __attribute__((packed)) root_directory {
	uint8_t filename[16];
	uint32_t file_size;
	uint16_t first_data_block_index;
	uint8_t unused_padding[10];
} root_directory;

static uint16_t *fat;

//Global variables to be used
superblock *superBlock; 
root_directory *rootDir;
//static int num_open_files = 0;



int fs_mount(const char *diskname)
{
	superBlock =  malloc(sizeof(struct superblock));
	rootDir = malloc(sizeof(struct root_directory));
	

	if(block_disk_open(diskname) == -1){
		return -1;
	} // If we cannot open disk 
	printf("Succesfully opened disk \n");
	if(block_read(SUPERBLOCKOFFSET, superBlock) == -1){
		return -1;
	} // If failed to update superblock

	printf("SuperBlock updated succesfully \n");
	int block_count = block_disk_count(); // Get count for currently opened disk
	if(block_count != superBlock->num_total_blocks){
		return -1;
	} // If the block counts don't match

	printf("Block counts match \n");
	// Initialize signCheck and add \0
	char signCheck[9];
	memcpy(signCheck, superBlock->signature, 8);
	signCheck[8] = '\0';
	if(strcmp(SIGN, signCheck) != 0){
		return -1;
	} // If signature doesn't match as per specifications

	printf("Signature matched as well \n");
	fat = malloc(sizeof(uint16_t) * superBlock->num_total_blocks * BLOCK_SIZE);
	printf("Number of FAT blocks : %d \n", superBlock->num_fat_blocks);
	for(int i = 0; i < superBlock->num_fat_blocks; ++i){
		printf("using block %d \n", i);
		if(block_read(i+1, fat + (BLOCK_SIZE*i)) == -1 ){
			return -1;
		}
	}
	printf("Done updating FAT \n");
	return 0;


}

int fs_umount(void)
{
	return 0;
}

int fs_info(void)
{
	
	return 0;
}

int fs_create(const char *filename)
{
	return 0;
}

int fs_delete(const char *filename)
{
	return 0;
}

int fs_ls(void)
{
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
