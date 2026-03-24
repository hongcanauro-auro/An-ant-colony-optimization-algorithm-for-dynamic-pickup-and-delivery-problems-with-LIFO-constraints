#include <opencv2/opencv.hpp>
#include <vector>

void visualizeRoutes(const std::vector<std::vector<int>>& routes, 
                     const std::map<int, cv::Point>& cityCoords) {
    // 创建空白画布 (白色背景)
    cv::Mat canvas(800, 800, CV_8UC3, cv::Scalar(255, 255, 255));
    
    // 绘制所有城市节点
    for (const auto& city : cityCoords) {
        cv::circle(canvas, city.second, 8, cv::Scalar(0, 100, 255), -1); // 橙色圆点
        cv::putText(canvas, std::to_string(city.first), 
                    city.second + cv::Point(-10, -15),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
    
    // 为每个旅行商分配不同颜色
    std::vector<cv::Scalar> colors = {
        cv::Scalar(255, 0, 0),   // 蓝色
        cv::Scalar(0, 255, 0),   // 绿色
        cv::Scalar(0, 0, 255),   // 红色
        cv::Scalar(128, 0, 128)  // 紫色
    };
    
    // 绘制路径
    for (size_t i = 0; i < routes.size(); ++i) {
        const auto& path = routes[i];
        for (size_t j = 0; j < path.size() - 1; ++j) {
            cv::Point p1 = cityCoords.at(path[j]);
            cv::Point p2 = cityCoords.at(path[j + 1]);
            cv::line(canvas, p1, p2, colors[i % colors.size()], 2);
        }
        // 返回起点形成闭环
        if (path.size() > 1) {
            cv::line(canvas, cityCoords.at(path.back()), 
                     cityCoords.at(path[0]), 
                     colors[i % colors.size()], 2);
        }
    }
    
    cv::imshow("MTMV Solution", canvas);
    cv::waitKey(0);
}