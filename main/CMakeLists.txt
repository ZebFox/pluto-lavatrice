idf_component_register(SRC_DIRS . config view view/pages view/images view/widgets view/fonts view/intl model model/descriptions controller controller/com controller/configuration peripherals peripherals/display utils
                    INCLUDE_DIRS .)

add_definitions("-DGEL_PARAMETER_CONFIGURATION_HEADER=\"parameter_conf.h\"")
