cmake_minimum_required(VERSION 2.8.3)
project(canon_vbm42)

#finding useful packages
find_package(catkin REQUIRED COMPONENTS roscpp sensor_msgs dynamic_reconfigure image_transport driver_base message_generation cv_bridge)

#other useful component for multithreading
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS thread)
find_package (OpenCV 3.1 EXACT REQUIRED)
find_package(CURL)

#set the default path for built executables to the "bin" directory (UNused)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory (UNused)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


#uncomment if you have defined messages
add_message_files(
  FILES
  PTZ.msg
)

generate_dynamic_reconfigure_options(cfg/CanonParams.cfg)
generate_messages()

# SET(USE_IPP no)
# IF(USE_IPP MATCHES yes)
# SET(IPP_INC xxx/xxx/xxx/include)
# SET(IPP_LIBDIR xxx/xxx/xxx/sharedlib)
# SET(IPP_LIBS ippjpeg ippj ippi ipps ippcore )
# SET(IPP_DIR libIppJpeg)
# SET(IPP_SRC JpegReader-ipp.cpp)
# SET(IPP_DEF -DUSE_IPP)
# SET(IPP_JPEG IppJpeg)
# ENDIF(USE_IPP MATCHES yes)

# catkin_package parameters: http://ros.org/doc/groovy/api/catkin/html/dev_guide/generated_cmake_api.html#catkin-package
catkin_package(
    CATKIN_DEPENDS roscpp sensor_msgs dynamic_reconfigure image_transport driver_base cv_bridge
    DEPENDS # TODO
    INCLUDE_DIRS include/ ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}# TODO include
  LIBRARIES ${Boost_LIBRARIES} ${OpenCV_LIBS}# TODO
)  
# ADD_DEFINITIONS(${IPP_DEF})
# 
# #common commands for building c++ executables and libraries
 add_library(${PROJECT_NAME} 
#     src/libSock/Socket.cpp
#     src/libSock/SocketCom.cpp
#     # src/libSock/Serializer.cpp
     src/libCanon/CanonDriver.cpp
#     src/libCanon/DgmQueue.cpp
#     src/libCanon/JpegReader.cpp
#     src/libCanon/Videogram.cpp
#     src/libCanon/login.cpp
#     src/libCanon/VideoManager.cpp
#     ${IPP_SRC}
#     src/libCanon/CommManager.cpp
#     src/libCanon/JpegStreamSaver.cpp
#     src/libCanon/Datagram.cpp
 )
# target_link_libraries(${PROJECT_NAME} ${IPP_JPEG} jpeg rt)
# 
# IF(USE_IPP MATCHES yes)
# add_library(${IPP_JPEG}
#     src/libIppJpeg/decoder.cpp
#     src/libIppJpeg/decqtbl.cpp
#     src/libIppJpeg/colorcomp.cpp
#     src/libIppJpeg/enchtbl.cpp
#     src/libIppJpeg/encqtbl.cpp
#     src/libIppJpeg/jpegbase.cpp
#     src/libIppJpeg/encoder.cpp
#     src/libIppJpeg/dechtbl.cpp
# )
# target_link_libraries(${IPP_JPEG} ${IPP_LIBS})
# ENDIF(USE_IPP MATCHES yes)
MESSAGE( STATUS "OpenCV_LIBS:     " ${CATKIN_GLOBAL_INCLUDE_DESTINATION})
include_directories(${Boost_INCLUDE_DIRS} include/ ${catkin_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${OpenCV_LIBS} ${CURL_LIBRARIES})


add_executable(${PROJECT_NAME}_node src/node/node.cpp)
target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME} ${catkin_LIBRARIES} ${OpenCV_LIBS})

add_executable(${PROJECT_NAME}_sub src/node/my_subscriber.cpp)
target_link_libraries(${PROJECT_NAME}_sub ${Boost_LIBRARIES} ${OpenCV_LIBS} ${catkin_LIBRARIES})
#creating shared lib
 install(TARGETS ${PROJECT_NAME}
 LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
#creating bin
install(TARGETS ${PROJECT_NAME}_node
RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(TARGETS ${PROJECT_NAME}_sub
RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
