#include "Transaction.h"
#include <libdevcore/Address.h>
#include <libdevcore/CommonIO.h>
//#include <libdevcore/SHA3.h>
#include <libdevcrypto/Hash.h>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/ECDSASignature.h>
#include <retesteth/EthChecks.h>
#include <retesteth/TestHelper.h>
#include <retesteth/testStructures/Common.h>

namespace test
{
namespace teststruct
{
Transaction::Transaction(DataObject const& _data)
{
    fromDataObject(_data);
}

void Transaction::fromDataObject(DataObject const& _data)
{
    try
    {
        m_data = spBYTES(new BYTES(_data.atKey("data")));
        m_gasLimit = spVALUE(new VALUE(_data.atKey("gasLimit")));
        m_gasPrice = spVALUE(new VALUE(_data.atKey("gasPrice")));
        m_nonce = spVALUE(new VALUE(_data.atKey("nonce")));
        m_value = spVALUE(new VALUE(_data.atKey("value")));

        // geth retesteth return to as null in some bc tests
        if (_data.atKey("to").type() == DataType::Null || _data.atKey("to").asString().empty())
            m_creation = true;
        else
        {
            m_creation = false;
            m_to = spFH20(new FH20(_data.atKey("to")));
        }

        if (_data.count("secretKey"))
        {
            VALUE a(_data.atKey("secretKey"));
            buildVRS(VALUE(_data.atKey("secretKey")));
        }
        else
        {
            m_v = spVALUE(new VALUE(_data.atKey("v")));
            if (m_v.getCContent() > dev::u256("0xff"))
                throw test::UpwardsException("Incorrect transaction `v` value: " + m_v.getCContent().asString());
            m_r = spVALUE(new VALUE(_data.atKey("r")));
            m_s = spVALUE(new VALUE(_data.atKey("s")));
        }
        requireJsonFields(_data, "Transaction " + _data.getKey(),
            {{"data", {{DataType::String}, jsonField::Required}},
             {"gasLimit", {{DataType::String}, jsonField::Required}},
             {"gasPrice", {{DataType::String}, jsonField::Required}},
             {"nonce", {{DataType::String}, jsonField::Required}},
             {"value", {{DataType::String}, jsonField::Required}},
             {"to", {{DataType::String, DataType::Null}, jsonField::Required}},
             {"secretKey", {{DataType::String}, jsonField::Optional}},
             {"v", {{DataType::String}, jsonField::Optional}},
             {"r", {{DataType::String}, jsonField::Optional}},
             {"s", {{DataType::String}, jsonField::Optional}},
             {"blockHash", {{DataType::String}, jsonField::Optional}},           // EthGetBlockBy transaction
             {"blockNumber", {{DataType::String}, jsonField::Optional}},         // EthGetBlockBy transaction
             {"from", {{DataType::String}, jsonField::Optional}},                // EthGetBlockBy transaction
             {"hash", {{DataType::String}, jsonField::Optional}},                // EthGetBlockBy transaction
             {"transactionIndex", {{DataType::String}, jsonField::Optional}},    // EthGetBlockBy transaction
             {"invalid", {{DataType::String}, jsonField::Optional}},             // BlockchainTest filling
                          });
    }
    catch (std::exception const& _ex)
    {
        throw UpwardsException(string("Transaction convertion error: ") + _ex.what() + _data.asJson());
    }
}

void Transaction::fromRLP(dev::RLP const& _rlp)
{
    // 0 - nonce        3 - to      6 - v
    // 1 - gasPrice     4 - value   7 - r
    // 2 - gasLimit     5 - data    8 - s
    DataObject trData;
    size_t i = 0;
    trData["nonce"] = rlpToString(_rlp[i++]);
    trData["gasPrice"] = rlpToString(_rlp[i++]);
    trData["gasLimit"] = rlpToString(_rlp[i++]);
    string const to = rlpToString(_rlp[i++], 0);
    trData["to"] = to == "0x" ? "" : to;
    trData["value"] = rlpToString(_rlp[i++]);
    trData["data"] = rlpToString(_rlp[i++], 0);
    trData["v"] = rlpToString(_rlp[i++]);
    trData["r"] = rlpToString(_rlp[i++]);
    trData["s"] = rlpToString(_rlp[i++]);
    fromDataObject(trData);
}

Transaction::Transaction(dev::RLP const& _rlp)
{
    fromRLP(_rlp);
}

Transaction::Transaction(BYTES const& _rlp)
{
    dev::bytes decodeRLP = sfromHex(_rlp.asString());
    dev::RLP rlp(decodeRLP, dev::RLP::VeryStrict);
    fromRLP(rlp);
}


void Transaction::streamHeader(dev::RLPStream& _s) const
{
    _s << nonce().asU256();
    _s << gasPrice().asU256();
    _s << gasLimit().asU256();
    if (m_creation)
        _s << "";
    else
        _s << dev::Address(to().asString());
    _s << value().asU256();
    _s << test::sfromHex(data().asString());
}

void Transaction::buildVRS(VALUE const& _secret)
{
    dev::RLPStream stream;
    stream.appendList(6);
    streamHeader(stream);
    dev::h256 hash(dev::sha3(stream.out()));
    auto sigStruct = dev::ecdsaSign(dev::Secret(_secret.asString()), hash);
    //dev::Signature sig = dev::ecdsaSign(dev::Secret(_secret.asString()), hash);

    //dev::Signature_ sig = dev::sign(dev::Secret(_secret.asString()), hash);
    //dev::SignatureStruct sigStruct = *(dev::SignatureStruct const*)&sig;
    //dev::SignatureStruct sigStruct = *(dev::SignatureStruct const*)(sPsig.get());
    ETH_FAIL_REQUIRE_MESSAGE(
        sigStruct->isValid(), TestOutputHelper::get().testName() + " Could not construct transaction signature!");

    // 27 because devcrypto signing donesn't count chain id
    DataObject v = DataObject(dev::toCompactHexPrefixed(dev::u256(sigStruct->v + 27)));
    DataObject r = DataObject(dev::toCompactHexPrefixed(dev::u256(sigStruct->r)));
    DataObject s = DataObject(dev::toCompactHexPrefixed(dev::u256(sigStruct->s)));
    m_v = spVALUE(new VALUE(v));
    m_r = spVALUE(new VALUE(r));
    m_s = spVALUE(new VALUE(s));
}


BYTES const Transaction::getSignedRLP() const
{
    dev::RLPStream sWithSignature;
    sWithSignature.appendList(9);
    streamHeader(sWithSignature);
    sWithSignature << v().asU256().convert_to<dev::byte>();
    sWithSignature << r().asU256();
    sWithSignature << s().asU256();
    return BYTES(dev::toHexPrefixed(sWithSignature.out()));
}

dev::RLPStream const Transaction::asRLPStream() const
{
    dev::RLPStream out;
    out.appendList(9);
    streamHeader(out);
    out << v().asU256().convert_to<dev::byte>();
    out << r().asU256();
    out << s().asU256();
    return out;
}

const DataObject Transaction::asDataObject(ExportOrder _order) const
{
    DataObject out;
    out["data"] = m_data.getCContent().asString();
    out["gasLimit"] = m_gasLimit.getCContent().asString();
    out["gasPrice"] = m_gasPrice.getCContent().asString();
    out["nonce"] = m_nonce.getCContent().asString();
    if (m_creation)
    {
        if (_order != ExportOrder::ToolStyle)
            out["to"] = "";
    }
    else
        out["to"] = m_to.getCContent().asString();
    out["value"] = m_value.getCContent().asString();
    out["v"] = m_v.getCContent().asString();
    out["r"] = m_r.getCContent().asString();
    out["s"] = m_s.getCContent().asString();
    if (_order == ExportOrder::ToolStyle)
    {
        out.performModifier(mod_removeLeadingZerosFromHexValues, {"data", "to"});
        out.renameKey("gasLimit", "gas");
        out.renameKey("data", "input");
    }
    if (_order == ExportOrder::OldStyle)
    {
        out.setKeyPos("r", 4);
        out.setKeyPos("s", 5);
        out.setKeyPos("v", 7);
    }
    return out;
}

FH32 Transaction::hash() const
{
    return FH32("0x" + dev::toString(dev::sha3(asRLPStream().out())));
}

bool Transaction::operator==(Transaction const& _rhs) const
{
    bool creationCondition = false;
    if (m_creation && _rhs.isCreation())
        creationCondition = true;
    else if (m_creation && !_rhs.isCreation())
        creationCondition = false;
    else if (!m_creation && _rhs.isCreation())
        creationCondition = false;
    else if (!m_creation && !_rhs.isCreation())
        creationCondition = to() == _rhs.to();
    return creationCondition && data() == _rhs.data() && gasLimit() == _rhs.gasLimit() && gasPrice() == _rhs.gasPrice() &&
           nonce() == _rhs.nonce() && value() == _rhs.value() && v() == _rhs.v() && r() == _rhs.r() && s() == _rhs.s();
}

}  // namespace teststruct
}  // namespace test
