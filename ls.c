#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

char ca[128] = {0};//用ASCII码充当下标，标记字母是否出现过的数组

struct fileid //用于存放文件路径与inode
{
    char name[PATH_MAX];//使用linux系统单条路径最高上限保证安全
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
    struct stat st;
    if(lstat(s, &st) < 0)//获取目录的状态信息，失败则返回-1
    {
        perror(s);//打印标准错误输出
        return;
    }

    if(S_ISDIR(st.st_mode) == 0)//使用宏S_ISDIR判断st中的st_mode是否表示一个目录,若不表示目录则为真
    {
        struct fileid f;
        snprintf(f.name, sizeof(f.name), "%s", s);//使用snprintf函数格式化输出到字符串
        f.inode = (long unsigned int)st.st_ino;//将st中的inode赋值给f
        printfile(f, s);//调用自定义的printfile函数打印出文件的名称
        return;
    }

    DIR *d = opendir(s);//使用opendir函数打开目录s，并返回指向目录流句柄类型DIR的指针到d
    if(d == NULL)//判断打开是否成功
    {
        perror(s);
        return;
    }

    int cap = 64, n = 0;//cap代表能存放的文件数，n代表实际已存放的文件
    struct fileid *arr = malloc(sizeof(struct fileid) * cap);
    if(arr == NULL)//防止arr内存分配失败
    {
        closedir(d);
        return;
    }

    struct dirent *de;//新建一个类型为表示目录项的结构体dirent的变量de
    while((d = readdir(d)) != NULL)//循环读取下一个目录项并判断其是否存在
    {
        if(ca['a'] == 0 && de->d_name[0] == '.')//在未输入a时过滤掉.开头的隐藏文件
        {
            continue;
        }

        if(n == cap)//在以存放的文件达到预设上限时对上限进行扩张(用realloc重新分配内存大小)
        {
            cap *= 2;
            struct fileid *temp = realloc(arr, sizeof(struct fileid) * cap);
            if(temp == NULL)//防止重分配失败
            {
                free(arr);
                closedir(d);
                return;
            }
            arr = temp;
        }

        size_t ls = strlen(s);
        int ret;
        if(ls > 0 && s[ls - 1] == '/')//判断目录路径s末尾是否已经带/，决定拼路径时是否再补分隔符
        {
            ret =  snprintf(arr[n].name, sizeof(arr[n].name), "%s%s",  s, de->d_name);
        }else
        {
            ret = snprintf(arr[n].name, sizeof(arr[n].name), "%s/%s",  s, de->d_name);
        }
        if(ret >= (int)sizeof(arr[n].name))//判断路径是否过长
        {
            fprintf(stderr, "myls: path too long, skip %s\n", de->d_name);//使用fprintf将针对路径过长的警告文本写入标准错误流stderr
            continue;
        }
        arr[n].inode = (long unsigned int)de->d_ino;//存入对应文件的inode
        n++;
    }
    closedir(d);

    for(int i = 0 ; i < n ; i++)//使用printfile逐个打印出文件名称
    {
        printfile(arr[i], s);
    }

    free(arr);//释放内存防止泄漏
}

void sort_dictionary(struct fileid arr[], int l, int r, char* s)//针对字典顺序的快速排序
{

}

void sort_time(struct fileid arr[], int l, int r, char* s)//针对时间顺序的快速排序
{

}

void printfile(struct fileid flid, char* s) //用于打印出所需内容（输入参数为包含文件名称与inode的结构体与地址）
{
    (void)s;//暂时未使用s，先转为void防止报警
    const char *base = strrchr(flid.name, '/');//找到最后一个‘/’
    base = (base != NULL) ? base + 1 : flid.name;//若有'/'则跳过，没有则使用整串
    printf("%s\n", base);
}


int main(int argc, char* argv[]) //主函数，读取命令行参数并加以处理
{
    int pathnum = 0;//路径的数量
    char* paths[argc > 1 ? argc : 1];//用于存入非选项的参数

    for(int i = 0 ; i < argc ; i++)//先录入所有的参数
    {
        if(argv[i][0] == '-')
        {
            for(int j = 1 ; argv[i][j] != '\0' ; j++)
            {
                ca[argv[i][j]]++;//直接使用ASCII码充当下标，记录命令中字母的出现次数
            }
        }else if(i > 0)
        {
            paths[pathnum++] = argv[i];//存入命令字符串本身
        }
    }

    if(pathnum == 0)//在没有给出路径的情况下则默认为当前目录
    {
        paths[pathnum++] = '.';//未给命令时使程序等价于ls .
    }
    for(int i = 0 ; i < pathnum ; i++)//导入函数中对目录逐个处理
    {
        indexprocess(paths[i]);
    }

    return 0;
}