#include <iostream>
#include <fstream>
#include <sstream>
#include <CL/cl.h>
#include <algorithm>
#include <cmath>

#include "P_vision_edge.hpp"
#include "P_formwork.hpp"



struct ScreenImage {
    uint16_t width;          // 图片宽度
    uint16_t height;         // 图片高度
    uint32_t stride;      // 每行像素的字节数（考虑对齐）
    std::vector<uint8_t> pixelData;
};

struct RGB {
    uint8_t r, g, b;
};


std::vector<RGB> getRGBmap(const ScreenImage& image){
    std::vector<RGB> rgbmap;
    rgbmap.reserve(image.width*image.height);
    const uint8_t* pData = image.pixelData.data();
    for(int y=0;y<image.height;y++){
        const uint8_t* pRow = pData + y * image.stride;  // 行级指针计算
        for(int x=0;x<image.width;x++)
        {   const uint8_t* pPixel = pRow + x * 3;
            rgbmap.push_back({
                pPixel[2],  // R (原BGR顺序的B通道)
                pPixel[1],  // G 
                pPixel[0]   // B (原BGR顺序的R通道)
            });

        }
    }
    return rgbmap;
};



const int ARRAY_SIZE = 100;


//返回上下文创建的函数
cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatforms;
    cl_platform_id My_PlatformId;
    cl_context context = NULL;
 

    errNum = clGetPlatformIDs(1, &My_PlatformId, &numPlatforms);
    if (errNum != CL_SUCCESS || numPlatforms <= 0)
    {
        std::cerr << "未找到OpenCL平台" << std::endl;
        return NULL;
    }
 

    cl_context_properties contextProperties[] =
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)My_PlatformId,
        0
    };
    context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU,
                                      NULL, NULL, &errNum);
    if (errNum != CL_SUCCESS)
    {
        std::cout << " GPU上下文创建失败" << std::endl;

    }
 
    return context;
}
 

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_int errNum;
    cl_device_id *devices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;
 

    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "函数失败clGetContextInfo(...,GL_CONTEXT_DEVICES,...)";
        return NULL;
    }
 
    if (deviceBufferSize <= 0)
    {
        std::cerr << "无可获取设备.";
        return NULL;
    }
 

    devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
    if (errNum != CL_SUCCESS)
    {
        delete [] devices;
        std::cerr << "未能得到设备ID";
        return NULL;
    }
 

    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if (commandQueue == NULL)
    {
        delete [] devices;
        std::cerr << "设备0命令队列创建失败";
        return NULL;
    }
 
    *device = devices[0];
    delete [] devices;
    return commandQueue;
}
 

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int errNum;
    cl_program program;
 
    std::ifstream kernelFile(fileName, std::ios::in);
    if (!kernelFile.is_open())
    {
        std::cerr << "cl文件读取失败" << fileName << std::endl;
        return NULL;
    }
 
    std::ostringstream oss;
    oss << kernelFile.rdbuf();
 
    std::string srcStdStr = oss.str();
    const char *srcStr = srcStdStr.c_str();
    program = clCreateProgramWithSource(context, 1,
                                        (const char**)&srcStr,
                                        NULL, NULL);
    if (program == NULL)
    {
        std::cerr << "内核代码的编译单元对象创建失败" << std::endl;
        return NULL;
    }
 
    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {

        char buildLog[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              sizeof(buildLog), buildLog, NULL);
 
        std::cerr << "kernel错误: " << std::endl;
        std::cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }
 
    return program;
}
 

bool CreateMemObjects(cl_context context, cl_mem memObjects[10],
                      std::vector<RGB>& input,int x,int y)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   input.size() * sizeof(RGB), input.data(), NULL);


    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   x*y * sizeof(PRO_RGB), NULL, NULL);


    if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
    {
        std::cerr << "内存创建错误" << std::endl;
        return false;
    }
 
    return true;
}
 

void Cleanup(cl_context context, cl_command_queue commandQueue,
             cl_program program, cl_kernel kernel[3],cl_mem memObjects[10])
{
    for (int i = 0; i < 2; i++)
    {
        if (memObjects[i] != 0)
            clReleaseMemObject(memObjects[i]);
    }
    if (commandQueue != 0)
        clReleaseCommandQueue(commandQueue);
 
    for(int i = 0;i<1;i++){
        if (kernel != 0)
        clReleaseKernel(kernel[i]);}
 

    if (program != 0)
        clReleaseProgram(program);
 
    if (context != 0)
        clReleaseContext(context);
 
}




int start_GPU(ScreenImage originalIMAGE)//此处输入图像
{

std::vector<MapRecordTree> picture_list_one;
picture_list_one.resize(20);

int k = picture_list_one.size();

picture_list_one.back().second_time;
picture_list_one.back().second_sequence;
picture_list_one.back().width_x = originalIMAGE.width;
picture_list_one.back().height_y = originalIMAGE.height;
picture_list_one.back().last_one = &picture_list_one[k-2];


    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel[1] = {0};
    cl_mem memObjects[2] = { 0,0};
    cl_int errNum;
 

    context = CreateContext();
    if (context == NULL)
    {
        std::cerr << "未能创建OpenCL上下文." << std::endl;
        return 1;
    }
 

    commandQueue = CreateCommandQueue(context, &device);
    if (commandQueue == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
 

    program = CreateProgram(context, device, "P_vision.cl");
    if (program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
 


    kernel[0] = clCreateKernel(program, "first_kernel", NULL);
    if (kernel[0] == NULL)
    {
        std::cerr << "未能创建kernel1号" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }




    std::vector<RGB> input = getRGBmap(originalIMAGE);

    std::vector<PRO_RGB> DETAIL_MAP(originalIMAGE.width*originalIMAGE.height);

    int half_width = originalIMAGE.width/2;
    int half_height = originalIMAGE.height/2;
    int quarter_width = half_width/2;
    int quarter_height = half_height/2;

    int work_width =1920;
    if(originalIMAGE.width != 1920){
        if(originalIMAGE.width/16 ==0)work_width =originalIMAGE.width;
    else work_width = originalIMAGE.width+(16-originalIMAGE.width%16);
}

    int work_height =1080;
    if(originalIMAGE.height !=1080){
        if(originalIMAGE.height/16 ==0)work_height =originalIMAGE.height;
    else work_height = originalIMAGE.height+(16-originalIMAGE.height%16);
}

    if (!CreateMemObjects(context, memObjects, input,originalIMAGE.width,originalIMAGE.height))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
 


    errNum |= clSetKernelArg(kernel[0], 0, sizeof(int), &originalIMAGE.width);
    errNum |= clSetKernelArg(kernel[0], 1, sizeof(int), &originalIMAGE.height);
    errNum |= clSetKernelArg(kernel[0], 2, sizeof(cl_mem), &memObjects[0]);
    errNum |= clSetKernelArg(kernel[0], 3, sizeof(cl_mem), &memObjects[1]);



    if (errNum != CL_SUCCESS)
    {
        std::cerr << "内核函数参数设置错误" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
 
    size_t globalWorkSize[2] = { work_width,work_height };
    size_t localWorkSize[2] = { 16,16 };
 


    errNum = clEnqueueNDRangeKernel(commandQueue, kernel[0], 2, NULL,
                                    globalWorkSize, localWorkSize,
                                    0, NULL, NULL);
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "内核函数1号执行队列错误" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }


    errNum = clEnqueueReadBuffer(commandQueue, memObjects[1], CL_TRUE,
                                 0, (originalIMAGE.width)*(originalIMAGE.height)* sizeof(PRO_RGB), DETAIL_MAP.data(),
                                 0, NULL, NULL);
 
    if (errNum != CL_SUCCESS)
    {
        std::cerr << "结果数组读取错误" << std::endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return 1;
    }
 
    




//加工 “边缘处理结果数组”

//初始化端点、封闭图形

int x_inside_count = 0;//x点的链接的数量统计，用于point_list的位置统计
std::vector<x_inside> point_list;//存储图树的端点链接
point_list.reserve(1024000);
MapRecordTree current_MapTree;//这是图树，包涵一张图的全体构造
current_MapTree.to_row.reserve(current_MapTree.height_y);
current_MapTree.width_x = originalIMAGE.width;
current_MapTree.height_y = originalIMAGE.height;

//4、线段构造 此时只存在1、2、4三种端点类型
for(int yy=0;yy < originalIMAGE.height;yy +1){

    for(int xx=0;xx < originalIMAGE.width;xx +1){



//探测端点


if(DETAIL_MAP[originalIMAGE.width * yy + xx].point_kind = 5);{//通过条件：是预备端点
coordinate original_point;
original_point.x=xx;    original_point.y=yy;
original_point.attribute = 0;
std::vector<coordinate> current_point_branch(20);//原点分支存储一条拟合线的动态数组，每个成员都是
current_point_branch.push_back(original_point);//输入原点至判断树

char branch_state = 0;
char orignal_read_state = 0b0;//记录端点
int y = yy;int x = xx;
re:

    if(x>0){//左直线检测
        if( (DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 6 || DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 4) 
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 0 & 1 != 1) 
        {
            coordinate current_point;
            current_point.x = x - 1;current_point.y = y;
            x = x - 1;y = y;
            current_point.attribute = 6;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 1  || DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 0 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重与标记
                if(DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 1 && orignal_read_state >> 0 & 1)
                goto next1;
                if(DETAIL_MAP[originalIMAGE.width * y + x - 1].point_kind == 1) orignal_read_state |= (1 << 0);
                }
                


            coordinate current_point;
            current_point.x = x - 1;current_point.y = y;
            x = x - 1;y = y;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
    }
next1:
    if(x < originalIMAGE.width - 1){//右直线检测
        if( (DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 6 || DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 4 ) 
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 1 & 1 != 1) {
            
            coordinate current_point;
            current_point.x = x + 1;current_point.y = y;
            x = x + 1;y = y;
            current_point.attribute = 6;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 1 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 1 && orignal_read_state >> 1 & 1)
                goto next2;
                if(DETAIL_MAP[originalIMAGE.width * y + x + 1].point_kind == 1) orignal_read_state |= (1 << 1);;
                }

            coordinate current_point;
            current_point.x = x + 1;current_point.y = y;
            x = x + 1;y = y;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
    }
next2:
    if(y>0){//上直线检测
        if((DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 6 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 4)//是同类的直线
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 2 & 1 != 1) {
            
            coordinate current_point;
            current_point.x = x;current_point.y = y - 1;
            x = x;y = y - 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 5)//在同类的端点与预备端点处统计，通过则拟合直线或终止
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 2 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 1 && orignal_read_state >> 2 & 1)
                goto next3;
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x].point_kind == 1) orignal_read_state |= (1 << 2);
                }

            coordinate current_point;
            current_point.x = x;current_point.y = y - 1;
            x = x;y = y - 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
        }
    
next3:
    if(y<originalIMAGE.height){//下直线检测
        if((DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 6 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 4)
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 3 & 1 != 1) {
                        
            coordinate current_point;
            current_point.x = x;current_point.y = y + 1;
            x = x;y = y + 1;
            current_point.attribute = 6;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 3 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 1 && orignal_read_state >> 3 & 1)
                goto next4;
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x].point_kind == 1) orignal_read_state |= (1 << 3);;
                }

            coordinate current_point;
            current_point.x = x;current_point.y = y + 1;
            x = x;y = y + 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
        }
    
next4:
    if(x>0 && y>0){//左上斜线检测
        if((DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 7 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 2)
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 4 & 1 != 1) {
                
            coordinate current_point;
            current_point.x = x - 1;current_point.y = y - 1;
            x = x - 1;y = y - 1;
            current_point.attribute = 7;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if((DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 4 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 1 && orignal_read_state >> 4 & 1)
                goto next5;
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x - 1].point_kind == 1) orignal_read_state |= (1 << 4);;
                }

            coordinate current_point;
            current_point.x = x - 1;current_point.y = y - 1;
            x = x - 1;y = y - 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
        }
    
next5:
    if(x < originalIMAGE.width - 1 && y>0){//右上斜线检测
        if((DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 7 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 2)
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 5 & 1 != 1) {
                        
            coordinate current_point;
            current_point.x = x + 1;current_point.y = y - 1;
            x = x + 1;y = y - 1;
            current_point.attribute = 7;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 5 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 1 && orignal_read_state >> 5 & 1)
                goto next6;
                if(DETAIL_MAP[originalIMAGE.width * (y - 1) + x + 1].point_kind == 1) orignal_read_state |= (1 << 5);;
                }

            coordinate current_point;
            current_point.x = x + 1;current_point.y = y - 1;
            x = x + 1;y = y - 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
        }
    
next6:
    if(x>0 && y<originalIMAGE.height){//左下斜线检测
        if((DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 7 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 2)
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 6 & 1 != 1) {
                        
            coordinate current_point;
            current_point.x = x - 1;current_point.y = y + 1;
            x = x - 1;y = y + 1;
            current_point.attribute = 7;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 1 & 6 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 1 && orignal_read_state >> 6 & 1)
                goto next7;
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x - 1].point_kind == 1) orignal_read_state |= (1 << 6);;
                }

            coordinate current_point;
            current_point.x = x - 1;current_point.y = y + 1;
            x = x - 1;y = y + 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }
        }
    
next7:
    if(x < originalIMAGE.width - 1 && y<originalIMAGE.height){//右下斜线检测
        if((DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 7 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 2)
        && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 7 & 1 != 1) {
                        
            coordinate current_point;
            current_point.x = x + 1;current_point.y = y + 1;
            x = x + 1;y = y + 1;
            current_point.attribute = 7;
            current_point_branch.push_back(current_point);

            goto re;
        }
        else if( (DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 1 || DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 5)
            && DETAIL_MAP[originalIMAGE.width * y + x].edge_Kind >> 7 & 1 != 1){

                if(current_point_branch.size() == 1){ //端点查重
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 1 && orignal_read_state >> 7 & 1)
                goto end;
                if(DETAIL_MAP[originalIMAGE.width * (y + 1) + x + 1].point_kind == 1) orignal_read_state |= (1 << 7);
                }

            coordinate current_point;
            current_point.x = x + 1;current_point.y = y + 1;
            x = x + 1;y = y + 1;
            current_point.attribute = DETAIL_MAP[originalIMAGE.width * y + x].point_kind;
            current_point_branch.push_back(current_point);

            goto statistics_edge;//遭遇端点，直线拟合终止
        }

        }
    
end:


if (0){
//统计线段
statistics_edge: 
short k = current_point_branch.size();
coordinate end_point = current_point_branch[k-1];
coordinate first_point = current_point_branch[0];

current_MapTree.every_amount_of_X.reserve(current_MapTree.height_y);
current_MapTree.to_row[first_point.y].every_amount_of_direct.reserve(current_MapTree.width_x);


int all_x = 0;
for(int p = 0;p < k;p++){

current_point_branch[p];
all_x += (current_point_branch[p].x - first_point.x);

}

int ave_x = all_x/k;//x均值计算完毕


int all_y = 0;
for(int p = 0;p < k;p++){

current_point_branch[p];
all_y += (current_point_branch[p].y - first_point.y);

}

float gradient_accuracy;


char gradient_type;
int ave_y = all_y/k;//y均值计算完毕
float gradient = 0;//斜率定义
if(ave_y != 0 && ave_x != 0)    
{gradient = ave_y/ave_x;

    if(gradient > 0){

if(gradient < 0.86){
if(gradient < 0.41){
if(gradient < 0.2){
gradient_accuracy = 0;gradient_type = 0;//斜率<0.2，横
}
else{
{if(gradient < 0.33){gradient_accuracy = (gradient - 0.33)/0.13/2 + 1/2;gradient_type = 0;}else {gradient_accuracy = (0.41 - gradient)/0.09/2 + 1/2;gradient_type = 0;}}//0.33中心
}

}else{

    if(gradient < 0.61)
    {if(gradient < 0.5){gradient_accuracy = (gradient - 0.33)/0.17/2 + 1/2;gradient_type = 1;}else {gradient_accuracy = (0.61 - gradient)/0.14/2 + 1/2;gradient_type = 1;}}//0.5中心
    
    else
    {if(gradient < 0.75){gradient_accuracy = (gradient - 0.61)/0.14/2 + 1/2;gradient_type = 1;}else {gradient_accuracy = (0.86 - gradient)/0.11/2 + 1/2;gradient_type = 1;}}//0.75中心

    
}


}else{//gradient >= 0.86
if(gradient < 1.6)
{
if(gradient < 1.18) {if(gradient < 1){gradient_accuracy = (gradient - 0.86)/0.14/2 + 1/2;gradient_type = 1;}else {gradient_accuracy = (1.18 - gradient)/0.18/2 + 1/2;gradient_type = 1;}}//1中心
else {if(gradient < 1.33){gradient_accuracy = (gradient - 1.18)/0.15/2 + 1/2;gradient_type = 1;}else {gradient_accuracy = (1.6 - gradient)/0.27/2 + 1/2;gradient_type = 1;}}//1.33中心

}
else if(gradient < 5){
    if(gradient < 2.4) {if(gradient < 2){gradient_accuracy = (gradient - 1.6 + 2 + 1/2)/2;gradient_type = 1;}else {gradient_accuracy = (2.4 - gradient)/2/2 + 1/2;gradient_type = 1;}}//2中心
    else {if(gradient < 3){gradient_accuracy = (gradient - 2.4)/0.6/2 + 1/2;gradient_type = 2;}else {gradient_accuracy = (5 - gradient)/2/2 + 1/2;gradient_type = 2;}}//3中心

}

else {gradient_accuracy = 0;gradient_type = 2;}//斜率>5，竖

    
}
//一三象限的反转
if(first_point.x > end_point.x){//象限为三


if(gradient_type = 0)gradient_type = 4;
if(gradient_type = 1)gradient_type = 5;
if(gradient_type = 2)gradient_type = 6;

};




    }

//以下是负数斜率
else{gradient = abs(gradient);

if(gradient < 0.86){
if(gradient < 0.41){
if(gradient < 0.2){
gradient_accuracy=0;gradient_type = 4;//斜率<0.2，横
}
else{
{if(gradient < 0.33){gradient_accuracy = (gradient - 0.33)/0.13/2 + 1/2;gradient_type = 4;}else {gradient_accuracy = (0.41 - gradient)/0.09/2 + 1/2;gradient_type = 4;}}//0.33中心
}

}else{

    if(gradient < 0.61)
    {if(gradient < 0.5){gradient_accuracy = (gradient - 0.33)/0.17/2 + 1/2;gradient_type = 3;}else {gradient_accuracy = (0.61 - gradient)/0.14/2 + 1/2;gradient_type = 3;}}//0.5中心
    
    else
    {if(gradient < 0.75){gradient_accuracy = (gradient - 0.61)/0.14/2 + 1/2;gradient_type = 3;}else {gradient_accuracy = (0.86 - gradient)/0.11/2 + 1/2;gradient_type = 3;}}//0.75中心

    
}


}else{//gradient >= 0.86
if(gradient < 1.6)
{
if(gradient < 1.18) {if(gradient < 1){gradient_accuracy = (gradient - 0.86)/0.14/2 + 1/2;gradient_type = 3;}else {gradient_accuracy = (1.18 - gradient)/0.18/2 + 1/2;gradient_type = 3;}}//1中心
else {if(gradient < 1.33){gradient_accuracy = (gradient - 1.18)/0.15/2 + 1/2;gradient_type = 3;}else {gradient_accuracy = (1.6 - gradient)/0.27/2 + 1/2;gradient_type = 3;}}//1.33中心

}
else if(gradient < 5){
    if(gradient < 2.4) {if(gradient < 2){gradient_accuracy = (gradient - 1.6 + 2 + 1/2)/2;gradient_type = 3;}else {gradient_accuracy = (2.4 - gradient)/2/2 + 1/2;gradient_type = 3;}}//2中心
    else {if(gradient < 3){gradient_accuracy = (gradient - 2.4)/0.6/2 + 1/2;gradient_type = 2;}else {gradient_accuracy = (5 - gradient)/2/2 + 1/2;gradient_type = 2;}}//3中心

}

else {gradient_accuracy = 0;gradient_type = 2;}//斜率>5，竖

    
}

//二四象限的反转
if(end_point.x > first_point.x){//象限为四


if(gradient_type = 2)gradient_type = 6;
if(gradient_type = 3)gradient_type = 7;
if(gradient_type = 4)gradient_type = 0;

};

        }//以上负斜率
    
}

else {
    if(ave_x == 0 && first_point.y > end_point.y)
    gradient_type = 6;
    if(ave_y == 0 && first_point.x > end_point.x)
    gradient_type = 4;
    if(ave_x == 0 && first_point.y < end_point.y)
    gradient_type = 2;
    if(ave_y == 0 && first_point.x < end_point.x)
    gradient_type = 0;

    gradient_accuracy = 1;
    
}//横竖斜率对照表






//为斜率找到区划
if(first_point.x > end_point.x && first_point.y > end_point.y){
    if(gradient_type == 0) gradient_type == 4;
    if(gradient_type == 1) gradient_type == 5;
    if(gradient_type == 2) gradient_type == 6;
    }

if(first_point.x < end_point.x && first_point.y > end_point.y){
    if(gradient_type == 2) gradient_type == 6;
    if(gradient_type == 3) gradient_type == 7;
    if(gradient_type == 4) gradient_type == 0;
    }


int dif_amount = 0;//误差总数

if(ave_x == 0){//竖线情况
for(int o = 0;o < k;o++){
if( abs(first_point.y - current_point_branch[o].y) > 0.5)
dif_amount += 1;
}}


else if(ave_y == 0){//横线情况
for(int o = 0;o < k;o++){
if( abs(first_point.x - current_point_branch[o].x) > 0.5)
dif_amount += 1;
}}


else {//一般情况
for(int i = 0;i < k;i++){
    if( abs(current_point_branch[i].x *gradient
    - current_point_branch[i].y ) > 0.5)
    dif_amount += 1;}
    }
float accuracy= dif_amount/k;

float reward = 0;

for(int a = 0;a < k;a++){
if(current_point_branch[a].attribute == 6 || current_point_branch[a].attribute == 7) reward += 0.5;

else if(current_point_branch[a].attribute == 1 || current_point_branch[a].attribute == 5) reward += 1;
}

if(ave_x == 0 || ave_y == 0)
reward = reward * accuracy * 1.15;//直线
else reward = reward * accuracy * gradient_accuracy;//斜线

//在图树中加入边缘
x_inside end_possible_point;//预备加入的连接
end_possible_point.x = end_point.x;
end_possible_point.y = end_point.y;
end_possible_point.gradient = gradient_type;
end_possible_point.length = k;
end_possible_point.reward = reward;

//比较前后线段判断合并

if(branch_state == 0) {

point_list.push_back(end_possible_point);
branch_state = 1;
current_MapTree.to_row[first_point.y].direct_link[first_point.x] = &point_list[x_inside_count];//指针链接首连接
current_MapTree.to_row[first_point.y].every_amount_of_direct[first_point.x] += 1;//该点链接+1
current_MapTree.every_amount_of_X[first_point.y] + 1;//该行链接+1
x_inside_count += 1;}//初始化

if(branch_state == 2){
point_list.push_back(end_possible_point);//由于设计，同点的新线不能赋予在首个指针上
branch_state = 3;
current_MapTree.to_row[first_point.y].every_amount_of_direct[first_point.x] += 1;
current_MapTree.every_amount_of_X[first_point.y] + 1;
x_inside_count += 1;

}
//合并操作
if(branch_state ==1 || branch_state ==3){
//branch_state,0未生成，1已有的比对，2已有的新建
if(reward >= (*current_MapTree.to_row[first_point.y].direct_link[first_point.x]).reward * (k/point_list[x_inside_count].length)*0.9)
{point_list[x_inside_count].x = end_point.x;
point_list[x_inside_count].y = end_point.y;
point_list[x_inside_count].gradient = gradient;
point_list[x_inside_count].length = k;
point_list[x_inside_count].reward = reward;
goto re;
}//合并边缘通过

//不通过合并边缘，该边缘整合完毕，更改所有边缘属性
else {
    {for(int a;a < k;a++){
if(DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind == 7)
DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind =2;//斜线
if(DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind == 6)
DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind =4;//直线
if(DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind == 5 && a!= k)
DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind =4;
if(DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind == 5 && a== k)
DETAIL_MAP[originalIMAGE.width * (current_point_branch[a].y) + (current_point_branch[a].x)].point_kind =1;//端点
}//点属性赋予
point_list[x_inside_count].x = end_point.x;
point_list[x_inside_count].y = end_point.y;
point_list[x_inside_count].gradient = gradient;
point_list[x_inside_count].length = k;
point_list[x_inside_count].reward = reward;


DETAIL_MAP[originalIMAGE.width * (current_point_branch[0].y) + (current_point_branch[0].x) ].point_kind == 1;//原点
current_point_branch.clear();
coordinate original_point;
original_point.attribute = 1;
original_point.x=xx;    original_point.y=yy;
std::vector<coordinate> current_point_branch(10);//原点分支存储一条拟合线的动态数组，每个成员都是
current_point_branch.push_back(original_point);//输入原点至判断树
branch_state = 2;
goto re;//再寻找线段拟合
}
}



}



}//statistics_edge块结束

current_point_branch.clear();

}




}



}//边缘查找结束




//以下 远程连接的建立
std::vector<x_outside> remote_list;
int x_cross_count =0;
remote_list.reserve(4096000);
for(int scan_y=0;scan_y < originalIMAGE.height;scan_y += 1){

    for(int scan_x=0;scan_x < originalIMAGE.width;scan_x += 1){

if(DETAIL_MAP[originalIMAGE.width * scan_y + scan_x].point_kind = 1){//通过条件：是端点
int line_number = current_MapTree.to_row[scan_y].every_amount_of_direct[scan_x];//连线数量
int distance = 0;//
for(int k= 0;k < line_number;k++){
distance += (*(current_MapTree.to_row[scan_y].direct_link[scan_x] + k*14)).length;
}
distance = distance/line_number;//平均长度


//扫描范围矫准
int scan_wight_left = scan_x - distance;int scan_wight_right = scan_x + distance;
int scan_height_top = scan_y - distance;int scan_height_bottom = scan_y + distance;
if(scan_wight_left < 0)
scan_wight_left = 0;
if(scan_wight_right >= originalIMAGE.width)
scan_wight_right = originalIMAGE.width - 1;
if(scan_height_top < 0)
scan_height_top = 0;
if(scan_height_bottom >= originalIMAGE.height)
scan_height_bottom = originalIMAGE.height - 1;



for(int k = scan_height_top;k < scan_height_bottom;k++){

for(int r = scan_wight_left;r < scan_wight_right;r++)
{
    char ray_state =0;
    if(DETAIL_MAP[originalIMAGE.width * k + r].point_kind = 1){

    //统计方向
    float dif_y = k - scan_y;
    float dif_x = r - scan_x;
    float direction = 0;



    //以下为方向赋予
    if(dif_y != 0 && dif_x != 0){
    float direction = dif_y/dif_x;

    if(direction > 0){
    if(direction < 0.41)
    direction = 0;

    else if(direction < 2.4)direction = 1;
    else direction = 2;

    //一三象限的反转
    if(scan_x > r){//象限为三

    if(direction = 0)direction = 4;
    if(direction = 1)direction = 5;
    if(direction = 2)direction = 6;

    };
    }

    //以下是负数斜率
    else{direction = abs(direction);
    if(direction < 0.41)
    direction = 4;

    else if(direction < 2.4) direction = 3;
    else direction = 2;

    //二四象限的反转
    if(scan_x < r){//象限为四

    if(direction = 2)direction = 6;
    if(direction = 3)direction = 7;
    if(direction = 4)direction = 0;

    }

    }

    }

    else {
    if(dif_x == 0 && scan_y > k)//k是远连端点的高度
    direction = 6;
    if(dif_y == 0 && scan_x > r)
    direction = 4;
    if(dif_x == 0 && scan_y < k)
    direction = 2;
    if(dif_y == 0 && scan_x < r)
    direction = 0;
    }//方向对照表



    //统计相对距离
    float hypotenuse = sqrt((scan_y - k)*(scan_y - k) + (scan_x - r)*(scan_x - r))/distance;//实际距离/平均长度

    //数据载入
    x_outside ray;
    ray.x = dif_x;
    ray.y = dif_y;
    ray.gradient = direction;
    ray.length = hypotenuse;
    ray.reward = 0;
    current_MapTree.to_row[scan_y].every_amount_of_remote[scan_x] += 1;
    remote_list.push_back(ray);
    x_cross_count += 1;
    if(ray_state == 0)
    current_MapTree.to_row[scan_y].remote_link[scan_x] = &remote_list[x_cross_count];//远程指针链接首连接

    ray_state = 1;
};

;}

}//远程连接搜索结束



}

    }

}



    Cleanup(context, commandQueue, program, kernel, memObjects);
 
    return 0;
};





//通用分类器，分离朝向、颜色、端点
std::string classify_edge(std::string search_path,char gradient,RGB colour,unsigned char kind){


//分类端点
if(kind >> 0 & 1 == 1)
search_path = search_path + "\\1";
else search_path = search_path + "\\0";

if(kind >> 1 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 2 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 3 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 4 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 5 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 6 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";

if(kind >> 7 & 1 == 1)
search_path = search_path + "1";
else search_path = search_path + "0";


//朝向分类
switch(gradient){
case 0:
search_path = search_path + "\\0";
break;

case 1:
search_path = search_path + "\\1";
break;

case 2:
search_path = search_path + "\\2";
break;

case 3:
search_path = search_path + "\\3";
break;

case 4:
search_path = search_path + "\\4";
break;

case 5:
search_path = search_path + "\\5";
break;

case 6:
search_path = search_path + "\\6";
break;

case 7:
search_path = search_path + "\\7";
break;
}

//颜色分类
char r=colour.r/16;
switch(r){
case 0:
search_path = search_path + "\\0";//0~15
break;
case 1:
search_path = search_path + "\\1";//16~31
break;
case 2:
search_path = search_path + "\\2";//32~47
break;
case 3:
search_path = search_path + "\\3";//48~63
break;
case 4:
search_path = search_path + "\\4";//64~79
break;
case 5:
search_path = search_path + "\\5";//80~95
break;
case 6:
search_path = search_path + "\\6";//96~111
break;
case 7:
search_path = search_path + "\\7";//112~
break;
case 8:
search_path = search_path + "\\8";//128
break;
case 9:
search_path = search_path + "\\9";//144
break;
case 10:
search_path = search_path + "\\a";//160
break;
case 11:
search_path = search_path + "\\b";//176
break;
case 12:
search_path = search_path + "\\c";//192
break;
case 13:
search_path = search_path + "\\d";//208
break;
case 14:
search_path = search_path + "\\e";//224
break;
case 15:
search_path = search_path + "\\f";//240~255
break;

}

char g=colour.g/16;
switch(g){
case 0:
search_path = search_path + "0";
break;
case 1:
search_path = search_path + "1";
break;
case 2:
search_path = search_path + "2";
break;
case 3:
search_path = search_path + "3";
break;
case 4:
search_path = search_path + "4";
break;
case 5:
search_path = search_path + "5";
break;
case 6:
search_path = search_path + "6";
break;
case 7:
search_path = search_path + "7";
break;
case 8:
search_path = search_path + "8";
break;
case 9:
search_path = search_path + "9";
break;
case 10:
search_path = search_path + "a";
break;
case 11:
search_path = search_path + "b";
break;
case 12:
search_path = search_path + "c";
break;
case 13:
search_path = search_path + "d";
break;
case 14:
search_path = search_path + "e";
break;
case 15:
search_path = search_path + "f";
break;
}

char b=colour.b/16;
switch(b){
case 0:
search_path = search_path + "0";
break;
case 1:
search_path = search_path + "1";
break;
case 2:
search_path = search_path + "2";
break;
case 3:
search_path = search_path + "3";
break;
case 4:
search_path = search_path + "4";
break;
case 5:
search_path = search_path + "5";
break;
case 6:
search_path = search_path + "6";
break;
case 7:
search_path = search_path + "7";
break;
case 8:
search_path = search_path + "8";
break;
case 9:
search_path = search_path + "9";
break;
case 10:
search_path = search_path + "a";
break;
case 11:
search_path = search_path + "b";
break;
case 12:
search_path = search_path + "c";
break;
case 13:
search_path = search_path + "d";
break;
case 14:
search_path = search_path + "e";
break;
case 15:
search_path = search_path + "f";
break;

}




return search_path;
}


struct vision_neuro_node{
long group;//所属集合序号
long order;//集合内部序号
float proportion;//在集合内所占票数比例
long total_edge_num;//总边缘数
};//16字节


struct total_vote_tree_node{
long group;//所属集合序号
float similarity;//相似度汇总，即投票总值
total_vote_tree_node* last = NULL;
total_vote_tree_node* next = NULL ;
vote_tree_inside_node* child_node = NULL;
};

struct vote_tree_inside_node{
total_vote_tree_node* root = NULL;
long order;//集合内部序号
float proportion;//在集合内所占票数比例
vote_tree_inside_node* last = NULL;
vote_tree_inside_node* next = NULL;
};

std::vector<total_vote_tree_node> total_vote_tree;
std::vector<vote_tree_inside_node> vote_tree_inside;

//查询单个点上的聚合结果
int serach_one(MapRecordTree &MapTree,std::vector<PRO_RGB> &DETAIL_MAP,std::vector<RGB> &rgbmap,
   int x,int y,int width,int mode){


//起始对象，从该点开始的该边是判断起始边
std::string search_path_origin = "vision";//起始对象字符串


//从确定起始边开始
search_path_origin = classify_edge(search_path_origin,(*MapTree.to_row[y].direct_link[x]).gradient,
rgbmap[width * y + x],DETAIL_MAP[width * y + x].edge_Kind);

//加入相连边
for(char i = 1;i < MapTree.to_row[y].every_amount_of_direct[x];i++){

search_path_origin = search_path_origin + "\\direct";

//朝向分类
switch((*(MapTree.to_row[y].direct_link[x] + i)).gradient){
case 0:
search_path_origin = search_path_origin + "\\0";
break;

case 1:
search_path_origin = search_path_origin + "\\1";
break;

case 2:
search_path_origin = search_path_origin + "\\2";
break;

case 3:
search_path_origin = search_path_origin + "\\3";
break;

case 4:
search_path_origin = search_path_origin + "\\4";
break;

case 5:
search_path_origin = search_path_origin + "\\5";
break;

case 6:
search_path_origin = search_path_origin + "\\6";
break;

case 7:
search_path_origin = search_path_origin + "\\7";
break;
}
//相对长度判断
//按0.25，0.33，0.5，0.67，1，1.5，2，3，4的范围划分，这个根据实际需求可以修改机制
float relative_length = (*MapTree.to_row[y].direct_link[x]).length / (*(MapTree.to_row[y].direct_link[x] + i)).length;
if(relative_length < 1){
    if(relative_length < 0.5){
        if(relative_length < 0.25) search_path_origin = search_path_origin + "\\0.25-";
        else if(relative_length < 0.33) search_path_origin = search_path_origin + "\\0.25_0.33";
        else search_path_origin = search_path_origin + "\\0.33_0.5";}
    else{
        if(relative_length < 0.67) search_path_origin = search_path_origin + "\\0.5_0.67";
        else search_path_origin = search_path_origin + "\\0.67_1";
    }
}
else{// >=1
    if(relative_length < 3){
        if(relative_length < 1.5) search_path_origin = search_path_origin + "\\1_1.5";
        else if (relative_length < 2)search_path_origin = search_path_origin + "\\1.5_2";
        else search_path_origin = search_path_origin + "\\2_3";}
    else{
        if(relative_length < 4) search_path_origin = search_path_origin + "\\3_4";
        else search_path_origin = search_path_origin + "\\4+";
    }
}


}//循环内
long node_count = 0;
std::vector<vision_neuro_node> vote_list(node_count);

if(mode = 0)//0是探测直接连接
{

std::string search_path_direct = "";

for(char n = 1;n < MapTree.to_row[y].every_amount_of_direct[x];n++){

search_path_direct = classify_edge(search_path_origin,
    (*(MapTree.to_row[(*MapTree.to_row[y].direct_link[x]).y].direct_link[(*MapTree.to_row[y].direct_link[x]).x])).gradient,
    rgbmap[width * (*MapTree.to_row[y].direct_link[x]).y + (*MapTree.to_row[y].direct_link[x]).x],
    DETAIL_MAP[width * (*MapTree.to_row[y].direct_link[x]).y + (*MapTree.to_row[y].direct_link[x]).x].edge_Kind);

//相对长度判断，相对长度为起始对象与直连边缘的长度之比
float relative_length = (*MapTree.to_row[y].direct_link[x]).length / (*(MapTree.to_row[(*MapTree.to_row[y].direct_link[x]).y].direct_link[(*MapTree.to_row[y].direct_link[x]).x] + n)).length;
if(relative_length < 1){
    if(relative_length < 0.5){
        if(relative_length < 0.25) search_path_origin = search_path_origin + "\\0.25-";
        else if(relative_length < 0.33) search_path_origin = search_path_origin + "\\0.25_0.33";
        else search_path_origin = search_path_origin + "\\0.33_0.5";}
    else{
        if(relative_length < 0.67) search_path_origin = search_path_origin + "\\0.5_0.67";
        else search_path_origin = search_path_origin + "\\0.67_1";
    }
}
else{// >=1
    if(relative_length < 3){
        if(relative_length < 1.5) search_path_origin = search_path_origin + "\\1_1.5";
        else if (relative_length < 2)search_path_origin = search_path_origin + "\\1.5_2";
        else search_path_origin = search_path_origin + "\\2_3";}
    else{
        if(relative_length < 4) search_path_origin = search_path_origin + "\\3_4";
        else search_path_origin = search_path_origin + "\\4+";
    }
}

search_path_direct = search_path_direct +"\\0-100.v";//0-100是优先度的范围

    std::ifstream infile(search_path_direct, std::ios::binary);
    if (!infile) {
        std::cerr << "文件打开失败！路径不存在" << std::endl;
        return 1;//1是待更新
    }

    // 读取文件的数量信息
    
    infile.read(reinterpret_cast<char*>(&node_count), sizeof(long));
    if (!infile) {
        std::cerr << "读取节点数目统计失败！" << std::endl;
        return 0;
    }

    

    // 读取所有节点
    infile.read(reinterpret_cast<char*>(vote_list.data()), node_count * sizeof(vision_neuro_node));
    if (!infile) {
        std::cerr << "读取节点内容失败！" << std::endl;
        return 0;
    }

    infile.close();

//将vote_list的内容汇总到总投票树

// 汇总

    // 1. 分组： group -> 节点列表
    std::unordered_map<long, std::vector<const vision_neuro_node*>> group_map;
    for (const auto& node : vote_list) {
        group_map[node.group].push_back(&node);
    }

    // 2. 遍历分组，生成 total_vote_tree_node

    total_vote_tree_node* prev_group = nullptr;
    for (auto& [group_id, nodes] : group_map) {
        // 新建一个 group 汇总节点
        total_vote_tree.emplace_back();
        auto* group_node = &total_vote_tree.back();
        group_node->group = group_id;
        group_node->similarity = 0.0f;

        // 处理 group 内部元素
        vote_tree_inside_node* prev_inside = nullptr;
        for (auto* vn : nodes) {
            vote_tree_inside.emplace_back();
            auto* inside_node = &vote_tree_inside.back();
            inside_node->root = group_node;
            inside_node->order = vn->order;
            inside_node->proportion = vn->proportion;

            // 累计相似度
            group_node->similarity += vn->proportion;

            // 建立组内链表
            if (!group_node->child_node) {
                group_node->child_node = inside_node;
            }
            if (prev_inside) {
                prev_inside->next = inside_node;
                inside_node->last = prev_inside;
            }
            prev_inside = inside_node;
        }

        // 建立组间链表
        if (prev_group) {
            prev_group->next = group_node;
            group_node->last = prev_group;
        }
        prev_group = group_node;
    
    
}
    

vote_list.clear();
return 2;
}

}
else//远程连接
{
    for(int b =0;b <MapTree.to_row[y].every_amount_of_remote[x];b++){

std::string search_path_remote = search_path_origin +"\\remote";
for(int g = 0;g < MapTree.to_row[y].every_amount_of_remote[x];g++){

//方向分类
switch((*(MapTree.to_row[y].remote_link[x])).gradient){
case 0:
search_path_remote = search_path_remote + "\\0";
break;

case 1:
search_path_remote = search_path_remote + "\\1";
break;

case 2:
search_path_remote = search_path_remote + "\\2";
break;

case 3:
search_path_remote = search_path_remote + "\\3";
break;

case 4:
search_path_remote = search_path_remote + "\\4";
break;

case 5:
search_path_remote = search_path_remote + "\\5";
break;

case 6:
search_path_remote = search_path_remote + "\\6";
break;

case 7:
search_path_remote = search_path_remote + "\\7";
break;
}


//相对距离分类
float relative_distance = (*(MapTree.to_row[y].remote_link[x]+b)).length;
if(relative_distance < 1){
    if(relative_distance < 0.5){
        if(relative_distance < 0.1) search_path_remote = search_path_remote + "\\0.1-";
        else if(relative_distance < 0.2) search_path_remote = search_path_remote + "\\0.1_0.2";
        else search_path_remote = search_path_remote + "\\0.2_0.3";}
    else{
        if(relative_distance < 0.4) search_path_remote = search_path_remote + "\\0.3_0.4";
        else search_path_remote = search_path_remote + "\\0.4_0.5";
    }
}
else{// >=0.5
    if(relative_distance < 0.8){
        if(relative_distance < 0.6) search_path_remote = search_path_remote + "\\0.5_0.6";
        else if (relative_distance < 0.7)search_path_remote = search_path_remote + "\\0.6_0.7";
        else search_path_remote = search_path_remote + "\\0.7_0.8";}
    else{
        if(relative_distance < 0.9) search_path_remote = search_path_remote + "\\0.8_0.9";
        else search_path_remote = search_path_remote + "\\0.9+";
    }
}

search_path_remote = classify_edge(search_path_remote,
    (*(MapTree.to_row[y].remote_link[x] + g)).gradient,
    rgbmap[width * (*MapTree.to_row[y].remote_link[x]).y + (*MapTree.to_row[y].remote_link[x]).x],
    DETAIL_MAP[width * (*MapTree.to_row[y].remote_link[x]).y + (*MapTree.to_row[y].remote_link[x]).x].edge_Kind);


for(char k = 0;k<MapTree.to_row[y].every_amount_of_direct[x];k++){

//朝向分类
switch((*(MapTree.to_row[y].direct_link[x] + k)).gradient){
case 0:
search_path_remote = search_path_remote + "\\0";
break;

case 1:
search_path_remote = search_path_remote + "\\1";
break;

case 2:
search_path_remote = search_path_remote + "\\2";
break;

case 3:
search_path_remote = search_path_remote + "\\3";
break;

case 4:
search_path_remote = search_path_remote + "\\4";
break;

case 5:
search_path_remote = search_path_remote + "\\5";
break;

case 6:
search_path_remote = search_path_remote + "\\6";
break;

case 7:
search_path_remote = search_path_remote + "\\7";
break;
}


//相对长度
float relative_length = (*MapTree.to_row[y].direct_link[x]).length / (*(MapTree.to_row[(*MapTree.to_row[y].remote_link[x]).y].direct_link[(*MapTree.to_row[y].remote_link[x]).x] + k)).length;
if(relative_length < 1){
    if(relative_length < 0.5){
        if(relative_length < 0.25) search_path_remote = search_path_remote + "\\0.25-";
        else if(relative_length < 0.33) search_path_remote = search_path_remote + "\\0.25_0.33";
        else search_path_remote = search_path_remote + "\\0.33_0.5";}
    else{
        if(relative_length < 0.67) search_path_remote = search_path_remote + "\\0.5_0.67";
        else search_path_remote = search_path_remote + "\\0.67_1";
    }
}
else{// >=1
    if(relative_length < 3){
        if(relative_length < 1.5) search_path_remote = search_path_remote + "\\1_1.5";
        else if (relative_length < 2)search_path_remote = search_path_remote + "\\1.5_2";
        else search_path_remote = search_path_remote + "\\2_3";}
    else{
        if(relative_length < 4) search_path_remote = search_path_remote + "\\3_4";
        else search_path_remote = search_path_remote + "\\4+";
    }
}

search_path_remote = search_path_remote +"\\0-100.v";

    std::ifstream infile(search_path_remote, std::ios::binary);
    if (!infile) {
        std::cerr << "文件打开失败！" << std::endl;
        return 1;
    }

    // 读取文件的数量信息
    
    infile.read(reinterpret_cast<char*>(&node_count), sizeof(long));
    if (!infile) {
        std::cerr << "读取节点数目统计失败！" << std::endl;
        return 1;
    }

    
    // 读取所有节点
    infile.read(reinterpret_cast<char*>(vote_list.data()), node_count * sizeof(vision_neuro_node));
    if (!infile) {
        std::cerr << "读取节点内容失败！" << std::endl;
        return 1;
    }

    infile.close();

//汇总远距离端点的投票结果

    // 1. 分组： group -> 节点列表
    std::unordered_map<long, std::vector<const vision_neuro_node*>> group_map;
    for (const auto& node : vote_list) {
        group_map[node.group].push_back(&node);
    }

    // 2. 遍历分组，生成 total_vote_tree_node

    total_vote_tree_node* prev_group = nullptr;
    for (auto& [group_id, nodes] : group_map) {
        // 新建一个 group 汇总节点
        total_vote_tree.emplace_back();
        auto* group_node = &total_vote_tree.back();
        group_node->group = group_id;
        group_node->similarity = 0.0f;

        // 处理 group 内部元素
        vote_tree_inside_node* prev_inside = nullptr;
        for (auto* vn : nodes) {
            vote_tree_inside.emplace_back();
            auto* inside_node = &vote_tree_inside.back();
            inside_node->root = group_node;
            inside_node->order = vn->order;
            inside_node->proportion = vn->proportion;

            // 累计相似度
            group_node->similarity += vn->proportion;

            // 建立组内链表
            if (!group_node->child_node) {
                group_node->child_node = inside_node;
            }
            if (prev_inside) {
                prev_inside->next = inside_node;
                inside_node->last = prev_inside;
            }
            prev_inside = inside_node;
        }

        // 建立组间链表
        if (prev_group) {
            prev_group->next = group_node;
            group_node->last = prev_group;
        }
        prev_group = group_node;
    
    
}
}

}

vote_list.clear();
return 3;
}

   }
}


// 返回 similarity 最大的前 n 个 group_id
std::vector<long> top_n_group_ids(
    std::vector<total_vote_tree_node>& total_vote_tree,
    size_t n
) {
    // 临时存储指针
    std::vector<total_vote_tree_node*> nodes;
    nodes.reserve(total_vote_tree.size());

    for (auto& node : total_vote_tree) {
        nodes.push_back(&node);
    }

    if (n > nodes.size()) n = nodes.size();

    // 部分排序，取前 n 个 similarity 最大的
    std::partial_sort(
        nodes.begin(),
        nodes.begin() + n,
        nodes.end(),
        [](const total_vote_tree_node* a, const total_vote_tree_node* b) {
            return a->similarity > b->similarity; // 降序
        }
    );

    // 取出 group_id
    std::vector<long> result;
    result.reserve(n);
    for (size_t i = 0; i < n; i++) {
        result.push_back(nodes[i]->group);
    }

    return result;
}


// 返回 similarity ≥ m 的 group_id 列表，以及数量
std::pair<std::vector<long>, size_t> groups_with_min_similarity(
    const std::vector<total_vote_tree_node>& total_vote_tree,
    float m
) {
    std::vector<long> result;

    for (const auto& node : total_vote_tree) {
        if (node.similarity >= m) {
            result.push_back(node.group);
        }
    }

    return {result, result.size()};
}

//查找较高优先度的概念现象(所得为结果列表)
std::vector<long> serach_block(MapRecordTree &MapTree,int top,int bottom,int left,int right){






;
}

struct link_record_objedt{
short x;
short y;
short targrt_x;
short targrt_y;
};



//视觉数据库生成
long vision_group_number = 0;
//集合号码取用与更新
long get_group_newest(){
    std::ifstream ifile("Vision_Group.n",std::ios::binary);
    ifile.read(reinterpret_cast< char*>(vision_group_number), 4);
    ifile.close();
    long number = vision_group_number;
    vision_group_number += 1;
    std::ofstream ofile("Vision_Group.n",std::ios::binary);
    ofile.write(reinterpret_cast<char*>(vision_group_number), 4);
    ofile.close();
    return number;
}

//将视觉特征(边缘)制作成集合
void vision_group_record(MapRecordTree &MapTree,std::vector<std::string> path,
    std::vector<bool> mode,std::vector<link_record_objedt> read_list){
long group = get_group_newest();
int total_edge_num = path.size();
int order = 0;
int edge_grid = 0;
std::vector<int> single_proportion;
for(int d = 0;d < total_edge_num;d++){//统计总票数

//连接原对象的占比统计
for(char t = 0;t < MapTree.to_row[read_list[order].y].every_amount_of_direct[read_list[order].x];t++){
    int f = (MapTree.to_row[read_list[order].y].direct_link[read_list[order].x] + t)->length;
    edge_grid += f;single_proportion[order] += f;
}
//连接目标对象的占比统计
for(char e = 0;e < MapTree.to_row[read_list[order].targrt_y].every_amount_of_direct[read_list[order].targrt_x];e++){
    int f = (MapTree.to_row[read_list[order].targrt_y].direct_link[read_list[order].targrt_x] + e)->length;
    edge_grid += f;single_proportion[order] += f;
}

order += 1;
}//求解得知edge_grid总数



for(order = 0;order < total_edge_num;order += 1){

std::ifstream ifile(path[order],std::ios::binary);
if(!ifile.good()){
std::ofstream ofile(path[order],std::ios::binary);
long num = 0;
ofile.write(reinterpret_cast<char*>(num), sizeof(long));
ofile.close();
}
int group_num;
std::vector<vision_neuro_node> node;
ifile.read(reinterpret_cast<char*>(group_num), sizeof(long));
node.reserve(group_num + total_edge_num);
ifile.read(reinterpret_cast<char*>(node.data()), group_num * sizeof(vision_neuro_node));


vision_neuro_node l;
l.group = group;
l.order = order;
l.proportion = single_proportion[order]/edge_grid;
l.total_edge_num = total_edge_num;

node.push_back(l);

std::ofstream ofile(path[order],std::ios::binary);
ofile.write(reinterpret_cast<char*>(node.data()), sizeof(node));
ofile.close();


}
};
