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
#include <iterator>
#include <sstream>
#include <functional>
#include <cstdint>

#include "getcsvcontent.h"
#include "metadata.h"

//#include "splitter.h"
//#include "feature_filter.h"
#include "data_object.h"

double logGaussianPDF(double x, double _mean, double _sd) {
    auto w = (x - _mean) / _sd;
    return -0.5 * w * w - std::log(std::sqrt(2. * M_PI * _sd));
};

double gaussianPDF(double x, double _mean, double _sd) {
    return std::exp(logGaussianPDF(x, _mean, _sd));
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
                }
            };
            std::for_each(c.begin(), c.begin() + _targetColumn, func);
            std::for_each(c.begin() + _targetColumn, c.end() - 1, func);
            vec1.push_back(vec2);
        });
    return vec1;
}

struct DataRecord {
public:

private:
    uint16_t target;
    std::vector<double> features;
};



struct GroupedDataItem {
    std::vector<std::vector<double>> featureData;
    std::vector<double> means;
    std::vector<double> stddevs;
    std::function<double(std::vector<double>)> probFunc;
};

const std::string MetaDataFileName = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\irisMetaData.txt";
const std::string CsvDataFileName = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\iris.csv";

int main()
{
    std::vector<std::vector<std::string>> content = getCsvContent(CsvDataFileName);

    DataTableMetaData dataTableMetaData;
    dataTableMetaData.setMetaData(MetaDataFileName);
    const size_t targetColumn = dataTableMetaData.getTargetColumn();
    const size_t firstLineToRead = dataTableMetaData.getFirstLineToRead();

    DataTable dataTable;
    dataTable.setMetaData(dataTableMetaData);
    dataTable.setData(content);
    dataTable.testTrainSplit(30);
    DataTable trainDataTable = dataTable.getTrainDataTable();
    
    content.erase(content.begin(), content.begin() + firstLineToRead);

    std::vector<std::string> targets = {};
    std::for_each(content.begin(), content.end(), [&targets, targetColumn](const std::vector<std::string>& _x) {if (!std::count(targets.begin(), targets.end(), _x[targetColumn])) targets.push_back(_x[targetColumn]); });

    std::map<std::string, GroupedDataItem> groupedData;
    for (auto it = targets.cbegin(); it != targets.cend(); ++it) {
        std::string s = *it;

        // Filtering
        auto vec = FilterFunc(trainDataTable.getTiedData(), [s, targetColumn](const std::vector<std::string>& _x) {return _x[targetColumn].find(s) != std::string::npos; });

        // Conversion
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
                std::for_each(keys.begin(), keys.end(), [&groupedData, &prob, &selection, _x](const std::string& _s) {
                    double tmp = groupedData[_s].probFunc(_x);
                    selection = (tmp > prob) ? _s : selection;
                    prob = (tmp > prob) ? tmp : prob;
                    });
                std::cout << std::endl;
                std::cout << "Key: " << key << "\tVote: " << selection << "\tProbability: " << prob << std::endl;
            }
        );
    }

    std::cout << "End!\n";
}

