# Build Options
#   change yes to no to disable
#
# Board: it should exist either in <chibios>/os/hal/boards/
#  or <this_dir>/boards
BOARD = FS026
EEPROM_CUSTOM = custom
NO_USB_STARTUP_CHECK = yes
BLUETOOTH_CUSTOM = yes

ENCODER_MAP_ENABLE = yes    # Enable ENCODER MAP
DEBOUNCE_TYPE = sym_defer_g

SRC += $(LIB_PATH)/rdr_lib/rdr_common.c
