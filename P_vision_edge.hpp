#include <vector>


//单像素细节
struct PRO_RGB{
    unsigned char edge_Kind=0b00000000;//边缘类型
    unsigned char point_kind=0;//点的属性，是否端点1、斜连线2、待定3、直连线4、预备端点5
};


    struct x_outside{
    //指向远程连接
    short x;//另一端坐标
    short y;
    char gradient;//方向              转换为方向区间
    short length;//相对长度
    float reward;//强度
    edge_event* ptr = NULL;//指向事件神经元的指针
    };

    struct x_inside{
    //指向直接连接
    short x;//另端坐标
    short y;
    char gradient;//斜率              转换为方向区间
    short length;//长度
    float reward;//奖励
    edge_event* ptr = NULL;//指向事件神经元的指针
    };//19字节


    struct y_inside{

    std::vector<x_inside*> to_link;//指向端点的首个连接，每个代表一个x点
    std::vector<x_outside*> cross_link;//跨格连接
    std::vector<unsigned char> every_amount_of_link;//每个点的链接数目
    std::vector<unsigned char> every_amount_of_cross;//每个点的跨格链接数目
    };


    struct  MapRecordTree{

    int32_t second_time;
    int16_t second_sequence;
    short width_x = 0;
    short height_y = 0;
    MapRecordTree* last_one;
    MapRecordTree* next_one;
    int totality = 0;            //端点总量
    std::vector<y_inside> to_row;//指向行的全部端点
    std::vector<short> every_amount_of_X;   //每行的端点总数

    };



struct coordinate{char attribute;//起始0，直线4，斜线2，，前直线6，前斜线7，可能结束点:预备端点5，结束点:端点1
    int x;int y;//分支点屏幕坐标
};



