#ifndef THRIFT_CLIENT_H
#define THRIFT_CLIENT_H

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include "MemoryHierarchy.h"
#include "simple_types.h"
#include "simple_constants.h"
#include <iostream>

class Thrift_Client {
 public:
    Thrift_Client();
    ~Thrift_Client();
    void step();
    void operate(int cpuid, int type, int address);
 private:
    simple::MemoryHierarchyClient* client;
};

#endif
