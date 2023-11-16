// ProbFuncTest.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "getcsvcontent.h"
#include "prob_func.h"

const std::string FileName1 = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample1.txt";
const std::string FileName2 = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample2.txt";
const std::string FileName3 = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample3.txt";
const std::string FileName4 = "C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample4.txt";

int main()
{
    std::vector<double> data;
    {
        auto FileName = FileName1;
        std::vector<std::vector<std::string>> content = getCsvContent(FileName);
        std::for_each(content.cbegin(), content.cend(), [&data](std::vector<std::string> w) {
            std::transform(w.cbegin(), w.cend(), std::back_inserter(data), [](std::string s) {
                return std::stod(s);
                });
            });
    }

    std::vector<double> data1;
    {
        auto FileName = FileName3;
        std::vector<std::vector<std::string>> content = getCsvContent(FileName);
        std::for_each(content.cbegin(), content.cend(), [&data1](std::vector<std::string> w) {
            std::transform(w.cbegin(), w.cend(), std::back_inserter(data1), [](std::string s) {
                return std::stod(s);
                });
            });
    }

    std::vector<double> data2;
    {
        auto FileName = FileName4;
        std::vector<std::vector<std::string>> content = getCsvContent(FileName);
        std::for_each(content.cbegin(), content.cend(), [&data2](std::vector<std::string> w) {
            std::transform(w.cbegin(), w.cend(), std::back_inserter(data2), [](std::string s) {
                return std::stod(s);
                });
            });
    }
    
    std::vector<std::vector<double>> tableData;
    {
        auto FileName = FileName2;
        std::vector<std::vector<std::string>> content = getCsvContent(FileName, '\t');
        std::for_each(content.cbegin() + 2, content.cend(), [&tableData](std::vector<std::string> w) {
            std::vector<double> line;
            std::transform(w.cbegin(), w.cbegin() + 3, std::back_inserter(line), [](std::string s) {
                return std::stod(s);
                });
            tableData.push_back(line);
            });
    }

    ProbabilityDensity probabilityDensity1(data, ProbabilityDensity::Mode::GaussianKDE);
    probabilityDensity1.setBandWidth(40.0);
    ProbabilityDensity probabilityDensity2(data, ProbabilityDensity::Mode::EpanechnikovKDE);
    probabilityDensity2.setBandWidth(40.0);

    std::vector<double> vec1;
    std::vector<double> vec2;
    std::for_each(tableData.cbegin(), tableData.cend(), [&vec1, &vec2, &probabilityDensity1, &probabilityDensity2](std::vector<double> w) {
        vec1.push_back(probabilityDensity1(w[0]));
        vec2.push_back(probabilityDensity2(w[0]));
        // diff2.push_back(w[1] - probabilityDensity2(w[0]));
        });

    std::vector<double> diff1;
    std::vector<double> diff2;
    std::cout << "Differences: " << std::endl;
    for (size_t j = 0; j < data1.size(); ++j) {
        diff1.push_back(std::abs(vec1[j] - data1[j]));
        diff2.push_back(std::abs(vec2[j] - data2[j]));
    }

    for (size_t j = 0; j < diff1.size(); ++j) {
        std::cout << "Diff 1: " << diff1[j] << "    Diff 2: " << diff2[j] << std::endl;
    }

    std::cout << "Maximal Diff: " << std::endl;
    std::cout << "Max Diff 1: " << *std::max_element(diff1.begin(),diff1.end()) << "    Max Diff 2: " << *std::max_element(diff2.begin(), diff2.end()) << std::endl;
    for (auto&& bw : { 5.0, 5.2, 5.4, 5.6, 5.0, 6.0, 6.2, 6.4, 6.6, 100.0, 200.0, 40.0 }) {
        ProbabilityDensity probabilityDensity(data, ProbabilityDensity::Mode::EpanechnikovKDE);
        probabilityDensity.setBandWidth(bw);
        std::cout << "Bandwidth: " << bw << "   Value: " << probabilityDensity(122.254403131115) << std::endl;
    }

    ProbabilityDensity probabilityDensity(data, ProbabilityDensity::Mode::EpanechnikovKDE);
    probabilityDensity.setBandWidth(40.0);
    std::for_each(tableData.cbegin(), tableData.cend(), [&probabilityDensity](std::vector<double> w) {printf("%f\t%.20f\n", w[0] , probabilityDensity(w[0])); });
    std::cout << "Hello World!\n";
}
