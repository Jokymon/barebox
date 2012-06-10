f = open("include/config.h", "r")
lines = f.readlines()
f.close()

lines.insert(3, "\n")
lines.insert(3, "#define CONFIG_PASSWD_SUM_SHA256\n")
lines.insert(3, "\n")
lines.insert(3, "#define TEXT_BASE           0x00007c00\n")

f = open("include/config.h", "w")
f.write("".join(lines))
f.close()

