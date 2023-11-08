#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

double logGaussianPDF(double x, double _mean, double _sd) {
    auto w = (x - _mean) / _sd;
    return -0.5 * w * w - std::log(std::sqrt(2. * M_PI * _sd));
};

double gaussianPDF(double x, double _mean, double _sd) {
    return std::exp(logGaussianPDF(x, _mean, _sd));
};

struct GroupedDataItem {

    void calcMeans() {
        double scal = 1.0 / featureData.size();
        std::fill(means.begin(), means.end(), 0);
        std::for_each(featureData.begin(), featureData.end(), [this, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), means.begin(), [scal](double _a, double _b) {_b += _a * scal; return _b; });
            });

    }

    void calcStdDeviations() {
        calcMeans();
        double scal = 1.0 / (featureData.size() - 1.0);
        std::vector<double> vars = { 0.0,0.0,0.0,0.0 };
        std::vector<std::vector<double>> ws(featureData);
        std::for_each(ws.begin(), ws.end(), [this](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), means.begin(), _x.begin(), [](double _a, double _b) {return _a - _b; });
            });
        std::for_each(ws.begin(), ws.end(), [&vars, scal](std::vector<double>& _x) {
            std::transform(_x.begin(), _x.end(), vars.begin(), vars.begin(), [scal](double _a, double _b) {_b += scal * std::pow(_a, 2.0); return _b; });
            });
        std::transform(vars.begin(), vars.end(), stddevs.begin(), [](double _a) {return std::sqrt(_a); });
    }

    void resetMeansAndStdDevs(size_t _n) {
        means.resize(_n);
        stddevs.resize(_n);
    }

    void setRelativeFrequency(size_t _n) {
        relativeFrequency = static_cast<double>(featureData.size()) / _n;
    }

    void setProbabilityFunction(std::function<double(double, double, double)> _probF = gaussianPDF) {
        calcStdDeviations();
        probFunc = [this, _probF](std::vector<double> _x) -> double {
            double res = 1.0;
            for (auto j = 0; j < 4; ++j) {
                res *= _probF(_x[j], this->means[j], this->stddevs[j]);
            }
            return res * this->relativeFrequency;
            };
    }

    std::vector<std::vector<double>> featureData;
    std::vector<double> means;
    std::vector<double> stddevs;
    double relativeFrequency = 0.0;
    std::function<double(std::vector<double>)> probFunc;
};
