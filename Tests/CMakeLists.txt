add_executable (LuanaticTests LuanaticTests.cpp)
target_link_libraries(LuanaticTests ${LUANATICDEPS})
add_custom_target(check COMMAND LuanaticTests)
