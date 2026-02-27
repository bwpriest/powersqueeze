# Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
# powersqueeze Project Developers. See the top-level COPYRIGHT file for details.

function (add_hdknn_example example_name)
    set(example_source "${example_name}.cpp")
    set(example_exe "${example_name}")
    add_executable(${example_exe} ${example_source})
    target_link_libraries(${example_exe} PRIVATE hdknn)
    target_include_directories(
        ${example_exe}
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
    )
endfunction ()

add_hdknn_example(dhnsw_only)
add_hdknn_example(dnnd_only)
