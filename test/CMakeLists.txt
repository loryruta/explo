add_executable(explo_test
    main.cpp
    OctreeTest.cpp
    MiscTest.cpp
    DeltaChunkIteratorTest.cpp
    )

# ------------------------------------------------------------------------------------------------ Dependencies

# explo_lib
target_link_libraries(explo_test PRIVATE explo_lib)

# catch2
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(explo_test PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
