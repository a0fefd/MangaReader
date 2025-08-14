cmake_minimum_required(VERSION 4.0)
project(MangaReader C)

set(CMAKE_C_STANDARD 11)

add_executable(MangaReader
        main.c

        util.h
        util.c

        socketutil.h
        socketutil.c

        http.h
        http.c
)

include_directories("C:/Users/nbigi0/.vcpkg-clion/vcpkg/installed/x64-mingw-static/include/")

target_link_libraries(MangaReader PRIVATE "C:/Users/nbigi0/.vcpkg-clion/vcpkg/installed/x64-mingw-static/lib/libssl.a")
target_link_libraries(MangaReader PRIVATE "C:/Users/nbigi0/.vcpkg-clion/vcpkg/installed/x64-mingw-static/lib/libcrypto.a")
target_link_libraries(MangaReader PRIVATE ws2_32 crypt32)