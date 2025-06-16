file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(subprocess_app ${SOURCES})
target_include_directories(subprocess_app PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(subprocess_app PRIVATE Boost::system spdlog::spdlog yaml-cpp::yaml-cpp)
