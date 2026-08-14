add_library(CDLib INTERFACE)

target_include_directories(CDLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CDLib INTERFACE SFML::Graphics)
