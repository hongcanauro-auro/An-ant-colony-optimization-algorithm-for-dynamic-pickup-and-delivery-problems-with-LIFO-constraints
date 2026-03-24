#pragma once
#include<iostream>
#include<string.h>
#include<vector>
using namespace std;

class Destination {
public:
	string factoryId;
	vector<string> deliveryItemList;
	vector<string> pickupItemList;
	int arriveTime;
	int leaveTime;
	//	 int serviceTime;
	//	 double lng;
	//	 double lat;
	//	
		/**
		 * 表示vehicle_info->json信息的VehicleInfo里的Destination
		 * @param factoryId
		 * @param deliveryItemList
		 * @param pickupItemList
		 * @param arriveTime
		 * @param leaveTime
		 */
	Destination(string factoryId, vector<string> deliveryItemList, vector<string> pickupItemList, int arriveTime,
		int leaveTime) {
		this->factoryId = factoryId;
		this->deliveryItemList = deliveryItemList;
		this->pickupItemList = pickupItemList;
		this->arriveTime = arriveTime;
		this->leaveTime = leaveTime;
	}

	void setFactoryId(string factoryId) {
		this->factoryId = factoryId;
	}

	string getFactoryId() {
		return factoryId;
	}

	void setDeliveryItemList(vector<string> deliveryItemList) {
		this->deliveryItemList = deliveryItemList;
	}

	vector<string> getDeliveryItemList() {
		return deliveryItemList;
	}

	void setPickupItemList(vector<string> pickupItemList) {
		this->pickupItemList = pickupItemList;
	}

	vector<string> getPickupItemList() {
		return pickupItemList;
	}

	void setArriveTime(int arriveTime) {
		this->arriveTime = arriveTime;
	}

	int getArriveTime() {
		return arriveTime;
	}

	void setLeaveTime(int leaveTime) {
		this->leaveTime = leaveTime;
	}

	int getLeaveTime() {
		return leaveTime;
	}
};
