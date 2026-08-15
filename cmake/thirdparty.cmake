# Third-party dependencies, fetched with CPM.
# Requires cmake/get_cpm.cmake to be included first.

# Fetch SFML
if(UNIX AND NOT APPLE)
  message(STATUS
    "SFML needs system development packages. If configuration fails, install them:\n"
    "   Debian/Ubuntu: sudo apt install libxrandr-dev libxcursor-dev libxi-dev "
  )
endif()

CPMAddPackage(
  NAME SFML
  GITHUB_REPOSITORY SFML/SFML
  GIT_TAG 3.0.1
  GIT_SHALLOW ON
  SYSTEM YES
  EXCLUDE_FROM_ALL YES
  OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "SFML_BUILD_AUDIO OFF"
    "SFML_BUILD_NETWORK OFF"
)
