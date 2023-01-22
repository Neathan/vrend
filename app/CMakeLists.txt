add_executable(app "src/main.cpp" "src/log.h" "src/app.h" "src/app.cpp" "src/graphics/renderer.h" "src/graphics/renderer.h" "src/graphics/renderer.cpp" "src/graphics/validation.h" "src/graphics/validation.cpp" "src/appinfo.h" "src/graphics/extensions.h" "src/graphics/extensions.cpp" "src/graphics/device.h" "src/graphics/device.cpp" "src/graphics/swap_chain.h" "src/graphics/swap_chain.cpp" "src/graphics/render_pass.h" "src/graphics/render_pass.cpp" "src/graphics/pipeline.h" "src/graphics/pipeline.cpp"  "src/data/model.h" "src/data/model.cpp" "src/tools/convert_model.h" "src/tools/convert_model.cpp" "src/data/model_source.h" "src/data/mesh.h")

set_property(TARGET app PROPERTY CXX_STANDARD 17)

target_link_libraries(app PUBLIC glfw)
target_link_libraries(app PUBLIC glad)
target_link_libraries(app PUBLIC glm)
target_link_libraries(app PUBLIC spdlog)

target_link_libraries(app PUBLIC tinygltf)

target_include_directories(app PUBLIC "src")

target_compile_definitions(app PUBLIC GLFW_INCLUDE_NONE)


#==============================================================================
# COMPILE SHADERS
#

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/assets/shaders)
set(SHADER_BINARY_DIR $<TARGET_FILE_DIR:app>/assets/shaders)

file(GLOB SHADER_FILES
	${SHADER_SOURCE_DIR}/*.vert
	${SHADER_SOURCE_DIR}/*.frag
	${SHADER_SOURCE_DIR}/*.comp)

add_custom_target(shaders)
add_custom_target(shader_directory ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}")

find_program(shader_compiler "glslc")
if(shader_compiler)
	set(SHADER_COMPILER "glslc")
else()
	set(SHADER_COMPILER "${CMAKE_SOURCE_DIR}/bin/glslc")
endif()

foreach(FILE ${SHADER_FILES})
	get_filename_component(FILENAME ${FILE} NAME)
	add_custom_command(
		TARGET shaders
	COMMAND ${SHADER_COMPILER} "--target-env=vulkan" ${FILE} -o "${SHADER_BINARY_DIR}/${FILENAME}.spv"
		DEPENDS ${FILE}
		COMMENT "Compiling shader: ${FILENAME}"
	)
endforeach(FILE)


add_dependencies(shaders shader_directory)
add_dependencies(app shaders)

#==============================================================================
# COPY ASSETS
#

# Textures
set(TEXTURE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/assets/textures)
set(TEXTURE_OUTPUT_DIR $<TARGET_FILE_DIR:app>/assets/textures)

add_custom_target(copy_textures
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEXTURE_SOURCE_DIR} ${TEXTURE_OUTPUT_DIR}
)
add_dependencies(app copy_textures)

# Models
set(MODEL_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/assets/models)
set(MODEL_OUTPUT_DIR $<TARGET_FILE_DIR:app>/assets/models)

add_custom_target(copy_models
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${MODEL_SOURCE_DIR} ${MODEL_OUTPUT_DIR}
)
add_dependencies(app copy_models)
