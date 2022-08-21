
include_directories("${CMAKE_CURRENT_LIST_DIR}/nes")
include_directories("${CMAKE_CURRENT_LIST_DIR}/nes/mapper")

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} PRJ_BUILD_SRCS)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/nes PRJ_BUILD_SRCS)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/common PRJ_BUILD_SRCS)
set(PRJ_ASM_SRCS
	${CMAKE_CURRENT_LIST_DIR}/nes/6502.s
)