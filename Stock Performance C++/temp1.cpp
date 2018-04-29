#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <locale>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <map>
#include <algorithm>
#include <sstream>
#include <ctype.h>
#include <locale>
#include "Stock1.h"
#include <curl/curl.h>
#include <cctype>
#include <time.h>
#include "Yahoo.h"
#include "operators.h"

using namespace std;

//All stocks, stock name as Key, info as Val
map<string, Stock> Database; // store data initially read from CSV
vector<string> MktDate;
vector<double> MktCls;
vector <map<string, Stock>> DataBase_div(3);

void classify() {
    //iterate the Database(map<string, vector<double>>) and compute the idx to put into three groups
    
    double limit_upper, limit_lower;
    cout << "Please enter limit_upper (in % scale): "; cin >> limit_upper;
    cout << "Please enter limit_lower (in % scale): "; cin >> limit_lower;
    map<string,Stock>::iterator itr;
    for(itr = Database.begin(); itr != Database.end(); itr++){
        if(itr->second.getest()==0&&itr->second.getActual()==0) continue;
        if(itr->second.getSurprise() > limit_upper){
            //Beat map<string, vector<double>>
            DataBase_div[Beat_].insert(map<string,Stock>::value_type(itr -> first, itr->second));
            
        }
        else if(itr->second.getSurprise() < limit_lower){
            //miss map<string,vector<double>>
            DataBase_div[Miss_].insert(map<string,Stock>::value_type(itr -> first, itr->second));
        }
        else{
            //meet map<string, vector<double>>
            DataBase_div[Meet_].insert(map<string,Stock>::value_type(itr -> first, itr->second));
        }
        //Database.erase(itr);
    }
    Database.erase(Database.begin(),Database.end());
}

void checkdata(){
    map<string,Stock>::iterator itr;
    for (int i=0;i<3;i++){
        for(itr = DataBase_div[i].begin(); itr != DataBase_div[i].end();){
            
            if (itr->second.get_return_val().size()!=60){
                cout<< "Caution: Rt size is wrong: "<<itr->first<<endl;
                itr=DataBase_div[i].erase(itr);
            }
            else
                itr++;
        }
    }
}


//Read CSV
void read_Data() {
	ifstream  data("final_test.csv");
	string line;
	string name, date, startdate,enddate, surp, est, actual;
	getline(data, line);//Skip header
	while (!data.eof()) {
		getline(data, name, ',');
		if (name == "") continue;
		getline(data, date, ',');
		getline(data, startdate, ',');
		getline(data, enddate, ',');
		getline(data, actual, ',');
		getline(data, est, ',');
		getline(data, surp, '\n');
		const char *cstr = surp.c_str();//convert string into char* for future operation to use atof
		const char *cstr2 = est.c_str();
		const char *cstr3 = actual.c_str();
		Stock temp = Stock(name, date, startdate, enddate, atof(cstr), atof(cstr2), atof(cstr3));
		Database[name] = temp;
	}
}


// index market close by date and return marketRt
vector<double> specifyMktRt(Stock & ticker_){
    vector<double> iMktdata;
    for (int i=0; i< MktCls.size();i++){
        if (MktDate[i].compare(ticker_.get_startdate())==0){
            iMktdata.push_back(MktCls[i]);
            do{
                i++;
                iMktdata.push_back(MktCls[i]);
            }
            while(MktDate[i].compare(ticker_.get_enddate())!=0 && i<MktCls.size());
            break;
        }
    }
    if (iMktdata.size()<60)
    {   cout<< "Insufficient market data, Ticker: " << ticker_.get_name()<<endl;
        cout<< "Please set a longer date range for market data and try again!" <<endl;
        vector<double> err(1);
        return err;
    }
    //calculate return
    for (unsigned long j=iMktdata.size();j>0;j--){
        iMktdata[j]=iMktdata[j]/iMktdata[j-1]-1;
    }
    iMktdata.erase(iMktdata.begin());
    return iMktdata;
}

// Compute AAR
vector<double> Compute_AAR(map<string,Stock> &group, const int &snum, const int &stime){
    vector<double> AAR_sa(60);
    vector<string> keylist;
    map<string, Stock>::iterator itr;
    unsigned long l = group.size()-1;
    srand((unsigned)time( NULL ));
    
    for (itr=group.begin(); itr!=group.end(); itr++){
        keylist.push_back(itr->first);
    }
    
    for (int t=0; t<stime; t++){
        vector<double> AAR(60);
        vector<double> AR_temp(60);
        vector<double> Mkt_s;
        int getidx;
        string getkey;
        for (int s=0;s<snum;s++){
            getidx=rand()%(l+1);
            getkey=keylist[getidx];
            Mkt_s=specifyMktRt(group[getkey]);
            if (Mkt_s.size()<60) continue;
            if (s==0){
                AAR = group[getkey].get_return_val()-Mkt_s;
            }
            else{
                AR_temp= group[getkey].get_return_val()-Mkt_s;
                AAR = AAR+AR_temp;
            }
            getkey.clear();
        }
        
        AAR=AAR/snum;
        if (t==0){
            AAR_sa = AAR;
        }
        else{
            AAR_sa = AAR_sa+AAR;
        }
    }
    return AAR_sa/stime;
}

// Compute CAAR
vector<double> Compute_CAAR(vector<double> _AAR){
    for (int i=1;i<_AAR.size();i++){
            _AAR[i]=_AAR[i]+_AAR[i-1];
    }
    return _AAR;
}

void export_csv(Matrix myMat){
    ofstream AARcsv,CAARcsv;
    AARcsv.open("AAR.csv");
    CAARcsv.open("CAAR.csv");
    //vector<double> AARdata,CAARdata;
    for (int i=0;i<3;i++){
        AARcsv<<"Group"<<i;
        CAARcsv<<"Group"<<i;
        for (int k=0;k<60;k++){
            AARcsv<<","<<myMat.get_value(i, 0, k);
            CAARcsv<<","<<myMat.get_value(i, 1, k);
        }
        AARcsv<<endl;
        CAARcsv<<endl;
    }
    AARcsv.close();
    CAARcsv.close();
}

void plotResults( vector<double>* yData, vector<double>* yData2,vector<double>* yData3)
{
    getchar();
    FILE *gnuplotPipe,*tempDataFile;
    const char *AARName;
    AARName = "Plot";
    
    gnuplotPipe = popen("/opt/local/bin/gnuplot","w");
    if (gnuplotPipe) {
        fprintf(gnuplotPipe, "plot \"%s\" using 2 title 'Beat' lw 2 with linespoints,\"%s\" using 3 title 'Meet' lw 2 with linespoints,\"%s\" using 4 title 'Miss' lw 2 with linespoints \n", AARName,AARName,AARName);
        fflush(gnuplotPipe);
        tempDataFile = fopen(AARName,"w");
        int prd =60;
        for(int i=0; i< prd; i++ )
            fprintf(tempDataFile," %d %f %f %f\n",i,(*yData)[i],(*yData2)[i], (*yData3)[i]);
        fclose(tempDataFile);
        printf("press enter to continue...");
        getchar();
        remove(AARName);
        fprintf(gnuplotPipe,"exit \n");
        
        
    }
    else
        printf("gnuplot not found...");
}

int main()
{
	// Get target stock name and dates from the local file
	read_Data();
    
    // Divide row data into 3 groups
    classify();
    
    // Get market adj_close
    Yahoo yah0;
    yah0.GetData2("SPY", "2016-01-04", "2017-11-30", MktDate, MktCls);
    
    // retrieve with multi-thread
    Yahoo yah1, yah2, yah3;
    thread th1(&Yahoo::GetData, &yah1,std::ref(DataBase_div[Beat_]));
    thread th2(&Yahoo::GetData, &yah2,std::ref(DataBase_div[Meet_]));
    thread th3(&Yahoo::GetData, &yah3,std::ref(DataBase_div[Miss_]));
    th1.join();
    th2.join();
    th3.join();
    
    // Check data
    checkdata();
    
   //interactive flatform
    cout<<" "<<endl;
    while (1) {
        cout << "Hello user !" << endl<<"Welcome to the Stock application." << endl << "Type 'Exit' to quit the program."
        <<endl<<"Type 'Help' to see instruction."<<endl<<"Type Stock name to retrive stock detailed information."<<endl<<"Type P for plot"<<endl;
        
        string command;
        cin>>command;
        if (command == "Exit") {
            break;
        }
        else if (command == "Help") {
            continue;
        }
        else if (command == "P") {
            Matrix Return_Matrix;
            
            //sample 20 times, 40 tickers in one group everytime
            for (int i=0;i<3;i++){
                vector<double> MyAAR = Compute_AAR(DataBase_div[i], 40, 20);
                Return_Matrix.set_vector(i, 0, MyAAR);
                Return_Matrix.set_vector(i, 1, Compute_CAAR(MyAAR));
            }
            
            //export data to csv
            export_csv(Return_Matrix);
            
            
            vector<double> temp;
            temp.resize(61);
            for (unsigned int i = 0;i<61; i++) {
                temp[i] = i;
            }
            vector<double> AARbeat = Return_Matrix.get_vector(0,0);
            vector<double> AARmeet = Return_Matrix.get_vector(1,0);
            vector<double> AARmiss = Return_Matrix.get_vector(2,0);
            vector<double> CAARbeat = Return_Matrix.get_vector(0,1);
            vector<double> CAARmeet = Return_Matrix.get_vector(1,1);
            vector<double> CAARmiss = Return_Matrix.get_vector(2,1);
            plotResults(&AARbeat, &AARmeet, &AARmiss);
            system("pause");
            plotResults(&CAARbeat, &CAARmeet, &CAARmiss);
            system("pause");
        }
        else if(command !=""&&command != "P" && command!="Help"&& command != "Exit")
        {//retrive stock from database
            for (unsigned int i = 0; i < DataBase_div.size(); i++) {
                map<string, Stock>::iterator itr;
                //to make the system ignore the case sensitivity
                string uppticker;
                std::transform(
                               command.begin(),
                               command.end(),
                               back_inserter(uppticker),
                               (int (*)(int))std::toupper
                               );
                itr = DataBase_div[i].find(uppticker);
                if(itr!=DataBase_div[i].end())
                {
                    itr->second.toString();
                    break;
                }
                if(i==DataBase_div.size()-1)
                {
                    cout<<"this ticker does not exist in our database"<<endl;
                    cout<<"*******************************************"<<endl;
                }
                }
            }
        cout<<" "<<endl;
        cout<<" "<<endl;
        }
    
    return 0;
}
