#pragma once
#include<iostream>
#include<string.h>
#include<vector>
#include"node.h"
#include"oderitem.h"
using namespace std;

class Vehicle {
	 string id;// ���ƺ�
	 string gpsId;
	 string curFactoryId;
	 int gpsUpdateTime;
	 int operationTime;
	 int boardCapacity;// �ճ���������
	 double leftCapacity;
	 int arriveTimeAtCurrentFactory;// �������ﵱǰ������ʱ��
	 int leaveTimeAtCurrentFactory;// �������ﵱǰ������ʱ��
	 vector<orderitem> carryingItems;// �����͵Ķ���
	 vector<string> plannedRoute;// �滮��·��
public:
	 Vehicle(string id, string gpsId, int operationTime, int boardCapacity, vector<orderitem> carryingItems) {
		//super();
		this->id = id;
		this->gpsId = gpsId;
		this->operationTime = operationTime;
		this->boardCapacity = boardCapacity;
		this->carryingItems = carryingItems;
	}

	 double getLeftCapacity() {
		return leftCapacity;
	}

	 void setLeftCapacity(double leftCapacity) {
		this->leftCapacity = leftCapacity;
	}

	 string getId() {
		return id;
	}

	 void setId(string id) {
		this->id = id;
	}

	 string getGpsId() {
		return gpsId;
	}

	 void setGpsId(string gpsId) {
		this->gpsId = gpsId;
	}

	 string getCurFactoryId() {
		return curFactoryId;
	}

	 void setCurFactoryId(string curFactoryId) {
		this->curFactoryId = curFactoryId;
	}

	 int getGpsUpdateTime() {
		return gpsUpdateTime;
	}

	 void setGpsUpdateTime(int gpsUpdateTime) {
		this->gpsUpdateTime = gpsUpdateTime;
	}

	 int getOperationTime() {
		return operationTime;
	}

	 void setOperationTime(int operationTime) {
		this->operationTime = operationTime;
	}

	 int getBoardCapacity() {
		return boardCapacity;
	}

	 void setBoardCapacity(int boardCapacity) {
		this->boardCapacity = boardCapacity;
	}

	 int getArriveTimeAtCurrentFactory() {
		return arriveTimeAtCurrentFactory;
	}

	 void setArriveTimeAtCurrentFactory(int arriveTimeAtCurrentFactory) {
		this->arriveTimeAtCurrentFactory = arriveTimeAtCurrentFactory;
	}

	 int getLeaveTimeAtCurrentFactory() {
		return leaveTimeAtCurrentFactory;
	}

	 void setLeaveTimeAtCurrentFactory(int leaveTimeAtCurrentFactory) {
		this->leaveTimeAtCurrentFactory = leaveTimeAtCurrentFactory;
	}

	 vector<orderitem> getCarryingItems() {
		return carryingItems;
	}

	 void setCarryingItems(vector<orderitem> carryingItems) {
		this->carryingItems = carryingItems;
	}

	 vector<string> getPlannedRoute() {
		return plannedRoute;
	}

	 void setPlannedRoute(vector<string> plannedRoute) {
		this->plannedRoute = plannedRoute;
	}

	/* node getDes() {
		return des;
	}

	 void setDes(node des) {
		this->des = des;
	}*/

	 void setCurPositionInfo(string curFactoryId, int updateTime, int arriveTimeAtCurrentFactory,int leaveTimeAtCurrentFactory) {
		this->curFactoryId = curFactoryId;
		this->gpsUpdateTime = updateTime;
		if (curFactoryId.length() > 0) {
			this->arriveTimeAtCurrentFactory = arriveTimeAtCurrentFactory;
			this->leaveTimeAtCurrentFactory = leaveTimeAtCurrentFactory;
		}
		else {
			this->arriveTimeAtCurrentFactory = 0;
			this->leaveTimeAtCurrentFactory = 0;
		}

	}
};
