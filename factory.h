#pragma once
#include<iostream>
#include<string.h>
using namespace std;

class Factory {
public:
	string factoryId;
	int dockNum;

	Factory(string factoryId, int dockNum) {
		this->factoryId = factoryId;
		this->dockNum = dockNum;
	}

	string getFactoryId() {
		return factoryId;
	}

	void setFactoryId(string factoryId) {
		this->factoryId = factoryId;
	}

	int getDockNum() {
		return dockNum;
	}

	void setDockNum(int dockNum) {
		this->dockNum = dockNum;
	}

};