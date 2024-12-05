cmake_minimum_required(VERSION 3.29.3)
include_directories("..")
include_directories(D:/dulred/dev/3rdlibs/done/OpenAL1.1/include)
include_directories(D:/dulred/dev/3rdlibs/done/boost_1_86_0/include/boost-1_86)
include_directories(D:/dulred/dev/3rdlibs/done/ffmpeg-n4.4.2-95-ga8f16d4eb4-win64-gpl-shared-4.4/include)


link_directories(D:/dulred/dev/3rdlibs/done/OpenAL1.1/libs/Win64)
link_directories(D:/dulred/dev/3rdlibs/done/boost_1_86_0/lib)
link_directories(D:/dulred/dev/3rdlibs/done/ffmpeg-n4.4.2-95-ga8f16d4eb4-win64-gpl-shared-4.4/lib)

add_executable(xxVoCallTest main.cpp)

add_executable(play play.cpp)
add_executable(collect collect.cpp)
add_executable(audio_mix audio_mix.cpp)

target_link_libraries(xxVoCallTest xxVoCall OpenAL32)

target_link_libraries(collect OpenAL32)
target_link_libraries(play xxVoCall OpenAL32 boost_locale-mgw11-mt-sd-x64-1_86 avcodec avdevice avformat avutil swresample swscale)

