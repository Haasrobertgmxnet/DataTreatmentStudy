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

// template<typename T>
struct Splitter {

    size_t cnt = 0;
    std::vector<size_t> idcs1 = {};
    std::vector<size_t> idcs2 = {};
    std::pair< std::vector<size_t>, std::vector<size_t>> idcs = std::make_pair< std::vector<size_t>, std::vector<size_t>>({},{});

public:
    Splitter() {
    }

    Splitter(size_t _cnt) :cnt(_cnt) {
        resetIdcs1();
    }

    void reset(size_t _cnt) {
        cnt = _cnt;
        resetIdcs1();
    }

private:
    void resetIdcs1() {
        idcs1.resize(cnt);
        std::iota(std::begin(idcs1), std::end(idcs1), 0);

        idcs.first.resize(cnt);
        std::iota(std::begin(idcs.first), std::end(idcs.first), 0);
    }

public:
    void pickIdcsRandomly(size_t _icdsToPick) {
        if (_icdsToPick > cnt) {
            return;
        }
        idcs2.resize(0);
        idcs.second.resize(0);
        for (size_t j = 0; j < _icdsToPick; ++j) {
            size_t randIndex = rand() % cnt;
            if (std::find(idcs2.begin(), idcs2.end(), randIndex) != idcs2.end()) {

                continue;
            }
            idcs2.push_back(randIndex);
        }
        std::sort(idcs2.begin(), idcs2.end());
    }

    void removeIdcs() {
        if (idcs2.size() == 0) {
            return;
        }
        resetIdcs1();
        for (auto it = idcs2.cbegin(); it != idcs2.cend(); ++it) {
            auto found = std::find(idcs1.begin(), idcs1.end(), *it);
            if (found == idcs1.end()) {
                continue;
            }
            idcs1.erase(found);
        }
    }

    void readIcdsFromFile()
    {
        // Ref.: https://stackoverflow.com/questions/15138785/how-to-read-a-file-into-vector-in-c
        // Opening the file
        std::ifstream is("file.txt", std::ios::in);

        std::istream_iterator<size_t> start(is), end;
        std::vector<size_t> numbers(start, end);
        idcs2.resize(0);
        std::copy(numbers.begin(), numbers.end(), std::back_inserter(idcs2));
        std::sort(idcs2.begin(), idcs2.end());
    }
};

struct DataObject {
    std::vector<std::vector<std::string>> content = {};
    Splitter splitter;
    DataObject() {
    }

    DataObject(const std::vector<std::vector<std::string>>& _content) {
        std::copy(_content.begin(), _content.end(), std::back_inserter(content));
        splitter.reset(content.size());
    }

    std::vector<std::string>& trainData(std::size_t _i) {
        assert(_i < splitter.idcs1.size());
        return content[splitter.idcs1[_i]];
    }

    std::vector<std::string>& testData(std::size_t _i) {
        assert(_i < splitter.idcs2.size());
        return content[splitter.idcs2[_i]];
    }

    size_t getTrainDataSize() const {
        return splitter.idcs1.size();
    }

    size_t getTestDataSize() const {
        return splitter.idcs2.size();
    }

    void allTrainData(std::vector < std::vector<std::string>>& _trainData) {
        _trainData.resize(0);
        for (auto it = splitter.idcs1.cbegin(); it != splitter.idcs1.cend(); ++it) {
            _trainData.push_back(content[*it]);
        }
    }

    void allTestData(std::vector < std::vector<std::string>>& _testData) {
        _testData.resize(0);
        for (auto it = splitter.idcs2.cbegin(); it != splitter.idcs2.cend(); ++it) {
            _testData.push_back(content[*it]);
        }
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

