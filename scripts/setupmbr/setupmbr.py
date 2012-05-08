#minibox = open("minibox.img", "rb")
#data = minibox.read()
#minibox.close()
#
#import os
#c50img = open("c50m.img", "rb+")
#c50img.seek(0, os.SEEK_SET)
#c50img.write( data )
#c50img.close()


SECTOR_SIZE = 512
OFFSET_OF_SIGNATURE = 510
OFFSET_OF_PARTITION_TABLE = 446

BOOT_INDICATOR_OFFSET  = 0
CHS_BEGIN_OFFSET       = 1
TYPE_OFFSET            = 4
CHS_END_OFFSET         = 5
PARTITION_START_OFFSET = 8
PARTITION_SIZE_OFFSET  = 12

class Field:
    def __init__(self, start, size):
        self.start = start
        self.size = size

    def __get__(self, instance, owner):
        start = instance.start_offset + self.start
        values = instance.array[start:start+self.size]
        powers = range(len(values))
        powers = map(lambda x: 256**x, powers)
        summands = map(int.__mul__, values, powers)
        return sum(summands)

    def __set__(self, instance, value):
        print("Setting not yet implemented")

class Partition:
    def __init__(self, array, start_offset):
        self.array = array
        self.start_offset = start_offset

    boot_indicator = Field(0, 1)
    chs_begin = Field(1, 3)
    type = Field(4, 1)
    chs_end = Field(5, 3)
    partition_start = Field(8, 4)
    partition_size  = Field(12, 4)

def target2host_32(value):
    return value

class SetupMbrError:
    pass

def barebox_linear_image(daps_table, size, pers_sector_count):
    pass

def check_for_valid_mbr(sector, size):
    if size < SECTOR_SIZE:
        print("MBR too small to be valid")
        return False
    if (sector[OFFSET_OF_SIGNATURE] != 0x55) or \
       (sector[OFFSET_OF_SIGNATURE+1] != 0xAA):
        print("No MBR signature found")
        return False
    return True

def check_for_space(hd_image, size):
    if not check_for_valid_mbr(hd_image, size):
        return False

    partition = Partition(hd_image, OFFSET_OF_PARTITION_TABLE)

    spare_sector_count = target2host_32( partition.partition_start )

    print("Debug: Required free sectors for barebox prior first partition: %u, hd image provides: %u" % (
           (size + SECTOR_SIZE - 1) / SECTOR_SIZE,
           spare_sector_count))

    spare_sector_count *= SECTOR_SIZE
    if spare_sector_count < size:
        print("Not enough space after MBR to store minibox")
        print("Move begin of the first partition beyond sector %u" % ((size + SECTOR_SIZE - 1) / SECTOR_SIZE))
        return False
    return True

def store_pers_env_info(patch_area, pers_sector_start, pers_sector_count):
    pass

def barebox_overlay_mbr(fd_barebox, fd_hd, pers_sector_count):
    import mmap, os
    sb = os.fstat(fd_barebox.fileno())
    barebox_image = mmap.mmap(fd_barebox.fileno(), 0, access=mmap.ACCESS_READ)

    check_for_valid_mbr(barebox_image, sb.st_size)

    required_size = sb.st_size + pers_sector_count * SECTOR_SIZE
    hd_image = mmap.mmap(fd_hd.fileno(), required_size, access=mmap.ACCESS_WRITE)

    check_for_space(hd_image, required_size)

    hd_image.close()
    barebox_image.close()

def main():
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-s", dest="barebox_pers_size", type="int",
                      help="sector count of the persistant environment storage")
    parser.add_option("-m", dest="barebox_image_filename",
                      help="")
    parser.add_option("-d", dest="hd_image_filename",
                      help="")
    #parser.add_option("-v", 

    (options, args) = parser.parse_args()

    fd_barebox_image = open(options.barebox_image_filename, "r+b")
    fd_hd_image = open(options.hd_image_filename, "a+b")

    barebox_overlay_mbr(fd_barebox_image, fd_hd_image, options.barebox_pers_size)

    fd_barebox_image.close()
    fd_hd_image.close()

if __name__=="__main__":
    main()
