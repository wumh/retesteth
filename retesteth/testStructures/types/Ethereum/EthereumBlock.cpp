#include "EthereumBlock.h"
#include <libdevcore/CommonIO.h>
#include <libdevcore/RLP.h>
//#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>//防止编译错误
#include <retesteth/EthChecks.h>
using namespace dev;

namespace test
{
namespace teststruct
{
void EthereumBlock::recalculateUncleHash()
{
    // Transaction has is calculated from transaction trie, not from list
    // Transaction List
    /*
    RLPStream transactionList(m_transactions.size());
    for (auto const& tr : m_transactions)
        transactionList.appendRaw(tr.asRLPStream().out());
    */

    // Uncle List
    RLPStream uncleList(m_uncles.size());
    for (auto const& un : m_uncles)
        uncleList.appendRaw(un.asRLPStream().out());

    // FH32 newTxHash("0x" + toString(dev::sha3(transactionList.out())));
    // m_header.getContent().setTransactionHash(newTxHash);

    FH32 newUnHash("0x" + dev::toString(dev::sha3(uncleList.out())));
    m_header.getContent().setUnclesHash(newUnHash);

    FH32 newHeaderHash("0x" + dev::toString(dev::sha3(m_header.getCContent().asRLPStream().out())));
    m_header.getContent().setHeaderHash(newHeaderHash);
}

BYTES const EthereumBlock::getRLP() const
{
    try
    {
        // RLP of a block
        RLPStream stream(3);
        stream.appendRaw(m_header.getCContent().asRLPStream().out());

        // Transaction list
        RLPStream transactionList(m_transactions.size());
        for (auto const& tr : m_transactions)
            transactionList.appendRaw(tr.asRLPStream().out());
        stream.appendRaw(transactionList.out());

        // Uncle list
        RLPStream uncleList(m_uncles.size());
        for (auto const& un : m_uncles)
            uncleList.appendRaw(un.asRLPStream().out());
        stream.appendRaw(uncleList.out());

        return BYTES(dev::toHexPrefixed(stream.out()));
    }
    catch (std::exception const& _ex)
    {
        ETH_ERROR_MESSAGE(string("EthereumBlock::getRLP() ") + _ex.what());
    }
    return BYTES(DataObject());
}


}  // namespace teststruct
}  // namespace test
