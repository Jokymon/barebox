#define CONFIG_MALLOC_SIZE  0x400000
#define CONFIG_STACK_SIZE   0x7000
#define CONFIG_CBSIZE       1024
#define CONFIG_MAXARGS      16
#define CONFIG_BAREBOX_MAX_BARE_INIT_SIZE 0xffffffff

#define CONFIG_PROMPT       "minibox:"
#define CONFIG_PROMPT_HUSH_PS2 "> "

#define CONFIG_BOARDINFO "sandbox"

#define CONFIG_X86_HDBOOT 1

#define CONFIG_MODULES 0
#define __enabled_CONFIG_MODULES 0
#define __enabled_CONFIG_MODULES_MODULE 0

#define CONFIG_ENVIRONMENT_VARIABLES
#define __enabled_CONFIG_ENVIRONMENT_VARIABLES 1
#define __enabled_CONFIG_ENVIRONMENT_VARIABLES_MODULE 0

#define CONFIG_BANNER 1
#define __enabled_CONFIG_BANNER 1
#define __enabled_CONFIG_BANNER_MODULE 0

#define CONFIG_FS_TFTP 1
#define __enabled_CONFIG_FS_TFTP 1
#define __enabled_CONFIG_FS_TFTP_MODULE 0

#define CONFIG_PARAMETER 1
#define __enabled_CONFIG_PARAMETER 1
#define __enabled_CONFIG_PARAMETER_MODULE 0
