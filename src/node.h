#pragma once
#include<iostream>
#include<string.h>
#include<vector>
#include"oderitem.h"

class node {
public:

	 string id; // 
	// vector<orderitem> topicklist;
	 vector<string> deliverlist; // 
	 string order_id;
	
	 int arrivetime;
	 int leavetime;
	 int propertytime; // propertytime �ֱ��� creationtime��committed time
	 int servicetime;
	 bool pickup; // ˵���õ���ȡ����
	 double storage;
	 double dealpakage;
	 int operationtime;

	 node() {
		 id = "/0";
		 order_id = "/0";
		 arrivetime = 0;
		 leavetime = -1;
		 servicetime = 0;
		 pickup = 0;
		 storage = 15;
		 dealpakage = 0;
		 operationtime = 0;
	 }


	 node(string factoryId, vector<string> deliverlist, int arrivetime,int temp) {
		//super();
		this->id = factoryId;
	//	this->topicklist = topicklist;
		this->deliverlist = deliverlist;
		this->arrivetime = arrivetime;
		this->leavetime = leavetime;
	}

	 node(string factoryId,string orderid, int propertytime1, bool pickup1,int temp,double pac) {
		this->id = factoryId;
		this->propertytime = propertytime1;
		this->pickup = pickup1;
		this->operationtime = temp;
		this->order_id = orderid;
		this->dealpakage = pac;
	}

	 string getorderid() {
		 return this->order_id;
	 }

	 void setorderid(string ordersid) {
		 this->order_id = ordersid;
	 }

	 void setpickup(int i) {
		 this->pickup = i;
	 }
	 void setstorage(double i) {
		 this->storage = i;
	 }
	 double getstorage() {
		 return storage;
	 }
	 string getId() {
		return id;
	}

	 void setFactoryId(string factoryId) {
		this->id = factoryId;
	}

	/* vector<orderitem> getpicklist() {
		return topicklist;
	}

	 void setpicklist(vector<orderitem> topicklist) {
		this->topicklist = topicklist;
	}
	*/
	 vector<string> getdeliverlist() {
		return deliverlist;
	}

	 void addeliverlist(string added) {
		 this->deliverlist.push_back(added);
	 }

	 void popdeliverlist() {
		 if (deliverlist.size() == 0)cout << " wrong with deliverlist" << endl;
		 this->deliverlist.pop_back();
	 }

	 void popdelivernode() {
		 this->deliverlist.pop_back();
	 }

	 int getpicklen() {
		 return deliverlist.size();
	 }

	 void setdeliverlist(vector<string> deliverlist) {
		this->deliverlist = deliverlist;
	}

	 int getarrivetime() {
		return arrivetime;
	}

	 void setarrivetime(int arrivetime) {
		this->arrivetime = arrivetime;
	}

	 int getleavetime() {
		return leavetime;
	}

	 void setleavetime(int leavetime) {
		this->leavetime = leavetime;
	}

	 int getServiceTime() {
		return servicetime;
	}

	 void setpropertytime(int temp) {
		 propertytime = temp;
	 }

	 int getpropertytime() {
		 return propertytime;
	 }

	 void setServiceTime(int service) {
		this->servicetime = service;
	 }

	 void setpakage(double temp) {
		 this->dealpakage = temp;
	 }

	 double getpakage() {
		 return dealpakage;
	 }

	 bool getpickup() {
		 return pickup;
	 }

	 void setoperationtime(int operation) {
		 operationtime = operation;
	 }

	 int getoperationtime() {
		 return operationtime;
	 }

	 vector<string> getbeforedeliverlist() {
		 vector<string> temp;
		 temp = deliverlist;
		 if (pickup) {
			 temp.pop_back();
		 }
		 else {
			 temp.push_back(order_id);
		 }
		 return temp;
	 }

	 bool check_deliverlist(node& temp) {
		 vector<string>p = temp.getbeforedeliverlist();
		 if (p.size() == this->deliverlist.size()) {
			 for (int i = 0; i < this->deliverlist.size(); i++) {
				 if (p[i] != this->deliverlist[i]) {
					 return 0;
				 }
			 }
			 return 1;
		 }
		 return 0;
	 }
};