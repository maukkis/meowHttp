add_executable(wsText wsText.cpp)
add_executable(wsPing wsPing.cpp)
add_executable(httpsGet httpsGet.cpp)
add_executable(httpsPost httpsPost.cpp)
add_executable(wsBinary wsBinary.cpp)

add_dependencies(wsText meowHttp)
add_dependencies(wsPing meowHttp)
add_dependencies(httpsGet meowHttp)
add_dependencies(httpsPost meowHttp)
add_dependencies(wsBinary meowHttp)

add_test(NAME httpsGet COMMAND httpsGet)
add_test(NAME httpsPost COMMAND httpsPost)
add_test(NAME wsPing COMMAND wsPing)
add_test(NAME wsText COMMAND wsText)
add_test(NAME wsBinary COMMAND wsBinary)


target_link_libraries(wsPing PRIVATE meowHttp)
target_link_libraries(wsText PRIVATE meowHttp)
target_link_libraries(httpsGet PRIVATE meowHttp)
target_link_libraries(httpsPost PRIVATE meowHttp)
target_link_libraries(wsBinary PRIVATE meowHttp)
