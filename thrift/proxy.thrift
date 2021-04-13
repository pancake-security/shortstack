struct sequence_id {
  1: required i64 client_id,
  2: required i64 client_seq_no,
  3: required i64 server_seq_no,
  4: required i32 l1_idx,
  5: required i64 l1_seq_no,
}

service block_request_service {
  // Chain request (at non-head node)
  oneway void chain_request(1: sequence_id seq, 2: i32 block_id, 3: list<binary> arguments),

  // Management
  void setup_chain(1: i32 block_id, 2: string path, 6: list<string> chain, 7: i32 chain_role, 8: string next_block_id),
  void resend_pending(1: i32 block_id),
  void update_connections(1: i32 type, 2: i32 column, 3: string hostname, 4: i32 port, 5: i32 num_workers),
}

service block_response_service {
  // Chain acknowledgement (at non-head node)
  oneway void chain_ack(1: sequence_id seq),
}

service pancake_thrift extends block_request_service{
  i64 get_client_id();
  void register_client_id(1: i32 block_id, 2: i64 client_id);
  oneway void async_get(1:sequence_id seq_id, 2:string key);
  oneway void async_put(1:sequence_id seq_id, 2:string key, 3:string value);
  oneway void async_get_batch(1:sequence_id seq_id, 2:list<string> keys);
  oneway void async_put_batch(1:sequence_id seq_id, 2:list<string> keys, 3:list<string> values);
  string get(1:string key);
  void put(1:string key, 2:string value);
  list<string> get_batch(1:list<string> keys);
  void put_batch(1:list<string> keys, 2:list<string> values);
}

service pancake_thrift_response{
  oneway void async_response(1:sequence_id seq_id, 2:i32 op_code, 3:list<string>result);
}

service l2proxy extends block_request_service {
  oneway void l2request(1:sequence_id seq_id, 2:string key, 3:i32 replica, 4:string value);
}

service l3proxy {
  void register_client_id(1:i64 client_id);
  oneway void l3request(1:sequence_id seq_id, 2:string label, 3:string value, 4:bool is_read);
}
