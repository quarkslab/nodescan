set(PYTHON_SRC_FILES
	pynodescan.cpp
)

add_library(pynodescan SHARED ${PYTHON_SRC_FILES})
set_target_properties(pynodescan PROPERTIES PREFIX "") # We need our ".so" to be named "pyleeloo" (same name as in the BOOST_PYTHON_MODULE macro)
target_link_libraries(pynodescan nodescan)

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())" OUTPUT_VARIABLE PYTHON_INSTALL OUTPUT_STRIP_TRAILING_WHITESPACE)
install(TARGETS pynodescan LIBRARY DESTINATION ${PYTHON_INSTALL}/)
