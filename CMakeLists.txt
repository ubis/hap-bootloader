cmake_minimum_required(VERSION 3.14)
set(CMAKE_SYSTEM_NAME Generic)

# Project name
set(PROJECT_NAME HAPBoot)
PROJECT(${PROJECT_NAME} C ASM)

#------------------------------------------------------------------------------#
# 			    Compiler configuration
#------------------------------------------------------------------------------#

# Chip Configuration
set(ROOT_PROJ 	${CMAKE_CURRENT_SOURCE_DIR})
set(CPU 	"cortex-m3")
set(ARCH_NAME 	"arm")
set(FAMILY 	"ARMCM3_STM32F1")
set(CHIP 	"STM32F10x")
set(HEAP_SIZE 	0x0800)
set(STACK_SIZE 	0x0800)

# Toolchain path
set(TOOLCHAIN_PATH "")

SET(CMAKE_C_COMPILER 	${TOOLCHAIN_PATH}arm-none-eabi-gcc)
set(OBJCOPY 		${TOOLCHAIN_PATH}arm-none-eabi-objcopy)
set(OBJDUMP 		${TOOLCHAIN_PATH}arm-none-eabi-objdump)
set(CMAKE_ASM_COMPILER 	${TOOLCHAIN_PATH}arm-none-eabi-gcc)
set(SIZE 		${TOOLCHAIN_PATH}arm-none-eabi-size)

# Definitions
add_definitions(
	-DSTM32F10X_MD
	-DUSE_STDPERIPH_DRIVER
	-DVECT_TAB_FLASH
	-DGCC_ARMCM3
	-D__HEAP_SIZE=${HEAP_SIZE}
	-D__STACK_SIZE=${STACK_SIZE}
)

# Compilation flags
add_compile_options(
	-mcpu=${CPU}		# set target cpu
	-mthumb 		# use T32 instruction set
	-fno-strict-aliasing 	# no strict aliasing rules
	-fdata-sections	 	# place data into own section
	-ffunction-sections 	# place func into own section
	-Wall 			# show all warnings
	-Os 			# optimize for size
)

#------------------------------------------------------------------------------#
# 				Project sources
#------------------------------------------------------------------------------#

# Linker script
file(GLOB_RECURSE LINKER_SCRIPT src/stm32f103rb_flash.ld)

# All source files
file(GLOB SOURCES
	"src/*.c"
	"src/*.S"

	"lib/STM32F10x_StdPeriph_Driver/src/*.c"
	"lib/newlib/*.c"
	"lib/CMSIS/CM3/CoreSupport/*.c"
	"lib/CMSIS/CM3/DeviceSupport/ST/${CHIP}/*.c"

	"openblt/Target/Source/*.c"
	"openblt/Target/Source/${FAMILY}/*.c"
	"openblt/Target/Source/${FAMILY}/GCC/*.c"
)

# All header files
file(GLOB_RECURSE HEADERS
	"src/*.h"

	"lib/STM32F10x_StdPeriph_Driver/inc/*.h"
	"lib/STM32F10x_StdPeriph_Driver/*.h"
	"lib/CMSIS/CM3/CoreSupport/*.h"
	"lib/CMSIS/CM3/DeviceSupport/ST/${CHIP}/*.h"

	"openblt/Target/Source/*.h"
	"openblt/Target/Source/${FAMILY}/*.h"
	"openblt/Target/Source/${FAMILY}/GCC/*.h"
)

# Remove flash_layout.c from the sources to prevent compilation
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/flash_layout.c)

set (INCLUDE_DIRS "")

# Add all header folders
foreach (_headerFile ${HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list (APPEND INCLUDE_DIRS ${_dir})
endforeach()

# Remove duplicated directories
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# Include all folders
include_directories(${INCLUDE_DIRS})

#------------------------------------------------------------------------------#
# 			     Linker configuration
#------------------------------------------------------------------------------#

# Set output firmware name
set(EXE_NAME "${PROJECT_NAME}_${CHIP}")
string(TOLOWER ${EXE_NAME} EXE_NAME)

# Create elf executable
add_executable(${EXE_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS
	-mcpu=${CPU}
	-Wl,--defsym=__HEAP_SIZE=${HEAP_SIZE}
	-Wl,--defsym=__STACK_SIZE=${STACK_SIZE}
	-Wl,-script=${LINKER_SCRIPT}
	-specs=nano.specs
	-Wl,--gc-sections
)

# ';' is appended in linker flags so replace with empty space
string(REPLACE ";" " " CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

#------------------------------------------------------------------------------#
# 			     Build configuration
#------------------------------------------------------------------------------#

# Set output file names
set(ELF_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.elf)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.bin)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.hex)

# Set build command
add_custom_command(TARGET "${EXE_NAME}.elf" POST_BUILD
	COMMENT "Building bootloader..."

	# Build .hex and .bin files
	COMMAND ${OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
	COMMAND ${OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}

	# Display size
	COMMAND ${SIZE} -B -d ${EXE_NAME}.elf
)
