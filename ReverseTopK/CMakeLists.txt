
set(dyy
        dyy_data.cpp
        dyy_RstarTree.cpp
        dyy_RTK_method.cpp)

add_library(dyylib SHARED ${dyy})
set_target_properties(dyylib PROPERTIES LINKER_LANGUAGE CXX)
#target_link_directories(fexiprolib PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
#target_link_libraries(fexiprolib PRIVATE armadillo pthread OpenMP::OpenMP_CXX spdlog::spdlog ${Boost_LIBRARIES})
target_include_directories(dyylib PRIVATE ./)

add_executable(main main.cpp dyy_data.cpp dyy_RstarTree.cpp dyy_RTK_method.cpp)
#target_include_directories(main PRIVATE ./)
#target_link_libraries(main PRIVATE dyylib)
