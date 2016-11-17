###
# Purpose: to create a bare-metal project with mbed SDK.

###
# GNU ARM Embedded Toolchain
CC   = arm-none-eabi-gcc
LD   = arm-none-eabi-ld
AS   = arm-none-eabi-as
CP   = arm-none-eabi-objcopy
OD   = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

###
# Directory Structure
OBJDIR = obj
BINDIR = bin
INCDIR = inc
SRCDIR = src

###
# Standard Peripheral Library path
STM_DIR = ../STM32F4xx_DSP_StdPeriph_Lib_V*

# This is where the source files are located,
STM_SRC = $(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/src
# DSP Libraries
DSP_DIR = $(STM_DIR)/Libraries/CMSIS/DSP_Lib/Source/

# Tell make to look in that folder if it cannot find a source
# in the current directory
vpath %.c $(STM_SRC)
vpath %.c $(DSP_DIR)
vpath %.c $(SRCDIR)
vpath %.s $(SRCDIR)

# Find source files
ASOURCES      = $(notdir $(shell find -L $(SRCDIR) -name '*.s'))
CSOURCES      = $(notdir $(shell find -L $(SRCDIR) -name '*.c'))

##########################################
#           SELECT C FILE INCLUDES       #
##########################################
# These source files implement the functions we use.
# make finds them by searching the vpath defined above.
DSP_SOURCES   = FastMathFunctions/arm_sin_f32.c FastMathFunctions/arm_cos_f32.c
DSP_SOURCES  += CommonTables/arm_common_tables.c CommonTables/arm_const_structs.c
DSP_SOURCES  += BasicMathFunctions/arm_mult_f32.c
DSP_SOURCES  += BasicMathFunctions/arm_scale_f32.c
# DSP_SOURCES  += BasicMathFunctions/arm_mult_q7.c BasicMathFunctions/arm_mult_q15.c BasicMathFunctions/arm_mult_q31.c
# DSP_SOURCES  += BasicMathFunctions/arm_scale_f32.c BasicMathFunctions/arm_shift_q7.c BasicMathFunctions/arm_shift_q15.c BasicMathFunctions/arm_shift_q31.c
# DSP_SOURCES  += TransformFunctions/*.c
#DSP_SOURCES  += ComplexMathFunctions/arm_cmplx_mag_squared_f32.c ComplexMathFunctions/arm_cmplx_mult_cmplx_q*.c
#DSP_SOURCES  += ComplexMathFunctions/arm_cmplx_mult_cmplx_f32.c
DSP_SOURCES  += FilteringFunctions/arm_fir_f32.c FilteringFunctions/arm_fir_init_f32.c
DSP_SOURCES  += FilteringFunctions/arm_biquad_cascade_df2T_f32.c FilteringFunctions/arm_biquad_cascade_df2T_init_f32.c
DSP_SOURCES  += SupportFunctions/arm_copy_f32.c

CSOURCES     += $(DSP_SOURCES)

# Find header directories
INC           = $(shell find -L $(INCDIR) -name '*.h' -exec dirname {} \; | uniq)
INC          += $(STM_DIR)/Libraries/CMSIS/Include
INC          += $(STM_DIR)/Libraries/CMSIS/ST/STM32F4xx/Include
INC          += $(STM_DIR)/Libraries/STM32F4xx_StdPeriph_Driver/inc
INCLUDES      = $(INC:%=-I%)
# INCLUDES     += -lm

# Find libraries
INCLUDES_LIBS =
LINK_LIBS     =

# Create object list
OBJECTS       = $(ASOURCES:%.s=$(OBJDIR)/%.o)
OBJECTS      += $(CSOURCES:%.c=$(OBJDIR)/%.o)

# Extract names for object files - $(OBJDIR)/*.o

# Define output files ELF & BIN
BINELF        = main.elf
BIN           = main.bin

###
# MCU FLAGS
MCFLAGS  = -std=c11 -mcpu=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork -g -Wall

##########################################
#               COMPILE FLAGS            #
##########################################
DEFS     = -DSTM32F40_41xxx
# DSP STUFF
DEFS    += -DARM_MATH_CM4 -D__FPU_PRESENT=1
CFLAGS   = -c $(MCFLAGS) $(DEFS) $(INCLUDES)

# LINKER FLAGS
LDSCRIPT = STM32F407VG_FLASH.ld
LDFLAGS  = -T $(LDSCRIPT) $(MCFLAGS) --specs=nosys.specs $(INCLUDES_LIBS) $(LINK_LIBS)

# Include math library
LDFLAGS += -lm

###
# Build Rules
.PHONY: all release debug clean install

all: release

debug: DEFS    += -DUSE_FULL_ASSERT
debug: MCFLAGS += -O0
debug: CFLAGS  += -g
debug: LDFLAGS += -g
debug: $(BINDIR)/$(BIN)
	# Create appropriate directories organized into subdirectories
	mkdir -p obj/$(dir $(DSP_SOURCES))

	@echo "\033[1m   Start Debug session \033[0m"
	# osascript -e 'tell application "Terminal" to do script "st-util"'
	# xfce4-terminal --command="sudo st-util"
	urxvt -e sudo st-util &
	sleep 1s
	@echo "\033[1m   Run GDB --TUI, load build/main.elf, and connect to Debug session \033[0m"
	# osascript -e "tell application \"Terminal\" to do script \"arm-none-eabi-gdb --tui --eval-command='file `pwd`/$(BINDIR)/$(BINELF)' --eval-command='tar rem localhost:4242'\""
	# xfce4-terminal --command="arm-none-eabi-gdb --tui --eval-command='file `pwd`/$(BINDIR)/$(BINELF)' --eval-command='tar rem localhost:4242'"
	urxvt -e arm-none-eabi-gdb --tui --eval-command='file `pwd`/$(BINDIR)/$(BINELF)' --eval-command='tar rem localhost:4242' &

release: MCFLAGS += -O3
release: $(BINDIR)/$(BIN)

$(BINDIR)/$(BIN): $(BINDIR)/$(BINELF)
	$(CP) -O binary $< $@
	@echo -e "\033[1m   Converted "$<" to "$@"! \033[0m \n"

$(BINDIR)/$(BINELF): $(OBJECTS)
	# Create Binary directory
	mkdir -p $(BINDIR)

	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo -e "\033[1m   Linked! \033[0m \n"
	$(SIZE) $(BINDIR)/$(BINELF)

$(OBJDIR)/%.o: %.c
	# Create appropriate directories organized into subdirectories
	mkdir -p $(dir $@)

	$(CC) $(CFLAGS) $< -o $@
	@echo -e "\033[1m   Compiled "$<"! \033[0m \n"

$(OBJDIR)/%.o: %.s
	# Create appropriate directories organized into subdirectories
	mkdir -p $(dir $@)

	$(CC) $(CFLAGS) $< -o $@
	@echo -e "\033[1m   Assembled "$<"! \033[0m \n"

clean:
	rm -f $(OBJECTS) $(BINDIR)/$(BINELF) $(BINDIR)/$(BIN)

install: release
	@echo -e "\033[1m   Installing program at 0x08000000... \033[0m"
	sudo st-flash write $(BINDIR)/$(BIN) 0x08000000
