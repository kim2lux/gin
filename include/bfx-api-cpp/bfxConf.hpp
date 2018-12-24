#pragma once

// std
#include <chrono>
#include <fstream>
#include <iostream>
#include <iostream>
#include <map>
#include <regex>
#include <unordered_set>
#include <utility>
#include <vector>

// internal jsonutils
#include "jsonutils.hpp"

// internal error
#include "error.hpp"

// internal HTTPRequest
#include "HTTPRequest.hpp"

// namespaces
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::unordered_set;
using std::vector;
#include "rapidjson/document.h"
using namespace rapidjson;
// ...
#include <limits>
typedef std::numeric_limits< double > dbl;

namespace BfxAPI
{

    class BfxConf {
    
    public:
        BfxConf(const std::string & apiUrl, std::string filePath, enum BfxClientErrors error,
        const std::string & accessKey, const std::string & secretKey)
        : WDconfFilePath_(filePath), Request(apiUrl), bfxApiStatusCode_(error) {
            Request.setAccessKey(accessKey);
            Request.setSecretKey(secretKey);
        }

        const string getWDconfFilePath() const noexcept
        { return WDconfFilePath_; }

        const BfxClientErrors& getBfxApiStatusCode() const noexcept
        { return bfxApiStatusCode_; }

        const CURLcode getCurlStatusCode() const noexcept
        { return Request.getLastStatusCode(); }

        const string strResponse() const noexcept
        { return Request.getLastResponse(); }

        bool hasApiError()
        {
            return (checkErrors() != noError || Request.hasError());
        }

        // Setters
        void setWDconfFilePath(const string &path) noexcept
        { WDconfFilePath_ = path; }

        void setKeys(const string &accessKey, const string &secretKey) noexcept
        {
            Request.setAccessKey(accessKey);
            Request.setSecretKey(secretKey);
        }
        BfxClientErrors checkErrors() {
            bfxApiStatusCode_ = Request.hasError()
                ? curlERR
                : schemaValidator_.validateSchema(
                    Request.getLastPath(),
                    Request.getLastResponse()
                );
            return bfxApiStatusCode_;
        }
    public:
        // containers with supported parameters
        unordered_set<string> symbols_; // valid symbol pairs
        unordered_set<string> currencies_; // valid currencies
        unordered_set<string> methods_; // valid deposit methods
        unordered_set<string> walletNames_; // valid walletTypes
        unordered_set<string> types_; // valid Types (see new order endpoint)
        // BitfinexAPI settings
        string WDconfFilePath_;
        // internal jsonutils instances
        jsonutils::BfxSchemaValidator schemaValidator_;
        // internal HTTPRequest instance
        HTTPRequest Request;
        // dynamic and status variables
        BfxClientErrors bfxApiStatusCode_;
    };
}