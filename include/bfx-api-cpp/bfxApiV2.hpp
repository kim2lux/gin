#pragma once

#include "bfxConf.hpp"

namespace BfxAPI
{

class bitfinexAPIv2 : public BfxConf
{
  public:
    explicit bitfinexAPIv2(const string &accessKey, const string &secretKey) : BfxConf("https://api.bitfinex.com/v2", "withdraw.conf",
                                                                                       BfxClientErrors::noError, accessKey, secretKey)
    {
    }

    bool check()
    {
        if (!hasApiError()) {
            cerr << strResponse() << endl;
            return true;
        }
        else
        {
            // see bfxERR enum in BitfinexAPI.hpp::BitfinexAPI
//            cerr << "BfxApiStatusCode: ";
  //          cerr << getBfxApiStatusCode() << endl;
            // see https://curl.haxx.se/libcurl/c/libcurl-errors.html
      //      cerr << "CurlStatusCode: ";
    //        cerr << getCurlStatusCode() << endl;
            return false;
        }
    }

    int getStatus()
    {
        Request.get("/platform/status");
        if (check() == false)
            exit (0);
        Document document;
        document.Parse(strResponse().c_str());
        Value &data = document;
        return data[0].GetInt();
    }
};
} // namespace BfxAPI