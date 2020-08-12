PROJECT( programa )

cmake_minimum_required(VERSION 2.8)

find_package( OpenCV REQUIRED )

ADD_EXECUTABLE( programa main.cpp )

SET(CMAKE_C_COMPILER mpicc)

SET(CMAKE_CXX_COMPILER mpicxx)

target_link_libraries( programa mpi ${OpenCV_LIBS} )


