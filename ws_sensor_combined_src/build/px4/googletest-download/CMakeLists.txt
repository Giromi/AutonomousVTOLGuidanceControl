cmake_minimum_required(VERSION 2.8.4)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
	URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip # 1.13
	SOURCE_DIR "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-src"
	BINARY_DIR "/home/giromi/Repos/VTOL/ws_sensor_combined_src/build/px4/googletest-build"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	TEST_COMMAND ""
	# Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
