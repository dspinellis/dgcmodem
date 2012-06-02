IMPORTED_ARCH := $(shell if uname --hardware-platform >/dev/null 2>&1 && ! uname --hardware-platform | grep -q "unknown"; then uname --hardware-platform; else uname --machine | sed 's/^i.86$$/i386/'; fi)
IMPORTED_COMPILATION_FLAGS = -O2 -ffast-math -fno-strict-aliasing -fno-common
ifeq ($(IMPORTED_ARCH),i386)
IMPORTED_COMPILATION_FLAGS += -march=pentium
endif
ifeq ($(IMPORTED_ARCH),x86_64)
IMPORTED_COMPILATION_FLAGS += -mcmodel=kernel -mno-red-zone -mfpmath=387
endif
IMPORTED_TARGET = TARGET_DGC_LINUX
IMPORTED_FRAMEWORK = FWK_LINUX_SOFTK56
IMPORTED_FRAMEWORK_DEFS = -DFRAME_WORK=FWK_LINUX_SOFTK56 -DTARGET_DGC_LINUX -DNO_DUMPMGR_SUPPORT -DNO_BLAM_SUPPORT -DFRAME_WORK_IMPORTED -DUSE_DCP
IMPORTED_GENERAL_DEFS = 
IMPORTED_DEBUG_DEFS = #-DNOASSERTS -DUSE_TRACE_ONLY="T__FATAL_ERROR|T__ERROR" -DUSE_TRACE
IMPORTED_CNXTLINUXVERSION = 1.13
