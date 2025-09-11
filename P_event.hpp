#include <vector>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <functional>
#include "P_formwork.hpp"


std::chrono::milliseconds hundred_millisecond(100);//100毫秒时间的设置作为事件统计中的一单位时间

//持久化保存上一次的时间戳和steady_clock的参考点
static auto last_steady_time = std::chrono::steady_clock::now();
static int32_t last_system_seconds = 0;

//获取当前时间
int32_t get_current_second_id() {
    //获取当前时间点
    auto system_now = std::chrono::system_clock::now();
    auto steady_now = std::chrono::steady_clock::now();
    int32_t current_system_seconds = std::chrono::duration_cast<std::chrono::seconds>(system_now.time_since_epoch()).count();

    auto steady_elapsed = std::chrono::duration_cast<std::chrono::seconds>(steady_now - last_steady_time);

    if (current_system_seconds < last_system_seconds){std::cout<<"警告：系统时间回拨";}
    else {
        //正常情况：更新持久化状态
        int32_t last_system_seconds = current_system_seconds;
        last_steady_time = steady_now;
    }
    //转换为自纪元以来的秒数
    return current_system_seconds;
}


bool is_time_updated() {
    static auto last_time = std::chrono::system_clock::now();//保存上一次时间点
    
    //获取当前时间并转换为秒级精度
    auto current_time = std::chrono::system_clock::now();
    auto current_sec = std::chrono::duration_cast<std::chrono::seconds>(current_time.time_since_epoch());
    auto last_sec = std::chrono::duration_cast<std::chrono::seconds>(last_time.time_since_epoch());
    
    //比较是否进入新的一秒
    if (current_sec > last_sec) {
        last_time = current_time;//更新记录的时间点
        return true;
    }
    return false;
}

//时间变量转换成字符串
void save_time_to_char_array(char* buffer, size_t buffer_size) {
    using namespace std::chrono;
    
    //获取当前UTC时间
    auto now = system_clock::now();
    time_t current_time = system_clock::to_time_t(now);
    
    //转换为tm结构（线程安全版本，需C++17或编译器支持）
    struct tm time_info;

    gmtime_s(&time_info, &current_time);  // Windows安全函数
    
    //格式化时间为字符串
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info);
}



//设置存储时间数据的数组
char timefilename[13]; // 10 + 2 (.t) + 1 (null) = 13

//将秒数保存到以该数值命名的文件中
bool save_seconds_to_buffer(uint32_t seconds,char* timefilename) {
    //将uint32_t转为字符数组
    char buffer[11];
    int len = snprintf(buffer, sizeof(buffer), "%u", seconds);
    
    if (len < 0 || static_cast<size_t>(len) >= sizeof(buffer)) {
        fprintf(stderr, "秒字符转换失败\n");
        return 0;
    }

    //创建文件名（原数值+.t）
    len = snprintf(timefilename, sizeof(timefilename), "%s.t", buffer);
    
    if (len < 0 || static_cast<size_t>(len) >= sizeof(timefilename)) {
        fprintf(stderr, "时间文件生成失败\n");
        return 0;
    }

    return 1;
}

//写入文件
bool writeTimeFile(char* timefilename)
{std::ofstream file(timefilename);
if (!file.is_open()) {
    fprintf(stderr, "时间文件创建失败 %s: %s\n", 
            timefilename, strerror(errno));
    return false;
}
//写入事件


    file.close();
}


using Aclock = std::chrono::steady_clock;
auto globalPastTime = Aclock::now();//初始化时间锚点

    bool precise_millisecond_trigger(void) {
        using clock = std::chrono::steady_clock;//使用单调时钟避免系统时间跳变
        uint8_t fifty_ms;
        
            //计算剩余等待时间
            auto now = clock::now();
    
            //等待到目标时间
            if (globalPastTime - now >= std::chrono::milliseconds(fifty_ms)) {
                globalPastTime += std::chrono::milliseconds(fifty_ms);
                return 1;
            }
            else{
                return 0;
            }
            
    }



//事件链，存储顺序事件，以供因果推断，这个要与视觉部分对接，还没想好
struct Event_unit{
int32_t seconds;//秒数
unsigned char sequence;//按时间单位分割的秒内时序

};

//近时事件链：记录最近时刻的的全体事件的发生顺序


//关注事件链；用于记载高关注度的事件



struct event_causality{
unsigned long origin_group;//起始现象的编号
unsigned long result_group;//结果现象的编号
unsigned short sum_number;//统计数量
float ave_time;//平均出现时间
float stability;//稳定度，判断可预测程度
std::vector<event_causality_part> part;
};


struct event_causality_part{
char time_scale;//时间单位乘10的该值次方，得到所取时间尺度
char time_range;//在时间尺度中的进一步精确范围
float probalility;//结果出现在以确定时间范围的概率
};



struct stat_event_info{
unsigned long origin_group;//起始现象的编号
unsigned long result_group;//结果现象的编号
std::vector<unsigned short> gap_time;//间隔的单位时间
};



//基于视觉现象观测到的最初级因果的统计
event_causality pri_causality_stat(stat_event_info object){

event_causality stat_data;
stat_data.origin_group = object.origin_group;
stat_data.result_group = object.result_group;
stat_data.sum_number = object.gap_time.size();

int total_time = 0;
for(int a = 0;a < stat_data.sum_number;a++){
total_time += object.gap_time[a];
}
stat_data.ave_time = total_time/stat_data.sum_number;

int total_deviation = 0;
for(int b = 0;b < stat_data.sum_number;b++){
total_deviation += abs(object.gap_time[b] - stat_data.ave_time);
}
stat_data.stability = total_deviation/stat_data.sum_number;

std::vector<char> time_scale;
std::vector<char> time_range;
for(int c = 0;c < stat_data.sum_number;c++){
float _time= object.gap_time[c]; 
for(;_time > 10;_time/10)
time_scale[c] += 1;
time_range[c] = _time;
}

char time_sata[5][11] = {0};


for(int d = 0;d < stat_data.sum_number;d++){
time_sata[time_scale[d]][time_range[d]] += 1;
}

for(int e = 0;e < 5;e++){
short count = 0;
    for(int f = 0;f < 11;f++){
        if(time_sata[e][f] > 0){

event_causality_part g;
g.time_scale = e;
g.time_range = f;
g.probalility = static_cast<float>(time_sata[e][f])/stat_data.sum_number;
stat_data.part[count] = g;
count += 1;
}

}

}

return stat_data;
}


