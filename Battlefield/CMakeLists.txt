# Project 1: Battlefield.dll - the turn based tactical battle core (pure C++20, no engine dependency).

add_library(Battlefield SHARED
  Private/Platform/BattlefieldLog.cpp
  Private/Core/BaseUnit.cpp
  Private/Core/BattleController.cpp
  Private/Core/BattleQueries.cpp
  Private/Core/BattlefieldData.cpp
  Private/Core/Grid.cpp
  Private/Core/GridEntity.cpp
  Private/Core/TeamSystem.cpp
  Private/Core/Troupe.cpp
  Private/Core/StatusEffects/BaseStatusEffect.cpp
  Private/Core/StatusEffects/StatusEffect.cpp
  Private/Math/GridMath.cpp
)

target_include_directories(Battlefield
  PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Public>
          $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)

target_compile_features(Battlefield PUBLIC cxx_std_20)
target_compile_definitions(Battlefield PRIVATE BATTLEFIELD_EXPORTS)

# Debug and Release share one install prefix (dist): the Debug binaries get a "d" suffix (Battlefieldd.dll/.lib)
# so the second install does not overwrite the first. find_package(Battlefield) picks the right one per config.
set_target_properties(Battlefield PROPERTIES DEBUG_POSTFIX d)

if(MSVC)
  target_compile_options(Battlefield PRIVATE /W4 /permissive- /utf-8)
else()
  target_compile_options(Battlefield PRIVATE -Wall -Wextra)
endif()

# --- Installable CMake package: find_package(Battlefield CONFIG) gives Battlefield::Battlefield ---
include(CMakePackageConfigHelpers)

install(TARGETS Battlefield EXPORT BattlefieldTargets
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY Public/ DESTINATION include)
install(EXPORT BattlefieldTargets NAMESPACE Battlefield:: FILE BattlefieldTargets.cmake
  DESTINATION lib/cmake/Battlefield)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/BattlefieldConfig.cmake
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/BattlefieldTargets.cmake\")
")
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/BattlefieldConfigVersion.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BattlefieldConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/BattlefieldConfigVersion.cmake
  DESTINATION lib/cmake/Battlefield)
