
# compile options (see README.md for descriptions)
# 0 = disable
# 1 = enable

# ---- STOCK QUANSHENG FERATURES ----
ENABLE_UART                   ?= 1
ENABLE_AIRCOPY                ?= 0
ENABLE_FMRADIO                ?= 0
ENABLE_VOX                    ?= 0
ENABLE_DTMF_CALLING           ?= 0
ENABLE_FLASHLIGHT             ?= 0

# ---- CUSTOM MODS ----
ENABLE_WIDE_RX                ?= 1
ENABLE_CTCSS_TAIL_PHASE_SHIFT ?= 0
ENABLE_AM_FIX                 ?= 1
ENABLE_SQUELCH_MORE_SENSITIVE ?= 1
ENABLE_FASTER_CHANNEL_SCAN    ?= 1
ENABLE_SPECTRUM               ?= 0
ENABLE_BLMIN_TMP_OFF          ?= 0

# ---- DEBUGGING ----
ENABLE_AM_FIX_SHOW_DATA       ?= 0
ENABLE_AGC_SHOW_DATA          ?= 0
ENABLE_UART_RW_BK_REGS        ?= 0

# ---- COMPILER/LINKER OPTIONS ----
ENABLE_SWD                    ?= 0

#############################################################

# --- joaquim.org
ENABLE_MESSENGER              			?= 0
ENABLE_MESSENGER_UART					?= 0

#### Display and Keypad remote Control ####
# https://home.joaquim.org/display-explorer/
ENABLE_REMOTE_CONTROL			  		?= 1

ENABLE_UART_DEBUG			  			?= 1

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
	K5PROG = utils/k5prog/k5prog.exe -D -F -YYYYY -p /dev/com4 -b
else
	MKDIR = mkdir -p $(1)
	RM = rm -rf
	FixPath = $1
	WHERE = which
	DEL = del
	K5PROG = utils/k5prog/k5prog -D -F -YYY -p /dev/ttyUSB3 -b
endif

#------------------------------------------------------------------------------

BSP_DEFINITIONS := $(wildcard hardware/*/*.def)
BSP_HEADERS     := $(patsubst hardware/%,bsp/%,$(BSP_DEFINITIONS))
BSP_HEADERS     := $(patsubst %.def,%.h,$(BSP_HEADERS))

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

# Source files FreeRTOS

C_SRC += \
	$(EXTERNAL_LIB)/FreeRTOS/list.c \
	$(EXTERNAL_LIB)/FreeRTOS/queue.c \
	$(EXTERNAL_LIB)/FreeRTOS/tasks.c \
	$(EXTERNAL_LIB)/FreeRTOS/timers.c \
	$(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/port.c \

# Include folders external libs
IPATH += \
	$(EXTERNAL_LIB)/FreeRTOS/include/. \
	$(EXTERNAL_LIB)/FreeRTOS/portable/GCC/ARM_CM0/. \

#OLD \
	font.c \
	bitmaps.c \
	ui/main.c \
	ui/menu.c \
	app/menu.c \
	\
	app/action.c \
	app/app.c \
	app/chFrScanner.c \
	app/common.c \
	app/dtmf.c \
	app/generic.c \
	app/main.c \
	app/scanner.c \
	helper/boot.c \
	scheduler.c \
	ui/welcome.c \
	ui/inputbox.c \
	ui/battery.c \
	ui/helper.c \
	ui/scanner.c \
	ui/ui.c \	

C_SRC += \
	external/printf/printf.c \
	init.c \
	board.c \
	misc.c \
	settings.c \
	version.c \
	driver/adc.c \
	driver/backlight.c \
	driver/bk4819.c \
	driver/eeprom.c \
	driver/gpio.c \
	driver/i2c.c \
	driver/keyboard.c \
	driver/spi.c \
	driver/st7565.c \
	driver/system.c \
	driver/systick.c \
	radio/dcs.c \
	radio/frequencies.c \
	radio/functions.c \
	radio/radio.c \
	radio/audio.c \
	radio/vfo.c \
	helper/battery.c \
	main.c \

C_SRC += \
	tasks/task_main.c \
	tasks/applications_task.c \

C_SRC += \
	ui/status.c \
	gui/ui.c \
	gui/gui.c \
	gui/font_10.c \
	gui/font_small.c \
	gui/font_n_16.c \
	gui/font_n_20.c \


ifeq ($(ENABLE_UART),1)
	C_SRC += driver/aes.c
endif
ifeq ($(ENABLE_FMRADIO),1)
	C_SRC += driver/bk1080.c
endif
ifeq ($(filter $(ENABLE_AIRCOPY) $(ENABLE_UART),1),1)
	C_SRC += driver/crc.c
endif
ifeq ($(ENABLE_UART),1)
	C_SRC += driver/uart.c
endif
ifeq ($(ENABLE_AIRCOPY),1)
	C_SRC += app/aircopy.c
endif
ifeq ($(ENABLE_FLASHLIGHT),1)
	C_SRC += app/flashlight.c
endif
ifeq ($(ENABLE_FMRADIO),1)
	C_SRC += app/fm.c
endif
ifeq ($(ENABLE_SPECTRUM), 1)
	C_SRC += app/spectrum.c
endif
ifeq ($(ENABLE_UART),1)
	C_SRC += app/uart.c
endif
ifeq ($(ENABLE_MESSENGER),1)
	C_SRC += app/messenger.c
endif
ifeq ($(ENABLE_AM_FIX), 1)
	C_SRC += radio/am_fix.c
endif
ifeq ($(ENABLE_AIRCOPY),1)
	C_SRC += ui/aircopy.c
endif
ifeq ($(ENABLE_FMRADIO),1)
	C_SRC += ui/fmradio.c
endif
ifeq ($(ENABLE_MESSENGER),1)
	C_SRC += ui/messenger.c
endif

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
	external/printf/. \
	external/CMSIS_5/CMSIS/Core/Include/. \
	external/CMSIS_5/Device/ARM/ARMCM0/Include/. \


CFLAGS += -DPRINTF_INCLUDE_CONFIG_H
CFLAGS += -DAUTHOR_STRING=\"$(AUTHOR_STRING)\" -DVERSION_STRING=\"$(VERSION_STRING)\"

ifeq ($(ENABLE_UART_DEBUG),1)
	CFLAGS += -DENABLE_UART_DEBUG
endif
ifeq ($(ENABLE_REMOTE_CONTROL),1)
	CFLAGS += -DENABLE_REMOTE_CONTROL
endif
ifeq ($(ENABLE_SPECTRUM),1)
	CFLAGS += -DENABLE_SPECTRUM
endif
ifeq ($(ENABLE_SWD),1)
	CFLAGS += -DENABLE_SWD
endif
ifeq ($(ENABLE_AIRCOPY),1)
	CFLAGS += -DENABLE_AIRCOPY
endif
ifeq ($(ENABLE_FMRADIO),1)
	CFLAGS += -DENABLE_FMRADIO
endif
ifeq ($(ENABLE_UART),1)
	CFLAGS += -DENABLE_UART
endif
ifeq ($(ENABLE_VOX),1)
	CFLAGS += -DENABLE_VOX
endif
ifeq ($(ENABLE_WIDE_RX),1)
	CFLAGS += -DENABLE_WIDE_RX
endif
ifeq ($(ENABLE_CTCSS_TAIL_PHASE_SHIFT),1)
	CFLAGS += -DENABLE_CTCSS_TAIL_PHASE_SHIFT
endif
ifeq ($(ENABLE_AM_FIX),1)
	CFLAGS += -DENABLE_AM_FIX
endif
ifeq ($(ENABLE_AM_FIX_SHOW_DATA),1)
	CFLAGS += -DENABLE_AM_FIX_SHOW_DATA
endif
ifeq ($(ENABLE_SQUELCH_MORE_SENSITIVE),1)
	CFLAGS += -DENABLE_SQUELCH_MORE_SENSITIVE
endif
ifeq ($(ENABLE_BACKLIGHT_ON_RX),1)
	CFLAGS += -DENABLE_BACKLIGHT_ON_RX
endif
ifeq ($(ENABLE_REDUCE_LOW_MID_TX_POWER),1)
	CFLAGS += -DENABLE_REDUCE_LOW_MID_TX_POWER
endif
ifeq ($(ENABLE_BLMIN_TMP_OFF),1)
	CFLAGS += -DENABLE_BLMIN_TMP_OFF
endif
ifeq ($(ENABLE_DTMF_CALLING),1)
	CFLAGS += -DENABLE_DTMF_CALLING
endif
ifeq ($(ENABLE_FLASHLIGHT),1)
	CFLAGS += -DENABLE_FLASHLIGHT
endif
ifeq ($(ENABLE_UART_RW_BK_REGS),1)
	CFLAGS += -DENABLE_UART_RW_BK_REGS
endif
ifeq ($(ENABLE_UART), 0)
	ENABLE_MESSENGER_UART := 0
	ENABLE_REMOTE_CONTROL := 0
endif
ifeq ($(ENABLE_MESSENGER),1)
	CFLAGS += -DENABLE_MESSENGER
endif
ifeq ($(ENABLE_MESSENGER_UART),1)
	CFLAGS += -DENABLE_MESSENGER_UART
endif

# C flags common to all targets
CFLAGS += -Os -Wall -Werror -mcpu=cortex-m0 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -std=c2x -MMD
CFLAGS += -flto=auto
CFLAGS += -ftree-vectorize -funroll-loops
CFLAGS += -Wextra

# Assembler flags common to all targets
ASFLAGS += -mcpu=cortex-m0

# Linker flags
LDFLAGS +=
LDFLAGS += -z noexecstack -mcpu=cortex-m0 -nostartfiles -Wl,-L,linker -Wl,-T,$(LD_FILE) -Wl,--gc-sections

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

# Create objects from C SRC files
$(BUILD)/%.o: %.c
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
