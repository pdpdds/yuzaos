#pragma once
#include <errno.h>
#include <stdio.h>
//#include <endian.h>
#include <stdint.h>
#include <string.h>
#include "getopt.h"
#include <stdbool.h>
#include "uuid.h"
#include <sys/types.h>
#include <unistd.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <time.h>

#define htobe32(x) htonl(x)
#define htobe16(x) htonl16(x)
#define be32toh(x) swap_uint32 (x)
# define htobe64(x) swap_uint64(x)


#define COOKIE(x)           (*(uint64_t *) x)
#define COOKIE32(x)         (*(uint32_t *) x)
#define FOOTER_FEAT_RSVD    (2)
#define VHD_VERSION_1       (0x00010000UL)
#define VHD_VMAJ_MASK       (0xFFFF0000UL)
#define VHD_VMIN_MASK       (0x0000FFFFUL)
#define DYN_VERSION_1       (0x00010000UL)
#define DYN_VMAJ_MASK       (0xFFFF0000UL)
#define DYN_VMIN_MASK       (0x0000FFFFUL)
#define FOOTER_DOFF_FIXED   (0xFFFFFFFFFFFFFFFFULL)
#define DYN_DOFF_DYN        (0xFFFFFFFFFFFFFFFFULL)
#define SECONDS_OFFSET      946684800
#define FOOTER_TYPE_FIXED   (2)
#define FOOTER_TYPE_DYN     (3)
#define FOOTER_TYPE_DIFF    (4)
#define SEC_SHIFT           (9)
#define SEC_SZ              (1 << SEC_SHIFT)
#define SEC_MASK            (SEC_SZ - 1)
#define round_up(what, on) ((((what) + (on) - 1) / (on)) * (on))
#define DYN_BLOCK_SZ        0x200000
#define BAT_ENTRY_EMPTY     0xFFFFFFFF

/* All fields Big-Endian */
struct vhd_id
{
	uint32_t f1;
	uint16_t f2;
	uint16_t f3;
	uint8_t  f4[8];
};

/* All fields Big-Endian */
struct vhd_chs
{
	uint16_t c;
	uint8_t  h;
	uint8_t  s;
};

/* All fields Big-Endian */
struct vhd_footer
{
	uint64_t cookie;
	uint32_t features;
	uint32_t file_format_ver;
	uint64_t data_offset;
	uint32_t time_stamp;
	uint32_t creator_app;
	uint32_t creator_ver;
	uint32_t creator_os;
	uint64_t original_size;
	uint64_t current_size;
	struct vhd_chs disk_geometry;
	uint32_t disk_type;
	uint32_t checksum;
	struct vhd_id vhd_id;
	uint8_t saved_state;
	uint8_t reserved[427];
};

/* All fields Big-Endian */
struct vhd_ploc
{
	uint32_t code;
	uint32_t sectors;
	uint32_t length;
	uint32_t reserved;
	uint64_t offset;
};

/* All fields Big-Endian */
struct vhd_dyn
{
	uint64_t cookie;
	uint64_t data_offset;
	uint64_t table_offset;
	uint32_t header_version;
	uint32_t max_tab_entries;
	uint32_t block_size;
	uint32_t checksum;
	struct vhd_id parent;
	uint32_t parent_time_stamp;
	uint32_t reserved0;
	uint8_t parent_utf16[512];
	struct vhd_ploc pe[8];
	uint8_t reserved1[256];
};

typedef uint32_t vhd_batent;

typedef int (*op_read_t)(struct vhd*, void*, off64_t, size_t);
typedef int (*op_write_t)(struct vhd*, void*, off64_t, size_t);


struct vhd
{
	struct vhd_footer footer;
	struct vhd_dyn dyn;
	char uuid_str[37];
	char* name;
	off64_t size;
	off64_t offset;
	FILE* fd;
	off64_t file_size;
	uint32_t type;

#define OPEN_RAW_OK (1 << 1)
#define OPEN_RW     (1 << 2)
#define OPEN_CREAT  (1 << 3)
#define COMPAT_SIZE (1 << 4)
	unsigned flags;
	op_read_t read;
	op_write_t write;
};

#ifdef __cplusplus
extern "C" {
#endif
int vhd_open(struct vhd* vhd, char* name, unsigned flags);
int vhd_close(struct vhd* vhd, int status);
int op_raw_read(struct vhd* vhd, void* buf, off64_t offset, size_t size);
int op_raw_write(struct vhd* vhd, void* buf, off64_t offset, size_t size);

#ifdef __cplusplus
}
#endif