cmake_minimum_required(VERSION 3.20)
project(BattlefieldCpp VERSION 1.1.0 LANGUAGES CXX)

# Group the projects nicely in the IDE solution explorer.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Everything (both DLLs and the runner) lands in <build>/bin/<config> so Windows finds the DLLs next to the exe.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BATTLEFIELD_BUILD_TESTS "Build BattlefieldTests.dll and the console runner" ON)

add_subdirectory(Battlefield)

if(BATTLEFIELD_BUILD_TESTS)
  enable_testing()
  add_subdirectory(BattlefieldTests)
  # F5 in Visual Studio / Rider runs the tests under the debugger.
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT BattlefieldTestRunner)
endif()
