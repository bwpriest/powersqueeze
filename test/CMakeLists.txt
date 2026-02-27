# Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
# powersqueeze Project Developers. See the top-level COPYRIGHT file for details.

find_package(MPI REQUIRED)

function (add_psqz_mpi_test test_name)
    set(test_source "${test_name}.cpp")
    set(test_exe "${test_name}")
    add_executable(${test_exe} ${test_source})
    target_link_libraries(${test_exe} PUBLIC psqz::psqz)
    if (${TEST_WITH_SLURM})
        add_test(${test_exe} "srun" "${CMAKE_CURRENT_BINARY_DIR}/${test_exe}")
    else ()
        add_test(${test_exe} ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} "4"
                 ${MPIEXEC_PREFLAGS} "${CMAKE_CURRENT_BINARY_DIR}/${test_exe}"
        )
    endif ()
endfunction ()

function (add_psqz_test test_name)
    set(test_source "${test_name}.cpp")
    set(test_exe "${test_name}")
    add_executable(${test_exe} ${test_source})
    target_link_libraries(${test_exe} PUBLIC psqz::psqz)
    add_test(${test_exe} "${CMAKE_CURRENT_BINARY_DIR}/${test_exe}")
endfunction ()

