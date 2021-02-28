/**
 * Autogenerated by Thrift Compiler (0.12.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef l2proxy_H
#define l2proxy_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "proxy_types.h"



#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class l2proxyIf {
 public:
  virtual ~l2proxyIf() {}
  virtual void l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value) = 0;
};

class l2proxyIfFactory {
 public:
  typedef l2proxyIf Handler;

  virtual ~l2proxyIfFactory() {}

  virtual l2proxyIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(l2proxyIf* /* handler */) = 0;
};

class l2proxyIfSingletonFactory : virtual public l2proxyIfFactory {
 public:
  l2proxyIfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<l2proxyIf>& iface) : iface_(iface) {}
  virtual ~l2proxyIfSingletonFactory() {}

  virtual l2proxyIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(l2proxyIf* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<l2proxyIf> iface_;
};

class l2proxyNull : virtual public l2proxyIf {
 public:
  virtual ~l2proxyNull() {}
  void l2request(const sequence_id& /* seq_id */, const std::string& /* key */, const int32_t /* replica */, const std::string& /* value */) {
    return;
  }
};

typedef struct _l2proxy_l2request_args__isset {
  _l2proxy_l2request_args__isset() : seq_id(false), key(false), replica(false), value(false) {}
  bool seq_id :1;
  bool key :1;
  bool replica :1;
  bool value :1;
} _l2proxy_l2request_args__isset;

class l2proxy_l2request_args {
 public:

  l2proxy_l2request_args(const l2proxy_l2request_args&);
  l2proxy_l2request_args& operator=(const l2proxy_l2request_args&);
  l2proxy_l2request_args() : key(), replica(0), value() {
  }

  virtual ~l2proxy_l2request_args() throw();
  sequence_id seq_id;
  std::string key;
  int32_t replica;
  std::string value;

  _l2proxy_l2request_args__isset __isset;

  void __set_seq_id(const sequence_id& val);

  void __set_key(const std::string& val);

  void __set_replica(const int32_t val);

  void __set_value(const std::string& val);

  bool operator == (const l2proxy_l2request_args & rhs) const
  {
    if (!(seq_id == rhs.seq_id))
      return false;
    if (!(key == rhs.key))
      return false;
    if (!(replica == rhs.replica))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const l2proxy_l2request_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const l2proxy_l2request_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class l2proxy_l2request_pargs {
 public:


  virtual ~l2proxy_l2request_pargs() throw();
  const sequence_id* seq_id;
  const std::string* key;
  const int32_t* replica;
  const std::string* value;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class l2proxyClient : virtual public l2proxyIf {
 public:
  l2proxyClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  l2proxyClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value);
  void send_l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value);
 protected:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class l2proxyProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<l2proxyIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (l2proxyProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_l2request(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  l2proxyProcessor(::apache::thrift::stdcxx::shared_ptr<l2proxyIf> iface) :
    iface_(iface) {
    processMap_["l2request"] = &l2proxyProcessor::process_l2request;
  }

  virtual ~l2proxyProcessor() {}
};

class l2proxyProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  l2proxyProcessorFactory(const ::apache::thrift::stdcxx::shared_ptr< l2proxyIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< l2proxyIfFactory > handlerFactory_;
};

class l2proxyMultiface : virtual public l2proxyIf {
 public:
  l2proxyMultiface(std::vector<apache::thrift::stdcxx::shared_ptr<l2proxyIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~l2proxyMultiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<l2proxyIf> > ifaces_;
  l2proxyMultiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<l2proxyIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->l2request(seq_id, key, replica, value);
    }
    ifaces_[i]->l2request(seq_id, key, replica, value);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class l2proxyConcurrentClient : virtual public l2proxyIf {
 public:
  l2proxyConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  l2proxyConcurrentClient(apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
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
  void l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value);
  void send_l2request(const sequence_id& seq_id, const std::string& key, const int32_t replica, const std::string& value);
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