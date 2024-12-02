# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Fraia/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/tmp"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/src"
  "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Fraia/Desktop/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores---Fraian-liranzo-2022-0045/Microcontroladores/Puerta_1/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
