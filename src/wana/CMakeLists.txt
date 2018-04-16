if(WITH_OSD_INSTRUMENT_FUNCTIONS AND CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  set(GCC_C_FLAGS "-finstrument-functions")
  set(GCC_C_FLAGS "${GCC_C_FLAGS} -finstrument-functions-exclude-function-list=_mm_loadu_si128,_mm_cmpeq_epi32,_mm_movemask_epi8")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_C_FLAGS}")
  set(wana_cyg_functions_src ${CMAKE_SOURCE_DIR}/src/tracing/cyg_profile_functions.c)
endif()

set(wana_srcs
  WANAgent.cc
  WANAgentClient.cc
  ${CMAKE_SOURCE_DIR}/src/common/TrackedOp.cc
  ${wana_cyg_functions_src})

add_library(wana STATIC ${wana_srcs}
  $<TARGET_OBJECTS:cls_references_objs>
  $<TARGET_OBJECTS:global_common_objs>
  $<TARGET_OBJECTS:heap_profiler_objs>)
target_link_libraries(wana ${LEVELDB_LIBRARIES} dmclock ${CMAKE_DL_LIBS} ${ALLOC_LIBS})
if(WITH_OSD_INSTRUMENT_FUNCTIONS)
  add_dependencies(wana cyg_profile_tp)
endif()

