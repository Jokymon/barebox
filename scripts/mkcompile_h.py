import sys, os, os.path, datetime

(TARGET, ARCH, CC) = sys.argv[1:]

if "KBUILD_BUILD_VERSION" in os.environ.keys():
    VERSION = os.environ["KBUILD_BUILD_VERSION"]
else:
    if os.path.isfile(".version"):
        f = open(".version")
        VERSION = f.read().strip()
        f.close()
    else:
        f = open(".version", "w")
        f.write("0\n")
        f.close()
        VERSION = "0"

if "KBUILD_BUILD_TIMESTAMP" in os.environ.keys():
    TIMESTAMP = os.environ["KBUILD_BUILD_TIMESTAMP"]
else:
    d = datetime.datetime.now()
    TIMESTAMP = d.strftime("%Y-%m-%d %H:%M")

if "KBUILD_BUILD_USER" in os.environ.keys():
    BAREBOX_COMPILE_BY = os.environ["KBUILD_BUILD_USER"]
else:
    BAREBOX_COMPILE_BY = os.environ["USERNAME"]

if "KBUILD_BUILD_HOST" in os.environ.keys():
    BAREBOX_COMPILE_HOST = os.environ["KBUILD_BUILD_HOST"]
else:
    BAREBOX_COMPILE_HOST = os.environ["COMPUTERNAME"]

UTS_VERSION = "#%s" % VERSION
CONFIG_FLAGS = ""
UTS_VERSION = "%s %s %s" % (UTS_VERSION, CONFIG_FLAGS, TIMESTAMP)

UTS_LEN = 64

f = open(TARGET, "w")
f.write("/* This file is auto generated, version %s */\n" % VERSION)
if len(CONFIG_FLAGS)>0:
    f.write("/* %s */\n" % CONFIG_FLAGS)
f.write("#define UTS_MACHINE \"%s\"\n" % ARCH)
f.write("#define UTS_VERSION \"%s\"\n" % UTS_VERSION[:UTS_LEN])
f.write("#define BAREBOX_COMPILE_BY \"%s\"\n" % BAREBOX_COMPILE_BY[:UTS_LEN])
f.write("#define BAREBOX_COMPILE_HOST \"%s\"\n" % BAREBOX_COMPILE_HOST[:UTS_LEN])
#f.write("#define BAREBOX_COMPILE \"
f.close()
