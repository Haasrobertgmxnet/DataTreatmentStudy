#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

#include "prob_func.h"

std::string maxLikelihoodEstimator(std::vector<double> x, std::map<std::string, std::function<double(std::vector<double>)>> _m) {
    std::map<std::string, double> resMap;
    std::transform(_m.begin(), _m.end(), std::inserter(resMap, resMap.end()), [x](std::pair <std::string, std::function<double(std::vector<double>)>> _kvp) {
        return std::pair <std::string, double>(_kvp.first, _kvp.second(x));
        });
    auto res = std::max_element(resMap.begin(), resMap.end(), [](auto a, auto b) {
        return a.second < b.second;
        });
    return res->first;
};

struct GroupedDataItem {

    std::vector<double> getColumn(const size_t _col) const {
        if (_col >= 4) {
            return std::vector<double>({});
        }
        std::vector<double> res;
        std::transform(featureData.cbegin(), featureData.cend(), std::back_inserter(res), [_col](std::vector<double> _w) { return _w[_col]; });
        return res;
    }

    //void calcMeans() {
    //    double scal = 1.0 / featureData.size();
    //    std::fill(means.begin(), means.end(), 0);
    //    std::for_each(featureData.begin(), featureData.end(), [this, scal](std::vector<double>& _x) {
    //        std::transform(_x.begin(), _x.end(), means.begin(), means.begin(), [scal](double _a, double _b) {_b += _a * scal; return _b; });
    //        });

    //}

    //void calcStdDeviations() {
    //    calcMeans();
    //    double scal = 1.0 / (featureData.size() - 1.0);
    //    std::vector<double> vars = { 0.0,0.0,0.0,0.0 };
    //    std::vector<std::vector<double>> ws(featureData);
    //    std::for_each(ws.begin(), ws.end(), [this](std::vector<double>& _x) {
    //        std::transform(_x.begin(), _x.end(), means.begin(), _x.begin(), [](double _a, double _b) {return _a - _b; });
    //        });
    //    std::for_each(ws.begin(), ws.end(), [&vars, scal](std::vector<double>& _x) {
    //        std::transform(_x.begin(), _x.end(), vars.begin(), vars.begin(), [scal](double _a, double _b) {_b += scal * std::pow(_a, 2.0); return _b; });
    //        });
    //    std::transform(vars.begin(), vars.end(), stddevs.begin(), [](double _a) {return std::sqrt(_a); });
    //}

    //void resetMeansAndStdDevs(size_t _n) {
    //    means.resize(_n);
    //    stddevs.resize(_n);
    //}

    void setRelativeFrequency(size_t _n) {
        relativeFrequency = static_cast<double>(featureData.size()) / _n;
    }

    //void setProbabilityFunction1(std::function<double(double, double, double)> _probF = gaussianPDF) {
    //    calcStdDeviations();
    //    probFunc = [this, _probF](std::vector<double> _x) -> double {
    //        double res = 1.0;
    //        for (auto j = 0; j < 4; ++j) {
    //            res *= _probF(_x[j], this->means[j], this->stddevs[j]);
    //        }
    //        return res * this->relativeFrequency;
    //        };
    //}

    void setProbabilityFunction(ProbabilityDensity::Mode _mode = ProbabilityDensity::Mode::GaussianPDF) {
        probFunc = [this, _mode](std::vector<double> _x) -> double {
            double res = 1.0;
            for (auto j = 0; j < 4; ++j) {
                ProbabilityDensity probabilityDensity(getColumn(j), _mode);
                probabilityDensity.setBandWidth(0.1);
                res *= probabilityDensity(_x[j]);
            }
            return res * this->relativeFrequency;
            };
    }

    std::vector<std::vector<double>> featureData;
    //std::vector<double> means;
    //std::vector<double> stddevs;
    double relativeFrequency = 0.0;
    std::function<double(std::vector<double>)> probFunc;
    ProbabilityDensity::Mode mode = ProbabilityDensity::Mode::GaussianPDF;
};
