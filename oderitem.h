#pragma once
#include<iostream>
#include<string.h>

using namespace std;

class orderitem {
	 string type;
	 string orderid;
	 double demand;
	 string pickupfactoryid;
	 string deliveryfactoryid;
	 int creationtime;
	 int completiontime;
	 int loadtime;
	 int unloadtime;
	 int deliverystate;
	 bool pickpoint;
public:
	orderitem() {
		type = '\0';
		orderid = '\0';
		demand = 0;
	}

	 orderitem(string id, string type, string orderid, double demand, string pickupfactoryid,
		string deliveryfactoryid, int creationtime, int completiontime, int loadtime, int unloadtime,
		int deliverystate) {
		//super();
		this->type = type;
		this->orderid = orderid;
		this->demand = demand;
		this->pickupfactoryid = pickupfactoryid;
		this->deliveryfactoryid = deliveryfactoryid;
		this->creationtime = creationtime;
		this->completiontime = completiontime;
		this->loadtime = loadtime;
		this->unloadtime = unloadtime;
		this->deliverystate = deliverystate;
		this->pickpoint = 1;
	}

	 void setpickpoint(bool a) {
		 this->pickpoint = a;
	 }

	 bool getpickpoint() {
		 return pickpoint;
	 }
	 void setType(string type) {
		this->type = type;
	}

	 string getType() {
		return type;
	}

	 void setOrderId(string orderid) {
		this->orderid = orderid;
	}

	 string getOrderId() {
		return orderid;
	}

	 void setDemand(double demand) {
		this->demand = demand;
	}

	 double getDemand() {
		return demand;
	}

	 void setPickupFactoryId(string pickupfactoryid) {
		this->pickupfactoryid = pickupfactoryid;
	}

	 string getPickupFactoryId() {
		return pickupfactoryid;
	}

	 void setDeliveryFactoryId(string deliveryfactoryid) {
		this->deliveryfactoryid = deliveryfactoryid;
	}

	 string getDeliveryFactoryId() {
		return deliveryfactoryid;
	}

	 int getCreationTime() {
		return creationtime;
	}

	 void setCreationTime(int creationtime) {
		this->creationtime = creationtime;
	}

	 int getCommittedCompletionTime() {
		return completiontime;
	}

	 void setCommittedCompletionTime(int completiontime) {
		this->completiontime = completiontime;
	}

	 void setLoadTime(int loadtime) {
		this->loadtime = loadtime;
	}

	 int getLoadTime() {
		return loadtime;
	}

	 void setUnloadTime(int unloadtime) {
		this->unloadtime = unloadtime;
	}

	 int getUnloadTime() {
		return unloadtime;
	}

	 void setDeliveryState(int deliverystate) {
		this->deliverystate = deliverystate;
	}

	 int getDeliveryState() {
		return deliverystate;
	}
};