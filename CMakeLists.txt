################################################################################

cmake_minimum_required (VERSION 3.5)
project (example)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

################################################################################

# TARGET rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE "include/rapidjson")

################################################################################

# TARGET bfxapicpp
add_library(bfxapicpp INTERFACE)
target_include_directories(bfxapicpp INTERFACE "include/bfx-api-cpp")
target_link_libraries(bfxapicpp INTERFACE rapidjson)

################################################################################

# TARGET example

set ( LIB_TRADING libindicators.a )
link_directories(~/workspace/gintrade/bfx-cpp-api/app/lib)
add_executable (example src/example.cpp src/candle.cpp src/instrument.cpp src/engine.cpp src/wallet.cpp src/config.cpp src/calculate.cpp src/position.cpp)
target_include_directories (example PRIVATE include)
target_include_directories (example PUBLIC "tulipindicators-0.8.0")


link_directories(${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(example bfxapicpp -lcryptopp -lcurl ${LIB_TRADING})
# Assuming example executable built into /bin directory configuration files
# will have following paths
target_compile_definitions(example PUBLIC
JSON_DEFINITIONS_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/definitions.json"
WITHDRAWAL_CONF_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/withdraw.conf")
# Enable all compiler warnings
target_compile_options(example PRIVATE -Wall)

################################################################################

# TARGET test
add_executable (testing src/test.cpp)
target_include_directories (testing PRIVATE include)
target_link_libraries(testing
PUBLIC bfxapicpp
PRIVATE -lcryptopp -lcurl)
# Assuming test executable built into /bin directory configuration files
# will have following paths
target_compile_definitions(testing PUBLIC
JSON_DEFINITIONS_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/definitions.json"
WITHDRAWAL_CONF_FILE_PATH="${PROJECT_SOURCE_DIR}/doc/withdraw.conf")
# Enable all compiler warnings
target_compile_options(testing PRIVATE -Wall)
