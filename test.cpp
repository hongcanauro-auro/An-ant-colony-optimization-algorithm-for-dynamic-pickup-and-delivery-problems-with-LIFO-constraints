#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>

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

// 计算中位数
double calculateMedian(std::vector<double> numbers) {
    std::sort(numbers.begin(), numbers.end());
    size_t size = numbers.size();
    if (size % 2 == 0) {
        return (numbers[size / 2 - 1] + numbers[size / 2]) / 2.0;
    } else {
        return numbers[size / 2];
    }
}

// 计算最小值
double calculateMin(const std::vector<double>& numbers) {
    double minValue = numbers[0];
    for (double num : numbers) {
        if (num < minValue) {
            minValue = num;
        }
    }
    return minValue;
}

void processCSV(const std::string& inputFileName) {
    // 读取文件内容到内存
    std::vector<std::string> lines;
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开文件: " << inputFileName << std::endl;
        return;
    }
    std::string line;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // 处理每一行数据并计算统计量
    std::ofstream outputFile(inputFileName, std::ios::out | std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "无法打开文件进行写入: " << inputFileName << std::endl;
        return;
    }
    for (const std::string& currentLine : lines) {
        std::istringstream iss(currentLine);
        std::vector<double> numbers;
        std::string value;
        while (std::getline(iss, value, ',')) {
            try {
                numbers.push_back(std::stod(value));
            } catch (const std::invalid_argument& e) {
                std::cerr << "无效的数字: " << value << std::endl;
                continue;
            }
        }

        if (!numbers.empty()) {
            double mean = calculateMean(numbers);
            double variance = calculateVariance(numbers, mean);
            double median = calculateMedian(numbers);
            double minValue = calculateMin(numbers);

            outputFile <<" mean " <<"," << mean <<","<<" var "<< "," << variance << ","<<" middle " <<","<< median << "," <<" min "<<","<< minValue << "\n";
        }
    }
    outputFile.close();
}

std::vector<std::vector<double>> initUniformWeight() {
    std::vector<std::vector<double>> lambda_(6,std::vector<double>(2));
    for (int n = 0; n < 6; n++) {
        double a = 1.0 * n / 5;
        lambda_[n][0] = a;
        lambda_[n][1] = 1 - a;
    }
    return lambda_;
}

int main() {
    /*std::string inputFileName = "D:\\sci_pap\\project_code\\moead_ts\\data3.csv";
    processCSV(inputFileName);*/
    std::vector<std::vector<double>> lambda_ = initUniformWeight();
    for(int i = 0;i<6;i++){
        std::cout<<"{"<<lambda_[i][0]<<","<<lambda_[i][1]<<"},";
        std::cout<<std::endl;
    }
    system("pause");
    return 0;
}