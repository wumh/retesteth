#pragma once
#include "../../basetypes.h"
#include <libdevcore/RLP.h>
#include <retesteth/dataObject/DataObject.h>
#include <retesteth/dataObject/SPointer.h>
using namespace dataobject;
using namespace test::teststruct;

namespace test
{
namespace teststruct
{
// Ethereum blockheader description
struct BlockHeader : GCP_SPointerBase
{
    BlockHeader(DataObject const&);
    BlockHeader(dev::RLP const&);

    DataObject const asDataObject() const;
    dev::RLPStream const asRLPStream() const;
    bool operator==(BlockHeader const& _rhs) const;
    bool operator!=(BlockHeader const& _rhs) const { return !(*this == _rhs); };
    bool hasUncles() const;

    FH32 const& stateRoot() const { return m_stateRoot.getCContent(); }
    FH32 const& receiptTrie() const { return m_receiptsRoot.getCContent(); }
    FH32 const& transactionRoot() const { return m_transactionsRoot.getCContent(); }
    FH32 const& uncleHash() const { return m_sha3Uncles.getCContent(); }
    FH32 const& hash() const { return m_hash.getCContent(); }
    VALUE const& number() const { return m_number.getCContent(); }
    VALUE const& timestamp() const { return m_timestamp.getCContent(); }
    FH20 const& author() const { return m_author.getCContent(); }
    VALUE const& difficulty() const { return m_difficulty.getCContent(); }
    VALUE const& gasLimit() const { return m_gasLimit.getCContent(); }
    VALUE const& gasUsed() const { return m_gasUsed.getCContent(); }
    BYTES const& extraData() const { return m_extraData.getCContent(); }
    FH8 const& nonce() const { return m_nonce.getCContent(); }
    FH32 const& mixHash() const { return m_mixHash.getCContent(); }
    FH32 const& parentHash() const { return m_parentHash.getCContent(); }

    void setLogsBloom(FH256 const& _logs) { m_logsBloom = spFH256(new FH256(_logs)); }
    void setTimestamp(VALUE const& _value) { m_timestamp = spVALUE(new VALUE(_value.asU256())); }
    void setDifficulty(VALUE const& _value) { m_difficulty = spVALUE(new VALUE(_value.asU256())); }
    void setTransactionHash(FH32 const& _hash) { m_transactionsRoot = spFH32(new FH32(_hash)); }
    void setTrReceiptsHash(FH32 const& _hash) { m_receiptsRoot = spFH32(new FH32(_hash)); }
    void setStateRoot(FH32 const& _hash) { m_stateRoot = spFH32(new FH32(_hash)); }
    void setNumber(VALUE const& _number) { m_number = spVALUE(new VALUE(_number)); }
    void setExtraData(BYTES const& _extraData) { m_extraData = spBYTES(new BYTES(_extraData)); }
    void setParentHash(FH32 const& _hash) { m_parentHash = spFH32(new FH32(_hash)); }
    void setUnclesHash(FH32 const& _hash) { m_sha3Uncles = spFH32(new FH32(_hash)); }
    void setHeaderHash(FH32 const& _hash) { m_hash = spFH32(new FH32(_hash)); }
    void setGasUsed(VALUE const& _gasUsed) { m_gasUsed = spVALUE(new VALUE(_gasUsed)); }
    void setGasLimit(VALUE const& _gasLimit) { m_gasLimit = spVALUE(new VALUE(_gasLimit)); }
    void recalculateHash();

private:
    BlockHeader() {}
    void fromData(DataObject const&);
    spFH20 m_author;
    spVALUE m_difficulty;
    spBYTES m_extraData;
    spVALUE m_gasLimit;
    spVALUE m_gasUsed;
    spVALUE m_pstotal;                   //添加头的数据结构成员
    spVALUE m_psAverage;                   //添加头的数据结构成员
    spFH32 m_hash;
    spFH256 m_logsBloom;
    spFH32 m_mixHash;
    spFH8 m_nonce;
    spVALUE m_number;
    spFH32 m_parentHash;
    spFH32 m_receiptsRoot;
    spFH32 m_tasksRoot;                   //添加头的数据结构成员
    spFH32 m_CreditHash;                   //添加头的数据结构成员
    spFH32 m_sha3Uncles;
    spFH32 m_stateRoot;
    spVALUE m_timestamp;
    spFH32 m_transactionsRoot;
};

typedef GCP_SPointer<BlockHeader> spBlockHeader;


}  // namespace teststruct
}  // namespace test
