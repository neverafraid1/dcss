# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/wangzhen/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/181.5087.36/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/wangzhen/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/181.5087.36/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug

# Include any dependencies generated for this target.
include src/TG/CMakeFiles/dcsstd.dir/depend.make

# Include the progress variables for this target.
include src/TG/CMakeFiles/dcsstd.dir/progress.make

# Include the compile flags for this target's objects.
include src/TG/CMakeFiles/dcsstd.dir/flags.make

src/TG/CMakeFiles/dcsstd.dir/main.cpp.o: src/TG/CMakeFiles/dcsstd.dir/flags.make
src/TG/CMakeFiles/dcsstd.dir/main.cpp.o: ../src/TG/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/TG/CMakeFiles/dcsstd.dir/main.cpp.o"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dcsstd.dir/main.cpp.o -c /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/main.cpp

src/TG/CMakeFiles/dcsstd.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dcsstd.dir/main.cpp.i"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/main.cpp > CMakeFiles/dcsstd.dir/main.cpp.i

src/TG/CMakeFiles/dcsstd.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dcsstd.dir/main.cpp.s"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/main.cpp -o CMakeFiles/dcsstd.dir/main.cpp.s

src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.requires:

.PHONY : src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.requires

src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.provides: src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.requires
	$(MAKE) -f src/TG/CMakeFiles/dcsstd.dir/build.make src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.provides.build
.PHONY : src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.provides

src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.provides.build: src/TG/CMakeFiles/dcsstd.dir/main.cpp.o


src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o: src/TG/CMakeFiles/dcsstd.dir/flags.make
src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o: ../src/TG/OKTGEngine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o -c /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/OKTGEngine.cpp

src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dcsstd.dir/OKTGEngine.cpp.i"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/OKTGEngine.cpp > CMakeFiles/dcsstd.dir/OKTGEngine.cpp.i

src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dcsstd.dir/OKTGEngine.cpp.s"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/OKTGEngine.cpp -o CMakeFiles/dcsstd.dir/OKTGEngine.cpp.s

src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.requires:

.PHONY : src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.requires

src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.provides: src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.requires
	$(MAKE) -f src/TG/CMakeFiles/dcsstd.dir/build.make src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.provides.build
.PHONY : src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.provides

src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.provides.build: src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o


src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o: src/TG/CMakeFiles/dcsstd.dir/flags.make
src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o: ../src/TG/TGEngine.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dcsstd.dir/TGEngine.cpp.o -c /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/TGEngine.cpp

src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dcsstd.dir/TGEngine.cpp.i"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/TGEngine.cpp > CMakeFiles/dcsstd.dir/TGEngine.cpp.i

src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dcsstd.dir/TGEngine.cpp.s"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG/TGEngine.cpp -o CMakeFiles/dcsstd.dir/TGEngine.cpp.s

src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.requires:

.PHONY : src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.requires

src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.provides: src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.requires
	$(MAKE) -f src/TG/CMakeFiles/dcsstd.dir/build.make src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.provides.build
.PHONY : src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.provides

src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.provides.build: src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o


src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o: src/TG/CMakeFiles/dcsstd.dir/flags.make
src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o: ../src/MD5/MD5.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o -c /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/MD5/MD5.cpp

src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.i"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/MD5/MD5.cpp > CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.i

src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.s"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/MD5/MD5.cpp -o CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.s

src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.requires:

.PHONY : src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.requires

src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.provides: src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.requires
	$(MAKE) -f src/TG/CMakeFiles/dcsstd.dir/build.make src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.provides.build
.PHONY : src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.provides

src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.provides.build: src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o


# Object files for target dcsstd
dcsstd_OBJECTS = \
"CMakeFiles/dcsstd.dir/main.cpp.o" \
"CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o" \
"CMakeFiles/dcsstd.dir/TGEngine.cpp.o" \
"CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o"

# External object files for target dcsstd
dcsstd_EXTERNAL_OBJECTS =

src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/main.cpp.o
src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o
src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o
src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o
src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/build.make
src/TG/dcsstd: src/Logger/libdcsslog.a
src/TG/dcsstd: src/TG/CMakeFiles/dcsstd.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable dcsstd"
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dcsstd.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/TG/CMakeFiles/dcsstd.dir/build: src/TG/dcsstd

.PHONY : src/TG/CMakeFiles/dcsstd.dir/build

src/TG/CMakeFiles/dcsstd.dir/requires: src/TG/CMakeFiles/dcsstd.dir/main.cpp.o.requires
src/TG/CMakeFiles/dcsstd.dir/requires: src/TG/CMakeFiles/dcsstd.dir/OKTGEngine.cpp.o.requires
src/TG/CMakeFiles/dcsstd.dir/requires: src/TG/CMakeFiles/dcsstd.dir/TGEngine.cpp.o.requires
src/TG/CMakeFiles/dcsstd.dir/requires: src/TG/CMakeFiles/dcsstd.dir/__/MD5/MD5.cpp.o.requires

.PHONY : src/TG/CMakeFiles/dcsstd.dir/requires

src/TG/CMakeFiles/dcsstd.dir/clean:
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG && $(CMAKE_COMMAND) -P CMakeFiles/dcsstd.dir/cmake_clean.cmake
.PHONY : src/TG/CMakeFiles/dcsstd.dir/clean

src/TG/CMakeFiles/dcsstd.dir/depend:
	cd /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/src/TG /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG /home/wangzhen/CLionProjects/DigitalCurrencyStrategySystem/cmake-build-debug/src/TG/CMakeFiles/dcsstd.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/TG/CMakeFiles/dcsstd.dir/depend

