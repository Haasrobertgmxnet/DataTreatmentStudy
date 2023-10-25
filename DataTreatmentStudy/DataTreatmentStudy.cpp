// DataTreatmentStudy.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>
// #include <tuple>
#include <cassert>

template<typename T>
struct ResType {
    std::_Tree_iterator<std::_Tree_val<std::_Tree_simple_types<T>>> data;
    ResType(const std::_Tree_iterator<std::_Tree_val<std::_Tree_simple_types<T>>>& _data) {
        data = _data;
    }
};

struct Splitter {
    size_t cnt = 0;
    std::pair< std::vector<size_t>, std::vector<size_t>> idcs = std::make_pair< std::vector<size_t>, std::vector<size_t>>({},{});

public:
    Splitter() {
    }

    Splitter(size_t _cnt) :cnt(_cnt) {
        resetIdcsFirst();
    }

    void reset(size_t _cnt) {
        cnt = _cnt;
        resetIdcsFirst();
    }

private:
    void resetIdcsFirst() {
        idcs.first.resize(cnt);
        std::iota(std::begin(idcs.first), std::end(idcs.first), 0);
    }

public:
    void pickIdcsRandomly(size_t _icdsToPick) {
        if (_icdsToPick > cnt) {
            return;
        }
        idcs.second.resize(0);
        for (size_t j = 0; j < _icdsToPick; ++j) {
            size_t randIndex = rand() % cnt;
            if (std::find(idcs.second.begin(), idcs.second.end(), randIndex) != idcs.second.end()) {

                continue;
            }
            idcs.second.push_back(randIndex);
        }
        std::sort(idcs.second.begin(), idcs.second.end());
    }

    void removeIdcs() {
        if (idcs.second.size() == 0) {
            return;
        }
        resetIdcsFirst();
        for (auto it = idcs.second.cbegin(); it != idcs.second.cend(); ++it) {
            auto found = std::find(idcs.first.begin(), idcs.first.end(), *it);
            if (found == idcs.first.end()) {
                continue;
            }
            idcs.first.erase(found);
        }
    }

    void readIcdsFromFile()
    {
        // Ref.: https://stackoverflow.com/questions/15138785/how-to-read-a-file-into-vector-in-c
        // Opening the file
        std::ifstream is("file.txt", std::ios::in);

        std::istream_iterator<size_t> start(is), end;
        std::vector<size_t> numbers(start, end);
        idcs.second.resize(0);
        std::copy(numbers.begin(), numbers.end(), std::back_inserter(idcs.second));
        std::sort(idcs.second.begin(), idcs.second.end());
    }
};

struct Filter {
    size_t targetIdx = 1;

};

struct DataObject {
    std::vector<std::vector<std::string>> content = {};
    Splitter splitter;
    Filter filter;
    DataObject() {
    }

    DataObject(const std::vector<std::vector<std::string>>& _content) {
        std::copy(_content.begin(), _content.end(), std::back_inserter(content));
        splitter.reset(content.size());
    }

    std::vector<std::string>& dataItem(std::size_t _i, std::vector<size_t> _ws) {
        assert(_i < _ws.size());
        return content[_ws[_i]];
    }

    std::vector<std::string>& trainData(std::size_t _i) {
        return dataItem(_i, splitter.idcs.first);
    }

    std::vector<std::string>& testData(std::size_t _i) {
        return dataItem(_i, splitter.idcs.second);
    }

    size_t getTrainDataSize() const {
        return splitter.idcs.first.size();
    }

    size_t getTestDataSize() const {
        return splitter.idcs.second.size();
    }

    void setData(std::vector < std::vector<std::string>>& _tData, std::vector<size_t >& _ws) {
        _tData.resize(0);
        for (auto it = _ws.cbegin(); it != _ws.cend(); ++it) {
            _tData.push_back(content[*it]);
        }
    }

    void allTrainData(std::vector < std::vector<std::string>>& _tData) {
        setData(_tData, splitter.idcs.first);
    }

    void allTestData(std::vector < std::vector<std::string>>& _tData) {
        setData(_tData, splitter.idcs.second);
    }
};

const unsigned int targetIndex = 1;

const std::vector<std::vector<std::string>> RawData = { {"2.3", "One"}, 
    {"3.1", "Two"}, 
    {"1.7", "One"}, 
    {"3.4", "Two"}, 
    {"2.0", "One"}, 
    {"3.7", "Two"}, 
    {"1.7", "One"}, 
    {"3.2", "Two"},
    {"1.4", "One"}, 
    {"1.9", "Three"} };

int main()
{

    size_t countRawData = RawData.size();
    std::map<std::string, int> codingTable;

    unsigned j = 0;
    for (auto it = RawData.cbegin(); it != RawData.cend(); ++it) {
        auto key = it->at(targetIndex);
        if (codingTable.find(key) != codingTable.end()) {
            continue;
        }
        ++j;
        ResType<std::pair<std::string const, int>> res(codingTable.insert(codingTable.begin(), std::pair<std::string, int>(key, j)));
    }

    std::map<std::string, std::vector<size_t>> idxMap;

    for (auto it = codingTable.cbegin(); it != codingTable.cend(); ++it) {
        {
            idxMap.insert(idxMap.begin(), std::pair<std::string, std::vector<size_t>>(it->first, { }));
        }
    }

    for (auto it = idxMap.begin(); it != idxMap.end(); ++it) {
        std::vector<size_t> idx;
        for (auto it1 = RawData.cbegin(); it1 != RawData.cend(); ++it1) {
            if (it1->at(targetIndex) != it->first) {
                continue;
            }
            idx.push_back(std::distance(RawData.cbegin(), it1));
        }
        (it->second).insert((it->second).end(), idx.begin(), idx.end());
    }

    Splitter splitter(10);
    splitter.pickIdcsRandomly(3);
    splitter.removeIdcs();
    splitter.removeIdcs();
    splitter.readIcdsFromFile();

    DataObject dataObj(RawData);
    dataObj.splitter.pickIdcsRandomly(3);
    dataObj.splitter.removeIdcs();

    for (size_t j = 0; j < dataObj.getTrainDataSize(); ++j) {
        auto ws = dataObj.trainData(j);
        ws[0] = "2.333";
    }

    for (size_t j = 0; j < dataObj.getTestDataSize(); ++j) {
        auto ws = dataObj.testData(j);
        ws[0] = "2.4711";
        dataObj.testData(j) = ws;
    }

    std::vector<std::vector<std::string>> trainData = {};

    dataObj.allTrainData(trainData);

    std::cout << "Hello World!\n";
}

