#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

UGFXLIB = ugfx

COMPONENT_ADD_INCLUDEDIRS := .\
    ./include \
    $(UGFXLIB) \

COMPONENT_PRIV_INCLUDEDIRS +=  .\
    ./include \
    $(UGFXLIB) \
    $(UGFXLIB)/src \

