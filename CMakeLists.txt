cmake_minimum_required(VERSION 4.0)
project(MangaReader C)

set(CMAKE_C_STANDARD 11)
set(LOCAL_INCL_DIR "headers")

add_executable(MangaReader
        main.c

        ${LOCAL_INCL_DIR}/util.h
        util.c

        ${LOCAL_INCL_DIR}/socketutil.h
        socketutil.c

        ${LOCAL_INCL_DIR}/http.h
        http.c
)

include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/" 
        "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_INCL_DIR}/"
)

target_link_libraries(MangaReader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl.a")
target_link_libraries(MangaReader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto.a")
target_link_libraries(MangaReader PRIVATE ws2_32 crypt32)