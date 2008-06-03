/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
package simple;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import com.facebook.thrift.*;

import com.facebook.thrift.protocol.*;
import com.facebook.thrift.transport.*;

public class MemoryHierarchy {

  /**
   * Ahh, now onto the cool part, defining a service. Services just need a name
   * and can optionally inherit from another service using the extends keyword.
   */
  public interface Iface {

    public void step() throws TException;

    /**
     * lets have a simple generic method for now that recieves all events
     * we can always optimize this at a later point.
     */
    public void operate(Work w) throws TException;

  }

  public static class Client implements Iface {
    public Client(TProtocol prot)
    {
      this(prot, prot);
    }

    public Client(TProtocol iprot, TProtocol oprot)
    {
      iprot_ = iprot;
      oprot_ = oprot;
    }

    protected TProtocol iprot_;
    protected TProtocol oprot_;

    protected int seqid_;

    public TProtocol getInputProtocol()
    {
      return this.iprot_;
    }

    public TProtocol getOutputProtocol()
    {
      return this.oprot_;
    }

    public void step() throws TException
    {
      send_step();
      recv_step();
    }

    public void send_step() throws TException
    {
      oprot_.writeMessageBegin(new TMessage("step", TMessageType.CALL, seqid_));
      step_args args = new step_args();
      args.write(oprot_);
      oprot_.writeMessageEnd();
      oprot_.getTransport().flush();
    }

    public void recv_step() throws TException
    {
      TMessage msg = iprot_.readMessageBegin();
      if (msg.type == TMessageType.EXCEPTION) {
        TApplicationException x = TApplicationException.read(iprot_);
        iprot_.readMessageEnd();
        throw x;
      }
      step_result result = new step_result();
      result.read(iprot_);
      iprot_.readMessageEnd();
      return;
    }

    public void operate(Work w) throws TException
    {
      send_operate(w);
      recv_operate();
    }

    public void send_operate(Work w) throws TException
    {
      oprot_.writeMessageBegin(new TMessage("operate", TMessageType.CALL, seqid_));
      operate_args args = new operate_args();
      args.w = w;
      args.write(oprot_);
      oprot_.writeMessageEnd();
      oprot_.getTransport().flush();
    }

    public void recv_operate() throws TException
    {
      TMessage msg = iprot_.readMessageBegin();
      if (msg.type == TMessageType.EXCEPTION) {
        TApplicationException x = TApplicationException.read(iprot_);
        iprot_.readMessageEnd();
        throw x;
      }
      operate_result result = new operate_result();
      result.read(iprot_);
      iprot_.readMessageEnd();
      return;
    }

  }
  public static class Processor implements TProcessor {
    public Processor(Iface iface)
    {
      iface_ = iface;
      processMap_.put("step", new step());
      processMap_.put("operate", new operate());
    }

    protected static interface ProcessFunction {
      public void process(int seqid, TProtocol iprot, TProtocol oprot) throws TException;
    }

    private Iface iface_;
    protected final HashMap<String,ProcessFunction> processMap_ = new HashMap<String,ProcessFunction>();

    public boolean process(TProtocol iprot, TProtocol oprot) throws TException
    {
      TMessage msg = iprot.readMessageBegin();
      ProcessFunction fn = processMap_.get(msg.name);
      if (fn == null) {
        TProtocolUtil.skip(iprot, TType.STRUCT);
        iprot.readMessageEnd();
        TApplicationException x = new TApplicationException(TApplicationException.UNKNOWN_METHOD, "Invalid method name: '"+msg.name+"'");
        oprot.writeMessageBegin(new TMessage(msg.name, TMessageType.EXCEPTION, msg.seqid));
        x.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
        return true;
      }
      fn.process(msg.seqid, iprot, oprot);
      return true;
    }

    private class step implements ProcessFunction {
      public void process(int seqid, TProtocol iprot, TProtocol oprot) throws TException
      {
        step_args args = new step_args();
        args.read(iprot);
        iprot.readMessageEnd();
        step_result result = new step_result();
        iface_.step();
        oprot.writeMessageBegin(new TMessage("step", TMessageType.REPLY, seqid));
        result.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
      }

    }

    private class operate implements ProcessFunction {
      public void process(int seqid, TProtocol iprot, TProtocol oprot) throws TException
      {
        operate_args args = new operate_args();
        args.read(iprot);
        iprot.readMessageEnd();
        operate_result result = new operate_result();
        iface_.operate(args.w);
        oprot.writeMessageBegin(new TMessage("operate", TMessageType.REPLY, seqid));
        result.write(oprot);
        oprot.writeMessageEnd();
        oprot.getTransport().flush();
      }

    }

  }

  public static class step_args implements TBase, java.io.Serializable   {
    public step_args() {
    }

    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof step_args)
        return this.equals((step_args)that);
      return false;
    }

    public boolean equals(step_args that) {
      if (that == null)
        return false;

      return true;
    }

    public int hashCode() {
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin();
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();
    }

    public void write(TProtocol oprot) throws TException {
      TStruct struct = new TStruct("step_args");
      oprot.writeStructBegin(struct);
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    public String toString() {
      StringBuilder sb = new StringBuilder("step_args(");
      sb.append(")");
      return sb.toString();
    }

  }

  public static class step_result implements TBase, java.io.Serializable   {
    public step_result() {
    }

    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof step_result)
        return this.equals((step_result)that);
      return false;
    }

    public boolean equals(step_result that) {
      if (that == null)
        return false;

      return true;
    }

    public int hashCode() {
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin();
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();
    }

    public void write(TProtocol oprot) throws TException {
      TStruct struct = new TStruct("step_result");
      oprot.writeStructBegin(struct);

      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    public String toString() {
      StringBuilder sb = new StringBuilder("step_result(");
      sb.append(")");
      return sb.toString();
    }

  }

  public static class operate_args implements TBase, java.io.Serializable   {
    public Work w;

    public final Isset __isset = new Isset();
    public static final class Isset implements java.io.Serializable {
      public boolean w = false;
    }

    public operate_args() {
    }

    public operate_args(
      Work w)
    {
      this();
      this.w = w;
      this.__isset.w = true;
    }

    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof operate_args)
        return this.equals((operate_args)that);
      return false;
    }

    public boolean equals(operate_args that) {
      if (that == null)
        return false;

      boolean this_present_w = true && (this.w != null);
      boolean that_present_w = true && (that.w != null);
      if (this_present_w || that_present_w) {
        if (!(this_present_w && that_present_w))
          return false;
        if (!this.w.equals(that.w))
          return false;
      }

      return true;
    }

    public int hashCode() {
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin();
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          case 1:
            if (field.type == TType.STRUCT) {
              this.w = new Work();
              this.w.read(iprot);
              this.__isset.w = true;
            } else { 
              TProtocolUtil.skip(iprot, field.type);
            }
            break;
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();
    }

    public void write(TProtocol oprot) throws TException {
      TStruct struct = new TStruct("operate_args");
      oprot.writeStructBegin(struct);
      TField field = new TField();
      if (this.w != null) {
        field.name = "w";
        field.type = TType.STRUCT;
        field.id = 1;
        oprot.writeFieldBegin(field);
        this.w.write(oprot);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    public String toString() {
      StringBuilder sb = new StringBuilder("operate_args(");
      sb.append("w:");
      sb.append(this.w.toString());
      sb.append(")");
      return sb.toString();
    }

  }

  public static class operate_result implements TBase, java.io.Serializable   {
    public operate_result() {
    }

    public boolean equals(Object that) {
      if (that == null)
        return false;
      if (that instanceof operate_result)
        return this.equals((operate_result)that);
      return false;
    }

    public boolean equals(operate_result that) {
      if (that == null)
        return false;

      return true;
    }

    public int hashCode() {
      return 0;
    }

    public void read(TProtocol iprot) throws TException {
      TField field;
      iprot.readStructBegin();
      while (true)
      {
        field = iprot.readFieldBegin();
        if (field.type == TType.STOP) { 
          break;
        }
        switch (field.id)
        {
          default:
            TProtocolUtil.skip(iprot, field.type);
            break;
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();
    }

    public void write(TProtocol oprot) throws TException {
      TStruct struct = new TStruct("operate_result");
      oprot.writeStructBegin(struct);

      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

    public String toString() {
      StringBuilder sb = new StringBuilder("operate_result(");
      sb.append(")");
      return sb.toString();
    }

  }

}