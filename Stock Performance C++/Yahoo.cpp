//
//  Yahoo.cpp
//  Project_team6
//
//  Created by LIANGLINQIAN on 2017/12/7.
//  Copyright © 2017年 LIANGLINQIAN. All rights reserved.
//

#include <stdio.h>
#include "Yahoo.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <locale>
#include <iomanip>
#include <stdlib.h>
#include <mutex>
#include <curl/curl.h>
using namespace std;

std::mutex mtx;
size_t write_data(void *ptr, int size, int nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

struct MemoryStruct {
    char *memory;
    size_t size;
};

void *myrealloc(void *ptr, size_t size)
{
    /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
    if (ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}


size_t write_data2(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)data;
    
    mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

string getTimeinSeconds(string Time)//change function parameter to find N days before and later
{
    string hour="T16:00:00";
    Time = Time+hour;
    std::tm t = {0};
    std::istringstream ssTime(Time);
    char time[100];
    memset(time, 0, 100);
    //change time format and check business day
    if(ssTime >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S"))
    {
        //cout << std::put_time(&t, "%c %Z") << "\n"
        //<< std::mktime(&t) << "\n";
        sprintf (time, "%ld", mktime(&t));
        return string(time);
    }
    else
    {
        cout << "Parse failed\n";
        return "";
    }
}

int Yahoo::GetData2(string market, string startdate, string enddate, vector<string>& Date, vector<double>& AdjClose)
{
    string symbol = market;
    string startDate = startdate;
    string endDate = enddate;
    cout<< "Stock Ticker : "<<symbol<< endl;
    string startTime = getTimeinSeconds(startDate);
    string endTime = getTimeinSeconds(endDate);
    struct MemoryStruct data;
    data.memory = NULL;
    data.size = 0;
        
    //file pointer to create file that store the data
    FILE *fp;
    
    if (handle)
    {
        if (sCookies.length() == 0 || sCrumb.length() == 0)
        {
            curl_easy_setopt(handle, CURLOPT_URL, "https://finance.yahoo.com/quote/AMZN/history?p=AMZN");
            curl_easy_setopt(handle, CURLOPT_URL, "https://finance.yahoo.com/quote/AMZN/history");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(handle, CURLOPT_COOKIEJAR, "cookies.txt");
            curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "cookies.txt");
                
            curl_easy_setopt(handle, CURLOPT_ENCODING, "");
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);
                
            /* perform, then store the expected code in 'result'*/
            result = curl_easy_perform(handle);
                
            /* Check for errors */
            if (result != CURLE_OK)
            {
                /* if errors have occured, tell us what is wrong with 'result'*/
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
                return 1;
            }
                
            char cKey[] = "CrumbStore\":{\"crumb\":\"";
            char *ptr1 = strstr(data.memory, cKey);
            char *ptr2 = ptr1 + strlen(cKey);
            char *ptr3 = strstr(ptr2, "\"}");
            if ( ptr3 != NULL )
                *ptr3 = NULL;
                
            sCrumb = ptr2;
                
            fp = fopen("cookies.txt", "r");
            char cCookies[100];
            if (fp) {
                while (fscanf(fp, "%s", cCookies) != EOF);
                fclose(fp);}
            else
                cerr << "cookies.txt does not exists" << endl;
                
            sCookies = cCookies;
            free(data.memory);
            data.memory = NULL;
            data.size= 0;
            }
            
            string urlA = "https://query1.finance.yahoo.com/v7/finance/download/";
            string urlB = "?period1=";
            string urlC = "&period2=";
            string urlD = "&interval=1d&events=history&crumb=";
            string url = urlA + symbol + urlB + startTime + urlC + endTime + urlD + sCrumb;
            const char * cURL = url.c_str();
            const char * cookies = sCookies.c_str();
            curl_easy_setopt(handle, CURLOPT_COOKIE, cookies);
            curl_easy_setopt(handle, CURLOPT_URL, cURL);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);
            result = curl_easy_perform(handle);
            
            /* Check for errors */
            if (result != CURLE_OK)
            {
                /* if errors have occurred, tell us what is wrong with 'result', and adjust close set to be zero*/
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
                return 1;
            }
            stringstream sData;
            sData.str(data.memory);
            string line;
            getline(sData, line);
            //cout << line << endl;
            //get adjclose
            for (; getline(sData, line); )
            {
                if(line.length()!=0){
                    int counter = 0;
                    string adjclose;
                    string date;
                    //cout << line << endl;
                    // find adjclose
                    for(int i=0;i<line.length();i++)
                    {
                        if(line[i]==',')
                            counter++;
                        if (counter==0&&line[i]!=',')
                         {
                            date+=line[i];
                         }
                        if(counter==5&&line[i]!=',')
                        {
                            adjclose+=line[i];
                        }
                    }
                    //change string to double and take off no value data
                    //cout<<date<<endl;
                    if (adjclose != "null")
                    {double value = stod(adjclose);
                        //input into vector
                        AdjClose.push_back(value);
                        Date.push_back(date);
                    }
                    else
                    {
                        cout<<"today data is wrong"<<endl;
                        continue;
                    }}
                else
                {
                    //if return blank, recatch the data
                    cout<<"recatch this date"<<endl;
                    GetData2(market,startdate,enddate, Date, AdjClose);
                }
            }
            cout<<"  parsing successfully"<<endl;
            cout<<"size of adjclose ="<<AdjClose.size()<<endl;
            free(data.memory);
            data.size= 0;
        }
        else
        {
            fprintf(stderr, "Curl init failed!\n");
            return 1;
        }
    return 0;

}

void Yahoo::GetData(map<string, Stock> &Data)
{
    map<string, Stock>::iterator itr;
    for(itr=Data.begin();itr!=Data.end();){
        vector<double> AdjClose;
        string symbol =itr->second.get_name();
        string startDate = itr->second.get_startdate();
        string endDate = itr->second.get_enddate();
        cout<< "Stock Ticker : "<<symbol;
        string startTime = getTimeinSeconds(startDate);
        string endTime = getTimeinSeconds(endDate);
        struct MemoryStruct data;
        data.memory = NULL;
        data.size = 0;
    
    //file pointer to create file that store the data
        FILE *fp;
    
    /* if everything's all right with the easy handle... */
    if (handle)
    {
        //while (true)
        //{
        if (sCookies.length() == 0 || sCrumb.length() == 0)
        {
            //fp = fopen(outfilename, "w");
            curl_easy_setopt(handle, CURLOPT_URL, "https://finance.yahoo.com/quote/AMZN/history?p=AMZN");
            curl_easy_setopt(handle, CURLOPT_URL, "https://finance.yahoo.com/quote/AMZN/history");
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(handle, CURLOPT_COOKIEJAR, "cookies.txt");
            curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "cookies.txt");
            
            curl_easy_setopt(handle, CURLOPT_ENCODING, "");
            
             /*Check for errors */
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);
            
            /* perform, then store the expected code in 'result'*/
            result = curl_easy_perform(handle);
            
            /* Check for errors */
            if (result != CURLE_OK)
            {
                /* if errors have occured, tell us what is wrong with 'result'*/
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
                return;
            }
            
            char cKey[] = "CrumbStore\":{\"crumb\":\"";
            char *ptr1 = strstr(data.memory, cKey);
            char *ptr2 = ptr1 + strlen(cKey);
            char *ptr3 = strstr(ptr2, "\"}");
            if ( ptr3 != NULL )
                *ptr3 = NULL;
            
            sCrumb = ptr2;
            
            fp = fopen("cookies.txt", "r");
            char cCookies[100];
            if (fp) {
                while (fscanf(fp, "%s", cCookies) != EOF);
                fclose(fp);
            }
            else
                cerr << "cookies.txt does not exists" << endl;
            
            sCookies = cCookies;
            free(data.memory);
            data.memory = NULL;
            data.size= 0;
        }
        
        //if (itr == symbolList.end())
        // break;
        
        string urlA = "https://query1.finance.yahoo.com/v7/finance/download/";
        string urlB = "?period1=";
        string urlC = "&period2=";
        string urlD = "&interval=1d&events=history&crumb=";
        string url = urlA + symbol + urlB + startTime + urlC + endTime + urlD + sCrumb;
        const char * cURL = url.c_str();
        const char * cookies = sCookies.c_str();
        curl_easy_setopt(handle, CURLOPT_COOKIE, cookies);   // Only needed for 1st stock
        curl_easy_setopt(handle, CURLOPT_URL, cURL);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&data);
        result = curl_easy_perform(handle);
        
        /* Check for errors */
        if (result != CURLE_OK)
        {
            /* if errors have occurred, tell us what is wrong with 'result', and adjust close set to be zero*/
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
            break;
        }
        stringstream sData;
        sData.str(data.memory);
        string line;
        getline(sData, line);
        //cout << line << endl;
        
        //get adjclose
        for (; getline(sData, line);)
        {
            int counter = 0;
            string adjclose;
            //cout << line << endl;
            // find adjclose
            for(int i=0;i<line.length();i++)
            {
                if(line[i]==',')
                    counter++;
                if(counter==5&&line[i]!=',')
                {
                    adjclose+=line[i];
                }
            }
            //change string to double and take off no value data
            if (adjclose != "null"&& adjclose.length()!=0)
            {
                double value = stod(adjclose);
                //input into vector
                AdjClose.push_back(value);
            }
            else
            {break;}
        }
        free(data.memory);
        data.size= 0;
    }
    else
    {
        fprintf(stderr, "Curl init failed!\n");
        return;
    }
        
        if(AdjClose.size()!=0)
        {
        cout<<"  parsing successfully"<<endl;
        cout<<"size of adjclose = "<<AdjClose.size()<<endl;
        itr->second.set_AdjClose(AdjClose);
        itr->second.compute_return();
        itr++;
        }
        else{
            Data.erase(itr++);
        }
    }
    cout<<"All data have been retrieved"<<endl;
}

