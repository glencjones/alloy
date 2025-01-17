/*
 * Copyright (c) 2017-2018, The Alloy Developers.
 * Portions Copyright (c) 2012-2017, The CryptoNote Developers, The Bytecoin Developers.
 *
 * This file is part of Alloy.
 *
 * This file is subject to the terms and conditions defined in the
 * file 'LICENSE', which is part of this source code package.
 */

#include "CachedTransaction.h"
#include <Common/Varint.h>
#include "Config/CryptoNoteConfig.h"
#include "CryptoNoteTools.h"

using namespace Crypto;
using namespace CryptoNote;

CachedTransaction::CachedTransaction(Transaction&& transaction) : transaction(std::move(transaction)) {
}

CachedTransaction::CachedTransaction(const Transaction& transaction) : transaction(transaction) {
}

CachedTransaction::CachedTransaction(const BinaryArray& transactionBinaryArray) : transactionBinaryArray(transactionBinaryArray) {
  if (!fromBinaryArray<Transaction>(transaction, this->transactionBinaryArray.get())) {
    throw std::runtime_error("CachedTransaction::CachedTransaction(BinaryArray&&), deserealization error.");
  }
}

const Transaction& CachedTransaction::getTransaction() const {
  return transaction;
}

const Crypto::Hash& CachedTransaction::getTransactionHash() const {
  if (!transactionHash.is_initialized()) {
    transactionHash = getBinaryArrayHash(getTransactionBinaryArray());
  }

  return transactionHash.get();
}

const Crypto::Hash& CachedTransaction::getTransactionPrefixHash() const {
  if (!transactionPrefixHash.is_initialized()) {
    transactionPrefixHash = getObjectHash(static_cast<const TransactionPrefix&>(transaction));
  }

  return transactionPrefixHash.get();
}

const BinaryArray& CachedTransaction::getTransactionBinaryArray() const {
  if (!transactionBinaryArray.is_initialized()) {
    transactionBinaryArray = toBinaryArray(transaction);
  }

  return transactionBinaryArray.get();
}

uint64_t CachedTransaction::getTransactionFee() const {
  if (!transactionFee.is_initialized()) {
    uint64_t summaryInputAmount = 0;
    uint64_t summaryOutputAmount = 0;
    for (auto& out : transaction.outputs) {
      summaryOutputAmount += out.amount;
    }

    for (auto& in : transaction.inputs) {
      if (in.type() == typeid(KeyInput)) {
        summaryInputAmount += boost::get<KeyInput>(in).amount;
      } else if (in.type() == typeid(BaseInput)) {
        return 0;
      } else {
        assert(false && "Unknown out type");
      }
    }

    transactionFee = summaryInputAmount - summaryOutputAmount;
  }

  return transactionFee.get();
}
