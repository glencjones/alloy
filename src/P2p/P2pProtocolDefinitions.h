/*
 * Copyright (c) 2017-2018, The Alloy Developers.
 * Portions Copyright (c) 2012-2017, The CryptoNote Developers, The Bytecoin Developers.
 *
 * This file is part of Alloy.
 *
 * This file is subject to the terms and conditions defined in the
 * file 'LICENSE', which is part of this source code package.
 */

#pragma once

#include "P2pProtocolTypes.h"

#include "Crypto/crypto.h"
#include "Config/CryptoNoteConfig.h"
#include "CryptoNoteCore/CoreStatistics.h"

// new serialization
#include "Serialization/ISerializer.h"
#include "Serialization/SerializationOverloads.h"
#include "CryptoNoteCore/CryptoNoteSerialization.h"

namespace CryptoNote
{
  inline bool serialize(uuid& v, Common::StringView name, ISerializer& s) {
    return s.binary(&v, sizeof(v), name);
  }

  struct network_config
  {
    void serialize(ISerializer& s) {
      KV_MEMBER(connections_count)
      KV_MEMBER(handshake_interval)
      KV_MEMBER(packet_max_size)
      KV_MEMBER(config_id)
    }

    uint32_t connections_count;
    uint32_t connection_timeout;
    uint32_t ping_connection_timeout;
    uint32_t handshake_interval;
    uint32_t packet_max_size;
    uint32_t config_id;
    uint32_t send_peerlist_sz;
  };

  enum P2PProtocolVersion : uint8_t {
    V0 = 0,
    V1 = 1,
    CURRENT = V1
  };

  struct basic_node_data
  {
    uuid network_id;
    uint8_t version;
    uint64_t local_time;
    uint32_t my_port;
    PeerIdType peer_id;

    void serialize(ISerializer& s) {
      KV_MEMBER(network_id)
      if (s.type() == ISerializer::INPUT) {
        version = 0;
      }
      KV_MEMBER(version)
      KV_MEMBER(peer_id)
      KV_MEMBER(local_time)
      KV_MEMBER(my_port)
    }
  };
  
  struct CORE_SYNC_DATA
  {
    uint32_t current_height;
    Crypto::Hash top_id;

    void serialize(ISerializer& s) {
      KV_MEMBER(current_height)
      KV_MEMBER(top_id)
    }
  };

#define P2P_COMMANDS_POOL_BASE 1000

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  struct COMMAND_HANDSHAKE
  {
    enum { ID = P2P_COMMANDS_POOL_BASE + 1 };

    struct request
    {
      basic_node_data node_data;
      CORE_SYNC_DATA payload_data;

      void serialize(ISerializer& s) {
        KV_MEMBER(node_data)
        KV_MEMBER(payload_data)
      }

    };

    struct response
    {
      basic_node_data node_data;
      CORE_SYNC_DATA payload_data;
      std::list<PeerlistEntry> local_peerlist; 

      void serialize(ISerializer& s) {
        KV_MEMBER(node_data)
        KV_MEMBER(payload_data)
        serializeAsBinary(local_peerlist, "local_peerlist", s);
      }
    };
  };


  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  struct COMMAND_TIMED_SYNC
  {
    enum { ID = P2P_COMMANDS_POOL_BASE + 2 };

    struct request
    {
      CORE_SYNC_DATA payload_data;

      void serialize(ISerializer& s) {
        KV_MEMBER(payload_data)
      }

    };

    struct response
    {
      uint64_t local_time;
      CORE_SYNC_DATA payload_data;
      std::list<PeerlistEntry> local_peerlist;

      void serialize(ISerializer& s) {
        KV_MEMBER(local_time)
        KV_MEMBER(payload_data)
        serializeAsBinary(local_peerlist, "local_peerlist", s);
      }
    };
  };

  /************************************************************************/
  /*                                                                      */
  /************************************************************************/

  struct COMMAND_PING
  {
    /*
      Used to make "callback" connection, to be sure that opponent node 
      have accessible connection point. Only other nodes can add peer to peerlist,
      and ONLY in case when peer has accepted connection and answered to ping.
    */
    enum { ID = P2P_COMMANDS_POOL_BASE + 3 };

#define PING_OK_RESPONSE_STATUS_TEXT "OK"

    struct request
    {
      /*actually we don't need to send any real data*/
      void serialize(ISerializer& s) {}
    };

    struct response
    {
      std::string status;
      PeerIdType peer_id;

      void serialize(ISerializer& s) {
        KV_MEMBER(status)
        KV_MEMBER(peer_id)
      }
    };
  };

  


}
