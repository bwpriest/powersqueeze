# Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
# powersqueeze Project Developers. See the top-level COPYRIGHT file for details.

function (add_psqz_example example_name)
    set(example_source "${example_name}.cpp")
    set(example_exe "${example_name}")
    add_executable(${example_exe} ${example_source})
    target_link_libraries(${example_exe} PRIVATE psqz)
    target_include_directories(
        ${example_exe}
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
    )
endfunction ()

add_psqz_example(jaccard_tsv)
add_psqz_example(power_iteration_kron)
add_psqz_example(power_iteration_tsv)
add_psqz_example(power_iteration_wdc)
add_psqz_example(streaming_pi_tsv)
