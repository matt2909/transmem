import com.facebook.thrift.TException;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.server.TServer;
import com.facebook.thrift.server.TSimpleServer;
import com.facebook.thrift.transport.TServerSocket;
import com.facebook.thrift.transport.TServerTransport;

// Generated code
import simple.*;

import java.util.HashMap;

public class JavaServer {

  public static class MemoryHierarchyHandler implements MemoryHierarchy.Iface {

    private long count_ops = 0;
    private int depth = 0;
    private int abort = 0;
    private int commit = 0;
    private int timeStamp = 0;
    public MemoryHierarchyHandler() {
  
    }

    public void step() {
      System.out.println("step()");
    }

    public void operate(Work work) {
      //System.out.println("operate(" + work.cpuid + ", {" + work.type + ", 0x" + Integer.toHexString(work.addr) + "})");
      count_ops++;
      switch(work.type) {
         case 0xA: depth--; abort++; dumpDepth(work.addr); break;
         case 0xB: depth++; dumpDepth(work.addr); break;
         case 0xC: depth--; commit++; dumpDepth(work.addr); break;
         /*case 0xA: System.out.println("[" + work.cpuid + "] Abort"); break;
         case 0xB: System.out.println("[" + work.cpuid + "] Begin"); break;
         case 0xC: System.out.println("[" + work.cpuid + "] Commit"); break;
         case 0xD: System.out.println("[" + work.cpuid + "] Enable"); break;
         case 0xE: System.out.println("[" + work.cpuid + "] Disable"); break;
         case 0xF: System.out.println("[" + work.cpuid + "] Info"); break;
	 */
    	 default: break;
      }
    }

    private void dumpDepth(int timeStamp) {
       if(this.timeStamp == 0) 
          this.timeStamp = timeStamp;
       System.out.println((timeStamp - this.timeStamp) + " " + depth + " (" + commit + ":" + abort + ")");
    }
  }

  public static void main(String [] args) {
    try {
      MemoryHierarchyHandler handler = new MemoryHierarchyHandler();
      MemoryHierarchy.Processor processor = new MemoryHierarchy.Processor(handler);
      TServerTransport serverTransport = new TServerSocket(9091);
      TServer server = new TSimpleServer(processor, serverTransport);

      // Use this for a multithreaded server
      // server = new TThreadPoolServer(processor, serverTransport);

      System.out.println("Starting the server...");
      server.serve();

    } catch (Exception x) {
      x.printStackTrace();
    }
    System.out.println("done.");
  }


}
