#==============================
# 编译要求核验
#==============================

cmake_minimum_required(VERSION 3.10)

#==============================
# 项目设定
#==============================

project("Prometheus Mk4 Update1" LANGUAGES CXX CUDA)

#==============================
# 内部编译单元
#==============================

add_subdirectory("Core")
add_subdirectory("System")

#==============================
# 外部编译单元
#==============================

add_subdirectory("ThirdParty/GalaxyCamera")
add_subdirectory("ThirdParty/SerialPort")
add_subdirectory("ThirdParty/SerialPortUtilities")