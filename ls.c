#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

char ca[128] = {0};//用ASCII码充当下标，标记字母是否出现过的数组

struct fileid //用于存放文件名称与inode
{
    char name[256];
    long unsigned int inode;
};

bool cmp_dictionary(char* a, char* b)//检查a与b的字典序大小
{

}

bool cmp_time(char* a, char* b)//检查a与b的时间序大小
{

}

char filetype(char* s) //用于区分文件类型
{

}

void indexprocess(char* s) //用于处理目录下内容
{

}

void sort_dictionary(struct fileid arr, int l, int r, char* s)//针对字典顺序的快速排序
{

}

void sort_time(struct fileid arr, int l, int r, char* s)//针对时间顺序的快速排序
{

}

void printfile(struct fileid flid, char* s) //用于打印出所需内容（输入参数为包含文件名称与inode的结构体与地址）
{

}


int main(int argc, char* argv[]) //主函数，读取命令行参数并加以处理
{
    int pathnum = 0;//路径的数量
    for(int i = 0 ; i < argc ; i++)//先录入所有的参数
    {
        if(argv[i][0] == '-')
        {
            for(int j = 1 ; argv[i][j] != '\0' ; j++)
            {
                ca[argv[i][j]]++;//直接使用ASCII码充当下标，记录命令中字母的出现次数
            }
        }
    }
    
}