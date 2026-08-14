add_executable(app main.cpp)

if(MSVC)
  # ASan on MSVC is incompatible with /RTC1 from the default Debug flags
  string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

  set(app_flags /W4)
  set(app_debug_flags /Od /fsanitize=address)
  set(app_link_flags "")
  set(app_debug_link_flags /INCREMENTAL:NO)
  set(app_release_flags /O2 /fp:fast)
elseif(MINGW)
  # MinGW ships without the sanitizer runtimes
  set(app_flags -Wall -Wextra)
  set(app_debug_flags -O0 -g)
  set(app_link_flags -static)
  set(app_debug_link_flags "")
  set(app_release_flags -O3 -ffast-math)
else()
  set(app_flags -Wall -Wextra)
  set(app_debug_flags -O0 -g -fsanitize=address,undefined)
  set(app_link_flags "")
  set(app_debug_link_flags -fsanitize=address,undefined)
  set(app_release_flags -O3 -ffast-math)
endif()

target_compile_options(app PRIVATE
  ${app_flags}
  $<$<CONFIG:Debug>:${app_debug_flags}>
  $<$<CONFIG:Release>:${app_release_flags}>
)

target_link_options(app PRIVATE
  ${app_link_flags}
  $<$<CONFIG:Debug>:${app_debug_link_flags}>
)

target_link_libraries(app PRIVATE CDLib)
