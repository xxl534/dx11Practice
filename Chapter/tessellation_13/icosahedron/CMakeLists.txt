AUX_SOURCE_DIRECTORY(. SRC)

FILE(GLOB_RECURSE INC "*.h")
FILE(GLOB_RECURSE INC_TEMP "*.inl")
LIST(APPEND INC ${INC_TEMP})
	
LINK_DIRECTORIES(
../../../../3rd/lib
)
INCLUDE_DIRECTORIES(
../../../Common;
../../../3rd/include/fx11;
../../../3rd/include/directXTK;
)

ADD_EXE( IcosahedronTessellation "${INC}" "${SRC}" )
SET_TARGET_PROPERTIES(IcosahedronTessellation PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
SET_TARGET_PROPERTIES(IcosahedronTessellation PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
SET_PROPERTY(TARGET IcosahedronTessellation PROPERTY FOLDER "Chapter/Tessellation")

ADD_DEPENDENCIES( IcosahedronTessellation Common )
TARGET_LINK_LIBRARIES( IcosahedronTessellation Common )
FOREACH(item ${LINK_LIB_DEBUG})
	TARGET_LINK_LIBRARIES(IcosahedronTessellation debug ${item})
ENDFOREACH(item)
FOREACH(item ${LINK_LIB_RELEASE})
	TARGET_LINK_LIBRARIES(IcosahedronTessellation optimized ${item})
ENDFOREACH(item)