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
static int instance = 1;
// solution 每一列当中相同的节点即使相邻不进行合并
static vector<string> veichles;
static unordered_map<string,int> order_location;

static double weight[8][2] =
{
	{0.001, 0.999},
	{0.2,0.8},
	{0.4,0.6},
	{0.6,0.4},
	{0.8,0.2},
	{0.999, 0.001},
    {0.0,1.0},
    {100.0/36.0,1.0}
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
		cout<<each<<endl;
		for(node value:tempsolution[each]){
			cout<<"current order:"<<value.getorderid();
			if(value.getpickup() == 0&&value.getpropertytime() - value.getarrivetime() < 0)cout<<"delay time:"<<value.getpropertytime() - value.getarrivetime()<<"  ";
			cout<<"pick up"<<value.getpickup();
			cout<<"  "<<value.getdeliverlist().size();
			cout << " arrive " << value.getarrivetime() << "service " << value.getServiceTime() << " leave " << value.getleavetime();
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

//对solution上的每一个时间节点进行设置,当且仅当对已经完全配对的解决方案
// 预设第一个节点存在对应的时间节点，第一个时间点总是已经设定成功的

void Timecaculate(std::unordered_map<std::string, std::vector<node>>& nonsolution) {
}

void Timecaculate(std::unordered_map<std::string, std::vector<node>>& nonsolution,int logo) {
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

// 计算时间，但是工厂是无限制的，不考虑排队
void Timecaculate(std::unordered_map<std::string, std::vector<node>>& nonsolution,string fuck) {
	unordered_map<std::string, int> numpos;
	for (std::string each : veichles) {
        numpos[each] = 0;
    }
	for(string value:veichles){
		int v_size = nonsolution[value].size();
		while(numpos[value] < v_size - 1){
		if (numpos[value] + 1 >= nonsolution[value].size()) {
            continue;
        }
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
		for(int i = numpos[value] - len;i<=numpos[value];i++){
			nonsolution[value][i].setarrivetime(Mix_arrivetime);
			nonsolution[value][i].setServiceTime(sum + Mix_arrivetime + delay);
		}
		}
	}
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
	unordered_map<string,int> divide_order;
	double cost = 0;
	for(string each:veichles){
		vector<node> nodelist = tempsolution[each];
		for(int i = 1;i<nodelist.size();i++){
		if (nodelist[i].getpickup() == 0){
			double tempcost = max(0, (nodelist[i].getarrivetime() - nodelist[i].getpropertytime()));
			if(tempcost > 0){
				if(nodelist[i].getorderid()[0] == 'f'){
					if(divide_order[nodelist[i].getorderid().substr(5)] < tempcost){
						divide_order[nodelist[i].getorderid().substr(5)] = tempcost;
					}
				}else cost += tempcost;
			}
		}
		}
	}
	for(auto it = divide_order.begin();it!=divide_order.end();it++){
		cost +=double(it->second);
	}
	return cost;
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

// 切比雪夫
unordered_map<string, vector<node>> crossover_v2(unordered_map<string, vector<node>> parent1, 
unordered_map<string, vector<node>> parent2,double* w) {
	unordered_map<string, vector<node>> child;
	vector<node> templist;
	vector<orderitem> leftorder;
	unordered_set<string> tempset;

	// 初始化子代结构
	for (string str:veichles) {
		child[str] = vector<node>();
	}
	tempset = current_dealingorders;
	unordered_set<string> before_set;
	// 遍历子代车辆
	vector<string> tempveichles = veichles;
	tempveichles = shuffle_vector(tempveichles);
	for (string vehicle_id:tempveichles) {
		// 随机选择父代
		if (getrandbool()) {
			templist = parent1[vehicle_id];
		} else {
			templist = parent2[vehicle_id];
		}
		
		// 过滤节点
		for(auto it = templist.begin();it!=templist.end();){
			if(before_set.count(it->getorderid()) == 1){
				it = templist.erase(it);
				continue;
			}
			if(it->getpickup() == 0){
				if(tempset.count(it->getorderid()) == 1){
					before_set.insert(it->getorderid());
					tempset.erase(it->getorderid());
				}
			}
			it++;
		}
	
		child[vehicle_id] = templist;
	}
	
	// 收集未处理订单
	for (string order_id : tempset) {
		if (orderlist.find(order_id) != orderlist.end()) {
			leftorder.push_back(orderlist[order_id]);
		}
	}

	// 补全订单
	completeorderwithCI(leftorder, child, w);

	return child;
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

#if 1
// 暂时没有考虑隔天的订单
int main() {
	int whole_iteration = 300;
	int delta = 0.9;
	readtime(); readistance(); readfactory(); 
	for (instance = 2; instance <= 34; instance++) {
		std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
		std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
		orderlist.clear();order.clear();veichles.clear();solution.clear();
		readveichle(instance_veichle); 
		readorder(instance_order,"fuck");
		cout<<"begin"<<endl;
		vector<double> cost_data;
		for (int loop = 0; loop < 20; loop++) {
			current_time = 600;
			initialsolution("fuck");
			vector<orderitem> todoorderlist;
			vector<orderitem> currentorderlist;
			unordered_map<string, vector<node>>currentproblem = solution;
			for (int i = 0; i < order.size(); i++)
				todoorderlist.push_back(order[i]);
			int iter = 0; // 订单处理的轮数
			unordered_map<string, vector<node>> population;
			while (todoorderlist.size() != 0) {
				current_dealingorders.clear();
				reference_point[0] = 0;
				reference_point[1] = 0;
                currentorderlist = getneworders(todoorderlist);
				if(currentorderlist.size() == 0){
					current_time += timeinterval;
					continue;
				}
				getnewproblem(solution,currentproblem);
				for(int i = 0;i<currentorderlist.size();i++)
					current_dealingorders.insert(currentorderlist[i].getOrderId());
				for(string value:veichles){
					for(int i = 1;i<currentproblem[value].size();i++){
						if(currentproblem[value][i].getpickup()){
							current_dealingorders.insert(currentproblem[value][i].getorderid());
						}
					}
				}
				population = currentproblem;
				shuffle_vector(currentorderlist);
				initial_population(population,currentorderlist,weight[6]);
                cout<<f2_totaldistance(population)<<" ";
			for(int iter = 0;iter<whole_iteration;iter++){
				population = Tabu_search(population, "fuck");
                cout<<f2_totaldistance(population)<<" ";
			}
				mergesolution(solution, population);
				// 对解决方案进行融合之后，得到一个较为完整的解之后进行参考点的更新
				current_time += timeinterval;
			}
		double temp_total_cost = total_cost(solution);
		//cout<<f1_timecosuming(solution)*alpha + f2_totaldistance(solution)<<endl;
		cout<<f2_totaldistance(solution)<<" "<<temp_total_cost<<" "<<Total_node_count(solution)<<endl;
		//printsolution(solution);
		}
		
	}
	return 0;
}
#endif

// ant colony system ............................................................................ant colony system //

// 遍历veichles从而对solution进行遍历
double alpha2 = 0.1;
double gama = 0.7; // 信息素采用较大的更新率
double same = 15;
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
	unordered_map<string,int> divide_order;
	double discost = 0;double cost = 0;double timcost = 0;
	for(string each:veichles){
		vector<node> nodelist = tempsolution[each];
		for(int i = 1;i<nodelist.size();i++){
			discost += Distance[location[nodelist[i - 1].getId()]][location[nodelist[i].getId()]];
		if (nodelist[i].getpickup() == 0){
			double tempcost = max(0, (nodelist[i].getarrivetime() - nodelist[i].getpropertytime()));
			if(tempcost > 0){
				if(nodelist[i].getorderid()[0] == 'f'){
					if(divide_order[nodelist[i].getorderid().substr(5)] < tempcost){
						divide_order[nodelist[i].getorderid().substr(5)] = tempcost;
					}
				}else timcost += alpha * tempcost;
			}
		}
		}
	}
	for(auto it = divide_order.begin();it!=divide_order.end();it++){
		timcost +=alpha*double(it->second);
	}
	cost = discost/double(veichle_num) + timcost;
	//if(Total_node_count(solution) == 107)cout<<"cost of total cost"<<discost/double(veichle_num)<<" "<<timcost<<endl;
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
    
        void writeNumbersToCSV(std::vector<double>& numbers,string filename) {
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

vector<vector<double>> initial_eta_veichleorder_relation(unordered_map<string,vector<node>> startsolution,unordered_map<string,int> order_dnode,unordered_map<string,int> order_pnode,
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
            current_car_location = select_node[findMaxIndex(probility_of_nodeselect)];         
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
    double order_tao = 1.0/total_cost(local_solution);
    vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
    return current_car_eta_matrix;
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

// 非lifo插入
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
			temp = finalcost(dealsolution,weight[6]);
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
					toinsertdeliver.push_back(j);
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
					temp = finalcost(dealsolution,weight[6]);
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
		cout<<1/order_tao<<endl;
		order_tao = 0.005;
		vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
		return current_car_eta_matrix;
}

// 不需要满足lifo条件的插入方式
vector<vector<double>> global_node_eta_matrix(vector<orderitem> temporderlist,
	unordered_map<string, vector<node>> imcompletesolution,vector<node> currentroute_nodelist,string logo,string logo2){
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
							//if (deliverjudge(routelist[i].getdeliverlist(), routelist[j].getbeforedeliverlist())) {
								toinsertdeliver.push_back(j);
							//}
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
		cout<<f2_totaldistance(dealsolution)<<" ";
	//	cout<<Total_node_count(dealsolution)<<" "<<total_cost(dealsolution)<<" "<<f1_timecosuming(dealsolution)<<" "<<f2_totaldistance(dealsolution)<<endl;
	//	printsolution(dealsolution);
		/*printfactory(dealsolution,"fuck");
		cout<<"fuck!!!!!1"<<endl;*/
		//cout<<1/order_tao<<endl;
		//order_tao = 0.005;
		vector<vector<double>> current_car_eta_matrix(currentroute_nodelist.size(),vector<double>(currentroute_nodelist.size(),order_tao));
		return current_car_eta_matrix;
}

unordered_map<string, vector<node>> couple_exchange(unordered_map<string, vector<node>> tempsolution,string loge){
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
	return minsolution;
}

unordered_map<string, vector<node>> couple_relocate(unordered_map<string, vector<node>> tempsolution,string logo){
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
	int select_car = getrangenumber(0,veichle_num - 1);
	int select_order = getrangenumber(0,car_orderlist[veichles[select_car]].size() -1);
	if(car_orderlist[veichles[select_car]].size() == 0)return tempsolution;
	orderitem neworder = orderlist[car_orderlist[veichles[select_car]][select_order]];
	string temporder_id = neworder.getOrderId();
	minsolution[veichles[select_car]].erase(remove_if(minsolution[veichles[select_car]].begin(),minsolution[veichles[select_car]].end(),[temporder_id](node value){ return value.getorderid() == temporder_id;}),minsolution[veichles[select_car]].end());
	PDlist(minsolution[veichles[select_car]]);
	insert_order1(minsolution,neworder);
	return minsolution;
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

unordered_map<string, vector<node>> block_exchange(unordered_map<string, vector<node>> tempsolution){
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
	int select_car1 = getrangenumber(0,veichle_num - 1);
	int select_car2 = getrangenumber(0,veichle_num - 1);
	while(select_car2 == select_car1){
		select_car2 = getrangenumber(0,veichle_num - 1);
	}
	int select_order1 = getrangenumber(0,car_orderlist[veichles[select_car1]].size() -1);
	int select_order2 = getrangenumber(0,car_orderlist[veichles[select_car2]].size() - 1);
	if(car_orderlist[veichles[select_car1]].size() == 0||car_orderlist[veichles[select_car2]].size() == 0){
		return tempsolution;
	}
	
	swap_vector_ranges(minsolution[veichles[select_car1]],order_pnode[veichles[select_car1]][car_orderlist[veichles[select_car1]][select_order1]],order_dnode[veichles[select_car1]][car_orderlist[veichles[select_car1]][select_order1]],
	minsolution[veichles[select_car2]],order_pnode[veichles[select_car2]][car_orderlist[veichles[select_car2]][select_order2]],order_dnode[veichles[select_car2]][car_orderlist[veichles[select_car2]][select_order2]]);

	if(PDlist(minsolution[veichles[select_car1]],"fuck")&&PDlist(minsolution[veichles[select_car2]],"fuck")){
		tempsolution = minsolution;
	}
	Timecaculate(tempsolution);
	return tempsolution;
}

unordered_map<string, vector<node>> block_relocate(unordered_map<string, vector<node>> tempsolution,string logo){
	unordered_map<string,unordered_map<string,int>> order_pnode,order_dnode;
	unordered_map<string,vector<string>> car_orderlist;
	unordered_map<string,vector<orderitem>> left_orderlist;
	unordered_map<string,vector<node>> minsolution = tempsolution;
	unordered_map<string,vector<node>> aftersolution;
	double mincost = 100000000000;
	for(string each_car:veichles){
		car_orderlist[each_car];
		order_pnode[each_car];
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
	int select_car = getrangenumber(0,veichle_num - 1);
	int select_order = getrangenumber(0,car_orderlist[veichles[select_car]].size() - 1);
	if(car_orderlist[veichles[select_car]].size() == 0)return tempsolution;
	vector<node> insert_nodelist;
	insert_nodelist = extract_and_remove(minsolution[veichles[select_car]],order_pnode[veichles[select_car]][car_orderlist[veichles[select_car]][select_order]],order_dnode[veichles[select_car]][car_orderlist[veichles[select_car]][select_order]]);
	PDlist(minsolution[veichles[select_car]],"fuck");
	for(string each_car:veichles){
		for(int i = 0; i < minsolution[each_car].size();i++){
			unordered_map<string,vector<node>> middle = minsolution;
			block_insert(middle[each_car],insert_nodelist,i);
			if(PDlist(middle[each_car],"fuck")){
				Timecaculate(middle);
				double tempcost = total_cost(middle);
				if(tempcost<mincost){
					aftersolution = middle;
					mincost = tempcost;
				}
			}
		}
	}
	return aftersolution;
	
}

unordered_map<string, vector<node>> Tabu_search(unordered_map<string, vector<node>>initialsolution) {
	queue<string> Tabulist;
	int node_num = Total_node_count(initialsolution);
	tabulist_size = (node_num < 8) ? 2 : static_cast<int>(std::round(node_num * 0.3));
	string temptransformer,bestneighbor_transform;
	unordered_map<string, vector<node>> currentsolution,tempsolution ,bestsolution, bestneighbor;
	int iter = 0, niter = 0,Maxiter = 20,Neighbortheshold = 8;
	bestsolution = initialsolution; currentsolution = initialsolution;
	double min = total_cost(bestsolution);
	while (iter < Maxiter) {
		bestneighbor = currentsolution;
		niter = 0;
		double min_neighbor = total_cost(bestneighbor);
		while (niter < Neighbortheshold) {
			switch (getrangenumber(1, 4)) {
			case 1:
				tempsolution = block_exchange(currentsolution);
				break;
			case 2:
				tempsolution = block_relocate(currentsolution,"fuck");
				break;
			case 3:
				tempsolution = couple_exchange(currentsolution,"fuck");
				break;
			case 4:
				tempsolution = couple_relocate(currentsolution,"fuck");
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
	double mincost = finalcost(minsolution,weight[6]);
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
			double tempcost = finalcost(localsolution,weight[6]);
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
	double mincost = finalcost(tempsolution,weight[6]);;
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
			int anchor = weighted_random(0,size - 1),nr = getrangenumber(0,1);
			while(book[anchor][nr]){
				anchor = weighted_random(0,size - 1);
				nr = getrangenumber(0,1);
			}
			book[anchor][nr] = true;
			switch (nr) {
			case 1:
				tempsolution = couple_exchange(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			case 0:
				tempsolution = couple_relocate(currentsolution,order_pnode,order_dnode,car_orderlist,anchor,order_car);
				break;
			}
			temptransformer = transform_solution_to_string(tempsolution);
			if (noTabu(temptransformer,Tabulist)) {
				double tempmin = finalcost(tempsolution,weight[6]);
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

// 路径上的节点之间的信息进行共享，创建一个大的信息素队列
#if 0
int main() {
    int whole_iteration = 60;
    int constuct_ant_num = 5;
    double local_update_order = 0.1;
    double local_update_car = 0.1;
    double global_update_order = 0.05;
    double global_update_car = 0.05;
    double node_select = 1;
    double order_select = 0;
    double test_q_order = 0.7;
    double test_q_node = 0.7;
	double local_search_update = 0.;
	double time_factor = 0.0;
	readtime(); readistance(); readfactory();
	for(int instance = 1;instance <= 16;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	//solution.clear();
    unordered_map<string,vector<orderitem>> problem;
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
        vector<double> cost_num;
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
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
		
        unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
        vector<node> currentroute_nodelist = global_node_construct(currentorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
        vector<vector<double>> currentroute_node_relation = global_node_matrix(currentroute_nodelist);
      // vector<vector<double>> currentroute_eta_relation = global_node_eta_matrix(startsolution,order_dnode,order_pnode,veichle_order_relation,currentorderlist,currentroute_nodelist,currentroute_node_relation);
        vector<vector<double>>currentroute_eta_relation = global_node_eta_matrix(currentorderlist,startsolution,currentroute_nodelist);
		vector<double> currentroute_time_relation = global_time_matrix(currentroute_nodelist);
	   // vector<vector<double>>eta_veichleorder_relation(veichle_num, vector<double>(currentorderlist.size(), currentroute_eta_relation[0][0]*double(currentroute_nodelist.size())/double(currentorderlist.size())));
		vector<vector<double>>eta_veichleorder_relation(veichle_num, vector<double>(currentorderlist.size(), currentroute_eta_relation[0][0]));
		double order_tao = eta_veichleorder_relation[0][0];
        double car_tao = currentroute_eta_relation[0][0];
		
		for (int iter = 0; iter < whole_iteration; iter++) {
            unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
			for(int ant = 0;ant<constuct_ant_num;ant++){
			    unordered_map<string, vector<orderitem>> unordered_solution;
			   // unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
                unordered_map<string, vector<orderitem>> order_set;
	            for (string temp : veichles) {
		            order_set[temp];
	            }
	            for (int i = 0; i < currentorderlist.size(); i++) {
			        vector<double> temp_problity; int location = 0; double compare = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select);
			        temp_problity.push_back(eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select));
			        double sum = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select);
			        for (int j = 1; j < veichle_num; j++) {
				        if (compare > eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select)) {
					     location = j; compare = eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select);
				        }
				        sum += eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select);
				        temp_problity.push_back(eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select));
			    }
			    if (generateRandomNumber() <= test_q_order) {
				    order_set[veichles[location]].push_back(currentorderlist[i]);
			    }
			    else {
				    for (int j = 0; j < veichle_num; j++)temp_problity[j] /= sum;
				    order_set[veichles[wheel_select(generateRandomNumber(), temp_problity,"fuck33")]].push_back(currentorderlist[i]);
			    }
	            }
                unordered_solution = order_set;
			    for (int i = 0; i < veichle_num; i++) {
				    for (orderitem temp_order : unordered_solution[veichles[i]]) {
					    eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - local_update_order) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + local_update_order * order_tao;
				    }
			    }
                unordered_map<string,vector<node>> local_solution = startsolution;
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
                            sum += pow(currentroute_node_relation[current_car_location][i],node_select)*currentroute_eta_relation[current_car_location][i]*pow(currentroute_time_relation[i],time_factor);
                            probility_of_nodeselect.push_back(pow(currentroute_node_relation[current_car_location][i],node_select)*currentroute_eta_relation[current_car_location][i]*pow(currentroute_time_relation[i],time_factor));
                        }
                        for (int i = 0; i < probility_of_nodeselect.size(); i++) {
                            probility_of_nodeselect[i] /= sum;
                        }
                        last_node = current_car_location;
                        if (generateRandomNumber() <= test_q_node) {
                            current_car_location = select_node[findMaxIndex(probility_of_nodeselect)];
                        }
                        else current_car_location = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
                        currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*car_tao;
                        if (currentroute_nodelist[current_car_location].getpickup()) {
                            int templocation = current_car_location;
                            currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
                        }
                        local_solution[topvalue].push_back(currentroute_nodelist[current_car_location]);
                        PDlist_next(local_solution[topvalue], "fuck333");
                        
                    }
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
                        currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*car_tao;
                        PDlist_next(local_solution[topvalue], "fuck444");
                        left_orderlist.pop_back();
                    }
            
                }
                Timecaculate(local_solution);
                double tempcost = total_cost(local_solution);
                if(tempcost<local_best_cost3){
                    local_bestproblem1 = unordered_solution;
                    local_best_cost3 = tempcost;
                    local_bestsolution3 = local_solution;
                }
            }
            // global update
            /*for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_bestproblem1[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - global_update_order)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + global_update_order/local_best_cost3;
				}
			}*/
			
            /*for(int i = 0;i<veichle_num;i++){
                int last_node = i;
                for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
                    if(local_bestsolution3[veichles[i]][j].getpickup()){
                        currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                        last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }else{
                        currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                        last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }
                }
            }*/
			if(local_best_cost3<global_best_cost){
                global_best_cost = local_best_cost3;
                best_currentsolution = local_bestsolution3;
            }
			unordered_map<string,vector<node>> local_solution = local_bestsolution3;
			unordered_map<string,vector<orderitem>> local_search_problem = couple_exchange(local_solution);
			local_bestsolution3 = local_solution;
			local_best_cost3 = total_cost(local_solution);
			for (int i = 0; i < veichle_num; i++) {
				local_search_problem[veichles[i]];
				for (orderitem temp_order : local_search_problem[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - local_search_update)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + local_search_update/local_best_cost3;
				}
			}
			for(int i = 0;i<veichle_num;i++){
                int last_node = i;
                for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
                    if(local_bestsolution3[veichles[i]][j].getpickup()){
                        currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - local_search_update)* currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_search_update/local_best_cost3;
                        last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }else{
                        currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - local_search_update)* currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_search_update/local_best_cost3;
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
    cost_data.push_back(total_cost(solution));
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\test_moead_data1.csv");
}
}
#endif

// 进行禁忌搜索
#if 0
int main() {
    int whole_iteration = 60;
    int constuct_ant_num = 5;
    double local_update_order = 0.1;
    double local_update_car = 0.1;
    double global_update_order = 0.05;
    double global_update_car = 0.05;
    double node_select = 1;
    double order_select = 0;
    double test_q_order = 0.7;
    double test_q_node = 0.7;
	double local_search_update = 0.1;
	double time_factor = 0.0;
	readtime(); readistance(); readfactory();
	for(int instance = 5;instance <= 20;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	readorder(instance_order);
	cout<<"begin"<<endl;
	vector<double> cost_data;
	for(int example = 0;example<20;example++){
	//solution.clear();
    unordered_map<string,vector<orderitem>> problem;
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
        vector<double> cost_num;
		//cout<<unfinishorderlist.size()<<endl;
		// orderlist 前面部分都是具有取货点和送货点，后面部分都是仅仅只含有送货结点没有取货点
		unordered_map<string, int> currentorder_location;
		unordered_map<string,vector<node>> best_currentsolution = startsolution;double global_best_cost = 100000000000;unordered_map<string,vector<orderitem>> best_currentproblem;
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
		
        unordered_map<string,int> order_pnode,order_dnode;vector<int> exist_pickup;
        vector<node> currentroute_nodelist = global_node_construct(currentorderlist,order_dnode,order_pnode,exist_pickup,startsolution);
        vector<vector<double>> currentroute_node_relation = global_node_matrix(currentroute_nodelist);
      // vector<vector<double>> currentroute_eta_relation = global_node_eta_matrix(startsolution,order_dnode,order_pnode,veichle_order_relation,currentorderlist,currentroute_nodelist,currentroute_node_relation);
        vector<vector<double>>currentroute_eta_relation = global_node_eta_matrix(currentorderlist,startsolution,currentroute_nodelist);
		vector<double> currentroute_time_relation = global_time_matrix(currentroute_nodelist);
	   // vector<vector<double>>eta_veichleorder_relation(veichle_num, vector<double>(currentorderlist.size(), currentroute_eta_relation[0][0]*double(currentroute_nodelist.size())/double(currentorderlist.size())));
		vector<vector<double>>eta_veichleorder_relation(veichle_num, vector<double>(currentorderlist.size(), currentroute_eta_relation[0][0]));
		double order_tao = eta_veichleorder_relation[0][0];
        double car_tao = currentroute_eta_relation[0][0];
		
		for (int iter = 0; iter < whole_iteration; iter++) {
            unordered_map<string, vector<orderitem>> local_bestproblem1; double local_best_cost3 = 10000000000;unordered_map<string,vector<node>>local_bestsolution3;
			for(int ant = 0;ant<constuct_ant_num;ant++){
			    unordered_map<string, vector<orderitem>> unordered_solution;
			   // unordered_solution = allocate_order(veichle_order_relation, order_relation, currentorderlist, eta_veichleorder_relation);
                unordered_map<string, vector<orderitem>> order_set;
	            for (string temp : veichles) {
		            order_set[temp];
	            }
	            for (int i = 0; i < currentorderlist.size(); i++) {
			        vector<double> temp_problity; int location = 0; double compare = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select);
			        temp_problity.push_back(eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select));
			        double sum = eta_veichleorder_relation[0][i] * pow(veichle_order_relation[0][i], order_select);
			        for (int j = 1; j < veichle_num; j++) {
				        if (compare > eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select)) {
					     location = j; compare = eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select);
				        }
				        sum += eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select);
				        temp_problity.push_back(eta_veichleorder_relation[j][i] * pow(veichle_order_relation[j][i], order_select));
			    }
			    if (generateRandomNumber() <= test_q_order) {
				    order_set[veichles[location]].push_back(currentorderlist[i]);
			    }
			    else {
				    for (int j = 0; j < veichle_num; j++)temp_problity[j] /= sum;
				    order_set[veichles[wheel_select(generateRandomNumber(), temp_problity,"fuck33")]].push_back(currentorderlist[i]);
			    }
	            }
                unordered_solution = order_set;
			    for (int i = 0; i < veichle_num; i++) {
				    for (orderitem temp_order : unordered_solution[veichles[i]]) {
					    eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - local_update_order) * eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + local_update_order * order_tao;
				    }
			    }
                unordered_map<string,vector<node>> local_solution = startsolution;
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
                            sum += pow(currentroute_node_relation[current_car_location][i],node_select)*currentroute_eta_relation[current_car_location][i]*pow(currentroute_time_relation[i],time_factor);
                            probility_of_nodeselect.push_back(pow(currentroute_node_relation[current_car_location][i],node_select)*currentroute_eta_relation[current_car_location][i]*pow(currentroute_time_relation[i],time_factor));
                        }
                        for (int i = 0; i < probility_of_nodeselect.size(); i++) {
                            probility_of_nodeselect[i] /= sum;
                        }
                        last_node = current_car_location;
                        if (generateRandomNumber() <= test_q_node) {
                            current_car_location = select_node[findMaxIndex(probility_of_nodeselect)];
                        }
                        else current_car_location = select_node[wheel_select(generateRandomNumber(), probility_of_nodeselect,"fuck44")];
                        currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*car_tao;
                        if (currentroute_nodelist[current_car_location].getpickup()) {
                            int templocation = current_car_location;
                            currentroute_exist[topvalue].erase(std::remove_if(currentroute_exist[topvalue].begin(), currentroute_exist[topvalue].end(), [templocation](int value) { return value == templocation; }),currentroute_exist[topvalue].end());
                        }
                        local_solution[topvalue].push_back(currentroute_nodelist[current_car_location]);
                        PDlist_next(local_solution[topvalue], "fuck333");
                        
                    }
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
                        currentroute_eta_relation[last_node][current_car_location] = (1 - local_update_car)*currentroute_eta_relation[last_node][current_car_location] + local_update_car*car_tao;
                        PDlist_next(local_solution[topvalue], "fuck444");
                        left_orderlist.pop_back();
                    }
            
                }
			//	Tabu_search(local_solution,10);
                Timecaculate(local_solution);
                double tempcost = total_cost(local_solution);
                if(tempcost<local_best_cost3){
                    local_bestproblem1 = unordered_solution;
                    local_best_cost3 = tempcost;
                    local_bestsolution3 = local_solution;
                }
            }
            // global update
            /*for (int i = 0; i < veichle_num; i++) {
				for (orderitem temp_order : local_bestproblem1[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - global_update_order)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + global_update_order/local_best_cost3;
				}
			}
			
            for(int i = 0;i<veichle_num;i++){
                int last_node = i;
                for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
                    if(local_bestsolution3[veichles[i]][j].getpickup()){
                        currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                        last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }else{
                        currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - global_update_car)* currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + global_update_car/local_best_cost3;
                        last_node = order_dnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }
                }
            }
			if(local_best_cost3<global_best_cost){
                global_best_cost = local_best_cost3;
                best_currentsolution = local_bestsolution3;
            }*/
			unordered_map<string,vector<node>> local_solution = local_bestsolution3;
			local_solution = Tabu_search(local_bestsolution3,"fuck");
		//	local_solution = Tabu_search(local_bestsolution3,1);
			/*unordered_map<string,int> order_pnode,order_dnode;
			vector<string> car_orderlist;unordered_map<string,string> order_car;
			getpdglist(order_pnode,order_dnode,car_orderlist,local_bestsolution3,order_car);
			int anchor = getrangenumber(0,car_orderlist.size() - 1);*/
		//	local_solution = block_exchange(local_bestsolution3,order_pnode,order_dnode,car_orderlist,anchor,order_car);

		//	local_solution = block_relocate(local_bestsolution3,order_pnode,order_dnode,car_orderlist,anchor,order_car);

		//	local_solution = couple_exchange(local_bestsolution3,order_pnode,order_dnode,car_orderlist,anchor,order_car);

		//	local_solution = couple_relocate(local_bestsolution3,order_pnode,order_dnode,car_orderlist,anchor,order_car);

		//	local_solution = block_exchange(local_bestsolution3);
		//	local_solution = block_relocate(local_bestsolution3,"fuck");
		//	local_solution = couple_exchange(local_bestsolution3,"fuck");
		//	local_solution = couple_relocate(local_bestsolution3,"fuck");
			unordered_map<string,vector<orderitem>> local_search_problem = reloadto_problem(local_solution);
			local_bestsolution3 = local_solution;
			local_best_cost3 = total_cost(local_solution);
			for (int i = 0; i < veichle_num; i++) {
				local_search_problem[veichles[i]];
				for (orderitem temp_order : local_search_problem[veichles[i]]) {
					eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] = (1 - local_search_update)* eta_veichleorder_relation[i][currentorder_location[temp_order.getOrderId()]] + local_search_update/local_best_cost3;
				}
			}
			for(int i = 0;i<veichle_num;i++){
                int last_node = i;
                for(int j = 1;j<local_bestsolution3[veichles[i]].size();j++){
                    if(local_bestsolution3[veichles[i]][j].getpickup()){
                        currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - local_search_update)* currentroute_eta_relation[last_node][order_pnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_search_update/local_best_cost3;
                        last_node = order_pnode[local_bestsolution3[veichles[i]][j].getorderid()];
                    }else{
                        currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] = (1 - local_search_update)* currentroute_eta_relation[last_node][order_dnode[local_bestsolution3[veichles[i]][j].getorderid()]] + local_search_update/local_best_cost3;
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
    cost_data.push_back(total_cost(solution));
	cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<endl;
	/*if(Total_node_count(solution) != 107){
		printsolution(solution);
		cout<<"fuck"<<endl;
	}*/
	/*if(total_cost(solution) > 130){
		printsolution(solution);
		system("pause");
	}*/
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\test_moead_data1.csv");
}
}
#endif

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

int selectElement1D(const std::vector<double>& probArray, double test_q_order) {
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

// 采用含有时间以及储存量的信息素矩阵
// 不分配车辆，尝试解决，车辆在选取过程当中由于数量趋势自然所导致的多选择取货点
// 确定插入或者是构建顺序进行构建，对插入的顺序进行优化
#if 0
int main(){
	int whole_iteration = 50;
    int constuct_ant_num = 6;
    double local_update_car = 0.1;
    double global_update_car = 0.1;
    double node_select = 1.5;
	double node_select_eta = 1;
    double test_q_node = 0.85;
	double gamma3 = 0.1;
	double local_search_update = 0.1;
	readtime(); readistance(); readfactory();
	for(int instance = 2;instance <= 2;instance++){
    std::string instance_veichle = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\vehicle_info.csv";
    std::string instance_order = "D:\\sci_pap\\project_code\\moead_ts\\benchmark\\instance_" + to_string(instance) + "\\" + to_string(instance) + ".csv";
    orderlist.clear();order.clear();veichles.clear();solution.clear();
	readveichle(instance_veichle); 
	//readveichle("fick","dick");
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
        vector<vector<double>>currentroute_eta_relation = global_node_eta_matrix(currentorderlist,startsolution,currentroute_nodelist,"fuck","nolifo");
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
							for(int u = 0;u<local_solution[veichles[i]].back().getdeliverlist().size();u++)
								select_node[i].push_back(order_dnode[local_solution[veichles[i]].back().getdeliverlist()[u]]);
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
                double tempcost = total_cost(local_solution);
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
    cost_data.push_back(total_cost(solution));
	//printfactory(solution);
	printfactory(solution,"fuck");
	cout<<Total_node_count(solution)<<" "<<total_cost(solution)<<" "<<f2_totaldistance(solution)<<endl;
	}
    writeNumbersToCSV(cost_data,"D:\\sci_pap\\project_code\\moead_ts\\hybrid_pheremone.csv");
}
}
#endif

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

