#include<iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include<chrono>
#include<random>
#include<array>
#include<queue>
#include<execution>
#include<algorithm>
#include<iomanip>
#include<unordered_set>
#include<set>
#include <map>
#include <utility>
#include"lifo_tree.h"
#include"veichle.h"
#include"node.h"
#include"oderitem.h"
#include"factory.h"

static int T = 2; // 邻居数量
static int N = 6;
static int veichle_num = 5;
static int port_dock = 6;	//港口容纳数量
static double capacity = 15;	//汽车容量
//static double capacity = 1000000000;
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
static int instance = 1;
// solution 每一列当中相同的节点即使相邻不进行合并
static vector<string> veichles;
static unordered_map<string,int> order_location;

static double weight[7][2] =
{
	{0.001, 0.999},
	{0.2,0.8},
	{0.4,0.6},
	{0.6,0.4},
	{0.8,0.2},
	{0.999, 0.001},
    {0,1}
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

inline double generateRandomNumber() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0.0, 1.0);
	return dis(gen);
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
		cout<<each<<" "<<tempsolution[each].size()<<endl;
		for(node value:tempsolution[each]){
			cout<<"current order:"<<value.getorderid()<<" "<<value.getpickup()<<" "<<value.getstorage();
			cout << endl;
		}
		cout << endl;
	}
}

void printfactory(unordered_map<string,vector<node>>&tempsolution){
	for(string each:veichles){
		cout<<each<<endl;
		for(node value:tempsolution[each]){
			int type;
			if(value.getpickup()){
				type = 1;
			}else{
				type = -1;
			}
			cout<<value.getId()<<" "<<order_location[value.getorderid()]*type<<endl;
		}
	}
}

void clearCSV(const std::string& filename) {
    std::ofstream file(filename, std::ios::trunc); // 以清空模式打开文件
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }
    // 文件打开后立即截断，无需写入内容
    file.close();
}

void printfactory(unordered_map<string, vector<node>>& tempsolution,string logo) {
    // 打开CSV文件（可根据需求修改文件名）
	clearCSV("D:\\sci_pap\\project_code\\moead_ts\\factory_output.csv");
    ofstream outfile("D:\\sci_pap\\project_code\\moead_ts\\factory_output.csv");
    if (!outfile.is_open()) {
        cerr << "Failed to open output file!" << endl;
        return;
    }
    
    // 写入CSV表头
    //outfile << "Vehicle,ID,ProcessedValue\n";
    
    // 遍历每辆车
    for (const string& vehicle : veichles) {  // 注意确认变量名是否正确（vehicles/veichles）
        // 遍历该车的所有节点
		 outfile << vehicle << "\n";
        for (node n : tempsolution[vehicle]) {
            // 计算数值
            int type = n.getpickup() ? 1 : -1;
            int processed_value = order_location[n.getorderid()] * type;
            
            // 写入CSV行（格式：车辆名,ID,处理后的值）
            outfile << n.getId() << ","
                    << processed_value <<","<<n.getstorage()<< "\n";
        }
    }
    
    outfile.close();
}


void printCurrentTime() {
    // 获取当前时间戳
    std::time_t now = std::time(nullptr);
    
    // 转换为本地时间结构
    std::tm* local_time = std::localtime(&now);
    
    // 使用iomanip进行格式化输出
    std::cout << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << std::endl;
}

// 不考虑时间
void Timecaculate(std::unordered_map<std::string, std::vector<node>>& nonsolution) {
	return;
}

int Total_node_count(unordered_map<string, vector<node>> tempsolution) {
	int sum = 0;
	for (string each : veichles) {
		sum +=tempsolution[each].size();
	}
	return sum;
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

int findMinIndex(vector<double> vec) {
	if (vec.empty()) {
		return -1;  // 处理向量为空的情况
	}
	int maxIndex = 0;  // 假设第一个元素是最小的，将其索引初始化为 0
	for (size_t i = 1; i < vec.size(); ++i) {  // 从第二个元素开始遍历
		if (vec[i] < vec[maxIndex]) {  // 如果当前元素比假设的最大元素大
			maxIndex = i;  // 更新最大元素的索引
		}
	}
	return maxIndex;  // 返回最大元素的索引
}


int weighted_random(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // 生成[min, max]范围内的所有整数
    std::vector<int> numbers;
    for (int i = min; i <= max; ++i) {
        numbers.push_back(i);
    }
    
    // 计算对数权重（底数6，偏移量100）
    std::vector<double> weights;
    const double base = 6.0;
    const int offset = 100;  // 偏移量设为100
    
    for (int num : numbers) {
        // 公式：weight = log6(num + 100)
        double weighted_num = num + offset;
        double weight = std::log(weighted_num) / std::log(base);
        weights.push_back(weight);
    }
    
    // 构建离散概率分布
    std::discrete_distribution<> dist(weights.begin(), weights.end());
    
    // 返回随机选择的数
    return numbers[dist(gen)];
}

void PDlist(vector<node>& tempnode) {
	vector<string> temp; double pack = capacity;
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
		pack = capacity;
		for (string str : tempnode[i].getdeliverlist()) {
			pack -= orderlist[str].getDemand();
		}
		tempnode[i].setstorage(pack);
	}
}

inline void total_pack(unordered_map<string, vector<node>>& tempsolution) {
	for (string value:veichles) {
		PDlist(tempsolution[value]);
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

double f1_timecosuming(unordered_map<string,vector<node>>tempsolution) { 
	return 0;
}

double distancecosuming(vector<node> &nodelist) {
	double sum = 0;
	for (int i = 1; i < nodelist.size(); i++) {
		sum += Distance[location[nodelist[i - 1].id]][location[nodelist[i].id]];
	}
	return sum;
}

double f2_totaldistance(unordered_map<string, vector<node>>& tempsolution) {
	vector<node>routelist; double sum = 0;
	for (auto it = tempsolution.begin(); it != tempsolution.end(); it++) {
		routelist = it->second;
		sum  = sum + distancecosuming(routelist);
	}
	return sum/double(veichle_num);
}

inline double finalcost(unordered_map<string, vector<node>>& tempsolution,double*weight_f1_f2) {
	return f1_timecosuming(tempsolution) * weight_f1_f2[0] + f2_totaldistance(tempsolution) * weight_f1_f2[1];
}

inline double Tchebyceff(unordered_map<string,vector<node>>& tempsolution, double* weight_f1_f2) {
	double a1 = abs(f1_timecosuming(tempsolution) - reference_point[0]) * weight_f1_f2[0] , a2 = abs(f2_totaldistance(tempsolution) - reference_point[1]) * weight_f1_f2[1];
	return max(a1,a2);
}

unordered_map<string, vector<node>> Tabu_search(unordered_map<string, vector<node>>initialsolution);
unordered_map<string, vector<node>> Tabu_search(unordered_map<string, vector<node>>initialsolution,string loge);

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

void readveichle(string filename,string logo) {
	veichles.clear();
	veichles.push_back("V_1");
	veichle_num = veichles.size();
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
					order_location[order.back().getOrderId()] = order.size();
					orderlist[temporder.getOrderId()] = temporder;
				}
				//cout << "marked" << endl;
			}
			else {
				order.push_back(temp);
				order_location[order.back().getOrderId()] = order.size();
				orderlist[temp.getOrderId()] = temp;
			}
		}

	}
}

void readorder(string filename,string logo) {
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
			temp.setCommittedCompletionTime(temp.getCommittedCompletionTime() - temp.getCreationTime());
			temp.setCreationTime(0);
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
					order_location[order.back().getOrderId()] = order.size();
					orderlist[temporder.getOrderId()] = temporder;
				}
				//cout << "marked" << endl;
			}
			else {
				order.push_back(temp);
				order_location[order.back().getOrderId()] = order.size();
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
	for (string each:veichles) {
		if(solution[each].size() != 0)solution[each].clear();
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
		temp.setorderid(each);
		solution[each].push_back(temp);
	}
}

void initialsolution(string logo) {
	node temp;int iter = 0;
	for (string each:veichles) {
		if(solution[each].size() != 0)solution[each].clear();
		int random_integer = iter++; // 生成随机数
		temp.setFactoryId(numlocation[random_integer]);
		temp.setServiceTime(0);  // 初始化第一个节点的servicetime，用于时间产生
		temp.setstorage(capacity);
		temp.setpickup(0);
		temp.setorderid(each);
		temp.setpakage(0);
		solution[each].push_back(temp);
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

inline void mergeroutelist(vector<node>& temp, vector<node>temp1) {
	temp.insert(temp.end(), make_move_iterator(temp1.begin()), make_move_iterator(temp1.end()));
}

void mergesolution(unordered_map<string,vector<node>>& temp,unordered_map<string,vector<node>> added) {
	for (auto it = temp.begin(); it != temp.end(); it++) {
		mergeroutelist(it->second, added[it->first]);
	}
}

template <typename T>
std::vector<T> shuffle_vector(const std::vector<T>& input) {
    // 创建输入向量的副本
    std::vector<T> shuffled = input;
    
    // 使用随机设备初始化随机数生成器
    std::random_device rd;
    std::mt19937 rng(rd());
    
    // 使用 Fisher-Yates 洗牌算法进行随机排列
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    
    return shuffled;
}

// 存在问题，其中没有包含哪些在solution当中但是没有完成的完整订单
vector<orderitem> getneworders(vector<orderitem>& temp) {
	vector<orderitem> afterlist;
	for (auto it = temp.begin(); it != temp.end();) {
		if (it->getCreationTime()>=(current_time - timeinterval)&&it->getCreationTime()<current_time) {
			afterlist.push_back(*it);
			it = temp.erase(it);
		}
		else {
			break;
		}
	}
	return afterlist;
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

double single_cost_f2(vector<node> nodelist) {
	double cost = 0;
	if (nodelist.size() == 1)return 0;
	for (int i = 1; i < nodelist.size(); i++) {
		cost += Distance[location[nodelist[i - 1].getId()]][location[nodelist[i].getId()]]/double(veichle_num);
	}
	return cost;
}

void insert_order1(unordered_map<string,vector<node>> &dealsolution,orderitem neworder);
double total_cost(unordered_map<string,vector<node>>tempsolution);
bool PDlist(vector<node> &tempnodelist,string temp);
void insert_order_tc(unordered_map<string,vector<node>> &dealsolution,orderitem neworder,double *w){
	unordered_map<string, vector<node>> memorysolution = dealsolution;
	double temp = 100000000000; vector<node> templist1;
	node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
	string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
	vector<node>routelist, templist;
	for (string each_car:veichles) {
		routelist = dealsolution[each_car];
		if (routelist.size() == 1) {
			templist = routelist;
			// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
			insertBefore(templist, 1, deliver_node);
			insertAfter(templist, 0, pickup_node);
			templist1 = dealsolution[each_car];
			dealsolution[each_car] = templist;
			Timecaculate(dealsolution);
			temp = finalcost(dealsolution,w);
			dealsolution[each_car] = templist1;
			dealsolution = memorysolution;
			if (temp < min) {
				minveichle = each_car;
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
					PDlist(templist,"fucj");
					for (int k = 0; k < templist.size(); k++) {
						if (templist[k].getstorage() < 0) {
							getout = 1; break;
						}
					}
					if (getout)continue;
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = finalcost(dealsolution,w);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
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

void insert_order_tyc(unordered_map<string,vector<node>> &dealsolution,orderitem neworder,double *w){
	unordered_map<string, vector<node>> memorysolution = dealsolution;
	double temp = 100000000000; vector<node> templist1;
	node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
	string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
	vector<node>routelist, templist;
	for (string each_car:veichles) {
		routelist = dealsolution[each_car];
		if (routelist.size() == 1) {
			templist = routelist;
			// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
			insertBefore(templist, 1, deliver_node);
			insertAfter(templist, 0, pickup_node);
			templist1 = dealsolution[each_car];
			dealsolution[each_car] = templist;
			Timecaculate(dealsolution);
			temp = finalcost(dealsolution,w);
			dealsolution[each_car] = templist1;
			dealsolution = memorysolution;
			if (temp < min) {
				minveichle = each_car;
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
					PDlist(templist,"fucj");
					for (int k = 0; k < templist.size(); k++) {
						if (templist[k].getstorage() < 0) {
							getout = 1; break;
						}
					}
					if (getout)continue;
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = finalcost(dealsolution,w);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
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

// inline double finalcost(unordered_map<string, vector<node>>& tempsolution,double*weight_f1_f2)
void initial_population(unordered_map<string,vector<node>> &tempsolution,vector<orderitem> temporderlist,double *w){
	temporderlist = shuffle_vector(temporderlist);
	for(orderitem each_order:temporderlist){
		insert_order_tc(tempsolution,each_order,w);
	}
}

// 切比雪夫 double Tchebyceff(unordered_map<string,vector<node>>& tempsolution, double* weight_f1_f2)
void completeorderwithCI(vector<orderitem> leftorderlist,unordered_map<string,vector<node>>& lastsolution,double*w){
	leftorderlist = shuffle_vector(leftorderlist);
	for(orderitem each_order:leftorderlist){
		insert_order_tyc(lastsolution,each_order,w);
	}
}

vector<vector<bool>> check_feasible( vector<node>& route, int begin_pos) {
    int n = route.size();
    vector<vector<bool>> feasible(n, vector<bool>(n, false));
    for (int i = begin_pos; i < n-1; ++i) {
        vector<node> stack;
        for (int j = i+1; j < n; ++j) {
            if (!route[j].getpickup()) {
                if (!stack.empty() && 
                    stack.back().getorderid() == route[j].getorderid()) {
                    stack.pop_back();
                } else {
                    break;  // 存在不匹配的送货节点
                }
            } else {
                stack.push_back(route[j]);
            }
            feasible[i][j] = stack.empty();
        }
    }
    return feasible;
}

// 块反转核心逻辑
void reverse_route_segment(vector<node>& route, int begin, int end) {
    // 简化的块反转，实际需要按PD对处理
    reverse(route.begin() + begin, route.begin() + end + 1); 
}

// 2-opt优化主函数
unordered_map<string, vector<node>> local_2opt(const unordered_map<string, vector<node>>& input) {
    unordered_map<string, vector<node>> solution = input;
    
    for (auto& [vid, route] : solution) {
        auto feasible = check_feasible(route, 0);
        bool improved = true;
        
        while (improved) {
            improved = false;
            double best_cost = single_cost_f2(route); // 需要实现成本计算
            
            for (int i = 0; i < route.size()-2; ++i) {
                for (int j = i+2; j < route.size(); ++j) {
                    if (!feasible[i][j]) continue;
                    
                    auto new_route = route;
                    reverse_route_segment(new_route, i, j);
                    double new_cost = single_cost_f2(new_route);
                    
                    if (new_cost < best_cost) {
                        route = new_route;
                        best_cost = new_cost;
                        improved = true;
                        break;
                    }
                }
                if (improved) break;
            }
        }
    }
    return solution;
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

inline string transform_solution_to_string(unordered_map<string,vector<node>>initialsolution) {
	string temp = "fuck";
	for(string each_car:veichles){
		for(node value:initialsolution[each_car]){
			temp += value.getorderid();
		}
	}
	return temp;
}

bool noTabu(string temp, queue<string> &tabuliat) {
	while (tabuliat.size() > tabulist_size)
		tabuliat.pop();
	queue<string> templist = tabuliat;

	while(templist.size() != 0){
		if(templist.front() == temp)return 0;
		templist.pop();
	}
	tabuliat.push(temp);
	return 1;
}

void getnewproblem(unordered_map<string,vector<node>>&tempsolution,unordered_map<string,vector<node>>&startsolution){
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
		startsolution[each].erase(startsolution[each].begin(),startsolution[each].end() - to_the_end);
		tempsolution[each].resize(tempsolution[each].size() - to_the_end);
	}
}

// 更新reference_point
void update_reference(vector<unordered_map<string,vector<node>>> population) {
	double minf1 = reference_point[0], minf2 = reference_point[1];
	for (int i = 0; i < N; i++) {
		minf1 = min(f1_timecosuming(population[i]), minf1);
		minf2 = min(minf2,f2_totaldistance(population[i]));
	}
	reference_point[0] = minf1;
	reference_point[1] = minf2;
}

void update_reference(unordered_map<string,vector<node>> tempsolution){
	reference_point[0] = min(f1_timecosuming(tempsolution),reference_point[0]);
	reference_point[1] = min(f2_totaldistance(tempsolution),reference_point[1]);
}

unordered_map<string,vector<node>> select_population(vector<unordered_map<string,vector<node>>> population){
	vector<double> cost;
	for(int i = 0;i<population.size();i++){
		cost.push_back(total_cost(population[i]));
	}
	return population[findMinIndex(cost)];
}

pair<int,int> mating_selection(vector<unordered_map<string,vector<node>>> population,bool type,int id){
	double delta_ = 0.9;
	vector<int> perm;
	if(type){
		perm.push_back(id);
		if(id == 0){
			perm.push_back(id + 1);
			perm.push_back(id + 2);
		}
		else if(id == N - 1){
			perm.push_back(id - 1);
			perm.push_back(id - 2);
		}
		else{
			perm.push_back(id - 1);
			perm.push_back(id + 1);
		}
	}else{
		for(int i = 0;i<N;i++){
			perm.push_back(i);
		}
	}
	perm = shuffle_vector(perm);
	pair<int,int> mating;
	mating.first = perm[0];
	mating.second = perm[1];
	return mating;
}

// 默认领域当中的解的数量都为2时的情况
void update_problem(unordered_map<string,vector<node>> child,vector<unordered_map<string,vector<node>>>& population,int id,bool type){
	int nr = 2;
	int delta = 0.9,time = 0;
	vector<int> perm;
	if(type){
		if(id == 0){
			perm.push_back(id + 1);
			perm.push_back(id + 2);
		}
		else if(id == N){
			perm.push_back(id - 1);
			perm.push_back(id - 2);
		}
		else{
			perm.push_back(id - 1);
			perm.push_back(id + 1);
		}
	}else{
		for(int i = 0;i<N;i++){
			perm.push_back(i);
		}
	}
	perm = shuffle_vector(perm);
	for(int i = 0;i<perm.size();i++){
		int k;
		if(type == 1){
			k = perm[i];
		}else{
			k = perm[i];
		}
		if(Tchebyceff(child,weight[k])<Tchebyceff(population[k],weight[k])){
			population[k] = child;
			time++;
		}
		if(time >= nr){
			return;
		}
	}
}

#if 0
// 暂时没有考虑隔天的订单
int main() {
	int whole_iteration = 50;
	int delta = 0.9;
	readtime(); readistance(); readfactory(); 
	for (instance = 1; instance <= 34; instance++) {
		std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
        std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
        orderlist.clear();order.clear();veichles.clear();solution.clear();
	    readveichle(instance_veichle); 
	    readveichle("fick","dick");
	    readorder(instance_order,"fuck");
		cout<<"begin"<<endl;
		vector<double> cost_data;
		for (int loop = 0; loop < 20; loop++) {
			current_time = 600;
			initialsolution("fuck");
			vector<orderitem> todoorderlist;
			unordered_map<string, vector<node>>currentproblem = solution;
			for (int i = 0; i < order.size(); i++)
				todoorderlist.push_back(order[i]);
			initial_population(solution,todoorderlist,weight[6]);
            cout<<f2_totaldistance(solution)<<" "<<Total_node_count(solution)<<endl;
			auto startTime = std::chrono::high_resolution_clock::now();
			const auto tenMinutes = std::chrono::seconds(600);
            for(int iter = 0;iter<whole_iteration;iter++){
                cout<<iter<<",";
                solution = Tabu_search(solution,"fuck");
                cout<<f2_totaldistance(solution)<<" "<<Total_node_count(solution)<<endl;
				auto currentTime = std::chrono::high_resolution_clock::now();
				// 计算从开始到现在经过的时间
				auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
		
				// 判断是否超过十分钟
				if (elapsedTime >= tenMinutes) {
					std::cout << "循环已运行超过十分钟，停止循环。" << std::endl;
					break;
				}
			}
        double temp_total_cost = total_cost(solution);

		cout<<f2_totaldistance(solution)<<" "<<temp_total_cost<<" "<<Total_node_count(solution)<<endl;
	//	printsolution(solution);
		}
		
	}
	return 0;
}
#endif

// ant colony system ............................................................................ant colony system //

// 遍历veichles从而对solution进行遍历
double alpha2 = 0.1;
double gama = 0.7; // 信息素采用较大的更新率
double same = 16;
double beta_der = 0.1;
double rho1 = 0.7; //分配订单时，启发式和信息素二者的占比
double rho2 = 0.7; //单个订单路径规划时，启发式和信息素二者的占比
double q1 = 0.7; //订单分配时，基于探索还是基于概率最大的一个,两种算法公用参数
double q2 = 0.85; //路径规划时，基于探索还是基于概率最大的一个
double S = 0.85;
int antnum1 = 1;
int antnum2 = 4;
int route_construct_interation = 4;
int solution_construct_interation = 15;

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

// 按照搜索路径的方法，将所有的结点从solution当中去除，只留下初始结点，然后再从初始结点之后进行逐个插入
vector<orderitem> getnewproblem(vector<orderitem>& todoorderlist, unordered_map<string,vector<node>>& tempsolution,unordered_map<string,vector<node>>&startsolution,
	unordered_map<string, int>& order_location,string logo) {
	vector<orderitem> afterlist; int iteration = 0;current_time = 0;
	for (auto it = todoorderlist.begin(); it != todoorderlist.end();) {
	//	if (it->getCreationTime() >= (current_time - timeinterval) && it->getCreationTime() < current_time) {
		if (it->getCreationTime() >= 0 && it->getCreationTime() <= 24*60*60) {
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
		order_set[veichles[wheel_select(generateRandomNumber(), temp_problity,"fuck1")]].push_back(cuorders[i]);
	}
	return order_set;
}

// 仅仅只是在初始化eta矩阵的时候才会使用，仅仅是按照启发式进行分配的订单
unordered_map<string, vector<orderitem>> allocate_order(vector<vector<double>> veichle_order_relation,vector<orderitem>cuorders) {
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
		order_set[veichles[wheel_select(generateRandomNumber(), temp_problity,"fuck1")]].push_back(cuorders[i]);
	}
	return order_set;
}

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

void printroute(unordered_map<string,vector<node>> routesolution){
	for(string value:veichles){
		cout<<value<<" ";
		for(node tempvalue:routesolution[value]){
			cout<<location[tempvalue.getId()]<<" "<<tempvalue.getpickup()<<" ";
		}
		cout<<endl;
	}
}

// 启发式矩阵的初始化,启发式信息仅仅采用了距离作为启发式的初始信息
vector<vector<double>> local_node_matrix(vector<node> templist) {
	vector<vector<double>> tempmatrix(templist.size(), vector<double>(templist.size(), 0));
	for (int i = 0; i < templist.size(); i++) {
		for (int j = i + 1; j < templist.size(); j++) {
			if (Distance[location[templist[i].getId()]][location[templist[j].getId()]] == 0) {
				tempmatrix[i][j] = same;
			}else tempmatrix[i][j] = 1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]];
			tempmatrix[j][i] = tempmatrix[i][j];
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
    double cost = f2_totaldistance(tempsolution);
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
            // 改成小于和论文相同
			if (generateRandomNumber() <= q1) {
				order_set[veichles[location]].push_back(cuorders[i]);
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
        variance /= numbers.size();
        return std::sqrt(variance);
    }
    
        void writeNumbersToCSV(std::vector<double>& numbers,string filename,int iter) {
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
            file << " " << mean << "," << variance <<","<<iter<< "\n";
        
            numbers.clear();
            file.close();
            // std::cout << "数据已成功写入 " << filename << std::endl;
        }

void writecost(std::vector<double>& numbers,string filename) {
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
    file<<'\n';
    numbers.clear();
    file.close();
    // std::cout << "数据已成功写入 " << filename << std::endl;
}

vector<node> global_node_construct(vector<orderitem> unordered_solution, unordered_map<string, int>& order_dnode,unordered_map<string,int>&order_pnode, vector<int>&existpickup,unordered_map<string,vector<node>> startsolution) {
	vector<node> afterlist;
	int iteration = 0;
	for(string each_car:veichles){
		afterlist.push_back(startsolution[each_car].front());
	}
	iteration = veichle_num - 1;
	for(int i = 0;i<veichle_num;i++){
		order_pnode[veichles[i]] = i;
		order_dnode[veichles[i]] = i;
	}
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

vector<vector<double>> global_node_matrix(vector<node> templist){
    vector<vector<double>> tempmatrix(templist.size(), vector<double>(templist.size(), 0));
	for (int i = 0; i < templist.size(); i++) {
		for (int j = i + 1; j < templist.size(); j++) {
			if (Distance[location[templist[i].getId()]][location[templist[j].getId()]] == 0) {
				tempmatrix[i][j] = same;
			}else tempmatrix[i][j] = 1 / Distance[location[templist[i].getId()]][location[templist[j].getId()]];
			tempmatrix[j][i] = tempmatrix[i][j];
		}
		tempmatrix[i][i] = 0.0;
	}
	return tempmatrix;
}

vector<vector<double>> global_node_distance(vector<node> templist){
    vector<vector<double>> tempmatrix(templist.size(), vector<double>(templist.size(), 0));
	for (int i = 0; i < templist.size(); i++) {
		for (int j = i + 1; j < templist.size(); j++) {
			if (Distance[location[templist[i].getId()]][location[templist[j].getId()]] == 0) {
				tempmatrix[i][j] = 0;
			}else tempmatrix[i][j] = Distance[location[templist[i].getId()]][location[templist[j].getId()]];
			tempmatrix[j][i] = tempmatrix[i][j];
		}
		tempmatrix[i][i] = 0.0;
	}
	return tempmatrix;
}

vector<vector<double>> global_node_eta_matrix(unordered_map<string,vector<node>> startsolution,unordered_map<string,int> order_dnode,unordered_map<string,int> order_pnode,
    vector<vector<double>> veichle_order,vector<orderitem> cuorders,vector<node> currentroute_nodelist,vector<vector<double>> currentroute_node_relation){
    unordered_map<string,vector<node>> local_solution = startsolution;
    unordered_map<string,vector<orderitem>> unordered_solution = allocate_order(veichle_order,cuorders);
    unordered_map<string,vector<int>> currentroute_exist;
    for(string each_car:veichles){
        currentroute_exist[each_car];
        for(orderitem value:unordered_solution[each_car]){
            currentroute_exist[each_car].push_back(order_pnode[value.getOrderId()]);
        }
    }
    for(int each_car = 0;each_car<veichle_num;each_car++){
        int current_car_location = each_car,last_node;
        string topvalue = veichles[each_car];
        while(currentroute_exist[topvalue].size()!=0){
            vector<int>select_node;vector<double>probility_of_nodeselect;double sum = 0;
			if(local_solution[topvalue].back().getdeliverlist().size()!=0){
                if(order_dnode.count(local_solution[topvalue].back().getdeliverlist().back()) == 0){
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
                select_node.push_back(order_dnode[local_solution[topvalue].back().getdeliverlist().back()]);
            }
            for(int i:currentroute_exist[topvalue]){
                if (i == 0)cout << "what fuck bro it involvo 0" << endl;
                if(currentroute_nodelist[i].getpakage() > local_solution[topvalue].back().getstorage()){
                    continue;
                }else{
                    select_node.push_back(i);
                }
            }
            for(int i:select_node){
                sum += currentroute_node_relation[current_car_location][i];
                probility_of_nodeselect.push_back(currentroute_node_relation[current_car_location][i]);
            }
            for (int i = 0; i < probility_of_nodeselect.size(); i++) {
                probility_of_nodeselect[i] /= sum;
            }
            last_node = current_car_location;
            if (generateRandomNumber() <= q2) {
                current_car_location = select_node[findMaxIndex(probility_of_nodeselect)];
            }
            else current_car_location = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
                            
            if (currentroute_nodelist[current_car_location].getpickup()) {
                int templocation = current_car_location;
                currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
            }
            local_solution[topvalue].push_back(currentroute_nodelist[current_car_location]);
            PDlist_next(local_solution[topvalue], "fuck333");
            
        }
        vector<string> left_orderlist = local_solution[topvalue].back().getdeliverlist();
            // 需要分成两种情况是因为其中有一种情况没有order就不进行分配orderlist了
            while (left_orderlist.size() != 0) {
            last_node = current_car_location;
            if(currentroute_nodelist.size() - 1<order_dnode[left_orderlist.back()]){
                cout<<"fuck that is it"<<endl;
                cout<<endl;
            }
            local_solution[topvalue].push_back(currentroute_nodelist[order_dnode[left_orderlist.back()]]);
            current_car_location = order_dnode[left_orderlist.back()];
            // local update
            PDlist_next(local_solution[topvalue], "fuck444");
            left_orderlist.pop_back();
            }

    }
    Timecaculate(local_solution);
    double order_tao = 1.0/(total_cost(local_solution)*double(currentroute_nodelist.size()));
    vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
    return current_car_eta_matrix;
}

void writesolution(string filename,unordered_map<string,vector<node>> tempsolution){
    std::ofstream outFile(filename, std::ios::app);

    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        // 若文件打开失败，输出错误信息
        std::cerr << "无法打开文件: " << filename << std::endl;
    }
    outFile<<total_cost(tempsolution)<<'\n';
    // 遍历字符串向量
    for (string value:veichles) {
        // 将每个字符串写入文件，并添加换行符
        outFile << value <<',';
        for(node topnode:tempsolution[value]){
            if(topnode.getpickup()){
                string str = topnode.getorderid();
                outFile << str <<',';
            }
        }
        outFile<<'\n';
    }

    // 关闭文件
    outFile.close();
}


bool PDlist(vector<node> &tempnodelist,string temp){
	for(int i = 1;i<tempnodelist.size();i++){
		vector<string> left_order = tempnodelist[i - 1].getdeliverlist();
		if(tempnodelist[i].getpickup()){
			left_order.push_back(tempnodelist[i].getorderid());
			tempnodelist[i].setstorage(tempnodelist[i -1].getstorage() - tempnodelist[i].getpakage());
			if(tempnodelist[i].getstorage() < 0)return false;
			tempnodelist[i].setdeliverlist(left_order);
		}else{
			left_order.pop_back();
			tempnodelist[i].setstorage(tempnodelist[i - 1].getstorage() + tempnodelist[i].getpakage());
			if(tempnodelist[i].getstorage() < 0)return false;
			tempnodelist[i].setdeliverlist(left_order);
		}
	}
	return true;
}

unordered_map<string,vector<orderitem>> solution_statics(unordered_map<string,vector<node>> tempsolution){
	unordered_map<string,vector<orderitem>> solution_order;
	for(string each_car:veichles){
		solution_order[each_car];
		for(int i = 0;i<tempsolution[each_car].size();i++){
			if(tempsolution[each_car][i].getpickup()){
				solution_order[each_car].push_back(orderlist[tempsolution[each_car][i].getorderid()]);
			}
		}
	}
	return solution_order;
}

// 针对订单的couple匹配的时候进行考虑的问题，进行local search时需要进行考虑的
void getpdglist(unordered_map<string,vector<pair<int,int>>> &pdlinkmap,unordered_map<string,vector<node>> tempsolution){
	for(string each_car:veichles){
		unordered_map<string,int> order_pnode,order_dnode;
		pdlinkmap[each_car];
		for(int i = 1;i<tempsolution[each_car].size();i++){
			if(tempsolution[each_car][i].getpickup()){
				order_pnode[tempsolution[each_car][i].getorderid()] = i;
			}else{
				if(order_pnode.count(tempsolution[each_car][i].getorderid()) == 1){
					order_dnode[tempsolution[each_car][i].getorderid()] = i;
					pair temp = make_pair(order_pnode[tempsolution[each_car][i].getorderid()],order_dnode[tempsolution[each_car][i].getorderid()]);
					pdlinkmap[each_car].push_back(temp);
				}
			}
		}
	}
}

// 进行couple交换找到其中有提升的解,并且返回的是一个完整的order的全部分布
unordered_map<string,vector<orderitem>> couple_exchange(unordered_map<string,vector<node>> &tempsolution){
	unordered_map<string,unordered_map<string,int>> order_pnode,order_dnode;
	unordered_map<string,vector<string>> car_orderlist;
	unordered_map<string,vector<orderitem>> left_orderlist;
	unordered_map<string,vector<node>> minsolution = tempsolution;
	double mincost = 100000000000;
	for(string each_car:veichles){
		car_orderlist[each_car];
		for(int i = 1;i<tempsolution[each_car].size();i++){
			if(tempsolution[each_car][i].getpickup()){
				car_orderlist[each_car].push_back(tempsolution[each_car][i].getorderid());
				order_pnode[each_car][tempsolution[each_car][i].getorderid()] = i;
			}else{
				if(order_pnode[each_car].count(tempsolution[each_car][i].getorderid()) == 1){
					order_dnode[each_car][tempsolution[each_car][i].getorderid()] = i;
				}
			}
		}
	}
	for(int i = 0;i<veichle_num;i++){
		if(car_orderlist[veichles[i]].size() == 0)continue;
		unordered_map<string,vector<node>> localsolution = tempsolution;
		for(int j = i + 1;j<veichle_num;j++){
			if(car_orderlist[veichles[j]].size() == 0)continue;
			for(int iter1 = 0;iter1<car_orderlist[veichles[i]].size();iter1++){
				localsolution = tempsolution;
				for(int iter2 = 0;iter2<car_orderlist[veichles[j]].size();iter2++){
					localsolution = tempsolution;
					localsolution[veichles[i]][order_pnode[veichles[i]][car_orderlist[veichles[i]][iter1]]] = tempsolution[veichles[j]][order_pnode[veichles[j]][car_orderlist[veichles[j]][iter2]]];
					localsolution[veichles[i]][order_dnode[veichles[i]][car_orderlist[veichles[i]][iter1]]] = tempsolution[veichles[j]][order_dnode[veichles[j]][car_orderlist[veichles[j]][iter2]]];
					localsolution[veichles[j]][order_pnode[veichles[j]][car_orderlist[veichles[j]][iter2]]] = tempsolution[veichles[i]][order_pnode[veichles[i]][car_orderlist[veichles[i]][iter1]]];
					localsolution[veichles[j]][order_dnode[veichles[j]][car_orderlist[veichles[j]][iter2]]] = tempsolution[veichles[i]][order_dnode[veichles[i]][car_orderlist[veichles[i]][iter1]]];
					if(PDlist(localsolution[veichles[i]],"fuck")&&PDlist(localsolution[veichles[j]],"fuck")){
						Timecaculate(localsolution);
						double tempcost = total_cost(localsolution);
						unordered_map<string,vector<orderitem>> temp_orderlist;
						temp_orderlist[veichles[j]].push_back(orderlist[car_orderlist[veichles[i]][iter1]]);
						temp_orderlist[veichles[i]].push_back(orderlist[car_orderlist[veichles[j]][iter2]]);
						if(tempcost<mincost){
							left_orderlist = temp_orderlist;
							mincost = tempcost;
							minsolution = localsolution;
						}
					}
				}

			}
		}
	}
	tempsolution = minsolution;
	left_orderlist = solution_statics(minsolution);
	return left_orderlist;

}

vector<double> global_time_matrix(vector<node> tempnodelist){
	double time_same = 2;
	vector<double> matrix(tempnodelist.size(),0);
	for(int i = veichle_num;i<tempnodelist.size();i++){
		if(orderlist[tempnodelist[i].getorderid()].getCommittedCompletionTime() - current_time > 0)
		matrix[i] = 1/double(orderlist[tempnodelist[i].getorderid()].getCommittedCompletionTime() - current_time);
		else{
			if(orderlist[tempnodelist[i].getorderid()].getCommittedCompletionTime() - current_time == 0){
				matrix[i] = time_same;
			}else{
				matrix[i] = abs(orderlist[tempnodelist[i].getorderid()].getCommittedCompletionTime() - current_time);
			}
		}
	}
	return matrix;
}

unordered_map<string,vector<orderitem>> reloadto_problem(unordered_map<string,vector<node>> tempsolution){
	unordered_map<string,vector<orderitem>> temp_problem;
	for(string each_car:veichles){
		temp_problem[each_car];
		for(int i = 1;i<tempsolution[each_car].size();i++){
			if(tempsolution[each_car][i].getpickup()){
				temp_problem[each_car].push_back(orderlist[tempsolution[each_car][i].getorderid()]);
			}
		}
	}
	return temp_problem;
}

void insert_order1(unordered_map<string,vector<node>> &dealsolution,orderitem neworder){
		unordered_map<string, vector<node>> memorysolution = dealsolution;
		double temp = 100000000000; vector<node> templist1;
		node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
		string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
		vector<node>routelist, templist;
		for (string each_car:veichles) {
			routelist = dealsolution[each_car];
			if (routelist.size() == 1) {
				templist = routelist;
				// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
				insertBefore(templist, 1, deliver_node);
				insertAfter(templist, 0, pickup_node);
				templist1 = dealsolution[each_car];
				dealsolution[each_car] = templist;
				Timecaculate(dealsolution);
				temp = total_cost(dealsolution);
				dealsolution[each_car] = templist1;
				dealsolution = memorysolution;
				if (temp < min) {
					minveichle = each_car;
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
						PDlist(templist,"fucj");
						for (int k = 0; k < templist.size(); k++) {
							if (templist[k].getstorage() < 0) {
								getout = 1; break;
							}
						}
						if (getout)continue;
						templist1 = dealsolution[each_car];
						dealsolution[each_car] = templist;
						Timecaculate(dealsolution);
						temp = total_cost(dealsolution);
						dealsolution[each_car] = templist1;
						dealsolution = memorysolution;
						if (temp < min) {
							minveichle = each_car;
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

void insert_order1(unordered_map<string,vector<node>> &dealsolution,orderitem neworder,string break_logo){
	unordered_map<string, vector<node>> memorysolution = dealsolution;
	double temp = 100000000000; vector<node> templist1;
	node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
	string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
	vector<node>routelist, templist;
	for (string each_car:veichles) {
		routelist = dealsolution[each_car];
		if (routelist.size() == 1) {
			templist = routelist;
			// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
			insertBefore(templist, 1, deliver_node);
			insertAfter(templist, 0, pickup_node);
			templist1 = dealsolution[each_car];
			dealsolution[each_car] = templist;
			Timecaculate(dealsolution);
			temp = total_cost(dealsolution);
			dealsolution[each_car] = templist1;
			dealsolution = memorysolution;
			if (temp < min) {
				minveichle = each_car;
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
					PDlist(templist,"fucj");
					for (int k = 0; k < templist.size(); k++) {
						if (templist[k].getstorage() < 0) {
							getout = 1; break;
						}
					}
					if (getout)continue;
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = total_cost(dealsolution);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
						beginpos = i;
						endpos = j;
						min = temp;
						break;
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

vector<vector<double>> global_node_eta_matrix(vector<orderitem> temporderlist,
	unordered_map<string, vector<node>> imcompletesolution,vector<node> currentroute_nodelist){
		for(string each_car:veichles){
			vector<string> left_order = imcompletesolution[each_car][0].getdeliverlist();
			while (left_order.size() != 0){
				string tempstr = left_order.back();
				orderitem neworder = orderlist[tempstr];
				node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
				imcompletesolution[each_car].push_back(deliver_node);
				PDlist_next(imcompletesolution[each_car],"fuck777");
				left_order.pop_back();
			}
		}
		unordered_map<string, vector<node>> dealsolution = imcompletesolution;
		for (orderitem neworder : temporderlist) {
			unordered_map<string, vector<node>> memorysolution = dealsolution;
			double temp = 100000000000; vector<node> templist1;
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
			string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
			vector<node>routelist, templist;
			for (string each_car:veichles) {
				routelist = dealsolution[each_car];
				if (routelist.size() == 1) {
					templist = routelist;
					// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
					insertBefore(templist, 1, deliver_node);
					insertAfter(templist, 0, pickup_node);
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = total_cost(dealsolution);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
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
							PDlist(templist,"fucj");
							for (int k = 0; k < templist.size(); k++) {
								if (templist[k].getstorage() < 0) {
									getout = 1; break;
								}
							}
							if (getout)continue;
							templist1 = dealsolution[each_car];
							dealsolution[each_car] = templist;
							Timecaculate(dealsolution);
							temp = total_cost(dealsolution);
							dealsolution[each_car] = templist1;
							dealsolution = memorysolution;
							if (temp < min) {
								minveichle = each_car;
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
	//	dealsolution = Tabu_search(dealsolution,"fuck");
	//	printsolution(dealsolution);
		double order_tao = 1/(total_cost(dealsolution));
	//	cout<<Total_node_count(dealsolution)<<" "<<total_cost(dealsolution)<<" "<<f1_timecosuming(dealsolution)<<" "<<f2_totaldistance(dealsolution)<<endl;
	//	printsolution(dealsolution);
		vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
		return current_car_eta_matrix;
}

vector<vector<double>> global_node_eta_matrix(vector<orderitem> temporderlist,
	unordered_map<string, vector<node>> imcompletesolution,vector<node> currentroute_nodelist,string logo){
		temporderlist = shuffle_vector(temporderlist);
		for(string each_car:veichles){
			vector<string> left_order = imcompletesolution[each_car][0].getdeliverlist();
			while (left_order.size() != 0){
				string tempstr = left_order.back();
				orderitem neworder = orderlist[tempstr];
				node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
				imcompletesolution[each_car].push_back(deliver_node);
				PDlist_next(imcompletesolution[each_car],"fuck777");
				left_order.pop_back();
			}
		}
		unordered_map<string, vector<node>> dealsolution = imcompletesolution;
		for (orderitem neworder : temporderlist) {
			unordered_map<string, vector<node>> memorysolution = dealsolution;
			double temp = 100000000000; vector<node> templist1;
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
			string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
			vector<node>routelist, templist;
			for (string each_car:veichles) {
				routelist = dealsolution[each_car];
				if (routelist.size() == 1) {
					templist = routelist;
					// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
					insertBefore(templist, 1, deliver_node);
					insertAfter(templist, 0, pickup_node);
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = f2_totaldistance(dealsolution);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
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
							PDlist(templist,"fucj");
							for (int k = 0; k < templist.size(); k++) {
								if (templist[k].getstorage() < 0) {
									getout = 1; break;
								}
							}
							if (getout)continue;
							templist1 = dealsolution[each_car];
							dealsolution[each_car] = templist;
							Timecaculate(dealsolution);
							temp = f2_totaldistance(dealsolution);
							dealsolution[each_car] = templist1;
							dealsolution = memorysolution;
							if (temp < min) {
								minveichle = each_car;
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
	//	dealsolution = Tabu_search(dealsolution,"fuck");
	//	printsolution(dealsolution);
		double order_tao = 1/(f2_totaldistance(dealsolution));
	//	cout<<Total_node_count(dealsolution)<<" "<<total_cost(dealsolution)<<" "<<f1_timecosuming(dealsolution)<<" "<<f2_totaldistance(dealsolution)<<endl;
	//	printsolution(dealsolution);
		/*printfactory(dealsolution,"fuck");
		cout<<"fuck!!!!!1"<<endl;*/
		cout<<1/order_tao<<" "<<Total_node_count(dealsolution)<<endl;
		order_tao = 1.0/300.0;
		vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
		return current_car_eta_matrix;
}

void remove_range(vector<node>& vec, int a, int b) {
    // 有效性检查
    if (vec.empty()) return;
    if (a > b) swap(a, b);
    if (a < 0 || b >= vec.size()) return; // 或抛出异常

    // 转换为迭代器范围 [first, last)
    auto first = vec.begin() + a;
    auto last  = vec.begin() + b + 1; // +1 因 erase 需要 [start, end)

    vec.erase(first, last);
}

void swap_vector_ranges(vector<node>& vec1, int a1, int b1,
                       vector<node>& vec2, int a2, int b2) {
    // 检查索引有效性
    if (a1 < 0 || b1 >= vec1.size() || a1 > b1 ||
        a2 < 0 || b2 >= vec2.size() || a2 > b2) {
        return;  // 或抛出异常 invalid_argument("Invalid index range")
    }

    // 提取区间元素（移动语义减少拷贝开销）
    vector<node> temp1(
        make_move_iterator(vec1.begin() + a1),
        make_move_iterator(vec1.begin() + b1 + 1)
    );
    vector<node> temp2(
        make_move_iterator(vec2.begin() + a2),
        make_move_iterator(vec2.begin() + b2 + 1)
    );

    // 删除原区间元素
    vec1.erase(vec1.begin() + a1, vec1.begin() + b1 + 1);
    vec2.erase(vec2.begin() + a2, vec2.begin() + b2 + 1);

    // 插入对方元素（移动语义）
    vec1.insert(vec1.begin() + a1,
               make_move_iterator(temp2.begin()),
               make_move_iterator(temp2.end()));
    vec2.insert(vec2.begin() + a2,
               make_move_iterator(temp1.begin()),
               make_move_iterator(temp1.end()));
}

void swap_vector_ranges_internal(std::vector<node>& vec, int a1, int b1, int a2, int b2) {
    // 1. 基础有效性检查（索引是否合法）
    const int size = static_cast<int>(vec.size());
    if (a1 < 0 || b1 >= size || a1 > b1 ||
        a2 < 0 || b2 >= size || a2 > b2) {
        return; // 非法输入直接返回
        // 可选：抛出异常 throw std::invalid_argument("Invalid index range");
    }

    // 2. 确保 a1 <= a2，简化后续逻辑
    if (a1 > a2) {
        std::swap(a1, a2);
        std::swap(b1, b2);
    }

    // 3. 严格非重叠检查（包括相邻）
    if (b1 >= a2) {
        return; // 区间重叠或相邻，不执行交换
    }

    // 4. 提取两个区间的元素（移动语义优化）
    const int len1 = b1 - a1 + 1;
    const int len2 = b2 - a2 + 1;
    std::vector<node> temp1, temp2;
    temp1.reserve(len1);
    temp2.reserve(len2);
    std::move(vec.begin() + a1, vec.begin() + b1 + 1, std::back_inserter(temp1));
    std::move(vec.begin() + a2, vec.begin() + b2 + 1, std::back_inserter(temp2));

    // 5. 删除原区间（先删前面的 a1 区间）
    vec.erase(vec.begin() + a1, vec.begin() + b1 + 1);
    const int a2_new = a2 - len1; // 删除 a1 区间后，a2 的新位置
    vec.erase(vec.begin() + a2_new, vec.begin() + a2_new + len2);

    // 6. 插入交换后的区间
    vec.insert(vec.begin() + a1, temp2.begin(), temp2.end());
    const int new_a2 = a1 + static_cast<int>(temp2.size());
    vec.insert(vec.begin() + new_a2, temp1.begin(), temp1.end());
}

vector<node> extract_and_remove(std::vector<node>& vec, int a, int b) {
    // 索引有效性检查
    if (vec.empty() || a < 0 || b >= vec.size() || a > b) {
        return {}; // 返回空vector或抛出异常
    }

    // 提取区间元素（使用移动语义）
    std::vector<node> extracted(
        std::make_move_iterator(vec.begin() + a),
        std::make_move_iterator(vec.begin() + b + 1)
    );

    // 删除原区间元素
    vec.erase(vec.begin() + a, vec.begin() + b + 1);
    
    return extracted;
}

void block_insert(std::vector<node>& target, std::vector<node> source, size_t index) noexcept {
    if (source.empty()) return;

    // 关键修改点：插入位置 = index + 1（但要确保不超过 target.size()）
    const size_t insert_pos = std::min(index + 1, target.size());

    // 预分配内存（避免多次扩容）
    const size_t new_size = target.size() + source.size();
    if (target.capacity() < new_size) {
        target.reserve(new_size);
    }

    // 移动插入到 insert_pos 位置
    target.insert(
        target.begin() + insert_pos,
        std::make_move_iterator(source.begin()),
        std::make_move_iterator(source.end())
    );

}

void getpdglist(unordered_map<string,int> &order_pnode,unordered_map<string,int> &order_dnode,
vector<string> &car_orderlist,unordered_map<string,vector<node>> tempsolution,unordered_map<string,string> &order_car){
	for(string each_car:veichles){
		for(int i = 1;i<tempsolution[each_car].size();i++){
			if(tempsolution[each_car][i].getpickup()){
				car_orderlist.push_back(tempsolution[each_car][i].getorderid());
				order_car[tempsolution[each_car][i].getorderid()] = each_car;
				order_pnode[tempsolution[each_car][i].getorderid()] = i;
			}else{
				if(order_pnode.count(tempsolution[each_car][i].getorderid()) == 1){
					order_dnode[tempsolution[each_car][i].getorderid()] = i;
				}
			}
		}
	}
}

unordered_map<string, vector<node>> block_exchange(unordered_map<string, vector<node>> tempsolution,unordered_map<string,int> order_pnode
	,unordered_map<string,int> order_dnode,vector<string> car_orderlist,int anchor,unordered_map<string,string> order_car){
	unordered_map<string,vector<node>> localsolution = tempsolution;
	unordered_map<string,vector<node>> minsolution = tempsolution;
	double mincost = total_cost(tempsolution);
	int select_order1 = anchor;
	string select_car1 = order_car[car_orderlist[anchor]];
	for(int i = 0;i < car_orderlist.size() ;i++){
		string select_car2 = order_car[car_orderlist[i]];
		int select_order2 = i;
		if(i == anchor)continue;
		localsolution = tempsolution;
		if(select_car1 == select_car2){
			swap_vector_ranges_internal(localsolution[select_car1],order_pnode[car_orderlist[select_order1]],order_dnode[car_orderlist[select_order1]],
				order_pnode[car_orderlist[select_order2]],order_dnode[car_orderlist[select_order2]]);
		}else{
			swap_vector_ranges(localsolution[select_car1],order_pnode[car_orderlist[select_order1]],order_dnode[car_orderlist[select_order1]],
			localsolution[select_car2],order_pnode[car_orderlist[select_order2]],order_dnode[car_orderlist[select_order2]]);
		}
		if(PDlist(localsolution[select_car1],"fuck")&&PDlist(localsolution[select_car2],"fuck")){
			Timecaculate(localsolution);
			double tempcost = total_cost(localsolution);
			if(tempcost < mincost){
				mincost = tempcost;
				minsolution = localsolution;
				break;
			}

		}
	}
	return minsolution;
}

unordered_map<string, vector<node>> block_relocate(unordered_map<string, vector<node>> tempsolution,unordered_map<string,int> order_pnode
,unordered_map<string,int> order_dnode,vector<string> car_orderlist,int anchor,unordered_map<string,string> order_car){
	unordered_map<string,vector<node>> localsolution = tempsolution;
	unordered_map<string,vector<node>> aftersolution = tempsolution;
	double mincost = total_cost(tempsolution);
	int select_order = anchor;
	string select_car = order_car[car_orderlist[anchor]];
	if(car_orderlist.size() == 0)return tempsolution;
	vector<node> insert_nodelist;
	insert_nodelist = extract_and_remove(localsolution[select_car],order_pnode[car_orderlist[select_order]],order_dnode[car_orderlist[select_order]]);
	PDlist(localsolution[select_car],"fuck");
	for(string each_car:veichles){
		for(int i = 0; i < localsolution[each_car].size();i++){
			unordered_map<string,vector<node>> middle = localsolution;
			block_insert(middle[each_car],insert_nodelist,i);
			if(PDlist(middle[each_car],"fuck")){
				Timecaculate(middle);
				double tempcost = total_cost(middle);
				if(tempcost<mincost){
					aftersolution = middle;
					mincost = tempcost;
					break;
				}
			}
		}
	}
	return aftersolution;
}

unordered_map<string, vector<node>> couple_exchange(unordered_map<string, vector<node>> tempsolution,unordered_map<string,int> order_pnode
,unordered_map<string,int> order_dnode,vector<string> car_orderlist,int anchor,unordered_map<string,string> order_car){
	unordered_map<string,vector<node>> minsolution = tempsolution;
	unordered_map<string,vector<node>> localsolution = tempsolution;
	double mincost = total_cost(tempsolution);
	int iter1 = anchor;string select_car1 = order_car[car_orderlist[anchor]],select_car2;
	for(int iter2 = 0;iter2<car_orderlist.size();iter2++){
		localsolution = tempsolution;
		select_car2 = order_car[car_orderlist[iter2]];
		localsolution[select_car1][order_pnode[car_orderlist[iter1]]] = tempsolution[select_car2][order_pnode[car_orderlist[iter2]]] ;
		localsolution[select_car1][order_dnode[car_orderlist[iter1]]] = tempsolution[select_car2][order_dnode[car_orderlist[iter2]]] ;
		localsolution[select_car2][order_pnode[car_orderlist[iter2]]] = tempsolution[select_car1][order_pnode[car_orderlist[iter1]]];
		localsolution[select_car2][order_dnode[car_orderlist[iter2]]] = tempsolution[select_car1][order_dnode[car_orderlist[iter1]]];
		if(PDlist(localsolution[select_car1],"fuck")&&PDlist(localsolution[select_car2],"fuck")){
			Timecaculate(localsolution);
			double tempcost = total_cost(localsolution);
			if(tempcost<mincost){
				mincost = tempcost;
				minsolution = localsolution;
				break;
			}
		}
	}
	return minsolution;
}

unordered_map<string, vector<node>> couple_relocate(unordered_map<string, vector<node>> tempsolution,unordered_map<string,int> order_pnode
,unordered_map<string,int> order_dnode,vector<string> car_orderlist,int anchor,unordered_map<string,string> order_car){
	unordered_map<string,vector<node>> minsolution = tempsolution;
	double mincost = total_cost(tempsolution);
	string select_car = order_car[car_orderlist[anchor]];
	int select_order = anchor;
	if(car_orderlist.size() == 0)return tempsolution;
	orderitem neworder = orderlist[car_orderlist[select_order]];
	string temporder_id = neworder.getOrderId();
	minsolution[select_car].erase(remove_if(minsolution[select_car].begin(),minsolution[select_car].end(),[temporder_id](node value){ return value.getorderid() == temporder_id;}),minsolution[select_car].end());
	PDlist(minsolution[select_car]);
	insert_order1(minsolution,neworder,"break");
	return minsolution;
}

unordered_map<string, vector<node>> Tabu_search(unordered_map<string, vector<node>>initialsolution,string loge) {
	queue<string> Tabulist;
	int node_num = Total_node_count(initialsolution);
	tabulist_size = (node_num < 8) ? 2 : static_cast<int>(std::round(node_num * 0.3));
	string temptransformer,bestneighbor_transform;
	unordered_map<string, vector<node>> currentsolution,tempsolution ,bestsolution, bestneighbor;
	int iter = 0, niter = 0,Maxiter = 10,Neighbortheshold = 4,bestGen = 0;
	bestsolution = initialsolution; currentsolution = initialsolution;
	double min = total_cost(bestsolution);
	while (iter < Maxiter) {
		bestneighbor = currentsolution;
		niter = 0;
		unordered_map<string,int> order_pnode,order_dnode;
		vector<string> car_orderlist;unordered_map<string,string> order_car;
		getpdglist(order_pnode,order_dnode,car_orderlist,currentsolution,order_car);
		double min_neighbor = total_cost(bestneighbor);
		int size  = 0;
		size += car_orderlist.size();
		if(size < 2)break;
		vector<vector<bool>> book(size,vector<bool>(4,false));
		bool feasibleNeighbor = false;int cnt = 0;
		while (niter < Neighbortheshold) {
			int anchor = weighted_random(0,size - 1),nr = getrangenumber(0,3);
			while(book[anchor][nr]){
				anchor = weighted_random(0,size - 1);
				nr = getrangenumber(0,3);
			}
			book[anchor][nr] = true;
			switch (nr) {
			case 1:
				tempsolution = block_exchange(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			case 2:
				tempsolution = block_relocate(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			case 3:
				tempsolution = couple_exchange(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			case 0:
				tempsolution = couple_relocate(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			}
			temptransformer = transform_solution_to_string(tempsolution);
			if (noTabu(temptransformer,Tabulist)) {
				double tempmin = total_cost(tempsolution);
				if (tempmin < min_neighbor) {
					bestneighbor = tempsolution;
					bestneighbor_transform = temptransformer;
					min_neighbor = tempmin;
				}
				feasibleNeighbor = true;
				niter++;
			}
			cnt++;
			if(cnt > size*3){
				break;
			}
		}
		if(!feasibleNeighbor){
			break;
		}
		if (min_neighbor < min) {
			bestGen = iter;
			bestsolution = bestneighbor;
			min = min_neighbor;
		}
		currentsolution = bestneighbor;
		Tabulist.push(bestneighbor_transform);
		iter++;
	}
	return bestsolution;
}

std::tuple<int, int, int> selectElement3D(
    std::vector<std::vector<std::vector<double>>>& probMatrix, // 移除 const
    double test_q_order
) {
    double rand1 = generateRandomNumber(); // 移除 const

    if (rand1 > test_q_order) {
        // ========== 轮盘赌逻辑 ==========
        std::vector<double> cumulative;
        std::vector<std::tuple<int, int, int>> indices;
        double total = 0.0;

        // 遍历三维矩阵（移除所有 const 修饰符）
        for (int i = 0; i < probMatrix.size(); ++i) {
            auto& layer = probMatrix[i]; // 移除 const
            for (int j = 0; j < layer.size(); ++j) {
                auto& row = layer[j]; // 移除 const
                for (int k = 0; k < row.size(); ++k) {
                    double val = row[k]; // 移除 const
                    if (val < 0.0) {
                        throw std::invalid_argument("Probability values must be non-negative");
                    }
                    total += val;
                    cumulative.push_back(total);
                    indices.emplace_back(i, j, k);
                }
            }
        }

        // 检查总和合法性
        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid 3D probability matrix (sum <= 0 or empty)");
        }

        // 生成随机数并查找
        double rand2 = generateRandomNumber() * total; // 移除 const
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();

    } else {
        // ========== 最大值逻辑 ==========
        std::tuple<int, int, int> max_pos = {-1, -1, -1};
        double max_val = -1.0;

        for (int i = 0; i < probMatrix.size(); ++i) {
            auto& layer = probMatrix[i]; // 移除 const
            for (int j = 0; j < layer.size(); ++j) {
                auto& row = layer[j]; // 移除 const
                for (int k = 0; k < row.size(); ++k) {
                    double val = row[k]; // 移除 const
                    if (val < 0.0) {
                        throw std::invalid_argument("Probability values must be non-negative");
                    }
                    
                    // 优先选择第一个最大值
                    if (val > max_val || (val == max_val && std::get<0>(max_pos) == -1)) {
                        max_val = val;
                        max_pos = {i, j, k};
                    }
                }
            }
        }

        if (std::get<0>(max_pos) == -1) {
            throw std::invalid_argument("3D matrix is empty or all elements are negative");
        }
        return max_pos;
    }
}

tuple<int, int, int, int> selectElement4D(
    std::vector<std::vector<std::vector<std::vector<double>>>>& probMatrix,
    double test_q_order
) {
    double rand1 = generateRandomNumber();

    if (rand1 > test_q_order) {
        // ========== 轮盘赌逻辑 ==========
        std::vector<double> cumulative;
        std::vector<std::tuple<int, int, int, int>> indices;
        double total = 0.0;

        // 遍历四维矩阵
        for (int i = 0; i < probMatrix.size(); ++i) {
            auto& hyperlayer = probMatrix[i];
            for (int j = 0; j < hyperlayer.size(); ++j) {
                auto& layer = hyperlayer[j];
                for (int k = 0; k < layer.size(); ++k) {
                    auto& row = layer[k];
                    for (int l = 0; l < row.size(); ++l) {
                        double val = row[l];
                        if (val < 0.0) {
                            throw std::invalid_argument("Probability values must be non-negative");
                        }
                        total += val;
                        cumulative.push_back(total);
                        indices.emplace_back(i, j, k, l);
                    }
                }
            }
        }

        // 检查总和合法性
        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid 4D probability matrix (sum <= 0 or empty)");
        }

        // 生成随机数并查找
        double rand2 = generateRandomNumber() * total;
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();

    } else {
        // ========== 最大值逻辑 ==========
        std::tuple<int, int, int, int> max_pos = {-1, -1, -1, -1};
        double max_val = -1.0;

        for (int i = 0; i < probMatrix.size(); ++i) {
            auto& hyperlayer = probMatrix[i];
            for (int j = 0; j < hyperlayer.size(); ++j) {
                auto& layer = hyperlayer[j];
                for (int k = 0; k < layer.size(); ++k) {
                    auto& row = layer[k];
                    for (int l = 0; l < row.size(); ++l) {
                        double val = row[l];
                        if (val < 0.0) {
                            throw std::invalid_argument("Probability values must be non-negative");
                        }
                        
                        // 优先选择第一个最大值
                        if (val > max_val || (val == max_val && std::get<0>(max_pos) == -1)) {
                            max_val = val;
                            max_pos = {i, j, k, l};
                        }
                    }
                }
            }
        }

        if (std::get<0>(max_pos) == -1) {
            throw std::invalid_argument("4D matrix is empty or all elements are negative");

        }
        return max_pos;
    }
}

pair<int, int> selectElement(const std::vector<std::vector<double>>& probMatrix, double test_q_order) {
    const double rand1 = generateRandomNumber();

    if (rand1 > test_q_order) {
        // ========== 轮盘赌逻辑 ==========
        std::vector<double> cumulative;
        std::vector<std::pair<int, int>> indices;
        double total = 0.0;

        // 遍历所有元素计算累积和
        for (int i = 0; i < probMatrix.size(); ++i) {
            const auto& row = probMatrix[i];
            for (int j = 0; j < row.size(); ++j) {
                const double val = row[j];
                if (val < 0.0) {
                    throw std::invalid_argument("Probability values must be non-negative");
                }
                total += val;
                cumulative.push_back(total);
                indices.emplace_back(i, j);
            }
        }

        // 检查总和合法性
        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid probability matrix (sum <= 0 or empty)");
        }

        // 生成随机数并查找
        const double rand2 = generateRandomNumber() * total;
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        
        // 处理浮点误差（当 rand2 接近 total 时选最后一个元素）
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();

    } else {
        // ========== 最大值逻辑 ==========
        std::pair<int, int> max_pos = {-1, -1};
        double max_val = -1.0; // 初始化为 -1 以兼容 0 值

        for (int i = 0; i < probMatrix.size(); ++i) {
            const auto& row = probMatrix[i];
            for (int j = 0; j < row.size(); ++j) {
                const double val = row[j];
                if (val < 0.0) {
                    throw std::invalid_argument("Probability values must be non-negative");
                }
                
                // 核心逻辑：更新最大值位置（优先第一个最大值）
                if (val > max_val || (val == max_val && max_pos.first == -1)) {
                    max_val = val;
                    max_pos = {i, j};
                }
            }
        }

        // 检查是否存在有效元素
        if (max_pos.first == -1) {
            throw std::invalid_argument("Matrix is empty or all elements are negative");
        }
        return max_pos;
    }
}

std::pair<int, int> selectElement2(const std::vector<std::vector<double>>& probMatrix, double test_q_order) {
    const double rand1 = generateRandomNumber();

    if (rand1 > test_q_order) {
        // ========== 均匀概率选择逻辑 ==========
        std::vector<std::pair<int, int>> validIndices;
        
        // 遍历所有元素，收集有效坐标
        for (int i = 0; i < probMatrix.size(); ++i) {
            const auto& row = probMatrix[i];
            for (int j = 0; j < row.size(); ++j) {
                if (row[j] < 0.0) {
                    throw std::invalid_argument("Probability values must be non-negative");
                }
                validIndices.emplace_back(i, j);
            }
        }

        if (validIndices.empty()) {
            throw std::invalid_argument("Matrix is empty or all elements are negative");
        }

        // 生成随机索引（均匀概率）
        const double rand2 = generateRandomNumber() * validIndices.size();
        size_t index = static_cast<size_t>(rand2);
        if (index >= validIndices.size()) index = validIndices.size() - 1;  // 处理浮点误差

        return validIndices[index];

    } else {
        // ========== 轮盘赌选择逻辑 ==========
        std::vector<double> cumulative;
        std::vector<std::pair<int, int>> indices;
        double total = 0.0;

        // 计算累积概率
        for (int i = 0; i < probMatrix.size(); ++i) {
            const auto& row = probMatrix[i];
            for (int j = 0; j < row.size(); ++j) {
                const double val = row[j];
                if (val < 0.0) {
                    throw std::invalid_argument("Probability values must be non-negative");
                }
                total += val;
                cumulative.push_back(total);
                indices.emplace_back(i, j);
            }
        }

        // 检查概率合法性
        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid probability matrix (sum <= 0 or empty)");
        }

        // 根据概率选择元素
        const double rand2 = generateRandomNumber() * total;
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();
    }
}

int selectElement1D(std::vector<double>& probArray, double test_q_order) {
    const double rand1 = generateRandomNumber();

    if (rand1 > test_q_order) {
        // ========== 轮盘赌逻辑 ==========
        std::vector<double> cumulative;
        std::vector<int> indices;  // 存储一维索引
        double total = 0.0;

        for (int i = 0; i < probArray.size(); ++i) {
            const double val = probArray[i];
            if (val < 0.0) {
                throw std::invalid_argument("Probability values must be non-negative");
            }
            total += val;
            cumulative.push_back(total);
            indices.push_back(i);  // 直接存储一维索引
        }

        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid probability array (sum <= 0 or empty)");
        }

        const double rand2 = generateRandomNumber() * total;
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();

    } else {
        // ========== 最大值逻辑 ==========
        int max_index = -1;
        double max_val = -1.0;

        for (int i = 0; i < probArray.size(); ++i) {
            const double val = probArray[i];
            if (val < 0.0) {
                throw std::invalid_argument("Probability values must be non-negative");
            }
            
            if (val > max_val || (val == max_val && max_index == -1)) {
                max_val = val;
                max_index = i;  // 直接记录一维索引
            }
        }

        if (max_index == -1) {
            throw std::invalid_argument("Array is empty or all elements are negative");
        }
        return max_index;  // 直接返回 int 索引
    }
}

int selectElement1D2(const std::vector<double>& probArray, double test_q_order) {
    const double rand1 = generateRandomNumber();

    if (rand1 <= test_q_order) {
        // ========== 轮盘赌逻辑（与selectElement2的else分支一致） ==========
        std::vector<double> cumulative;
        std::vector<int> indices;  // 存储有效索引
        double total = 0.0;

        // 遍历所有元素，计算累积概率
        for (int i = 0; i < probArray.size(); ++i) {
            const double val = probArray[i];
            if (val < 0.0) {
                throw std::invalid_argument("Probability values must be non-negative");
            }
            total += val;
            cumulative.push_back(total);
            indices.push_back(i);
        }

        // 检查概率合法性
        if (total <= 0.0 || cumulative.empty()) {
            throw std::invalid_argument("Invalid probability array (sum <= 0 or empty)");
        }

        // 根据概率选择元素
        const double rand2 = generateRandomNumber() * total;
        auto it = std::lower_bound(cumulative.begin(), cumulative.end(), rand2);
        return (it != cumulative.end()) ? indices[it - cumulative.begin()] : indices.back();

    } else {
        // ========== 均匀分布逻辑（与selectElement2的if分支一致） ==========
        std::vector<int> validIndices;

        // 遍历所有元素，收集有效索引
        for (int i = 0; i < probArray.size(); ++i) {
            const double val = probArray[i];
            if (val < 0.0) {
                throw std::invalid_argument("Probability values must be non-negative");
            }
            validIndices.push_back(i);
        }

        if (validIndices.empty()) {
            throw std::invalid_argument("Array is empty or all elements are negative");
        }

        // 生成随机索引（均匀概率）
        const double rand2 = generateRandomNumber() * validIndices.size();
        size_t index = static_cast<size_t>(rand2);
        if (index >= validIndices.size()) index = validIndices.size() - 1;  // 处理浮点误差

        return validIndices[index];
    }

}

template<typename T>
void preorderHelper(const vector<T>& tree, const vector<bool>& valid, int index) {
    if (index >= tree.size() || !valid[index]) {
        return;
    }
    // 访问当前节点
    cout << tree[index] << " ";
    // 递归处理左子树
    preorderHelper(tree, valid, 2 * index + 1);
    // 递归处理右子树
    preorderHelper(tree, valid, 2 * index + 2);
}

template<typename T>
void preorderTraversal(const vector<T>& tree, const vector<bool>& valid) {
    if (tree.empty() || valid.empty() || !valid[0]) {
        return;
    }
    preorderHelper(tree, valid, 0);
}

double findMaxValue(const std::vector<double>& vec) {
    if (vec.empty()) {
        throw std::invalid_argument("Vector is empty"); // 处理空向量
    }
    auto max_iter = std::max_element(vec.begin(), vec.end());
    return *max_iter;
}

double findMaxFromIndices(const std::vector<double>& data, const std::vector<int>& indices) {
    if (data.empty()) {
        throw std::invalid_argument("输入向量不能为空");  // 空数据检查[1,2,5](@ref)
    }

    double max_val;
    bool first_valid = false;  // 标记是否找到第一个有效元素

    for (int idx : indices) {
        if (idx >= 0 && idx < static_cast<int>(data.size())) {
            if (!first_valid) {  // 初始化第一个有效值
                max_val = data[idx];
                first_valid = true;
            } else {
                if (data[idx] > max_val) {  // 直接比较并更新最大值
                    max_val = data[idx];
                }
            }
        }
    }

    if (!first_valid) {
        throw std::runtime_error("索引范围内无有效元素");  // 无有效索引处理[3,5](@ref)
    }

    return max_val;
}

// 先处理单个车辆
vector<vector<double>> initial_order_eta(vector<orderitem> temp,unordered_map<string,vector<node>> solutionlist){
	vector<vector<double>> matrix(temp.size() + veichle_num,vector<double>(temp.size() + veichle_num,0));
    int rows = temp.size() + veichle_num,cols = temp.size() + veichle_num;
	for(int i = 0;i<rows;i++){
        for(int j = 0;j<cols;j++){
			if(j>=0&&j<veichle_num)continue;
            if(i>=0&&i<veichle_num){
				if( Distance[location[solutionlist[veichles[i]].front().getId()]][location[temp[j - veichle_num].getPickupFactoryId()]]== 0){
					matrix[i][j] = 1.0/same;
				}else{
					matrix[i][j] = 1.0/Distance[location[solutionlist[veichles[i]].front().getId()]][location[temp[j - veichle_num].getPickupFactoryId()]];
				}
            }else{
				if(Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]] == 0){
					matrix[i][j] = 1.0/same;
				}else
				{
					matrix[i][j] = 1.0/Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]];
					//matrix[i][j] = 1.0/min(Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]],Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getDeliveryFactoryId()]]);
				}
				
                
            }
        }
    }
    return matrix;
}

vector<vector<double>> initial_order_eta(vector<orderitem> temp,unordered_map<string,vector<node>> solutionlist,string fuck){
	vector<vector<double>> matrix(temp.size() + veichle_num,vector<double>(temp.size() + veichle_num,0));
    int rows = temp.size() + veichle_num,cols = temp.size() + veichle_num;
	for(int i = 0;i<rows;i++){
        for(int j = 0;j<cols;j++){
			if(j>=0&&j<veichle_num)continue;
            if(i>=0&&i<veichle_num){
				if( Distance[location[solutionlist[veichles[i]].front().getId()]][location[temp[j - veichle_num].getPickupFactoryId()]]== 0){
					matrix[i][j] = 1.0/same;
				}else{
					matrix[i][j] = 1.0/Distance[location[solutionlist[veichles[i]].front().getId()]][location[temp[j - veichle_num].getPickupFactoryId()]];
				}
            }else{
				if(Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]] == 0){
					matrix[i][j] = 1.0/same;
				}else
				{
					//matrix[i][j] = 1.0/Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]];
					matrix[i][j] = 1.0/min(Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getPickupFactoryId()]],Distance[location[temp[i - veichle_num].getPickupFactoryId()]][location[temp[j - veichle_num].getDeliveryFactoryId()]]);
				}
				
                
            }
        }
    }
    return matrix;
}

double initial_order_tao(vector<orderitem> temporderlist,
	unordered_map<string, vector<node>> imcompletesolution,vector<node> currentroute_nodelist,unordered_map<string,vector<node>> &retursolution){
		temporderlist = shuffle_vector(temporderlist);
		int count = 0;
        for(string each_car:veichles){
			vector<string> left_order = imcompletesolution[each_car][0].getdeliverlist();
			while (left_order.size() != 0){
				string tempstr = left_order.back();
				orderitem neworder = orderlist[tempstr];
				node deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
				imcompletesolution[each_car].push_back(deliver_node);
				PDlist_next(imcompletesolution[each_car],"fuck777");
				left_order.pop_back();
			}
		}
		unordered_map<string, vector<node>> dealsolution = imcompletesolution;
		for (orderitem neworder : temporderlist) {
			unordered_map<string, vector<node>> memorysolution = dealsolution;
			double temp = 100000000000; vector<node> templist1;
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
			string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
			vector<node>routelist, templist;
			for (string each_car:veichles) {
				routelist = dealsolution[each_car];
				if (routelist.size() == 1) {
					templist = routelist;
					// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
					insertBefore(templist, 1, deliver_node);
					insertAfter(templist, 0, pickup_node);
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = f2_totaldistance(dealsolution);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
						beginpos = 0;
						endpos = 1;
						min = temp;
					}
					count++;
				}
				else {
					for (int i = 0; i < routelist.size(); i++) {
						vector<int> toinsertdeliver;
						for (int j = i + 1; j <= routelist.size(); j++) {
							if(j == routelist.size()){
								if(routelist[i].getdeliverlist().size() == 0)toinsertdeliver.push_back(j);
								break;
							}
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
							PDlist(templist,"fucj");
							for (int k = 0; k < templist.size(); k++) {
								if (templist[k].getstorage() < 0) {
									getout = 1; break;
								}
							}
							if (getout)continue;
							templist1 = dealsolution[each_car];
							dealsolution[each_car] = templist;
							Timecaculate(dealsolution);
							temp = f2_totaldistance(dealsolution);
							dealsolution[each_car] = templist1;
							dealsolution = memorysolution;
							if (temp < min) {
								minveichle = each_car;
								beginpos = i;
								endpos = j;
								min = temp;
							}
							count++;
						}
					}
				}
			}
			insertBefore(dealsolution[minveichle], endpos, deliver_node);
			insertAfter(dealsolution[minveichle], beginpos, pickup_node);
			PDlist(dealsolution[minveichle]);
			//Timecaculate(dealsolution);
		}
		retursolution = dealsolution;
        double order_tao = 1/(f2_totaldistance(dealsolution));
        cout<<1/order_tao<<" "<<Total_node_count(dealsolution)<<endl;
		cout<<count<<endl;
        return order_tao;
}

vector<vector<vector<double>>> initial_eta(vector<orderitem> unfinishorderlist,unordered_map<string,vector<node>> startsolution){
    int rows = unfinishorderlist.size() + veichle_num;
    int cols = unfinishorderlist.size();
    vector<vector<vector<double>>> matrix(2,vector<vector<double>>(unfinishorderlist.size() + veichle_num,vector<double>(unfinishorderlist.size(),0)));;
    // 左子树延展
    for(int i = 0;i<rows;i++){
        for(int j = 0;j<cols;j++){
            if(i>=0&&i<veichle_num){
                matrix[0][i][j] = 0;
            }else{
                matrix[0][i][j] = 1/Distance[location[unfinishorderlist[i - veichle_num].getPickupFactoryId()]][location[unfinishorderlist[j].getPickupFactoryId()]];
            }
        }
    }
    // 右子树延展
    for(int i = 0;i<rows;i++){
        for(int j = 0;j<cols;j++){
            if(i>=0&&i<veichle_num){
                matrix[1][i][j] = 1/Distance[location[startsolution[veichles[i]].front().getId()]][location[unfinishorderlist[j].getPickupFactoryId()]];
            }else{
                matrix[1][i][j] = 1/Distance[location[unfinishorderlist[i - veichle_num].getDeliveryFactoryId()]][location[unfinishorderlist[j].getPickupFactoryId()]];
            }
        }
    }
    return matrix;
}

vector<double> intial_orderlist(vector<orderitem> &o_list,unordered_map<string,vector<node>> &t){
	vector<double> m;
	for(int i = 0;i<o_list.size();i++){
		double min_value = 100000000;
		for(int j = 0;j<veichle_num;j++){
			double temp = Distance[location[t[veichles[j]].back().getId()]][location[o_list[i].getPickupFactoryId()]];
			if(temp<min_value){
				min_value = temp;
			}
		}
		m.push_back(1.0/min_value);
	}
	return m;
}

static int count_right = 0;
// 使用订单的顺序存储的方法进行判断，括号匹配,n的三次方的时间复杂度
vector<int> order_construct(unordered_map<string,vector<node>> &tempsolution,vector<vector<double>> &tao_matrix,vector<vector<double>> &eta_matrix,double q,double local_update,double local_tao,
	node p_node,node d_node,unordered_map<string,int> order_pnode,unordered_map<string,int>order_dnode,double index1,double index2){
	// 每一对的概率以及每一对进行插入的位置
	vector<vector<double>> probility(veichle_num);vector<vector<pair<int,int>>> insert_loc(veichle_num);
	double sigma = 0.5;
	for(int each = 0;each<veichle_num;each++){
		string each_car = veichles[each];
		double p_storage = capacity,d_storage = 0; // 动态更新当前节点的可以进行存储的值，从而节省N的时间复杂度的判断时间
		// 默认存在起始点
		vector<string> order_stack;
		for(int loc1 = 0;loc1<tempsolution[each_car].size();loc1++){
			string mark_order = "fuck";
			if(loc1!=0){
				if(tempsolution[each_car][loc1].getpickup()){
					order_stack.push_back(tempsolution[each_car][loc1].getorderid());
				}else{
					order_stack.pop_back();
				}
				if(tempsolution[each_car][loc1].getpickup()){	// 确保初始的点的demand为0，其开始不受影响
					p_storage -= tempsolution[each_car][loc1].getpakage();
				}else{
					p_storage += tempsolution[each_car][loc1].getpakage();
				}
			}
			if(order_stack.size()!=0)mark_order = order_stack.back();
			
			double d_storage_max = 0;d_storage = 0;
			for(int loc2 = loc1 + 1;loc2<tempsolution[each_car].size()+1;loc2++){
				if((p_storage - d_storage_max) < p_node.getpakage())break;
				if(loc2 == tempsolution[each_car].size()){
					if(mark_order!="fuck")break;
				}else{
					if(mark_order == "fuck"){
						if(tempsolution[each_car][loc2].getbeforedeliverlist().size()!= 0){
							if(tempsolution[each_car][loc2].getpickup()){
								d_storage += tempsolution[each_car][loc2].getpakage();
							}else{
								d_storage -= tempsolution[each_car][loc2].getpakage();
							}
							if(d_storage_max<d_storage)d_storage_max = d_storage;
							continue;
						}
					}else if(tempsolution[each_car][loc2].getbeforedeliverlist().back() != mark_order){
						if(tempsolution[each_car][loc2].getpickup()){
							d_storage += tempsolution[each_car][loc2].getpakage();
						}else{
							d_storage -= tempsolution[each_car][loc2].getpakage();
						}
						if(d_storage_max<d_storage)d_storage_max = d_storage;
						continue;
					}
				}
				
				if(loc1 == tempsolution[each_car].size() - 1){
					int point1 = 0,point2 = 0,point3 = 0;
					point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
					point2 = order_pnode[p_node.getorderid()];
					point3 = order_dnode[d_node.getorderid()];
					/*if(eta_matrix[point1][point2] + eta_matrix[point2][point3] < 0){
						cout<<"1 fuck less 0"<<endl;
					}*/
					probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3]+ sigma),index1)*
					//1);
					//pow((tao_matrix[point1][point2] +tao_matrix[point2][point3]),index2));
					//pow(1/(1/tao_matrix[point1][point2] + 1/tao_matrix[point2][point3]),index2));
					pow(tao_matrix[point1][point2]*tao_matrix[point2][point3],index2));
					//pow(1/(tao_matrix[point1][point2] + tao_matrix[point2][point3]+ sigma),index2));
					/*probility[each].push_back(pow(eta_matrix[point1][point2]*eta_matrix[point2][point3],index1)*
					pow(tao_matrix[point1][point2]*tao_matrix[point2][point3],index2));*/
					pair<int,int> temp_pair;
					temp_pair.first = loc1;
					temp_pair.second = loc2;
					insert_loc[each].push_back(temp_pair);
					count_right++;
				}else if(loc2 == tempsolution[each_car].size()){
					int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
					if(tempsolution[each_car][loc1].getpickup()){
						point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
					}else{
						point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
					}
					point2 = order_pnode[p_node.getorderid()];
					if(tempsolution[each_car][loc1 + 1].getpickup()){
						point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
					}else{
						point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
					}
					
					point5 = order_dnode[d_node.getorderid()];
					if(tempsolution[each_car][loc2 - 1].getpickup()){
						point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
					}else{
						point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
					}
					/*if(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + eta_matrix[point4][point5] < 0)cout<<"2 fuck less 0"<<endl;*/
					probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + eta_matrix[point4][point5] + sigma),index1)*
					//1);
					//pow((tao_matrix[point1][point2]+tao_matrix[point2][point3]-tao_matrix[point1][point3])+(tao_matrix[point4][point5]),index2));
					//pow(1/abs(1/tao_matrix[point1][point2] + 1/tao_matrix[point2][point3] - 1/tao_matrix[point1][point3] + 1/tao_matrix[point4][point5]),index2));
					pow((tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3])*(tao_matrix[point4][point5]),index2));
					//pow(1/abs(tao_matrix[point1][point2] + tao_matrix[point2][point3] - tao_matrix[point1][point3] + tao_matrix[point4][point5]+ sigma),index2));
					/*probility[each].push_back(pow((eta_matrix[point1][point2]*eta_matrix[point2][point3]/eta_matrix[point1][point3])*(eta_matrix[point4][point5]),index1)*
					pow((tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3])*(tao_matrix[point4][point5]),index2));*/
					pair<int,int> temp_pair;
					temp_pair.first = loc1;
					temp_pair.second = loc2;
					insert_loc[each].push_back(temp_pair);
					count_right++;
				}else if(loc1 + 1 == loc2){
					int point1 = 0,point2 = 0,point3 = 0,point4 = 0;
					if(tempsolution[each_car][loc1].getpickup()){
						point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
					}else{
						point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
					}
					if(tempsolution[each_car][loc2].getpickup()){
						point4 = order_pnode[tempsolution[each_car][loc2].getorderid()];
					}else{
						point4 = order_dnode[tempsolution[each_car][loc2].getorderid()];
					}
					point2 = order_pnode[p_node.getorderid()];
					point3 = order_dnode[d_node.getorderid()];
					/*if(eta_matrix[point1][point2] + eta_matrix[point2][point3] + eta_matrix[point3][point4] - eta_matrix[point1][point4] < 0)cout<<"3 fuck less 0"<<endl;*/
					probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] + eta_matrix[point3][point4] - eta_matrix[point1][point4]+ sigma),index1)*
					//1);
					//pow(tao_matrix[point1][point2]+tao_matrix[point2][point3]+tao_matrix[point3][point4] - tao_matrix[point1][point4],index2));
					//pow(1/abs(1/tao_matrix[point1][point2] + 1/tao_matrix[point2][point3] + 1/tao_matrix[point3][point4] - 1/tao_matrix[point1][point4]),index2));
					pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]*tao_matrix[point3][point4] / tao_matrix[point1][point4],index2));
					//pow(1/abs(tao_matrix[point1][point2] + tao_matrix[point2][point3] + tao_matrix[point3][point4] - tao_matrix[point1][point4]+ sigma),index2));
					/*probility[each].push_back(pow(eta_matrix[point1][point2]*eta_matrix[point2][point3]*eta_matrix[point3][point4] / eta_matrix[point1][point4],index1)*
					pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]*tao_matrix[point3][point4] / tao_matrix[point1][point4],index2));*/
					pair<int,int> temp_pair;
					temp_pair.first = loc1;
					temp_pair.second = loc2;
					insert_loc[each].push_back(temp_pair);
					count_right++;
				}else{
					int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0,point6 = 0;
					if(tempsolution[each_car][loc1].getpickup()){
						point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
					}else{
						point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
					}
					point2 = order_pnode[p_node.getorderid()];
					if(tempsolution[each_car][loc1 + 1].getpickup()){
						point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
					}else{
						point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
					}
					if(tempsolution[each_car][loc2].getpickup()){
						point6 = order_pnode[tempsolution[each_car][loc2].getorderid()];
					}else{
						point6 = order_dnode[tempsolution[each_car][loc2].getorderid()];
					}
					point5 = order_dnode[d_node.getorderid()];
					if(tempsolution[each_car][loc2 - 1].getpickup()){
						point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
					}else{
						point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
					}
					/*if(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + eta_matrix[point4][point5] + eta_matrix[point5][point6] - eta_matrix[point4][point6] < 0 )cout<<"4 fuck less than 0"<<endl;*/
					probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + eta_matrix[point4][point5] + eta_matrix[point5][point6] - eta_matrix[point4][point6] + sigma),index1)*
					//1);
					//pow((tao_matrix[point1][point2]+tao_matrix[point2][point3]-tao_matrix[point1][point3])+(tao_matrix[point4][point5]+tao_matrix[point5][point6]-tao_matrix[point4][point6]),index2));
					//pow(1/abs(1/tao_matrix[point1][point2] + 1/tao_matrix[point2][point3] - 1/tao_matrix[point1][point3] + 1/tao_matrix[point4][point5] + 1/tao_matrix[point5][point6] - 1/tao_matrix[point4][point6]),index2));
					pow((tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3])*(tao_matrix[point4][point5]*tao_matrix[point5][point6]/tao_matrix[point4][point6]),index2));
					//pow(1/(tao_matrix[point1][point2] + tao_matrix[point2][point3] - tao_matrix[point1][point3] + tao_matrix[point4][point5] + tao_matrix[point5][point6] - tao_matrix[point4][point6]+ sigma),index2));
					/*probility[each].push_back(pow((eta_matrix[point1][point2]*eta_matrix[point2][point3]/eta_matrix[point1][point3])*(eta_matrix[point4][point5]*eta_matrix[point5][point6]/eta_matrix[point4][point6]),index1)*
					pow((tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3])*(tao_matrix[point4][point5]*tao_matrix[point5][point6]/tao_matrix[point4][point6]),index2));*/
					pair<int,int> temp_pair;
					temp_pair.first = loc1;
					temp_pair.second = loc2;
					insert_loc[each].push_back(temp_pair);
					count_right++;
				}
				if(loc2 == tempsolution[each_car].size())break;
				if(tempsolution[each_car][loc2].getorderid() == mark_order){
					break;
				}
				if(tempsolution[each_car][loc2].getpickup()){
					d_storage += tempsolution[each_car][loc2].getpakage();
				}else{
					d_storage -= tempsolution[each_car][loc2].getpakage();
				}
				if(d_storage_max<d_storage)d_storage_max = d_storage;
			}
		}
	}
	pair<int,int> best_place = selectElement(probility,q);
	int loc1 = insert_loc[best_place.first][best_place.second].first;
	int loc2 = insert_loc[best_place.first][best_place.second].second;
	string each_car = veichles[best_place.first];
	vector<int> temp_result;
	if(loc1 == tempsolution[veichles[best_place.first]].size() - 1){
		int point1 = 0,point2 = 0,point3 = 0;
		point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
	}else if(loc2 == tempsolution[veichles[best_place.first]].size()){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
	}else if(loc1 + 1 == loc2){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point3][point4] = (1 - local_update)*tao_matrix[point3][point4] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point3);temp_result.push_back(point4);
	}else{
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0,point6 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point6 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point6 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		tao_matrix[point5][point6] = (1 - local_update)*tao_matrix[point5][point6] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
		temp_result.push_back(point5);temp_result.push_back(point6);
	}
	insertBefore(tempsolution[veichles[best_place.first]],insert_loc[best_place.first][best_place.second].second,d_node);
	insertAfter(tempsolution[veichles[best_place.first]],insert_loc[best_place.first][best_place.second].first,p_node);
	PDlist(tempsolution[veichles[best_place.first]]);
	return temp_result;
}

vector<string> order_seqence_construct(vector<string>exist_order,unordered_map<string,int> order_loc,vector<vector<double>> &tao,vector<vector<double>> eta,double l,double local_update,double q
,double index1,double index2){
	vector<string> order_seqence;
	while(exist_order.size()!=0){
		vector<double> probility;
		for(string value:exist_order){
			vector<double> plist;
			if(order_seqence.size()!=0)
				plist.push_back(pow(eta[order_loc[order_seqence.back()]][order_loc[value]],index1)*pow(tao[order_loc[order_seqence.back()]][order_loc[value]],index2));
			else{
				vector<double> temp_p;
				for(int i = 0;i<veichle_num;i++)
					temp_p.push_back(pow(eta[order_loc[veichles[i]]][order_loc[value]],index1)*pow(tao[order_loc[veichles[i]]][order_loc[value]],index2));
				plist.push_back(findMaxValue(temp_p));
			}
			probility.push_back(findMaxValue(plist));
		}
		int value = selectElement1D(probility,q);
		string order_num = exist_order[value];
		if(order_seqence.size() == 0){
			for(int i = 0;i<veichle_num;i++)
				tao[i][order_loc[order_num]] = (1 - local_update)*tao[i][order_loc[order_num]] + local_update*l;
		}else{
			tao[order_loc[order_seqence.back()]][order_loc[order_num]] = (1 - local_update)*tao[order_loc[order_seqence.back()]][order_loc[order_num]] + local_update*l;
		}
		order_seqence.push_back(order_num);
		exist_order.erase(remove_if(exist_order.begin(),exist_order.end(),[order_num](string value){ return value == order_num; }),exist_order.end());
	}
	return order_seqence;
}

vector<node> orderHelper(unordered_map<int,int>& tree, int index, int is_left,unordered_map<string,int> order_pnode,
    unordered_map<string,int> order_dnode,vector<node> currentroute_nodelist,node s,vector<orderitem>&o) {
    // 如果当前索引越节点为空，直接返回
    vector<node> reslut;
    if (tree.count(index) == 0) {
        return reslut;
    }
    // 递归处理左子节点（标记为左）
    vector<node> left_nodelist = orderHelper(tree, 2 * index + 1, 1,order_pnode,order_dnode,currentroute_nodelist,s,o);
    
    if(is_left>1){
        if(left_nodelist.size()!=0)cout<<"fuck!!!!!!!构建失误"<<endl;
        left_nodelist.push_back(s);
    }else{
        left_nodelist.emplace(left_nodelist.begin(),currentroute_nodelist[order_pnode[o[tree[index]].getOrderId()]]);
        left_nodelist.push_back(currentroute_nodelist[order_dnode[o[tree[index]].getOrderId()]]);
    }
    
    vector<node> right_nodelist = orderHelper(tree, 2 * index + 2, 0,order_pnode,order_dnode,currentroute_nodelist,s,o);
    
    reslut = left_nodelist;
    reslut.insert(reslut.end(),right_nodelist.begin(),right_nodelist.end());
    return reslut;
}

vector<node> orderHelper(TreeNode* tree,int is_left,unordered_map<string,int> order_pnode,
    unordered_map<string,int> order_dnode,vector<node> currentroute_nodelist,node s,vector<orderitem>&o) {
    // 如果当前索引越节点为空，直接返回
    vector<node> reslut;
    if (tree == NULL) {
        return reslut;
    }
    // 递归处理左子节点（标记为左）
    vector<node> left_nodelist = orderHelper(tree->left,1,order_pnode,order_dnode,currentroute_nodelist,s,o);
    
    if(is_left>1){
        if(left_nodelist.size()!=0)cout<<"fuck!!!!!!!构建失误"<<endl;
        left_nodelist.push_back(s);
    }else{
        left_nodelist.emplace(left_nodelist.begin(),currentroute_nodelist[order_pnode[tree->order]]);
        left_nodelist.push_back(currentroute_nodelist[order_dnode[tree->order]]);
    }
    
    vector<node> right_nodelist = orderHelper(tree->right,0,order_pnode,order_dnode,currentroute_nodelist,s,o);
    
    reslut = left_nodelist;
    reslut.insert(reslut.end(),right_nodelist.begin(),right_nodelist.end());
    return reslut;
}

vector<int> tree_construct(BinaryTree&t,vector<vector<double>> &tao_matrix,vector<vector<double>> eta_matrix,
	string order_num,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double demand,double tao,double local_update){
		vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
		vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
		vector<vector<double>> p;
		vector<double> p1,p2;
		for(TreeNode*n:loc1){
			if(demand>n->storage){
				p1.push_back(0);continue;
			}
			p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index1)*
			pow((tao_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index2));
		}
		p.push_back(p1);
		for(TreeNode*n:loc2){
			if(demand>n->storage + n->demand){
				p2.push_back(0);continue;
			}
			if(n->leftparent != "fuck"){
				p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index1)*
				pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index2));
			}else{
				p2.push_back(pow(eta_matrix[order_dnode[n->order]][order_pnode[order_num]],index1)*pow(tao_matrix[order_dnode[n->order]][order_pnode[order_num]],index2));
			}
		}
		p.push_back(p2);
		vector<int> pair_list;
		pair<int,int> temp = selectElement(p,q);
		if(temp.first == 0){
			tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
			tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] + local_update*tao;
			pair_list.push_back(order_pnode[loc1[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc1[temp.second]->order]);
		}else{
			if(loc2[temp.second]->leftparent != "fuck"){
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
				pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc2[temp.second]->leftparent]);
			}else{
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			}
		}
		if(temp.first == 0){ 	//左索引
			t.insert(loc1[temp.second],1,order_num,demand);
		}else{
			t.insert(loc2[temp.second],0,order_num,demand);
		}
	return pair_list;		
}

vector<int> tree_construct(BinaryTree&t,vector<vector<double>> &tao_matrix,vector<vector<double>> eta_matrix,
	vector<string> to_insert,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double demand,double tao,double local_update){
		vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
		vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
		vector<vector<vector<double>>> d3p;
		for(string order_num:to_insert){
		vector<vector<double>> p;
		vector<double> p1,p2;
		for(TreeNode*n:loc1){
			if(demand>n->storage){
				p1.push_back(0);continue;
			}
			p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index1)*
			pow((tao_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index2));
		}
		p.push_back(p1);
		for(TreeNode*n:loc2){
			if(demand>n->storage + n->demand){
				p2.push_back(0);continue;
			}
			if(n->leftparent != "fuck"){
				p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index1)*
				pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index2));
			}else{
				p2.push_back(pow(eta_matrix[order_dnode[n->order]][order_pnode[order_num]],index1)*pow(tao_matrix[order_dnode[n->order]][order_pnode[order_num]],index2));
			}
		}
		p.push_back(p2);
		}
		vector<int> pair_list;
		tuple<int,int,int> tri = selectElement3D(d3p,q);
		int first = get<0>(tri);
		int second = get<1>(tri);
		int third = get<2>(tri);
		string order_num = to_insert[first];
		pair<int,int> temp;temp.first = second;temp.second = third;
		if(temp.first == 0){
			tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
			tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] + local_update*tao;
			pair_list.push_back(order_pnode[loc1[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc1[temp.second]->order]);
		}else{
			if(loc2[temp.second]->leftparent != "fuck"){
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
				pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc2[temp.second]->leftparent]);
			}else{
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			}
		}
		if(temp.first == 0){ 	//左索引
			t.insert(loc1[temp.second],1,order_num,demand);
		}else{
			t.insert(loc2[temp.second],0,order_num,demand);
		}
	return pair_list;		
}

vector<int> tree_construct(unordered_map<string,BinaryTree>&union_t,vector<vector<double>> &tao_matrix,vector<vector<double>> eta_matrix,
	vector<string>& to_insert,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double tao,double local_update,vector<orderitem> ors,unordered_map<string,int> order_loc,double beta2){
		vector<vector<TreeNode*>> loc1_union(veichle_num);
		vector<vector<TreeNode*>> loc2_union(veichle_num);
		vector<vector<vector<vector<double>>>> d4p;
		for(int name = 0;name<veichle_num;name++){
			BinaryTree t = union_t[veichles[name]];
			vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
			vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
			loc1_union[name] = loc1;
			loc2_union[name] = loc2;
			vector<vector<vector<double>>> d3p;
			for(string order_num:to_insert){
			vector<vector<double>> p;
			vector<double> p1,p2;
			double demand = ors[order_loc[order_num] - veichle_num].getDemand();
			for(TreeNode*n:loc1){
				if(demand>(n->storage)){
					p1.push_back(0);continue;
				}
				p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index1)*
				pow((tao_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[n->order]][order_dnode[order_num]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index2));
			}
			p.push_back(p1);
			//cout<<p1.size()<<endl;
			for(TreeNode*n:loc2){
				if(demand>(n->storage + n->demand)){
					p2.push_back(0);continue;
				}
				if(n->leftparent != "fuck"){
					p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index1)*
					pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[order_num]][order_dnode[n->leftparent]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index2));
				}else{
					p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta2),index1)*pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta2),index2));
				}
			}
			//cout<<p2.size()<<endl;
			p.push_back(p2);
			d3p.push_back(p);
			}
			d4p.push_back(d3p);
		}
		
		vector<int> pair_list;
		tuple<int,int,int,int> tri = selectElement4D(d4p,q);
		int first = get<0>(tri); // car
		int second = get<1>(tri); // order
		int third = get<2>(tri); // 2
		int forth = get<3>(tri); // insert place
		double demand = ors[order_loc[to_insert[second]] - veichle_num].getDemand();
		string order_num = to_insert[second];
		pair<int,int> temp;temp.first = third;temp.second = forth;
		if(temp.first == 0){	//左索引
			vector<TreeNode*>loc1 = loc1_union[first];
			tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
			tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] + local_update*tao;
			pair_list.push_back(order_pnode[loc1[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc1[temp.second]->order]);
			union_t[veichles[first]].insert(loc1[temp.second],1,order_num,demand);
		}else{
			vector<TreeNode*>loc2 = loc2_union[first];
			if(loc2[temp.second]->leftparent != "fuck"){
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
				pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc2[temp.second]->leftparent]);
			}else{
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			}
			union_t[veichles[first]].insert(loc2[temp.second],0,order_num,demand);
		}
		if (second < to_insert.size()) to_insert.erase(to_insert.begin() + second);
	return pair_list;		
}

vector<int> tree_construct(unordered_map<string,BinaryTree>&union_t,vector<vector<double>> &tao_matrix,vector<vector<double>> eta_matrix,
	string order_num,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double tao,double local_update,vector<orderitem> ors,unordered_map<string,int> order_loc,double beta2){
		double sigma = double(veichle_num)*0.1;
		vector<string> to_insert;
		to_insert.push_back(order_num);
		vector<vector<TreeNode*>> loc1_union(veichle_num);
		vector<vector<TreeNode*>> loc2_union(veichle_num);
		vector<vector<vector<vector<double>>>> d4p;
		for(int name = 0;name<veichle_num;name++){
			BinaryTree t = union_t[veichles[name]];
			vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
			vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
			loc1_union[name] = loc1;
			loc2_union[name] = loc2;
			vector<vector<vector<double>>> d3p;
			vector<vector<double>> p;
			vector<double> p1,p2;
			double demand = ors[order_loc[order_num] - veichle_num].getDemand();
			for(TreeNode*n:loc1){
				if(demand>(n->storage)){
					p1.push_back(0);continue;
				}
				int point1 = order_pnode[n->order];int point2 = order_pnode[order_num];int point3 = order_dnode[order_num];int point4 = order_dnode[n->order];
				/*p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index1)*
				pow((tao_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[n->order]][order_dnode[order_num]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index2));*/
				p1.push_back(pow(1/(sigma+eta_matrix[point1][point2] + eta_matrix[point2][point3] + eta_matrix[point3][point4] - eta_matrix[point1][point4]),index1)*pow((tao_matrix[point1][point2]*tao_matrix[point2][point3])/tao_matrix[point1][point4],index2));
			}
			p.push_back(p1);
			//cout<<p1.size()<<endl;
			for(TreeNode*n:loc2){
				if(demand>(n->storage + n->demand)){
					p2.push_back(0);continue;
				}
				if(n->leftparent != "fuck"){
					int point1 = order_dnode[n->order];int point2 = order_pnode[order_num];int point3 = order_dnode[order_num];int point4 = order_dnode[n->leftparent];
					p2.push_back(pow(1/(sigma + eta_matrix[point1][point2] + eta_matrix[point2][point3] + eta_matrix[point3][point4] - eta_matrix[point1][point4]),index1)*pow((tao_matrix[point1][point2]*tao_matrix[point2][point3])/tao_matrix[point1][point4],index2));
					/*p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index1)*
					pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*tao_matrix[order_dnode[order_num]][order_dnode[n->leftparent]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta + beta2),index2));*/
				}else{
					int point1 = order_dnode[n->order];int point2 = order_pnode[order_num];int point3 = order_dnode[order_num];
					p2.push_back(pow(1/(sigma + eta_matrix[point1][point2] + eta_matrix[point2][point3]),index1)*pow(tao_matrix[point1][point2],index2));
					//p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta2*eta_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta2),index1)*pow((tao_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta2*tao_matrix[order_pnode[order_num]][order_dnode[order_num]])/(1 + beta2),index2));
				}
			}
			//cout<<p2.size()<<endl;
			p.push_back(p2);
			d3p.push_back(p);
			d4p.push_back(d3p);
		}
		
		vector<int> pair_list;
		tuple<int,int,int,int> tri = selectElement4D(d4p,q);
		int first = get<0>(tri); // car
		int second = get<1>(tri); // order
		int third = get<2>(tri); // 2
		int forth = get<3>(tri); // insert place
		double demand = ors[order_loc[order_num] - veichle_num].getDemand();
		pair<int,int> temp;temp.first = third;temp.second = forth;
		if(temp.first == 0){	//左索引
			vector<TreeNode*>loc1 = loc1_union[first];
			tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_pnode[loc1[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
			tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc1[temp.second]->order]] + local_update*tao;
			pair_list.push_back(order_pnode[loc1[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc1[temp.second]->order]);
			union_t[veichles[first]].insert(loc1[temp.second],1,order_num,demand);
		}else{
			vector<TreeNode*>loc2 = loc2_union[first];
			if(loc2[temp.second]->leftparent != "fuck"){
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] = (1 - local_update)*tao_matrix[order_dnode[order_num]][order_dnode[loc2[temp.second]->leftparent]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
				pair_list.push_back(order_dnode[order_num]);pair_list.push_back(order_dnode[loc2[temp.second]->leftparent]);
			}else{
				tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] = (1 - local_update)*tao_matrix[order_dnode[loc2[temp.second]->order]][order_pnode[order_num]] + local_update*tao;
				pair_list.push_back(order_dnode[loc2[temp.second]->order]);pair_list.push_back(order_pnode[order_num]);
			}
			union_t[veichles[first]].insert(loc2[temp.second],0,order_num,demand);
		}
	return pair_list;		
}

vector<int> tree_construct(BinaryTree&t,vector<vector<vector<double>>> &tao_matrix,vector<vector<double>> eta_matrix,
	string order_num,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double demand,double tao,double local_update,unordered_map<string,int> order_loc,double beta2){
		vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
		vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
		vector<vector<double>> p;
		vector<double> p1,p2;
		for(TreeNode*n:loc1){
			if(demand>n->storage){
				p1.push_back(0);continue;
			}
			p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index1)*
			pow(tao_matrix[0][order_loc[n->order]][order_loc[order_num]],index2));
		}
		p.push_back(p1);
		for(TreeNode*n:loc2){
			if(demand>n->storage + n->demand){
				p2.push_back(0);continue;
			}
			if(n->leftparent != "fuck"){
				p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index1)*
				pow((tao_matrix[1][order_loc[n->order]][order_loc[order_num]] + beta2 * tao_matrix[0][order_loc[n->leftparent]][order_loc[order_num]])/(1 + beta2),index2));
			}else{
				p2.push_back(pow(eta_matrix[order_dnode[n->order]][order_pnode[order_num]],index1)*pow(tao_matrix[1][order_loc[n->order]][order_loc[order_num]],index2));
			}
		}
		p.push_back(p2);
		vector<int> pair_list;
		pair<int,int> temp = selectElement(p,q);
		if(temp.first == 0){
			tao_matrix[0][order_loc[loc1[temp.second]->order]][order_loc[order_num]] = (1 - local_update)*tao_matrix[0][order_loc[loc1[temp.second]->order]][order_loc[order_num]] + local_update*tao;
		}else{
			tao_matrix[1][order_loc[loc2[temp.second]->order]][order_loc[order_num]] = (1 - local_update)*tao_matrix[1][order_loc[loc2[temp.second]->order]][order_loc[order_num]] + local_update*tao;
		}
		if(temp.first == 0){ 	//左索引
			t.insert(loc1[temp.second],1,order_num,demand);
		}else{
			t.insert(loc2[temp.second],0,order_num,demand);
		}
	return pair_list;		
}

vector<int> tree_construct(BinaryTree&t,vector<vector<vector<double>>> &tao_matrix,vector<vector<double>> eta_matrix,
	vector<string> order_list,vector<node> nodelist,unordered_map<string,int> order_pnode,unordered_map<string,int> order_dnode
	,double q,double beta,double index1,double index2,double demand,double tao,double local_update,unordered_map<string,int> order_loc,double beta2){
		vector<TreeNode*> loc1 = t.getNodesWithAvailableLeft();
		vector<TreeNode*> loc2 = t.getNodesWithAvailableRight();
		vector<vector<double>> p;
		vector<double> p1,p2;
	for(string order_num:order_list){
		for(TreeNode*n:loc1){
			if(demand>n->storage){
				p1.push_back(0);continue;
			}
			p1.push_back(pow((eta_matrix[order_pnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[n->order]][order_dnode[order_num]])/(1 + beta),index1)*
			pow(tao_matrix[0][order_loc[n->order]][order_loc[order_num]],index2));
		}
		p.push_back(p1);
		for(TreeNode*n:loc2){
			if(demand>n->storage + n->demand){
				p2.push_back(0);continue;
			}
			if(n->leftparent != "fuck"){
				p2.push_back(pow((eta_matrix[order_dnode[n->order]][order_pnode[order_num]] + beta*eta_matrix[order_dnode[order_num]][order_dnode[n->leftparent]])/(1 + beta),index1)*
				pow((tao_matrix[1][order_loc[n->order]][order_loc[order_num]] + beta2 * tao_matrix[0][order_loc[n->leftparent]][order_loc[order_num]])/(1 + beta2),index2));
			}else{
				p2.push_back(pow(eta_matrix[order_dnode[n->order]][order_pnode[order_num]],index1)*pow(tao_matrix[1][order_loc[n->order]][order_loc[order_num]],index2));
			}
		}
		p.push_back(p2);
	}
		vector<int> pair_list;
		pair<int,int> temp = selectElement(p,q);
		string order_num;
		if(temp.first == 0){
			tao_matrix[0][order_loc[loc1[temp.second]->order]][order_loc[order_num]] = (1 - local_update)*tao_matrix[0][order_loc[loc1[temp.second]->order]][order_loc[order_num]] + local_update*tao;
		}else{
			tao_matrix[1][order_loc[loc2[temp.second]->order]][order_loc[order_num]] = (1 - local_update)*tao_matrix[1][order_loc[loc2[temp.second]->order]][order_loc[order_num]] + local_update*tao;
		}
		
		if(temp.first == 0){ 	//左索引
			t.insert(loc1[temp.second],1,order_num,demand);
		}else{
			t.insert(loc2[temp.second],0,order_num,demand);
		}
	return pair_list;
}

unordered_map<int,int> treehelper(vector<node>&nodelist,unordered_map<string,int> &loc){ // 代表订单在序列当中的位置
    unordered_map<int,int> result;
    unordered_map<int,int> t;
    node last_node = nodelist.front();
    for(int i = 1 ;i<nodelist.size();i++){
        if(i == 1){
            result[2] = loc[nodelist[i].getorderid()];
            t[loc[nodelist[i].getorderid()]] = 2;
            last_node = nodelist[i];
            continue;
        }
        if(nodelist[i].getpickup()){
            if(last_node.getpickup()){
                int n = 2*t[loc[last_node.getorderid()]] + 1;
                result[2*t[loc[last_node.getorderid()]] + 1] = loc[nodelist[i].getorderid()];
                t[loc[nodelist[i].getorderid()]] = n;
                last_node = nodelist[i];
            }else{
                int n = 2*t[loc[last_node.getorderid()]] + 2;
                result[2*t[loc[last_node.getorderid()]] + 2] = loc[nodelist[i].getorderid()];
                t[loc[nodelist[i].getorderid()]] = n;
                last_node = nodelist[i];
            }
        }else{
            last_node = nodelist[i];
        }
    }
    return result;
}

int getParentIndex(int childIndex) {
    if (childIndex <= 0) {
        return -1; // 根节点没有父    
    }
    return (childIndex - 1) / 2; // 整数除法自动向下取整
}

std::pair<int, bool> getParentAndChildSide(int childIndex) {
    if (childIndex <= 0) {
        return {-1, false}; // 根节点无父节点，bool 无意义
    }
    int parentIndex = floor((childIndex - 1) / 2);
    int leftChild = 2 * parentIndex + 1;
    bool isLeft = (childIndex == leftChild);
    return {parentIndex, isLeft};
}

void global_update(unordered_map<int,int> &tree,vector<vector<vector<double>>>&m,double t,int index,double rate){
    if(tree.count(index) == 0)return;

    if(tree.count(2*index + 1) == 1){
        m[0][veichle_num + tree[index]][tree[2*index + 1]] = (1 - rate)*m[0][veichle_num + tree[index]][tree[2*index + 1]] + rate*t;
        global_update(tree,m,t,2*index + 1,rate);
    }

    if(tree.count(2*index + 2) == 1){
        m[1][veichle_num + tree[index]][tree[2*index + 2]] = (1 - rate)*m[1][veichle_num + tree[index]][tree[2*index + 2]] + rate*t;
        global_update(tree,m,t,2*index + 2,rate);
    }

}

void global_update(TreeNode* tree,vector<vector<vector<double>>>&tao_matrix,unordered_map<string,int> order_loc,double rate,double c,double b){
	if(tree == NULL)return;
	if(tree->left != NULL){
		tao_matrix[0][order_loc[tree->order]][order_loc[tree->left->order]] = (1 - rate)*tao_matrix[0][order_loc[tree->order]][order_loc[tree->left->order]]
		+ rate*c;
		global_update(tree->left,tao_matrix,order_loc,rate,c,b);
	}
	if(tree->right != NULL){
		tao_matrix[1][order_loc[tree->order]][order_loc[tree->right->order]] = (1 - rate)*tao_matrix[1][order_loc[tree->order]][order_loc[tree->right->order]]
		+ rate*c;
		if(tree->leftparent!="fuck"){
			tao_matrix[0][order_loc[tree->leftparent]][order_loc[tree->order]] = (1 - b*rate)*tao_matrix[0][order_loc[tree->leftparent]][order_loc[tree->order]]
			+ b*rate*c;
		}
		global_update(tree->right,tao_matrix,order_loc,rate,c,b);
	}
	
}

void global_update_tree(vector<unordered_map<int,int>> &tree,vector<vector<vector<double>>>&m,double t,double rate){
    for(int i = 0;i<veichle_num;i++){
        global_update(tree[i],m,t,0,rate);
    }
}

void printTreeHelper(unordered_map<int,int>& tree, int index, const string& prefix, bool isLeft, bool isRoot) {
    if (index >= tree.size() || tree[index] == -1) return;

    // 打印当前节点
    cout << prefix;
    if (!isRoot) {
        cout << (isLeft ? "├──L:" : "└──R:"); // 非根节点添加连接符号
    } else {
        cout << "Root:"; // 根节点特殊标记
    }
    cout << tree[index] << endl;

    // 计算子节点是否存在
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;
    bool hasLeft = (leftChild < tree.size()) && (tree[leftChild] != -1);
    bool hasRight = (rightChild < tree.size()) && (tree[rightChild] != -1);

    // 构建新的前缀（缩进和连接线）
    string newPrefix = prefix + (isRoot ? "" : (isLeft ? "│   " : "    "));

    // 递归打印子节点
    if (hasLeft) {
        printTreeHelper(tree, leftChild, newPrefix, true, false);
    }
    if (hasRight) {
        printTreeHelper(tree, rightChild, newPrefix, false, false);
    }
}

// 主函数调用入口
void printTree(unordered_map<int,int>& tree) {
    if (tree.empty()) {
        cout << "Empty tree!" << endl;
        return;
    }
    printTreeHelper(tree, 0, "", false, true); // 从根节点开始
}

pair<bool,int> find_left(unordered_map<int,int> tree,int loc){
	int n = loc;
	while(n%2 == 0&&n!=0){
		n = getParentIndex(n);
	}
	if(n == 0){
		pair<bool,int> r = {0,1};
		return r;
	}
	pair<bool,int> r = {1,getParentIndex(n)};
	return r;
}

// 用蚁群算法对插入顺序进行优化，得到一个插入的顺序
// 用信息素对插入顺序进行优化，同时利用图结构的插入进行优化
// 单车辆，构建二叉树，后序遍历构建路径，静态PDP问题
// 向左延展代表了包含，向右延展代表了向后邻接
// 按照pick up node的顺序进行插入的排列
#if 1
int main(){
	int whole_iteration = 200;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.3;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.8;
    double test_q2 = 0.8;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	double beta = 0.1;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 1;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;unordered_map<string,vector<node>> greedy_solution;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_matrix(currentroute_nodelist);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist,greedy_solution);
	cout<<1/order_tao<<endl;
	//order_tao = 1.0/250.0;
	vector<vector<double>> tao_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
    vector<vector<double>> order_tao_matrix(unfinishorderlist.size(),vector<double>(unfinishorderlist.size(),order_tao));
    double tao = order_tao;
    vector<int> best_orderlist;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> last_point(veichle_num);
			for(int i = 0;i<veichle_num;i++)
				last_point[i].push_back(i);
			vector<int> exist_order(unfinishorderlist.size());
			iota(exist_order.begin(),exist_order.end(),0);
			unordered_map<string,BinaryTree> local_tree;
			for(string t:veichles){
				local_tree[t].initRoot(t,capacity);
			}
			vector<vector<int>> update_list;
			while(exist_order.size()!=0){
				int order_num;string car;
				vector<vector<double>> p1(veichle_num,vector<double>(exist_order.size(),0));
				for(int c = 0;c<veichle_num;c++){
					for(int t = 0;t<exist_order.size();t++){
						p1[c][t] = pow(order_eta_matrix[exist_order[t]][last_point[c].back()],eta_index1)*pow(order_tao_matrix[exist_order[t]][last_point[c].back()],tao_index1);
					}
				}
				pair<int,int> t = selectElement(p1,test_q1);
				order_num = exist_order[t.second];
				car = veichles[t.first];
				order_tao_matrix[order_num][last_point[t.first].back()] = (1 - local_update_car)*order_tao_matrix[order_num][last_point[t.first].back()] + local_update_car*order_tao;
				last_point[t.first].push_back(order_num);
				vector<int> temp_pair = tree_construct(local_tree[car],tao_relation,node_relation,unfinishorderlist[order_num].getOrderId(),currentroute_nodelist,
				order_pnode,order_dnode,test_q2,beta,eta_index2,tao_index2,unfinishorderlist[order_num].getDemand(),tao,local_update_car);
				update_list.push_back(temp_pair);
				exist_order.erase(remove_if(exist_order.begin(),exist_order.end(),[order_num](int value){ return value == order_num; }),exist_order.end());
			}

			for(string each_car:veichles){
				TreeNode*temp_node = local_tree[each_car].getRoot();
				node startnode = startsolution[each_car].front();
				local_solution[each_car] = orderHelper(temp_node,2,order_pnode,order_dnode,currentroute_nodelist,startnode,unfinishorderlist);
			}

            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				local_bestpair = update_list;
				local_bestorder = last_point;
			}
			/*for(int i = 0;i<veichle_num;i++){
				int last_node = i;
				for(int j = 1;j<local_solution[veichles[i]].size();j++){
					if(local_solution[veichles[i]][j].getpickup()){
						tao_relation[last_node][order_pnode[local_solution[veichles[i]][j].getorderid()]] = (1 - local_update_car)*tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_update_car*tao;
						last_node = order_pnode[local_solution[veichles[i]][j].getorderid()];
					}else{
						tao_relation[last_node][order_dnode[local_solution[veichles[i]][j].getorderid()]] = (1 - local_update_car)*tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_update_car*tao;
						last_node = order_dnode[local_solution[veichles[i]][j].getorderid()];
					}
				}
			}*/
		}
        // global update
        
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<endl;
		for(int i = 0;i<veichle_num;i++){

			for(int j = 0;j<local_bestorder[i].size();j++){
				cout<<local_bestorder[i][j]<<" ";
			}
			cout<<endl;
		}
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
        }

    	for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}

		/*for(int i = 0;i<local_bestpair.size();i++){
			for(int j = 0;j<local_bestpair[i].size();j = j + 2){
				tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] = (1 - global_update_car)*tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] + global_update_car/local_best_cost3;
			}
		}*/
		for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<local_bestorder[i].size() - 1;g++){
				order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}
		cost_num.push_back(global_best_cost);
    }
	//mergesolution(solution,best_currentsolution);
	solution = best_currentsolution;
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	//printfactory(solution,"fuck");
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv",instance);
}
}
#endif//no

// 尝试订单和车辆同时进行选择，采用时间复杂度较大的哪个方法 n^3
#if 0
int main(){
	int whole_iteration = 200;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.3;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.8;
    double test_q2 = 0.8;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	double beta = 0.1;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 1;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_matrix(currentroute_nodelist);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	cout<<1/order_tao<<endl;
	order_tao = 1.0/300.0;
	vector<vector<double>> tao_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
    vector<vector<double>> order_tao_matrix(unfinishorderlist.size(),vector<double>(unfinishorderlist.size(),order_tao));
    double tao = order_tao;
    vector<int> best_orderlist;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> last_point(veichle_num);
			for(int i = 0;i<veichle_num;i++)
				last_point[i].push_back(i);
			vector<int> exist_order(unfinishorderlist.size());
			iota(exist_order.begin(),exist_order.end(),0);
			
			unordered_map<string,BinaryTree> local_tree;
			for(string t:veichles){
				local_tree[t].initRoot(t,capacity);
			}
			vector<vector<int>> update_list;
			for(int o = 0;o<exist_order.size();o++){
				int order_num;string car;
				vector<vector<double>> p1(veichle_num,vector<double>(exist_order.size(),0));
				for(int c = 0;c<veichle_num;c++){
					for(int t = 0;t<exist_order.size();t++){
						p1[c][t] = pow(order_eta_matrix[exist_order[t]][last_point[c].back()],eta_index1)*pow(order_tao_matrix[exist_order[t]][last_point[c].back()],tao_index1);
					}
				}
				pair<int,int> t = selectElement(p1,test_q1);
				order_num = exist_order[o];
				car = veichles[t.first];
				order_tao_matrix[order_num][last_point[t.first].back()] = (1 - local_update_car)*order_tao_matrix[order_num][last_point[t.first].back()] + local_update_car*order_tao;
				last_point[t.first].push_back(order_num);
				vector<int> temp_pair = tree_construct(local_tree[car],tao_relation,node_relation,unfinishorderlist[order_num].getOrderId(),currentroute_nodelist,
				order_pnode,order_dnode,test_q2,beta,eta_index2,tao_index2,unfinishorderlist[order_num].getDemand(),tao,local_update_car);
				update_list.push_back(temp_pair);
				//exist_order.erase(remove_if(exist_order.begin(),exist_order.end(),[order_num](int value){ return value == order_num; }),exist_order.end());
			}

			for(string each_car:veichles){
				TreeNode*temp_node = local_tree[each_car].getRoot();
				node startnode = startsolution[each_car].front();
				local_solution[each_car] = orderHelper(temp_node,2,order_pnode,order_dnode,currentroute_nodelist,startnode,unfinishorderlist);
			}

            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				local_bestpair = update_list;
				local_bestorder = last_point;
			}
			/*for(int i = 0;i<veichle_num;i++){
				int last_node = i;
				for(int j = 1;j<local_solution[veichles[i]].size();j++){
					if(local_solution[veichles[i]][j].getpickup()){
						tao_relation[last_node][order_pnode[local_solution[veichles[i]][j].getorderid()]] = (1 - local_update_car)*tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_update_car*tao;
						last_node = order_pnode[local_solution[veichles[i]][j].getorderid()];
					}else{
						tao_relation[last_node][order_dnode[local_solution[veichles[i]][j].getorderid()]] = (1 - local_update_car)*tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_update_car*tao;
						last_node = order_dnode[local_solution[veichles[i]][j].getorderid()];
					}
				}
			}*/
		}
        // global update
        
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<endl;
		for(int i = 0;i<veichle_num;i++){

			for(int j = 0;j<local_bestorder[i].size();j++){
				cout<<local_bestorder[i][j]<<" ";
			}
			cout<<endl;
		}
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
        }

    	for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}

		/*for(int i = 0;i<local_bestpair.size();i++){
			for(int j = 0;j<local_bestpair[i].size();j = j + 2){
				tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] = (1 - global_update_car)*tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] + global_update_car/local_best_cost3;
			}
		}*/
		for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<local_bestorder[i].size() - 1;g++){
				order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}
		cost_num.push_back(global_best_cost);
    }
	//mergesolution(solution,best_currentsolution);
	solution = best_currentsolution;
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	//printfactory(solution,"fuck");
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif//no

#if 0
int main(){
	int whole_iteration = 200;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.2;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.8;
    double test_q2 = 0.8;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	double beta = 0.1;
	double beta2 = 0.6;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 1;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_matrix(currentroute_nodelist);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	cout<<1/order_tao<<endl;
	vector<vector<double>> order_tao_matrix(unfinishorderlist.size(),vector<double>(unfinishorderlist.size(),order_tao));
	vector<vector<vector<double>>> tao_matrix(2,vector<vector<double>>(unfinishorderlist.size() + veichle_num,vector<double>(unfinishorderlist.size() + veichle_num,order_tao)));
	//order_tao/=(double)unfinishorderlist.size();
	double tao = order_tao;
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
	//tao /= unfinishorderlist.size();order_tao = tao;
    unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
    vector<int> best_orderlist;unordered_map<string,BinaryTree> global_best_tree;vector<vector<int>> global_bestorder;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;unordered_map<string,BinaryTree> local_best_tree;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> last_point(veichle_num);
			for(int i = 0;i<veichle_num;i++)
				last_point[i].push_back(i);
			vector<int> exist_order(unfinishorderlist.size());
			iota(exist_order.begin(),exist_order.end(),0);
			unordered_map<string,BinaryTree> local_tree;
			for(string t:veichles){
				local_tree[t].initRoot(t,capacity);
			}
			vector<vector<int>> update_list;
			for(int o = 0;o<exist_order.size();o++){
				int order_num;string car;
				vector<vector<double>> p1(veichle_num,vector<double>(exist_order.size(),0));
				for(int c = 0;c<veichle_num;c++){
					for(int t = 0;t<exist_order.size();t++){
						p1[c][t] = pow(order_eta_matrix[exist_order[t]][last_point[c].back()],eta_index1)*pow(order_tao_matrix[exist_order[t]][last_point[c].back()],tao_index1);
					}
				}
				pair<int,int> t = selectElement(p1,test_q1);
				order_num = exist_order[o];
				//order_num = exist_order.front();
				car = veichles[t.first];
				order_tao_matrix[order_num][last_point[t.first].back()] = (1 - local_update_car)*order_tao_matrix[order_num][last_point[t.first].back()] + local_update_car*order_tao;
				last_point[t.first].push_back(order_num);
				vector<int> temp_pair = tree_construct(local_tree[car],tao_matrix,node_relation,unfinishorderlist[order_num].getOrderId(),currentroute_nodelist,
				order_pnode,order_dnode,test_q2,beta,eta_index2,tao_index2,unfinishorderlist[order_num].getDemand(),tao,local_update_car,order_loc,beta2);
				update_list.push_back(temp_pair);
				//exist_order.erase(remove_if(exist_order.begin(),exist_order.end(),[order_num](int value){ return value == order_num; }),exist_order.end());
			}

			for(string each_car:veichles){
				TreeNode*temp_node = local_tree[each_car].getRoot();
				node startnode = startsolution[each_car].front();
				local_solution[each_car] = orderHelper(temp_node,2,order_pnode,order_dnode,currentroute_nodelist,startnode,unfinishorderlist);
			}

            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				local_bestpair = update_list;
				local_bestorder = last_point;
				local_best_tree = local_tree;
			}
		}
        // global update
        
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_best_tree = local_best_tree;
			global_bestorder = local_bestorder;
        }
        
		if(better)
		for(string each_car:veichles)
			global_update(local_best_tree[each_car].getRoot(),tao_matrix,order_loc,global_update_car,1.0/local_best_cost3,beta2);

		/*for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<global_bestorder[i].size() - 1;g++){
				order_tao_matrix[global_bestorder[i][g + 1]][global_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[global_bestorder[i][g + 1]][global_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}*/
		if(better){
		for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<local_bestorder[i].size() - 1;g++){
				order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}
		}	
		cost_num.push_back(global_best_cost);
    }
	//mergesolution(solution,best_currentsolution);
	solution = best_currentsolution;
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	//printfactory(solution,"fuck");
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif//no

// 信息素是树节点之间的
#if 0
int main(){
	int whole_iteration = 200;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.2;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.8;
    double test_q2 = 0.8;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	double beta = 0.1;
	double beta2 = 0.6;
	readtime(); readistance(); readfactory();
	for(int instance = 2;instance <= 2;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_matrix(currentroute_nodelist);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	cout<<1/order_tao<<endl;
	vector<vector<double>> order_tao_matrix(unfinishorderlist.size(),vector<double>(unfinishorderlist.size(),order_tao));
	vector<vector<vector<double>>> tao_matrix(2,vector<vector<double>>(unfinishorderlist.size() + veichle_num,vector<double>(unfinishorderlist.size() + veichle_num,order_tao)));
	//order_tao/=(double)unfinishorderlist.size();
	double tao = order_tao;
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
	//tao /= unfinishorderlist.size();order_tao = tao;
    unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
    vector<int> best_orderlist;unordered_map<string,BinaryTree> global_best_tree;vector<vector<int>> global_bestorder;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;unordered_map<string,BinaryTree> local_best_tree;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> last_point(veichle_num);
			for(int i = 0;i<veichle_num;i++)
				last_point[i].push_back(i);
			vector<int> exist_order(unfinishorderlist.size());
			iota(exist_order.begin(),exist_order.end(),0);
			unordered_map<string,BinaryTree> local_tree;
			for(string t:veichles){
				local_tree[t].initRoot(t,capacity);
			}
			vector<vector<int>> update_list;
			while(exist_order.size()!=0){
				int order_num;string car;
				vector<vector<double>> p1(veichle_num,vector<double>(exist_order.size(),0));
				for(int c = 0;c<veichle_num;c++){
					for(int t = 0;t<exist_order.size();t++){
						p1[c][t] = pow(order_eta_matrix[exist_order[t]][last_point[c].back()],eta_index1)*pow(order_tao_matrix[exist_order[t]][last_point[c].back()],tao_index1);
					}
				}
				pair<int,int> t = selectElement(p1,test_q1);
				order_num = exist_order[t.second];
				order_num = exist_order.front();
				car = veichles[t.first];
				order_tao_matrix[order_num][last_point[t.first].back()] = (1 - local_update_car)*order_tao_matrix[order_num][last_point[t.first].back()] + local_update_car*order_tao;
				last_point[t.first].push_back(order_num);
				vector<int> temp_pair = tree_construct(local_tree[car],tao_matrix,node_relation,unfinishorderlist[order_num].getOrderId(),currentroute_nodelist,
				order_pnode,order_dnode,test_q2,beta,eta_index2,tao_index2,unfinishorderlist[order_num].getDemand(),tao,local_update_car,order_loc,beta2);
				update_list.push_back(temp_pair);
				exist_order.erase(remove_if(exist_order.begin(),exist_order.end(),[order_num](int value){ return value == order_num; }),exist_order.end());
			}

			for(string each_car:veichles){
				TreeNode*temp_node = local_tree[each_car].getRoot();
				node startnode = startsolution[each_car].front();
				local_solution[each_car] = orderHelper(temp_node,2,order_pnode,order_dnode,currentroute_nodelist,startnode,unfinishorderlist);
			}

            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				local_bestpair = update_list;
				local_bestorder = last_point;
				local_best_tree = local_tree;
			}
		}
        // global update
        
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_best_tree = local_best_tree;
			global_bestorder = local_bestorder;
        }
        
		if(better)
		for(string each_car:veichles)
			global_update(local_best_tree[each_car].getRoot(),tao_matrix,order_loc,global_update_car,1.0/local_best_cost3,beta2);

		/*for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<global_bestorder[i].size() - 1;g++){
				order_tao_matrix[global_bestorder[i][g + 1]][global_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[global_bestorder[i][g + 1]][global_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}*/
		if(better){
		for(int i = 0;i<veichle_num;i++){
			for(int g = 0;g<local_bestorder[i].size() - 1;g++){
				order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] = (1 - global_update_car)*order_tao_matrix[local_bestorder[i][g + 1]][local_bestorder[i][g]] + global_update_car/local_best_cost3;
			}
		}
		}	
		cost_num.push_back(global_best_cost);
    }
	//mergesolution(solution,best_currentsolution);
	solution = best_currentsolution;
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	//printfactory(solution,"fuck");
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif//no

#if 0
int main(){
	int whole_iteration = 50;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.1;
    double eta_index1 = 1.5;
	double tao_index1 = 1;
    double eta_index2 = 1.5;
    double tao_index2 = 1;
    double test_q2 = 0.85;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 2;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;unordered_map<string,int>order_loc;
	for (int i = 0; i < order.size(); i++){
		unfinishorderlist.push_back(order[i]);
        order_loc[order[i].getOrderId()] = i;
    }
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
    vector<vector<double>> order_tao_matrix(unfinishorderlist.size(),vector<double>(unfinishorderlist.size(),order_tao));
    double tao = order_tao;
    tao = 1/300.0;
    vector<vector<vector<double>>> tao_matrix(2,vector<vector<double>>(unfinishorderlist.size() + veichle_num,vector<double>(unfinishorderlist.size(),tao)));
    vector<vector<vector<double>>> eta_matrix = initial_eta(unfinishorderlist,startsolution);
	vector<int> best_orderlist;vector<unordered_map<int,int>> best_tree;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<int> local_orderlist;vector<unordered_map<int,int>> local_tree;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
			vector<int> order_list(problem_size);vector<int>exist_order(unfinishorderlist.size());
            iota(order_list.begin(),order_list.end(),0);
            iota(exist_order.begin(),exist_order.end(),0);
           // order_list = shuffle_vector(order_list);
            vector<int> history_list;
            for(int i = 0;i<veichle_num;i++){
                history_list.push_back(i);
            }
			for(int num = 0;num<problem_size;num++){
               // cout<<order_list[num]<<" ";
                for(int value:exist_order){

                }
			}
            vector<unordered_map<int,int>> reslut_tree(veichle_num);
            vector<vector<double>> select_stroage(veichle_num);
            vector<vector<int>> select_node(veichle_num);

            for(int i = 0;i<veichle_num;i++){
                select_node[i].push_back(2);
                select_stroage[i].push_back(capacity);
                reslut_tree[i].insert({0,-(veichle_num - i)});
            }
            for(int i = 0;i<problem_size;i++){
                pair<int,int> t;int num;
                int select_loc; //代表选中的索引
                int select_car;double p_t = 0;
            for(int number:order_list){
                vector<vector<double>> probility_select(veichle_num);
                for(int i = 0;i<veichle_num;i++){
                    for(int j = 0;j<select_node[i].size();j++){
                        pair<int,bool> o = getParentAndChildSide(select_node[i][j]);
                        double p;
                        if(select_stroage[i][j] < unfinishorderlist[number].getDemand()){
                            p = 0;probility_select[i].push_back(p);continue;
                        }
                        int lo = reslut_tree[i][o.first] + veichle_num;
                        if(o.second){   //左子树
                            p = pow(eta_matrix[0][lo][number],eta_index2)*pow(tao_matrix[0][lo][number],tao_index2);
                        }else{
                            p = pow(eta_matrix[1][lo][number],eta_index2)*pow(tao_matrix[1][lo][number],tao_index2);
                        }
                        probility_select[i].push_back(p);
                    }
                }
                pair<int,int> t1 = selectElement(probility_select,test_q2);
                if(p_t<probility_select[t1.first][t1.second]){
                    t = t1;num = number;
                    p_t = probility_select[t1.first][t1.second];
                }
            }
                select_car = t.first;
                select_loc = t.second;
                pair<int,bool> o = getParentAndChildSide(select_node[select_car][select_loc]);
                int lo = reslut_tree[select_car][o.first] + veichle_num;
                if(o.second){
                    tao_matrix[0][lo][num] = (1 - local_update_car)*tao_matrix[0][lo][num] + local_update_car*tao;
                }else{
                    tao_matrix[1][lo][num] = (1 - local_update_car)*tao_matrix[1][lo][num] +  local_update_car*tao;
                }
                reslut_tree[select_car].insert({select_node[select_car][select_loc],num});
                int l1,l2;
                l1 = 2*select_node[select_car][select_loc] + 1;
                l2 = 2*select_node[select_car][select_loc] + 2;
                double a1,a2;
                a2 = select_stroage[select_car][select_loc];
                a1 = a2 - unfinishorderlist[num].getDemand();
                select_node[select_car].erase(select_node[select_car].begin() + select_loc);
                select_stroage[select_car].erase(select_stroage[select_car].begin() + select_loc);
                select_node[select_car].push_back(l1);
                select_node[select_car].push_back(l2);
                select_stroage[select_car].push_back(a1);
                select_stroage[select_car].push_back(a2);
                order_list.erase(
                    std::remove(order_list.begin(), order_list.end(), num),
                    order_list.end()
                );
        }
			for(int i = 0;i<veichle_num;i++){
                local_solution[veichles[i]] = orderHelper(reslut_tree[i],0,7,order_pnode,order_dnode,
                    currentroute_nodelist,local_solution[veichles[i]][0],unfinishorderlist);
                /*unordered_map<int,int> temp_tree = treehelper(local_solution[veichles[i]],order_loc);
                local_solution[veichles[i]] = orderHelper(temp_tree,0,7,order_pnode,order_dnode,
                    currentroute_nodelist,local_solution[veichles[i]][0],unfinishorderlist);*/
            }
       //   printsolution(local_solution);
			Timecaculate(local_solution);
            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
                local_orderlist = order_list;
                local_tree = reslut_tree;
			}
		}
        // global update
    
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
            best_orderlist = local_orderlist;
            best_tree = local_tree;
        }
        if(better){
            global_update_tree(local_tree,tao_matrix,1/local_best_cost3,global_update_car);
        }else{
            global_update_tree(local_tree,tao_matrix,1/local_best_cost3,global_update_car);
        }
		cost_num.push_back(global_best_cost);
    }
    //printTree(best_tree[0]);
	mergesolution(solution,best_currentsolution);
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	//printfactory(solution,"fuck");
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif//no

#if 0
int main(){
	int whole_iteration = 50;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.1;
    double node_select = 2;
	double node_select_eta = 1;
    double test_q_node = 0.85;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 2;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readveichle("fick","dick");
	readorder(instance_order,"fuck");
	cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	//solution.clear();
    unordered_map<string,vector<orderitem>> problem;
	current_time = 600;
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	vector<orderitem> currentorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
	while (unfinishorderlist.size() != 0) {
        vector<double> cost_num;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
		currentorderlist = getnewproblem(unfinishorderlist, solution,startsolution,currentorder_location);
		if(currentorderlist.size() == 0){
			current_time += timeinterval;
			continue;
		}
        unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
        vector<node> currentroute_nodelist = global_node_construct(currentorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
        vector<vector<double>> currentroute_node_relation = global_node_matrix(currentroute_nodelist);
        vector<vector<double>>currentroute_eta_relation = global_node_eta_matrix(currentorderlist,startsolution,currentroute_nodelist,"fuck");
		//vector<vector<double>>currentroute_eta_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),1.0/140.0));
		//vector<double> currentroute_time_relation = global_time_matrix(currentroute_nodelist);
        double tao = currentroute_eta_relation[0][0];
		cout<<1/tao<<endl;
		vector<int> current_exist;
		for(int i = 0;i<currentorderlist.size();i++){
			current_exist.push_back(order_pnode[currentorderlist[i].getOrderId()]);
		}
		for (int iter = 0; iter < whole_iteration; iter++) {
            unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
			for(int ant = 0;ant<constuct_ant_num;ant++){
				unordered_map<string,vector<node>> local_solution = startsolution;
				vector<int> exist = current_exist;
				vector<int> current_location(veichle_num);
				for(int i = 0;i<veichle_num;i++)
					current_location[i] = i;
				while(exist.size() != 0){
					string current_car;vector<vector<double>> probility_of_selection(veichle_num,vector<double>());
					vector<vector<int>> select_node(veichle_num,vector<int>());
					// 为localsolution根据当下的情况构建虚拟解
					for(int i = 0;i<veichle_num;i++){
						if(local_solution[veichles[i]].back().getdeliverlist().size() != 0){
							select_node[i].push_back(order_dnode[local_solution[veichles[i]].back().getdeliverlist().back()]);
						}
						for(int j:exist){
							if(currentroute_nodelist[j].getpakage() > local_solution[veichles[i]].back().getstorage()){
								continue;
							}else{
								select_node[i].push_back(j);
							}
						}
					}
					for(int i = 0;i<veichle_num;i++){
						int type = 0;
						if(local_solution[veichles[i]].back().getdeliverlist().size() == 0){
							type = 0;
						}else{
							type = 1;
						}
						for(int j : select_node[i]){
							double temp_p = 0;
							if(type == 0){
								temp_p = pow(currentroute_node_relation[current_location[i]][j],node_select)*pow(currentroute_eta_relation[current_location[i]][j],node_select_eta);
							}else{
								if(currentroute_nodelist[j].getpickup() == 0){
									temp_p = pow(currentroute_node_relation[current_location[i]][j],node_select)*pow(currentroute_eta_relation[current_location[i]][j],node_select_eta);
								}else{
									int end2 = order_dnode[local_solution[veichles[i]].back().getdeliverlist().back()];
									int end1 = order_dnode[currentroute_nodelist[j].getorderid()];
									int start1 = current_location[i];
									int start2 = j;
								//	temp_p = pow((currentroute_node_relation[start1][start2] + currentroute_node_relation[end1][end2])/2,node_select)*pow((currentroute_eta_relation[start1][start2] + currentroute_eta_relation[end1][end2])/2,node_select_eta);
									temp_p = pow(currentroute_node_relation[current_location[i]][j],node_select)*pow(currentroute_eta_relation[current_location[i]][j],node_select_eta);
							
								}
							}
							probility_of_selection[i].push_back(temp_p);
						}
					}
					pair<int,int> selected_node = selectElement(probility_of_selection,test_q_node);
					int last_node = current_location[selected_node.first];
					current_location[selected_node.first] = select_node[selected_node.first][selected_node.second];
					string topvalue = veichles[selected_node.first];
					int current_car_location = select_node[selected_node.first][selected_node.second];
					currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*tao;
					if (currentroute_nodelist[current_car_location].getpickup()) {
						int templocation = current_car_location;
						exist.erase(std::remove_if(exist.begin(), exist.end(), [templocation](int value) { return value == templocation; }),exist.end());
					}
					local_solution[topvalue].push_back(currentroute_nodelist[current_car_location]);
					PDlist_next(local_solution[topvalue], "fuck333");
            	}
				for(int i = 0;i<veichle_num;i++){
					string topvalue = veichles[i];
					int last_node,current_car_location = current_location[i];
					vector<string> left_orderlist = local_solution[topvalue].back().getdeliverlist();
                    while (left_orderlist.size() != 0) {
                        last_node = current_car_location;
                        if(currentroute_nodelist.size() - 1<order_dnode[left_orderlist.back()]){
                            cout<<"fuck that is it"<<endl;
                            cout<<endl;
                        }
                        local_solution[topvalue].push_back(currentroute_nodelist[order_dnode[left_orderlist.back()]]);
                        current_car_location = order_dnode[left_orderlist.back()];
                        // local update
                        currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*tao;
                        PDlist_next(local_solution[topvalue], "fuck444");
                        left_orderlist.pop_back();
                    }
				}
				Timecaculate(local_solution);
                double tempcost = f2_totaldistance(local_solution);
                if(tempcost<local_best_cost3){
                    local_best_cost3 = tempcost;
                    local_bestsolution3 = local_solution;
				}
			}

			/*for(int i = 0;i<veichle_num;i++){
				for(int j = 0;j<currentroute_eta_relation[i].size();j++){
					currentroute_eta_relation[i][j] = (1 - global_update_car)*currentroute_eta_relation[i][j];
				}
			}*/

            // global update
            for(int i = 0;i<veichle_num;i++){
                int last_node = i;
                for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
                    if(local_bestsolution3[veichles[i]][j].getpickup()){
                        currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                    //    currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
						last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }else{
                        currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                    //    currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
						last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }
                }
            }

			if(local_best_cost3<global_best_cost){
                global_best_cost = local_best_cost3;
                best_currentsolution = local_bestsolution3;
            }
			cost_num.push_back(global_best_cost);
        }
       	writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
        mergesolution(solution, best_currentsolution);
        startsolution = best_currentsolution;
        current_time += timeinterval;
	}
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    //cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	printfactory(solution,"fuck");
	cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif//no

void checkTriangleMatrix(double matrix[154][154]) {
    int n = 154;
    vector<string> violations;

    // 遍历所有三点组合（i<j<k）
    for (int i = 0; i < n; ++i) {
        for (int j = i+1; j < n; ++j) {
            for (int k = j+1; k < n; ++k) {
                double a = matrix[i][j];
                double b = matrix[i][k];
                double c = matrix[j][k];

                // 检查三角不等式[1,2](@ref)
                if (((a + b < c) || (a + c < b) || (b + c < a))) {
                    string msg =  to_string(i) + "," + to_string(j) + "," + to_string(k) +" || "
                               + to_string(a) + "," + to_string(b) + "," + to_string(c);
                               
                    violations.push_back(msg);
                }
            }
        }
    }

    // 结果输出
    if (violations.empty()) {
        cout << "所有三点组合满足三角不等式" << endl;
    } else {
        cout << "发现 " << violations.size() << " 处违规：" << endl;
        for (int i = 0; i < min(5, (int)violations.size()); ++i) {
            cout << violations[i] << endl;
        }
    }
}

double findMaxTriangleViolation(double distMat[154][154]) {
    double max_violation = 0.0;
    int n = 154;
    
    // 遍历所有三元组i < j < k
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                // 获取三条边的距离
                double a = distMat[i][j];
                double b = distMat[i][k];
                double c = distMat[j][k];
                
                // 将三条边排序以确定最大值
                double edges[] = {a, b, c};
                std::sort(edges, edges + 3);
                double max_edge = edges[2];
                double sum_other_two = edges[0] + edges[1];
                
                // 检查是否违反三角不等式
                if (max_edge > sum_other_two) {
                    double violation = max_edge - sum_other_two;
                    if (violation > max_violation) {
                        max_violation = violation;
                    }
                }
            }
        }
    }
    return max_violation;
}

#if 0
// 示例矩阵初始化与调用
int main() {
	readistance();
    // 执行检查
    cout<<findMaxTriangleViolation(Distance);
    return 0;
}
#endif

int check_position1(unordered_map<string,vector<node>> dealsolution,orderitem neworder,vector<string>&test){
	int count = 0;
	unordered_map<string, vector<node>> memorysolution = dealsolution;
			double temp = 100000000000; vector<node> templist1;
			node pickup_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), deliver_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
			string minveichle = "fuck"; int beginpos, endpos; vector<int>metric; double min = 100000000000l;
			vector<node>routelist, templist;
			for (string each_car:veichles) {
				routelist = dealsolution[each_car];
				if (routelist.size() == 1) {
					templist = routelist;
					// 计算得到新的node列，从而进行判断其在货物的取放上是否满足容量的限制
					insertBefore(templist, 1, deliver_node);
					insertAfter(templist, 0, pickup_node);
					templist1 = dealsolution[each_car];
					dealsolution[each_car] = templist;
					Timecaculate(dealsolution);
					temp = f2_totaldistance(dealsolution);
					dealsolution[each_car] = templist1;
					dealsolution = memorysolution;
					if (temp < min) {
						minveichle = each_car;
						beginpos = 0;
						endpos = 1;
						min = temp;
					}
					test.push_back(each_car + 'c' + to_string(0)+'a'+to_string(1));
					count++;
				}
				else {
					for (int i = 0; i < routelist.size(); i++) {
						vector<int> toinsertdeliver;
						for (int j = i + 1; j <= routelist.size(); j++) {
							if(j == routelist.size()){
								if(routelist[i].getdeliverlist().size() == 0)toinsertdeliver.push_back(j);
								break;
							}
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
							PDlist(templist,"fucj");
							for (int k = 0; k < templist.size(); k++) {
								if (templist[k].getstorage() < 0) {
									getout = 1; break;
								}
							}
							if (getout)continue;
							templist1 = dealsolution[each_car];
							dealsolution[each_car] = templist;
							Timecaculate(dealsolution);
							temp = f2_totaldistance(dealsolution);
							dealsolution[each_car] = templist1;
							dealsolution = memorysolution;
							if (temp < min) {
								minveichle = each_car;
								beginpos = i;
								endpos = j;
								min = temp;
							}
							test.push_back(each_car + 'c' + to_string(i)+'a'+to_string(j));
							count++;
						}
					}
				}
			}
			insertBefore(dealsolution[minveichle], endpos, deliver_node);
			insertAfter(dealsolution[minveichle], beginpos, pickup_node);
			PDlist(dealsolution[minveichle]);
			//Timecaculate(dealsolution);
	return count;
}

int check_position2(unordered_map<string,vector<node>> tempsolution,orderitem neworder,vector<string>&test){
	int count = 0;
	node p_node(neworder.getPickupFactoryId(), neworder.getOrderId(), neworder.getCreationTime(), 1, neworder.getLoadTime(),neworder.getDemand()), d_node(neworder.getDeliveryFactoryId(), neworder.getOrderId(), neworder.getCommittedCompletionTime(), 0, neworder.getUnloadTime(),neworder.getDemand());
	for(int each = 0;each<veichle_num;each++){
		string each_car = veichles[each];
		double p_storage = capacity,d_storage = 0; // 动态更新当前节点的可以进行存储的值，从而节省N的时间复杂度的判断时间
		// 默认存在起始点
		vector<string> order_stack;
		for(int loc1 = 0;loc1<tempsolution[each_car].size();loc1++){
			string mark_order = "fuck";
			if(loc1!=0){
				if(tempsolution[each_car][loc1].getpickup()){
					order_stack.push_back(tempsolution[each_car][loc1].getorderid());
				}else{
					order_stack.pop_back();
				}
				if(tempsolution[each_car][loc1].getpickup()){	// 确保初始的点的demand为0，其开始不受影响
					p_storage -= tempsolution[each_car][loc1].getpakage();
				}else{
					p_storage += tempsolution[each_car][loc1].getpakage();
				}
			}
			if(order_stack.size()!=0)mark_order = order_stack.back();
			
			double d_storage_max = 0;d_storage = 0;
			for(int loc2 = loc1 + 1;loc2<tempsolution[each_car].size()+1;loc2++){
				if((p_storage - d_storage_max) < p_node.getpakage())break;
				if(loc2 == tempsolution[each_car].size()){
					if(mark_order!="fuck")break;
				}else{
					if(mark_order == "fuck"){
						if(tempsolution[each_car][loc2].getbeforedeliverlist().size()!= 0){
							if(tempsolution[each_car][loc2].getpickup()){
								d_storage += tempsolution[each_car][loc2].getpakage();
							}else{
								d_storage -= tempsolution[each_car][loc2].getpakage();
							}
							if(d_storage_max<d_storage)d_storage_max = d_storage;
							continue;
						}
					}else if(tempsolution[each_car][loc2].getbeforedeliverlist().back() != mark_order){
						if(tempsolution[each_car][loc2].getpickup()){
							d_storage += tempsolution[each_car][loc2].getpakage();
						}else{
							d_storage -= tempsolution[each_car][loc2].getpakage();
						}
						if(d_storage_max<d_storage)d_storage_max = d_storage;
						continue;
					}
				}
				test.push_back(veichles[each]+'c'+to_string(loc1)+'a'+to_string(loc2));
				count++;
				if(loc2 == tempsolution[each_car].size())break;
				if(tempsolution[each_car][loc2].getorderid() == mark_order){
					break;
				}
				if(tempsolution[each_car][loc2].getpickup()){
					d_storage += tempsolution[each_car][loc2].getpakage();
				}else{
					d_storage -= tempsolution[each_car][loc2].getpakage();
				}
				if(d_storage_max<d_storage)d_storage_max = d_storage;
			}
		}
	}
	return count;
}

void removeCommonElements(std::vector<std::string>& vec_a, 
	std::vector<std::string>& vec_b) {
// 创建哈希集合记录对方容器原始内容
const std::unordered_set<std::string> set_b(vec_b.begin(), vec_b.end());
const std::unordered_set<std::string> set_a(vec_a.begin(), vec_a.end());

// 定义lambda表达式进行元素过滤
auto filter = [](auto& vec, const auto& forbid_set) {
	vec.erase(
		std::remove_if(vec.begin(), vec.end(),
			[&](const std::string& s) {
				return forbid_set.count(s) > 0;
			}),
		vec.end()
	);
};

// 双向过滤公共元素
filter(vec_a, set_b);
filter(vec_b, set_a);
}

void deduplicate_and_find_duplicates(std::vector<std::string>& vec, 
	std::vector<std::string>& duplicates) {
std::unordered_set<std::string> seen;     // 记录已出现的元素
std::unordered_set<std::string> dup_cache; // 防止重复记录相同元素

// 第一遍遍历：检测重复元素
for (const auto& elem : vec) {
if (seen.count(elem)) {  // 发现重复元素[1,4](@ref)
if (!dup_cache.count(elem)) {  // 避免重复添加
duplicates.push_back(elem);
dup_cache.insert(elem);
}
} else {
seen.insert(elem);
}
}

// 第二遍遍历：构建去重后的vector
seen.clear();
std::vector<std::string> dedup_vec;
for (const auto& elem : vec) {
if (!seen.count(elem)) {
dedup_vec.push_back(elem);
seen.insert(elem);
}
}
vec.swap(dedup_vec);  // 替换原始容器
}

#if 0
int main(){
	readtime(); readistance(); readfactory();
	int instance = 7;
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	vector<string> order_seqence;
	for(int i = 0;i<problem_size;i++)
		order_seqence.push_back(unfinishorderlist[i].getOrderId());
	unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
	
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_distance(currentroute_nodelist);
	vector<orderitem> temp_orderlist = unfinishorderlist;
	temp_orderlist.pop_back();
	double order_tao = initial_order_tao(temp_orderlist,startsolution,currentroute_nodelist,best_currentsolution);
	printsolution(best_currentsolution);
	cout<<"test_begin"<<endl;
	vector<string> test1,test2;
	int l1 = check_position1(best_currentsolution,unfinishorderlist.back(),test1);
	int l2 = check_position2(best_currentsolution,unfinishorderlist.back(),test2);
	cout<<l1<<" "<<l2<<endl;// 将所有的位置进行存储之后然后进行字符串的比对，折后消除重复寻找不同的部分从而找到哪些是对应缺漏的位置
	vector<string> t1,t2;
	deduplicate_and_find_duplicates(test1,t1);
	deduplicate_and_find_duplicates(test2,t2);
	cout<<test1.size()<<" "<<test2.size()<<endl;
	for(string each:t1){
		cout<<each<<" ";
	}
	removeCommonElements(test1,test2);
	cout<<test1.size()<<" "<<test2.size()<<endl;
	cout<<"test1"<<endl;
	for(string each:test1){
		cout<<each<<" ";
	}
	cout<<"test2"<<endl;
	for(string each:test2){
		cout<<each<<" ";
	}
}
#endif

void wilcoxonTest( std::vector<double>& group0, 
	 std::vector<double>& group1,
	double& U, double& p) {
// 合并数据并标记组别（0=group0，1=group1）
std::vector<double> values;
std::vector<int> groups;
for (size_t i = 0; i < group0.size(); ++i) {
values.push_back(group0[i]);
groups.push_back(0);
}
for (size_t i = 0; i < group1.size(); ++i) {
values.push_back(group1[i]);
groups.push_back(1);
}

// 按值排序并同步调整组别标记
std::vector<size_t> index(values.size());
for (size_t i = 0; i < index.size(); ++i) index[i] = i;
std::sort(index.begin(), index.end(), [&](size_t a, size_t b) {
return values[a] < values[b];
});

// 提取排序后的值和组别
std::vector<double> sortedValues;
std::vector<int> sortedGroups;
for (auto i : index) {
sortedValues.push_back(values[i]);
sortedGroups.push_back(groups[i]);
}

// 计算秩次（处理相同值）
std::vector<double> ranks(sortedValues.size(), 0.0);
size_t i = 0;
while (i < sortedValues.size()) {
size_t start = i;
while (i < sortedValues.size() && sortedValues[i] == sortedValues[start]) ++i;
double avgRank = (start + i + 1) / 2.0;
for (size_t j = start; j < i; ++j) ranks[j] = avgRank;
}

// 计算各组秩和
double sum0 = 0.0, sum1 = 0.0;
for (size_t j = 0; j < sortedGroups.size(); ++j) {
if (sortedGroups[j] == 0) sum0 += ranks[j];
else sum1 += ranks[j];
}

// 计算U统计量
int n0 = group0.size(), n1 = group1.size();
U = sum0 - n0*(n0+1)/2.0;
U = std::min(U, n0*n1 - U);  // 双尾检验取较小值

// 正态近似计算p值（大样本）
double mu = n0 * n1 / 2.0;
double sigma = sqrt(n0 * n1 * (n0 + n1 + 1) / 12.0);
double Z = (U - mu) / sigma;
p = 2 * (1 - 0.5 * (1 + erf(fabs(Z) / sqrt(2))));
}

// 找到对应取货点所需要对应的送货点
vector<int> find_end_point(unordered_map<string,vector<node>> tempsolution,int each,int beginpos,node p_node,node d_node){
	vector<int> insert_list;
	string each_car = veichles[each];
		double p_storage = capacity,d_storage = 0; // 动态更新当前节点的可以进行存储的值，从而节省N的时间复杂度的判断时间
		// 默认存在起始点
		vector<string> order_stack;
		for(int loc1 = 0;loc1<tempsolution[each_car].size();loc1++){
			string mark_order = "fuck";
			if(loc1!=0){
				if(tempsolution[each_car][loc1].getpickup()){
					order_stack.push_back(tempsolution[each_car][loc1].getorderid());
				}else{
					order_stack.pop_back();
				}
				if(tempsolution[each_car][loc1].getpickup()){	// 确保初始的点的demand为0，其开始不受影响
					p_storage -= tempsolution[each_car][loc1].getpakage();
				}else{
					p_storage += tempsolution[each_car][loc1].getpakage();
				}
			}
			if(order_stack.size()!=0)mark_order = order_stack.back();
			if(loc1 != beginpos)continue;
			double d_storage_max = 0;d_storage = 0;
			for(int loc2 = loc1 + 1;loc2<tempsolution[each_car].size()+1;loc2++){
				if((p_storage - d_storage_max) < p_node.getpakage())break;
				if(loc2 == tempsolution[each_car].size()){
					if(mark_order!="fuck")break;
				}else{
					if(mark_order == "fuck"){
						if(tempsolution[each_car][loc2].getbeforedeliverlist().size()!= 0){
							if(tempsolution[each_car][loc2].getpickup()){
								d_storage += tempsolution[each_car][loc2].getpakage();
							}else{
								d_storage -= tempsolution[each_car][loc2].getpakage();
							}
							if(d_storage_max<d_storage)d_storage_max = d_storage;
							continue;
						}
					}else if(tempsolution[each_car][loc2].getbeforedeliverlist().back() != mark_order){
						if(tempsolution[each_car][loc2].getpickup()){
							d_storage += tempsolution[each_car][loc2].getpakage();
						}else{
							d_storage -= tempsolution[each_car][loc2].getpakage();
						}
						if(d_storage_max<d_storage)d_storage_max = d_storage;
						continue;
					}
				}
				
				insert_list.push_back(loc2);

				if(loc2 == tempsolution[each_car].size())break;
				if(tempsolution[each_car][loc2].getorderid() == mark_order){
					break;
				}
				if(tempsolution[each_car][loc2].getpickup()){
					d_storage += tempsolution[each_car][loc2].getpakage();
				}else{
					d_storage -= tempsolution[each_car][loc2].getpakage();
				}
				if(d_storage_max<d_storage)d_storage_max = d_storage;
			}
			if(loc1 == beginpos)break;
		}
	return insert_list;
}

vector<int> find_start_point(unordered_map<string,vector<node>> tempsolution,int each,int endpos,node p_node,node d_node){
	vector<int> insert_list;vector<node> r_solution;
	double p_storage = capacity,d_storage = 0;
	vector<string> order_stack;string each_car = veichles[each];
	vector<string> temp_order_stack;
	for(int loc = tempsolution[veichles[each]].size();loc >= 1;loc--){
		string mark_order = "fuck";
		if(loc != tempsolution[veichles[each]].size()){
			if(!tempsolution[each_car][loc].getpickup()){
				order_stack.push_back(tempsolution[each_car][loc].getorderid());
				temp_order_stack.push_back(tempsolution[each_car][loc].getorderid());
				p_storage -= tempsolution[each_car][loc].getpakage();
			}else{
				order_stack.pop_back();
				temp_order_stack.pop_back();
				p_storage += tempsolution[each_car][loc].getpakage();
			}
		}
		if(order_stack.size()!=0)mark_order = order_stack.back();
		if(loc != endpos)continue;
		double d_storage_max = 0;
		for(int loc2 = loc - 1;loc2>-1;loc2--){
			if((p_storage - d_storage_max)<p_node.getpakage())break;
			
			if(loc2 == 0){
				if(mark_order != "fuck")break;
			}else{
				if(mark_order == "fuck"){
					if(temp_order_stack.size()!=0){
						if(!tempsolution[each_car][loc2].getpickup()){
							d_storage += tempsolution[each_car][loc2].getpakage();
						}else{
							d_storage -= tempsolution[each_car][loc2].getpakage();
						}
						if(d_storage_max<d_storage)d_storage_max = d_storage;			
						continue;
					}
				}else if(temp_order_stack.back()!=mark_order){
					if(!tempsolution[each_car][loc2].getpickup()){
						d_storage += tempsolution[each_car][loc2].getpakage();
					}else{
						d_storage -= tempsolution[each_car][loc2].getpakage();
					}
					if(d_storage_max<d_storage)d_storage_max = d_storage;			
					continue;
				}
			}

			if(loc2!=0){
				if(!tempsolution[each_car][loc2].getpickup()){
					temp_order_stack.push_back(tempsolution[each_car][loc2].getorderid());
				}else{
					temp_order_stack.pop_back();
				}
			}

			insert_list.push_back(loc2);
			if(loc2 == 0)break;
			if(tempsolution[each_car][loc2].getorderid() == mark_order){
				break;
			}
			if(!tempsolution[each_car][loc2].getpickup()){
				d_storage += tempsolution[each_car][loc2].getpakage();
			}else{
				d_storage -= tempsolution[each_car][loc2].getpakage();
			}
			if(d_storage_max<d_storage)d_storage_max = d_storage;
		}
		if(loc == endpos)break;
	}

	return insert_list;
}

// 先以高选择性确定取货点然后在选取送货点
// 或者逆序先确定送货点然后再确定取货点，即通过这种方式先大概率筛选那些大概率不满足条件的插入对
vector<int> order_construct_start_to_end(unordered_map<string,vector<node>> &tempsolution,vector<vector<double>> &tao_matrix,vector<vector<double>> &eta_matrix,double q,double local_update,double local_tao,
	node p_node,node d_node,unordered_map<string,int> order_pnode,unordered_map<string,int>order_dnode,double index1,double index2){
	// 每一对的概率以及每一对进行插入的位置
	vector<vector<double>> probility(veichle_num);vector<vector<int>> insert_point(veichle_num);
	double sigma = 0.5;
	for(int each = 0;each<veichle_num;each++){
		string each_car = veichles[each];
		double p_storage = capacity,d_storage = 0; // 动态更新当前节点的可以进行存储的值，从而节省N的时间复杂度的判断时间
		// 默认存在起始点
		vector<string> order_stack;
		for(int loc1 = 0;loc1<tempsolution[each_car].size();loc1++){
			string mark_order = "fuck";
			if(loc1!=0){
				if(tempsolution[each_car][loc1].getpickup()){
					order_stack.push_back(tempsolution[each_car][loc1].getorderid());
				}else{
					order_stack.pop_back();
				}
				if(tempsolution[each_car][loc1].getpickup()){	// 确保初始的点的demand为0，其开始不受影响
					p_storage -= tempsolution[each_car][loc1].getpakage();
				}else{
					p_storage += tempsolution[each_car][loc1].getpakage();
				}
			}
			if(order_stack.size()!=0)mark_order = order_stack.back();
			if(p_storage < p_node.getpakage())continue;
			if(loc1 == tempsolution[each_car].size() - 1){
				int point1 = 0,point2 = 0,point3 = 0;
				point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
				point2 = order_pnode[p_node.getorderid()];
				probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + sigma),index1)*pow(tao_matrix[point1][point2],index2));
				insert_point[each].push_back(loc1);
			}else{
				int point1 = 0,point2 = 0,point3 = 0;
				if(tempsolution[each_car][loc1].getpickup()){
					point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
				}else{
					point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
				}
				point2 = order_pnode[p_node.getorderid()];
				if(tempsolution[each_car][loc1 + 1].getpickup()){
					point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
				}else{
					point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
				}
				probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3],index2));
				insert_point[each].push_back(loc1);
			}
		}
	}
	pair<int,int> insert_place = selectElement(probility,q);
	int select_car = insert_place.first;
	string each_car = veichles[select_car];
	int beginpos = insert_point[select_car][insert_place.second];
	int loc1 = beginpos;
	vector<int> endlist = find_end_point(tempsolution,select_car,beginpos,p_node,d_node);
	if(endlist.size() == 0)cout<<"fuck endlist size equals 0"<<endl;
	vector<double> probility2;
	for(int loc2:endlist){
		if(loc1 == tempsolution[each_car].size() - 1){
			int point1 = 0,point2 = 0,point3 = 0;
			point2 = order_pnode[p_node.getorderid()];
			point3 = order_dnode[d_node.getorderid()];
			probility2.push_back(pow(1.0/(eta_matrix[point2][point3] + sigma),index1)*pow(tao_matrix[point2][point3],index2));
		}else if(loc2 == tempsolution[each_car].size()){
			int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
			point5 = order_dnode[d_node.getorderid()];
			if(tempsolution[each_car][loc2 - 1].getpickup()){
				point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
			}else{
				point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
			}
			probility2.push_back(pow(1.0/(eta_matrix[point4][point5] + sigma),index1)*pow(tao_matrix[point4][point5],index2));
		}else if(loc1 + 1 == loc2){
			int point1 = 0,point2 = 0,point3 = 0,point4 = 0;
			if(tempsolution[each_car][loc2].getpickup()){
				point4 = order_pnode[tempsolution[each_car][loc2].getorderid()];
			}else{
				point4 = order_dnode[tempsolution[each_car][loc2].getorderid()];
			}
			point2 = order_pnode[p_node.getorderid()];
			point3 = order_dnode[d_node.getorderid()];
			probility2.push_back(pow(1.0/(eta_matrix[point2][point3] + eta_matrix[point3][point4] - eta_matrix[point2][point4] + sigma),index1)*pow(tao_matrix[point2][point3]*tao_matrix[point3][point4]/tao_matrix[point2][point4],index2));
		}else{
			int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0,point6 = 0;
			if(tempsolution[each_car][loc2].getpickup()){
				point6 = order_pnode[tempsolution[each_car][loc2].getorderid()];
			}else{
				point6 = order_dnode[tempsolution[each_car][loc2].getorderid()];
			}
			point5 = order_dnode[d_node.getorderid()];
			if(tempsolution[each_car][loc2 - 1].getpickup()){
				point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
			}else{
				point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
			}
			probility2.push_back(pow(1.0/(eta_matrix[point4][point5] + eta_matrix[point5][point6] - eta_matrix[point4][point6] + sigma),index1)*pow(tao_matrix[point4][point5]*tao_matrix[point5][point6]/tao_matrix[point4][point6],index2));
		}
	}
	int best_loc = selectElement1D(probility2,q);
	pair<int,int> best_place;
	best_place.first = select_car;
	best_place.second = endlist[best_loc];
	int loc2 = endlist[best_loc];
	
	vector<int> temp_result;
	if(loc1 == tempsolution[veichles[best_place.first]].size() - 1){
		int point1 = 0,point2 = 0,point3 = 0;
		point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
	}else if(loc2 == tempsolution[veichles[best_place.first]].size()){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
	}else if(loc1 + 1 == loc2){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point3][point4] = (1 - local_update)*tao_matrix[point3][point4] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point3);temp_result.push_back(point4);
	}else{
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0,point6 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point6 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point6 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		tao_matrix[point5][point6] = (1 - local_update)*tao_matrix[point5][point6] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
		temp_result.push_back(point5);temp_result.push_back(point6);
	}
	insertBefore(tempsolution[veichles[best_place.first]],loc2,d_node);
	insertAfter(tempsolution[veichles[best_place.first]],loc1,p_node);
	PDlist(tempsolution[veichles[best_place.first]]);
	return temp_result;
}

vector<int> order_construct_end_to_start(unordered_map<string,vector<node>> &tempsolution,vector<vector<double>> &tao_matrix,vector<vector<double>> &eta_matrix,double q,double local_update,double local_tao,
	node p_node,node d_node,unordered_map<string,int> order_pnode,unordered_map<string,int>order_dnode,double index1,double index2){
	// 每一对的概率以及每一对进行插入的位置
	vector<vector<double>> probility(veichle_num);vector<vector<int>> insert_point(veichle_num);
	double sigma = 0.5;
	for(int each = 0;each<veichle_num;each++){
		string each_car = veichles[each];
		double p_storage = capacity,d_storage = 0;
		vector<string> order_stack;vector<string> temp_order_stack;
		for(int loc = tempsolution[each_car].size();loc>=1;loc--){
			string mark_order = "fuck";
			if(loc != tempsolution[veichles[each]].size()){
				if(!tempsolution[each_car][loc].getpickup()){
					order_stack.push_back(tempsolution[each_car][loc].getorderid());
					temp_order_stack.push_back(tempsolution[each_car][loc].getorderid());
					p_storage -= tempsolution[each_car][loc].getpakage();
				}else{
					order_stack.pop_back();
					temp_order_stack.pop_back();
					p_storage += tempsolution[each_car][loc].getpakage();
				}
			}
			if(order_stack.size()!=0)mark_order = order_stack.back();
			if(p_storage < p_node.getpakage())continue;
			if(loc == tempsolution[each_car].size()){
				int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
				point5 = order_dnode[d_node.getorderid()];
				if(tempsolution[each_car][loc - 1].getpickup()){
					point4 = order_pnode[tempsolution[each_car][loc - 1].getorderid()];
				}else{
					point4 = order_dnode[tempsolution[each_car][loc - 1].getorderid()];
				}
				probility[each].push_back(pow(1.0/(eta_matrix[point4][point5] + sigma),index1)*pow(tao_matrix[point4][point5],index2));
				insert_point[each].push_back(loc);
			}else{
				int point1,point2,point3;
				if(tempsolution[each_car][loc - 1].getpickup()){
					point1 = order_pnode[tempsolution[each_car][loc - 1].getorderid()];
				}else{
					point1 = order_dnode[tempsolution[each_car][loc - 1].getorderid()];
				}
				point2 = order_dnode[d_node.getorderid()];
				if(tempsolution[each_car][loc].getpickup()){
					point3 = order_pnode[tempsolution[each_car][loc].getorderid()];
				}else{
					point3 = order_dnode[tempsolution[each_car][loc].getorderid()];
				}
				probility[each].push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3],index2));
				insert_point[each].push_back(loc);
			}
		}
	}
	pair<int,int> insert_place = selectElement(probility,q);
	int select_car = insert_place.first;
	string each_car = veichles[select_car];
	int endpos = insert_point[select_car][insert_place.second];
	int loc2 = endpos;
	//cout<<endpos<<" "<<each_car<<endl;
	//printsolution(tempsolution);
	vector<int> startlist = find_start_point(tempsolution,select_car,endpos,p_node,d_node);
	//cout<<startlist.size()<<endl;
	if(startlist.size() == 0)cout<<"fuck startlist size equals 0"<<endl;
	vector<double> probility2;
	for(int loc : startlist){
		if(loc == tempsolution[each_car].size() - 1){
			int point1 = 0,point2 = 0,point3 = 0;
			point1 = order_dnode[tempsolution[each_car][loc].getorderid()];
			point2 = order_pnode[p_node.getorderid()];
			point3 = order_dnode[d_node.getorderid()];
			probility2.push_back(pow(1.0/(eta_matrix[point2][point3] + eta_matrix[point1][point2] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3],index2));
		}else if(loc2 == tempsolution[each_car].size()){
			int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
			if(tempsolution[each_car][loc].getpickup()){
				point1 = order_pnode[tempsolution[each_car][loc].getorderid()];
			}else{
				point1 = order_dnode[tempsolution[each_car][loc].getorderid()];
			}
			point2 = order_pnode[p_node.getorderid()];
			if(tempsolution[each_car][loc + 1].getpickup()){
				point3 = order_pnode[tempsolution[each_car][loc + 1].getorderid()];
			}else{
				point3 = order_dnode[tempsolution[each_car][loc + 1].getorderid()];
			}
			probility2.push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3],index2));
		}else if(loc + 1 == loc2){
			int point1 = 0,point2 = 0,point3 = 0;
			if(tempsolution[each_car][loc].getpickup()){
				point1 = order_pnode[tempsolution[each_car][loc].getorderid()];
			}else{
				point1 = order_dnode[tempsolution[each_car][loc].getorderid()];
			}
			point2 = order_pnode[p_node.getorderid()];
			point3 = order_dnode[d_node.getorderid()];
			probility2.push_back(pow(1.0/(eta_matrix[point2][point3] + eta_matrix[point1][point2] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point2][point3]*tao_matrix[point1][point2]/tao_matrix[point1][point3],index2));
		}else{
			int point1 = 0,point2 = 0,point3 = 0;
			if(tempsolution[each_car][loc].getpickup()){
				point1 = order_pnode[tempsolution[each_car][loc].getorderid()];
			}else{
				point1 = order_dnode[tempsolution[each_car][loc].getorderid()];
			}
			point2 = order_pnode[p_node.getorderid()];
			if(tempsolution[each_car][loc + 1].getpickup()){
				point3 = order_pnode[tempsolution[each_car][loc + 1].getorderid()];
			}else{
				point3 = order_dnode[tempsolution[each_car][loc + 1].getorderid()];
			}
			probility2.push_back(pow(1.0/(eta_matrix[point1][point2] + eta_matrix[point2][point3] - eta_matrix[point1][point3] + sigma),index1)*pow(tao_matrix[point1][point2]*tao_matrix[point2][point3]/tao_matrix[point1][point3],index2));
		}
	}
	int best_loc = selectElement1D(probility2,q);
	pair<int,int> best_place;
	best_place.first = select_car;
	best_place.second = startlist[best_loc];
	int loc1 = startlist[best_loc];
	
	vector<int> temp_result;
	if(loc1 == tempsolution[veichles[best_place.first]].size() - 1){
		int point1 = 0,point2 = 0,point3 = 0;
		point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
	}else if(loc2 == tempsolution[veichles[best_place.first]].size()){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
	}else if(loc1 + 1 == loc2){
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		point3 = order_dnode[d_node.getorderid()];
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point3][point4] = (1 - local_update)*tao_matrix[point3][point4] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point3);temp_result.push_back(point4);
	}else{
		int point1 = 0,point2 = 0,point3 = 0,point4 = 0,point5 = 0,point6 = 0;
		if(tempsolution[each_car][loc1].getpickup()){
			point1 = order_pnode[tempsolution[each_car][loc1].getorderid()];
		}else{
			point1 = order_dnode[tempsolution[each_car][loc1].getorderid()];
		}
		point2 = order_pnode[p_node.getorderid()];
		if(tempsolution[each_car][loc1 + 1].getpickup()){
			point3 = order_pnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}else{
			point3 = order_dnode[tempsolution[each_car][loc1 + 1].getorderid()];
		}
		if(tempsolution[each_car][loc2].getpickup()){
			point6 = order_pnode[tempsolution[each_car][loc2].getorderid()];
		}else{
			point6 = order_dnode[tempsolution[each_car][loc2].getorderid()];
		}
		point5 = order_dnode[d_node.getorderid()];
		if(tempsolution[each_car][loc2 - 1].getpickup()){
			point4 = order_pnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}else{
			point4 = order_dnode[tempsolution[each_car][loc2 - 1].getorderid()];
		}
		tao_matrix[point1][point2] = (1 - local_update)*tao_matrix[point1][point2] + local_update*local_tao;
		tao_matrix[point2][point3] = (1 - local_update)*tao_matrix[point2][point3] + local_update*local_tao;
		tao_matrix[point4][point5] = (1 - local_update)*tao_matrix[point4][point5] + local_update*local_tao;
		tao_matrix[point5][point6] = (1 - local_update)*tao_matrix[point5][point6] + local_update*local_tao;
		temp_result.push_back(point1);temp_result.push_back(point2);
		temp_result.push_back(point2);temp_result.push_back(point3);
		temp_result.push_back(point4);temp_result.push_back(point5);
		temp_result.push_back(point5);temp_result.push_back(point6);
	}
	insertBefore(tempsolution[veichles[best_place.first]],loc2,d_node);
	insertAfter(tempsolution[veichles[best_place.first]],loc1,p_node);
	PDlist(tempsolution[veichles[best_place.first]]);
	return temp_result;
}

// 固定一个点然后再确认另外一个点的方法
#if 0
int main(){
	int whole_iteration = 75;
    int constuct_ant_num = 4;
    double local_update_car = 0.1;
    double global_update_car = 0.1;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.9;
    double test_q2 = 0.9;

	/*test_q2 = 1;
	global_update_car = 0.0;
	local_update_car = 0.0;
	eta_index2 = 1;*/

	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 32;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle);
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	vector<string> order_seqence;
	for(int i = 0;i<problem_size;i++)
		order_seqence.push_back(unfinishorderlist[i].getOrderId());
	unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
	
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_distance(currentroute_nodelist);
	
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist,best_currentsolution);
	//double order_tao = 0.005;
	vector<vector<double>> order_tao_relation(problem_size + veichle_num,vector<double>(problem_size + veichle_num,order_tao));
	global_best_cost = 1/order_tao;
	cout<<1/order_tao<<endl;
	vector<vector<double>> tao_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
	
	double tao = order_tao;
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	
	// 确定Order之间的关系
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution,"fuck");
	// 作为第几个订单进行处理的信息素概率
	
	auto tenMinutes = std::chrono::seconds(600);
    vector<int> best_orderlist;vector<string>global_bestorder;
	auto startTime = std::chrono::high_resolution_clock::now();
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;unordered_map<string,BinaryTree> local_best_tree;
		vector<int> insert_order(unfinishorderlist.size());vector<string> best_order_seqence;vector<vector<int>> local_bestlist;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> update_list;
			vector<string> o_squence = order_seqence_construct(order_seqence,order_loc,order_tao_relation,order_eta_matrix,order_tao,local_update_car,test_q1,eta_index1,tao_index1);
			//o_squence = shuffle_vector(order_seqence);
			for(string o:o_squence){
				vector<int> temp_list;
				/*temp_list = order_construct_start_to_end(local_solution,tao_relation,node_relation,test_q2,local_update_car,tao,
				currentroute_nodelist[order_pnode[o]],currentroute_nodelist[order_dnode[o]],order_pnode,order_dnode,eta_index2,tao_index2);*/
				temp_list = order_construct_end_to_start(local_solution,tao_relation,node_relation,test_q2,local_update_car,tao,
				currentroute_nodelist[order_pnode[o]],currentroute_nodelist[order_dnode[o]],order_pnode,order_dnode,eta_index2,tao_index2);
				update_list.push_back(temp_list);
			}
			//cout<<count_right<<" ";count_right = 0;
			for(string value:veichles){
				PDlist(local_solution[value]);
			}
			//printsolution(local_solution);
            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				best_order_seqence = o_squence;
				local_bestlist = update_list;
			}
		}
        
		for(int i = 0;i<veichle_num;i++){
			order_tao_relation[i][order_loc[best_order_seqence[0]]] = (1 - global_update_car)*order_tao_relation[i][order_loc[best_order_seqence[0]]] + global_update_car/local_best_cost3;
		}
		
		for(int i = 1;i<best_order_seqence.size();i++){
			order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] = (1 - global_update_car)*order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] + global_update_car/local_best_cost3;
		}

        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		//cout<<local_best_cost3<<" ";
		if(local_best_cost3<global_best_cost){
			for(string value:veichles){
				PDlist(local_bestsolution3[value],"fuck");
			}
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_bestorder = best_order_seqence;
        }

		for(int i = 0;i<local_bestlist.size();i++){
			for(int j = 0;j<local_bestlist[i].size();j = j + 2){
				tao_relation[local_bestlist[i][j]][local_bestlist[i][j + 1]] = (1 - global_update_car)*tao_relation[local_bestlist[i][j]][local_bestlist[i][j + 1]] + global_update_car/local_best_cost3;
			}
		}

		int nr = getrangenumber(0,3);
		int anchor = weighted_random(0,unfinishorderlist.size() - 1);
		unordered_map<string,int> order_pnode1,order_dnode1;vector<string> car_orderlist;unordered_map<string,string> order_car;
		getpdglist(order_pnode1,order_dnode1,car_orderlist,best_currentsolution,order_car);
		unordered_map<string,vector<node>> tempsolution;
		switch (nr) {
			case 1:
				local_bestsolution3 = block_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 2:
				local_bestsolution3 = block_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 3:
				local_bestsolution3 = couple_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 0:
				local_bestsolution3 = couple_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
		}

		// global update
    	for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}
		local_best_cost3 = total_cost(local_bestsolution3);
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_bestorder = best_order_seqence;
        }
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
		if (elapsedTime >= tenMinutes) {
			std::cout << "循环已运行超过十分钟，停止循环。" << std::endl;
			break;
		}
    }
	
	solution = best_currentsolution;
    
    cost_data.push_back(total_cost(solution));
	
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\dpd_lifo_tree.csv",instance);
}
}
#endif

#if 0
int main(){
	int whole_iteration = 75;
    int constuct_ant_num = 4;
    double local_update_car = 0.1;
    double global_update_car = 0.1;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.9;
    double test_q2 = 0.9;

	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 32;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	vector<string> order_seqence;
	for(int i = 0;i<problem_size;i++)
		order_seqence.push_back(unfinishorderlist[i].getOrderId());
	unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
	
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string, int> currentorder_location;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_distance(currentroute_nodelist);
	
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist,best_currentsolution);
	//double order_tao = 0.005;
	vector<vector<double>> order_tao_relation(problem_size + veichle_num,vector<double>(problem_size + veichle_num,order_tao));
	global_best_cost = 1/order_tao;
	cout<<1/order_tao<<endl;
	vector<vector<double>> tao_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
	double tao = order_tao;
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	
	// 确定Order之间的关系
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution,"fuck");
	// 作为第几个订单进行处理的信息素概率
	
	auto tenMinutes = std::chrono::seconds(600);
    vector<int> best_orderlist;vector<string>global_bestorder;
	auto startTime = std::chrono::high_resolution_clock::now();
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;unordered_map<string,BinaryTree> local_best_tree;
		vector<int> insert_order(unfinishorderlist.size());vector<string> best_order_seqence;vector<vector<int>> local_bestlist;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
            vector<vector<int>> update_list;
			vector<string> o_squence = order_seqence_construct(order_seqence,order_loc,order_tao_relation,order_eta_matrix,order_tao,local_update_car,test_q1,eta_index1,tao_index1);
			//o_squence = shuffle_vector(order_seqence);
			for(string o:o_squence){
				vector<int> temp_list = order_construct(local_solution,tao_relation,node_relation,test_q2,local_update_car,tao,
				currentroute_nodelist[order_pnode[o]],currentroute_nodelist[order_dnode[o]],order_pnode,order_dnode,eta_index2,tao_index2);
				update_list.push_back(temp_list);
			}
			//cout<<count_right<<" ";count_right = 0;
			for(string value:veichles){
				PDlist(local_solution[value]);
			}
			//printsolution(local_solution);
            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				best_order_seqence = o_squence;
				local_bestlist = update_list;
			}
		}
        
		for(int i = 0;i<veichle_num;i++){
			order_tao_relation[i][order_loc[best_order_seqence[0]]] = (1 - global_update_car)*order_tao_relation[i][order_loc[best_order_seqence[0]]] + global_update_car/local_best_cost3;
		}
		
		for(int i = 1;i<best_order_seqence.size();i++){
			order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] = (1 - global_update_car)*order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] + global_update_car/local_best_cost3;
		}

        bool better = 0;
        //cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		cout<<local_best_cost3<<" ";
		if(local_best_cost3<global_best_cost){
			for(string value:veichles){
				PDlist(local_bestsolution3[value],"fuck");
			}
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_bestorder = best_order_seqence;
        }

		/*for(int i = 0;i<local_bestlist.size();i++){
			for(int j = 0;j<local_bestlist[i].size();j = j + 2){
				tao_relation[local_bestlist[i][j]][local_bestlist[i][j + 1]] = (1 - global_update_car)*tao_relation[local_bestlist[i][j]][local_bestlist[i][j + 1]] + global_update_car/local_best_cost3;
			}
		}*/

		int nr = getrangenumber(0,3);
		int anchor = weighted_random(0,unfinishorderlist.size() - 1);
		unordered_map<string,int> order_pnode1,order_dnode1;vector<string> car_orderlist;unordered_map<string,string> order_car;
		getpdglist(order_pnode1,order_dnode1,car_orderlist,best_currentsolution,order_car);
		unordered_map<string,vector<node>> tempsolution;
		/*switch (nr) {
			case 1:
				local_bestsolution3 = block_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 2:
				local_bestsolution3 = block_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 3:
				local_bestsolution3 = couple_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 0:
				local_bestsolution3 = couple_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
		}

		// global update
    	for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}
		local_best_cost3 = total_cost(local_bestsolution3);
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
			global_bestorder = best_order_seqence;
        }*/
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
		if (elapsedTime >= tenMinutes) {
			std::cout << "循环已运行超过十分钟，停止循环。" << std::endl;
			break;
		}
    }
	
	solution = best_currentsolution;
    
    cost_data.push_back(total_cost(solution));
	
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\dpd_lifo_tree.csv",instance);
}
}
#endif

#if 1
int main(){
	int whole_iteration = 50;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.3;
    double eta_index1 = 2;
	double tao_index1 = 1;
    double eta_index2 = 2;
    double tao_index2 = 1;
	double test_q1 = 0.8;
    double test_q2 = 0.8;
	double gamma3 = 0.1;
	double global_update_beside = 0.1;
	double local_search_update = 0.1;
	double beta = 0.1;
	double beta2 = 0.0;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 37;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	
	readorder(instance_order,"fuck");
	std::cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	initialsolution("fuck");
	vector<orderitem> unfinishorderlist;
	unordered_map<string, vector<node>>startsolution;
	startsolution = solution;
	for (int i = 0; i < order.size(); i++)
		unfinishorderlist.push_back(order[i]);
	int iter = 0; // 订单处理的轮数
    vector<double> cost_num;
	int problem_size = unfinishorderlist.size();
	vector<string> order_seqence;
	for(int i = 0;i<problem_size;i++)
		order_seqence.push_back(unfinishorderlist[i].getOrderId());
	
	// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
	unordered_map<string,int> order_loc;
	for(int i = 0;i<veichle_num;i++)
		order_loc[veichles[i]] = i;
	for(int i = 0;i<unfinishorderlist.size();i++)
		order_loc[unfinishorderlist[i].getOrderId()] = i + veichle_num;
	unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
    unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
    vector<node> currentroute_nodelist = global_node_construct(unfinishorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
	vector<vector<double>> node_relation = global_node_distance(currentroute_nodelist);
	double order_tao = initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist,best_currentsolution);
	//double order_tao = 0.005;
	vector<vector<double>> order_tao_relation(problem_size + veichle_num,vector<double>(problem_size + veichle_num,order_tao));
	vector<vector<double>> tao_relation(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
	//initial_order_tao(unfinishorderlist,startsolution,currentroute_nodelist);
	vector<vector<double>> order_eta_matrix = initial_order_eta(unfinishorderlist,startsolution);
	// 作为第几个订单进行处理的信息素概率
    double tao = order_tao;
    vector<int> best_orderlist;
	for (int iter = 0; iter < whole_iteration; iter++) {
        double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
		vector<vector<int>> local_bestpair,local_bestorder;vector<string> best_order_seqence;vector<vector<int>> local_bestlist;
		for(int ant = 0;ant<constuct_ant_num;ant++){
			unordered_map<string,vector<node>> local_solution = startsolution;
			unordered_map<string,BinaryTree> local_tree;
			for(string t:veichles){
				local_tree[t].initRoot(t,capacity);
			}
			vector<string> o_squence = order_seqence_construct(order_seqence,order_loc,order_tao_relation,order_eta_matrix,order_tao,local_update_car,test_q1,eta_index1,tao_index1);
			vector<vector<int>> update_list;
			for(string to_insert:o_squence){
				vector<int> temp_list = tree_construct(local_tree,tao_relation,node_relation,to_insert,currentroute_nodelist,
					order_pnode,order_dnode,test_q2,beta,eta_index2,tao_index2,tao,local_update_car,unfinishorderlist,order_loc,beta2);
				update_list.push_back(temp_list);
			}

			for(int i = 0;i<veichle_num;i++){
				local_solution[veichles[i]] = orderHelper(local_tree[veichles[i]].getRoot(),2,order_pnode,order_dnode,currentroute_nodelist,startsolution[veichles[i]].back(),unfinishorderlist);
			}
			
            double tempcost = f2_totaldistance(local_solution);
            //cout<<tempcost<<" ";
            if(tempcost<local_best_cost3){
                local_best_cost3 = tempcost;
                local_bestsolution3 = local_solution;
				local_bestpair = update_list;
				best_order_seqence = o_squence;
			}
			
		}
        // global update
        
        bool better = 0;
        cout<<local_best_cost3<<" "<<Total_node_count(best_currentsolution)<<" ";
		for(int i = 0;i<veichle_num;i++){
			order_tao_relation[i][order_loc[best_order_seqence[0]]] = (1 - global_update_car)*order_tao_relation[i][order_loc[best_order_seqence[0]]] + global_update_car/local_best_cost3;
		}
		
		for(int i = 1;i<best_order_seqence.size();i++){
			order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] = (1 - global_update_car)*order_tao_relation[order_loc[best_order_seqence[i - 1]]][order_loc[best_order_seqence[i]]] + global_update_car/local_best_cost3;
		}
		
		
		if(local_best_cost3<global_best_cost){
			for(string value:veichles){
				PDlist(local_bestsolution3[value],"fuck");
			}
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
        }

    	/*for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}*/

		for(int i = 0;i<local_bestpair.size();i++){
			for(int j = 0;j<local_bestpair[i].size() - 1;j++){
				if(j == 0)
					tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] = (1 - global_update_car)*tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] + global_update_car/local_best_cost3;
				else{
					tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] = (1 - global_update_beside)*tao_relation[local_bestpair[i][j]][local_bestpair[i][j + 1]] + global_update_beside/local_best_cost3;
				}
			}
		}
		
		int nr = getrangenumber(0,3);
		int anchor = weighted_random(0,unfinishorderlist.size() - 1);
		unordered_map<string,int> order_pnode1,order_dnode1;vector<string> car_orderlist;unordered_map<string,string> order_car;
		getpdglist(order_pnode1,order_dnode1,car_orderlist,best_currentsolution,order_car);
		unordered_map<string,vector<node>> tempsolution;
		switch (nr) {
			case 1:
				local_bestsolution3 = block_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 2:
				local_bestsolution3 = block_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 3:
				local_bestsolution3 = couple_exchange(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
			case 0:
				local_bestsolution3 = couple_relocate(best_currentsolution,order_pnode1,order_dnode1,car_orderlist,anchor,order_car);
				break;
		}

		for(int i = 0;i<veichle_num;i++){
			int last_node = i;
			for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
				if(local_bestsolution3[veichles[i]][j].getpickup()){
					tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}else{
					tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* tao_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
					last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
				}
			}
		}
		local_best_cost3 = total_cost(local_bestsolution3);
		if(local_best_cost3<global_best_cost){
            better = 1;
            global_best_cost = local_best_cost3;
            best_currentsolution = local_bestsolution3;
        }
		cost_num.push_back(global_best_cost);
    }
	//mergesolution(solution,best_currentsolution);
	solution = best_currentsolution;
    writecost(cost_num,"D:\\sci_pap\\project_code\\moead_ts\\temp_cost.csv");
    writesolution("D:\\sci_pap\\project_code\\moead_ts\\check2.csv",solution);
    cost_data.push_back(total_cost(solution));
	cout<<"end: "<<endl;
	std::cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\dpd_lifo_tree.csv",instance);
}
}
#endif