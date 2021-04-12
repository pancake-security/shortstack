/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "block_response_service.h"




block_response_service_chain_ack_args::~block_response_service_chain_ack_args() throw() {
}


uint32_t block_response_service_chain_ack_args::read(::apache::thrift::protocol::TProtocol* iprot) {

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
          xfer += this->seq.read(iprot);
          this->__isset.seq = true;
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

uint32_t block_response_service_chain_ack_args::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("block_response_service_chain_ack_args");

  xfer += oprot->writeFieldBegin("seq", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->seq.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}


block_response_service_chain_ack_pargs::~block_response_service_chain_ack_pargs() throw() {
}


uint32_t block_response_service_chain_ack_pargs::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  ::apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("block_response_service_chain_ack_pargs");

  xfer += oprot->writeFieldBegin("seq", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += (*(this->seq)).write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void block_response_serviceClient::chain_ack(const sequence_id& seq)
{
  send_chain_ack(seq);
}

void block_response_serviceClient::send_chain_ack(const sequence_id& seq)
{
  int32_t cseqid = 0;
  oprot_->writeMessageBegin("chain_ack", ::apache::thrift::protocol::T_ONEWAY, cseqid);

  block_response_service_chain_ack_pargs args;
  args.seq = &seq;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();
}

bool block_response_serviceProcessor::dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext) {
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

void block_response_serviceProcessor::process_chain_ack(int32_t, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol*, void* callContext)
{
  void* ctx = NULL;
  if (this->eventHandler_.get() != NULL) {
    ctx = this->eventHandler_->getContext("block_response_service.chain_ack", callContext);
  }
  ::apache::thrift::TProcessorContextFreer freer(this->eventHandler_.get(), ctx, "block_response_service.chain_ack");

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->preRead(ctx, "block_response_service.chain_ack");
  }

  block_response_service_chain_ack_args args;
  args.read(iprot);
  iprot->readMessageEnd();
  uint32_t bytes = iprot->getTransport()->readEnd();

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->postRead(ctx, "block_response_service.chain_ack", bytes);
  }

  try {
    iface_->chain_ack(args.seq);
  } catch (const std::exception&) {
    if (this->eventHandler_.get() != NULL) {
      this->eventHandler_->handlerError(ctx, "block_response_service.chain_ack");
    }
    return;
  }

  if (this->eventHandler_.get() != NULL) {
    this->eventHandler_->asyncComplete(ctx, "block_response_service.chain_ack");
  }

  return;
}

::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > block_response_serviceProcessorFactory::getProcessor(const ::apache::thrift::TConnectionInfo& connInfo) {
  ::apache::thrift::ReleaseHandler< block_response_serviceIfFactory > cleanup(handlerFactory_);
  ::apache::thrift::stdcxx::shared_ptr< block_response_serviceIf > handler(handlerFactory_->getHandler(connInfo), cleanup);
  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > processor(new block_response_serviceProcessor(handler));
  return processor;
}

void block_response_serviceConcurrentClient::chain_ack(const sequence_id& seq)
{
  send_chain_ack(seq);
}

void block_response_serviceConcurrentClient::send_chain_ack(const sequence_id& seq)
{
  int32_t cseqid = 0;
  ::apache::thrift::async::TConcurrentSendSentry sentry(&this->sync_);
  oprot_->writeMessageBegin("chain_ack", ::apache::thrift::protocol::T_ONEWAY, cseqid);

  block_response_service_chain_ack_pargs args;
  args.seq = &seq;
  args.write(oprot_);

  oprot_->writeMessageEnd();
  oprot_->getTransport()->writeEnd();
  oprot_->getTransport()->flush();

  sentry.commit();
}


