add_executable(raspindi)

target_sources(raspindi PRIVATE
        main.cpp
)
target_include_directories(raspindi PRIVATE
        ../include/
)
target_link_directories(raspindi PRIVATE
        ../lib/ndi/
)
target_link_libraries(raspindi PRIVATE
    ndi 
    dl 
    config++ 
    mmal_core 
    mmal 
    mmal_util
)
