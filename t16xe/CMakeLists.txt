cmake_minimum_required(VERSION 4.0)
project(t16xe)
add_executable(${PROJECT_NAME} "src/main.cpp" "src/tasm.cpp" "src/t16xe.cpp" ) # "src/tcc/tcc.cpp"
# add_custom_target(rebuild
#     COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
#     COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
#     COMMAND ${CMAKE_COMMAND} ..
#     COMMAND ${CMAKE_COMMAND} --build .
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     COMMENT "Cleaning and rebuilding..."
# )

# add_custom_target(run
#     COMMAND t16xe
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     DEPENDS t16xe
#     COMMENT "Running t16xe..."
# )