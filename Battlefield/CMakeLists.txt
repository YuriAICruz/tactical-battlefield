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

if(MSVC)
  target_compile_options(Battlefield PRIVATE /W4 /permissive- /utf-8)
else()
  target_compile_options(Battlefield PRIVATE -Wall -Wextra)
endif()

install(TARGETS Battlefield RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY Public/ DESTINATION include)
