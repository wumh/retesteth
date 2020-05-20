#include "VALUE.h"
#include <retesteth/EthChecks.h>
#include <retesteth/ethObjects/object.h>
#include <retesteth/TestHelper.h>
using namespace test::teststruct;

namespace test
{
namespace teststruct
{
VALUE::VALUE(DataObject const& _data, dev::u256 _limit) : m_data(_data)
{
    string const& k = _data.getKey();
    string const& v = _data.asString();
    if (v[0] == '0' && v[1] == 'x')
    {
        if (v.size() == 2)
            ETH_ERROR_MESSAGE(
                "Key `" + k + "` is VALUE, but set as empty byte string: `" + v + "`");
        // don't allow 0x001, but allow 0x0, 0x00
        if ((v[2] == '0' && v.size() % 2 == 1 && v.size() != 3) ||
            (v[2] == '0' && v[3] == '0' && v.size() % 2 == 0 && v.size() > 4))
            ETH_ERROR_MESSAGE("Key `" + k + "` is VALUE and has leading 0 `" + v + "`");
    }
    else
        ETH_ERROR_MESSAGE("Key `" + k + "` is VALUE but not prefixed hex: `" + v + "`");

    if (v.size() > 64 + 2)
        ETH_ERROR_MESSAGE("Key `" + k + "` >u256 `" + v + "`");
    if (dev::u256(v) > _limit)
        ETH_ERROR_MESSAGE(
            "Key `" + k + "` > limit `" + dev::toString(_limit) + "`, key = `" + v + "`");

    m_data.performModifier(mod_valuesToLowerCase);
}

string VALUE::asDecString() const
{
    return to_string(test::hexOrDecStringToInt(m_data.asString()));
}

}  // namespace teststruct
}  // namespace test