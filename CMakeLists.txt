cmake_minimum_required(VERSION 3.14)
SET(CMAKE_SYSTEM_NAME Generic)

# Project name
set(PROJECT_NAME HAPBoot)
PROJECT(${PROJECT_NAME} C ASM)

###################### CHIP CONFIGURATION ##########################
SET(ROOT_PROJ   ${CMAKE_CURRENT_SOURCE_DIR})
SET(CPU         "cortex-m3")
SET(ARCH_NAME   "arm")
SET(FAMILY      "stm32f3")
SET(CHIP        "STM32F303xC")
####################################################################

# Toolchain path
set(TOOLCHAIN_PATH  "")

SET(CMAKE_C_COMPILER    ${TOOLCHAIN_PATH}arm-none-eabi-gcc)
set(OBJCOPY             ${TOOLCHAIN_PATH}arm-none-eabi-objcopy)
set(OBJDUMP             ${TOOLCHAIN_PATH}arm-none-eabi-objdump)
set(CMAKE_ASM_COMPILER 	${TOOLCHAIN_PATH}arm-none-eabi-gcc)
set(SIZE                ${TOOLCHAIN_PATH}arm-none-eabi-size)

# Definitions passed at compile time (#defines)
add_definitions(-DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DVECT_TAB_FLASH -DGCC_ARMCM3)
add_definitions(-D__HEAP_SIZE=0x0800 -D__STACK_SIZE=0x0800)

# Compilation flags
add_compile_options(-mcpu=${CPU})
add_compile_options(-mthumb)
add_compile_options(-fno-strict-aliasing -Wno-maybe-uninitialized)
add_compile_options(-fdata-sections -ffunction-sections -Wall -g3)
add_compile_options(-Og)
# Only for debugging

# Linker script path
file(GLOB_RECURSE LINKER_SCRIPT src/stm32f103rb_flash.ld)

################################## Source code ###############################################################
# Retrieve all sources # "platforms/${FAMILY}/Startup/*.s"
file(GLOB SOURCES
	"src/*.c"
	"src/*.S"
	"openblt/Target/Source/*.c"
	"openblt/Target/Source/ARMCM3_STM32F1/*.c"
	"openblt/Target/Source/ARMCM3_STM32F1/GCC/*.c"
	"lib/STM32F10x_StdPeriph_Driver/src/*.c"
	"lib/newlib/*.c"
	"lib/CMSIS/CM3/CoreSupport/*.c"
	"lib/CMSIS/CM3/DeviceSupport/ST/STM32F10x/*.c")
#Retrieve all locations of headers
file(GLOB_RECURSE HEADERS
	"src/*.h"
	"openblt/Target/Source/*.h"
	"openblt/Target/Source/ARMCM3_STM32F1/*.h"
	"openblt/Target/Source/ARMCM3_STM32F1/GCC/*.h"
	"lib/STM32F10x_StdPeriph_Driver/inc/*.h"
	"lib/STM32F10x_StdPeriph_Driver/*.h"
	"lib/CMSIS/CM3/CoreSupport/*.h"
	"lib/CMSIS/CM3/DeviceSupport/ST/STM32F10x/*.h")
set (INCLUDE_DIRS "")
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/flash_layout.c)
foreach (_headerFile ${HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)
include_directories(${INCLUDE_DIRS})
#link_directories(${ARM_LIB})
################################## Source code END ###########################################################

set(EXE_NAME "${PROJECT_NAME}_${CHIP}")
add_executable(${EXE_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})
get_directory_property(defs COMPILE_OPTIONS)
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${CPU} -mthumb -fno-strict-aliasing -Wno-maybe-uninitialized -fdata-sections -ffunction-sections -Wall -g3 -Og -Wl,--defsym=__HEAP_SIZE=0x0800 -Wl,--defsym=__STACK_SIZE=0x0800 -Wl,-script=${LINKER_SCRIPT} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map -specs=nano.specs -Wl,--gc-sections")

# Libs and external dependencies
target_link_libraries(${EXE_NAME}.elf)

# Outputs
set(ELF_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.elf)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${EXE_NAME}.bin)

add_custom_command(TARGET "${EXE_NAME}.elf" POST_BUILD
        # Build .hex and .bin files
        COMMAND ${OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
        COMMAND ${OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
        COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"

        # Display sizes
        COMMAND ${SIZE} -B -d ${EXE_NAME}.elf
        COMMENT "Invoking: Cross ARM GNU Print Size"
)
