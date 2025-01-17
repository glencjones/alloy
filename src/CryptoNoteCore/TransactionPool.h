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
#include <unordered_map>

#include "Crypto/crypto.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "ITransactionPool.h"
#include <Logging/LoggerMessage.h>
#include <Logging/LoggerRef.h>
#include "TransactionValidatiorState.h"

namespace CryptoNote {

class TransactionPool : public ITransactionPool {
public:
  TransactionPool(Logging::ILogger& logger);

  virtual bool pushTransaction(CachedTransaction&& transaction, TransactionValidatorState&& transactionState) override;
  virtual const CachedTransaction& getTransaction(const Crypto::Hash& hash) const override;
  virtual bool removeTransaction(const Crypto::Hash& hash) override;

  virtual size_t getTransactionCount() const override;
  virtual std::vector<Crypto::Hash> getTransactionHashes() const override;
  virtual bool checkIfTransactionPresent(const Crypto::Hash& hash) const override;

  virtual const TransactionValidatorState& getPoolTransactionValidationState() const override;
  virtual std::vector<CachedTransaction> getPoolTransactions() const override;

  virtual uint64_t getTransactionReceiveTime(const Crypto::Hash& hash) const override;
  virtual std::vector<Crypto::Hash> getTransactionHashesByPaymentId(const Crypto::Hash& paymentId) const override;
private:
  TransactionValidatorState poolState;

  struct PendingTransactionInfo {
    uint64_t receiveTime;
    CachedTransaction cachedTransaction;
    boost::optional<Crypto::Hash> paymentId;

    const Crypto::Hash& getTransactionHash() const;
  };

  struct TransactionPriorityComparator {
    // lhs > hrs
    bool operator()(const PendingTransactionInfo& lhs, const PendingTransactionInfo& rhs) const;
  };

  struct TransactionHashTag {};
  struct TransactionCostTag {};
  struct PaymentIdTag {};

  typedef boost::multi_index::ordered_non_unique<
    boost::multi_index::tag<TransactionCostTag>,
    boost::multi_index::identity<PendingTransactionInfo>,
    TransactionPriorityComparator
  > TransactionCostIndex;

  typedef boost::multi_index::hashed_unique<
    boost::multi_index::tag<TransactionHashTag>,
    boost::multi_index::const_mem_fun<
      PendingTransactionInfo,
      const Crypto::Hash&,
      &PendingTransactionInfo::getTransactionHash
    >
  > TransactionHashIndex;

  struct PaymentIdHasher {
    size_t operator() (const boost::optional<Crypto::Hash>& paymentId) const;
  };

  typedef boost::multi_index::hashed_non_unique<
    boost::multi_index::tag<PaymentIdTag>,
    BOOST_MULTI_INDEX_MEMBER(PendingTransactionInfo, boost::optional<Crypto::Hash>, paymentId),
    PaymentIdHasher
  > PaymentIdIndex;

  typedef boost::multi_index_container<
    PendingTransactionInfo,
    boost::multi_index::indexed_by<
      TransactionHashIndex,
      TransactionCostIndex,
      PaymentIdIndex
    >
  > TransactionsContainer;

  TransactionsContainer transactions;
  TransactionsContainer::index<TransactionHashTag>::type& transactionHashIndex;
  TransactionsContainer::index<TransactionCostTag>::type& transactionCostIndex;
  TransactionsContainer::index<PaymentIdTag>::type& paymentIdIndex;
  
  Logging::LoggerRef logger;
};

}
