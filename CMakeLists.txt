cmake_minimum_required(VERSION 3.25)

project(byte_level_subwords
    VERSION 
        0.1
    DESCRIPTION 
        "FAST Tokenizer for toy natural language understanding"
    LANGUAGES 
        CXX
)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Ofast)
find_package(benchmark REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    thirdparty/tqdm.cpp/include)

file(GLOB SOURCES "src/*.cc")

add_executable(run ${SOURCES})
target_link_libraries(run benchmark::benchmark)