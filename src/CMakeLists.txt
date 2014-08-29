set(SRC_FILES
	async_engine.cpp
	data_triggers.cpp
	engine.cpp
	host_state_machine.cpp
	ipstr.cpp
	lvl4_state_machine.cpp
	state_machine.cpp
	target.cpp
	target_set.cpp
	timestamp.cpp
	connected_target.cpp
	protocols/ssl.cpp
	protocols/ssh.cpp
	protocols/sip.cpp
)

set(CONFIG_H ${CMAKE_CURRENT_BINARY_DIR}/include/ns/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/ns/config.h.in ${CONFIG_H})

set(HEADER_DIST_FILES
	include/ns/action.h
	include/ns/async_engine.h
	include/ns/buffer.h
	include/ns/connected_target.h
	include/ns/data_trigger_base.h
	include/ns/data_triggers.h
	include/ns/data_trigger_variant.h
	include/ns/engine.h
	include/ns/host_state_machine.h
	include/ns/ipstr.h
	include/ns/lvl4_buffer.h
	include/ns/lvl4_properties_storage.h
	include/ns/lvl4_state_machine.h
	include/ns/state_machine.h
	include/ns/static_max_size.h
	include/ns/target_file.h
	include/ns/target.h
	include/ns/target_set.h
	include/ns/timestamp.h

	include/ns/protocols/sip.h
	include/ns/protocols/ssh.h
	include/ns/protocols/ssl.h
	include/ns/protocols/ssl_structs.h

	${CONFIG_H}
)

add_library(nodescan SHARED ${SRC_FILES})
target_link_libraries(nodescan leeloo ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${TBB_LIBRARIES}) 

install(TARGETS nodescan LIBRARY DESTINATION lib)
install(FILES ${HEADER_DIST_FILES} DESTINATION include/ns)
