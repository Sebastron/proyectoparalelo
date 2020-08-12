PROJECT( programa )

cmake_minimum_required(VERSION 2.8)

find_package( OpenCV REQUIRED )

find_package( MPI REQUIRED )

ADD_EXECUTABLE( programa main.cpp )

add_definitions( -DOMPI_SKIP_MPICXX )

SET(CMAKE_C_COMPILER mpicc)

SET(CMAKE_CXX_COMPILER mpicxx)

SET(OPENCV_DIR /usr/local/OpenCV/lib/cmake/opencv4)

target_link_libraries( programa mpi ${OpenCV_LIBS} )


