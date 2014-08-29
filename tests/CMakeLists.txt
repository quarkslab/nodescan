set(LINK_LIBRARIES nodescan ${Boost_LIBRARIES} ${PYTHON_LIBRARIES} ${TBB_LIBRARIES})

add_executable(size_trigger size_trigger.cpp)
target_link_libraries(size_trigger ${LINK_LIBRARIES}) 

add_executable(http http.cpp)
target_link_libraries(http ${LINK_LIBRARIES}) 

add_executable(buffer buffer.cpp)
target_link_libraries(buffer ${LINK_LIBRARIES}) 
add_test(TestBuffer buffer)

add_executable(data_trigger_variant data_trigger_variant.cpp)
target_link_libraries(data_trigger_variant ${LINK_LIBRARIES}) 
add_test(TestDataTriggerVariant data_trigger_variant)

add_executable(recon recon.cpp)
target_link_libraries(recon ${LINK_LIBRARIES})

add_executable(ssl ssl.cpp)
target_link_libraries(ssl ${LINK_LIBRARIES})

add_executable(ssh ssh.cpp)
target_link_libraries(ssh ${LINK_LIBRARIES})

add_executable(sip sip.cpp)
target_link_libraries(sip ${LINK_LIBRARIES})

add_executable(target_set target_set.cpp)
target_link_libraries(target_set ${LINK_LIBRARIES})
