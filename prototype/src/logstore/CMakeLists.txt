#add_library(logstore logstore/logstore.cc logstore/manager.cc logstore/segment.cc logstore/scheduler.cc)
add_subdirectory(scheduler)

add_library(logstore logstore.cc manager.cc segment.cc)

target_include_directories(logstore PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(logstore indexmap placement selection storage_adapter scheduler)
target_compile_features(logstore PRIVATE cxx_std_17)

