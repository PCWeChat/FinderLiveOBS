project(obs_live.cpp)

set(obs_live_HEADERS obs_live.h)

set(obs_live_SOURCES obs_live.c)

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec avutil avformat swscale)
include_directories(${FFMPEG_INCLUDE_DIRS})

if (WIN32)
	set(MODULE_DESCRIPTION "OBS Live Output")
	configure_file(${CMAKE_SOURCE_DIR}/cmake/winrc/obs-module.rc.in obs_live.rc)
	list(APPEND obs_live_SOURCES
		obs_live.rc)
	set(obs_live_LIBS
		ws2_32
		winmm
		Iphlpapi)

	if (MINGW)
		set_source_files_properties(${obs_live_SOURCES}
			PROPERTIES COMPILE_FLAGS "-Wno-multichar")
	endif()

else()
	set(obs_live_HEADERS )
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_library(obs_live MODULE
	${obs_live_HEADERS}
	${obs_live_SOURCES})

target_link_libraries(obs_live
	libobs
	w32-pthreads
	${FFMPEG_LIBRARIES})
set_target_properties(obs_live PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(obs_live data)
