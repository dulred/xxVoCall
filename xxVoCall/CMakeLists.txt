cmake_minimum_required(VERSION 3.29.3)

include_directories(D:/dulred/dev/3rdlibs/done/OpenAL1.1/include)

link_directories(D:/dulred/dev/3rdlibs/done/OpenAL1.1/libs/Win64)

add_library(xxVoCall xxVoCall.h xxVoCall.cpp)

target_link_libraries(xxVoCall OpenAL32)
