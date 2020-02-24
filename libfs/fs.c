#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

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

static uint16_t fat;

int fs_mount(const char *diskname)
{
	/* TODO: Phase 1 */
}

int fs_umount(void)
{
	/* TODO: Phase 1 */
}

int fs_info(void)
{
	/* TODO: Phase 1 */
}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
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
