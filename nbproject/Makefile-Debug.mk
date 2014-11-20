#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1444234597/svm_common.o \
	${OBJECTDIR}/_ext/1444234597/svm_hideo.o \
	${OBJECTDIR}/_ext/1444234597/svm_learn.o \
	${OBJECTDIR}/libsvm/svm.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`pkg-config --cflags opencv` 
CXXFLAGS=`pkg-config --cflags opencv` 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs opencv`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk opencvhogtrainer

opencvhogtrainer: ${OBJECTFILES}
	${LINK.cc} -o opencvhogtrainer ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1444234597/svm_common.o: /home/dahoc/projects/openCVHogTrainer/svmlight/svm_common.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1444234597
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1444234597/svm_common.o /home/dahoc/projects/openCVHogTrainer/svmlight/svm_common.c

${OBJECTDIR}/_ext/1444234597/svm_hideo.o: /home/dahoc/projects/openCVHogTrainer/svmlight/svm_hideo.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1444234597
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1444234597/svm_hideo.o /home/dahoc/projects/openCVHogTrainer/svmlight/svm_hideo.c

${OBJECTDIR}/_ext/1444234597/svm_learn.o: /home/dahoc/projects/openCVHogTrainer/svmlight/svm_learn.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1444234597
	${RM} "$@.d"
	$(COMPILE.c) -g `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1444234597/svm_learn.o /home/dahoc/projects/openCVHogTrainer/svmlight/svm_learn.c

${OBJECTDIR}/libsvm/svm.o: libsvm/svm.cpp 
	${MKDIR} -p ${OBJECTDIR}/libsvm
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libsvm/svm.o libsvm/svm.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} opencvhogtrainer

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
