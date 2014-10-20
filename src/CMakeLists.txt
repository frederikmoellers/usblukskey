# source files
set(usbLUKSkey_SOURCES "main.cpp" "settings.cpp")

# add the executable
add_executable("usbLUKSkey" ${usbLUKSkey_SOURCES})

# find boost libraries and use them
find_package(Boost 1.55 REQUIRED COMPONENTS "chrono" "filesystem" "system" "thread")
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries ("usbLUKSkey" ${Boost_LIBRARIES})

install (TARGETS "usbLUKSkey" DESTINATION bin)
