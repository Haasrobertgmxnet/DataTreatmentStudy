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
#include "data_object.h"
#include "grouped_data.h"

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

const std::string MetaDataFileName = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\irisMetaData.txt";
const std::string CsvDataFileName = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\DataTreatmentStudy\\data\\iris.csv";

struct PredictionVsProperResults{
    std::string predictedValue;
    std::string properValue;
};

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
    DataTable testDataTable = dataTable.getTestDataTable();

    std::vector<std::string> targets = dataTable.getTargetNames();

    size_t samples = 0;
    std::map<std::string, GroupedDataItem> groupedData;
    for (auto it = targets.cbegin(); it != targets.cend(); ++it) {
        std::string s = *it;

        // Filtering
        auto vec = FilterFunc(trainDataTable.getTiedData(), [s, targetColumn](const std::vector<std::string>& _x) {return _x[targetColumn].find(s) != std::string::npos; });

        // Conversion
        groupedData[s].featureData = ConvFunc(vec, targetColumn);
        samples += groupedData[s].featureData.size();
    }

    for (auto&& item : groupedData) {
        std::cout << item.first << std::endl;
        item.second.resetMeansAndStdDevs(4);
        item.second.calcStdDeviations();
        item.second.setRelativeFrequency(samples);
        item.second.setProbabilityFunction();
    }

    std::vector< PredictionVsProperResults> predictedVsProperResults;
    std::map<std::string, std::function<double(std::vector<double>)>> funcs;
    std::transform(targets.cbegin(), targets.cend(), std::inserter(funcs, funcs.end()),
        [&groupedData](auto& _target) {
            return std::pair<std::string, std::function<double(std::vector<double>)>>(_target, groupedData[_target].probFunc);
        }
    );

    std::for_each(targets.cbegin(), targets.cend(), [&groupedData, &predictedVsProperResults, funcs](std::string _target) {
        std::transform(groupedData[_target].featureData.begin(), groupedData[_target].featureData.end(),
        std::back_inserter(predictedVsProperResults),
        [_target, &funcs](auto _item) {
                PredictionVsProperResults res;
                res.properValue = _target;
                res.predictedValue = maxLikelihoodEstimator(_item, funcs);
                return res;
            }
        );
        }
    );

    size_t correctlyPredicted = 0;

    std::for_each(predictedVsProperResults.cbegin(), predictedVsProperResults.cend(), [&correctlyPredicted](auto obj) {
        correctlyPredicted += (obj.properValue == obj.predictedValue) ? 1 : 0;
        });

    double accuracy = static_cast<double>(correctlyPredicted) / 120.0;
    printf("Correctly Predicted: %u\n", correctlyPredicted);
    std::cout << "Accuracy: " << accuracy << std::endl;

    std::cout << "End!\n";
}

