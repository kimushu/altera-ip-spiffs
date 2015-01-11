#
# spiffs_dev_sw.tcl
#

# Create a new storage device for SPIFFS (https://github.com/pellepl/spiffs)
create_sw_package spiffs_dev

# The version of this software
set_sw_property version 1.0

# Initialize the driver in alt_sys_init()
set_sw_property auto_initialize true

# Location in generated BSP that above sources will be copied into
set_sw_property bsp_subdirectory drivers

#
# Source file listings...
#

# C/C++ source files
add_sw_property c_source HAL/src/spiffs_dev.c
add_sw_property c_source external/spiffs/src/spiffs_cache.c
add_sw_property c_source external/spiffs/src/spiffs_check.c
add_sw_property c_source external/spiffs/src/spiffs_gc.c
add_sw_property c_source external/spiffs/src/spiffs_hydrogen.c
add_sw_property c_source external/spiffs/src/spiffs_nucleus.c

# Include files
add_sw_property include_source HAL/inc/spiffs_dev.h
add_sw_property include_source HAL/inc/spiffs_config.h
add_sw_property include_source external/spiffs/src/spiffs.h
add_sw_property include_source external/spiffs/src/spiffs_nucleus.h
add_sw_property include_directory external/spiffs/src

# This driver supports HAL & UCOSII BSP (OS) types
add_sw_property supported_bsp_type HAL
add_sw_property supported_bsp_type UCOSII

# Add File-system mount point setting to the BSP:
add_sw_setting2 mountpoint quoted_string
set_sw_setting_property mountpoint default_value "/mnt/spiffs"
set_sw_setting_property mountpoint identifier SPIFFS_DEV_NAME
set_sw_setting_property mountpoint description "Mount point of file system"
set_sw_setting_property mountpoint destination system_h_define

add_sw_setting2 flash.spi_controller_base unquoted_string
set_sw_setting_property flash.spi_controller_base default_value "XXXX_BASE"
set_sw_setting_property flash.spi_controller_base identifier SPIFFS_SPIC_BASE
set_sw_setting_property flash.spi_controller_base description "Base of SPI controller"
set_sw_setting_property flash.spi_controller_base destination system_h_define

add_sw_setting2 flash.spi_slave_number decimal_number
set_sw_setting_property flash.spi_slave_number default_value 0
set_sw_setting_property flash.spi_slave_number identifier SPIFFS_SLAVE_NUMBER
set_sw_setting_property flash.spi_slave_number description "Slave number of SPI flash"
set_sw_setting_property flash.spi_slave_number destination system_h_define

add_sw_setting2 flash.start_address hex_number
set_sw_setting_property flash.start_address default_value 0x0
set_sw_setting_property flash.start_address identifier SPIFFS_START_ADDR
set_sw_setting_property flash.start_address description "Start address of flash (in bytes)"
set_sw_setting_property flash.start_address destination system_h_define

add_sw_setting2 flash.end_address hex_number
set_sw_setting_property flash.end_address default_value 0x80000
set_sw_setting_property flash.end_address identifier SPIFFS_END_ADDR
set_sw_setting_property flash.end_address description "End address of flash (in bytes)"
set_sw_setting_property flash.end_address destination system_h_define

add_sw_setting2 flash.block_size decimal_number
set_sw_setting_property flash.block_size default_value 65536
set_sw_setting_property flash.block_size identifier SPIFFS_ERASE_SIZE
set_sw_setting_property flash.block_size description "Erase block size (in bytes)"
set_sw_setting_property flash.block_size destination system_h_define

add_sw_setting2 flash.write_page_size decimal_number
set_sw_setting_property flash.write_page_size default_value 256
set_sw_setting_property flash.write_page_size identifier SPIFFS_WRITE_PAGE_SIZE
set_sw_setting_property flash.write_page_size description "Write page size (in bytes)"
set_sw_setting_property flash.write_page_size destination system_h_define

add_sw_setting2 flash.address_width decimal_number
set_sw_setting_property flash.address_width default_value 3
set_sw_setting_property flash.address_width identifier SPIFFS_ADDR_WIDTH
set_sw_setting_property flash.address_width description "Address width of flash (in bytes)"
set_sw_setting_property flash.address_width destination system_h_define

add_sw_setting2 flash.opcode_read hex_number
set_sw_setting_property flash.opcode_read default_value 0x03
set_sw_setting_property flash.opcode_read identifier SPIFFS_OPCODE_READ
set_sw_setting_property flash.opcode_read description "Read bytes opcode"
set_sw_setting_property flash.opcode_read destination system_h_define

add_sw_setting2 flash.opcode_wren hex_number
set_sw_setting_property flash.opcode_wren default_value 0x06
set_sw_setting_property flash.opcode_wren identifier SPIFFS_OPCODE_WREN
set_sw_setting_property flash.opcode_wren description "Write enable opcode"
set_sw_setting_property flash.opcode_wren destination system_h_define

add_sw_setting2 flash.opcode_write hex_number
set_sw_setting_property flash.opcode_write default_value 0x02
set_sw_setting_property flash.opcode_write identifier SPIFFS_OPCODE_WRITE
set_sw_setting_property flash.opcode_write description "Write bytes opcode"
set_sw_setting_property flash.opcode_write destination system_h_define

add_sw_setting2 flash.opcode_erase hex_number
set_sw_setting_property flash.opcode_erase default_value 0xd8
set_sw_setting_property flash.opcode_erase identifier SPIFFS_OPCODE_ERASE
set_sw_setting_property flash.opcode_erase description "Erase page opcode"
set_sw_setting_property flash.opcode_erase destination system_h_define

add_sw_setting2 flash.opcode_rdsts hex_number
set_sw_setting_property flash.opcode_rdsts default_value 0x05
set_sw_setting_property flash.opcode_rdsts identifier SPIFFS_OPCODE_RDSTS
set_sw_setting_property flash.opcode_rdsts description "Read status opcode"
set_sw_setting_property flash.opcode_rdsts destination system_h_define

add_sw_setting2 fs.logical_block_size decimal_number
set_sw_setting_property fs.logical_block_size default_value 65536
set_sw_setting_property fs.logical_block_size identifier SPIFFS_LOG_BLOCK_SIZE
set_sw_setting_property fs.logical_block_size description "Logical block size (in bytes). This must be a multiple of erase block size. Generally, larger value is good for this parameter."
set_sw_setting_property fs.logical_block_size destination system_h_define

add_sw_setting2 fs.logical_page_size decimal_number
set_sw_setting_property fs.logical_page_size default_value 256
set_sw_setting_property fs.logical_page_size identifier SPIFFS_LOG_PAGE_SIZE
set_sw_setting_property fs.logical_page_size description "Log page size (in bytes). This must be a divisor of logical block size. Generally, smaller value is good for this parameter. Use the golden rule: logical_block_size/256."
set_sw_setting_property fs.logical_page_size destination system_h_define

add_sw_setting2 fs.max_fds decimal_number
set_sw_setting_property fs.max_fds default_value 16
set_sw_setting_property fs.max_fds identifier SPIFFS_MAX_FDS
set_sw_setting_property fs.max_fds description "Maximum number of file descriptors"
set_sw_setting_property fs.max_fds destination system_h_define

# End of file
