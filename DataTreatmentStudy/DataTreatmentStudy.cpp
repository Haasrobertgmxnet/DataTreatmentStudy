// DataTreatmentStudy.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>
#include <ranges>
#include <cassert>
#include <string>
#include <sstream>
#include <cmath>

int getCsvContent(std::vector<std::vector<std::string>>& _csvContent, std::string _csvFile) {
    std::vector<std::string> row;
    std::string line, word;

    std::fstream file(_csvFile, std::ios::in);
    if (!file.is_open())
    {
        std::cout << "Could not open the file\n";
        return -1;
    }
    int i = 0;
    while (getline(file, line))
    {
        row.clear();

        std::stringstream str(line);

        while (getline(str, word, ','))
            row.push_back(word);
        _csvContent.push_back(row);
        ++i;
    }
    return i;
}

void getMetaData(std::map<std::string, size_t>& _metaData, std::string _metaDataFile) {
    std::vector<std::vector<std::string>> rawContent = {};
    getCsvContent(rawContent, _metaDataFile);

    std::cout << std::endl;
    std::for_each(rawContent.begin(), rawContent.end(), [&_metaData](std::vector < std::string>& _x) {_metaData.insert({ _x[0], std::stoul(_x[1]) }); });
    // std::transform(rawContent.cbegin(), rawContent.cend(), _metaData.begin(), [](std::vector < std::string>& _x) {std::back_inserter(_x[0], _x[1]); });
    return;
}

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
            // printf("j: %d\trand: %u\n", j, randIndex);
            if (std::find(idcs.second.begin(), idcs.second.end(), randIndex) != idcs.second.end()) {
                --j;
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

int main()
{
    std::string fname = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\irisMetaData.txt";
    std::map<std::string, size_t> metaData;
    getMetaData(metaData, fname);

    const size_t targetColumn = metaData["targetColumn"];
    const size_t lineCount = metaData["numberOfLines"];
    const size_t firstLineToRead = metaData["firstLineToRead"];

    fname = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\iris.csv";
    std::vector<std::vector<std::string>> content;
    getCsvContent(content, fname);

    content.erase(content.begin(), content.begin() + firstLineToRead);

    std::vector<std::string> targets = {};
    std::for_each(content.begin(), content.end(), [&targets, targetColumn](const std::vector<std::string>& _x) {if (!std::count(targets.begin(), targets.end(), _x[targetColumn])) targets.push_back(_x[targetColumn]); });

    DataObject dataObj(content);
    dataObj.splitter.pickIdcsRandomly(30);
    dataObj.splitter.removeIdcs();

    std::vector<std::vector<std::string>> trainData = {};
    dataObj.allTrainData(trainData);

    std::map<std::string, std::vector<std::vector<double>>> groupedData;

    for (auto it = targets.cbegin(); it != targets.cend(); ++it) {
        std::string s = *it;

        // Filtering
        auto view2 = trainData | std::views::filter([s, targetColumn](const std::vector<std::string>& _x) {return _x[targetColumn].find(s) != std::string::npos; });
        std::for_each(view2.begin(), view2.end(), [targetColumn](const std::vector<std::string>& _s) {std::cout << _s[targetColumn] << std::endl; });
        std::vector<std::vector<std::string>> vec = {};
        std::transform(view2.begin(), view2.end(), std::back_inserter(vec),
            [](const std::vector<std::string>& c) { return c; });

        std::for_each(vec.begin(), vec.end(), [](const std::vector<std::string>& _x) {std::cout << "__v[0]: " << _x[0] << "  __v[1]: " << _x[1] << std::endl; });
        std::cout << std::endl;

        // Conversion
        std::vector<std::vector<double>> vec1 = {};
        std::for_each(view2.begin(), view2.end(),
            [&vec1, targetColumn](std::vector<std::string>& c) {
                size_t j = 0;
                std::vector<double> vec2 = {};
                std::for_each(c.begin(), c.begin() + targetColumn, [&vec2](std::string _s) { vec2.push_back(std::stod(_s)); });
                std::for_each(c.begin() + targetColumn, c.end() - 1, [&vec2](std::string _s) { vec2.push_back(std::stod(_s)); });
                vec1.push_back(vec2); 
            });
        groupedData[s] = vec1;
    }

    for (auto item : groupedData) {
        std::cout << item.first << std::endl;
        std::vector<double> means = { 0.0,0.0,0.0,0.0 };
        double scal = 1.0 / item.second.size();
        std::for_each(item.second.begin(), item.second.end(), [&means, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), means.begin(), [scal](double _a, double _b) {_b += _a * scal; return _b; });
            });
        std::vector<std::vector<double>> ws(item.second);
        std::for_each(ws.begin(), ws.end(), [&means](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), _x.begin(), [](double _a, double _b) {return _a - _b; });
            });

        std::vector<double> means1 = { 0.0,0.0,0.0,0.0 };
        scal = 1.0 / ws.size();
        std::for_each(ws.begin(), ws.end(), [&means1, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means1.begin(), means1.begin(), [scal](double _a, double _b) {_b += _a * scal; return _b; });
            });

        std::vector<double> vars = { 0.0,0.0,0.0,0.0 };
        scal = 1.0 / (ws.size() - 1);
        std::for_each(ws.begin(), ws.end(), [&vars, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), vars.begin(), vars.begin(), [scal](double _a, double _b) {_b += scal * std::pow(_a, 2.0); return _b; });
            });
        std::vector<double> stddevs = { 0.0,0.0,0.0,0.0 };
        std::transform(vars.begin(), vars.end(), stddevs.begin(), [](double _a) {return std::sqrt(_a); });
    }

    std::cout << "Hello World!\n";
}

