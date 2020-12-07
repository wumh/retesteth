#include "Common.h"
#include <dataObject/DataObject.h>
#include <retesteth/Options.h>
#include <retesteth/session/Session.h>
using namespace std;
namespace test
{
CompareResult compareAccounts(AccountBase const& _expectAccount, Account const& _remoteAccount);

Account remoteGetAccount(SessionInterface& _session, VALUE const& _bNumber, VALUE const& _trIndex, FH20 const& _account)
{
    VALUE balance(_session.eth_getBalance(_account, _bNumber));
    VALUE nonce(_session.eth_getTransactionCount(_account, _bNumber));
    BYTES code(_session.eth_getCode(_account, _bNumber));

    bool hasStorage = true;
    FH32 beginHash = FH32::zero();
    Storage tmpStorage = Storage(DataObject(DataType::Object));
    size_t safety = 500;
    while (hasStorage && --safety)
    {
        // Read storage from remote account by 20 records at a time
        DebugStorageRangeAt res(_session.debug_storageRangeAt(_bNumber, _trIndex, _account, beginHash, 20));
        if (res.nextKey().isZero())
            hasStorage = false;
        else
            beginHash = res.nextKey();
        tmpStorage.merge(res.storage());
    }
    if (safety == 0)
        ETH_ERROR_MESSAGE("remoteGetAccount::DebugStorageRangeAt seems like an endless loop!");
    return Account(_account, balance, nonce, code, tmpStorage);
}

// Get full remote state from the client
State getRemoteState(SessionInterface& _session)
{
    VALUE recentBNumber(_session.eth_blockNumber());
    EthGetBlockBy recentBlock(_session.eth_getBlockByNumber(recentBNumber, Request::LESSOBJECTS));
    VALUE trIndex(recentBlock.transactions().size());

    // Construct accountList by asking packs of 10th of accounts from remote client
    std::vector<FH20> accountList;
    FH32 nextKey("0x0000000000000000000000000000000000000000000000000000000000000001");
    while (!nextKey.isZero())
    {
        DebugAccountRange range(_session.debug_accountRange(recentBNumber, trIndex, nextKey, 10));
        for (auto const& el : range.addresses())
        {
            accountList.push_back(el);
            if (!Options::get().fullstate && accountList.size() > 50)
                throw StateTooBig();
        }
        nextKey = range.nextKey();
    }

    // Use recentBNumber and trIndex once to request account information in loop
    auto getRemoteAccount = [&_session, &recentBNumber, &trIndex](
                                FH20 const& _account) { return remoteGetAccount(_session, recentBNumber, trIndex, _account); };

    size_t byteSize = 0;
    std::vector<spAccount> stateAccountMap;
    for (auto const& acc : accountList)
    {
        spAccount remAccount(new Account(getRemoteAccount(acc)));
        stateAccountMap.push_back(remAccount);
        if (!Options::get().fullstate)
        {
            byteSize += remAccount.getCContent().storage().getKeys().size() * 64;
            byteSize += remAccount.getCContent().code().asString().size() / 2;
            if (byteSize > 1048510) // 1MB
                throw StateTooBig();
        }
    }
    return State(stateAccountMap);
}

// Compare expected state with session asking post state data on the fly
void compareStates(StateBase const& _stateExpect, SessionInterface& _session)
{
    CompareResult result = CompareResult::Success;

    VALUE recentBNumber(_session.eth_blockNumber());
    EthGetBlockBy recentBlock(_session.eth_getBlockByNumber(recentBNumber, Request::LESSOBJECTS));
    VALUE trIndex(recentBlock.transactions().size());

    // Use recentBNumber and trIndex once to request account information in loop
    auto getRemoteAccount = [&_session, &recentBNumber, &trIndex](
                                FH20 const& _account) { return remoteGetAccount(_session, recentBNumber, trIndex, _account); };

    // Construct accountList by asking packs of 10th of accounts from remote client
    std::set<FH20> remoteAccountList;
    FH32 nextKey("0x0000000000000000000000000000000000000000000000000000000000000001");
    while (!nextKey.isZero())
    {
        DebugAccountRange range(_session.debug_accountRange(recentBNumber, trIndex, nextKey, 10));
        for (auto const& el : range.addresses())
            remoteAccountList.insert(el);
        nextKey = range.nextKey();
    }

    for (auto const& ael : _stateExpect.accounts())
    {
        AccountBase const& a = ael.second.getCContent();
        bool remoteHasAccount = remoteAccountList.count(a.address());
        if (a.shouldNotExist() && remoteHasAccount)
        {
            ETH_MARK_ERROR("Compare States: '" + a.address().asString() + "' address not expected to exist!");
            result = CompareResult::AccountShouldNotExist;
            continue;
        }
        else if (!a.shouldNotExist() && !remoteHasAccount)
        {
            ETH_MARK_ERROR("Compare States: Missing expected address: '" + a.address().asString() + "'");
            result = CompareResult::MissingExpectedAccount;
            continue;
        }
        else if (a.shouldNotExist() && !remoteHasAccount)
            continue;

        // Compare account in postState with expect section account
        CompareResult accountCompareResult = compareAccounts(a, getRemoteAccount(a.address()));
        if (accountCompareResult != CompareResult::Success)
            result = accountCompareResult;
    }

    if (result != CompareResult::Success)
        ETH_ERROR_MESSAGE("CompareStates failed with errors: " + CompareResultToString(result));
}

// Compare one storage against another
CompareResult compareStorage(Storage const& _expectStorage, Storage const& _remoteStorage, FH20 const& _remoteAccount)
{
    CompareResult result = CompareResult::Success;
    string const message = "Check State: Remote account '" + _remoteAccount.asString() + "' ";

    for (auto const& element : _expectStorage.getKeys())
    {
        VALUE const& expKey = std::get<0>(element.second).getCContent();
        VALUE const& expVal = std::get<1>(element.second).getCContent();

        // If remote storage doesn't exist and expected is not 00 (zeros omited)
        if (!_remoteStorage.hasKey(expKey) && expVal.asU256() != 0)
        {
            ETH_MARK_ERROR(message + "test expected storage key: '" + expKey.asString() + "' to be set to: '" +
                           expVal.asString() + "', but remote key '" + expKey.asString() + "' does not exist!");
            result = CompareResult::IncorrectStorage;
        }
        else if (_remoteStorage.hasKey(expKey))
        {
            VALUE const& remoteVal = _remoteStorage.atKey(expKey);
            if (remoteVal != expVal)
            {
                ETH_MARK_ERROR(message + "has incorrect storage [" + expKey.asString() + "] = `" + remoteVal.asString() +
                               "`, test expected [" + expKey.asString() + "] = `" + expVal.asString() + "`");
                result = CompareResult::IncorrectStorage;
            }
        }
    }

    if (_expectStorage.getKeys().size() < _remoteStorage.getKeys().size())
    {
        ETH_MARK_ERROR(message + " has more storage records than expected!");
        result = CompareResult::IncorrectStorage;
    }

    return result;
}

// Compare Expected Account agains Account
CompareResult compareAccounts(AccountBase const& _expectAccount, Account const& _remoteAccount)
{
    // report all errors, but return the last error as a compare result
    CompareResult result = CompareResult::Success;
    auto checkEqual = [&_remoteAccount](VALUE const& _expect, VALUE const& _remote, string const& _what) -> bool {
        if ((_expect != _remote) && (_expect.asString()!="0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"))
        {
            ETH_WARNING("Check State: Remote account '" + _remoteAccount.address().asString() + "': has incorrect " + _what +
                           " '" + _remote.asDecString() + "', test expected '" + _expect.asDecString() + "' (" +
                           _remote.asString() + " != " + _expect.asString() + ")");
            return false;
        }
        return true;
    };

    if (_expectAccount.hasBalance())
        if (!checkEqual(_expectAccount.balance(), _remoteAccount.balance(), "balance"))
            result = CompareResult::IncorrectBalance;

    if (_expectAccount.hasNonce())
        if (!checkEqual(_expectAccount.nonce(), _remoteAccount.nonce(), "nonce"))
            result = CompareResult::IncorrectNonce;

    if (_expectAccount.hasCode())
    {
        if (_expectAccount.code() != _remoteAccount.code())
        {
            ETH_MARK_ERROR("Check State: Remote account '" + _remoteAccount.address().asString() + "': has incorrect code '" +
                           _remoteAccount.code().asString() + "', test expected '" + _expectAccount.code().asString() + "'");
            result = CompareResult::IncorrectCode;
        }
    }

    // Check that state post has values from expected storage
    if (_expectAccount.hasStorage())
    {
        CompareResult res = compareStorage(_expectAccount.storage(), _remoteAccount.storage(), _remoteAccount.address());
        if (res != CompareResult::Success)
            result = res;
    }

    return result;
}

// Compare expected state again post state
void compareStates(StateBase const& _stateExpect, State const& _statePost)
{
    CompareResult result = CompareResult::Success;
    for (auto const& ael : _stateExpect.accounts())
    {
        AccountBase const& a = ael.second.getCContent();
        bool remoteHasAccount = _statePost.hasAccount(a.address());
        if (a.shouldNotExist() && remoteHasAccount)
        {
            ETH_MARK_ERROR("Compare States: '" + a.address().asString() + "' address not expected to exist!");
            result = CompareResult::AccountShouldNotExist;
            continue;
        }
        else if (!a.shouldNotExist() && !remoteHasAccount)
        {
            ETH_MARK_ERROR("Compare States: Missing expected address: '" + a.address().asString() + "'");
            result = CompareResult::MissingExpectedAccount;
            continue;
        }
        else if (a.shouldNotExist() && !remoteHasAccount)
            continue;

        // Compare account in postState with expect section account
        CompareResult accountCompareResult = compareAccounts(a, _statePost.getAccount(a.address()));
        if (accountCompareResult != CompareResult::Success)
            result = accountCompareResult;
    }
    if (result != CompareResult::Success)
        ETH_ERROR_MESSAGE("CompareStates failed with errors_1: " + CompareResultToString(result));
}

string CompareResultToString(CompareResult res)
{
    switch (res)
    {
    case CompareResult::Success:
        return "Success";
    case CompareResult::AccountShouldNotExist:
        return "AccountShouldNotExist";
    case CompareResult::MissingExpectedAccount:
        return "MissingExpectedAccount";
    case CompareResult::IncorrectBalance:
        return "IncorrectBalance";
    case CompareResult::IncorrectNonce:
        return "IncorrectNonce";
    case CompareResult::IncorrectCode:
        return "IncorrectCode";
    case CompareResult::IncorrectStorage:
        return "IncorrectStorage";
    default:
        return "Unparsed Compare Result";
    }
}
}
