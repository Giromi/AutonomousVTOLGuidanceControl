# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-src"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-build"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/tmp"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/src/googletest-stamp"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/src"
  "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/src/googletest-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/src/googletest-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-download/googletest-prefix/src/googletest-stamp${cfgdir}") # cfgdir has leading slash
endif()
