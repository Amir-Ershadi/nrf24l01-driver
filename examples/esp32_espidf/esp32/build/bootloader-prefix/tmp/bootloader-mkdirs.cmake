# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/tmp"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/src"
  "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/amire/OneDrive/Desktop/mesh_pro/NRF_Driver/project/esp32/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
