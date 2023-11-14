#pragma once



#include <cmath> // sin()
#include <numbers> // the pi
#include <vector>
#include <algorithm>
#include <functional>

double logGaussianPDF(double x, double _mean, double _sd) {
    auto w = (x - _mean) / _sd;
    return -0.5 * w * w - std::log(std::sqrt(2. * std::numbers::pi * _sd));
};

double gaussianPDF(double x, double _mean, double _sd) {
    return std::exp(logGaussianPDF(x, _mean, _sd));
};

double GaussianKernel(double x) {
    return gaussianPDF(x, 0.0, 1.0);
}

double EpanechnikovKernel(double x) {
    if (std::abs(x) > 1.0) {
        return 0.0;
    }
    return 0.75 * (1.0 - std::pow(x, 2.0));
}

class ProbabilityDensity {
public:
    enum class Mode { GaussianPDF, LogGaussianPDF, GaussianKDE, EpanechnikovKDE };

    explicit ProbabilityDensity(const std::vector<double>& w, Mode m = Mode::GaussianPDF)
        : mode{ m }, data{w}
    {
        double scal = 1.0;
        double vars = 0.0;
        scal = 1.0 / data.size();
        std::for_each(data.begin(), data.end(), [this, scal](double _x) { mean += _x * scal; });
        // std dev
        scal = 1.0 / (data.size() - 1);

        std::for_each(data.begin(), data.end(), [this, &vars, scal](double _x) { vars += std::pow(_x - mean, 2.0) * scal; });
        stddev = std::sqrt(vars);
        switch (mode) {
        case Mode::GaussianPDF:
            probDensity = [this](double x) {return gaussianPDF(x, mean, stddev); };
            break;
        case Mode::LogGaussianPDF:
            probDensity = [this](double x) {return logGaussianPDF(x, mean, stddev); };
            break;
        case Mode::GaussianKDE:
            kernel = GaussianKernel;
            probDensity = [this](double x) {
                double res = 0.0;
                std::for_each(data.begin(), data.end(), [x, &res, this](double x0) {
                    res += GaussianKernel((x - x0) / h);
                    }
                );
                return res / h / data.size();
                };
            break;
        case Mode::EpanechnikovKDE:
            kernel = EpanechnikovKernel;
            probDensity = [this](double x) {
                double res = 0.0;
                std::for_each(data.begin(), data.end(), [x, &res, this](double x0) {
                    res += kernel((x - x0) / h);
                    }
                );
                return res / h / data.size();
                };
            break;
        }
    }

    void setBandWidth(double _h) {
        h = _h;
    }

    [[nodiscard]] double operator()(double arg) const
    {
        double erg = NAN; // not a number
        switch (mode) {
        case Mode::GaussianPDF:
            return gaussianPDF(arg, mean, stddev);
        case Mode::LogGaussianPDF:
            return logGaussianPDF(arg, mean, stddev);
        case Mode::GaussianKDE:
            return probDensity(arg);
            erg = 0.0;
            break;
        case Mode::EpanechnikovKDE:
            return probDensity(arg);
            erg = 0.0;
            break;
        }
        return erg;
    }
private:
    Mode mode;
    std::vector<double> data;
    double mean = 0.0;
    double stddev = 0.0;
    std::function<double(double)> probDensity;
    std::function<double(double)> kernel;
    double h = 0.5;
};