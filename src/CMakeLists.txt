add_executable(raspindi)

target_sources(raspindi PRIVATE
        main.cpp
        # event_loop.cpp
)
target_include_directories(raspindi PRIVATE
        ../include/
        /usr/include/libcamera/
)
target_link_directories(raspindi PRIVATE
        ../lib/ndi/
        ../lib/
)
target_link_libraries(raspindi PRIVATE
#     ndi 
#     dl 
    config++
    encoders
    outputs
    ndioutput
    camera_app
    boost_program_options
)

add_library(ndioutput ndi_output.cpp)

target_include_directories(ndioutput PRIVATE
        ../include/
        /usr/include/libcamera/
)
target_link_directories(ndioutput PRIVATE
        ../lib/ndi/
)
target_link_libraries(raspindi PRIVATE
    ndi
    camera
    camera-base
    event_pthreads
    event_core
)