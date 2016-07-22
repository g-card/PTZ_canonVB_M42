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

#message generation and dynamic reconf.
generate_dynamic_reconfigure_options(cfg/CanonParams.cfg)
generate_messages()

# catkin_package parameters: http://ros.org/doc/groovy/api/catkin/html/dev_guide/generated_cmake_api.html#catkin-package
catkin_package(
    CATKIN_DEPENDS roscpp sensor_msgs dynamic_reconfigure image_transport driver_base cv_bridge
    DEPENDS
    INCLUDE_DIRS include/ ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
  LIBRARIES ${Boost_LIBRARIES} ${OpenCV_LIBS}
)  
#common commands for building c++ executables and libraries
add_library(${PROJECT_NAME} src/libCanon/CanonDriver.cpp src/capteurs.cpp)
include_directories(${Boost_INCLUDE_DIRS} include/ ${catkin_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${OpenCV_LIBS} ${CURL_LIBRARIES})


add_executable(main src/node/node.cpp)
target_link_libraries(main ${PROJECT_NAME} ${catkin_LIBRARIES} ${OpenCV_LIBS})

add_executable(${PROJECT_NAME}_sub src/node/my_subscriber.cpp)
target_link_libraries(${PROJECT_NAME}_sub ${Boost_LIBRARIES} ${OpenCV_LIBS} ${catkin_LIBRARIES})

add_executable(cmd src/node/cmd.cpp)
target_link_libraries(cmd ${PROJECT_NAME} ${catkin_LIBRARIES} ${OpenCV_LIBS} ${ZBAR_LIBRARIES}zbar)

#creating shared lib
 install(TARGETS ${PROJECT_NAME}
 LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
#creating bin
install(TARGETS main
RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(TARGETS ${PROJECT_NAME}_sub
RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(TARGETS cmd
RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
