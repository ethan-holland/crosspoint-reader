add_executable(ShelfSyncPlanTest
  ShelfSyncPlanTest.cpp
  ${REPO_ROOT}/src/util/ShelfSyncPlan.cpp
)

target_include_directories(ShelfSyncPlanTest PRIVATE
  ${REPO_ROOT}/src/util
)

target_link_libraries(ShelfSyncPlanTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ShelfSyncPlanTest)
