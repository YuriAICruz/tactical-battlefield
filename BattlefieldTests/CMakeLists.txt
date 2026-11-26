# Project 2: BattlefieldTests.dll - the transpiled automation tests, plus a tiny console host.

add_library(BattlefieldTests SHARED
  Private/Framework/TestExports.cpp
  Private/Tests/ActorTests.cpp
  Private/Tests/BattlefieldControllerTests.cpp
  Private/Tests/BattlefieldFlowTests.cpp
  Private/Tests/BattlefieldQueryTests.cpp
  Private/Tests/BugfixTests.cpp
  Private/Tests/GridTests.cpp
  Private/Tests/TeamTests.cpp
)

target_include_directories(BattlefieldTests
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Public
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)

target_link_libraries(BattlefieldTests PUBLIC Battlefield)
target_compile_definitions(BattlefieldTests PRIVATE BATTLEFIELDTESTS_EXPORTS)

# C4100/C4189: the transpiled tests keep the original `Parameters` argument and a few unused locals.
if(MSVC)
  target_compile_options(BattlefieldTests PRIVATE /W4 /permissive- /utf-8 /wd4100 /wd4189)
else()
  target_compile_options(BattlefieldTests PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable)
endif()

# Console host: loads BattlefieldTests.dll and reports through the C API.
add_executable(BattlefieldTestRunner Runner/main.cpp)
target_link_libraries(BattlefieldTestRunner PRIVATE BattlefieldTests)

add_test(NAME BattlefieldTests COMMAND BattlefieldTestRunner --quiet)
