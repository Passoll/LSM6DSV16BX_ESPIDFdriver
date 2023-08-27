idf_component_register(SRCS "lsm6dsv16bx.cpp" "ad_wire.cpp" "lsm6dsv16bx_reg.c"
                    INCLUDE_DIRS "." "include" 
                    REQUIRES driver hal)
