add_executable(raspindi)

target_sources(raspindi PRIVATE
        main.cpp
        event_loop.cpp
)
target_include_directories(raspindi PRIVATE
        ../include/
        /usr/include/libcamera/
)
target_link_directories(raspindi PRIVATE
        ../lib/ndi/
        /lib/
)
target_link_libraries(raspindi PRIVATE
    ndi 
    dl 
    config++
    camera
    camera-base
    event_pthreads
    event_core
)