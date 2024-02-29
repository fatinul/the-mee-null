cmake_minimum_required(VERSION 3.11)
project(the-mee-null
        VERSION 0.1
        LANGUAGES CXX)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG 05fc866d7434566fd6cd0f39f62ba4bd8af855d7
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(the-mee-null src/main.cpp)
target_link_libraries(the-mee-null PRIVATE ftxui::component)