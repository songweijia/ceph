#ifndef CEPH_WAN_AGENT_H
#define CEPH_WAN_AGENT_H

#include "mon/MonClient.h"
#include "msg/Dispatcher.h"

#define CEPH_WANA_PROTOCOL    (91) /* cluster internal */

class Messenger;
class Message;
class MOSDOp;
class MonClient;
class WANAgent;

class WANAgent : public Dispatcher {

public:
  // constructor
  WANAgent(
    CephContext *cct_,	            // ceph context
    Messenger *ms_near,             // receiving updates from local OSDs, for 
                                    // broadcasting to remote wan agents.
    Messenger *ms_far,              // receiving updates from remote wan agents, for
                                    // replay in the local data centers.
    Messenger *ms_objecter,         // send updates to local data center.
    MonClient *mc                   // Monitor client
  ): Dispatcher(cct_),
  far_messenger(ms_far),
  near_messenger(ms_near),
  objecter_messenger(ms_objecter),
  mon_client(mc) {}
  // destructor
  virtual ~WANAgent();
  // dispatcher methods
  bool ms_can_fast_dispatch_any() const override { return true; }
  bool ms_can_fast_dispatch(const Message *m) const override { return false;}
  void ms_fast_dispatch(Message *m) override;
  void ms_fast_preprocess(Message *m) override {}
  bool ms_dispatch(Message *m) override;
  void ms_handle_connect(Connection *con) override {}
  void ms_handle_fast_connect(Connection *con) override {}
  void ms_handle_accept(Connection *con) override {}
  void ms_handle_fast_accept(Connection *con) override {}
  bool ms_handle_reset(Connection *con) override {return false;}
  void ms_handle_remote_reset(Connection *con) override {}
  bool ms_handle_refused(Connection *con) override {return false;}
  bool ms_get_authorizer(int dest_type, AuthAuthorizer **a, bool force_new) override { return false;}
  bool ms_verify_authorizer(Connection *con,
                            int peer_type,
                            int protocol,
                            ceph::bufferlist& authorizer,
                            ceph::bufferlist& authorizer_reply,
                            bool& isvalid,
                            CryptoKey& session_key) override { return false; }
  //signal handler
  void handle_signal(int signum);
  
protected:
  /*
    TODO:STEP 2, implementing the client connecting to the remote data center.
  class RemoteClient : public Dispatcher {
  };
  */ 
  Messenger * far_messenger;
  Messenger * near_messenger;
  Messenger * objecter_messenger;
  MonClient * mon_client;
};

#endif//CEPH_WAN_AGENT_H
