all: android osx ios

.PHONY: clean
.PHONY: clean-android
.PHONY: clean-osx
.PHONY: clean-xcode
.PHONY: clean-ios
.PHONY: clean-rpi
.PHONY: clean-linux
.PHONY: clean-benchmark
.PHONY: android
.PHONY: osx
.PHONY: xcode
.PHONY: ios
.PHONY: ios-sim
.PHONY: rpi
.PHONY: linux
.PHONY: benchmark
.PHONY: check-ndk
.PHONY: cmake-osx
.PHONY: cmake-xcode
.PHONY: cmake-android
.PHONY: cmake-ios
.PHONY: cmake-ios-sim
.PHONY: cmake-rpi
.PHONY: cmake-linux
.PHONY: install-android

ANDROID_BUILD_DIR = build/android
OSX_BUILD_DIR = build/osx
OSX_XCODE_BUILD_DIR = build/xcode
IOS_BUILD_DIR = build/ios
IOS_SIM_BUILD_DIR = build/ios-sim
RPI_BUILD_DIR = build/rpi
LINUX_BUILD_DIR = build/linux
TESTS_BUILD_DIR = build/tests
BENCH_BUILD_DIR = build/bench

TOOLCHAIN_DIR = toolchains
OSX_TARGET = tangram
IOS_TARGET = tangram
OSX_XCODE_PROJ = tangram.xcodeproj
IOS_XCODE_PROJ = tangram.xcodeproj

ifdef ANDROID_X86
	ANDROID_BUILD_DIR = build/android-x86
	ANDROID_TOOLCHAIN = x86-clang3.5
	ANDROID_ARCH = x86
else
	ANDROID_TOOLCHAIN = arm-linux-androideabi-clang3.5
endif

ifndef ANDROID_ARCH
	ANDROID_ARCH = armeabi-v7a
endif

ifndef ANDROID_API_LEVEL
	ANDROID_API_LEVEL = android-15
endif

ifndef TANGRAM_CMAKE_OPTIONS
	TANGRAM_CMAKE_OPTIONS = \
		-DBENCHMARK=0 \
		-DUNIT_TESTS=0
endif

BENCH_CMAKE_PARAMS = \
	-DBENCHMARK=1 \
	-DAPPLICATION=0 \
	-DCMAKE_BUILD_TYPE=Release

UNIT_TESTS_CMAKE_PARAMS = \
	-DUNIT_TESTS=1 \
	-DAPPLICATION=0 \
	-DCMAKE_BUILD_TYPE=Debug

ANDROID_CMAKE_PARAMS = \
	-DPLATFORM_TARGET=android \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_DIR}/android.toolchain.cmake \
	-DMAKE_BUILD_TOOL=$$ANDROID_NDK/prebuilt/darwin-x86_64/bin/make \
	-DANDROID_ABI=${ANDROID_ARCH} \
	-DANDROID_STL=c++_shared \
	-DANDROID_TOOLCHAIN_NAME=${ANDROID_TOOLCHAIN} \
	-DANDROID_NATIVE_API_LEVEL=${ANDROID_API_LEVEL} \
	-DLIBRARY_OUTPUT_PATH_ROOT=../../android/tangram

IOS_CMAKE_PARAMS = \
	-DPLATFORM_TARGET=ios \
	-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_DIR}/iOS.toolchain.cmake \
	-G Xcode

DARWIN_XCODE_CMAKE_PARAMS = \
        ${TANGRAM_CMAKE_OPTIONS} \
	-DPLATFORM_TARGET=darwin \
	-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="10.9" \
	-G Xcode

DARWIN_CMAKE_PARAMS = \
        ${TANGRAM_CMAKE_OPTIONS} \
	-DPLATFORM_TARGET=darwin

RPI_CMAKE_PARAMS = \
        ${TANGRAM_CMAKE_OPTIONS} \
	-DPLATFORM_TARGET=raspberrypi

LINUX_CMAKE_PARAMS = \
        ${TANGRAM_CMAKE_OPTIONS} \
	-DPLATFORM_TARGET=linux

clean: clean-android clean-osx clean-ios clean-rpi clean-tests clean-xcode clean-linux

clean-android:
	@cd android/ && \
	./gradlew clean
	rm -rf ${ANDROID_BUILD_DIR}
	rm -rf android/tangram/libs

clean-osx:
	rm -rf ${OSX_BUILD_DIR}

clean-ios:
	rm -rf ${IOS_BUILD_DIR}

clean-rpi:
	rm -rf ${RPI_BUILD_DIR}

clean-linux:
	rm -rf ${LINUX_BUILD_DIR}

clean-xcode:
	rm -rf ${OSX_XCODE_BUILD_DIR}

clean-tests:
	rm -rf ${TESTS_BUILD_DIR}

clean-benchmark:
	rm -rf ${BENCH_BUILD_DIR}

android: android/tangram/libs/${ANDROID_ARCH}/libtangram.so android/build.gradle
	@cd android/ && \
	./gradlew demo:assembleDebug

android/tangram/libs/${ANDROID_ARCH}/libtangram.so: install-android

install-android: ${ANDROID_BUILD_DIR}/Makefile
	@cd ${ANDROID_BUILD_DIR} && \
	${MAKE} && \
	${MAKE} install

${ANDROID_BUILD_DIR}/Makefile: check-ndk cmake-android

cmake-android:
	@mkdir -p ${ANDROID_BUILD_DIR}
	@cd ${ANDROID_BUILD_DIR} && \
	cmake ../.. ${ANDROID_CMAKE_PARAMS}

osx: ${OSX_BUILD_DIR}/Makefile
	@cd ${OSX_BUILD_DIR} && \
	${MAKE}

${OSX_BUILD_DIR}/Makefile: cmake-osx

xcode: ${OSX_XCODE_BUILD_DIR}/${OSX_XCODE_PROJ}
	xcodebuild -target ${OSX_TARGET} -project ${OSX_XCODE_BUILD_DIR}/${OSX_XCODE_PROJ}

${OSX_XCODE_BUILD_DIR}/${OSX_XCODE_PROJ}: cmake-xcode

cmake-xcode:
	@mkdir -p ${OSX_XCODE_BUILD_DIR}
	@cd ${OSX_XCODE_BUILD_DIR} && \
	cmake ../.. ${DARWIN_XCODE_CMAKE_PARAMS}

cmake-osx:
	@mkdir -p ${OSX_BUILD_DIR}
	@cd ${OSX_BUILD_DIR} && \
	cmake ../.. ${DARWIN_CMAKE_PARAMS}

ios: ${IOS_BUILD_DIR}/${IOS_XCODE_PROJ}
	xcodebuild -target ${IOS_TARGET} -project ${IOS_BUILD_DIR}/${IOS_XCODE_PROJ}

${IOS_BUILD_DIR}/${IOS_XCODE_PROJ}: cmake-ios

cmake-ios:
	@mkdir -p ${IOS_BUILD_DIR}
	@cd ${IOS_BUILD_DIR} && \
	cmake ../.. ${IOS_CMAKE_PARAMS}

ios-sim: ${IOS_SIM_BUILD_DIR}/${IOS_XCODE_PROJ}
	xcodebuild -target ${IOS_TARGET} -project ${IOS_SIM_BUILD_DIR}/${IOS_XCODE_PROJ}

${IOS_SIM_BUILD_DIR}/${IOS_XCODE_PROJ}: cmake-ios-sim

cmake-ios-sim:
	@mkdir -p ${IOS_SIM_BUILD_DIR}
	@cd ${IOS_SIM_BUILD_DIR} && \
	cmake ../.. ${IOS_CMAKE_PARAMS} -DIOS_PLATFORM=SIMULATOR

rpi: cmake-rpi
	@cd ${RPI_BUILD_DIR} && \
	${MAKE}

cmake-rpi:
	@mkdir -p ${RPI_BUILD_DIR}
	@cd ${RPI_BUILD_DIR} && \
	cmake ../.. ${RPI_CMAKE_PARAMS}

linux: cmake-linux
	cd ${LINUX_BUILD_DIR} && \
	${MAKE}

cmake-linux:
	mkdir -p ${LINUX_BUILD_DIR}
	cd ${LINUX_BUILD_DIR} &&\
	cmake ../.. ${LINUX_CMAKE_PARAMS}

tests: unit-tests

unit-tests:
	@mkdir -p ${TESTS_BUILD_DIR}
	@cd ${TESTS_BUILD_DIR} && \
	cmake ../.. ${UNIT_TESTS_CMAKE_PARAMS} && \
	${MAKE}

benchmark:
	@mkdir -p ${BENCH_BUILD_DIR}
	@cd ${BENCH_BUILD_DIR} && \
	cmake ../../ ${BENCH_CMAKE_PARAMS} && \
	${MAKE}

check-ndk:
ifndef ANDROID_NDK
	$(error ANDROID_NDK is undefined)
endif

format:
	@for file in `git diff --diff-filter=ACMRTUXB --name-only -- '*.cpp' '*.h'`; do \
		if [[ -e $$file ]]; then clang-format -i $$file; fi \
	done
	@echo "format done on `git diff --diff-filter=ACMRTUXB --name-only -- '*.cpp' '*.h'`"

swig-bindings:
	@mkdir -p generated
	@swig -v -c++  -Icore/src -o android/tangram/jni/jniGenerated.cpp \
		-outdir generated \
		-package com.mapzen.tangram -java swig/tangram.i
	@astyle --style=attach --indent=spaces=2 android/tangram/jni/jniGenerated.cpp
	@astyle --style=java generated/*.java
	@mv generated/*.java android/tangram/src/com/mapzen/tangram
