#pragma once
#include<iostream>
#include <string>
#include <map>
#include <vector>
using namespace std;

//Matrix index renaming
#define Beat_  0
#define Meet_  1
#define Miss_  2
#define AAR_  0
#define CAAR_  1
#define Period_  61
#define HEIGHT 3
#define WIDTH 2
#define LENGTH 61


class Stock {
private:
	string name;
	string date;
	string start_date;
	string end_date;
	double surprise;
	double est;
	double actual;
	vector<double> AdjClose;
	vector<double> return_val;
public:
	Stock() {

	}
	Stock(string name_,string date_, string start_date_, string end_date_, double surprise_, double est_, double actual_) {
		name = name_;
		date = date_;
		start_date = start_date_;
		end_date = end_date_;
		surprise = surprise_;
		est = est_;
		actual = actual_;
	}
	void set_AdjClose(vector<double> & adjclose_) {
		AdjClose.resize(adjclose_.size());
		for (unsigned int i = 0; i < adjclose_.size(); i++) {
			AdjClose[i] = adjclose_[i];
		}
	}
	/*
	void addDay(Day day) {
		data.push_back(day.getDate());
		dataMap[day.getDate()] = day;
	}
	Day getDay(string date) {
		for (unsigned int i = 0; i<data.size(); i++) {
			if (data[i] == date) {
				return dataMap[date];
			}
		}
		return Day("Date not exist", 0, 0, 0, 0, 0, 0);
	}
	*/
	string get_name() { return name; }
	string get_date() { return date; }
	string get_startdate() { return start_date; }
	string get_enddate() { return end_date; }
	double getSurprise() { return surprise; }
	double getest() { return est; }//new
	double getActual() { return actual; }//new
	vector<double> get_ADjClose(){
		return AdjClose;
	}
	vector<double> get_return_val() {
		return return_val;
	}
	void display_adjclose() {
		for (unsigned int i = 0; i<AdjClose.size(); i++) {
			cout << AdjClose[i] << endl;
		}
		cout << "end of Adjclose" << endl;
	}
	void toString() {//new

		cout << "Name : " << name << endl;
		cout << "Day-zero : " << date << endl;
		cout << "Start Date : " << start_date << "    " << "End Date : " << end_date << endl;
		cout << "Surprise : " << surprise << endl;
		cout << "Estimate : " << est << endl;
		cout << "Actual : " << actual << endl;
		cout << "AdjClose vector :" << endl;
		for (unsigned int i = 0; i < AdjClose.size(); i++) {
			if (AdjClose[i] < 10) {
				cout <<" "<< AdjClose[i] << " ";
			}
			else {
				cout << AdjClose[i] << " ";
			}
			
			if (i % 5 == 4) {
				cout << endl;
			}
		}
		cout << endl;
	}
	void compute_return() {
		return_val.resize(AdjClose.size() - 1);
		for (unsigned int i = 0; i<return_val.size();i++) {
			return_val[i] = (AdjClose[i + 1] - AdjClose[i] )/ AdjClose[i];
		}
	}

	~Stock() {}
};

class Matrix {
    
private:
    vector<vector<vector<double>>> array3D;
    
public:
    Matrix() {
        array3D.resize(HEIGHT);
        for (int i = 0; i < HEIGHT; i++) {
            array3D[i].resize(WIDTH);
            for (int j = 0; j < WIDTH; j++) {
                array3D[i][j].resize(LENGTH);
            }
        }
        
    }
    
    void set_vector(int i, int j, vector<double> V_) {
        array3D[i][j] = V_;
    }
    
    vector<double> get_vector(int i, int j) {
        return array3D[i][j];
    }
    
    void set_value(int i, int j, int k, double value_) {
        array3D[i][j][k] = value_;
    }
    
    double get_value(int i, int j, int k) {
        return array3D[i][j][k];
    }
    
    ~Matrix() {
        
    }
    
};



