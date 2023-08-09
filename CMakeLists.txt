add_library(eeprom INTERFACE)

target_sources(eeprom INTERFACE
 ${CMAKE_CURRENT_LIST_DIR}/eeprom_24LC16B.c
 ${CMAKE_CURRENT_LIST_DIR}/eeprom_24AAxxx.c
)

target_include_directories(eeprom INTERFACE ${CMAKE_CURRENT_LIST_DIR})
