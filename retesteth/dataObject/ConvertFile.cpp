#include <dataObject/ConvertFile.h>
// Manually construct dataobject from file string content
// bacuse Json::Reader::parse has a memory leak

namespace dataobject
{
string const errorPrefix = "Error parsing json: ";

size_t stripSpaces(string const& _input, size_t _i)
{
    size_t i = _i;
    for (; i < _input.length(); i++)
    {
        if (_input[i] == ' ' || _input[i] == '\n' || _input[i] == '\r' || _input[i] == '\t')
            continue;
        else
            return i;
    }
    return i;
}

string parseKeyValue(string const& _input, size_t& _i)
{
    if (_i + 1 > _input.size())
        std::cerr << errorPrefix + "reached EOF before reading char: `\"`" << std::endl;

    size_t endPos = _input.find('"', _i + 1);
    if (endPos != string::npos)
    {
        string key = _input.substr(_i + 1, endPos - _i - 1);
        _i = endPos + 1;
        return key;
    }
    else
        std::cerr << errorPrefix + "not found key ending char: `\"`" << std::endl;
    return string();
}

bool readDigit(string const& _input, size_t& _i, int& _result)
{
    bool readMinus = false;
    char e = _input[_i];
    if (e == '-')
    {
        readMinus = true;
        _i++;
    }

    bool digit = true;
    string readNumber;
    while (digit)
    {
        e = _input[_i];
        if (e == '0' || e == '1' || e == '2' || e == '3' || e == '4' || e == '5' || e == '6' ||
            e == '7' || e == '8' || e == '9')
        {
            readNumber += e;
            _i++;
        }
        else
        {
            digit = false;
            _i = stripSpaces(_input, _i);
        }
    }
    if (readNumber.size())
    {
        _result = std::atoi(readNumber.c_str());
        if (readMinus)
            _result *= -1;
        return true;
    }
    return false;
}

/// Convert Json object represented as string to DataObject
DataObject ConvertJsoncppStringToData(
    std::string const& _input, string const& _stopper, bool _autosort)
{
    std::vector<DataObject*> applyDepth;  // indexes at root array of objects that we are reading
                                          // into
    DataObject root;
    root.setAutosort(_autosort);
    DataObject* actualRoot = &root;
    string debug;
    for (size_t i = 0; i < _input.length(); i++)
    {
        // std::cerr << root.asJson() << std::endl;
        i = stripSpaces(_input, i);
        if (i > 40)
            debug = _input.substr(i - 40, 80);

        if (_input[i] == '"')
        {
            DataObject obj;
            string key = parseKeyValue(_input, i);
            i = stripSpaces(_input, i);
            if (_input.at(i) == ':')
            {
                obj.setKey(key);
                bool replaceKey = false;
                for (size_t objI = 0; objI < actualRoot->getSubObjects().size(); objI++)
                {
                    if (actualRoot->getSubObjects().at(objI).getKey() == key)
                    {
                        replaceKey = true;
                        applyDepth.push_back(actualRoot);
                        actualRoot = &actualRoot->getSubObjectsUnsafe().at(objI);
                        actualRoot->clearSubobjects();
                        break;
                    }
                }

                if (replaceKey)
                    continue;
                applyDepth.push_back(actualRoot);  // remember the header
                actualRoot = &actualRoot->addSubObject(obj);
                actualRoot->setAutosort(_autosort);
                continue;
            }
            else
            {
                if (actualRoot->type() == DataType::Array)
                {
                    actualRoot->addArrayObject(DataObject(key));
                    if (_input.at(i) != ',')
                        i--;  // because cycle iteration we need to process ending clouse
                    continue;
                }
                else
                    actualRoot->setString(key);
                if (_input.at(i) != ',')
                    i--;  // because cycle iteration we need to process ending clouse
                actualRoot = applyDepth.at(applyDepth.size() - 1);
                applyDepth.pop_back();
                continue;
            }
        }
        if (_input[i] == '{')
        {
            if (actualRoot->type() == DataType::Array || actualRoot->type() == DataType::Object)
            {
                DataObject newObj(DataType::Object);
                applyDepth.push_back(actualRoot);
                actualRoot = &actualRoot->addSubObject(newObj);
                actualRoot->setAutosort(_autosort);
                continue;
            }

            actualRoot->addSubObject(DataObject());
            actualRoot->getSubObjectsUnsafe().pop_back();
            continue;
        }
        if (_input[i] == '[')
        {
            if (actualRoot->type() == DataType::Array || actualRoot->type() == DataType::Object)
            {
                DataObject* newObj = &actualRoot->addSubObject(DataObject(DataType::Array));
                newObj->setAutosort(_autosort);
                continue;
            }

            actualRoot->addArrayObject(DataObject());
            actualRoot->getSubObjectsUnsafe().pop_back();
            continue;
        }

        if (_input[i] == ']' || _input[i] == '}')
        {
            if (actualRoot->type() == DataType::Array && _input.at(i) != ']')
                std::cerr << "expected ']' closing the array! around: " << debug << std::endl;
            if (actualRoot->type() == DataType::Object && _input.at(i) != '}')
                std::cerr << "expected '}' closing the object! around: " << debug << std::endl;

            if (!_stopper.empty() && actualRoot->getKey() == _stopper)
                return root;

            if (applyDepth.size() == 0)
            {
                i++;
                i = stripSpaces(_input, i);
                if (i != _input.length())
                    std::cerr << errorPrefix + "expected end of json!" << std::endl;
                return root;
            }
            else
            {
                actualRoot = applyDepth.at(applyDepth.size() - 1);
                applyDepth.pop_back();
                // i++;
                if (_input[i + 1] == ',')
                    i++;
                continue;
            }
        }

        int res;
        if (readDigit(_input, i, res))
        {
            if (actualRoot->type() == DataType::Array)
                actualRoot->addArrayObject(res);
            else
            {
                actualRoot->setInt(res);
                actualRoot = applyDepth.at(applyDepth.size() - 1);
                applyDepth.pop_back();
            }
            if (_input[i] != ',')
                i--;
            continue;
        }
    }
    return root;
}
}
