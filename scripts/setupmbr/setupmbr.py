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

def populate_globals_from_header(header_file):
    f = open(header_file, "r")
    for line in f.readlines():
        line = line.strip()
        if line.startswith("#define"):
            (variable, value) = line[len("#define"):].strip().split(" ")
            globals()[variable] = eval(value)

populate_globals_from_header("arch/x86/mach-i386/include/mach/barebox.lds.h")

class Field(object):
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
        if value >= 2**(size*8):
            raise ValueError("%u does not fit in this field" % value)
        powers = []
        while value!=0:
            powers.append( int(value % 256) )
            value /= 256
        powers.extend( self.size*[0] )
        start = instance.start_offset += self.start
        instance.array[start:start+self.size] = powers[:self.size]

class StructTemplate(object):
    def __len__(self):
        len = 0
        for attr in self.__class__.__dict__.values():
            if isinstance(attr, Field):
                len += attr.size
        return len

class DAPS(StructTemplate):
    def __init__(self, array, start_offset):
        self.array = array
        self.start_offset = start_offset

    size = Field(0, 1)
    res1 = Field(1, 1)
    count = Field(2, 1)
    res2 = Field(3, 1)
    offset = Field(4, 2)
    segment = Field(6, 2)
    lba = Field(8, 4)

class Partition(StructTemplate):
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

def fill_daps(sector, count, offset, segment, lba):
    assert count < 128
    assert offset < 0x10000
    assert segment < 0x10000

    sector.size = len(sector)
    sector.res1 = 0
    sector.count = count
    sector.res2 = 0
    sector.offset = host2target_16(offset)
    sector.segment = host2target_16(segment)
    sector.lba = host2target_64(lba)

def invalidate_daps(sector):
    sector.size = MARK_DAPS_INVALID
    sector.res1 = 0

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

    # embed barebox's boot code into the disk driver image
    hd_image[0:OFFSET_OF_PARTITION_TABLE] = barebox_image[0:OFFSET_OF_PARTITION_TABLE]

	# embed the barebox main image into the disk drive image,
	# but keep the persistant environment storage untouched
	# (if defined), e.g. store the main image behind this special area.
    hd_image_start = (pers_sector_count+1) * SECTOR_SIZE
    barebox_image_start = SECTOR_SIZE
    size = sb.st_size - SECTOR_SIZE
    hd_image[hd_image_start:hd_image_start+size] = barebox_image[barebox_image_start:barebox_image_start+size]

    embed = PATCH_AREA
    indirect = (pers_sector_count + 1) * SECTOR_SIZE

    fill_daps(DAPS(hd_image, embed), 1, INDIRECT_AREA, INDIRECT_SEGMENT, 1 + pers_sector_count)

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
