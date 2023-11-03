#pragma once

#include <vector>
#include <cassert>
#include <iterator>
#include <string>

#include "splitter.h"

struct DataObject {
    std::vector<std::vector<std::string>> content = {};
    std::vector<std::string> targets = {};

    Splitter splitter;

    DataObject() = default;
    //DataObject(const std::vector<std::vector<std::string>>& _content) {
    //    setContent(_content);
    //}

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
