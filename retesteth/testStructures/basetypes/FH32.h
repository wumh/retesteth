#pragma once
#include "FH.h"
#include <retesteth/dataObject/DataObject.h>
#include <retesteth/dataObject/SPointer.h>
using namespace dataobject;

namespace test
{
namespace teststruct
{
// Validate and manage the type of FixedHash32
// Deserialized from string of "0x1122...32" exact length
struct FH32 : FH
{
    FH32(DataObject const& _data) : FH(_data, 32) {}
    FH32(string const& _data) : FH(_data, 32) {}
    bool isZero() const { return m_data == zero().asString(); }
    static FH32 zero() { return FH32("0x0000000000000000000000000000000000000000000000000000000000000000"); }
};

typedef GCP_SPointer<FH32> spFH32;

}  // namespace teststruct
}  // namespace test
