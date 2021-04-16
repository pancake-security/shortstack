#include "chain_module.h"
#include <spdlog/spdlog.h>

// using namespace utils;

chain_module::chain_module()
    : next_(std::make_unique<next_chain_module_cxn>("nil")),
      prev_(std::make_unique<prev_chain_module_cxn>()),
      pending_(0) {}

chain_module::~chain_module() {
  next_->reset("nil");
  if (response_processor_.joinable())
    response_processor_.join();
}

void chain_module::setup(const std::string &path,
                         const std::vector<std::string> &chain,
                         chain_role role,
                         const std::string &next_block_id) {
//   path_ = path;
  chain_ = chain;
  role_ = role;
  auto protocol = next_->reset(next_block_id);
  if (protocol && role_ != chain_role::tail) {
    auto handler = std::make_shared<chain_response_handler>(this);
    auto processor = std::make_shared<block_response_serviceProcessor>(handler);
    if (response_processor_.joinable())
      response_processor_.join();
    response_processor_ = std::thread([processor, protocol] {
      while (true) {
        try {
          if (!processor->process(protocol, protocol, nullptr)) {
            break;
          }
        } catch (std::exception &e) {
          break;
        }
      }
    });
  }

  spdlog::info("chain_module setup, role: {}, next: {}", (int) role, next_block_id);
  
  setup_callback();
  
}

// TODO: resend pending requests in sequence number order
void chain_module::resend_pending() {
  auto ops = pending_.lock_table();
  try {
    for (const auto &op: ops) {
      if(is_tail()) {
        replication_complete(op.second.seq, op.second.args);
      } else {
        next_->request(op.second.seq, op.second.args);
      }
    }
  } catch (...) {
    ops.unlock();
    std::rethrow_exception(std::current_exception());
  }
  ops.unlock();
  spdlog::info("Resent pending requests");
}

void chain_module::ack(const sequence_id &seq) {
  remove_pending(seq);
  spdlog::debug("Ack, seq_no: {}, len(pending): {}", seq.server_seq_no, pending_.size());
  ack_callback(seq);
  if (!is_head()) {
    if (prev_ == nullptr) {
      spdlog::error("Invalid state: Previous is null");
    }
    prev_->ack(seq);
  }
}

void chain_module::chain_request(sequence_id seq, const arg_list &args) {
  
  if(is_head()) {
      seq.server_seq_no = chain_seq_no_ + 1;
  }

  if(seq.server_seq_no <= chain_seq_no_) {
      spdlog::info("Duplicate chain_request, server_seq_no: {}", seq.server_seq_no);
      return;
  }

  if(seq.server_seq_no > chain_seq_no_ + 1) {
      spdlog::error("Received out of order chain_request, server_seq_no: {}", seq.server_seq_no);
      throw std::logic_error("Received out of order chain_request");
      return;
  }

  run_command(seq, args);
  chain_seq_no_ += 1;
  add_pending(seq, args);

  if (is_tail()) {
    replication_complete(seq, args);
  } else {
    // Do not need a lock since this is the only thread handling chain requests
    next_->request(seq, args);
  }
}
