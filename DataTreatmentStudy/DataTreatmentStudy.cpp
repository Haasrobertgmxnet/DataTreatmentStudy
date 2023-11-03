// DataTreatmentStudy.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//
#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <ranges>
#include <cassert>
#include <string>
#include <sstream>
#include <functional>

#include "splitter.h"


double logGaussianPDF(double x, double _mean, double _sd) {
    auto w = (x - _mean) / _sd;
    return -0.5 * w * w - std::log(std::sqrt(2. * M_PI * _sd));
};

double gaussianPDF(double x, double _mean, double _sd) {
    return std::exp(logGaussianPDF(x, _mean, _sd));
};

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

struct Filter {
    std::vector<std::string> targets = {};
    std::vector<std::vector<std::string>> trainData = {};


    Filter() = default;

    void setTargets(std::vector<std::string>& _targets) {
        std::copy(_targets.begin(), _targets.end(), std::back_inserter(targets));
    }



};

std::vector < std::vector<std::string>> FilterFunc(const std::vector<std::vector<std::string>>& _content, const std::function<bool(const std::vector<std::string>&)>& _func) {
    // Filtering
    auto view2 = _content | std::views::filter(_func);
    std::vector<std::vector<std::string>> vec = {};
    std::transform(view2.begin(), view2.end(), std::back_inserter(vec),
        [](const std::vector<std::string>& c) { return c; });
    return vec;
}

std::vector<std::vector<double>> ConvFunc(const std::vector<std::vector<std::string>>& _vec, size_t _targetColumn) {
    std::vector<std::vector<double>> vec1 = {};
    std::for_each(_vec.begin(), _vec.end(),
        [&vec1, _targetColumn](std::vector<std::string> c) {
            size_t j = 0;
            std::vector<double> vec2 = {};
            auto func = [&vec2](std::string _s) {
                try {
                    vec2.push_back(std::stod(_s));
                }
                catch (std::exception ex) {
                    // skip column, if its datatype cannot converted to a numeric format
                    // 
                }
            };
            std::for_each(c.begin(), c.begin() + _targetColumn, func);
            std::for_each(c.begin() + _targetColumn, c.end() - 1, func);
            vec1.push_back(vec2);
        });
    return vec1;
}

struct DataObject {
    std::vector<std::vector<std::string>> content = {};
    std::vector<std::string> targets = {};
    Splitter splitter;
    Filter filter;

    DataObject() = default;

    void setContent(const std::vector<std::vector<std::string>>& _content) {
        std::copy(_content.begin(), _content.end(), std::back_inserter(content));
        splitter.reset(content.size());
    }

    std::vector<std::string>& dataItem(std::size_t _i, std::vector<size_t> _ws) {
        assert(_i < _ws.size());
        return content[_ws[_i]];
    }

    std::vector<std::string>& trainData(std::size_t _i) {
        return dataItem(_i, splitter.getIdcs().first);
    }

    std::vector<std::string>& testData(std::size_t _i) {
        return dataItem(_i, splitter.getIdcs().second);
    }

    constexpr size_t getTrainDataSize() {
        return splitter.getIdcs().first.size();
    }

    constexpr size_t getTestDataSize() {
        return splitter.getIdcs().second.size();
    }

    void setData(std::vector < std::vector<std::string>>& _tData, std::vector<size_t >& _ws) {
        _tData.resize(0);
        for (auto it = _ws.cbegin(); it != _ws.cend(); ++it) {
            _tData.push_back(content[*it]);
        }
    }

    std::vector < std::vector<std::string>>& allTrainData(std::vector < std::vector<std::string>>& _tData) {
        setData(_tData, splitter.getIdcs().first);
        return _tData;
    }

    std::vector <std::vector<std::string>> getAllTrainData() {
        std::vector < std::vector<std::string>> tData;
        setData(tData, splitter.getIdcs().first);
        return tData;
    }

    void allTestData(std::vector < std::vector<std::string>>& _tData) {
        setData(_tData, splitter.getIdcs().second);
    }

    std::vector<std::string> getTargets() const {
        return targets;
    }

};

struct GroupedDataItem {
    std::vector<std::vector<double>> featureData;
    std::vector<double> means;
    std::vector<double> stddevs;
    std::function<double(std::vector<double>)> probFunc;
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

    DataObject dataObj;
    dataObj.setContent(content);
    dataObj.splitter.pickIdcsRandomly(30);
    dataObj.splitter.removeIdcs();

    std::map<std::string, std::vector<std::vector<double>>> _groupedData;
    std::map<std::string, GroupedDataItem> groupedData;

    for (auto it = targets.cbegin(); it != targets.cend(); ++it) {
        std::string s = *it;

        // Filtering
        auto vec = FilterFunc(dataObj.getAllTrainData(), [s, targetColumn](const std::vector<std::string>& _x) {return _x[targetColumn].find(s) != std::string::npos; });

        // Conversion
        _groupedData[s] = ConvFunc(vec, targetColumn);
        groupedData[s].featureData = ConvFunc(vec, targetColumn);
    }

    for (auto&& item : groupedData) {
        std::cout << item.first << std::endl;
        std::vector<double> means = { 0.0,0.0,0.0,0.0 };
        double scal = 1.0 / item.second.featureData.size();
        std::for_each(item.second.featureData.begin(), item.second.featureData.end(), [&means, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), means.begin(), [scal](double _a, double _b) {_b += _a * scal; return _b; });
            });
        std::vector<std::vector<double>> ws(item.second.featureData);
        std::for_each(ws.begin(), ws.end(), [&means](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), _x.begin(), [](double _a, double _b) {return _a - _b; });
            });

        std::vector<double> vars = { 0.0,0.0,0.0,0.0 };
        scal = 1.0 / (ws.size() - 1);
        std::for_each(ws.begin(), ws.end(), [&vars, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), vars.begin(), vars.begin(), [scal](double _a, double _b) {_b += scal * std::pow(_a, 2.0); return _b; });
            });
        std::vector<double> stddevs = { 0.0,0.0,0.0,0.0 };
        std::transform(vars.begin(), vars.end(), stddevs.begin(), [](double _a) {return std::sqrt(_a); });
        std::copy(means.begin(), means.end(), std::back_inserter(item.second.means));
        std::copy(stddevs.begin(), stddevs.end(), std::back_inserter(item.second.stddevs));


        double relativeFrequency = item.second.featureData.size() / 120.0;

        //auto prob = [&means, &stddevs, relativeFrequency](std::vector<double> _x) -> double {
        //    double res = 1.0;
        //    std::vector<double> ws = {};
        //    std::vector<double> ws1 = {};
        //    std::transform(_x.begin(), _x.end(), means.begin(), ws.begin(), [](double _a, double _b) { return _a - _b; });
        //    std::transform(ws.begin(), ws.end(), stddevs.begin(), ws1.begin(), [](double _a, double _b) { return gaussianPDF(_a, 0, _b); });
        //    std::for_each(ws1.begin(), ws1.end(), [&res](double _x) {res *= _x; });
        //    return res * relativeFrequency;
        //};

        //auto prob = [&item, relativeFrequency](std::vector<double> _x) -> double {
        //    double res = 1.0;
        //    std::vector<double> ws = {};
        //    std::vector<double> ws1 = {};

        //    
        //    std::transform(_x.begin(), _x.end(), item.second.means.begin(), ws.begin(), [&ws](double _a, double _b) { return  _a - _b; });
        //    std::transform(ws.begin(), ws.end(), item.second.stddevs.begin(), ws1.begin(), [](double _a, double _b) { return gaussianPDF(_a, 0, _b); });
        //    std::for_each(ws1.begin(), ws1.end(), [&res](double _x) {res *= _x; });
        //    return res * relativeFrequency;
        //};

        auto prob = [&item, relativeFrequency](std::vector<double> _x) -> double {
            double res = 1.0;
            for (auto j = 0; j < 4; ++j) {
                res *= gaussianPDF(_x[j], item.second.means[j], item.second.stddevs[j]);
            }
            return res * relativeFrequency;
        };
        item.second.probFunc = prob;
    }

    std::vector<std::string> keys = { "\"Setosa\"" , "\"Versicolor\"" , "\"Virginica\"" };

    for (auto key : keys) {
        std::cout << "Key: " << key << std::endl;
        std::for_each(groupedData[key].featureData.begin(), groupedData[key].featureData.end(),
            [&groupedData, &keys, key](const std::vector<double>& _x) {
                double prob = 0.0;
                std::string selection = "";
                //std::for_each(keys.begin(), keys.end(), [&groupedData, _x](const std::string& _s) {std::cout << "\tKey: " << _s; });
                std::for_each(keys.begin(), keys.end(), [&groupedData, &prob, &selection, _x](const std::string& _s) {
                    double tmp = groupedData[_s].probFunc(_x);
                    selection = (tmp > prob) ? _s : selection;
                    prob = (tmp > prob) ? tmp : prob;
                    // std::cout << "\tKey: " << _s << "\tValue: " << tmp; 
                    });
                std::cout << std::endl;
                std::cout << "Key: " << key << "\tVote: " << selection << "\tProbability: " << prob << std::endl;
            }
        );
    }

    std::cout << "End!\n";
}

