#pragma once

#include <vector>
#include <cassert>
#include <iterator>
#include <string>

#include "metadata.h"
#include "splitter.h"
#include "target_filter.h"
#include "feature_filter.h"

template<typename T>
T getTrainData(const T& _data, const std::vector<size_t>& _idcs) {
    T resData;
    resData.resize(_idcs.size());
    std::transform(_idcs.begin(), _idcs.end(), resData.begin(), [&_data](size_t j) {return _data[j]; });
    return resData;
}

struct DataTable {
public:

    DataTable() {

    }
    void setMetaData(const DataTableMetaData& _metaData) {
        metaData = _metaData;
    }

    void setData(const std::vector<std::vector<std::string>>& _rawData) {
        FeatureFilter featureFilter;
        std::vector<std::vector<std::string>>::const_iterator it = _rawData.cbegin();
        std::advance(it, metaData.getFirstLineToRead());
        filteredData = featureFilter.applyFilter(it, _rawData.cend());
        targets = TargetFilter::applyFilter(it, _rawData.cend(), metaData.getTargetColumn());
    }

    void testTrainSplit(size_t _idcs) {
        splitter.reset(filteredData.size());
        auto tn = getTargetNames();
        splitter.pickIdcsRandomly(_idcs, getTargetNames().size());
        splitter.removeIdcs();
    }

    std::vector <std::vector<std::string>> getTrainFeatureData() {
        return getTrainData<std::vector < std::vector<std::string>>>(filteredData, splitter.getIdcs().first);
    }

    std::vector<std::string> getTrainTargetData() {
        return getTrainData<std::vector<std::string>>(targets, splitter.getIdcs().first);
    }

    DataTable getTrainDataTable() {
        DataTable tData;
        DataTableMetaData metaData;
        metaData.firstLineToRead = 0;
        tData.filteredData = getTrainFeatureData();
        tData.targets = getTrainTargetData();
        return tData;
    };

    std::vector <std::vector<std::string>> getTestFeatureData() {
        return getTrainData<std::vector < std::vector<std::string>>>(filteredData, splitter.getIdcs().second);
    }

    std::vector<std::string> getTestTargetData() {
        return getTrainData<std::vector<std::string>>(targets, splitter.getIdcs().second);
    }

    DataTable getTestDataTable() {
        DataTable tData;
        DataTableMetaData metaData;
        metaData.firstLineToRead = 0;
        tData.filteredData = getTestFeatureData();
        tData.targets = getTestTargetData();
        return tData;
    };

    std::vector <std::vector<std::string>> getTiedData() {
        // std::for_each(filteredData.begin(), filteredData.end(), [](std::vector<std::string >& _s) {std::cout << _s[0] << std::endl; });
        std::vector <std::vector<std::string>> resData(filteredData.size());
        std::transform(filteredData.begin(), filteredData.end(), targets.begin(), resData.begin(),
            [](std::vector<std::string>& _a, std::string& _b) { _a.push_back(_b); return _a; /*std::cout << _b << std::endl; _a.push_back(_b);*/  });
        return std::move(resData);
    }

    std::vector<std::string> getTargetNames() {
        std::vector<std::string> res = {};
        std::for_each(targets.begin(), targets.end(), [this, &res](std::string _x) {if (!std::count(res.begin(), res.end(), _x)) res.push_back(_x); });
        return res;
    }

private:
    DataTableMetaData metaData;
    Splitter splitter;
    std::vector<std::vector<std::string>> filteredData;
    std::vector<std::string> targets;
    // std::vector<DataRecord> items;
};