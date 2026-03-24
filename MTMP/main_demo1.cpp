#include<iostream>
#include<math.h>
#include<unordered_map>
#include<unordered_set>
#include<fstream>
#include<sstream>
#include<random>
#include<iomanip>
#include<chrono>
#include<string>
#include<vector>
#include <cctype>
#include <algorithm>

using namespace std;
static int cities_size;
static int veichles_size = 2;// 3 5
static double Max_dis = 10000;

void read_info(std::vector<double>& x, 
               std::vector<double>& y, 
               std::vector<int>& ins1,
               std::vector<int>& ins2,
               std::vector<int>& ins3,
               std::string& location) {
    
    std::ifstream file(location);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << location << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过标记行和空行
        if (line.find('<') != std::string::npos || line.empty()) {
            continue;
        }

        // 创建一个新的字符串流，并替换逗号为空格
        std::string clean_line;
        for (char c : line) {
            // 保留数字、小数点和负号（如果文件中有负数的话）
            if (std::isdigit(c) || c == '.' || c == ',' || c == '-') {
                // 将逗号替换为空格
                if (c == ',') {
                    clean_line += ' ';
                } else {
                    clean_line += c;
                }
            }
            // 其他字符直接忽略
        }

        std::istringstream iss(clean_line);
        int index;
        double x_val, y_val;
        int i1, i2, i3;

        // 尝试解析数据
        if (iss >> index >> x_val >> y_val >> i1 >> i2 >> i3) {
            x.push_back(x_val);
            y.push_back(y_val);
            ins1.push_back(i1);
            ins2.push_back(i2);
            ins3.push_back(i3);
        } else {
            // 更详细的错误信息
            std::cerr << "解析失败: " << line << std::endl;
            std::cerr << "清洗后: " << clean_line << std::endl;
        }
    }
}

vector<int> shuffle_vector(std::vector<int>& input, 
                               std::mt19937* custom_rng = nullptr) {
    if (input.empty()) return input;
    
    std::vector<int> shuffled = input;
    
    if (custom_rng) {
        // 使用提供的随机引擎
        shuffle(shuffled.begin(), shuffled.end(), *custom_rng);
    } else {
        // 使用默认随机引擎
        static std::random_device rd;
        static std::mt19937 default_rng(rd());
        shuffle(shuffled.begin(), shuffled.end(), default_rng);
    }
    
    return shuffled;
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

void global_update_3D(vector<vector<vector<double>>> &m,vector<vector<int>> s,double tao,double p){
    tao = 1.0/tao;

    for(int i = 0;i<m.size();i++){
        for(int j = 0;j<m[i].size();j++){
            for(int k = 0;k<m[i][j].size();k++){
                m[i][j][k] = (1 - p)*m[i][j][k];
            }
        }
    }

    for(int i = 0;i<m.size();i++){
        for(int j = 1;j<s[i].size();j++){
            m[i][s[i][j - 1]][s[i][j]] = m[i][s[i][j - 1]][s[i][j]] + p*tao;
            //m[i][s[i][j - 1]][s[i][j]] = m[i][s[i][j - 1]][s[i][j]] + p*tao;
            m[i][s[i][j]][s[i][j - 1]] = m[i][s[i][j - 1]][s[i][j]];
        }
    }
}

void global_update_2D(vector<vector<double>> &m,vector<vector<int>> s,double tao,double p){
    tao = 1.0/tao;
    for(int i = 0;i<m.size();i++){
        for(int j = 0;j<m[i].size();j++){
            m[i][j] = (1.0 - p)*m[i][j];
        }
    }
    for(int i = 0;i<s.size();i++){
        for(int j = 1;j<s[i].size();j++){
            m[s[i][j - 1]][s[i][j]] =m[s[i][j - 1]][s[i][j]] + p*tao;
            m[s[i][j]][s[i][j - 1]] = m[s[i][j - 1]][s[i][j]];
        }
    }

    /*for(int i = 0;i<s.size();i++){
        for(int j = 1;j<s[i].size();j++){
            m[s[i][j - 1]][s[i][j]] = (1-p)*m[s[i][j - 1]][s[i][j]] + p*tao;
            m[s[i][j]][s[i][j - 1]] = m[s[i][j - 1]][s[i][j]];
        }
    }*/
}

double total_cost(vector<vector<int>> s,vector<vector<double>> distance){
    double cost = 0;
    for(int i = 0;i<s.size();i++){
        for(int j = 1;j<s[i].size();j++){
            cost += distance[s[i][j - 1]][s[i][j]];
        }
    }
    return cost;
}

double single_cost(vector<int> s,vector<vector<double>> distance){
    double cost = 0;
    for(int j = 1;j<s.size();j++){
        cost += distance[s[j - 1]][s[j]];
    }
    return cost;
}

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

int wheel_select(double p,vector<double> test) {
	if (test.size() == 0)cout << "nothing" << endl;
	for (int i = 1; i < test.size(); i++) {
		test[i] += test[i - 1];
	}
	if (p >= 0 && p < test[0])return 0;

	for (int i = 1; i < test.size(); i++) {
		if(p>test[i - 1]&&p<=test[i])return i;
	}
	return 0;
}

vector<int> generate_random_numbers(int n, int min_val, int max_val) {
    vector<int> temp,result;
    for(int i = min_val;i<=max_val;i++){
        temp.push_back(i);
    }
    temp = shuffle_vector(temp);
    for(int i = 0;i<n;i++){
        result.push_back(temp[i]);
    }
    return result;
}

vector<int> generate_random_numbers_ins(int n, int min_val, int max_val,vector<int> ins,unordered_map<int,int> mp) {
    vector<int> temp,result;
    for(int i = min_val;i<=max_val;i++){
        temp.push_back(i);
    }
    temp = shuffle_vector(temp);

    while(result.size()!=n){
        bool j = 0;
        for(int i = 0;i<result.size();i++){
            if(mp[temp.back()] == mp[result[i]]){
                j = 1;break;
            }
        }
        if(!j){
            result.push_back(temp.back());
            temp.pop_back();
        }else{
            temp.pop_back();
        }
    }

    return result;
}

vector<vector<int>> trans_solution(vector<vector<int>> s,unordered_map<int,int> n){

    for(int i = 0;i<s.size();i++){
        for(int j = 0;j<s[i].size();j++){
            s[i][j] = n[s[i][j]];
        }
    }

    return s;
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
                double val = row[j];
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
            for(int i = 0;i<probMatrix.size();i++){
                for(int j = 0;j<probMatrix[i].size();j++){
                    cout<<probMatrix[i][j]<<" ";
                }
                cout<<endl;
            }
            cout<<total<<endl;
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

void printsolution(vector<vector<int>> s){
    for(int i = 0;i<s.size();i++){
        cout<<"veichle"<<i<<": ";
        for(int j = 0;j<s[i].size();j++){
            cout<<s[i][j]<<" ";
        }
        cout<<endl;
    }
}

vector<vector<int>> node_exchange(vector<vector<int>>s,vector<vector<double>> distance){
    double temp_cost = 10000000000.0;
    vector<vector<int>> reslut = s;
    for(int i = 0;i<s.size();i++){
        double cost = single_cost(s[i],distance);
        for(int j = 1;j<s[i].size() - 1;j++){
            for(int k = j + 1;k<s[i].size() - 1;k++){
                vector<int> temp;
                temp = s[i];
                temp[j] = s[i][k];
                temp[k] = s[i][j];
                temp_cost = single_cost(temp,distance);
                if(temp_cost<cost){
                    cost = temp_cost;
                    reslut[i] = temp;
                }
            }
        }
    }
    return reslut;
}

vector<vector<int>> node_relocate(vector<vector<int>>s,vector<vector<double>> distance){
    vector<vector<int>> reslut = s;
    for(int i = 0;i<s.size();i++){
        double min_cost = single_cost(s[i],distance);
        for(int j = 1;j<s[i].size() - 1;j++){
            vector<vector<int>> temp;
            int node_to_move = s[i][j];
            vector<int> removed_sequence;
            int n = s[i].size();
            for (int k = 0; k < n; k++) {
                if (k != j) removed_sequence.push_back(s[i][k]);
            }
            for (int k = 1; k < n - 1; k++) {
                if (k == j) continue; // 如果新位置和原位置相同则跳过

                std::vector<int> new_sequence = removed_sequence;
                new_sequence.insert(new_sequence.begin() + k, node_to_move);

                double current_cost = single_cost(new_sequence, distance);

                if (current_cost < min_cost) {
                    min_cost = current_cost;
                    reslut[i] = new_sequence;
                }
            }
        }
    }
    return reslut;
}

vector<vector<int>> tabu_search(vector<vector<int>> solution, 
                                vector<vector<double>> distance, 
                                vector<int> ins, 
                                int tabu_tenure, 
                                int max_iter) {
    // 初始化
    vector<vector<int>> best_solution = solution;
    double best_cost = total_cost(solution, distance);
    vector<vector<int>> current_solution = solution;
    double current_cost = best_cost;
    
    // 禁忌表：存储<操作签名, 过期迭代>
    unordered_map<string, int> tabu_list;
    // 操作签名格式： 
    // 交换操作："swap_car_index_i_j"
    // 重定位操作："relocate_car_index_pos"

    for (int iter = 0; iter < max_iter; iter++) {
        vector<vector<int>> neighbor_solution;
        double neighbor_cost = 100000000000.0;
        string best_move_signature;

        // 1. 生成邻域解
        // 1.1 节点交换操作
        for (int car = 0; car < veichles_size; car++) {
            for (int i = 1; i < current_solution[car].size()-1; i++) {
                for (int j = i+1; j < current_solution[car].size()-1; j++) {
                    // 跳过禁忌操作
                    string signature = "swap_" + to_string(car) + "_" + 
                                      to_string(i) + "_" + to_string(j);
                    if (tabu_list.find(signature) != tabu_list.end() && 
                        tabu_list[signature] > iter) continue;

                    // 尝试交换
                    vector<vector<int>> temp = current_solution;
                    swap(temp[car][i], temp[car][j]);
                    double cost = total_cost(temp, distance);

                    // 更新最佳邻域解
                    if (cost < neighbor_cost) {
                        neighbor_solution = temp;
                        neighbor_cost = cost;
                        best_move_signature = signature;
                    }
                }
            }
        }

        // 1.2 节点重定位操作
        for (int car = 0; car < veichles_size; car++) {
            for (int i = 1; i < current_solution[car].size()-1; i++) {
                for (int new_pos = 1; new_pos < current_solution[car].size()-1; new_pos++) {
                    if (i == new_pos) continue;
                    
                    string signature = "relocate_" + to_string(car) + "_" + 
                                      to_string(i) + "_" + to_string(new_pos);
                    if (tabu_list.find(signature) != tabu_list.end() && 
                        tabu_list[signature] > iter) continue;

                    vector<vector<int>> temp = current_solution;
                    int node = temp[car][i];
                    temp[car].erase(temp[car].begin() + i);
                    temp[car].insert(temp[car].begin() + new_pos, node);
                    double cost = total_cost(temp, distance);

                    if (cost < neighbor_cost) {
                        neighbor_solution = temp;
                        neighbor_cost = cost;
                        best_move_signature = signature;
                    }
                }
            }
        }

        // 2. 更新当前解
        current_solution = neighbor_solution;
        current_cost = neighbor_cost;
        
        // 3. 更新禁忌表
        tabu_list[best_move_signature] = iter + tabu_tenure;
        
        // 4. 更新历史最优（特赦准则）
        if (current_cost < best_cost) {
            best_solution = current_solution;
            best_cost = current_cost;
        }
        
        // 5. 定期清理过期禁忌表
        if (iter % 100 == 0) {
            for (auto it = tabu_list.begin(); it != tabu_list.end(); ) {
                if (it->second <= iter) {
                    it = tabu_list.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    return best_solution;
}

vector<vector<int>> two_opt_optimize(vector<vector<int>> solution, 
                                    vector<vector<double>> distance) {
    vector<vector<int>> optimized = solution;
    
    for (int car = 0; car < veichles_size; car++) {
        bool improvement = true;
        while (improvement) {
            improvement = false;
            double best_delta = 0;
            int best_i = -1, best_j = -1;
            
            // 遍历所有可能的边交换
            for (int i = 1; i < optimized[car].size() - 2; i++) {
                for (int j = i + 1; j < optimized[car].size() - 1; j++) {
                    // 计算成本变化
                    double delta = 
                        - distance[optimized[car][i-1]][optimized[car][i]]
                        - distance[optimized[car][j]][optimized[car][j+1]]
                        + distance[optimized[car][i-1]][optimized[car][j]]
                        + distance[optimized[car][i]][optimized[car][j+1]];
                    
                    // 记录最佳改进
                    if (delta < best_delta) {
                        best_delta = delta;
                        best_i = i;
                        best_j = j;
                    }
                }
            }
            
            // 应用最佳改进
            if (best_delta < -0.001) {
                reverse(optimized[car].begin() + best_i, 
                        optimized[car].begin() + best_j + 1);
                improvement = true;
            }
        }
    }
    return optimized;
}

vector<vector<int>> greedy_alg(vector<vector<double>> distance,vector<int> start_loc,vector<int> exist_ins){
    
    vector<vector<int>> temp_solution(veichles_size);double cost = 0;
    unordered_set<int> left_cities;int cities = cities_size;
    for(int i = 0;i<cities;i++)
        left_cities.insert(i);
    vector<int> now_loc = start_loc;
    vector<unordered_set<int>> feasible_option(veichles_size);
    for(int i = 0;i<veichles_size;i++){
        temp_solution[i].push_back(start_loc[i]);
        exist_ins[start_loc[i]] = exist_ins[start_loc[i]] - 1;
        if(exist_ins[start_loc[i]] == 0)
            left_cities.erase(start_loc[i]);
        for(int j = 0;j<cities;j++){
            if(j == start_loc[i])continue;
            feasible_option[i].insert(j);
        }
    }
    while(left_cities.size()!=0){
        vector<vector<double>> probility(veichles_size);
        for(int car = 0;car<veichles_size;car++){
            for(int i = 0;i<cities;i++){
                if(exist_ins[i] == 0){
                        probility[car].push_back(0);
                }else{
                    if(feasible_option[car].count(i) == 0){
                        probility[car].push_back(0);
                    }else {
                        probility[car].push_back(1.0/distance[now_loc[car]][i]);
                    }
                }
            }
        }
        pair<int,int> selected_loc = selectElement(probility,1);
        temp_solution[selected_loc.first].push_back(selected_loc.second);
        feasible_option[selected_loc.first].erase(selected_loc.second);
        exist_ins[selected_loc.second] = exist_ins[selected_loc.second] - 1;
        if(exist_ins[selected_loc.second] == 0){
            left_cities.erase(selected_loc.second);
        }
        now_loc[selected_loc.first] = selected_loc.second;
    }
    for(int i = 0;i<veichles_size;i++){
        temp_solution[i].push_back(start_loc[i]);
    }
    cost = total_cost(temp_solution,distance);
    cout<<"greedy cost: "<<cost<<endl;
    return temp_solution;
}

vector<vector<double>> nomralize_2D(vector<vector<double>>&m,double tao){
    double sum = 0;int count = 0;
    for(int i = 0;i<m.size();i++){
        for(int j = 0;j<m[i].size();j++){
            sum += m[i][j];count++;
        }
    }
    for(int i = 0;i<m.size();i++){
        for(int j = 0;j<m[i].size();j++){
            m[i][j] = m[i][j]*count*tao/sum;
        }
    }
    return m;
}

vector<vector<int>> sample_2D(int n){
    vector<int> x,y;
    x = generate_random_numbers(n,0,cities_size - 1);
    y = generate_random_numbers(n,0,cities_size - 1);
    vector<vector<int>> back;
    for(int i = 0;i<n;i++){
        vector<int> temp;
        temp.push_back(x[i]);
        temp.push_back(y[i]);
        back.push_back(temp);
    }
    return back;
}

vector<vector<double>> mutation_matrix_2D(vector<vector<double>>&node_tao,vector<vector<double>>distance,int n,double tao){
    vector<vector<int>> mutation_dot = sample_2D(n);
    for(int i = 0;i<mutation_dot.size();i++){
        node_tao[mutation_dot[i][0]][mutation_dot[i][1]] = node_tao[mutation_dot[i][0]][mutation_dot[i][1]] + tao*distance[mutation_dot[i][0]][mutation_dot[i][1]]/Max_dis;
    }
    return node_tao;
}

static int ant_size = 5;
static int iter_size = 10000;
static double q1 = 0.9;
static double q2 = 0.8;
static double g_rate1 = 0.2;
static double l_rate1 = 0.2;
static double g_rate2 = 0.2;
static double l_rate2 = 0.2;
static double index_tao = 1;
static double index_eta = 2;
static int mode = 3;
static double mutation_update = 0.05;

vector<vector<double>> mutation_update_matrix_2D(vector<vector<double>>&node_tao,vector<vector<double>> distance,int n,double tao){
    vector<vector<int>> mutation_dot = sample_2D(n);
    for(int i = 0;i<mutation_dot.size();i++){
        node_tao[mutation_dot[i][0]][mutation_dot[i][1]] = (1-mutation_update)*node_tao[mutation_dot[i][0]][mutation_dot[i][1]] + mutation_update*tao;
    }
    return node_tao;
}

#if 0
int main(){
    
    std::vector<double> x,y; std::vector<int>ins1,ins2,ins3,ins;
    string location = "D:\\sci_pap\\project_code\\MTMP\\MTMP_data.csv";
    read_info(x,y,ins1,ins2,ins3,location);
    switch (mode)
    {
    case 1:
        veichles_size = 2;
        ins = ins1;
        break;
    case 2:
        veichles_size = 3;
        ins = ins2; 
        break;
    case 3:
        veichles_size = 5;
        ins = ins3;
        break;
    default:
        break;
    }
    vector<vector<double>> distance(x.size(),vector<double>(y.size(),0));
    cities_size = x.size();
    
    for(int i = 0;i<ins1.size();i++){
        for(int j = 0;j<i;j++){
            distance[i][j] = sqrt(pow((x[i] - x[j]),2)+pow((y[i] - y[j]),2));
            distance[j][i] = distance[i][j];
        }
    }
    
    
    vector<int> start_loc = generate_random_numbers(veichles_size,0,cities_size - 1);
    vector<vector<int>> greedy_solution = greedy_alg(distance,start_loc,ins);
    double tao = 1.0/total_cost(greedy_solution,distance);
    vector<vector<vector<double>>> node_tao(veichles_size,vector<vector<double>>(cities_size,vector<double>(cities_size,tao)));
    vector<vector<int>> best_solution;double global_best = 1000000000;
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int iter = 0;iter<iter_size;iter++){
        vector<vector<int>> local_best_solution;double local_best_cost = 10000000000;
        for(int ant = 0;ant<ant_size;ant++){
            unordered_set<int> left_cities;
            for(int i = 0;i<cities_size;i++)
                left_cities.insert(i);
            vector<int> exist_ins = ins;
            vector<int> now_loc = start_loc;
            vector<vector<int>> temp_solution(veichles_size);
            vector<unordered_set<int>> feasible_option(veichles_size); //重复的点不能进行重复选
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                exist_ins[start_loc[i]] = exist_ins[start_loc[i]] - 1;
                if(exist_ins[start_loc[i]] == 0)
                    left_cities.erase(start_loc[i]);
                for(int j = 0;j<cities_size;j++){
                    if(j == start_loc[i])continue;
                    feasible_option[i].insert(j);
                }
            }
            
            while(left_cities.size() != 0){
                vector<vector<double>> probility(veichles_size);

                for(int car = 0;car<veichles_size;car++){
                
                    for(int i = 0;i<cities_size;i++){
                        if(exist_ins[i] == 0){
                            probility[car].push_back(0);
                        }else{
                            if(feasible_option[car].count(i) == 0){
                                probility[car].push_back(0);
                            }else {
                                probility[car].push_back(pow(1.0/distance[now_loc[car]][i],index_eta)*pow(node_tao[car][now_loc[car]][i],index_tao));
                            }
                        }
                    }
                    
                }
                pair<int,int> selected_loc = selectElement(probility,q2);
                temp_solution[selected_loc.first].push_back(selected_loc.second);
                feasible_option[selected_loc.first].erase(selected_loc.second);
                exist_ins[selected_loc.second] = exist_ins[selected_loc.second] - 1;
                if(exist_ins[selected_loc.second] == 0){
                    left_cities.erase(selected_loc.second);
                }
                
                node_tao[selected_loc.first][now_loc[selected_loc.first]][selected_loc.second] = (1 - l_rate2)*node_tao[selected_loc.first][now_loc[selected_loc.first]][selected_loc.second] + l_rate2*tao;
                now_loc[selected_loc.first] = selected_loc.second;
            }
            
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                node_tao[i][now_loc[i]][start_loc[i]] = (1 - l_rate2)*node_tao[i][now_loc[i]][start_loc[i]] + l_rate2*tao;
            }
                    
            double temp_cost = total_cost(temp_solution,distance);

            if(temp_cost<local_best_cost){
                local_best_cost = temp_cost;
                local_best_solution = temp_solution;
            }
        }
        double iter_bestcost = total_cost(local_best_solution,distance);
        global_update_3D(node_tao,local_best_solution,iter_bestcost,g_rate2);
        if(getrangenumber(0,1) == 0){
            local_best_solution = node_exchange(local_best_solution,distance);
        }else{
            local_best_solution = node_relocate(local_best_solution,distance);
        }
        iter_bestcost = total_cost(local_best_solution,distance);
        global_update_3D(node_tao,local_best_solution,iter_bestcost,g_rate2);
        local_best_solution = two_opt_optimize(local_best_solution,distance);
        iter_bestcost = total_cost(local_best_solution,distance);
        global_update_3D(node_tao,local_best_solution,iter_bestcost,g_rate2);
        if(iter_bestcost<global_best){
            if(iter_bestcost == 0)cout<<"fuck!!!!!!!!!!!!!!";
            global_best = iter_bestcost;
            best_solution = local_best_solution;

            best_solution = node_exchange(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_3D(node_tao,best_solution,global_best,g_rate2);

            best_solution = node_relocate(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_3D(node_tao,best_solution,global_best,g_rate2);

            best_solution = two_opt_optimize(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_3D(node_tao,best_solution,global_best,g_rate2);
        }
        global_update_3D(node_tao,best_solution,global_best,g_rate2);
        cout<<iter_bestcost<<" "<<global_best<<endl;
        for(int i = 0;i<node_tao.size();i++){
            for(int j = 0;j<node_tao[i].size();j++){
                for(int k = 0;k<node_tao[i][j].size();k++){
                    node_tao[i][j][k] += tao;
                }
            }
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    cout<<"Time count: "<<duration.count()/1000000.0<<" s"<<endl;
    cout<< global_best<<endl;
    printsolution(best_solution);
}
#endif

#if 1
int main(){
    
    std::vector<double> x,y; std::vector<int>ins1,ins2,ins3,ins;
    string location = "D:\\sci_pap\\project_code\\MTMP\\MTMP_data.csv";
    read_info(x,y,ins1,ins2,ins3,location);
    switch (mode)
    {
    case 1:
        veichles_size = 2;
        ins = ins1;
        break;
    case 2:
        veichles_size = 3;
        ins = ins2;
        break;
    case 3:
        veichles_size = 5;
        ins = ins3;
        break;
    default:
        break;
    }
    vector<vector<double>> distance(x.size(),vector<double>(y.size(),0));
    cities_size = x.size();
    
    for(int i = 0;i<ins1.size();i++){
        for(int j = 0;j<i;j++){
            distance[i][j] = sqrt(pow((x[i] - x[j]),2)+pow((y[i] - y[j]),2));
            distance[j][i] = distance[i][j];
        }
    }
    
    vector<int> start_loc = generate_random_numbers(veichles_size,0,cities_size - 1);
    vector<vector<int>> greedy_solution = greedy_alg(distance,start_loc,ins);
    double tao = 1.0/total_cost(greedy_solution,distance);
    vector<vector<double>> node_tao(cities_size,vector<double>(cities_size,tao));
    vector<vector<int>> best_solution;double global_best = 1000000000000.0;
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int iter = 0;iter<iter_size;iter++){
        vector<vector<int>> local_best_solution;double local_best_cost = 10000000000.0;
        for(int ant = 0;ant<ant_size;ant++){
            unordered_set<int> left_cities;
            for(int i = 0;i<cities_size;i++)
                left_cities.insert(i);
            vector<int> exist_ins = ins;
            vector<int> now_loc = start_loc;
            vector<vector<int>> temp_solution(veichles_size);
            vector<unordered_set<int>> feasible_option(veichles_size); //重复的点不能进行重复选
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                exist_ins[start_loc[i]] = exist_ins[start_loc[i]] - 1;
                if(exist_ins[start_loc[i]] == 0)
                    left_cities.erase(start_loc[i]);
                for(int j = 0;j<cities_size;j++){
                    if(j == start_loc[i])continue;
                    feasible_option[i].insert(j);
                }
            }
            
            while(left_cities.size() != 0){
                vector<vector<double>> probility(veichles_size);

                for(int car = 0;car<veichles_size;car++){
                
                    for(int i = 0;i<cities_size;i++){
                        if(exist_ins[i] == 0){
                            probility[car].push_back(0);
                        }else{
                            if(feasible_option[car].count(i) == 0){
                                probility[car].push_back(0);
                            }else {
                                probility[car].push_back(pow(1.0/distance[now_loc[car]][i],index_eta)*pow(node_tao[now_loc[car]][i],index_tao));
                            }
                        }
                    }
                    
                }
                pair<int,int> selected_loc = selectElement(probility,q2);
                temp_solution[selected_loc.first].push_back(selected_loc.second);
                feasible_option[selected_loc.first].erase(selected_loc.second);
                exist_ins[selected_loc.second] = exist_ins[selected_loc.second] - 1;
                if(exist_ins[selected_loc.second] == 0){
                    left_cities.erase(selected_loc.second);
                }
                
                node_tao[now_loc[selected_loc.first]][selected_loc.second] = (1 - l_rate2)*node_tao[now_loc[selected_loc.first]][selected_loc.second] + l_rate2*tao;
                now_loc[selected_loc.first] = selected_loc.second;
            }
            
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                node_tao[now_loc[i]][start_loc[i]] = (1 - l_rate2)*node_tao[now_loc[i]][start_loc[i]] + l_rate2*tao;
            }
                    
            double temp_cost = total_cost(temp_solution,distance);

            if(temp_cost<local_best_cost){
                local_best_cost = temp_cost;
                local_best_solution = temp_solution;
            }
        }
        double iter_bestcost = total_cost(local_best_solution,distance);
        global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
        switch(generate_random_numbers(1,0,2)[0]){
            case 0:
            local_best_solution = node_exchange(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            case 1:
            local_best_solution = node_relocate(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            case 2:
            local_best_solution = two_opt_optimize(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            default:
            break;
        }
        
        if(iter_bestcost<global_best){
            if(iter_bestcost == 0)cout<<"fuck!!!!!!!!!!!!!!";
            global_best = iter_bestcost;
            best_solution = local_best_solution;

            best_solution = node_exchange(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);

            best_solution = node_relocate(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);

            best_solution = two_opt_optimize(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);
            
        }
        global_update_2D(node_tao,best_solution,global_best,g_rate2);
        cout<<iter_bestcost<<" "<<global_best<<endl;
        //mutation_update_matrix_2D(node_tao,distance,3,tao);
        mutation_update_matrix_2D(node_tao,distance,3,1.0/global_best);
        nomralize_2D(node_tao,tao);
        
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    cout<<"Time count: "<<duration.count()/1000000.0<<" s"<<endl;
    cout<< global_best<<endl;
    printsolution(best_solution);
}
#endif

// 利用ins构建一个矩阵，完全访问几次就在信息素矩阵当中出现几次的矩阵
// 由展平后的序列转化成转化前的cite 序号

#if 0
int main(){
    std::vector<double> x,y; std::vector<int>ins1,ins2,ins3,ins;
    string location = "D:\\sci_pap\\project_code\\MTMP\\MTMP_data.csv";
    read_info(x,y,ins1,ins2,ins3,location);
    switch (mode)
    {
    case 1:
        veichles_size = 2;
        ins = ins1;
        break;
    case 2:
        veichles_size = 3;
        ins = ins2;
        break;
    case 3:
        veichles_size = 5;
        ins = ins3;
        break;
    default:
        break;
    }
    vector<vector<double>> distance(x.size(),vector<double>(y.size(),0));
    cities_size = x.size();
    
    for(int i = 0;i<ins1.size();i++){
        for(int j = 0;j<i;j++){
            distance[i][j] = sqrt(pow((x[i] - x[j]),2)+pow((y[i] - y[j]),2));
            distance[j][i] = distance[i][j];
        }
    }

    
    double cities_ins = 0;
    for(int i = 0;i<cities_size;i++)
        cities_ins += ins[i];
    vector<vector<double>> trans_distance(cities_ins,vector<double>(cities_ins,0));
    /*vector<int> start_loc = generate_random_numbers(veichles_size,0,cities_size - 1);
    vector<vector<int>> greedy_solution = greedy_alg(distance,start_loc,ins);*/

    unordered_map<int,int> num_loc;int count = 0;
    unordered_map<int,int> reverse_loc;
    for(int i = 0;i<cities_size;i++){
        reverse_loc[i] = count;
        for(int j = 0;j<ins[i];j++){
            num_loc[j + count] = i;
        }
        count += ins[i];
    }
    vector<int> start_loc(veichles_size);
    vector<int> temp_loc = generate_random_numbers(veichles_size,0,cities_size - 1);
    
    for(int i = 0;i<veichles_size;i++)
        start_loc[i] = reverse_loc[temp_loc[i]];
    vector<vector<int>> greedy_solution = greedy_alg(distance,temp_loc,ins);

    double tao = 1.0/total_cost(greedy_solution,distance);
    
    vector<vector<double>> node_tao(cities_ins,vector<double>(cities_ins,tao));
    for(int i = 0;i<cities_ins;i++){
        for(int j = 0;j<i;j++){
            if(num_loc[i] == num_loc[j]){
                trans_distance[i][j] = -1;
            }else{
                trans_distance[i][j] = distance[num_loc[i]][num_loc[j]];
            }
            trans_distance[j][i] = trans_distance[i][j];
        }
    }
    distance = trans_distance;
    vector<vector<int>> best_solution;double global_best = 1000000000000.0;
    auto start_time = std::chrono::high_resolution_clock::now();
    for(int iter = 0;iter<iter_size;iter++){
        vector<vector<int>> local_best_solution;double local_best_cost = 10000000000.0;
        for(int ant = 0;ant<ant_size;ant++){
            vector<vector<int>> temp_solution(veichles_size);
            vector<int> now_loc = start_loc;
            unordered_set<int> left_node;
            for(int i = 0;i<cities_ins;i++){
                left_node.insert(i);
            }
            vector<unordered_set<int>> feasible_option(veichles_size);
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                left_node.erase(start_loc[i]);
                for(int j = 0;j<cities_ins;j++){
                    if(num_loc[j] == num_loc[start_loc[i]]){
                        continue;
                    }
                    feasible_option[i].insert(j);
                }
            }
            while(left_node.size()!=0){
                vector<vector<double>> probility(veichles_size);
                for(int car = 0;car<veichles_size;car++){

                    for(int i = 0;i<cities_ins;i++){
                        if(feasible_option[car].count(i) == 0){
                            probility[car].push_back(0);
                        }else{
                            probility[car].push_back(pow(1.0/distance[now_loc[car]][i],index_eta)*pow(node_tao[now_loc[car]][i],index_tao));
                        }
                    }

                }
                pair<int,int> selected_loc = selectElement(probility,q2);
                
                temp_solution[selected_loc.first].push_back(selected_loc.second);
                for(int i = 0;i<veichles_size;i++)
                    feasible_option[i].erase(selected_loc.second);
                if(ins[num_loc[selected_loc.second]]>1){
                    for(int i = selected_loc.second + ins[num_loc[selected_loc.second]];i >= selected_loc.second - ins[num_loc[selected_loc.second]];i--){
                        if(i < 0)break;
                        if(num_loc[i] == num_loc[selected_loc.second]){
                            feasible_option[selected_loc.first].erase(i);
                        }
                    }
                }
                left_node.erase(selected_loc.second);
                node_tao[now_loc[selected_loc.first]][selected_loc.second] = (1 - l_rate2)*node_tao[now_loc[selected_loc.first]][selected_loc.second] + l_rate2*tao;
                now_loc[selected_loc.first] = selected_loc.second;
            }
            for(int i = 0;i<veichles_size;i++){
                temp_solution[i].push_back(start_loc[i]);
                node_tao[now_loc[i]][start_loc[i]] = (1 - l_rate2)*node_tao[now_loc[i]][start_loc[i]] + l_rate2*tao;
            }
            
            double temp_cost = total_cost(temp_solution,distance);

            if(temp_cost<local_best_cost){
                local_best_cost = temp_cost;
                local_best_solution = temp_solution;
            }
        }
        double iter_bestcost = total_cost(local_best_solution,distance);
        global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
        switch(generate_random_numbers(1,0,2)[0]){
            case 0:
            local_best_solution = node_exchange(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            case 1:
            local_best_solution = node_relocate(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            case 2:
            local_best_solution = two_opt_optimize(local_best_solution,distance);
            iter_bestcost = total_cost(local_best_solution,distance);
            global_update_2D(node_tao,local_best_solution,iter_bestcost,g_rate2);
            break;
            default:
            break;
        }
        
        if(iter_bestcost<global_best){
            if(iter_bestcost == 0)cout<<"fuck!!!!!!!!!!!!!!";
            global_best = iter_bestcost;
            best_solution = local_best_solution;

            best_solution = node_exchange(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);

            best_solution = node_relocate(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);

            best_solution = two_opt_optimize(best_solution,distance);
            global_best = total_cost(best_solution,distance);
            global_update_2D(node_tao,best_solution,global_best,g_rate2);
            
        }
        global_update_2D(node_tao,best_solution,global_best,g_rate2);
        cout<<iter_bestcost<<" "<<global_best<<endl;
        //mutation_matrix_2D(node_tao,distance,5,tao);
        nomralize_2D(node_tao,tao);
        
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    cout<<"Time count: "<<duration.count()/1000000.0<<" s"<<endl;
    cout<< global_best<<endl;
    best_solution = trans_solution(best_solution,num_loc);
    printsolution(best_solution);

}
#endif
/*int main(){
    
    std::vector<double> x,y; std::vector<int>ins1,ins2,ins3,ins;
    string location = "MTMP_data.csv";
    read_info(x,y,ins1,ins2,ins3,location);
    switch (mode)
    {
    case 1:
        veichles_size = 2;
        ins = ins1;
        break;
    case 2:
        veichles_size = 3;
        ins = ins2;
        break;
    case 3:
        veichles_size = 5;
        ins = ins3;
        break;
    default:
        break;
    }
    vector<vector<double>> distance(x.size(),vector<double>(y.size(),0));
    cities_size = x.size();
    for(int i = 0;i<ins1.size();i++){
        for(int j = 0;j<i;j++){
            distance[i][j] = sqrt(pow((x[i] - x[j]),2)+pow((y[i] - y[j]),2));
            distance[j][i] = distance[i][j];
        }
    }

    double tao = 1.0/138919.0;

    vector<vector<double>> order_allocate_tao(cities_size,vector<double>(cities_size,tao));
    vector<vector<double>> node_tao(cities_size,vector<double>(cities_size,tao));

    for(int iter = 0;iter<iter_size;iter++){

        for(int ant = 0;ant<ant_size;ant++){
            

        }

    }

}*/