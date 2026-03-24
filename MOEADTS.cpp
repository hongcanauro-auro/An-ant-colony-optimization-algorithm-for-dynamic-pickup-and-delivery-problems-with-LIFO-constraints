#include<iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include<chrono>
#include<random>
#include<array>
#include<queue>
#include<random>
#include<execution>
#include<algorithm>
#include<iomanip>
#include<unordered_set>
#include<set>
#include <map>
#include <utility>
#include"veichle.h"
#include"node.h"
#include"oderitem.h"
#include"factory.h"

static int T = 2; // 邻居数量
static int N = 6;
static int veichle_num = 5;
static int port_dock = 6;	//港口容纳数量
static double capacity = 15;	//汽车容量
static double alpha = 10000 / 3600;
static int Docknum = 6;
static double Distance[154][154];
static int Time[154][154];
static string numlocation[154];
static unordered_map<string, int> location;
unordered_map<string, vector<node>>solution;
static vector<orderitem> order;
static unordered_map<string, orderitem> orderlist;
static vector<Vehicle>vehicle;	
static int operation_time = 0;
static vector<Factory>factory;	//没必要
static vector<node> solutionline;// 计算车辆的运行时间
// static int timeinterval = 3600;// 规定每次输入的订单的时间间隔
static int timeinterval = 600; // 订单的更新时间间隔为20分钟,规定每次输入的订单的时间间隔
static double timecost = 0; // solution最终的时间消耗量
static int current_time = 600; // 当前的时间点，订单分割的时间点依据
static double reference_point[2];
static unordered_set<string> current_dealingorders; // remove 寻找缺失的order或者是排除重复的order
static int tabulist_size = 6;
// solution 每一列当中相同的节点即使相邻不进行合并
static vector<string> veichles;

static double weight[6][2] =
{
	{0.000100000000000, 0.999900000000000},
	{0.250000000000000, 0.750000000000000},
	{0.500000000000000, 0.500000000000000},
	{0.735294117647059, 0.264705882352941}, //目标所需
	{0.750000000000000, 0.250000000000000},
	{0.999900000000000, 0.000100000000000}
};

inline bool getrandbool() {
	std::random_device rd;  // 非确定性随机数生成器
	std::mt19937 gen(rd()); // 以rd()为种子生成mt19937随机数生成器
	std::uniform_int_distribution<> dis(0, 1); // 生成0到1的均匀分布
	bool number = dis(gen); // 生成0或1
	return number;
}

int getrangenumber(int min,int max) {
	std::random_device rd;  // 非确定性随机数生成器
	std::mt19937 gen(rd()); // 以rd()为种子生成mt19937随机数生成器
	std::uniform_int_distribution<> dis(min, max); // 生成0到1的均匀分布
	int number = dis(gen); // 生成0或1
	return number;
}

int time_to_seconds(const std::string& time_str) {
	std::istringstream time_stream(time_str);
	int hours, minutes, seconds;
	char colon; // 用于忽略字符串中的冒号

	// 从字符串中提取小时、分钟和秒
	time_stream >> hours >> colon >> minutes >> colon >> seconds;

	// 计算总秒数
	int total_seconds = hours * 3600 + minutes * 60 + seconds;
	return total_seconds;
}

string formatTime(int totalSeconds) {
	int hours = totalSeconds / 3600; // 一小时有3600秒
	int minutes = (totalSeconds % 3600) / 60; // 剩余的秒数中包含的分钟数
	int seconds = totalSeconds % 60; // 剩余的秒数

	// 使用std::setw和std::setfill来确保时间格式为两位数
	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << hours << ":"
		<< std::setw(2) << std::setfill('0') << minutes << ":"
		<< std::setw(2) << std::setfill('0') << seconds;

	return ss.str();
}

void printsolution(unordered_map<string,vector<node>>&tempsolution){
	for(string each:veichles){
		cout<<each<<endl;
		for(node value:tempsolution[each]){
			cout<<"current order:"<<value.getorderid();
			cout<<" arrive time:"<<value.getarrivetime()<<" service time:"<<value.getServiceTime()<<" leave time:"<<value.getleavetime();
			cout<<"pick up"<<value.getpickup();
			// cout << " arrive " << value.getarrivetime() << "service " << value.getServiceTime() << " leave " << value.getleavetime();
			cout << endl;
		}
		cout << endl;
	}
}

//对solution上的每一个时间节点进行设置,当且仅当对已经完全配对的解决方案
// 预设第一个节点存在对应的时间节点，第一个时间点总是已经设定成功的
/*inline int cleardocks(int i,int time,priority_queue<node> *factorydocks) {
	if (factorydocks[i].size() < port_dock) {
		return time;
	}
	while (factorydocks[i].size()>=port_dock) {
		factorydocks[i].pop();
	}
	return max(factorydocks[i].front().getServiceTime(), time);
}*/

void Timecaculate(std::unordered_map<std::string, std::vector<node>>& nonsolution) {
   // std::queue<node> factorydocks[154];
    std::unordered_map<std::string, int> numpos;
    std::unordered_set<std::string> exist;
    std::string value;
    int minarrive;
    // 定义优先队列及比较函数
    auto compare = [&nonsolution, &numpos](const std::string& a, const std::string& b) {
        return nonsolution[a][numpos[a]].getarrivetime() > nonsolution[b][numpos[b]].getarrivetime();
    };
	auto compare1 = [](node&a,node&b){
		return a.getServiceTime()>b.getServiceTime();
	};
	std::vector<std::priority_queue<node, std::vector<node>, decltype(compare1)>> factorydocks(154, std::priority_queue<node, std::vector<node>, decltype(compare1)>(compare1));
    std::priority_queue<std::string, std::vector<std::string>, decltype(compare)> pq(compare);
    for (std::string each : veichles) {
        factorydocks[location[nonsolution[each][0].getId()]].push(nonsolution[each][0]);
        numpos[each] = 0;
        pq.push(each);
    }
    while (!pq.empty()) {
        value = pq.top();
        pq.pop();
        if (numpos[value] + 1 >= nonsolution[value].size()) {
            continue;
        }
        //nonsolution[value][numpos[value] - 1].setleavetime(std::max(current_time, nonsolution[value][numpos[value] - 1].getServiceTime()));
        //nonsolution[value][numpos[value]].setarrivetime(nonsolution[value][numpos[value] - 1].getleavetime() + Time[location[nonsolution[value][numpos[value] - 1].getId()]][location[nonsolution[value][numpos[value]].getId()]]);
       // int delay = cleardocks(location[nonsolution[value][numpos[value]].getId()], nonsolution[value][numpos[value]].getarrivetime(), factorydocks) - nonsolution[value][numpos[value]].getarrivetime();
	    nonsolution[value][numpos[value]].setleavetime(max(current_time,nonsolution[value][numpos[value]].getServiceTime()));
		numpos[value]++;
	    int len = 0;
	    int Mix_arrivetime = nonsolution[value][numpos[value] - 1].getleavetime() + Time[location[nonsolution[value][numpos[value] - 1].getId()]][location[nonsolution[value][numpos[value]].getId()]];
		while(numpos[value] + 1<nonsolution[value].size()&&nonsolution[value][numpos[value]].getId() == nonsolution[value][numpos[value] + 1].getId()){
			len++;numpos[value]++;
		}
		int sum = 0;
		for(int i = numpos[value] - len;i<=numpos[value];i++){
			sum+=nonsolution[value][i].getoperationtime();
		}
		int delay = 0;
		if(factorydocks[location[nonsolution[value][numpos[value]].getId()]].size() >= Docknum){
			while(factorydocks[location[nonsolution[value][numpos[value]].getId()]].size()>Docknum)
				factorydocks[location[nonsolution[value][numpos[value]].getId()]].pop();
			node temp_delay = factorydocks[location[nonsolution[value][numpos[value]].getId()]].top();
			delay = max(0,temp_delay.getServiceTime() - Mix_arrivetime);
		}
		for(int i = numpos[value] - len;i<=numpos[value];i++){
			nonsolution[value][i].setarrivetime(Mix_arrivetime);
			nonsolution[value][i].setServiceTime(sum + Mix_arrivetime + delay);
		}
		factorydocks[location[nonsolution[value][numpos[value]].getId()]].push(nonsolution[value][numpos[value]]);
        if (numpos[value] < nonsolution[value].size() - 1) {
            pq.push(value);
        }
    }
}

// 默认第一个节点是已经完善的
void deliverlistcompute(vector<node>& tempnode) {
	vector<string> temp; double pack = 15;
	for (int i = 1; i < tempnode.size(); i++) {
		if (tempnode[i].getpickup()) {
			temp = tempnode[i - 1].getdeliverlist();
			temp.push_back(tempnode[i].getorderid());
			tempnode[i].setdeliverlist(temp);
		}
		else {
			temp = tempnode[i - 1].getdeliverlist();
			if (temp.size() == 0) {
				cout << "Fuck!!!!!!!!!!!";
				printsolution(solution);
			}
			temp.pop_back();
			tempnode[i].setdeliverlist(temp);
		}
	}
}

// 对车辆的储存量进行评估，storage代表经过node上节点操作之后车辆还剩下的储存空间,默认第一个节点的信息都已经完善
void packevaluation(vector<node>& temparry) {
	deliverlistcompute(temparry);
	double temp = 15;
	for (int i = 0; i < temparry.size(); i++) {
		temp = 15;
		for (string str : temparry[i].getdeliverlist()) {
			temp -= orderlist[str].getDemand();
		}
		temparry[i].setstorage(temp);
	}
}

void PDlist(vector<node>& tempnode) {
	vector<string> temp; double pack = 15;
	for (int i = 1; i < tempnode.size(); i++) {
		if (tempnode[i].getpickup()) {
			temp = tempnode[i - 1].getdeliverlist();
			temp.push_back(tempnode[i].getorderid());
			tempnode[i].setdeliverlist(temp);
		}
		else {
			temp = tempnode[i - 1].getdeliverlist();
			if (temp.size() == 0)cout << "motherfuck!!!!!!!!!!!";
			temp.pop_back();
			tempnode[i].setdeliverlist(temp);
		}
		pack = 15;
		for (string str : tempnode[i].getdeliverlist()) {
			pack -= orderlist[str].getDemand();
		}
		tempnode[i].setstorage(pack);
	}
}

inline void total_pack(unordered_map<string, vector<node>>& tempsolution) {
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		PDlist(it->second);
	}
}

inline void total_deliverlist(unordered_map<string, vector<node>>& tempsolution) {
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		deliverlistcompute(it->second);
	}
}

void computealldeliverlist(unordered_map<string, vector<node>>& temp) {
	for (auto it = temp.begin(); it != temp.end(); it++) {
		deliverlistcompute(it->second);
	}
}

inline void storagesetting(unordered_map<string, vector<node>>&temparry) {
	for (auto it = temparry.begin(); it != temparry.end(); it++) {
		packevaluation(it->second);
	}
}

bool location_judege(unordered_map<string, vector<node>> &temp, int num[]) {
	for (int i = 0; i < veichle_num; ++i) {
		// 获取 unordered_map 中的第 i 个键值对
		auto it = temp.begin();
		std::advance(it, i); // 移动迭代器到第 i 个元素

		// 检查 num[i] 是否等于对应 vector 的大小
		if (num[i] < it->second.size()) {
			// 如果 num[i] 小于 vector 的大小，说明没有遍历到尾部
			return false;
		}
	}
	// 所有 vector 都遍历到了尾部
	return true;
}

// 对当前节点进行匹配，相似于括号匹配算法，进行节点的匹配

std::vector<double> decomposeNumber(double number, double limit) {
    if(number<limit){
        std::vector<double> arry;
        arry.push_back(number);
        return arry;
    }
    std::vector<double> arry1 = decomposeNumber((number - int(number)%2)/2,limit);
    std::vector<double>arry2 = decomposeNumber((number - int(number)%2)/2 + int(number)%2 ,limit);
    std::vector<double> result = arry1;
    result.insert(result.end(), arry2.begin(), arry2.end());
    return result;
}

double f1_timecosuming(unordered_map<string,vector<node>>&tempsolution) { //统一使用秒作为单位但是最后转换成为小时进行比较
	double costime = 0;
	vector<node> temparry;
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		temparry = it->second;
		for (int i = 1; i < temparry.size(); i++) {
			if (!temparry[i].getpickup()) {
				costime += max(0, (temparry[i].getarrivetime() - temparry[i].getpropertytime()));
			}
		}
	}
	return costime/3600;
}

double distancecosuming(vector<node> &nodelist) {
	double sum = 0;
	for (int i = 0; i < nodelist.size() - 1; i++) {
		sum += Distance[location[nodelist[i].id]][location[nodelist[i + 1].id]];
	}
	return sum;
}

double f2_totaldistance(unordered_map<string, vector<node>>& tempsolution) {
	vector<node>routelist; double sum = 0;
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		routelist = it->second;
		sum  = sum + distancecosuming(routelist);
	}
	return sum;
}

inline double finalcost(unordered_map<string, vector<node>>& tempsolution,double*weight_f1_f2) {
	return f1_timecosuming(tempsolution) * weight_f1_f2[0] + f2_totaldistance(tempsolution) * weight_f1_f2[1];
}

inline double Tchebyceff(double timecost, double distance, double* num) {
	return abs(timecost - reference_point[0]) * num[0] > abs(distance - reference_point[1]) * num[1] ? abs(timecost - reference_point[0]) * num[0] : abs(distance - reference_point[1]) * num[1];
}

void readistance() {
	std::ifstream file(R"(D:\sci_pap\project_code\moead_ts\benchmark\Distance.csv)");
	std::string line;
	int i = 0;
	if (file.is_open()) {
		while (std::getline(file, line) && i < 154) {
			std::stringstream ss(line);
			std::string token;
			int j = 0;

			while (std::getline(ss, token, ',') && j < 154) {
				Distance[i][j] = std::stod(token);
				++j;
			}
			++i;
		}
		file.close();
	}
}

void readtime() {
	std::ifstream file(R"(D:\sci_pap\project_code\moead_ts\benchmark\Time.csv)");
	std::string line;
	int i = 0;
	if (file.is_open()) {
		while (std::getline(file, line) && i < 154) {
			std::stringstream ss(line);
			std::string token;
			int j = 0;

			while (std::getline(ss, token, ',') && j < 154) {
				Time[i][j] = std::stoi(token);
				++j;
			}
			++i;
		}
		file.close();
	}
}

void readveichle(string filename) {
	std::ifstream file(filename);
	std::string line;
	string veichle_id;
	getline(file, line);
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string cell;
		// 读取每一行中的每个单元格
		while (std::getline(ss, cell, ',')) {
			// 假设 car_num 是每行的第一个单元格
			if (cell.find("V_") == 0) {
				solution[cell] = { };
				veichles.push_back(cell);
				break; // 跳到下一行，因为已经找到 car_num
			}
		}
	}
	veichle_num = veichles.size();
	file.close();
}

void readorder(string filename) {
	std::ifstream file(filename);
	std::string line;
	string factory_id, pickup_id, delivery_id,completion_time,creation_time,order_id;
	string token;
	orderitem temp;
	if (file.is_open()) {
		std::getline(file, line);
		while (std::getline(file, line)) {
			std::stringstream ss(line);
			// 读取 order_id
			std::getline(ss, token, ',');
			temp.setOrderId(token);

			// 跳过四个不需要的token
			for (int i = 1; i < 4; ++i) {
				std::getline(ss, token, ',');
			}

			// 读取 demand
			std::getline(ss, token, ',');
			temp.setDemand(std::stod(token));

			// 读取 creation_time
			std::getline(ss, token, ',');
			temp.setCreationTime(time_to_seconds(token));

			// 读取 committed_completion_time
			std::getline(ss, token, ',');
			temp.setCommittedCompletionTime(time_to_seconds(token));

			// 读取 load_time
			std::getline(ss, token, ',');
			temp.setLoadTime(stoi(token));

			// 读取 unload_time
			std::getline(ss, token, ',');
			temp.setUnloadTime(stoi(token));

			// 读取 pickup_id
			std::getline(ss, token, ',');
			temp.setPickupFactoryId(token);
			// 读取 delivery_id
			std::getline(ss, token, ',');
			temp.setDeliveryFactoryId(token);

			// 将订单项添加到vector中
			if (orderlist.count(temp.getOrderId()) == 1) {
				cout << "damn !!!!!!!!!!!!!!!!!!!";
			}
			if (temp.getCreationTime() > temp.getCommittedCompletionTime())
				temp.setCommittedCompletionTime(temp.getCommittedCompletionTime() + 24 * 60 * 60);
			if (temp.getDemand() > capacity) {
				double number = temp.getDemand();
				double firstpart = number - floor(number);
				number = floor(number);
				vector<double> result = decomposeNumber(number,capacity);
				for(int iteration = 0;iteration<result.size();iteration++){
					orderitem temporder = temp;
					temporder.setDemand(result[iteration]);
					if(iteration == 0)temporder.setDemand(result[iteration] + firstpart);
					temporder.setOrderId("fuck" + to_string(iteration) + temp.getOrderId());
					order.push_back(temporder);
					orderlist[temporder.getOrderId()] = temporder;
				}
				//cout << "marked" << endl;
			}
			else {
				order.push_back(temp);
				orderlist[temp.getOrderId()] = temp;
			}
		}

	}
}

void readfactory() {
	std::ifstream file(R"(D:\sci_pap\project_code\moead_ts\benchmark\factory_info.csv)");
	std::string line;
	int lineNumber = 0;
	std::string factory_id;
	if (file.is_open()) {
		// 跳过第一行（列标题）
		std::getline(file, line);

		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::getline(ss, factory_id, ',');
			// 使用行号作为键，factory_id作为值
			numlocation[lineNumber] = factory_id;
			location[factory_id] = lineNumber;
			lineNumber++;
		}
		file.close();
	}
}

void initialsolution() {	//对solution变量进行随机工厂分配
	node temp;
	for (auto it = solution.begin(); it != solution.end(); it++) {
		if(it->second.size() != 0)it->second.clear();
		std::random_device rd; // 获取高质量的随机数种子
		std::mt19937 gen(rd()); // 创建基于梅森旋转算法的生成器
		// 设置随机数范围
		std::uniform_int_distribution<> dis(0, 153); // 创建均匀分布的随机数生成器
		// 生成随机数并输出
		int random_integer = dis(gen); // 生成随机数
		temp.setFactoryId(numlocation[random_integer]);
		temp.setServiceTime(0);  // 初始化第一个节点的servicetime，用于时间产生
		temp.setstorage(capacity);
		temp.setpickup(0);
		temp.setorderid(it->first);
		it->second.push_back(temp);
		reference_point[0] = 0;
		reference_point[1] = 0;
	}
}

bool deliverjudge(vector<string> a1, vector<string> a2) {
	if (a1.size() != a2.size())return 0;
	int len = a1.size();
	for (int i = 0; i < len; i++) {
		if (a1[i] != a2[i]) {
			return 0;
		}
	}
	return 1;
}

// 在索引i之后插入node
void insertAfter(std::vector<node>& vec, int index, node newNode) {
	if (index >= vec.size()) {
		// 如果索引超出范围，则添加到vector末尾
		vec.push_back(newNode);
	}
	else {
		vec.insert(vec.begin() + index + 1, newNode);
	}
}

// 在索引i之前插入node
void insertBefore(std::vector<node>& vec, int index, node newNode) {
	if (index == vec.size()) {
		vec.push_back(newNode);
		return;
	}
	vec.insert(vec.begin() + index, newNode);
}

//	进行解决方案的构建，唯一不确定的因素其对应的对象插入vector当中全局容器当中是否会有对应的更改
// 第一个节点除了leavetime其余的都已经确定完毕，尤其是对于deliverlist和storage还有sevicetime
// 有问题！！！！！！！！！！！！！！！！！！！！！！！！！
void CIinsertion(node pickup_node,node deliver_node,unordered_map<string, vector<node>>&dealsolution,double *delta) {
	total_deliverlist(dealsolution);
	double temp = 0; vector<node> templist1;
	string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000;
	vector<node>routelist, templist;
	for (auto it = dealsolution.begin(); it != dealsolution.end(); it++) {
		routelist = it->second;
		if (routelist.size() == 1) {
			temp = Distance[location[pickup_node.getId()]][location[routelist[0].getId()]] + Distance[location[pickup_node.getId()]][location[deliver_node.getId()]];
			temp += max(0, (deliver_node.getoperationtime()+operation_time + pickup_node.getoperationtime() + pickup_node.getpropertytime() + Time[location[pickup_node.getId()]][location[routelist[0].getId()]] + Time[location[pickup_node.getId()]][location[deliver_node.getId()]] - deliver_node.getpropertytime()));
			if (temp < min) {
				minveichle = it->first;
				beginpos = 0;
				endpos = 1;
				min = temp;
			}
		}
		else {
			for (int i = 0; i < routelist.size(); i++) {
				vector<int> toinsertdeliver;
				for (int j = i + 1; j < routelist.size(); j++) {
					if (deliverjudge(routelist[i].getdeliverlist(), routelist[j].getbeforedeliverlist())) {
						toinsertdeliver.push_back(j);
					}
				}
				if (i == routelist.size() - 1) {
					toinsertdeliver.push_back(routelist.size());
				}
				for (int j : toinsertdeliver) {
					bool getout = 0;
					templist = routelist;
					if (i == j - 1) {
						temp = Distance[location[pickup_node.getId()]][location[templist[i].getId()]] + Distance[location[pickup_node.getId()]][location[deliver_node.getId()]];
					}
					else {
						temp = Distance[location[pickup_node.getId()]][location[templist[i].getId()]] + Distance[location[pickup_node.getId()]][location[templist[j].getId()]];
					}
					// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
					insertBefore(templist, j, deliver_node);
					insertAfter(templist, i, pickup_node);
					PDlist(templist);
					for (int k = 0; k < templist.size(); k++) {
						if (templist[k].getstorage() < 0) {
							getout = 1; break;
						}
					}
					if (getout)continue;
					templist1 = dealsolution[it->first];
					dealsolution[it->first] = templist;
					Timecaculate(dealsolution);
					temp = finalcost(dealsolution, delta);
					dealsolution[it->first] = templist1;
					if (temp < min) {
						minveichle = it->first;
						beginpos = i;
						endpos = j;
						min = temp;
					}
				}
			}
		}
	}
	insertBefore(dealsolution[minveichle], endpos, deliver_node);
	insertAfter(dealsolution[minveichle], beginpos, pickup_node);
	PDlist(dealsolution[minveichle]);
	Timecaculate(dealsolution);
}

// 将原先的vector进行转换成为两个容器，temp是已经进行固定的运输点，而返回值代表的是可以进行修改的值
// 在划分的过程当中，保持第一个节点不变，划分出的第一个节点必须得是要下一个完成的节点，并且其一定要保持不变
vector<node> timecutnewproblem(vector<node>& temp,string car_num) {
	vector<node> afterlist;
	for (auto it = temp.begin(); it != temp.end();) {
		if (it->getServiceTime() > current_time) {
			afterlist.push_back(*it);
			current_dealingorders.insert(it->getorderid());
			it = temp.erase(it);
		}
		else {
			it++;
		}
	}
	if (afterlist.size() == 0) {
		afterlist.push_back(temp.back());
		current_dealingorders.insert(temp.back().getorderid());
		temp.pop_back();
	}
	return afterlist;
}

inline void mergeroutelist(vector<node>& temp, vector<node>temp1) {
	temp.insert(temp.end(), make_move_iterator(temp1.begin()), make_move_iterator(temp1.end()));
}

void mergesolution(unordered_map<string,vector<node>>& temp,unordered_map<string,vector<node>> added) {
	for (auto it = temp.begin(); it != temp.end(); it++) {
		mergeroutelist(it->second, added[it->first]);
	}
}

unordered_map<string, vector<node>> getnewproblem(unordered_map<string, vector<node>>& temp) {
	unordered_map<string, vector<node>> afterlist;
	for (auto it = temp.begin(); it != temp.end(); it++) {
		afterlist[it->first] = timecutnewproblem(it->second,it->first);
	}
	return afterlist;
}

// 存在问题，其中没有包含哪些在solution当中但是没有完成的完整订单
vector<orderitem> getneworders(vector<orderitem>& temp) {
	vector<orderitem> afterlist;
	for (auto it = temp.begin(); it != temp.end();) {
		if (it->getCreationTime()>=(current_time - timeinterval)&&it->getCreationTime()<current_time) {
			afterlist.push_back(*it);
			current_dealingorders.insert(it->getOrderId());
			it = temp.erase(it);
		}
		else {
			break;
		}
	}
	return afterlist;
}

// 利用切比雪夫方法判断
unordered_map<string, vector<node>> completeorderwithCI(vector<orderitem> temporderlist,
	unordered_map<string, vector<node>> imcompletesolution,double *delta) {
	total_deliverlist(imcompletesolution);
	unordered_map<string, vector<node>> dealsolution = imcompletesolution;
	double temp = 0; vector<node> templist1;
	for (orderitem neworder : temporderlist) {
		node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
		string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 1000000l;
		vector<node>routelist, templist;
		auto point1 = std::chrono::high_resolution_clock::now();
		for (auto it = dealsolution.begin(); it != dealsolution.end(); it++) {
			routelist = it->second;
			if (routelist.size() == 1) {
				temp = delta[1]*Distance[location[pickup_node.getId()]][location[routelist[0].getId()]] + Distance[location[pickup_node.getId()]][location[deliver_node.getId()]];
				temp += delta[0]*max(0, (deliver_node.getoperationtime() + operation_time + pickup_node.getoperationtime() + pickup_node.getpropertytime() + Time[location[pickup_node.getId()]][location[routelist[0].getId()]] + Time[location[pickup_node.getId()]][location[deliver_node.getId()]] - deliver_node.getpropertytime()));
				if (temp < min) {
					minveichle = it->first;
					beginpos = 0;
					endpos = 1;
					min = temp;
				}
			}
			else {
				for (int i = 0; i < routelist.size(); i++) {
					vector<int> toinsertdeliver;
					for (int j = i + 1; j < routelist.size(); j++) {
						if (deliverjudge(routelist[i].getdeliverlist(), routelist[j].getbeforedeliverlist())) {
							toinsertdeliver.push_back(j);
						}
					}
					if (i == routelist.size() - 1) {
						toinsertdeliver.push_back(routelist.size());
					}
					for (int j : toinsertdeliver) {
						bool getout = 0;
						templist = routelist;
						/*if (i == j - 1) {
							temp = Distance[location[pickup_node.getId()]][location[templist[i].getId()]] + Distance[location[pickup_node.getId()]][location[deliver_node.getId()]];
						}
						else {
							temp = Distance[location[pickup_node.getId()]][location[templist[i].getId()]] + Distance[location[pickup_node.getId()]][location[templist[j].getId()]];
						}*/
						// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
						insertBefore(templist, j, deliver_node);
						insertAfter(templist, i, pickup_node);
						PDlist(templist);
						for (int k = 0; k < templist.size(); k++) {
							if (templist[k].getstorage() < 0) {
								getout = 1; break;
							}
						}
						if (getout)continue;
						templist1 = dealsolution[it->first];
						dealsolution[it->first] = templist;
					//	timesettingall(dealsolution);
						Timecaculate(dealsolution);
						temp = Tchebyceff(f1_timecosuming(dealsolution), f2_totaldistance(dealsolution), delta);
						dealsolution[it->first] = templist1;
						if (temp < min) {
							minveichle = it->first;
							beginpos = i;
							endpos = j;
							min = temp;
						}
					}
				}
			}
			auto point2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed1 = point2 - point1;
			if (elapsed1.count() >= 10 ) {
				while (it != dealsolution.end()) {
					templist = it->second;
					int posi, posj;
					posi = it->second.size() - 1;
					posj = it->second.size();
					insertBefore(templist, posj, deliver_node);
					insertAfter(templist,posi , pickup_node);
					templist1 = dealsolution[it->first];
					dealsolution[it->first] = templist;
					Timecaculate(dealsolution);
					temp = Tchebyceff(f1_timecosuming(dealsolution), f2_totaldistance(dealsolution), delta);
					dealsolution[it->first] = templist1;
					if (temp < min) {
						minveichle = it->first;
						beginpos = posi;
						endpos = posj;
						min = temp;
					}
					it++;
				}
				break;
			}
		}
		/*auto point2 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed1 = point2 - point1;
		std::cout << elapsed1.count() << " ms " << endl;*/
		insertBefore(dealsolution[minveichle], endpos, deliver_node);
		insertAfter(dealsolution[minveichle], beginpos, pickup_node);
		PDlist(dealsolution[minveichle]);
		Timecaculate(dealsolution);
	}
	return dealsolution;
}

void CIblockinsert(unordered_map<string, vector<node>>& initialsolution,vector<node> insertarry,double*w) {
	vector<node> templist; double mincost = 100000000000, tempcost;
	unordered_map<string, vector<node>> tempsolution,minsolution;
	for (auto it = initialsolution.begin(); it != initialsolution.end(); it++) {
		for (int i = 0; i < it->second.size(); i++) {
			tempsolution = initialsolution;
			templist.clear();
			for (int k = 0; k <= it->second.size(); k++) {
				if (k == i + 1) {
					for (node value : insertarry) {
						templist.push_back(value);
					}
				}
				if (k == it->second.size())break;
				templist.push_back(tempsolution[it->first][k]);
			}
			bool getout = 0;
			PDlist(templist);
			for (int k = 0; k < templist.size(); k++) {
				if (templist[k].getstorage() < 0) {
					getout = 1; break;
				}
			}
			if (getout)continue;
			tempsolution[it->first] = templist;
			Timecaculate(tempsolution);
			tempcost = finalcost(tempsolution,w);
			if (tempcost < mincost) {
				mincost = tempcost;
				minsolution = tempsolution;
			}
		}
	}
	initialsolution = minsolution;
}

// 获得每一列couple的位置，利用couple也可以得知block的位置，从而进行修改
unordered_map<int, int> getcouplemating(vector<node>& routelist) {
	unordered_map<int, int> couplemating; //前一位为取货点，后一位为送货点
	for (int i = 0; i < routelist.size(); i++) {
		if (routelist[i].getpickup()) {
			for (int j = i + 1; j < routelist.size(); j++) {
				if (!routelist[j].getpickup()) {
					if (routelist[i].check_deliverlist(routelist[j])) {
						couplemating[i] = j;
						break;
					}
				}
			}
		}
	}
	return couplemating;
}

// 车辆当中的交叉，不同车辆的订单列的交换，不涉及到具体的nodelist的交换
unordered_map<string, vector<node>> crossover_v2(unordered_map<string, vector<node>> parent1, unordered_map<string, vector<node>>parent2,double*w) {
	unordered_map<string, vector<node>> child;
	vector<node> templist;
	vector<orderitem> leftorder;
	unordered_set<string> tempset;
	for (auto it = parent1.begin(); it != parent1.end(); it++)
		child[it->first];
	tempset = current_dealingorders;
	for (auto it1 = child.begin(); it1 != child.end(); it1++) {
		if (getrandbool()) {
			templist = parent1[it1->first];
			for (auto it = templist.begin(); it != templist.end();) {
				if (tempset.count(it->getorderid())) {
					if(!it->getpickup())tempset.erase(it->getorderid());
					it++;
				}
				else {
					it = templist.erase(it);
				}
			}
			child[it1->first] = templist;
		}
		else {
			templist = parent2[it1->first];
			for (auto it = templist.begin(); it != templist.end();) {
				if (tempset.count(it->getorderid())) {
					if (!it->getpickup())tempset.erase(it->getorderid());
					it++;
				}
				else {
					it = templist.erase(it);
				}
			}
			child[it1->first] = templist;
		}
	}
	for (string value: tempset) {
		leftorder.push_back(orderlist[value]);
	}
	child = completeorderwithCI(leftorder, child, w);
	return child;
}

// 其中进行对应的是目标的索引，
// 在进行relocate 或者是 exchange的时候采用中间变量进行复制，而后进行对应位置的复制即可
// 前者代表couple的前一个位置，后者代表couple的后一个位置
vector<pair<string, vector<int>>> couple_mating(unordered_map<string,vector<node>> tempsolution) {
	vector<pair<string, vector<int>>> mating_couple;
	unordered_map<string,vector<int>> moveable_order;
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		moveable_order.clear();
		for (int i = 1; i < it->second.size(); i++) {	// 排除第一个节点之后的固定节点，后面的可变节点进行对应
			if (it->second[i].getpickup()) {
				moveable_order[it->second[i].getorderid()].push_back(i);
			}
			else {
				if (moveable_order.count(it->second[i].getorderid()) == 1) {
					moveable_order[it->second[i].getorderid()].push_back(i);
				}
			}
		}
		for (auto it1 = moveable_order.begin(); it1 != moveable_order.end(); it1++) {
			if (it1->second.size() > 2)cout << "wrong !!!!!!!! couple" << endl;
			mating_couple.push_back(make_pair(it->first, it1->second));
		}
	}
	return mating_couple;
}

// 前者代表block的前一个位置，后者代表block的后一个位置
vector<pair<string, vector<int>>> block_mating(unordered_map<string,vector<node>> tempsolution) {
	vector<pair<string, vector<int>>> mating_block;
	unordered_map<string, vector<int>> moveable_order;
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		moveable_order.clear();
		for (int i = 1; i < it->second.size(); i++) {
			if (it->second[i].getpickup()) {
				if (it->second[i].getbeforedeliverlist().size() == 0) {
					moveable_order[it->second[i].getorderid()].push_back(i);
				}
			}
			else {
				if (moveable_order.count(it->second[i].getorderid()) == 1) {
					moveable_order[it->second[i].getorderid()].push_back(i);
				}
			}
		}
		for (auto it1 = moveable_order.begin(); it1 != moveable_order.end(); it1++) {
			if (it1->second.size() > 2)cout << "wrong !!!!!!!! block" << endl;
			mating_block.push_back(make_pair(it->first, it1->second));
		}


	}

	return mating_block;
}

bool judegenumber(int a1, int a2, int b1, int b2) {
	if ((a1 <= b1 && b1 <= a2) || (b1 <= a1 && a1 <= b2)) {
		return true;
	}
	// 情况二：两个线段完全重合
	if ((a1 <= b1 && b2 <= a2) || (b1 <= a1 && a2 <= b2)) {
		return true;
	}
	return false;
}

// 在进行couple变换当中，第一个节点所对应的couple或者是block都不允许进行改变，其为下一个所必须完成的订单
unordered_map<string, vector<node>> block_exchange(unordered_map<string, vector<node>>initialsolution, double* w) {
	unordered_map<string, vector<node>> minsolution = initialsolution,tempsolution,currentsolution;
	vector<pair<string, vector<int>>> block_set = block_mating(minsolution);
	vector<node> loclist1, templist, loclist2; int iter = 0;
	double mincost = finalcost(initialsolution,w),tempcost;
	for (int i = 0; i < block_set.size(); i++) {
		loclist1.clear(); iter = 0; currentsolution = initialsolution;
		for (auto it = currentsolution[block_set[i].first].begin(); it != currentsolution[block_set[i].first].end();) {
			if (iter >= block_set[i].second[0] && iter <= block_set[i].second[1]) {
				loclist1.push_back(*it);
				it = currentsolution[block_set[i].first].erase(it);
			}
			else {
				it++;
			}
			iter++;
		}
		for (int j = i + 1; j < block_set.size(); j++) {
			tempsolution = currentsolution; iter = 0; loclist2.clear(); 
			if (block_set[i].first == block_set[j].first) {
				if (judegenumber(block_set[i].second[0],block_set[i].second[1],block_set[j].second[0],block_set[j].second[1]))
				{
					cout << "fuck exist1!!!!!!!!!!1" << endl;
				}
				string movestr = block_set[i].first;
				tempsolution = initialsolution;
				for (auto it = tempsolution[movestr].begin(); it != tempsolution[movestr].end();) {
					if (iter >= block_set[j].second[0] && iter <= block_set[j].second[1]) {
						node tempnode = *it;
						loclist2.push_back(tempnode);
						it++;
					}
					else {
						it++;
					}
					iter++;
				}
				templist.clear();
				for (int k = 0; k < tempsolution[movestr].size(); k++) {
					if (k == block_set[i].second[0]) {
						for (node value : loclist2)templist.push_back(value);
						k = block_set[i].second[1];
						continue;
					}
					if (k == block_set[j].second[0]) {
						for (node value : loclist1)templist.push_back(value);
						k = block_set[j].second[1];
						continue;
					}
					templist.push_back(tempsolution[movestr][k]);
				}
				unordered_set<string> pickup_set, deliver_set;
				for (node value : templist) {
					if (value.getpickup()) {
						if (pickup_set.count(value.getorderid()) == 0) {
							pickup_set.insert(value.getorderid());
						}
						else {
							cout << "fuck!!!!wrong for replace"<<endl;
							for (node tempvalue : tempsolution[movestr])cout << tempvalue.getorderid() << " ";
							cout << endl;
							for (node tempvalue : templist)cout << tempvalue.getorderid() << " ";
							cout << endl;
							printsolution(tempsolution);
							tempsolution[movestr] = templist;
							total_pack(tempsolution);
							printsolution(tempsolution);
							for (node tempvalue : loclist1)cout << tempvalue.getorderid() << " "<<tempvalue.getpickup()<<endl;
							cout << endl;
							for (node tempvalue : loclist2)cout << tempvalue.getorderid() << " " << tempvalue.getpickup() << endl;
							cout << endl;
							exit(0);
						}
					}
					else {
						if (deliver_set.count(value.getorderid()) == 0) {
							deliver_set.insert(value.getorderid());
						}
						else {
							cout << "fuck!!!!wrong for replace" << endl;
							for (node tempvalue : tempsolution[movestr])cout << tempvalue.getorderid() << " ";
							cout << endl;
							for (node tempvalue : templist)cout << tempvalue.getorderid() << " ";
							cout << endl;
							printsolution(tempsolution);
							tempsolution[movestr] = templist;
							total_pack(tempsolution);
							printsolution(tempsolution);
							for (node tempvalue : loclist1)cout << tempvalue.getorderid() << " " << tempvalue.getpickup() << endl;
							cout << endl;
							for (node tempvalue : loclist2)cout << tempvalue.getorderid() << " " << tempvalue.getpickup() << endl;
							cout << endl;
							exit(0);
						}
					}
				}
				tempsolution[movestr] = templist;
			}
			else {
				for (auto it = tempsolution[block_set[j].first].begin(); it != tempsolution[block_set[j].first].end();) {
					if (iter >= block_set[j].second[0] && iter <= block_set[j].second[1]) {
						loclist2.push_back(*it);
						it = tempsolution[block_set[j].first].erase(it);
					}
					else {
						it++;
					}
					iter++;
				}
				tempsolution[block_set[j].first].insert(tempsolution[block_set[j].first].begin() + block_set[j].second[0], loclist1.begin(), loclist1.end());
				tempsolution[block_set[i].first].insert(tempsolution[block_set[i].first].begin() + block_set[i].second[0], loclist2.begin(), loclist2.end());
			}
			total_pack(tempsolution);
			Timecaculate(tempsolution);
			tempcost = finalcost(tempsolution, w);
			if (tempcost < mincost) {
				mincost = tempcost;
				minsolution = tempsolution;
			}
		}
	}
	total_pack(minsolution);
//	timesettingall(minsolution);
	return minsolution;
}

unordered_map<string, vector<node>> block_relocate(unordered_map<string, vector<node>>initialsolution,double*w) {
	unordered_map<string, vector<node>> minsolution = initialsolution,tempsolution;
	vector<pair<string, vector<int>>> block_set = block_mating(initialsolution);
	vector<node> templist; double mincost, tempcost;
	mincost = finalcost(initialsolution,w);
	/*for (int i = 0; i < block_set.size(); i++)
	{
		cout << block_set[i].first << endl;
		for (int value : block_set[i].second) {
			cout << value << " ";
		}
		cout << endl;
	}*/
	for (int i = 0; i < block_set.size(); i++) {
		tempsolution = initialsolution;
		templist.clear(); int iter = 0;
		for (auto it = tempsolution[block_set[i].first].begin(); it != tempsolution[block_set[i].first].end(); ) {
			if (iter >= block_set[i].second[0] && iter <= block_set[i].second[1]) {
				node tempnode = *it;
				templist.push_back(tempnode);
				it = tempsolution[block_set[i].first].erase(it);
			}
			else {
				it++;
			}
			iter++;
		}
		Timecaculate(tempsolution);
		total_pack(tempsolution);
		CIblockinsert(tempsolution, templist,w);
		Timecaculate(tempsolution);
		tempcost = finalcost(tempsolution, w);
		if (tempcost < mincost) {
			mincost = tempcost;
			minsolution = tempsolution;
		}
	}
	return minsolution;
}

unordered_map<string, vector<node>> couple_exchange(unordered_map<string, vector<node>>initialsolution, double* w) {
	unordered_map<string, vector<node>> tempsolution = initialsolution;
	unordered_map<string, vector<node>> minsolution = initialsolution;
	vector<pair<string, vector<int>>>couple_set = couple_mating(tempsolution);
	vector<node> templist; string p1 = "fuck", p2;
	/*for (int i = 0; i < couple_set.size(); i++)
	{
		cout << couple_set[i].first << endl;
		for (int value : couple_set[i].second) {
			cout << value << " ";
		}
		cout << endl;
	}*/
	double mincost = finalcost(tempsolution,w),tempcost;
	node p1_replace, d1_replace,p2_replace,d2_replace;
	for (int i = 0; i < couple_set.size(); i++) {
		p1_replace = initialsolution[couple_set[i].first][couple_set[i].second[0]];
		d1_replace = initialsolution[couple_set[i].first][couple_set[i].second[1]];
		for (int j = i + 1; j < couple_set.size(); j++) {
			tempsolution = initialsolution;
			p2_replace = initialsolution[couple_set[j].first][couple_set[j].second[0]];
			d2_replace = initialsolution[couple_set[j].first][couple_set[j].second[1]];
			tempsolution[couple_set[i].first][couple_set[i].second[0]] = p2_replace;
			tempsolution[couple_set[i].first][couple_set[i].second[1]] = d2_replace;
			tempsolution[couple_set[j].first][couple_set[j].second[0]] = p1_replace;
			tempsolution[couple_set[j].first][couple_set[j].second[1]] = d1_replace;
			bool getout = 0;
			templist = tempsolution[couple_set[i].first];
			PDlist(templist);
			for (int k = 0; k < templist.size(); k++) {
				if (templist[k].getstorage() < 0) {
					getout = 1; break;
				}
			}
			if (getout)continue;
			templist = tempsolution[couple_set[j].first];
			PDlist(templist);
			for (int k = 0; k < templist.size(); k++) {
				if (templist[k].getstorage() < 0) {
					getout = 1; break;
				}
			}
			if (getout)continue;
			Timecaculate(tempsolution);
			tempcost = finalcost(tempsolution,w);
			if (tempcost < mincost) {
				mincost = tempcost;
				p1 = couple_set[i].first;
				p2 = couple_set[j].first;
				minsolution = tempsolution;
			}
		}
	}
	if (p1 != "fuck") 
	{
		PDlist(minsolution[p1]);
		PDlist(minsolution[p2]);
	}
	total_pack(minsolution);
	return minsolution;
}

unordered_map<string, vector<node>> couple_relocate(unordered_map<string, vector<node>> initialsolution, double* w) {
	unordered_map<string, vector<node>> minsolution = initialsolution,tempsolution;
	vector<pair<string, vector<int>>>couple_set = couple_mating(initialsolution);
	double mincost = finalcost(initialsolution, w);
	double tempcost; 
	/*for (int i = 0; i < couple_set.size(); i++)
	{
		cout << couple_set[i].first << endl;
		for (int value : couple_set[i].second) {
			cout << value << " ";
		}
		cout << endl;
	}*/
	for (int i = 0; i < couple_set.size(); i++) {
		tempsolution = initialsolution;
		node p_replace, d_replace;
		int loc1 = couple_set[i].second[0];
		int loc2 = couple_set[i].second[1];
		p_replace = initialsolution[couple_set[i].first][couple_set[i].second[0]];
		d_replace = initialsolution[couple_set[i].first][couple_set[i].second[1]];
		vector<node> templist = tempsolution[couple_set[i].first];
		auto it = templist.rbegin() + (templist.size() - 1 - loc2);
		templist.erase((it + 1).base());
		it = templist.rbegin() + (templist.size() - 1 - loc1);
		templist.erase((it + 1).base());
		tempsolution[couple_set[i].first] = templist;
		Timecaculate(tempsolution);
		CIinsertion(p_replace, d_replace, tempsolution, w);
		tempcost = finalcost(tempsolution,w);
		if (tempcost < mincost) {
			mincost = tempcost;
			minsolution = tempsolution;
		}
	}
	total_pack(minsolution);
	return minsolution;
}

inline unordered_set<string> transform_solution_to_string(unordered_map<string,vector<node>>initialsolution) {
	unordered_set<string> aftertransform;
	for (auto it = initialsolution.begin(); it != initialsolution.end(); it++) {
		string temp;
		for (node value : it->second) {
			temp += value.getorderid();
		}
		aftertransform.insert(temp);
	}
	return aftertransform;
}

bool noTabu(unordered_set<string> temp, queue<unordered_set<string>> tabuliat) {
	while (tabuliat.size() > tabulist_size)
		tabuliat.pop();
	queue<unordered_set<string>> templist = tabuliat;
	for (int i = 0; i < templist.size(); i++) {
		int num_count = 0;
		for (string value : temp) {
			if (templist.front().count(value) == 1)num_count++;
		}
		if (num_count == veichle_num)return 0;
		templist.pop();
	}
	return 1;
}

//	对某个具体的解进行禁忌，将某个解决方案放在禁忌列表当中
unordered_map<string, vector<node>> Tabu_search(unordered_map<string, vector<node>>initialsolution,double*w) {
	queue<unordered_set<string>> Tabulist;
	unordered_set<string> temptransformer,bestneighbor_transform;
	unordered_map<string, vector<node>> currentsolution,tempsolution ,bestsolution, bestneighbor;
	int iter = 0, niter = 0,Maxiter = 10,Neighbortheshold = 10;
	bestsolution = initialsolution; currentsolution = initialsolution;
	double min = finalcost(bestsolution,w);
	while (iter < Maxiter) {
		bestneighbor = currentsolution;
		niter = 0;
		double min_neighbor = finalcost(bestneighbor, w);
		while (niter < Neighbortheshold) {
			switch (getrangenumber(1, 4)) {
			case 1:
				tempsolution = block_exchange(currentsolution, w);
				break;
			case 2:
				tempsolution = block_relocate(currentsolution, w);
				break;
			case 3:
				tempsolution = couple_exchange(currentsolution, w);
				break;
			case 4:
				tempsolution = couple_relocate(currentsolution, w);
				break;
			}
			temptransformer = transform_solution_to_string(tempsolution);
			if (noTabu(temptransformer,Tabulist)) {
				double tempmin = finalcost(tempsolution, w);
				if (tempmin < min_neighbor) {
					bestneighbor = tempsolution;
					bestneighbor_transform = temptransformer;
					min_neighbor = tempmin;
				}
			}
			niter++;
		}
		if (min_neighbor < min) {
			bestsolution = bestneighbor;
			min = min_neighbor;
		}
		currentsolution = bestneighbor;
		Tabulist.push(bestneighbor_transform);
		iter++;
	}
	return bestsolution;
}

// 更新reference_point
void update_reference(unordered_map<string,vector<node>>* population) {
	double minf1 = 1000000000, minf2 = 1000000000;
	for (int i = 0; i < N; i++) {
		minf1 = min(f1_timecosuming(population[i]), minf1);
		minf2 = min(minf2,f2_totaldistance(population[i]));
	}
	reference_point[0] = minf1;
	reference_point[1] = minf2;
}

#if 0
// 暂时没有考虑隔天的订单
int main() {
	readtime(); readistance(); readfactory(); 
	for (instance = 34; instance <= 34; instance++) {
		/*string instance_veichle = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_"+ to_string(instance) + "\\"+ to_string(instance) + ".csv";
		string instance_order = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";*/
		//string instance_veichle = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_64\\64.csv";
		//string instance_order = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_64\\vehicle_info.csv";
		std::string instance_veichle = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
		std::string instance_order = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
		readveichle(instance_veichle); readorder(instance_order);
		for (int loop = 0; loop < 1; loop++) {
			initialsolution();
			vector<orderitem> todoorderlist;
			vector<orderitem> currentorderlist;
			unordered_map<string, vector<node>>currentproblem;
			for (int i = 0; i < order.size(); i++)
				todoorderlist.push_back(order[i]);
			int iter = 0; // 订单处理的轮数
			unordered_map<string, vector<node>> population[6];
			while (todoorderlist.size() != 0) {
				current_dealingorders.clear();
				currentorderlist = getneworders(todoorderlist);
				currentproblem = getnewproblem(solution);
				for (int i = 0; i < N; i++) {
					population[i] = currentproblem;
				}
				for (int i = 0; i < N; i++) {
					// 解决方案的初始化采用的是各自的TC函数，利用TC函数获得最大值，初始化之后再更新对应的reference_point
					population[i] = completeorderwithCI(currentorderlist, currentproblem, weight[i]);
				}
				update_reference(population);
				/*for (int i = 0; i < N; i++) {
					if (i == 0) {
						population[0] = crossover_v2(population[1], population[2], weight[0]);
						continue;
					}
					if (i == 5) {
						population[5] = crossover_v2(population[3], population[4], weight[5]);
						break;
					}
					population[i] = crossover_v2(population[i - 1], population[i + 1], weight[i]);
				}
				for (int i = 0; i < N; i++)
					population[i] = Tabu_search(population[i], weight[i]);*/
				currentproblem = population[3];
				mergesolution(solution, currentproblem);
				total_pack(solution);
				// 对解决方案进行融合之后，得到一个较为完整的解之后进行参考点的更新
				current_time += timeinterval;
				printsolution(solution);
				cout << todoorderlist.size() << endl;
			}
			printsolution(solution);
		}
	}
	return 0;
}
#endif


// ant colony system ............................................................................ant colony system //

// 遍历veichles从而对solution进行遍历
double alpha2 = 0.1;
double same = 8;
double beta_der = 0.1;
double rho1 = 2; //分配订单时，启发式和信息素二者的占比
double rho2 = 3; //单个订单路径规划时，启发式和信息素二者的占比
double q1 = 0.9; //订单分配时，基于探索还是基于概率最大的一个
double q2 = 0.9; //路径规划时，基于探索还是基于概率最大的一个
int antnum = 3;
int route_construct_interation = 6;
int solution_construct_interation = 15;

inline double generateRandomNumber() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0.0, 1.0);
	return dis(gen);
}

double findMin(const std::vector<double>& numbers) {
    if (numbers.empty()) {
        // 处理向量为空的情况，这里可以根据需求进行修改
        std::cerr << "Error: The vector is empty." << std::endl;
        return 0.0;
    }
    return *std::min_element(numbers.begin(), numbers.end());
}


// 传入的只是和为一的概率，然后再进行累加
int wheel_select(double p,vector<double> test,string temp) {
	if (test.size() == 0)cout << "nothing" << endl;
	for (int i = 1; i < test.size(); i++) {
		test[i] += test[i - 1];
	}
	if (p >= 0 && p < test[0])return 0;

	for (int i = 1; i < test.size(); i++) {
		if(p>test[i - 1]&&p<=test[i])return i;
	}
	if (test.size() == veichles.size())cerr << "maybe order allocte" << endl;
	cerr << "wrong with probility"<<" "<<temp << endl;
	for (double temp : test)
		cout << temp << " ";
	cout << endl;
	return 0;
}

// 订单在vector当中的序号数，即为订单在矩阵当中的下表的序号，在每一个时间窗口内订单不会发生改变
// 使用迭代器进行遍历哈希表每次遍历的顺序可能不同
void initialorder_relation(vector<vector<double>>& order_relation, vector<orderitem> todoorderlist) {
	for (int i = 0; i < todoorderlist.size(); i++) {
		for (int j = i + 1; j < todoorderlist.size(); j++) {
			// 订单之间的关系表达式判断订单与订单之间的关联性
			// 订单之间的取货与送货点的关系 订单之间的时间上的考虑 订单所需要占用的容量之间的考虑
			// problem!!!!!!!!!!!
			order_relation[i][j] =1/(Distance[location[todoorderlist[i].getPickupFactoryId()]][location[todoorderlist[j].getPickupFactoryId()]]
				+ Distance[location[todoorderlist[i].getDeliveryFactoryId()]][location[todoorderlist[j].getDeliveryFactoryId()]]);
			order_relation[j][i] = order_relation[i][j];
		}
		order_relation[i][i] = 0;
		int sum = 0;
		for (int j = 0; j < todoorderlist.size(); j++) {
			sum += order_relation[i][j];
		}
		for (int j = 0; j < todoorderlist.size(); j++) {
			order_relation[i][j] /= sum;
		}
		order_relation[i][i] = 1;
	}
}

// 遍历solution的顺序和vector当中的车辆的顺序是一致的，二者之间的顺序对应是相同的
// 一定关联的订单，车辆和订单之间的关联系数用-1表示
void initialveichleorder_relation(vector<vector<double>>&veichle_order_relation,unordered_map<string,vector<node>>solution,
	vector<orderitem> todoorderlist) {
	for (int i = 0; i < todoorderlist.size(); i++) {
		double sum = 0;
		for (int j = 0; j < veichle_num; j++) {
			if (Distance[location[todoorderlist[i].getPickupFactoryId()]][location[solution[veichles[j]][0].getId()]] == 0) {
					veichle_order_relation[j][i] = same;
			}
			else {
					veichle_order_relation[j][i] = 1 / Distance[location[todoorderlist[i].getPickupFactoryId()]][location[solution[veichles[j]][0].getId()]];
			}
				sum =sum + veichle_order_relation[j][i];
			}
		for (int j = 0; j < veichle_num; j++) {
			veichle_order_relation[j][i] = veichle_order_relation[j][i]/ sum;
		}
	}
}

int findMaxIndex(vector<double> vec) {
	if (vec.empty()) {
		return -1;  // 处理向量为空的情况
	}
	int maxIndex = 0;  // 假设第一个元素是最大的，将其索引初始化为 0
	for (size_t i = 1; i < vec.size(); ++i) {  // 从第二个元素开始遍历
		if (vec[i] > vec[maxIndex]) {  // 如果当前元素比假设的最大元素大
			maxIndex = i;  // 更新最大元素的索引
		}
	}
	return maxIndex;  // 返回最大元素的索引
}

// 按照搜索路径的方法，将所有的结点从solution当中去除，只留下初始结点，然后再从初始结点之后进行逐个插入
vector<orderitem> getnewproblem(vector<orderitem>& todoorderlist, unordered_map<string,vector<node>>& tempsolution,unordered_map<string,vector<node>>&startsolution,
	unordered_map<string, int>& order_location) {
	vector<orderitem> afterlist; int iteration = 0;
	for (auto it = todoorderlist.begin(); it != todoorderlist.end();) {
		if (it->getCreationTime() >= (current_time - timeinterval) && it->getCreationTime() < current_time) {
			afterlist.push_back(*it);
			order_location[afterlist.back().getOrderId()] = iteration;
			iteration++;
			it = todoorderlist.erase(it);
		}
		else {
			if(it->getCreationTime() >=current_time)break;
			it++;
			cerr << "wrong with the order allocate" << endl;
		}
	}
	// 无订单不做处理
	if(afterlist.size() == 0)return afterlist;
	for (string each : veichles) {
		string location_order = "fuck";
		int to_the_end = 0;
		for(int i = 0;i<startsolution[each].size();i++){
			if(startsolution[each][i].getServiceTime() > current_time){
				location_order = startsolution[each][i].getorderid() + to_string(startsolution[each][i].getpickup());
				to_the_end = startsolution[each].size() - i;
				break;
			}
		}
		if(location_order == "fuck"){
			node tempnode = startsolution[each].back();
			startsolution[each].clear();
			startsolution[each].push_back(tempnode);
			tempsolution[each].pop_back();
			continue;
		}
		for(int i = startsolution[each].size() - 1;i>startsolution[each].size() - to_the_end;i--){
			if(startsolution[each][i].getpickup() == 1){
				afterlist.push_back(orderlist[startsolution[each][i].getorderid()]);
				order_location[afterlist.back().getOrderId()] = iteration;
				iteration++;
			}	
		}
		node tempnode = startsolution[each][startsolution[each].size() - to_the_end];
		startsolution[each].clear();
		startsolution[each].push_back(tempnode);
		tempsolution[each].resize(tempsolution[each].size() - to_the_end);
	}
	return afterlist;
}

// 对应车辆在矩阵当中的序列则对应着viechle容器当中车辆的顺序
// 对每一个车辆进行order的分配,在initial步骤上使用
unordered_map<string, vector<orderitem>> allocate_order(vector<vector<double>> veichle_order_relation,vector<vector<double>> order_relation,
	vector<orderitem>cuorders) {
	unordered_map<string, vector<orderitem>> order_set;
	for (string temp : veichles) {
		order_set[temp];
	}
	// 先不考虑订单之间的关系，即不考虑集合内部的已有订单对接下来选择订单的影响
	for (int i = 0; i < cuorders.size();i++) {
		vector<double> temp_problity;
		temp_problity.push_back(veichle_order_relation[0][i]);
		for (int j = 1; j < veichle_num; j++) {
			temp_problity.push_back(veichle_order_relation[j][i]);
		}
		order_set[veichles[findMaxIndex(temp_problity)]].push_back(cuorders[i]);
	}
	return order_set;
}


// 局部矩阵的构造，对于局部的取送货点的信息素矩阵的构造
// 1、单纯以factroy为地方矩阵的构造策略 （1） 局部order的factory矩阵 （2）全局order的factory矩阵可以进行矩阵杂交等策略
// 2、以某个订单的取送货点作为构造信息矩阵的行列，针对于订单之间的关系，从而实现订单之间的排列关系
// 用node点列来创建对应的矩阵而后也用node点列构建一个新的解
/*vector<vector<double>> initial_localmatrix_construct(vector<node> unordered_node_list) {
	vector<vector<double>> local_matrix(unordered_node_list.size(),vector<double>(unordered_node_list.size(),0));
	for (int i = 0; i < unordered_node_list.size();i++) {
		for (int j = i + 1; j < unordered_node_list.size(); j++) {
			local_matrix[i][j] = 100.0 / (Distance[location[unordered_node_list[i].getId()]][location[unordered_node_list[j].getId()]] * Time[location[unordered_node_list[i].getId()]][location[unordered_node_list[j].getId()]]);
			local_matrix[j][i] = local_matrix[i][j];
		}
	}
	return local_matrix;
}*/

// 包含strat_node从起始点向后进行延续,currentprobelm一定是已经完备的问题，也就是说尽管只有一个结点但是其这一个结点的所有信息都是完备的
vector<node> local_node_construct(vector<orderitem> unordered_solution, unordered_map<string, int>& order_dnode,unordered_map<string,int>&order_pnode, vector<int>&existpickup,vector<node> startsolution) {
	vector<node> afterlist;
	int iteration = 0;
	afterlist.push_back(startsolution.front());
	for (orderitem neworder:unordered_solution){
		iteration += 2;
		node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()), 
		deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			// cout<<pickup_node.getpakage()<<" "<<deliver_node.getpakage()<<endl;
		order_dnode[neworder.getOrderId()] = iteration;
		order_pnode[neworder.getOrderId()] = iteration - 1;
		afterlist.push_back(pickup_node);
		afterlist.push_back(deliver_node);
		existpickup.push_back(iteration - 1);
	}
	for(string neworder_name:startsolution.back().getdeliverlist()) {
		orderitem  neworder = orderlist[neworder_name];
		iteration += 1;
		node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
		order_dnode[neworder.getOrderId()] = iteration;
		afterlist.push_back(deliver_node);
	}

	return afterlist;
}

void dispsolution(unordered_map<string,vector<node>> tempsolution) {
	for (string tempstr : veichles) {
		cout << tempstr << endl;
		for (int i = 0; i < tempsolution[tempstr].size(); i++) {
			cout << tempsolution[tempstr][i].getId() << " : ";
			for (string temporder : tempsolution[tempstr][i].getdeliverlist())
				cout << temporder << " ";
			cout << endl;
		}
		cout << endl;
	}
}

// 仅仅构建最后一位，默认其前面的所有项都已经构件完毕
void PDlist_next(vector<node>& tempnode,string id) {
	if (tempnode.size() <= 1)return;
	vector<string>temp;
	if (tempnode[tempnode.size() - 1].getpickup()) {
		temp = tempnode[tempnode.size() - 2].getdeliverlist();
		temp.push_back(tempnode[tempnode.size() - 1].getorderid());
		tempnode[tempnode.size() - 1].setdeliverlist(temp);
		tempnode[tempnode.size() - 1].setstorage(tempnode[tempnode.size() - 2].getstorage() - tempnode[tempnode.size() - 1].getpakage());
	}
	else {
		temp = tempnode[tempnode.size() - 2].getdeliverlist();
		if (temp.size() == 0) {
			cout << "motherfuck!!!!!!!!!!! wrong with list in ant system";

		}
		temp.pop_back();
		tempnode[tempnode.size() - 1].setdeliverlist(temp);
		tempnode[tempnode.size() - 1].setstorage(tempnode[tempnode.size() - 2].getstorage() + tempnode[tempnode.size() - 1].getpakage());
	}
	double pack = capacity;
	for (string str : tempnode.back().getdeliverlist()) {
		pack -= orderlist[str].getDemand();
	}
	tempnode.back().setstorage(pack);
}

static double time_same = 10;

// 启发式矩阵的初始化,启发式信息仅仅采用了距离作为启发式的初始信息
vector<vector<double>> local_node_matrix(vector<node> templist) {
	vector<vector<double>> tempmatrix(templist.size(), vector<double>(templist.size(), 0));
	for (int i = 0; i < templist.size(); i++) {
		for (int j = i + 1; j < templist.size(); j++) {
			if (Distance[location[templist[i].getId()]][location[templist[j].getId()]] == 0) {
				tempmatrix[i][j] = same;
			}else {
				tempmatrix[i][j] = 1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]];
			//	tempmatrix[i][j] = pow((1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]]),rho1) * ((orderlist[templist[j].getorderid()].getCommittedCompletionTime() - current_time)>0?1/double(orderlist[templist[j].getorderid()].getCommittedCompletionTime() - current_time):time_same);
			}
			if (Distance[location[templist[i].getId()]][location[templist[j].getId()]] == 0){
				tempmatrix[j][i] = same;
			}else{
				tempmatrix[j][i] = 1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]];
			//	tempmatrix[j][i] = pow((1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]]),rho1) * ((orderlist[templist[i].getorderid()].getCommittedCompletionTime() - current_time)>0?1/double(orderlist[templist[i].getorderid()].getCommittedCompletionTime() - current_time):time_same);
			}
		}
		tempmatrix[i][i] = 0.0;
	}
	return tempmatrix;
}

double single_cost(vector<node> nodelist) {
	double cost = 0;
	if (nodelist.size() == 1)return 0;
	for (int i = 1; i < nodelist.size(); i++) {
		cost += Distance[location[nodelist[i - 1].getId()]][location[nodelist[i].getId()]]/double(veichle_num);
		if (nodelist[i].getpickup() == 0)
			cost += alpha * max(0, (nodelist[i].getarrivetime() - nodelist[i].getpropertytime()));
	}
	return cost;
}

/*double total_cost(unordered_map<string, vector<node>> tempsolution) {
	double sum = 0;
	for (string each : veichles) {
		sum += single_cost(tempsolution[each]);
	}
	return sum;
}*/

double total_cost(unordered_map<string,vector<node>>tempsolution){
	unordered_map<string,int> divide_order;
	double cost = 0;
	for(string each:veichles){
		vector<node> nodelist = tempsolution[each];
		for(int i = 1;i<nodelist.size();i++){
			cost += Distance[location[nodelist[i - 1].getId()]][location[nodelist[i].getId()]]/double(veichle_num);
		if (nodelist[i].getpickup() == 0){
			double tempcost = max(0, (nodelist[i].getarrivetime() - nodelist[i].getpropertytime()));
			if(tempcost > 0){
				if(nodelist[i].getorderid()[0] == 'f'){
					if(divide_order[nodelist[i].getorderid().substr(5)] < tempcost){
						divide_order[nodelist[i].getorderid().substr(5)] = tempcost;
					}
				}else cost += alpha * tempcost;
			}
		}
		}
	}
	for(auto it = divide_order.begin();it!=divide_order.end();it++){
		cost +=alpha*double(it->second);
	}
	return cost;
}

void disp_orderset(vector<orderitem> temp) {
	for (orderitem str : temp)
		cout << str.getOrderId() << " ";
	cout << endl;
}

// time*alpha + distance
// 没有考虑time overlap的eta矩阵
vector<vector<double>> initial_eta_matrix(vector<node> currentproblem,vector<node> nodelist,
	vector<int> exist_pickup,unordered_map<string,int> order_num) {
	double tao = 0;
	// 使用最小插入或者直接使用贪心算法
	// 使用贪心算法构建
	while (exist_pickup.size() != 0) {
		int select_order = 0; double cost = 100000000;
		if (currentproblem.back().getdeliverlist().size() != 0) {
			if (cost > ( Distance[location[currentproblem.back().getId()]][location[nodelist[order_num[currentproblem.back().getdeliverlist().back()]].getId()]])) {
				select_order = order_num[currentproblem.back().getdeliverlist().back()];
				cost = Distance[location[currentproblem.back().getId()]][location[nodelist[order_num[currentproblem.back().getdeliverlist().back()]].getId()]];
			}
		}
		for (int temp : exist_pickup) {
			if (nodelist[temp].getpakage()<=currentproblem.back().getstorage()) {
				if (cost > ( Distance[location[currentproblem.back().getId()]][location[nodelist[temp].getId()]])) {
					select_order = temp;
					cost = Distance[location[currentproblem.back().getId()]][location[nodelist[temp].getId()]];
				}
			}
		}
		if(nodelist[select_order].getpickup())exist_pickup.erase(std::remove_if(exist_pickup.begin(), exist_pickup.end(), [select_order](int value) { return value == select_order; }), exist_pickup.end());
		currentproblem.push_back(nodelist[select_order]);
		PDlist_next(currentproblem,"fuck1");
	}
	vector<string> left_order = currentproblem.back().getdeliverlist();
	while (left_order.size() != 0) {
		currentproblem.push_back(nodelist[order_num[left_order.back()]]);
		left_order.pop_back();
		PDlist_next(currentproblem,"fuck2");
	}
	tao = 1.0 / single_cost(currentproblem);
	vector<vector<double>> eta_matrix(nodelist.size(), vector<double>(nodelist.size(), tao));
	return eta_matrix;
}

void Timecaculate_next(vector<node>&routelist) {
	if (routelist.size() <= 1)return;
	routelist[routelist.size() - 2].setleavetime(max(current_time, routelist[routelist.size() - 2].getServiceTime()));
	routelist[routelist.size() - 1].setarrivetime(routelist[routelist.size() - 2].getleavetime() + Time[location[routelist[routelist.size() - 2].getId()]][location[routelist[routelist.size() - 1].getId()]]);
	routelist[routelist.size() - 1].setServiceTime(routelist[routelist.size() - 1].getarrivetime() + operation_time +routelist[routelist.size() - 1].getoperationtime());
}

unordered_map<string, vector<orderitem>> allocate_order(vector<vector<double>> veichle_order_relation, vector<vector<double>> order_relation,
vector<orderitem>cuorders,vector<vector<double>> eta_veichleorder_relation) {
	unordered_map<string, vector<orderitem>> order_set;
	for (string temp : veichles) {
		order_set[temp];
	}
	// 先不考虑订单之间的关系，即不考虑集合内部的已有订单对接下来选择订单的影响
	for (int i = 0; i < cuorders.size(); i++) {
			vector<double> temp_problity; int location = 0; double compare = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], rho1);
			temp_problity.push_back(eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], rho1));
			double sum = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], rho1);
			for (int j = 1; j < veichle_num; j++) {
				if (compare > eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], rho1)) {
					location = j; compare = eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], rho1);
				}
				sum += eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], rho1);
				temp_problity.push_back(eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], rho1));
			}
			if (generateRandomNumber() <= q1) {
				order_set[veichles[findMaxIndex(temp_problity)]].push_back(cuorders[i]);
			}
			else {
				for (int j = 0; j < veichle_num; j++)temp_problity[j] /= sum;
				order_set[veichles[wheel_select(generateRandomNumber(), temp_problity,"fuck33")]].push_back(cuorders[i]);
			}
	}
	return order_set;
}

void single_timecaculate(vector<node>& temproutelist) {
	if (temproutelist.size() <= 1)return;
	if (temproutelist[1].getpickup()) {
		temproutelist[0].setleavetime(max(current_time, temproutelist[0].getServiceTime()));
	}
	else {
		temproutelist[0].setleavetime(temproutelist[0].getServiceTime());
	}
	for (int i = 1; i < temproutelist.size(); i++) {
		temproutelist[i].setarrivetime(temproutelist[i - 1].getleavetime() + Time[location[temproutelist[i - 1].getId()]][location[temproutelist[i].getId()]]);
		temproutelist[i].setServiceTime(temproutelist[i].getarrivetime() + operation_time + temproutelist[i].getoperationtime());
		if (i == temproutelist.size() - 1) {
			break;
		}
		else {
			if (temproutelist[i + 1].getpickup()) {
				temproutelist[i].setleavetime(max(temproutelist[i + 1].getpropertytime(), temproutelist[i].getServiceTime()));
			}
			else {
				temproutelist[i].setleavetime(temproutelist[i].getServiceTime());
			}
		}
	}
}

void single_insert(vector<node>&routelist,node pickup_node,node deliver_node) {
	int beginpos = 0, endpos = 1; double temp = 100000000; int min = 100000000;
	vector<node> templist;
	if (routelist.size() == 1) {
		temp = Distance[location[pickup_node.getId()]][location[routelist[0].getId()]] + Distance[location[pickup_node.getId()]][location[deliver_node.getId()]];
		temp += max(0, (deliver_node.getoperationtime() + operation_time + pickup_node.getoperationtime() + pickup_node.getpropertytime() + Time[location[pickup_node.getId()]][location[routelist[0].getId()]] + Time[location[pickup_node.getId()]][location[deliver_node.getId()]] - deliver_node.getpropertytime()));
		if (temp < min) {
			beginpos = 0;
			endpos = 1;
			min = temp;
		}
	}
	else {
		for (int i = 0; i < routelist.size(); i++) {
			vector<int> toinsertdeliver;
			for (int j = i + 1; j < routelist.size(); j++) {
				if (deliverjudge(routelist[i].getdeliverlist(), routelist[j].getbeforedeliverlist())) {
					toinsertdeliver.push_back(j);
				}
			}
			if (i == routelist.size() - 1) {
				toinsertdeliver.push_back(routelist.size());
			}
			for (int j : toinsertdeliver) {
				bool getout = 0;
				templist = routelist;
				// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
				insertBefore(templist, j, deliver_node);
				insertAfter(templist, i, pickup_node);
				PDlist(templist);
				for (int k = 0; k < templist.size(); k++) {
					if (templist[k].getstorage() < 0) {
						getout = 1; break;
					}
				}
				if (getout)continue;
				single_timecaculate(templist);
				temp = single_cost(templist);
				if (temp < min) {
					beginpos = i;
					endpos = j;
					min = temp;
				}
			}
		}
	}
	insertBefore(routelist, endpos, deliver_node);
	insertAfter(routelist, beginpos, pickup_node);
	PDlist(routelist);
	single_timecaculate(routelist);
}

// 先采用常规方法进行初始矩阵的完善
vector<vector<double>> initial_eta_veichleorder_relation(vector<orderitem> orderlist1,unordered_map<string,vector<node>> tempsolution,
	vector<vector<double>> veichle_order_relation, vector<vector<double>> order_relation) {
	double tao = 0.1;
	unordered_map<string, vector<orderitem>> unordered_orderset = allocate_order(veichle_order_relation, order_relation, orderlist1);
	for (string each : veichles) {
		vector<string> left_orderlist = tempsolution[each].back().getdeliverlist();
		for(string each_order:left_orderlist){
			orderitem neworder = orderlist[each_order];
			node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			tempsolution[each].push_back(deliver_node);
			PDlist_next(tempsolution[each],"fuck77");
		}
		for (orderitem neworder : unordered_orderset[each]) {
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()),
			deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			single_insert(tempsolution[each],pickup_node,deliver_node);
		}
	}
	Timecaculate(tempsolution);
	tao = 1.0 / total_cost(tempsolution);
	vector<vector<double>> eta_matrix(veichle_num, vector<double>(orderlist1.size(), tao));
	return eta_matrix;
}

int Total_node_count(unordered_map<string, vector<node>> tempsolution) {
	int sum = 0;
	for (string each : veichles) {
		sum +=tempsolution[each].size();
	}
	return sum;
}

vector<vector<double>>  initial_eta_node_matrix(vector<orderitem> temporderlist,vector<node> tempnodelist,int nodelistsize) {
	vector<string> tempdeliverlist = tempnodelist.back().getdeliverlist();
	
	for (string id : tempdeliverlist) {
		orderitem neworder = orderlist[id];
		node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
		tempnodelist.push_back(deliver_node);
		PDlist_next(tempnodelist, id);
		Timecaculate_next(tempnodelist);
	}

	for (orderitem neworder : temporderlist) {
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()),
				deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			single_insert(tempnodelist, pickup_node, deliver_node);
	}

	double tao = 1/single_cost(tempnodelist);
	vector<vector<double>> after_matrix(nodelistsize, vector<double>(nodelistsize, tao));
	return after_matrix;
}

unordered_map<string,vector<vector<double>>> initial_global_eta_node_matrix(unordered_map<string,vector<node>> startsolution,unordered_map<string,vector<vector<double>>> currentroute_node_relation,
unordered_map<string,unordered_map<string,int>> deliver_order_match,unordered_map<string,unordered_map<string,int>> pick_order_match,unordered_map<string,vector<int>> node_exist,unordered_map<string,int>current_car_location,
unordered_map<string,vector<orderitem>> unordered_solution,unordered_map<string,vector<node>> currentroute_nodelist){
	unordered_map<string,vector<node>> local_solution = startsolution;
	unordered_map<string,vector<int>> currentroute_exist = node_exist;
	int last_node;
	for(string topvalue:veichles){
		current_car_location[topvalue] = 0;
		while(currentroute_exist[topvalue].size()!=0){
			vector<int>select_node;vector<double>probility_of_nodeselect;double sum = 0;
			if(local_solution[topvalue].back().getdeliverlist().size()!=0){
				if(deliver_order_match[topvalue].count(local_solution[topvalue].back().getdeliverlist().back()) == 0){
					cout << "what fuck bro the deliverlist is 0" << endl;
					printsolution(local_solution);
					for(orderitem valur:unordered_solution[topvalue]){
						cout<<valur.getOrderId()<<" ";
					}
					cout<<local_solution[topvalue].back().getdeliverlist().back()<<endl;
					printsolution(startsolution);
					cout<<topvalue<<endl;
					cout<<"fuck"<<endl;
				}
				select_node.push_back(deliver_order_match[topvalue][local_solution[topvalue].back().getdeliverlist().back()]);
			}
			for(int i:currentroute_exist[topvalue]){
				if (i == 0)cout << "what fuck bro it involvo 0" << endl;
				if(currentroute_nodelist[topvalue][i].getpakage() > local_solution[topvalue].back().getstorage()){
					continue;
				}else{
					select_node.push_back(i);
				}
			}
			for(int i:select_node){
				sum += currentroute_node_relation[topvalue][current_car_location[topvalue]][i];
				probility_of_nodeselect.push_back(currentroute_node_relation[topvalue][current_car_location[topvalue]][i]);
			}
			for (int i = 0; i < probility_of_nodeselect.size(); i++) {
				probility_of_nodeselect[i] /= sum;
			}
			last_node = current_car_location[topvalue];
			if (generateRandomNumber() <= q2) {
				current_car_location[topvalue] = select_node[findMaxIndex(probility_of_nodeselect)];
			}
			else current_car_location[topvalue] = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
							
			if (currentroute_nodelist[topvalue][current_car_location[topvalue]].getpickup()) {
				int templocation = current_car_location[topvalue];
				currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
			}
							
			local_solution[topvalue].push_back(currentroute_nodelist[topvalue][current_car_location[topvalue]]);
			PDlist_next(local_solution[topvalue], "fuck3");
							
			}
			vector<string> left_orderlist = local_solution[topvalue].back().getdeliverlist();
			if(unordered_solution[topvalue].size() == 0){
			// 需要分成两种情况是因为其中有一种情况没有order就不进行分配orderlist了
			while(left_orderlist.size() != 0){
				orderitem neworder = orderlist[left_orderlist.back()];
				node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
				local_solution[topvalue].push_back(deliver_node);
				PDlist_next(local_solution[topvalue],"fuck99");
				left_orderlist.pop_back();
				}
				}else{
				while (left_orderlist.size() != 0) {
					last_node = current_car_location[topvalue];
					if(currentroute_nodelist[topvalue].size() - 1<deliver_order_match[topvalue][left_orderlist.back()]){
						cout<<"fuck that is it"<<endl;
						cout<<endl;
					}
					local_solution[topvalue].push_back(currentroute_nodelist[topvalue][deliver_order_match[topvalue][left_orderlist.back()]]);
					current_car_location[topvalue] = deliver_order_match[topvalue][left_orderlist.back()];
					// local update
					PDlist_next(local_solution[topvalue], "fuck4");
					left_orderlist.pop_back();
				}
			}
	}
	Timecaculate(local_solution);
	unordered_map<string,vector<vector<double>>> current_car_eta_matrix;
	for(string each_car:veichles){
		if(unordered_solution[each_car].size() == 0)continue;
		vector<vector<double>> matrix(currentroute_nodelist[each_car].size(),vector<double>(currentroute_nodelist[each_car].size(),1/single_cost(local_solution[each_car])));
		current_car_eta_matrix[each_car] = matrix;
	}
	return current_car_eta_matrix;
}

// 不仅仅留下一个结点而是去除掉成对的结点而已
vector<orderitem> getnewproblem(unordered_map<string,vector<node>>& solution,vector<orderitem>&todoorderlist, unordered_map<string, int>& order_location) {
	vector<orderitem> afterlist; int iteration = 0;
	unordered_set<string> namedorder;

	for (auto it = todoorderlist.begin(); it != todoorderlist.end();) {
		if (it->getCreationTime() >= (current_time - timeinterval) && it->getCreationTime() < current_time) {
			afterlist.push_back(*it);
			order_location[afterlist.back().getOrderId()] = iteration;
			iteration++;
			it = todoorderlist.erase(it);
		}
		else {
			if (it->getCreationTime() >= current_time)break;
			it++;
			cerr << "wrong with the order allocate" << endl;
		}
	}

	// 没有考虑到正在head to的订单
	for (string str : veichles) {
		auto it = solution[str].begin();
		while (it + 1 != solution[str].end()&&it->getServiceTime()<current_time) {
				it = solution[str].erase(it);
		}
		if (it + 1 == solution[str].end())continue;
		for (; it != solution[str].end(); ) {
			if (it->getpickup() == 1) {
				if (it == solution[str].begin()) {
					it++;
					continue;
				}
				namedorder.insert(it->getorderid());
				afterlist.push_back(orderlist[it->getorderid()]);
				order_location[afterlist.back().getOrderId()] = iteration;
				iteration++;
				it = solution[str].erase(it);
			}
			else {
				if (namedorder.count(it->getorderid()) == 1) {
					it = solution[str].erase(it);
				}
				else {
					it++;
				}
			}
		}
	}

	return afterlist;
}

// 尝试通过随机序列进行插入
vector<int> random_permutation(int n) {
	// 初始化一个包含 0 到 n - 1 的向量
	std::vector<int> permutation(n);
	for (int i = 0; i < n; ++i) {
		permutation[i] = i;
	}

	// 使用随机数引擎
	std::random_device rd;
	std::mt19937 g(rd());

	// 对向量进行随机重排
	std::shuffle(permutation.begin(), permutation.end(), g);

	return permutation;
}

vector<vector<double>> initialveichleorder_relation(unordered_map<string, vector<node>>currentsolution, vector<orderitem> currentorderlist) {
	vector<vector<double>>veichle_order_relation(veichle_num, vector<double>(currentorderlist.size(), 0));
	orderitem neworder;
	for (int i = 0; i < veichle_num;i++) {
		for (int j = 0; j < currentorderlist.size(); j++) {
			neworder = currentorderlist[j];
			vector<node> tempnodelist = currentsolution[veichles[i]];
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()),
				deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			single_insert(tempnodelist, pickup_node, deliver_node);
			veichle_order_relation[i][j] = 1 / single_cost(tempnodelist);
		}
	}
	return veichle_order_relation;
}

vector<vector<double>>  initial_eta_veichleorder_relation( unordered_map<string, vector<node>> tempsolution, vector<orderitem> orderlist,
	vector<vector<double>> veichle_order_relation, vector<vector<double>> order_relation) {
	double tao = 0.1;
	unordered_map<string, vector<orderitem>> unordered_orderset = allocate_order(veichle_order_relation, order_relation, orderlist);
	for (string each : veichles) {
		for (orderitem neworder : unordered_orderset[each]) {
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()),
				deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			single_insert(tempsolution[each], pickup_node, deliver_node);
		}
	}
	Timecaculate(tempsolution);
	tao = 1.0 / total_cost(tempsolution);
	vector<vector<double>> eta_matrix(veichle_num, vector<double>(orderlist.size(), tao));
	return eta_matrix;
}

// 计算均值
double calculateMean(const std::vector<double>& numbers) {
    double sum = 0.0;
    for (double num : numbers) {
        sum += num;
    }
    return sum / numbers.size();
}

// 计算方差
double calculateVariance(const std::vector<double>& numbers, double mean) {
    double variance = 0.0;
    for (double num : numbers) {
        variance += std::pow(num - mean, 2);
    }
    return variance / numbers.size();
}

void writeNumbersToCSV(std::vector<double>& numbers, std::string filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return;
    }

    // 写入原始数据
    for (size_t i = 0; i < numbers.size(); ++i) {
        file << numbers[i];
		file << ",";
    }

    // 计算均值和方差
    double mean = calculateMean(numbers);
    double variance = calculateVariance(numbers, mean);

    // 写入空格、均值和方差
    file << " " << mean << "," << variance << "\n";

    numbers.clear();
    file.close();
    // std::cout << "数据已成功写入 " << filename << std::endl;
}

#if 0
int main() {
	readtime(); readistance(); readfactory();
	int instance = 31;
	std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	for(int example = 0;example<=10;example++){
	//solution.clear();
	current_time = 600;
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_cost = 1000000000;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
		// disp_orderset(currentorderlist);

		/*for (string each : veichles) {
			for (string temporder : currentproblem[each].back().getdeliverlist())
				if (unmoved_order.count(temporder) == 0)cout << " fuck !!!!!!!!!! the spilit procedure is wrong" << endl;
		}*/

		vector<vector<double>> order_relation(currentorderlist.size(),vector<double>(currentorderlist.size()));
		// 该顺序和orderlist以及veichles的内部顺序有关系
		vector<vector<double>>veichle_order_relation(veichle_num,vector<double>(currentorderlist.size(),0));
		initialorder_relation(order_relation, currentorderlist);
		initialveichleorder_relation(veichle_order_relation, startsolution, currentorderlist);
		vector<vector<double>>eta_veichleorder_relation = initial_eta_veichleorder_relation(currentorderlist, startsolution, veichle_order_relation, order_relation);
		double order_tao = eta_veichleorder_relation[0][0]*double(veichle_num);
		for (int iter1 = 0; iter1 < solution_construct_interation; iter1++) {
			// 完成每一个车辆的order分配
			// 解的回归
			unordered_map<string,vector<orderitem>> local_bestproblem1;double local_best_cost3 = 10000000000;
			for (int ant_order = 0; ant_order < antnum; ant_order++) {
				unordered_map<string, vector<orderitem>> unordered_solution;
				unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
				for (int i = 0; i < veichle_num; i++) {
					for (orderitem temp_order : unordered_solution[veichles[i]]) {
						eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - beta_der) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + beta_der * order_tao;
					}
				}

					// 对每一辆车进行遍历构建
				for (int each = 0; each < veichle_num; each++) {
					if (unordered_solution[veichles[each]].size() == 0)continue;
					unordered_map<string, int> order_dnode, order_pnode; vector<int> existpickup;
					vector<node> patial_order_nodelist = local_node_construct(unordered_solution[veichles[each]], order_dnode, order_pnode, existpickup, startsolution[veichles[each]]);
					vector <vector<double>> patial_order_relation = local_node_matrix(patial_order_nodelist);
					vector<vector<double>> eta_node_relation = initial_eta_node_matrix(unordered_solution[veichles[each]], startsolution[veichles[each]], patial_order_nodelist.size());
					double local_tao = eta_node_relation[0][0] / double(patial_order_nodelist.size());
					vector<node> localproblem = startsolution[veichles[each]];
					vector<node> local_best_problem = localproblem; double best_local_cost = 100000000;
					// 每条路径的构建,两种构建策略，一种是对单车辆执行插入操作得到最优，第二种是通过蚁群算法构建路径
					// 对局部的信息进行快速收敛排列，但是可能会忽略总体情况上的时间重叠等情况
					for (int iter2 = 0; iter2 < route_construct_interation; iter2++) {
						vector<node> local_best_route; double local_best_route_cost = 100000000;
						for (int route_ant = 0; route_ant < antnum; route_ant++) {
							// 对每一个车辆的路径进行构建
							localproblem = startsolution[veichles[each]];
							int current_car_location = 0; int last_node;
							vector<int> exist_pickup_node = existpickup;
							while (exist_pickup_node.size() != 0) {
								vector<int> select_node; vector<double> probility_of_nodeselect; double sum = 0;
								if (localproblem.back().getdeliverlist().size() != 0) {
									if (order_dnode[localproblem.back().getdeliverlist().back()] == 0) {
										cout << "what fuck bro the deliverlist is 0" << endl;
									}
									select_node.push_back(order_dnode[localproblem.back().getdeliverlist().back()]);
								}
								for (int i : exist_pickup_node) {
									if (i == 0)cout << "what fuck bro it involvo 0" << endl;
									if (patial_order_nodelist[i].getpakage() > localproblem.back().getstorage()) {
										continue;
									}
									else {
										select_node.push_back(i);
									}
								}
								//!!!!!标准化除的时double
								for (int i : select_node) {
									sum += pow(patial_order_relation[current_car_location][i], rho2) * eta_node_relation[current_car_location][i];
									probility_of_nodeselect.push_back(pow(patial_order_relation[current_car_location][i], rho2) * eta_node_relation[current_car_location][i]);
								}

								for (int i = 0; i < probility_of_nodeselect.size(); i++) {
									probility_of_nodeselect[i] /= sum;
								}
								// 找到当下，接下来的一个顶点所要去的位置
								last_node = current_car_location;
								if (generateRandomNumber() <= q2) {
									current_car_location = select_node[findMaxIndex(probility_of_nodeselect)];
								}
								else current_car_location = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck55")];

								if (patial_order_nodelist[current_car_location].getpickup()) {
									exist_pickup_node.erase(std::remove_if(exist_pickup_node.begin(), exist_pickup_node.end(), [current_car_location](int value) { return value == current_car_location; }), exist_pickup_node.end());
								}
								localproblem.push_back(patial_order_nodelist[current_car_location]);
								// local update
								eta_node_relation[last_node][current_car_location] = (1 - beta_der) * eta_node_relation[last_node][current_car_location] + beta_der * local_tao;
								eta_node_relation[current_car_location][last_node] = eta_node_relation[last_node][current_car_location];
								PDlist_next(localproblem, "fuck3");
								Timecaculate_next(localproblem);
							}
							// 接着按照当前的deliverlist取送货
							vector<string> left_orderlist = localproblem.back().getdeliverlist();
							while (left_orderlist.size() != 0) {
								last_node = current_car_location;
								localproblem.push_back(patial_order_nodelist[order_dnode[left_orderlist.back()]]);
								current_car_location = order_dnode[left_orderlist.back()];
								// local update
								eta_node_relation[last_node][current_car_location] = (1 - beta_der) * eta_node_relation[last_node][current_car_location] + beta_der * local_tao;
								eta_node_relation[current_car_location][last_node] = eta_node_relation[last_node][current_car_location];
								PDlist_next(localproblem, "fuck4");
								Timecaculate_next(localproblem);
								left_orderlist.pop_back();
							}
							double local_temp_cost = single_cost(localproblem);
							if (local_temp_cost < local_best_route_cost) {
								local_best_route = localproblem;
								local_best_route_cost = local_temp_cost;
							}
						}
						// golabl update
						// 用local_best_route进行信息素更新

						if (local_best_route_cost < best_local_cost) {
							best_local_cost = local_best_route_cost;
							local_best_problem = local_best_route;
						}
						int firstnode = 0, secondnode;
						for (int i = 0; i < local_best_problem.size() - 2; i++) {
							if (local_best_problem[i + 1].getpickup()) {
								secondnode = order_pnode[local_best_problem[i + 1].getorderid()];
							}
							else {
								secondnode = order_dnode[local_best_problem[i + 1].getorderid()];
							}
							eta_node_relation[firstnode][secondnode] = (1 - alpha2) * eta_node_relation[firstnode][secondnode] + alpha2 / best_local_cost;
							eta_node_relation[firstnode][secondnode] = eta_node_relation[secondnode][firstnode];
							firstnode = secondnode;
						}
					}
					startsolution[veichles[each]] = local_best_problem;
				}
				// 更新order的分配矩阵
				// 可能需要考虑，不同车辆之间的overlap从而根据这一特性进行加权
				// 可以用单个时间之和和总拟合之后的差值作为惩罚依据，但是未知是哪两辆车的影响
				/*double tempcost_each = 0;
				for (string tempeach : veichles) {
					tempcost_each += single_cost(currentproblem[tempeach]);
				}*/
				Timecaculate(startsolution);
				
			}
			for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : best_currentproblem[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - alpha2)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + alpha2* best_currentproblem_cost;
				}
			}
		}
		mergesolution(solution, best_currentsolution);
		startsolution = best_currentsolution;
		current_time += timeinterval;
	}
	cout << total_cost(solution)<<" ";
}
}
#endif

#if 0
int main() {
	readtime(); readistance(); readfactory();
	int instance = 50;
	std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	for(int example = 0;example<=10;example++){
	//solution.clear();
	current_time = 600;
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 10000000000;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
	//	printsolution(startsolution);
		
		vector<vector<double>> order_relation(currentorderlist.size(),vector<double>(currentorderlist.size()));
		// 该顺序和orderlist以及veichles的内部顺序有关系
		vector<vector<double>>veichle_order_relation(veichle_num,vector<double>(currentorderlist.size(),0));
		initialorder_relation(order_relation, currentorderlist);
		initialveichleorder_relation(veichle_order_relation, startsolution, currentorderlist);
		vector<vector<double>>eta_veichleorder_relation = initial_eta_veichleorder_relation(currentorderlist, startsolution, veichle_order_relation, order_relation);
		double order_tao = eta_veichleorder_relation[0][0]*double(veichle_num);
		// printsolution(currentsolution);
		for (int iter1 = 0; iter1 < solution_construct_interation; iter1++) {
			// 完成每一个车辆的order分配
			// 解的回归
			unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;

			for (int ant_order = 0; ant_order < antnum; ant_order++) {
				unordered_map<string, vector<orderitem>> unordered_solution;
				unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
				/*for(string each:veichles){
					cout<<each<<" ";
					for(orderitem value:unordered_solution[each]){
						cout<<value.getOrderId()<<" ";
					}
					cout<<endl;
				}*/
				for (int i = 0; i < veichle_num; i++) {
					for (orderitem temp_order : unordered_solution[veichles[i]]) {
						eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - beta_der) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + beta_der * order_tao;
					}
				}
				/*for(int i = 0;i<veichle_num;i++){
					for(int j = 0;j<currentorderlist.size();j++)
					cout<<eta_veichleorder_relation[i][j]<<" ";
				}*/
				unordered_map<string,vector<node>>local_bestsolution2;
				double local_best_cost2 = 10000000000;
				// 车辆优先级构建，构建过程过程中一边计算节点的时间
				unordered_map<string,vector<vector<double>>> currentroute_eta_relation;
				unordered_map<string,vector<vector<double>>> currentroute_node_relation;
				unordered_map<string,vector<node>> currentroute_nodelist;
				unordered_map<string,unordered_map<string,int>> deliver_order_match;
				unordered_map<string,unordered_map<string,int>> pick_order_match;
				unordered_map<string,vector<int>>node_exist;
				unordered_map<string,double> local_tao;
				unordered_map<string,int> current_car_location;
				for(string value:veichles){
					if(unordered_solution[value].size() == 0){
						node_exist[value];
						continue;
					}
					unordered_map<string,int>order_dnode,order_pnode;vector<int> existpickup;
					currentroute_nodelist[value] = local_node_construct(unordered_solution[value],order_dnode,order_pnode,existpickup,startsolution[value]);
					deliver_order_match[value] = order_dnode;pick_order_match[value] = order_pnode;node_exist[value] = existpickup;

					currentroute_node_relation[value] = local_node_matrix(currentroute_nodelist[value]);
					currentroute_eta_relation[value] = initial_eta_node_matrix(unordered_solution[value],startsolution[value],currentroute_nodelist[value].size());
					local_tao[value] = currentroute_eta_relation[value][0][0]/double(currentroute_nodelist[value].size());
					current_car_location[value] = 0;
				}
				/*for(string each_car:veichles){
					cout<<each_car<<" ";
					for(node tempnode:currentroute_nodelist[each_car]){
						cout<<tempnode.getorderid()<<"-"<<tempnode.getpickup()<<" ";
					}
				}*/
				for(int iter2 = 0;iter2<route_construct_interation;iter2++){
					unordered_map<string,vector<node>>local_bestsolution1;
					double local_best_cost1 = 10000000000;

					for(int ant_route = 0;ant_route < antnum;ant_route++){
						//cout<<ant_route<<" ";
						string topvalue;std::queue<node> factorydocks[154];
						unordered_map<string,vector<node>> local_solution = startsolution;
						queue<node>factory[154];
						auto compare = [&local_solution](std::string& a, std::string& b) {
							return local_solution[a].back().getarrivetime() > local_solution[b].back().getarrivetime();
						};
						std::priority_queue<std::string, std::vector<std::string>, decltype(compare)> pq(compare);
						for(string each_car:veichles){
							if(unordered_solution[each_car].size() == 0&&startsolution[each_car].back().getdeliverlist().size() == 0){
								continue;
							}
							pq.push(each_car);
						}
						unordered_map<string,vector<int>> currentroute_exist = node_exist;
					while(!pq.empty()){
						topvalue = pq.top();
						pq.pop();
						if(currentroute_exist[topvalue].size()!=0){
								vector<int>select_node;vector<double>probility_of_nodeselect;double sum = 0;
							if(local_solution[topvalue].back().getdeliverlist().size()!=0){
								if(deliver_order_match[topvalue].count(local_solution[topvalue].back().getdeliverlist().back()) == 0){
									cout << "what fuck bro the deliverlist is 0" << endl;
									printsolution(local_solution);
									for(orderitem valur:unordered_solution[topvalue]){
										cout<<valur.getOrderId()<<" ";
									}
									cout<<local_solution[topvalue].back().getdeliverlist().back()<<endl;
									printsolution(startsolution);
									cout<<topvalue<<endl;
									cout<<"fuck"<<endl;
								}
								select_node.push_back(deliver_order_match[topvalue][local_solution[topvalue].back().getdeliverlist().back()]);
							}
							for(int i:currentroute_exist[topvalue]){
								if (i == 0)cout << "what fuck bro it involvo 0" << endl;
								if(currentroute_nodelist[topvalue][i].getpakage() > local_solution[topvalue].back().getstorage()){
									continue;
								}else{
									select_node.push_back(i);
								}
							}
							for(int i:select_node){
								sum += pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i];
								probility_of_nodeselect.push_back(pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i]);
							}
							for (int i = 0; i < probility_of_nodeselect.size(); i++) {
								probility_of_nodeselect[i] /= sum;
							}
							int last_node = current_car_location[topvalue];
							if (generateRandomNumber() <= q2) {
								current_car_location[topvalue] = select_node[findMaxIndex(probility_of_nodeselect)];
							}
							else current_car_location[topvalue] = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
							
							if (currentroute_nodelist[topvalue][current_car_location[topvalue]].getpickup()) {
								int templocation = current_car_location[topvalue];
								currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
							}
							local_solution[topvalue].back().setleavetime(max(current_time,local_solution[topvalue].back().getServiceTime()));
							local_solution[topvalue].push_back(currentroute_nodelist[topvalue][current_car_location[topvalue]]);
							local_solution[topvalue].back().setarrivetime(local_solution[topvalue][local_solution[topvalue].size() - 2].getleavetime() + Time[location[local_solution[topvalue][local_solution[topvalue].size() - 2].getId()]][location[local_solution[topvalue].back().getId()]]);
							int delay = cleardocks(location[local_solution[topvalue].back().getId()],local_solution[topvalue].back().getarrivetime(),factorydocks) - local_solution[topvalue].back().getarrivetime();
							local_solution[topvalue].back().setServiceTime(local_solution[topvalue].back().getarrivetime() + delay + local_solution[topvalue].back().getoperationtime());
							factorydocks[location[local_solution[topvalue].back().getId()]].push(local_solution[topvalue].back());
							PDlist_next(local_solution[topvalue], "fuck3");
							currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
							currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
							if(currentroute_exist[topvalue].size()==0&&local_solution[topvalue].back().getdeliverlist().size() == 0){
							}else{
								pq.push(topvalue);
							}
						}else{
							if(unordered_solution[topvalue].size() == 0){
								local_solution[topvalue].back().setleavetime(max(current_time,local_solution[topvalue].back().getServiceTime()));
								orderitem neworder = orderlist[local_solution[topvalue].back().getdeliverlist().back()];
								node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
								local_solution[topvalue].push_back(deliver_node);
								local_solution[topvalue].back().setarrivetime(local_solution[topvalue][local_solution[topvalue].size() - 2].getleavetime() + Time[location[local_solution[topvalue][local_solution[topvalue].size() - 2].getId()]][location[local_solution[topvalue][local_solution[topvalue].size() - 1].getId()]]);
								int delay = cleardocks(location[local_solution[topvalue].back().getId()],local_solution[topvalue].back().getarrivetime(),factorydocks) - local_solution[topvalue].back().getarrivetime();
								local_solution[topvalue].back().setServiceTime(local_solution[topvalue].back().getarrivetime() + delay + local_solution[topvalue].back().getoperationtime());
								factorydocks[location[local_solution[topvalue].back().getId()]].push(local_solution[topvalue].back());
								PDlist_next(local_solution[topvalue],"fuck1");

							}else{
							int last_node = current_car_location[topvalue];
							current_car_location[topvalue] = deliver_order_match[topvalue][local_solution[topvalue].back().getdeliverlist().back()];
							local_solution[topvalue].back().setleavetime(max(current_time,local_solution[topvalue].back().getServiceTime()));
							local_solution[topvalue].push_back(currentroute_nodelist[topvalue][current_car_location[topvalue]]);
							local_solution[topvalue].back().setarrivetime(local_solution[topvalue][local_solution[topvalue].size() - 2].getleavetime() + Time[location[local_solution[topvalue][local_solution[topvalue].size() - 2].getId()]][location[local_solution[topvalue].back().getId()]]);
							int delay = cleardocks(location[local_solution[topvalue].back().getId()],local_solution[topvalue].back().getarrivetime(),factorydocks) - local_solution[topvalue].back().getarrivetime();
							local_solution[topvalue].back().setServiceTime(local_solution[topvalue].back().getarrivetime() + delay + local_solution[topvalue].back().getoperationtime());
							factorydocks[location[local_solution[topvalue].back().getId()]].push(local_solution[topvalue].back());
							PDlist_next(local_solution[topvalue],"fuck4");
							current_car_location[topvalue] = deliver_order_match[topvalue][local_solution[topvalue].back().getorderid()];
							currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
							currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
							}
							if(local_solution[topvalue].back().getdeliverlist().size() == 0){
							}else{
								pq.push(topvalue);
							}
						}
					}
					double tempcost = total_cost(local_solution);
					//cout<<tempcost<<endl;
						if(local_best_cost1>tempcost){
							local_best_cost1 = tempcost;
							local_bestsolution1 = local_solution;
						}
				}
					// global update
					for(string each_car:veichles){
						double delta_local_bestsolution1 = single_cost(local_bestsolution1[each_car]);
						if(unordered_solution[each_car].size() == 0)continue;
						int firstnode = 0,secondnode;
						for(int i = 0;i<local_bestsolution1[each_car].size() - 2;i++){
							if(local_bestsolution1[each_car][i+1].getpickup()){
								secondnode = pick_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}else{
								secondnode = deliver_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}
							currentroute_eta_relation[each_car][firstnode][secondnode] = (1 - alpha2) * currentroute_eta_relation[each_car][firstnode][secondnode] + alpha2/delta_local_bestsolution1;
							currentroute_eta_relation[each_car][secondnode][firstnode] = currentroute_eta_relation[each_car][firstnode][secondnode];
							firstnode = secondnode;
						}

					}

					if(local_best_cost2>local_best_cost1){
						local_best_cost2 = local_best_cost1;
						local_bestsolution2 = local_bestsolution1;
					}
					//cout<<"fuck"<<endl;
				}
				if(local_best_cost3>local_best_cost2){
					local_bestsolution3 = local_bestsolution2;
					local_best_cost3 = local_best_cost2;
					local_bestproblem1 = unordered_solution;
				}

			}
				// 以上是使用蚁群算法，进行优先顺序进行构建同时构建过程当中计算各节点的时间
			double delta_currentorder = 1/local_best_cost3;
			for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_bestproblem1[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - alpha2)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + alpha2* delta_currentorder;
				}
			}
			if(global_best_cost>local_best_cost3){
				global_best_cost = local_best_cost3;
				best_currentsolution = local_bestsolution3;
			}
		}
		//printsolution(best_currentsolution);
		mergesolution(solution, best_currentsolution);
		startsolution = best_currentsolution;
		current_time += timeinterval;
	}
	cout << total_cost(solution)<<" "<<Total_node_count(solution);
	// printsolution(solution);
}
}
#endif

// 四层循环，单独设计了路径构建的蚂蚁矩阵
#if 0
int main() {
	readtime(); readistance(); readfactory();
	int instance = 1;
	// for(int instance = 1;instance <=64;instance++){
	std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	// orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	//solution.clear();
	current_time = 600;
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 10000000000;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
	//	printsolution(startsolution);
		
		vector<vector<double>> order_relation(currentorderlist.size(),vector<double>(currentorderlist.size()));
		// 该顺序和orderlist以及veichles的内部顺序有关系
		vector<vector<double>>veichle_order_relation(veichle_num,vector<double>(currentorderlist.size(),0));
		initialorder_relation(order_relation, currentorderlist);
		initialveichleorder_relation(veichle_order_relation, startsolution, currentorderlist);
		vector<vector<double>>eta_veichleorder_relation = initial_eta_veichleorder_relation(currentorderlist, startsolution, veichle_order_relation, order_relation);
		double order_tao = eta_veichleorder_relation[0][0]/double(veichle_num);
		// printsolution(currentsolution);
		for (int iter1 = 0; iter1 < solution_construct_interation; iter1++) {
			// 完成每一个车辆的order分配
			// 解的回归
			unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;

			for (int ant_order = 0; ant_order < antnum; ant_order++) {
				unordered_map<string, vector<orderitem>> unordered_solution;
				unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
				/*for(string each:veichles){
					cout<<each<<" ";
					for(orderitem value:unordered_solution[each]){
						cout<<value.getOrderId()<<" ";
					}
					cout<<endl;
				}*/
				for (int i = 0; i < veichle_num; i++) {
					for (orderitem temp_order : unordered_solution[veichles[i]]) {
						eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - beta_der) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + beta_der * order_tao;
					}
				}
				/*for(int i = 0;i<veichle_num;i++){
					for(int j = 0;j<currentorderlist.size();j++)
					cout<<eta_veichleorder_relation[i][j]<<" ";
				}*/
				unordered_map<string,vector<node>>local_bestsolution2;
				double local_best_cost2 = 10000000000;
				// 车辆优先级构建，构建过程过程中一边计算节点的时间
				unordered_map<string,vector<vector<double>>> currentroute_eta_relation;
				unordered_map<string,vector<vector<double>>> currentroute_node_relation;
				unordered_map<string,vector<node>> currentroute_nodelist;
				unordered_map<string,unordered_map<string,int>> deliver_order_match;
				unordered_map<string,unordered_map<string,int>> pick_order_match;
				unordered_map<string,vector<int>>node_exist;
				unordered_map<string,double> local_tao;
				unordered_map<string,int> current_car_location;
				for(string value:veichles){
					if(unordered_solution[value].size() == 0){
						node_exist[value];
						continue;
					}
					unordered_map<string,int>order_dnode,order_pnode;vector<int> existpickup;
					currentroute_nodelist[value] = local_node_construct(unordered_solution[value],order_dnode,order_pnode,existpickup,startsolution[value]);
					deliver_order_match[value] = order_dnode;pick_order_match[value] = order_pnode;node_exist[value] = existpickup;
					currentroute_node_relation[value] = local_node_matrix(currentroute_nodelist[value]);
				}
				currentroute_eta_relation = initial_global_eta_node_matrix(startsolution,currentroute_node_relation,deliver_order_match,pick_order_match,node_exist,current_car_location,unordered_solution,currentroute_nodelist);
				for(string value:veichles){
					if(unordered_solution[value].size() == 0)continue;
					local_tao[value] = currentroute_eta_relation[value][0][0]/double(currentroute_nodelist[value].size());
				}

				/*for(string each_car:veichles){
					cout<<each_car<<" ";
					for(node tempnode:currentroute_nodelist[each_car]){
						cout<<tempnode.getorderid()<<"-"<<tempnode.getpickup()<<" ";
					}
				}*/
				for(int iter2 = 0;iter2<route_construct_interation;iter2++){
					unordered_map<string,vector<node>>local_bestsolution1;
					double local_best_cost1 = 10000000000;

					for(int ant_route = 0;ant_route < antnum;ant_route++){
						//cout<<ant_route<<" ";
						unordered_map<string,vector<node>> local_solution = startsolution;
						unordered_map<string,vector<int>> currentroute_exist = node_exist;
						int last_node;
					for(string topvalue:veichles){
						current_car_location[topvalue] = 0;
						while(currentroute_exist[topvalue].size()!=0){
								vector<int>select_node;vector<double>probility_of_nodeselect;double sum = 0;
							if(local_solution[topvalue].back().getdeliverlist().size()!=0){
								if(deliver_order_match[topvalue].count(local_solution[topvalue].back().getdeliverlist().back()) == 0){
									cout << "what fuck bro the deliverlist is 0" << endl;
									printsolution(local_solution);
									for(orderitem valur:unordered_solution[topvalue]){
										cout<<valur.getOrderId()<<" ";
									}
									cout<<local_solution[topvalue].back().getdeliverlist().back()<<endl;
									printsolution(startsolution);
									cout<<topvalue<<endl;
									cout<<"fuck"<<endl;
								}
								select_node.push_back(deliver_order_match[topvalue][local_solution[topvalue].back().getdeliverlist().back()]);
							}
							for(int i:currentroute_exist[topvalue]){
								if (i == 0)cout << "what fuck bro it involvo 0" << endl;
								if(currentroute_nodelist[topvalue][i].getpakage() > local_solution[topvalue].back().getstorage()){
									continue;
								}else{
									select_node.push_back(i);
								}
							}
							for(int i:select_node){
								sum += pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i];
								probility_of_nodeselect.push_back(pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i]);
							}
							for (int i = 0; i < probility_of_nodeselect.size(); i++) {
								probility_of_nodeselect[i] /= sum;
							}
							last_node = current_car_location[topvalue];
							if (generateRandomNumber() <= q2) {
								current_car_location[topvalue] = select_node[findMaxIndex(probility_of_nodeselect)];
							}
							else current_car_location[topvalue] = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
							
							if (currentroute_nodelist[topvalue][current_car_location[topvalue]].getpickup()) {
								int templocation = current_car_location[topvalue];
								currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
							}
							
							local_solution[topvalue].push_back(currentroute_nodelist[topvalue][current_car_location[topvalue]]);
							PDlist_next(local_solution[topvalue], "fuck3");
							currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
							currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
						}
						vector<string> left_orderlist = local_solution[topvalue].back().getdeliverlist();
						if(unordered_solution[topvalue].size() == 0){
							// 需要分成两种情况是因为其中有一种情况没有order就不进行分配orderlist了
							while(left_orderlist.size() != 0){
								orderitem neworder = orderlist[left_orderlist.back()];
								node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
								local_solution[topvalue].push_back(deliver_node);
								PDlist_next(local_solution[topvalue],"fuck99");
								left_orderlist.pop_back();
							}
						}else{
							while (left_orderlist.size() != 0) {
								last_node = current_car_location[topvalue];
								if(currentroute_nodelist[topvalue].size() - 1<deliver_order_match[topvalue][left_orderlist.back()]){
									cout<<"fuck that is it"<<endl;
									cout<<endl;
								}
								local_solution[topvalue].push_back(currentroute_nodelist[topvalue][deliver_order_match[topvalue][left_orderlist.back()]]);
								current_car_location[topvalue] = deliver_order_match[topvalue][left_orderlist.back()];
								// local update
								currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
								currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
								PDlist_next(local_solution[topvalue], "fuck4");
								left_orderlist.pop_back();
							}
						}
					}
					Timecaculate(local_solution);
					double tempcost = total_cost(local_solution);
					//cout<<tempcost<<endl;
						if(local_best_cost1>tempcost){
							local_best_cost1 = tempcost;
							local_bestsolution1 = local_solution;
						}
				}
					// global update
					for(string each_car:veichles){
						double delta_local_bestsolution1 = single_cost(local_bestsolution1[each_car]);
						if(unordered_solution[each_car].size() == 0)continue;
						int firstnode = 0,secondnode;
						for(int i = 0;i<local_bestsolution1[each_car].size() - 2;i++){
							if(local_bestsolution1[each_car][i+1].getpickup()){
								secondnode = pick_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}else{
								secondnode = deliver_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}
							currentroute_eta_relation[each_car][firstnode][secondnode] = (1 - alpha2) * currentroute_eta_relation[each_car][firstnode][secondnode] + alpha2/delta_local_bestsolution1;
							currentroute_eta_relation[each_car][secondnode][firstnode] = currentroute_eta_relation[each_car][firstnode][secondnode];
							firstnode = secondnode;
						}

					}

					if(local_best_cost2>local_best_cost1){
						local_best_cost2 = local_best_cost1;
						local_bestsolution2 = local_bestsolution1;
					}
					//cout<<"fuck"<<endl;
				}
				if(local_best_cost3>local_best_cost2){
					local_bestsolution3 = local_bestsolution2;
					local_best_cost3 = local_best_cost2;
					local_bestproblem1 = unordered_solution;
				}

			}
				// 以上是使用蚁群算法，进行优先顺序进行构建同时构建过程当中计算各节点的时间
			double delta_currentorder = 1/local_best_cost3;
			for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_bestproblem1[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - alpha2)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + alpha2* delta_currentorder;
				}
			}
			if(global_best_cost>local_best_cost3){
				global_best_cost = local_best_cost3;
				best_currentsolution = local_bestsolution3;
			}
		}
		//printsolution(best_currentsolution);
		mergesolution(solution, best_currentsolution);
		startsolution = best_currentsolution;
		current_time += timeinterval;
		//printsolution(solution);
	}
	cost_data.push_back(total_cost(solution));
}
cout<<Total_node_count(solution) - veichle_num - 2*order.size()<<" ";
cout<<"Mean: "<<calculateMean(cost_data)<<"   Variance: "<<calculateVariance(cost_data,calculateMean(cost_data))<<"  Best: "<<findMin(cost_data)<<endl;
// }
}
#endif

// 只设置了三个循环，没有嵌套路径蚂蚁构建矩阵
#if 1
int main() {
	readtime(); readistance(); readfactory();
	int instance = 1;
	// for(int instance = 1;instance <=64;instance++){
	std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	// orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	//solution.clear();
	current_time = 600;
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 10000000000;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
	//	printsolution(startsolution);
		
		vector<vector<double>> order_relation(currentorderlist.size(),vector<double>(currentorderlist.size()));
		// 该顺序和orderlist以及veichles的内部顺序有关系
		vector<vector<double>>veichle_order_relation(veichle_num,vector<double>(currentorderlist.size(),0));
		initialorder_relation(order_relation, currentorderlist);
		initialveichleorder_relation(veichle_order_relation, startsolution, currentorderlist);
		vector<vector<double>>eta_veichleorder_relation = initial_eta_veichleorder_relation(currentorderlist, startsolution, veichle_order_relation, order_relation);
		double order_tao = eta_veichleorder_relation[0][0]/double(currentorderlist.size());
		// printsolution(currentsolution);
		for (int iter1 = 0; iter1 < solution_construct_interation; iter1++) {
			// 完成每一个车辆的order分配
			// 解的回归
			unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;

			for (int ant_order = 0; ant_order < antnum; ant_order++) {
				unordered_map<string, vector<orderitem>> unordered_solution;
				unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
				/*for(string each:veichles){
					cout<<each<<" ";
					for(orderitem value:unordered_solution[each]){
						cout<<value.getOrderId()<<" ";
					}
					cout<<endl;
				}*/
				for (int i = 0; i < veichle_num; i++) {
					for (orderitem temp_order : unordered_solution[veichles[i]]) {
						eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - beta_der) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + beta_der * order_tao;
					}
				}
				/*for(int i = 0;i<veichle_num;i++){
					for(int j = 0;j<currentorderlist.size();j++)
					cout<<eta_veichleorder_relation[i][j]<<" ";
				}*/
				unordered_map<string,vector<node>>local_bestsolution2;
				double local_best_cost2 = 10000000000;
				// 车辆优先级构建，构建过程过程中一边计算节点的时间
				unordered_map<string,vector<vector<double>>> currentroute_eta_relation;
				unordered_map<string,vector<vector<double>>> currentroute_node_relation;
				unordered_map<string,vector<node>> currentroute_nodelist;
				unordered_map<string,unordered_map<string,int>> deliver_order_match;
				unordered_map<string,unordered_map<string,int>> pick_order_match;
				unordered_map<string,vector<int>>node_exist;
				unordered_map<string,double> local_tao;
				unordered_map<string,int> current_car_location;
				for(string value:veichles){
					if(unordered_solution[value].size() == 0){
						node_exist[value];
						continue;
					}
					unordered_map<string,int>order_dnode,order_pnode;vector<int> existpickup;
					currentroute_nodelist[value] = local_node_construct(unordered_solution[value],order_dnode,order_pnode,existpickup,startsolution[value]);
					deliver_order_match[value] = order_dnode;pick_order_match[value] = order_pnode;node_exist[value] = existpickup;
					currentroute_node_relation[value] = local_node_matrix(currentroute_nodelist[value]);
				}
				currentroute_eta_relation = initial_global_eta_node_matrix(startsolution,currentroute_node_relation,deliver_order_match,pick_order_match,node_exist,current_car_location,unordered_solution,currentroute_nodelist);
				for(string value:veichles){
					if(unordered_solution[value].size() == 0)continue;
					local_tao[value] = currentroute_eta_relation[value][0][0]/double(currentroute_nodelist[value].size());
				}

				/*for(string each_car:veichles){
					cout<<each_car<<" ";
					for(node tempnode:currentroute_nodelist[each_car]){
						cout<<tempnode.getorderid()<<"-"<<tempnode.getpickup()<<" ";
					}
				}*/
				for(int iter2 = 0;iter2<route_construct_interation;iter2++){
					unordered_map<string,vector<node>>local_bestsolution1;
					double local_best_cost1 = 10000000000;
						unordered_map<string,vector<node>> local_solution = startsolution;
						unordered_map<string,vector<int>> currentroute_exist = node_exist;
						int last_node;
					for(string topvalue:veichles){
						current_car_location[topvalue] = 0;
						while(currentroute_exist[topvalue].size()!=0){
								vector<int>select_node;vector<double>probility_of_nodeselect;double sum = 0;
							if(local_solution[topvalue].back().getdeliverlist().size()!=0){
								if(deliver_order_match[topvalue].count(local_solution[topvalue].back().getdeliverlist().back()) == 0){
									cout << "what fuck bro the deliverlist is 0" << endl;
									printsolution(local_solution);
									for(orderitem valur:unordered_solution[topvalue]){
										cout<<valur.getOrderId()<<" ";
									}
									cout<<local_solution[topvalue].back().getdeliverlist().back()<<endl;
									printsolution(startsolution);
									cout<<topvalue<<endl;
									cout<<"fuck"<<endl;
								}
								select_node.push_back(deliver_order_match[topvalue][local_solution[topvalue].back().getdeliverlist().back()]);
							}
							for(int i:currentroute_exist[topvalue]){
								if (i == 0)cout << "what fuck bro it involvo 0" << endl;
								if(currentroute_nodelist[topvalue][i].getpakage() > local_solution[topvalue].back().getstorage()){
									continue;
								}else{
									select_node.push_back(i);
								}
							}
							for(int i:select_node){
								sum += pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i];
								probility_of_nodeselect.push_back(pow(currentroute_node_relation[topvalue][current_car_location[topvalue]][i], rho2) * currentroute_eta_relation[topvalue][current_car_location[topvalue]][i]);
							}
							for (int i = 0; i < probility_of_nodeselect.size(); i++) {
								probility_of_nodeselect[i] /= sum;
							}
							last_node = current_car_location[topvalue];
							if (generateRandomNumber() <= q2) {
								current_car_location[topvalue] = select_node[findMaxIndex(probility_of_nodeselect)];
							}
							else current_car_location[topvalue] = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
							
							if (currentroute_nodelist[topvalue][current_car_location[topvalue]].getpickup()) {
								int templocation = current_car_location[topvalue];
								currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
							}
							
							local_solution[topvalue].push_back(currentroute_nodelist[topvalue][current_car_location[topvalue]]);
							PDlist_next(local_solution[topvalue], "fuck3");
							currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
							currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
						}
						vector<string> left_orderlist = local_solution[topvalue].back().getdeliverlist();
						if(unordered_solution[topvalue].size() == 0){
							// 需要分成两种情况是因为其中有一种情况没有order就不进行分配orderlist了
							while(left_orderlist.size() != 0){
								orderitem neworder = orderlist[left_orderlist.back()];
								node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
								local_solution[topvalue].push_back(deliver_node);
								PDlist_next(local_solution[topvalue],"fuck99");
								left_orderlist.pop_back();
							}
						}else{
							while (left_orderlist.size() != 0) {
								last_node = current_car_location[topvalue];
								if(currentroute_nodelist[topvalue].size() - 1<deliver_order_match[topvalue][left_orderlist.back()]){
									cout<<"fuck that is it"<<endl;
									cout<<endl;
								}
								local_solution[topvalue].push_back(currentroute_nodelist[topvalue][deliver_order_match[topvalue][left_orderlist.back()]]);
								current_car_location[topvalue] = deliver_order_match[topvalue][left_orderlist.back()];
								// local update
								currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] = (1 - beta_der) * currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]] + beta_der * local_tao[topvalue];
								currentroute_eta_relation[topvalue][current_car_location[topvalue]][last_node] = currentroute_eta_relation[topvalue][last_node][current_car_location[topvalue]];
								PDlist_next(local_solution[topvalue], "fuck4");
								left_orderlist.pop_back();
							}
						}
					}
					Timecaculate(local_solution);
					double tempcost = total_cost(local_solution);
					//cout<<tempcost<<endl;
						if(local_best_cost1>tempcost){
							local_best_cost1 = tempcost;
							local_bestsolution1 = local_solution;
						}

					// global update
					for(string each_car:veichles){
						double delta_local_bestsolution1 = single_cost(local_bestsolution1[each_car]);
						if(unordered_solution[each_car].size() == 0)continue;
						int firstnode = 0,secondnode;
						for(int i = 0;i<local_bestsolution1[each_car].size() - 2;i++){
							if(local_bestsolution1[each_car][i+1].getpickup()){
								secondnode = pick_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}else{
								secondnode = deliver_order_match[each_car][local_bestsolution1[each_car][i + 1].getorderid()];
							}
							currentroute_eta_relation[each_car][firstnode][secondnode] = (1 - alpha2) * currentroute_eta_relation[each_car][firstnode][secondnode] + alpha2/delta_local_bestsolution1;
							currentroute_eta_relation[each_car][secondnode][firstnode] = currentroute_eta_relation[each_car][firstnode][secondnode];
							firstnode = secondnode;
						}

					}

					if(local_best_cost2>local_best_cost1){
						local_best_cost2 = local_best_cost1;
						local_bestsolution2 = local_bestsolution1;
					}
					//cout<<"fuck"<<endl;
				}
				if(local_best_cost3>local_best_cost2){
					local_bestsolution3 = local_bestsolution2;
					local_best_cost3 = local_best_cost2;
					local_bestproblem1 = unordered_solution;
				}

			}
				// 以上是使用蚁群算法，进行优先顺序进行构建同时构建过程当中计算各节点的时间
			double delta_currentorder = 1/local_best_cost3;
			for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_bestproblem1[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - alpha2)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + alpha2* delta_currentorder;
				}
			}
			if(global_best_cost>local_best_cost3){
				global_best_cost = local_best_cost3;
				best_currentsolution = local_bestsolution3;
			}
		}
		//printsolution(best_currentsolution);
		mergesolution(solution, best_currentsolution);
		startsolution = best_currentsolution;
		current_time += timeinterval;
	}
	cost_data.push_back(total_cost(solution));
	//printsolution(solution);
}
cout<<Total_node_count(solution) - veichle_num - 2*order.size()<<" ";
cout<<"Mean: "<<calculateMean(cost_data)<<"   Variance: "<<calculateVariance(cost_data,calculateMean(cost_data))<<"  Best: "<<findMin(cost_data)<<endl;
// }
}
#endif

vector<node> global_node_construct(vector<orderitem> unordered_solution, unordered_map<string, int>& order_dnode,unordered_map<string,int>&order_pnode, vector<int>&existpickup,unordered_map<string,vector<node>> startsolution) {
	vector<node> afterlist;
	int iteration = 0;
	for(string each_car:veichles){
		afterlist.push_back(startsolution[each_car].front());
	}
	iteration =4;
	for (orderitem neworder:unordered_solution){
		iteration += 2;
		node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(), neworder.getDemand()), 
		deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
		// cout<<pickup_node.getpakage()<<" "<<deliver_node.getpakage()<<endl;
		order_dnode[neworder.getOrderId()] = iteration;
		order_pnode[neworder.getOrderId()] = iteration - 1;
		afterlist.push_back(pickup_node);
		afterlist.push_back(deliver_node);
		existpickup.push_back(iteration - 1);
	}
	for(string each_car:veichles){
		for(string neworder_name:startsolution[each_car].back().getdeliverlist()) {
			orderitem  neworder = orderlist[neworder_name];
			iteration += 1;
			node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(), neworder.getDemand());
			order_dnode[neworder.getOrderId()] = iteration;
			afterlist.push_back(deliver_node);
		}
	}

	return afterlist;
}

static int whole_iteration = 20;
static int constuct_ant_num = 5;

#if 0
int main() {
	readtime(); readistance(); readfactory();
	int instance = 30;
	std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	for(int example = 0;example<=10;example++){
	//solution.clear();
	current_time = 600;
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 10000000000;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
		unordered_map<string,int> order_dnode,order_pnode;vector<int> existpickup;

		// 用序号进行遍历得到每一辆车的起始对应node在信息素矩阵当中的位置和在veichle当中进行遍历的位置
		vector<node> global_nodelist = global_node_construct(currentorderlist,order_dnode,order_pnode,existpickup,startsolution);
		
		for(int iter1 = 0;iter1<whole_iteration;iter1++){

			unordered_map<string,vector<node>> local_bestsolution1;
			double local_bestsolution1_cost = 10000000000;
			// 一个大的路径的构建
			for(int iter2 = 0;iter2<constuct_ant_num;iter2++){
				unordered_map<string,vector<node>> local_solution = startsolution;



				Timecaculate(local_solution);
				double tempcost = total_cost(local_solution);
				// local update



				if(tempcost<local_bestsolution1_cost){
					tempcost = local_bestsolution1_cost;
					local_bestsolution1 = local_solution;
				}
			}




		}
		


		mergesolution(solution, best_currentsolution);
		startsolution = best_currentsolution;
		current_time += timeinterval;
	}
	cout << total_cost(solution)<<" "<<Total_node_count(solution);
	// printsolution(solution);
}
}
#endif

#if 0
int main() {
	readtime(); readistance(); readfactory();
	int instance = 40;
	std::string instance_veichle = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
	std::string instance_order = "D:\\sci_pap\\基于多目标动态取送货优化\\cpp实现\\MOEADTS\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
	readveichle(instance_veichle); readorder(instance_order);
	initialsolution();
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>currentsolution;
	currentsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
		marked_order.clear(); unmoved_order.clear();
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		currentorderlist = getnewproblem(currentsolution,unfinishorderlist,currentorder_location);
		cout << currentorderlist.size() << endl;
		vector<vector<double>> order_relation(currentorderlist.size(), vector<double>(currentorderlist.size(),0));
		// 该顺序和orderlist以及viechles的内部顺序有关系
		vector<vector<double>>veichle_order_relation = initialveichleorder_relation(currentsolution, currentorderlist);
		initialorder_relation(order_relation, currentorderlist);
		for (int i = 0; i < veichle_num; i++) {
			for (int j = 0; j < currentorderlist.size(); j++) {
				cout << veichle_order_relation[i][j] << " ";
			}
			cout << endl;
		}
		vector<vector<double>>eta_veichleorder_relation = initial_eta_veichleorder_relation(currentsolution, currentorderlist, veichle_order_relation, order_relation);
		if (currentorderlist.size() == 0)continue;
		double order_tao = eta_veichleorder_relation[0][0] / double(veichle_num);
		unordered_map<string, vector<node>> best_currentsolution = currentsolution;
		double best_cost = 100000000;
		for (int iter1 = 0; iter1 < solution_construct_interation; iter1++) {
			// 完成每一个车辆的order分配 解的回归
			unordered_map<string, vector<orderitem>> local_currentproblem; double best_currentproblem_cost = 0;
			for (int ant_order = 0; ant_order < antnum; ant_order++) {
				unordered_map<string, vector<orderitem>> unordered_problem;
				unordered_map<string, vector<node>> unordered_solution = currentsolution;
				unordered_problem = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
				for (int i = 0; i < veichle_num; i++) {
					for (orderitem temp_order : unordered_problem[veichles[i]]) {
						eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - beta_der) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + beta_der * order_tao;
					}
				}
				for (string each : veichles) {
					vector<int> rpermutation = random_permutation(unordered_problem[each].size());
					for (int direct : rpermutation) {
						node pickup_node(unordered_problem[each][direct].getPickupFactoryId(), unordered_problem[each][direct].getOrderId(), unordered_problem[each][direct].getCreationTime(), 1, unordered_problem[each][direct].getLoadTime(), unordered_problem[each][direct].getDemand()),
							deliver_node(unordered_problem[each][direct].getDeliveryFactoryId(), unordered_problem[each][direct].getOrderId(), unordered_problem[each][direct].getCommittedCompletionTime(), 0, unordered_problem[each][direct].getUnloadTime(), unordered_problem[each][direct].getDemand());
						single_insert(unordered_solution[each], pickup_node, deliver_node);
					}
				}
				Timecaculate(unordered_solution);
				double tempcost =1/ total_cost(unordered_solution);
				if (tempcost > best_currentproblem_cost) {
					local_currentproblem = unordered_problem;
					best_currentproblem_cost = tempcost;
					best_currentsolution = unordered_solution;
				}
			}
			for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_currentproblem[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - alpha2) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + alpha2 * best_currentproblem_cost;
				}
			}
		}
		mergesolution(solution, best_currentsolution);
		currentsolution = best_currentsolution;
		current_time += timeinterval;
	}
	cout << total_cost(solution) << " " << Total_node_count(solution);
}
#endif

/*node test;
	test.setpropertytime(todoorderlist[0].getCreationTime());
	vector<node> temp;
	insertAfter(temp, 0, test);
	cout<<temp[0].getpropertytime();
	// order 耗尽即为终止
	//while (!todoorderlist.empty()) {
	//}
	/*for (orderitem value : todoorderlist)
		CIinsertion1(value, solution);*/