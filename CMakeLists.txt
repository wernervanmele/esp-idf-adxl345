set(SOURCES "adxl345.c")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    PRIV_REQUIRES "i2c_manager"
)

# set_source_files_properties(${SOURCES}
#     PROPERTIES COMPILE_FLAGS
#     -Wall -Wextra -Werror
# )
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror -Wno-format)