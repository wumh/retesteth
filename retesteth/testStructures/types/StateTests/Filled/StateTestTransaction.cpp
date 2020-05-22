#include "StateTestTransaction.h"
#include "../TransactionInGeneralSection.h"
#include <retesteth/TestHelper.h>
#include <retesteth/ethObjects/object.h>

namespace test
{
namespace teststruct
{
StateTestTransaction::StateTestTransaction(DataObject const& _data)
{
    m_to = spFH20(new FH20(_data.atKey("to")));
    m_secretKey = spFH32(new FH32(_data.atKey("secretKey")));
    m_gasPrice = spVALUE(new VALUE(_data.atKey("gasPrice")));
    m_nonce = spVALUE(new VALUE(_data.atKey("nonce")));

    for (auto const& el : _data.atKey("data").getSubObjects())
        m_data.push_back(el);
    for (auto const& el : _data.atKey("gasLimit").getSubObjects())
        m_gasLimit.push_back(el);
    for (auto const& el : _data.atKey("value").getSubObjects())
        m_value.push_back(el);
}

DataObject StateTestTransaction::asDataObject() const
{
    // Serialize data back to JSON
    DataObject out;
    for (BYTES const& el : m_data)
        out["data"].addArrayObject(el.asString());
    for (VALUE const& el : m_gasLimit)
        out["gasLimit"].addArrayObject(el.asString());
    for (VALUE const& el : m_value)
        out["value"].addArrayObject(el.asString());
    out["gasPrice"] = m_gasPrice.getCContent().asString();
    out["nonce"] = m_nonce.getCContent().asString();
    out["to"] = m_to.getCContent().asString();
    out["secretKey"] = m_secretKey.getCContent().asString();
    return out;
}

std::vector<TransactionInGeneralSection> StateTestTransaction::buildTransactions() const
{
    // Construct vector of all transactions that are described int data
    std::vector<TransactionInGeneralSection> out;
    for (size_t dIND = 0; dIND < m_data.size(); dIND++)
    {
        for (size_t gIND = 0; gIND < m_gasLimit.size(); gIND++)
        {
            for (size_t vIND = 0; vIND < m_value.size(); vIND++)
            {
                DataObject trData;
                trData["data"] = m_data.at(dIND).asString();
                trData["gasLimit"] = m_gasLimit.at(gIND).asString();
                trData["value"] = m_value.at(vIND).asString();

                trData["gasPrice"] = m_gasPrice.getCContent().asString();
                trData["nonce"] = m_nonce.getCContent().asString();
                trData["to"] = m_to.getCContent().asString();
                trData["secretKey"] = m_secretKey.getCContent().asString();
                out.push_back(TransactionInGeneralSection(trData, dIND, gIND, vIND));
            }
        }
    }
    return out;
}

}  // namespace teststruct
}  // namespace test