cmake_minimum_required(VERSION 3.14)
set(CMAKE_SYSTEM_NAME Generic)

# Project name
set(PROJECT_NAME HAPBoot)
PROJECT(${PROJECT_NAME} C ASM)

# get HEAD's hash since I don't do versioning
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_REV
		OUTPUT_STRIP_TRAILING_WHITESPACE)

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
	-DSTM32F1
	-DVECT_TAB_FLASH
	-D__HEAP_SIZE=${HEAP_SIZE}
	-D__STACK_SIZE=${STACK_SIZE}
	-DGIT_REV="${GIT_REV}"
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
# 				   Patching
#------------------------------------------------------------------------------#
# Taken from: https://github.com/facebook/hhvm/blob/master/CMakeLists.txt
# This must run before scanning for sources, otherwise new files added by
#  patches won't exist in the buildable source file list.
# I should consider using quilt probably.

# Scan for available patches
file(GLOB PATCHES "${CMAKE_SOURCE_DIR}/patches/*.patch")

# Apply patch
foreach(PATCH ${PATCHES})
	message(STATUS "Applying ${PATCH}")
	execute_process(
		COMMAND patch -p1 --forward --ignore-whitespace --binary
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		INPUT_FILE "${PATCH}"
		OUTPUT_VARIABLE OUTPUT
		RESULT_VARIABLE RESULT
	)

	if (NOT RESULT EQUAL 0)
		message(STATUS "\n${OUTPUT}")

		# Unfortunately although patch will recognise that a patch is
		#  already applied it will still return an error.
		execute_process(
			COMMAND patch -p1 -R --dry-run --binary
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
			INPUT_FILE "${PATCH}"
			OUTPUT_VARIABLE OUTPUT
			RESULT_VARIABLE RESULT2
		)

		message(STATUS "\n${OUTPUT}")

		if (RESULT2 EQUAL 0)
			message(STATUS "Patch was already applied: ${PATCH}")
		else()
			message(FATAL_ERROR "Error applying patch ${PATCH}")
		endif()
	endif()
endforeach(PATCH)

#------------------------------------------------------------------------------#
# 				Project sources
#------------------------------------------------------------------------------#

# Linker script
file(GLOB_RECURSE LINKER_SCRIPT src/stm32f103rb_flash.ld)

# Set libOpenCM3 path
set(LIBOPENCM3_DIR ${CMAKE_SOURCE_DIR}/libopencm3)

# Build libOpenCM3
add_custom_target(libopencm3 ALL make WORKING_DIRECTORY ${LIBOPENCM3_DIR})

# All source files
file(GLOB SOURCES
	"src/*.c"
	"src/*.S"

	"openblt/Target/Source/*.c"
	"openblt/Target/Source/${FAMILY}/*.c"
	"openblt/Target/Source/${FAMILY}/GCC/*.c"
)

# All header files
file(GLOB_RECURSE HEADERS
	"src/*.h"

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
include_directories(
	${INCLUDE_DIRS}
	${LIBOPENCM3_DIR}/include
)

#------------------------------------------------------------------------------#
# 			     Linker configuration
#------------------------------------------------------------------------------#

# Set output firmware name
set(EXE_NAME "${PROJECT_NAME}_${CHIP}")
string(TOLOWER ${EXE_NAME} EXE_NAME)

# Link libopenCM3
link_directories(${LIBOPENCM3_DIR}/lib)

# Create elf executable
add_executable(${EXE_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_link_libraries(${EXE_NAME}.elf opencm3_stm32f1)

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS
	-mcpu=${CPU}
	-nostartfiles
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
	COMMENT "Generating bootloader image..."

	# Build .hex and .bin files
	COMMAND ${OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
	COMMAND ${OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}

	# Display size
	COMMAND ${SIZE} -B -d ${EXE_NAME}.elf
)
