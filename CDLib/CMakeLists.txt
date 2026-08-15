add_library(CDLib STATIC 
    CDLib.cpp
    private/CommandManager.cpp
    private/WindowManager.cpp
)

target_include_directories(CDLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(CDLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(CDLib PUBLIC SFML::Graphics)
