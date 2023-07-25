idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")
set_source_files_properties(main.c
    PROPERTIES COMPILE_FLAGS
    -Wall -Wextra -Werror
)