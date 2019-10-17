cmake_minimum_required (VERSION 2.8.11)
project (ModalLogicFormulaProover)

if (WIN32)
	execute_process(COMMAND cmd /c npm install 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		RESULT_VARIABLE rv 
		OUTPUT_VARIABLE ov)
	message("rv='${rv}'")
	message("ov='${ov}'")
	execute_process(COMMAND cmd /c npm run build 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} R
		ESULT_VARIABLE rv 
		OUTPUT_VARIABLE ov)
	message("rv='${rv}'")
	message("ov='${ov}'")
else()	
	execute_process(COMMAND npm --version
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE rv 
		OUTPUT_VARIABLE ov)
	message("rv='${rv}'")
	message("ov='${ov}'")

	execute_process(COMMAND npm install 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		RESULT_VARIABLE rv 
		OUTPUT_VARIABLE ov)
	message("rv='${rv}'")
	message("ov='${ov}'")
	execute_process(COMMAND npm run build 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		RESULT_VARIABLE rv 
		OUTPUT_VARIABLE ov)
	message("rv='${rv}'")
	message("ov='${ov}'")
endif (WIN32)

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dist DESTINATION /)
#file(COPY e2e/views DESTINATION dist)
