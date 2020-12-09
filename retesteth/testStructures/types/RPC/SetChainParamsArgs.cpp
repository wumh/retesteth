#include "SetChainParamsArgs.h"
#include <retesteth/EthChecks.h>
#include <retesteth/testStructures/Common.h>

namespace test
{
namespace teststruct
{
SetChainParamsArgs::SetChainParamsArgs(DataObject const& _data)
{
    m_params = _data.atKey("params");  // Client specific params
    m_preState = spState(new State(_data.atKey("accounts")));
    m_sealEngine = sealEngineFromStr(_data.atKey("sealEngine").asString());

    // Fill up required fields for blocheader info
    DataObject const& genesis = _data.atKey("genesis");
    DataObject fullBlockHeader;
    fullBlockHeader["author"] = genesis.atKey("author");
    fullBlockHeader["difficulty"] = genesis.atKey("difficulty");
    fullBlockHeader["gasLimit"] = genesis.atKey("gasLimit");
    fullBlockHeader["extraData"] = genesis.atKey("extraData");
    fullBlockHeader["timestamp"] = genesis.atKey("timestamp");
    fullBlockHeader["nonce"] = genesis.atKey("nonce");
    fullBlockHeader["mixHash"] = genesis.atKey("mixHash");

    // Fields that are ommited in RPC setChainParams, use default fields of empty block
    fullBlockHeader["bloom"] = FH256::zero().asString();
    fullBlockHeader["gasUsed"] = "0x00";
    fullBlockHeader["pstotal"] = "0x00";                    //增加新链新增成员及其测试使用的默认值
    fullBlockHeader["psAverage"] = "0x00";                    //增加新链新增成员及其测试使用的默认值
    fullBlockHeader["number"] = "0x00";
    fullBlockHeader["receiptTrie"] = "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421";
    fullBlockHeader["tasksRoot"] = "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421";                    //增加新链新增成员及其测试使用的默认值
    fullBlockHeader["CreditHash"] = "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421";                    //增加新链新增成员及其测试使用的默认值
    fullBlockHeader["stateRoot"] = FH32::zero().asString();
    fullBlockHeader["transactionsTrie"] = "0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421";
    fullBlockHeader["uncleHash"] = "0x1dcc4de8dec75d7aab85b567b6ccd41ad312451b948a7413f0a142fd40d49347";
    fullBlockHeader["parentHash"] = FH32::zero().asString();
    m_genesis = spBlockHeader(new BlockHeader(fullBlockHeader));

    requireJsonFields(_data, "SetChainParamsArgs " + _data.getKey(),
        {{"params", {{DataType::Object}, jsonField::Required}}, {"accounts", {{DataType::Object}, jsonField::Required}},
            {"genesis", {{DataType::Object}, jsonField::Required}}, {"sealEngine", {{DataType::String}, jsonField::Required}}});
}

DataObject SetChainParamsArgs::asDataObject() const
{
    DataObject out;
    out["params"] = m_params;
    out["accounts"] = m_preState.getCContent().asDataObject();
    out["sealEngine"] = sealEngineToStr(m_sealEngine);
    out["genesis"]["author"] = m_genesis.getCContent().author().asString();
    out["genesis"]["difficulty"] = m_genesis.getCContent().difficulty().asString();
    out["genesis"]["gasLimit"] = m_genesis.getCContent().gasLimit().asString();
    out["genesis"]["extraData"] = m_genesis.getCContent().extraData().asString();
    out["genesis"]["timestamp"] = m_genesis.getCContent().timestamp().asString();
    out["genesis"]["nonce"] = m_genesis.getCContent().nonce().asString();
    out["genesis"]["mixHash"] = m_genesis.getCContent().mixHash().asString();
    return out;
}

}  // namespace teststruct
}  // namespace test
