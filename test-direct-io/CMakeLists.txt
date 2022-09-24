add_executable(generate gen_test_file.cpp)
add_executable(open test_open.c)

add_executable(read test_read.cpp)
target_include_directories(read PRIVATE src/include src/impl ${armadillo_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(read PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog Eigen3::Eigen ${Boost_LIBRARIES} fexiprolib)

add_executable(obs_read test_read_obsolete.cpp)
