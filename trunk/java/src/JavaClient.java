// Generated code
import simple.*;

import com.facebook.thrift.TException;
import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.transport.TSocket;
import com.facebook.thrift.transport.TTransportException;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TProtocol;

import java.util.AbstractMap;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;

public class JavaClient {
  public static void main(String [] args) {
    try {

      TTransport transport = new TSocket("localhost", 9090);
      TProtocol protocol = new TBinaryProtocol(transport);
      MemoryHierarchy.Client client = new MemoryHierarchy.Client(protocol);

      transport.open();

      client.step();
      System.out.println("client.step()");

    } catch (TException x) {
      x.printStackTrace();
    }

  }

}
