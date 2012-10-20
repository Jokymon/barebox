#ifndef __FILE_TYPE_H
#define __FILE_TYPE_H

/*
 * List of file types we know
 */
enum filetype {
	filetype_unknown,
	filetype_arm_zimage,
	filetype_lzo_compressed,
	filetype_arm_barebox,
	filetype_uimage,
	filetype_ubi,
	filetype_jffs2,
	filetype_gzip,
	filetype_bzip2,
	filetype_oftree,
	filetype_aimage,
	filetype_sh,
	filetype_mips_barebox,
	filetype_fat,
	filetype_mbr,
	filetype_bmp,
	filetype_png,
};

const char *file_type_to_string(enum filetype f);
enum filetype file_detect_type(void *_buf);
enum filetype file_name_detect_type(const char *filename);
enum filetype is_fat_or_mbr(const unsigned char *sector, unsigned long *bootsec);

#define ARM_HEAD_SIZE			0x30
#define ARM_HEAD_MAGICWORD_OFFSET	0x20
#define ARM_HEAD_SIZE_OFFSET		0x2C

#ifdef CONFIG_ARM
static inline int is_barebox_arm_head(const char *head)
{
	return !strcmp(head + ARM_HEAD_MAGICWORD_OFFSET, "barebox");
}
#else
static inline int is_barebox_arm_head(const char *head)
{
	return 0;
}
#endif

#endif /* __FILE_TYPE_H */
