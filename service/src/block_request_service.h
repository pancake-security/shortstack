/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef block_request_service_H
#define block_request_service_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "proxy_types.h"



#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class block_request_serviceIf {
 public:
  virtual ~block_request_serviceIf() {}
  virtual void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments) = 0;
  virtual void external_ack(const sequence_id& seq) = 0;
  virtual void external_ack_batch(const std::vector<sequence_id> & seqs) = 0;
  virtual void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id) = 0;
  virtual void resend_pending(const int32_t block_id) = 0;
  virtual void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers) = 0;
};

class block_request_serviceIfFactory {
 public:
  typedef block_request_serviceIf Handler;

  virtual ~block_request_serviceIfFactory() {}

  virtual block_request_serviceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(block_request_serviceIf* /* handler */) = 0;
};

class block_request_serviceIfSingletonFactory : virtual public block_request_serviceIfFactory {
 public:
  block_request_serviceIfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<block_request_serviceIf>& iface) : iface_(iface) {}
  virtual ~block_request_serviceIfSingletonFactory() {}

  virtual block_request_serviceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(block_request_serviceIf* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> iface_;
};

class block_request_serviceNull : virtual public block_request_serviceIf {
 public:
  virtual ~block_request_serviceNull() {}
  void chain_request(const sequence_id& /* seq */, const int32_t /* block_id */, const std::vector<std::string> & /* arguments */) {
    return;
  }
  void external_ack(const sequence_id& /* seq */) {
    return;
  }
  void external_ack_batch(const std::vector<sequence_id> & /* seqs */) {
    return;
  }
  void setup_chain(const int32_t /* block_id */, const std::string& /* path */, const std::vector<std::string> & /* chain */, const int32_t /* chain_role */, const std::string& /* next_block_id */) {
    return;
  }
  void resend_pending(const int32_t /* block_id */) {
    return;
  }
  void update_connections(const int32_t /* type */, const int32_t /* column */, const std::string& /* hostname */, const int32_t /* port */, const int32_t /* num_workers */) {
    return;
  }
};

typedef struct _block_request_service_chain_request_args__isset {
  _block_request_service_chain_request_args__isset() : seq(false), block_id(false), arguments(false) {}
  bool seq :1;
  bool block_id :1;
  bool arguments :1;
} _block_request_service_chain_request_args__isset;

class block_request_service_chain_request_args {
 public:

  block_request_service_chain_request_args(const block_request_service_chain_request_args&);
  block_request_service_chain_request_args& operator=(const block_request_service_chain_request_args&);
  block_request_service_chain_request_args() : block_id(0) {
  }

  virtual ~block_request_service_chain_request_args() throw();
  sequence_id seq;
  int32_t block_id;
  std::vector<std::string>  arguments;

  _block_request_service_chain_request_args__isset __isset;

  void __set_seq(const sequence_id& val);

  void __set_block_id(const int32_t val);

  void __set_arguments(const std::vector<std::string> & val);

  bool operator == (const block_request_service_chain_request_args & rhs) const
  {
    if (!(seq == rhs.seq))
      return false;
    if (!(block_id == rhs.block_id))
      return false;
    if (!(arguments == rhs.arguments))
      return false;
    return true;
  }
  bool operator != (const block_request_service_chain_request_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_chain_request_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_chain_request_pargs {
 public:


  virtual ~block_request_service_chain_request_pargs() throw();
  const sequence_id* seq;
  const int32_t* block_id;
  const std::vector<std::string> * arguments;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _block_request_service_external_ack_args__isset {
  _block_request_service_external_ack_args__isset() : seq(false) {}
  bool seq :1;
} _block_request_service_external_ack_args__isset;

class block_request_service_external_ack_args {
 public:

  block_request_service_external_ack_args(const block_request_service_external_ack_args&);
  block_request_service_external_ack_args& operator=(const block_request_service_external_ack_args&);
  block_request_service_external_ack_args() {
  }

  virtual ~block_request_service_external_ack_args() throw();
  sequence_id seq;

  _block_request_service_external_ack_args__isset __isset;

  void __set_seq(const sequence_id& val);

  bool operator == (const block_request_service_external_ack_args & rhs) const
  {
    if (!(seq == rhs.seq))
      return false;
    return true;
  }
  bool operator != (const block_request_service_external_ack_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_external_ack_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_external_ack_pargs {
 public:


  virtual ~block_request_service_external_ack_pargs() throw();
  const sequence_id* seq;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _block_request_service_external_ack_batch_args__isset {
  _block_request_service_external_ack_batch_args__isset() : seqs(false) {}
  bool seqs :1;
} _block_request_service_external_ack_batch_args__isset;

class block_request_service_external_ack_batch_args {
 public:

  block_request_service_external_ack_batch_args(const block_request_service_external_ack_batch_args&);
  block_request_service_external_ack_batch_args& operator=(const block_request_service_external_ack_batch_args&);
  block_request_service_external_ack_batch_args() {
  }

  virtual ~block_request_service_external_ack_batch_args() throw();
  std::vector<sequence_id>  seqs;

  _block_request_service_external_ack_batch_args__isset __isset;

  void __set_seqs(const std::vector<sequence_id> & val);

  bool operator == (const block_request_service_external_ack_batch_args & rhs) const
  {
    if (!(seqs == rhs.seqs))
      return false;
    return true;
  }
  bool operator != (const block_request_service_external_ack_batch_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_external_ack_batch_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_external_ack_batch_pargs {
 public:


  virtual ~block_request_service_external_ack_batch_pargs() throw();
  const std::vector<sequence_id> * seqs;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _block_request_service_setup_chain_args__isset {
  _block_request_service_setup_chain_args__isset() : block_id(false), path(false), chain(false), chain_role(false), next_block_id(false) {}
  bool block_id :1;
  bool path :1;
  bool chain :1;
  bool chain_role :1;
  bool next_block_id :1;
} _block_request_service_setup_chain_args__isset;

class block_request_service_setup_chain_args {
 public:

  block_request_service_setup_chain_args(const block_request_service_setup_chain_args&);
  block_request_service_setup_chain_args& operator=(const block_request_service_setup_chain_args&);
  block_request_service_setup_chain_args() : block_id(0), path(), chain_role(0), next_block_id() {
  }

  virtual ~block_request_service_setup_chain_args() throw();
  int32_t block_id;
  std::string path;
  std::vector<std::string>  chain;
  int32_t chain_role;
  std::string next_block_id;

  _block_request_service_setup_chain_args__isset __isset;

  void __set_block_id(const int32_t val);

  void __set_path(const std::string& val);

  void __set_chain(const std::vector<std::string> & val);

  void __set_chain_role(const int32_t val);

  void __set_next_block_id(const std::string& val);

  bool operator == (const block_request_service_setup_chain_args & rhs) const
  {
    if (!(block_id == rhs.block_id))
      return false;
    if (!(path == rhs.path))
      return false;
    if (!(chain == rhs.chain))
      return false;
    if (!(chain_role == rhs.chain_role))
      return false;
    if (!(next_block_id == rhs.next_block_id))
      return false;
    return true;
  }
  bool operator != (const block_request_service_setup_chain_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_setup_chain_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_setup_chain_pargs {
 public:


  virtual ~block_request_service_setup_chain_pargs() throw();
  const int32_t* block_id;
  const std::string* path;
  const std::vector<std::string> * chain;
  const int32_t* chain_role;
  const std::string* next_block_id;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_setup_chain_result {
 public:

  block_request_service_setup_chain_result(const block_request_service_setup_chain_result&);
  block_request_service_setup_chain_result& operator=(const block_request_service_setup_chain_result&);
  block_request_service_setup_chain_result() {
  }

  virtual ~block_request_service_setup_chain_result() throw();

  bool operator == (const block_request_service_setup_chain_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const block_request_service_setup_chain_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_setup_chain_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_setup_chain_presult {
 public:


  virtual ~block_request_service_setup_chain_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _block_request_service_resend_pending_args__isset {
  _block_request_service_resend_pending_args__isset() : block_id(false) {}
  bool block_id :1;
} _block_request_service_resend_pending_args__isset;

class block_request_service_resend_pending_args {
 public:

  block_request_service_resend_pending_args(const block_request_service_resend_pending_args&);
  block_request_service_resend_pending_args& operator=(const block_request_service_resend_pending_args&);
  block_request_service_resend_pending_args() : block_id(0) {
  }

  virtual ~block_request_service_resend_pending_args() throw();
  int32_t block_id;

  _block_request_service_resend_pending_args__isset __isset;

  void __set_block_id(const int32_t val);

  bool operator == (const block_request_service_resend_pending_args & rhs) const
  {
    if (!(block_id == rhs.block_id))
      return false;
    return true;
  }
  bool operator != (const block_request_service_resend_pending_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_resend_pending_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_resend_pending_pargs {
 public:


  virtual ~block_request_service_resend_pending_pargs() throw();
  const int32_t* block_id;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_resend_pending_result {
 public:

  block_request_service_resend_pending_result(const block_request_service_resend_pending_result&);
  block_request_service_resend_pending_result& operator=(const block_request_service_resend_pending_result&);
  block_request_service_resend_pending_result() {
  }

  virtual ~block_request_service_resend_pending_result() throw();

  bool operator == (const block_request_service_resend_pending_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const block_request_service_resend_pending_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_resend_pending_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_resend_pending_presult {
 public:


  virtual ~block_request_service_resend_pending_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _block_request_service_update_connections_args__isset {
  _block_request_service_update_connections_args__isset() : type(false), column(false), hostname(false), port(false), num_workers(false) {}
  bool type :1;
  bool column :1;
  bool hostname :1;
  bool port :1;
  bool num_workers :1;
} _block_request_service_update_connections_args__isset;

class block_request_service_update_connections_args {
 public:

  block_request_service_update_connections_args(const block_request_service_update_connections_args&);
  block_request_service_update_connections_args& operator=(const block_request_service_update_connections_args&);
  block_request_service_update_connections_args() : type(0), column(0), hostname(), port(0), num_workers(0) {
  }

  virtual ~block_request_service_update_connections_args() throw();
  int32_t type;
  int32_t column;
  std::string hostname;
  int32_t port;
  int32_t num_workers;

  _block_request_service_update_connections_args__isset __isset;

  void __set_type(const int32_t val);

  void __set_column(const int32_t val);

  void __set_hostname(const std::string& val);

  void __set_port(const int32_t val);

  void __set_num_workers(const int32_t val);

  bool operator == (const block_request_service_update_connections_args & rhs) const
  {
    if (!(type == rhs.type))
      return false;
    if (!(column == rhs.column))
      return false;
    if (!(hostname == rhs.hostname))
      return false;
    if (!(port == rhs.port))
      return false;
    if (!(num_workers == rhs.num_workers))
      return false;
    return true;
  }
  bool operator != (const block_request_service_update_connections_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_update_connections_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_update_connections_pargs {
 public:


  virtual ~block_request_service_update_connections_pargs() throw();
  const int32_t* type;
  const int32_t* column;
  const std::string* hostname;
  const int32_t* port;
  const int32_t* num_workers;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_update_connections_result {
 public:

  block_request_service_update_connections_result(const block_request_service_update_connections_result&);
  block_request_service_update_connections_result& operator=(const block_request_service_update_connections_result&);
  block_request_service_update_connections_result() {
  }

  virtual ~block_request_service_update_connections_result() throw();

  bool operator == (const block_request_service_update_connections_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const block_request_service_update_connections_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const block_request_service_update_connections_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class block_request_service_update_connections_presult {
 public:


  virtual ~block_request_service_update_connections_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class block_request_serviceClient : virtual public block_request_serviceIf {
 public:
  block_request_serviceClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  block_request_serviceClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);
  void send_chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);
  void external_ack(const sequence_id& seq);
  void send_external_ack(const sequence_id& seq);
  void external_ack_batch(const std::vector<sequence_id> & seqs);
  void send_external_ack_batch(const std::vector<sequence_id> & seqs);
  void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id);
  void send_setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id);
  void recv_setup_chain();
  void resend_pending(const int32_t block_id);
  void send_resend_pending(const int32_t block_id);
  void recv_resend_pending();
  void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);
  void send_update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);
  void recv_update_connections();
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class block_request_serviceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (block_request_serviceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_chain_request(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_external_ack(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_external_ack_batch(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_setup_chain(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_resend_pending(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_update_connections(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  block_request_serviceProcessor(::apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> iface) :
    iface_(iface) {
    processMap_["chain_request"] = &block_request_serviceProcessor::process_chain_request;
    processMap_["external_ack"] = &block_request_serviceProcessor::process_external_ack;
    processMap_["external_ack_batch"] = &block_request_serviceProcessor::process_external_ack_batch;
    processMap_["setup_chain"] = &block_request_serviceProcessor::process_setup_chain;
    processMap_["resend_pending"] = &block_request_serviceProcessor::process_resend_pending;
    processMap_["update_connections"] = &block_request_serviceProcessor::process_update_connections;
  }

  virtual ~block_request_serviceProcessor() {}
};

class block_request_serviceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  block_request_serviceProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< block_request_serviceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< block_request_serviceIfFactory > handlerFactory_;
};

class block_request_serviceMultiface : virtual public block_request_serviceIf {
 public:
  block_request_serviceMultiface(std::vector<apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~block_request_serviceMultiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> > ifaces_;
  block_request_serviceMultiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<block_request_serviceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->chain_request(seq, block_id, arguments);
    }
    ifaces_[i]->chain_request(seq, block_id, arguments);
  }

  void external_ack(const sequence_id& seq) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->external_ack(seq);
    }
    ifaces_[i]->external_ack(seq);
  }

  void external_ack_batch(const std::vector<sequence_id> & seqs) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->external_ack_batch(seqs);
    }
    ifaces_[i]->external_ack_batch(seqs);
  }

  void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->setup_chain(block_id, path, chain, chain_role, next_block_id);
    }
    ifaces_[i]->setup_chain(block_id, path, chain, chain_role, next_block_id);
  }

  void resend_pending(const int32_t block_id) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->resend_pending(block_id);
    }
    ifaces_[i]->resend_pending(block_id);
  }

  void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->update_connections(type, column, hostname, port, num_workers);
    }
    ifaces_[i]->update_connections(type, column, hostname, port, num_workers);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class block_request_serviceConcurrentClient : virtual public block_request_serviceIf {
 public:
  block_request_serviceConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  block_request_serviceConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);
  void send_chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);
  void external_ack(const sequence_id& seq);
  void send_external_ack(const sequence_id& seq);
  void external_ack_batch(const std::vector<sequence_id> & seqs);
  void send_external_ack_batch(const std::vector<sequence_id> & seqs);
  void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id);
  int32_t send_setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t chain_role, const std::string& next_block_id);
  void recv_setup_chain(const int32_t seqid);
  void resend_pending(const int32_t block_id);
  int32_t send_resend_pending(const int32_t block_id);
  void recv_resend_pending(const int32_t seqid);
  void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);
  int32_t send_update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);
  void recv_update_connections(const int32_t seqid);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _MSC_VER
  #pragma warning( pop )
#endif



#endif
