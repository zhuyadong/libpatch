cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE ON)

if ( WIN32 AND NOT CYGWIN AND NOT ( CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" ) )
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FILE_OFFSET_BITS=64  -DNDEBUG -DWIN32 /MT" CACHE STRING "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_FILE_OFFSET_BITS=64 -DWIN32 /MTd /ZI" CACHE STRING "")
    include_directories(src/w32inc)
else()
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FILE_OFFSET_BITS=64" CACHE STRING "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_FILE_OFFSET_BITS=64" CACHE STRING "")
endif ()

project(libpatch)
include_directories(
    src/bzip2
    src/bsdiff
)

set(libSRC
    src/bzip2/blocksort.c
    src/bzip2/huffman.c
    src/bzip2/crctable.c
    src/bzip2/randtable.c
    src/bzip2/compress.c
    src/bzip2/decompress.c
    src/bzip2/bzlib.c
    src/bsdiff/bsdiff.c
    src/bsdiff/bspatch.c
)

if (WIN32)
    add_library(patch SHARED ${libSRC})
    add_library(libpatch STATIC ${libSRC})
else()
    add_library(libpatch STATIC ${libSRC})
endif()

if (WIN32)
    add_executable(bsdiff src/bsdiff/bsdiff.c)
    target_compile_definitions(bsdiff PUBLIC BSDIFF_EXECUTABLE)
    target_link_libraries(bsdiff libpatch)

    add_executable(bspatch src/bsdiff/bspatch.c)
    target_compile_definitions(bspatch PUBLIC BSPATCH_EXECUTABLE)
    target_link_libraries(bspatch libpatch)
endif()

install(TARGETS libpatch
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
if (WIN32)
install(TARGETS bsdiff bspatch patch
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
endif()