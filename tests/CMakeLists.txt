add_executable(size_trigger size_trigger.cpp)
target_link_libraries(size_trigger nodescan) 

add_executable(http http.cpp)
target_link_libraries(http nodescan) 

add_executable(buffer buffer.cpp)
target_link_libraries(buffer nodescan) 
add_test(TestBuffer buffer)

add_executable(data_trigger_variant data_trigger_variant.cpp)
target_link_libraries(data_trigger_variant nodescan) 
add_test(TestDataTriggerVariant data_trigger_variant)

add_executable(servers servers.cpp)
target_link_libraries(servers nodescan tbb)

add_executable(recon recon.cpp)
target_link_libraries(recon nodescan tbb)

add_executable(ssl ssl.cpp)
target_link_libraries(ssl nodescan tbb)

add_executable(ssh ssh.cpp)
target_link_libraries(ssh nodescan tbb)

add_executable(sip sip.cpp)
target_link_libraries(sip nodescan tbb)

add_executable(target_set target_set.cpp)
target_link_libraries(target_set nodescan)

add_executable(whois whois.cpp)
target_link_libraries(whois nodescan tbb)
