cmake_minimum_required(VERSION 4.0)
project(MangaReader C)

set(CMAKE_C_STANDARD 11)
# set(LIB_DIR "")

add_executable(MangaReader
        main.c

        util.h
        util.c

        socketutil.h
        socketutil.c

        http.h
        http.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(MangaReader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl.a")
target_link_libraries(MangaReader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto.a")
target_link_libraries(MangaReader PRIVATE ws2_32 crypt32)