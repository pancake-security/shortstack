/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "pancake_thrift_response.h"




pancake_thrift_response_async_response_args::~pancake_thrift_response_async_response_args() throw() {
}


uint32_t pancake_thrift_response_async_response_args::read(::apache::thrift::protocol::TProtocol* iprot) {

  ::apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->seq_id.read(iprot);
          this->__isset.seq_id = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->op_code);
          this->__isset.op_code = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->result.clear();
            uint32_t _size69;
            ::apache::thrift::protocol::TType _etype72;
            xfer += iprot->readListBegin(_etype72, _size69);
            this->result.resize(_size69);
            uint32_t _i73;
            for (_i73 = 0; _i73 < _size69; ++_i73)
            {
              xfer += iprot->readString(this->result[_i73]);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.result = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t pancake_thrift_response_async_response_args::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("pancake_thrift_response_async_response_args");

  xfer += oprot->writeFieldBegin("seq_id", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->seq_id.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("op_code", ::apache::thrift::protocol::T_I32, 2);
  xfer += oprot->writeI32(this->op_code);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("result", ::apache::thrift::protocol::T_LIST, 3);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->result.size()));
    std::vector<std::string> ::const_iterator _iter74;
    for (_iter74 = this->result.begin(); _iter74 != this->result.end(); ++_iter74)
    {
      xfer += oprot->writeString((*_iter74));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}


pancake_thrift_response_async_response_pargs::~pancake_thrift_response_async_response_pargs() throw() {
}


uint32_t pancake_thrift_response_async_response_pargs::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("pancake_thrift_response_async_response_pargs");

  xfer += oprot->writeFieldBegin("seq_id", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += (*(this->seq_id)).write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("op_code", ::apache::thrift::protocol::T_I32, 2);
  xfer += oprot->writeI32((*(this->op_code)));
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("result", ::apache::thrift::protocol::T_LIST, 3);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>((*(this->result)).size()));
    std::vector<std::string> ::const_iterator _iter75;
    for (_iter75 = (*(this->result)).begin(); _iter75 != (*(this->result)).end(); ++_iter75)
    {
      xfer += oprot->writeString((*_iter75));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void pancake_thrift_responseClient::async_response(const sequence_id& seq_id, const int32_t op_code, const std::vector<std::string> & result)
{
  send_async_response(seq_id, op_code, result);
}

void pancake_thrift_responseClient::send_async_response(const sequence_id& seq_id, const int32_t op_code, const std::vector<std::string> & result)
{
  int32_t cseqid = 0;
  oprot_->writeMessageBegin("async_response", ::apache::thrift::protocol::T_ONEWAY, cseqid);

  pancake_thrift_response_async_response_pargs args;
  args.seq_id = &seq_id;
  args.op_code = &op_code;
  args.result = &result;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();
}

bool pancake_thrift_responseProcessor::dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext) {
  ProcessMap::iterator pfn;
  pfn = processMap_.find(fname);
  if (pfn == processMap_.end()) {
    iprot->skip(::apache::thrift::protocol::T_STRUCT);
    iprot->readMessageEnd();
    iprot->getTransport()->readEnd();
    ::apache::thrift::TApplicationException x(::apache::thrift::TApplicationException::UNKNOWN_METHOD, "Invalid method name: '"+fname+"'");
    oprot->writeMessageBegin(fname, ::apache::thrift::protocol::T_EXCEPTION, seqid);
    x.write(oprot);
    oprot->writeMessageEnd();
    oprot->getTransport()->writeEnd();
    oprot->getTransport()->flush();
    return true;
  }
  (this->*(pfn->second))(seqid, iprot, oprot, callContext);
  return true;
}

void pancake_thrift_responseProcessor::process_async_response(int32_t, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol*, void* callContext)
{
  void* ctx = NULL;
  if (this->eventHandler_.get() != NULL) {
    ctx = this->eventHandler_->getContext("pancake_thrift_response.async_response", callContext);
  }
  ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "pancake_thrift_response.async_response");

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preRead(ctx, "pancake_thrift_response.async_response");
  }

  pancake_thrift_response_async_response_args args;
  args.read(iprot);
  iprot->readMessageEnd();
  uint32_t bytes = iprot->getTransport()->readEnd();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postRead(ctx, "pancake_thrift_response.async_response", bytes);
  }

  try {
    iface_->async_response(args.seq_id, args.op_code, args.result);
  } catch (const std::exception&) {
    if (this->eventHandler_.get() != NULL) {
      this->eventHandler_->handlerError(ctx, "pancake_thrift_response.async_response");
    }
    return;
  }

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->asyncComplete(ctx, "pancake_thrift_response.async_response");
  }

  return;
}

::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > pancake_thrift_responseProcessorFactory::getProcessor(const ::apache::thrift::TConnectionInfo& connInfo) {
  ::apache::thrift::ReleaseHandler< pancake_thrift_responseIfFactory > cleanup(handlerFactory_);
  ::apache::thrift::stdcxx::shared_ptr< pancake_thrift_responseIf > handler(handlerFactory_->getHandler(connInfo), cleanup);
  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > processor(new pancake_thrift_responseProcessor(handler));
  return processor;
}

void pancake_thrift_responseConcurrentClient::async_response(const sequence_id& seq_id, const int32_t op_code, const std::vector<std::string> & result)
{
  send_async_response(seq_id, op_code, result);
}

void pancake_thrift_responseConcurrentClient::send_async_response(const sequence_id& seq_id, const int32_t op_code, const std::vector<std::string> & result)
{
  int32_t cseqid = 0;
  ::apache::thrift::async::TConcurrentSendSentry sentry(&this->sync_);
  oprot_->writeMessageBegin("async_response", ::apache::thrift::protocol::T_ONEWAY, cseqid);

  pancake_thrift_response_async_response_pargs args;
  args.seq_id = &seq_id;
  args.op_code = &op_code;
  args.result = &result;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();

  sentry.commit();
}



