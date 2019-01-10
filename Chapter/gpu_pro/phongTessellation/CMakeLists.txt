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

ADD_EXE( PhoneTessellation "${INC}" "${SRC}" )
SET_TARGET_PROPERTIES(PhoneTessellation PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${MEDIA_DIR}")
SET_TARGET_PROPERTIES(PhoneTessellation PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
SET_PROPERTY(TARGET PhoneTessellation PROPERTY FOLDER "Chapter/GpuPro")

ADD_DEPENDENCIES( PhoneTessellation Common )
TARGET_LINK_LIBRARIES( PhoneTessellation Common )
FOREACH(item ${LINK_LIB_DEBUG})
	TARGET_LINK_LIBRARIES(PhoneTessellation debug ${item})
ENDFOREACH(item)
FOREACH(item ${LINK_LIB_RELEASE})
	TARGET_LINK_LIBRARIES(PhoneTessellation optimized ${item})
ENDFOREACH(item)