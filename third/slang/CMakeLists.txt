add_library(slang INTERFACE)

# 使用 INTERFACE 关键字：父项目会自动继承这个路径
target_include_directories(slang
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_directories(slang
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)