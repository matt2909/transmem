#                                                              -*- Makefile -*-
# Copyright (c) Virtutech AB, All Rights Reserved
#
# Simics module makefile
#

# USER-TODO: specify the class(es) implemented in this module
MODULE_CLASSES=transaction_module
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR = /usr/local/lib
# USER-TODO: set file-names
SRC_FILES = transaction_module.c tm_utils.c tm_global.cpp transaction.cpp tm_init.cpp controller.cpp memory_trace.cpp thrift_client.cpp MemoryHierarchy.cpp simple_types.cpp simple_constants.cpp

MODULE_CFLAGS = 
MODULE_LDFLAGS = -lthrift

EXTRA_VPATH=${THRIFT_DIR} ${BOOST_DIR} ${LIB_DIR}

include $(MODULE_MAKEFILE)

