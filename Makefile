
# compile options (see README.md for descriptions)
# 0 = disable
# 1 = enable

ENABLE_REMOTE_CONTROL			?= 1
ENABLE_UART_DEBUG			  	?= 1

#------------------------------------------------------------------------------
AUTHOR_STRING ?= JOAQUIM
VERSION_STRING ?= V1.0.0
PROJECT_NAME := cfw_joaquimorg_oefw_V1.0.0

BUILD := _build
BIN := firmware

LD_FILE := firmware.ld

EXTERNAL_LIB := external

#------------------------------------------------------------------------------
# Tool Configure
#------------------------------------------------------------------------------

PYTHON = python

# Toolchain commands
# Should be added to your PATH
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

# Set make directory command, Windows tries to create a directory named "-p" if that flag is there.
ifeq ($(OS), Windows_NT) # windows
	MKDIR = mkdir $(subst /,\,$(1)) > nul 2>&1 || (exit 0)
	RM = rmdir /s /q
	FixPath = $(subst /,\,$1)
	WHERE = where
	DEL = del /q
	K5PROG = utils/k5prog/k5prog.exe -D -F -YYYYY -p /dev/$(COMPORT) -b
else
	MKDIR = mkdir -p $(1)
	RM = rm -rf
	FixPath = $1
	WHERE = which
	DEL = del
	K5PROG = utils/k5prog/k5prog -D -F -YYY -p /dev/$(COMPORT) -b
endif

#------------------------------------------------------------------------------

#BSP_DEFINITIONS := $(wildcard hardware/*/*.def)
#BSP_HEADERS     := $(patsubst hardware/%,bsp/%,$(BSP_DEFINITIONS))
#BSP_HEADERS     := $(patsubst %.def,%.h,$(BSP_HEADERS))

# Source files common to all targets
ASM_SRC += \
	start.S \

# Include folders external libs
IPATH += \
	$(EXTERNAL_LIB)/. \
	$(EXTERNAL_LIB)/printf/. \
	$(EXTERNAL_LIB)/CMSIS_5/CMSIS/Core/Include/. \
	$(EXTERNAL_LIB)/CMSIS_5/Device/ARM/ARMCM0/Include/. \

CFLAGS += -DARMCM0

# Source files external libs

C_SRC += \
	$(EXTERNAL_LIB)/FreeRTOS/list.c \
	$(EXTERNAL_LIB)/FreeRTOS/queue.c \
	$(EXTERNAL_LIB)/FreeRTOS/tasks.c \
	$(EXTERNAL_LIB)/FreeRTOS/timers.c \
	$(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/port.c \
	$(EXTERNAL_LIB)/printf/printf.c \

# Include folders external libs
IPATH += \
	$(EXTERNAL_LIB)/FreeRTOS/include/. \
	$(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/. \
	$(EXTERNAL_LIB)/printf/. \
	$(EXTERNAL_LIB)/CMSIS_5/CMSIS/Core/Include/. \
	$(EXTERNAL_LIB)/CMSIS_5/Device/ARM/ARMCM0/Include/. \

# driver
C_SRC += $(wildcard driver/*.c)

# radio
C_SRC += $(wildcard radio/*.c)

# tasks
C_SRC += $(wildcard tasks/*.c)

# gui
C_SRC += $(wildcard gui/*.c)

# other
C_SRC += \
	init.c \
	board.c \
	misc.c \
	settings.c \
	version.c \
	helper/battery.c \
	app/uart.c \
	main.c \
	ui/status.c \
	


# Include folders common to all targets
IPATH += \
	. \
	ui/. \
	gui/. \
	helper/. \
	app/. \
	apps/. \
	tasks/. \
	driver/. \
	radio/. \
	bsp/dp32g030/. \


CFLAGS += -DPRINTF_INCLUDE_CONFIG_H
CFLAGS += -DAUTHOR_STRING=\"$(AUTHOR_STRING)\" -DVERSION_STRING=\"$(VERSION_STRING)\"

ifeq ($(ENABLE_UART_DEBUG),1)
	CFLAGS += -DENABLE_UART_DEBUG
endif
ifeq ($(ENABLE_REMOTE_CONTROL),1)
	CFLAGS += -DENABLE_REMOTE_CONTROL
endif

# C flags common optimized for size
CFLAGS += -Os -Wall -Werror -mcpu=cortex-m0 -g -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -std=c2x -MMD
CFLAGS += -flto
CFLAGS += -ftree-vectorize -funroll-loops
CFLAGS += -Wextra -Wunused-parameter -Wconversion
CFLAGS += -fno-math-errno -pipe -ffunction-sections -fdata-sections -ffast-math
CFLAGS += -fsingle-precision-constant -finline-functions-called-once

# Assembler flags common to all targets
ASFLAGS += -mcpu=cortex-m0

# Linker flags
LDFLAGS += -z noseparate-code -z noexecstack -mcpu=cortex-m0 -nostartfiles -Wl,-L,linker -Wl,-T,$(LD_FILE) -Wl,--gc-sections
LDFLAGS += -Wl,--build-id=none

# Use newlib-nano instead of newlib
LDFLAGS += --specs=nosys.specs --specs=nano.specs

#show size
LDFLAGS += -Wl,--print-memory-usage

LIBS =

C_OBJECTS = $(addprefix $(BUILD)/, $(C_SRC:.c=.o) )
ASM_OBJECTS = $(addprefix $(BUILD)/, $(ASM_SRC:.S=.o) )


OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

INC_PATHS = $(addprefix -I,$(IPATH))

ifneq (, $(shell $(WHERE) python))
	MY_PYTHON := python
else ifneq (, $(shell $(WHERE) python3))
	MY_PYTHON := python3
endif

ifdef MY_PYTHON
	HAS_CRCMOD := $(shell $(MY_PYTHON) -c "import crcmod" 2>&1)
endif

ifndef MY_PYTHON
$(info )
$(info !!!!!!!! PYTHON NOT FOUND, *.PACKED.BIN WON'T BE BUILT)
$(info )
else ifneq (,$(HAS_CRCMOD))
$(info )
$(info !!!!!!!! CRCMOD NOT INSTALLED, *.PACKED.BIN WON'T BE BUILT)
$(info !!!!!!!! run: pip install crcmod)
$(info )
endif

.PHONY: all clean clean-all prog

# Default target - first one defined
all: $(BUILD) $(BUILD)/$(PROJECT_NAME).out $(BIN)

# Print out the value of a make variable.
# https://stackoverflow.com/questions/16467718/how-to-print-out-a-variable-in-makefile
print-%:
	@echo $* = $($*)

#------------------- Compile rules -------------------

# Create build directories
$(BUILD):
	@$(call MKDIR,$@)

$(BIN):
	@$(call MKDIR,$@)

clean:
	@-$(RM) $(call FixPath,$(BUILD))

clean-all:
	@-$(RM) $(call FixPath,$(BUILD))
	@-$(DEL) $(call FixPath,$(BIN)/*)

-include $(OBJECTS:.o=.d)

#bsp/dp32g030/%.h: hardware/dp32g030/%.def

# Create objects from C SRC files
$(BUILD)/%.o: %.c # | $(BSP_HEADERS)
	@echo CC $<
	@$(call MKDIR, $(@D))
	@$(CC) $(CFLAGS) $(INC_PATHS) -c $< -o $@


# Assemble files
$(BUILD)/%.o: %.S
	@echo AS $<
	@$(call MKDIR,$(@D))
	@$(CC) -x assembler-with-cpp $(ASFLAGS) $(INC_PATHS) -c $< -o $@

# Link
$(BUILD)/$(PROJECT_NAME).out: $(OBJECTS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)
#	@$(SIZE) $@
#------------------- Binary generator -------------------	
	@echo Create $(notdir $@)
	@$(OBJCOPY) -O binary $(BUILD)/$(PROJECT_NAME).out $(BIN)/$(PROJECT_NAME).bin
	@echo Create $(PROJECT_NAME).packed.bin
	@-$(MY_PYTHON) utils/fw-pack.py $(BIN)/$(PROJECT_NAME).bin $(AUTHOR_STRING) $(VERSION_STRING) $(BIN)/$(PROJECT_NAME).packed.bin


prog: all
	$(K5PROG) $(BIN)/$(PROJECT_NAME).bin
