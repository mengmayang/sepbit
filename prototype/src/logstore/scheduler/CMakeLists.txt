add_library(scheduler scheduler1.cc scheduler2.cc)

target_include_directories(scheduler PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_features(scheduler PRIVATE cxx_std_17)

target_link_libraries(scheduler selection)
