#include "GeneralStateTestsFiller.h"
#include "../../Common.h"
#include <retesteth/EthChecks.h>
#include <retesteth/Options.h>
#include <retesteth/ethObjects/object.h>

using namespace test::teststruct;
GeneralStateTestFiller::GeneralStateTestFiller(DataObject const& _data)
{
    try
    {
        checkDataObject(_data);
        for (auto const& el : _data.getSubObjects())
        {
            TestOutputHelper::get().setCurrentTestInfo(
                TestInfo("GeneralStateTestFiller", el.getKey()));
            m_tests.push_back(StateTestInFiller(el));
        }
    }
    catch (DataObjectException const& _ex)
    {
        ETH_ERROR_MESSAGE(_ex.what());
    }
}

StateTestInFiller::StateTestInFiller(DataObject const& _data)
{
    if (_data.count("_info"))
        m_info = GCP_SPointer<InfoIncomplete>(new InfoIncomplete(_data.atKey("_info")));
    m_env = GCP_SPointer<StateTestFillerEnv>(new StateTestFillerEnv(_data.atKey("env")));

    // -- Compile LLL in pre state into byte code if not already
    // -- Convert State::Storage keys/values into hex
    DataObject tmpD = _data.atKey("pre");
    for (auto& acc : tmpD.getSubObjectsUnsafe())
    {
        acc["code"].setString(test::replaceCode(acc.atKey("code").asString()));
        acc.performModifier(mod_valueToCompactEvenHexPrefixed);
        for (auto& rec : acc["storage"].getSubObjectsUnsafe())
            rec.performModifier(mod_keyToCompactEvenHexPrefixed);
    }
    // ---

    m_pre = GCP_SPointer<State>(new State(tmpD));
    m_transaction = GCP_SPointer<StateTestFillerTransaction>(
        new StateTestFillerTransaction(_data.atKey("transaction")));
    for (auto const& el : _data.atKey("expect").getSubObjects())
        m_expectSections.push_back(StateTestFillerExpectSection(el));
    m_name = _data.getKey();
}

// Gather all networks from all the expect sections
std::list<FORK> StateTestInFiller::getAllForksFromExpectSections() const
{
    std::list<FORK> out;
    for (auto const& ex : m_expectSections)
        for (auto const& el : ex.forks())
            out.push_back(el);
    return out;
}