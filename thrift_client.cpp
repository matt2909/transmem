#include "thrift_client.h"
#include <iostream>
using namespace facebook::thrift;
using namespace facebook::thrift::protocol;
using namespace facebook::thrift::transport;
using namespace boost;
using namespace simple;
using namespace std;

Thrift_Client::Thrift_Client() {
  shared_ptr<TTransport> socket(new TSocket("localhost", 9091));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  client = new MemoryHierarchyClient(protocol);
  cout << "Thrift client invoked!" << endl;
  try {
    transport->open();
    cout << "Transport opened" << endl;
  }
  catch (TException &tx) {
     printf("Exception triggered\n");
  }
}

Thrift_Client::~Thrift_Client() {
}

void Thrift_Client::step() {
}

void Thrift_Client::operate(int cpuid, int type, int addr) {
   Work work;
   work.cpuid = cpuid;
   work.type = (MemOp)type;
   work.addr = addr;

    try {
      client->operate(work);
    } catch (TException &tx) {
      cout << "InvalidOperation" << endl;
    }
}
