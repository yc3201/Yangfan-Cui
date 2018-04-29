//
//  Yahoo.h
//  Project_team6
//
//  Created by LIANGLINQIAN on 2017/12/7.
//  Copyright © 2017年 LIANGLINQIAN. All rights reserved.
//

#ifndef Yahoo_h
#define Yahoo_h
#include "Stock1.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <locale>
#include <iomanip>
#include <stdlib.h>
#include <thread>
#include <curl/curl.h>
using namespace std;
class Yahoo
{
public:
    CURL *handle;
    CURLcode result;
    string sCrumb;
    string sCookies;
    Yahoo(){curl_global_init(CURL_GLOBAL_ALL);handle = curl_easy_init();}
    ~Yahoo(){curl_easy_cleanup(handle);curl_global_cleanup();}
    void GetData(map<string, Stock> &Data);
    int GetData2(string market, string startdate, string enddate, vector<string>& Date, vector<double>& AdjClose);
    
};


#endif /* Yahoo_h */
