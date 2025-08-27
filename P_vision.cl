
//生成边缘图
__kernel void third_kernel(__global int width,__global int height,
__global uchar* rgb,__global uchar* detail)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    if (x >= width || y >= height) return;


short r,g,b,r_,g_,b_;
    
    if(x>0)//左
    {
    r = rgb[(width * y + x)*3];     r_ = rgb[(width * y + x - 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * y + x - 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * y + x - 1)*3 + 2];
    if( abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16 )
    //set_bit(detail[(width * y + x)*2], 0);
    detail[(width * y + x)*2] |= (1 << 0);}
    
    if(x<width-1)//右
    {
    r = rgb[(width * y + x)*3] ;    r_ = rgb[(width * y + x + 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * y + x + 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * y + x + 1)*3 + 2];
    if(abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16)
    //set_bit(detail[(width * y + x)*2], 1);
    detail[(width * y + x)*2] |= (1 << 1);}

    if(y>0)//上
    {
    r = rgb[(width * y + x)*3] ;    r_ = rgb[(width * (y - 1) + x)*3 ];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y - 1) + x)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y - 1) + x)*3 + 2];
    if(abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16)
    //set_bit(detail[(width * y + x)*2], 2);
    detail[(width * y + x)*2] |= (1 << 2);}

    if(y<height-1)//下
    {
    r = rgb[(width * y + x)*3] ;    r_ = rgb[(width * (y + 1) + x)*3 ];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y + 1) + x)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y + 1) + x)*3 + 2];
    if(abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16)
    //set_bit(detail[(width * y + x)*2], 3);
    detail[(width * y + x)*2] |= (1 << 3);}

    if(x>0 && y>0)//左上
    {
    r = rgb[(width * y + x)*3];     r_ = rgb[(width * (y - 1) + x - 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y - 1) + x - 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y - 1) + x - 1)*3 + 2];
    if( abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16 )
    //set_bit(detail[(width * y + x)*2], 4);
    detail[(width * y + x)*2] |= (1 << 4);}

    

    if(x<width-1 && y>0)//右上
    {
    r = rgb[(width * y + x)*3];     r_ = rgb[(width * (y - 1) + x + 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y - 1) + x + 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y - 1) + x + 1)*3 + 2];
    if( abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16 )
    //set_bit(detail[(width * y + x)*2], 5);
    detail[(width * y + x)*2] |= (1 << 5);}

    

    if(x>0 && y<height-1)//左下
    {
    r = rgb[(width * y + x)*3];     r_ = rgb[(width * (y + 1) + x - 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y + 1) + x - 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y + 1) + x - 1)*3 + 2];
    if( abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16 )
    //set_bit(detail[(width * y + x)*2], 6);
    detail[(width * y + x)*2] |= (1 << 6);}

    

    if(x<width-1 && y<height-1)//右下
    {
    r = rgb[(width * y + x)*3];     r_ = rgb[(width * (y + 1) + x + 1)*3];
    g = rgb[(width * y + x)*3 + 1]; g_ = rgb[(width * (y + 1) + x + 1)*3 + 1];
    b = rgb[(width * y + x)*3 + 2]; b_ = rgb[(width * (y + 1) + x + 1)*3 + 2];
    if( abs(r - r_) > 16 || abs(g - g_) > 16 || abs(b - b_) > 16 )
    //set_bit(detail[(width * y + x)*2], 7);
    detail[(width * y + x)*2] |= (1 << 7);}

    


//加工 “边缘处理结果数组”
//
//初始化端点、封闭图形
//1、找到待判定的点
    if ( (DETAIL_MAP[ (width * y + x)*2 ].edge_Kind > 0b0) )
    DETAIL_MAP[ (width * y + x)*2 + 1].point_kind = 3;

//2、分离孤立边缘
int count_down = 0;int count_up = 0;

    if(DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 3)
    {
    if(x>0){//左检测
        if(DETAIL_MAP[(width * y + x - 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 0 & 1 != 1) continue;
    }

    if(x < width - 1){//右检测
        if(DETAIL_MAP[(width * y + x + 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 1 & 1 != 1) continue;
    }

    if(y>0){//上检测
        if(DETAIL_MAP[(width * (y - 1) + x)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 2 & 1 != 1) continue;
    }

    if(y<height){//下检测
        if(DETAIL_MAP[(width * (y + 1) + x)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 3 & 1 != 1) continue;
    }

    if(x>0 && y>0){//左上检测
        if(DETAIL_MAP[(width * (y - 1) + x - 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 4 & 1 != 1) count_down += 1;
    }

    if(x < width - 1 && y>0){//右上检测
        if(DETAIL_MAP[(width * (y - 1) + x + 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 5 & 1 != 1) count_up += 1;
    }

    if(x>0 && y<height){//左下检测
        if(DETAIL_MAP[(width * (y + 1) + x - 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 6 & 1 != 1) count_up += 1;
    }

    if(x < width - 1 && y<height){//右下检测
        if(DETAIL_MAP[(width * (y + 1) + x + 1)*2 + 1].point_kind == 3 && DETAIL_MAP[(width * y + x)*2] >> 7 & 1 != 1) count_down += 1;
    }

    //所有孤立点被分类为2，1，5
    if((count_down == 2 || count_up == 2) && (count_down + count_up) == 2)DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 7;//斜直线
    else if( (count_down == 1 || count_up == 1) && (count_down + count_up) == 1 ) DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 5;//一端
    else DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 1;//一角或斜十或斜三叉
    }




//3、直线段构造
if(DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 3){
    char count_x = 0;char count_y = 0;

    if(x>0){//左检测
        if(DETAIL_MAP[(width * y + x - 1)*2 + 1].point_kind = 3 && DETAIL_MAP[(width * y + x)*2] >> 0 & 1 != 1);
        count_x += 1;
    }

    if(x < width - 1){//右检测
        if(DETAIL_MAP[(width * y + x + 1)*2 + 1].point_kind = 3 && DETAIL_MAP[(width * y + x)*2] >> 1 & 1 != 1);
        count_x += 1;
    }

    if(y>0){//上检测
        if(DETAIL_MAP[(width * (y - 1) + x)*2 + 1].point_kind = 3 && DETAIL_MAP[(width * y + x)*2] >> 2 & 1 != 1);
        count_y += 1;
    }

    if(y<height){//下检测
        if(DETAIL_MAP[(width * (y + 1) + x)*2 + 1].point_kind = 3 && DETAIL_MAP[(width * y + x)*2] >> 3 & 1 != 1);
        count_y += 1;
    }

    //所有直线点3被分类成预备端点5和直连线4，端点1
    if(count_x <= 1 && count_y <= 1){DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 5;}//一端或一个直角
    else if(count_x + count_y == 2)DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 6;//直线内部
    else DETAIL_MAP[(width * y + x)*2 + 1].point_kind = 1;//十或三叉形
}


}
    















