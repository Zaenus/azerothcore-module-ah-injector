# mod-ah-injector custom CMake configuration
# This file is included by the parent modules/CMakeLists.txt if it exists

# Define module-specific compile options
target_compile_options(modules INTERFACE -DMOD_AH_INJECTOR)

# Module version
set(MOD_AH_INJECTOR_VERSION "1.0.0")
target_compile_definitions(modules INTERFACE MOD_AH_INJECTOR_VERSION="${MOD_AH_INJECTOR_VERSION}")