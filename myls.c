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
#include <limits.h>

char ca[256] = {0};//用ASCII码充当下标，标记字母是否出现过的数组

struct fileid //用于存放文件路径与inode
{
    char name[PATH_MAX];//使用linux系统单条路径最高上限保证安全
    long unsigned int inode;
};

char filetype(char* s);//提前声明                             
void printfile(struct fileid flid, char* s);
void indexprocess(char* s);
bool cmp_dictionary(char* a, char* b);
bool cmp_time(char* a, char* b);
void sort_dictionary(struct fileid arr[], int l, int r, char* s);
void sort_time(struct fileid arr[], int l, int r, char* s);

void printfile(struct fileid flid, char* s) //用于打印出所需内容（输入参数为包含文件名称与inode的结构体与地址）
{
    (void)s;//暂时未使用s，先转为void防止报警
    const char *base = strrchr(flid.name, '/');//找到最后一个‘/’
    base = (base != NULL) ? base + 1 : flid.name;//若有'/'则跳过，没有则使用整串

    struct stat st;
    int ok = (lstat(flid.name, &st) == 0);

    if(ca['i'] != 0)//检测命令i
    {
        printf("%8lu ", flid.inode);//显示inode号
    }

    if(ca['l'] != 0 && ok != 0)//检测命令l
    {
        if(ca['s'] != 0)//检测命令s
        {
            printf("%6ld ", (long)(st.st_blocks / 2));//把512字节块数的st_blocks转换为kb为单位并输出磁盘块数大小
        }

        char perm[11];//建立权限字符缓冲区

        perm[0] = filetype(flid.name);//类型位
        perm[1] = (st.st_mode & S_IRUSR) ? 'r': '-';//判断有无所有者读权限，有则'r'，无则'-'
        perm[2] = (st.st_mode & S_IWUSR) ? 'w': '-';//判断有无所有者写权限
        perm[3] = (st.st_mode & S_IXUSR) ? ((st.st_mode & S_ISUID) ? 's': 'x') :((st.st_mode & S_ISUID) ? 'S' : '-');//先判断是否可所有者执行，再判断可否setuid
        perm[4] = (st.st_mode & S_IRGRP) ? 'r': '-';//判断有无组读权限
        perm[5] = (st.st_mode & S_IWGRP) ? 'w': '-';//判断有无组写权限
        perm[6] = (st.st_mode & S_IXGRP) ? ((st.st_mode & S_ISGID) ? 's': 'x') :((st.st_mode & S_ISGID) ? 'S' : '-');//先判断是否可组执行，再判断可否setgid
        perm[7] = (st.st_mode & S_IROTH) ? 'r': '-';//判断其他读
        perm[8] = (st.st_mode & S_IWOTH) ? 'w': '-';//判断其他写
        perm[9] = (st.st_mode & S_IXOTH) ? ((st.st_mode & S_ISVTX) ? 't': 'x') :((st.st_mode & S_ISVTX) ? 'T' : '-');//先判断其他执行，在判断sticky
        perm[10] = '\0';

        struct passwd *pw = getpwuid(st.st_uid);//根据uid查出用户名
        struct group *gr = getgrgid(st.st_gid);//根据gid查出组名
        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%b %e %H:%M",localtime(&st.st_mtime));//先用st.st_mtime获取文件最后修改时间，再用localtime(&st.st_mtime)把time_t转成本地时间的struct tm*，最后用strftime函数按“月份缩写 日期 小时：分钟”的格式写入tbuf

        printf("%s %3lu %-8s %-8s %8ld %s %s\n", perm, (unsigned long)st.st_nlink, ((pw != NULL) ? pw->pw_name : "?"),  ((gr != NULL) ? gr->gr_name : "?"),(long)st.st_size, tbuf, base);//依序输出权限字符串，硬链接数，用户名，组名，文件大小，格式化后修改时间，文件名
        return;
    }

    if(ca['l'] == 0 && ca['s'] != 0 && ok != 0)
    {
        printf("%6ld ", (long)(st.st_blocks / 2));//检测命令s,显示kb格式的磁盘块大小
    }
    printf("%s\n", base);//保险
}

bool cmp_dictionary(char* a, char* b)//检查a与b的字典序大小
{
    return (strcmp(a, b) < 0);//返回true表示a在字典序上应排在b前面
}

bool cmp_time(char* a, char* b)//检查a与b的时间序大小
{
    (void)a; (void)b;
    return strcmp(a, b) < 0;        /* 暂时按字典序，阶段 5 换真正的 */
}

void sort_dictionary(struct fileid arr[], int l, int r, char* s)//针对字典顺序的快速排序
{
    (void)s;//防警告占位符
    if(l >= r)
    {
        return;
    }
    int i = l , j = r;
    struct fileid pivot = arr[l + (r - l) / 2];//定义快速排序所需的基准点
    while(i <= j)//
    {
        while(i <= r &&cmp_dictionary(arr[i].name, pivot.name) == true)//从左找第一个>=pivot的
        {
            i++;
        }
        while (j >= l &&cmp_dictionary(pivot.name, arr[j].name) == true)//从右找第一个<=pivot的
        {
            j--;
        }
        if(i <= j)
        {
            struct fileid t = arr[i];
            arr[i] = arr[j];
            arr[j] = t;
            i++;
            j--;
        }
    }
    sort_dictionary(arr, l, j, s);//通过递归划分左右部分再次排序以实现快速排序
    sort_dictionary(arr, i, r, s);
}

void sort_time(struct fileid arr[], int l, int r, char* s)
{
    (void)arr; (void)l; (void)r; (void)s;
    /* 空实现，阶段 5 再写 */
}

char filetype(char* s) //用于区分文件类型
{
    struct stat st;
    if(lstat(s, &st) < 0)//获取目录状态信息，失败则返回负值
    {
        return '?';//表示类型未知
    }
    if(S_ISDIR(st.st_mode) != 0)
    {
        return 'd';//检测类型是否为目录
    }
    if(S_ISLNK(st.st_mode) != 0)
    {
        return 'l';//检测类型是否为符号连接
    }
    if(S_ISREG(st.st_mode) != 0)
    {
        return '-';//检测是否为普通文件
    }
    if(S_ISCHR(st.st_mode) != 0)
    {
        return 'c';//检测是否为字符设备
    }
    if(S_ISBLK(st.st_mode) != 0)
    {
        return 'b';//检测是否为块设备
    }
    if(S_ISFIFO(st.st_mode) != 0)
    {
        return 'p';//检测是否为命名管道
    }
    if(S_ISSOCK(st.st_mode) != 0)
    {
        return 's';//检测是否为套接字
    }
    return '?';//保险，理论上不使用
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
    while((de = readdir(d)) != NULL)//循环读取下一个目录项并判断其是否存在
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
    sort_dictionary(arr, 0, n - 1, s);//导入字典序排序函数进行排序

    if(ca['l'] != 0)//计算并输出总kb格式的磁盘块大小
    {
        long total = 0;
        for(int i = 0 ; i < n ; i++)
        {
            struct stat ss;
            if(lstat(arr[i].name, &ss) == 0) 
            {
                total += ss.st_blocks / 2;
            }
        }
        printf("total %ld\n", total);
    }


    for(int i = 0 ; i < n ; i++)//使用printfile逐个打印出文件名称
    {
        printfile(arr[i], s);
    }

    free(arr);//释放内存防止泄漏
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
                ca[(unsigned char)argv[i][j]]++;//直接使用ASCII码充当下标，记录命令中字母的出现次数
            }
        }else if(i > 0)
        {
            paths[pathnum++] = argv[i];//存入命令字符串本身
        }
    }

    if(pathnum == 0)//在没有给出路径的情况下则默认为当前目录
    {
        paths[pathnum++] = ".";//未给命令时使程序等价于ls .
    }
    for(int i = 0 ; i < pathnum ; i++)//导入函数中对目录逐个处理
    {
        struct stat st;
        if(lstat(paths[i], &st) == 0 && S_ISDIR(st.st_mode) != 0 && (pathnum > 1 || ca['R'] != 0))//判断是否需要在列出目录之前打印“路径：”作为标题
        {
            printf("%s:\n", paths[i]);
        }
        indexprocess(paths[i]);
    }

    return 0;
}