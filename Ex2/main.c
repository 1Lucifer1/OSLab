#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#pragma pack () /*取消指定对齐，恢复缺省对齐*/
typedef unsigned char u8;    //1字节
typedef unsigned short u16;    //2字节
typedef unsigned int u32;    //4字节

#define FAT_START  0x0200                // FAT12中，FAT区从第1扇区开始，1 * 512 = 512 = 0x0200
#define ROOT_START 0x2600                // FAT12中，根目录区从第19扇区开始，19 * 512 = 9728 = 0x2600
#define DATA_START 0x4200                // FAT12中，数据区从第33扇区开始，33 * 512 = 16896 = 0x4200

#define EXIT 0
#define LIST_FILE 1
#define LIST_FILE_L 2
#define CAT 3
#define MIS_PARAM 4

#define EXIT_TYPE -1
#define DIR_TYPE 0
#define FILE_TYPE 1
#define DOT_TYPE 2
#define DOUBLE_DOT_TYPE 3

#define TRUE 1
#define FALSE 0

char filePath[] = "../a.img";
char input[512];
int input_pos = 0;
FILE *fat12;

struct ListNode {
    char name[20]; // 文件或目录的单独名称
    char totalName[60]; // 文件或目录的总名称，点和点点还有终止都不算
    u32 size; // 文件为正常大小，目录为0；
    int startPos; // 内容的起始位置，方便文件读取。点和点点为0
    int type; // 0为文件夹，1为文件，-1为终止结点
    int subPos; // 指向子目录的起始下标，txt或点或点点都为-1
    int fileNum;
    int dirNum;
};
int listSize = 0; // 数组的大小,也作为可插入坐标计数
struct ListNode listTotal[100];

//根目录条目
struct RootEntry {
    char DIR_Name[11]; // 文件名8字节 扩展名3字节
    u8 DIR_Attr; // 文件属性
    char reserved[10]; // 保留位
    u16 DIR_WrtTime; // 最后一次写入时间
    u16 DIR_WrtDate; // 最后一次写入日期
    u16 DIR_FstClus; // 此条目对应的开始簇号
    u32 DIR_FileSize; // 文件大小
};
//根目录条目结束，32字节

/**
 * make this level end
 */
void initEndNode() {
    listTotal[listSize].name[0] = '0';
    listTotal[listSize].totalName[0] = '0';
    listTotal[listSize].subPos = -1;
    listTotal[listSize].startPos = 0;
    listTotal[listSize].size = -1;
    listTotal[listSize].type = EXIT_TYPE;
    listSize++;
}

/**
 * root file init
 */
void listInit() {
    listTotal[listSize].name[0] = '/';
    listTotal[listSize].totalName[0] = '/';
    listTotal[listSize].subPos = 2;
    listTotal[listSize].startPos = DATA_START;
    listTotal[listSize].size = -1;
    listTotal[listSize].type = DIR_TYPE;
    listTotal[listSize].fileNum = 0;
    listTotal[listSize].dirNum = 0;
    listSize++;
    initEndNode();
}

/**
 *
 * @param name 文件名
 * @return 0 文件名错误， 1 普通文件， 2 点， 3 点点
 */
int identifyEntry(const char *name) {
    int flag = FILE_TYPE;
    if (name[0] == '.') {
        flag = DOT_TYPE;
        if (name[1] == '.') {
            flag = DOUBLE_DOT_TYPE;
        }
        for (int i = 2; i < 11; i++) {
            if (name[i] != ' ') {
                return 0;
            }
        }
        return flag;
    }
    for (int i = 0; i < 11; i++) {
        if (!((name[i] >= '0' && name[i] <= '9')
              || (name[i] >= 'a' && name[i] <= 'z')
              || (name[i] >= 'A' && name[i] <= 'Z')
              || name[i] == ' ' || name[i] == '_')) {
            return 0;
        }
    }
    return flag;
}

/**
 *
 * @param parentNode 父节点
 * @param begin 读取的起始位置
 */
void load(int parentNode, int begin) {
    listTotal[parentNode].subPos = listSize;
    struct RootEntry entry;
    struct RootEntry *entry_ptr = &entry;
    for (int i = 0; i < 15; i++) {
        fseek(fat12, begin + i * 32, 0);
        fread(entry_ptr, 1, 32, fat12); // 根目录中每个条目占32个字节，每次读1个字节

        // windows下制作的软盘x00是文件，linux里是x20，文件夹都是x10，但linux里会有长文件问题，不知道img来源故这里只过滤linux中的长文件
        if (entry_ptr->DIR_Attr == 0x20 || entry_ptr->DIR_Attr == 0x10 || entry_ptr->DIR_Attr == 0x00) {
            struct ListNode *listNode = &listTotal[listSize];

            int entryType = identifyEntry(entry_ptr->DIR_Name);
            if (entryType == 0) continue;
            if (entryType == DOT_TYPE || entryType == DOUBLE_DOT_TYPE) {
                listNode->name[0] = '.';
                listNode->totalName[0] = '.';
                if (entryType == 3) {
                    listNode->name[1] = '.';
                    listNode->totalName[1] = '.';
                }
                listNode->type = FILE_TYPE;
                listNode->subPos = -1;
                listNode->size = -1;
                listNode->startPos = 0;
            } else {
                listNode->type = (entry_ptr->DIR_Attr == 0x10) ? DIR_TYPE : FILE_TYPE;
                listNode->size = (entry_ptr->DIR_Attr == 0x10) ? 0 : entry_ptr->DIR_FileSize;
                if (entry_ptr->DIR_Attr == 0x10) {
                    listTotal[parentNode].dirNum++;
                } else {
                    listTotal[parentNode].fileNum++;
                }
                listNode->startPos = (entry_ptr->DIR_FstClus + 31) * 512; // 文件也要递归的，起始簇号为2,2+31=33
                int cnt = 0;
                char *name = listNode->name;
                while (entry_ptr->DIR_Name[cnt] != 0x20) {
                    name[cnt] = entry_ptr->DIR_Name[cnt];
                    cnt++;
                }
                if (entry_ptr->DIR_Attr != 0x10) {
                    name[cnt++] = '.';
                    name[cnt++] = 'T';
                    name[cnt++] = 'X';
                    name[cnt++] = 'T';
                    listNode->subPos = -1;
                }
                u32 pos = strlen(listTotal[parentNode].totalName);
                memcpy(listNode->totalName, listTotal[parentNode].totalName, pos);
                memcpy(listNode->totalName + pos, listNode->name, cnt);
                listNode->totalName[pos + cnt] = '/';
                listNode->totalName[pos + cnt + 1] = '\0';

                name[cnt] = '\0';

            }
        }
        listSize++;
    }
    initEndNode();

    // recursion to find each sub file
    int end = listSize;
    for (int i = listTotal[parentNode].subPos; i < end; i++) {
        if ((listTotal[i].type == DIR_TYPE) && (listTotal[i].startPos != 0)) {
            load(i, listTotal[i].startPos);
        }
    }
}

void loadFile() {
    fat12 = fopen(filePath, "rb");    // 打开FAT12的映像文件
    assert(fat12 >= 0);
    listInit();
    load(0, ROOT_START);
}

int ls_print(const char *name, int isPrint, int root, int l_param) {
    struct ListNode *listNode = &listTotal[root];
    if (strcmp(name, listNode->totalName) == 0) {
        isPrint = TRUE;
    }
    int pos = listNode->subPos;
    if (pos == -1) return isPrint;
    int res = isPrint;
    if (isPrint) {
        if (l_param) {
            printf("%s", listNode->totalName);
            printf(" %d %d", listNode->dirNum, listNode->fileNum);
            printf(":\n");

            while (listTotal[pos].type != -1) {
                if (listTotal[pos].type == FILE_TYPE && listTotal[pos].startPos != 0) {
                    printf("%s %d\n", listTotal[pos].name, listTotal[pos].size);
                } else if (listTotal[pos].type == DIR_TYPE) {
                    printf("%s  %d %d\n", listTotal[pos].name, listTotal[pos].dirNum, listTotal[pos].fileNum);
                } else {
                    printf("%s\n", listTotal[pos].name);
                }
                pos++;
            }
        } else {
            printf("%s", listNode->totalName);
            printf(":\n");
            while (listTotal[pos].type != -1) {
                printf("%s  ", listTotal[pos].name);
                pos++;
            }
        }
        printf("\n");
    }
    pos = listNode->subPos;
    while (pos > 0 && listTotal[pos].type != -1) {
        if(ls_print(name, isPrint, pos, l_param)) res = TRUE;
        pos++;
    }
    return res;
}

void instruction_ls(const char * inputString, int l_param) {
    int pos = 0;
    char name[60];
    memset(name, 0, 60);
    int cnt = 0;

    while(inputString[pos] == ' '){
        pos++;
    }
    if(l_param){
        while(TRUE){
            if(inputString[pos] == '-'){
                while (inputString[pos] != ' ' && inputString[pos] != '\n'){
                    pos++;
                }
            } else{
                break;
            }
            while(inputString[pos] == ' '){
                pos++;
            }
        }
    }
    while (inputString[pos] != ' ' && inputString[pos] != '\n') {
        name[cnt++] = inputString[pos++];
    }
    name[cnt++] = '/';
    int twoFiles = FALSE;
    while(inputString[pos] != '\n'){
        if(inputString[pos] != ' '){
            if(l_param){
                if(inputString[pos] == '-'){
                    while (inputString[pos] != ' ' && inputString[pos] != '\n'){
                        pos++;
                    }
                    pos--;
                }else{
                    twoFiles = TRUE;
                    break;
                }
            }else{
                twoFiles = TRUE;
                break;
            }
        }
        pos++;
    }

    if(twoFiles){
        printf("Please input one file name.\n");
        return;
    }
    int res = ls_print(name, FALSE, 0, l_param);
    if(res == 0){
        printf("Can,t find directory, please input right directory name.\n");
    }
}

int cat_print(const char * name, int isPrint, int root) {
    struct ListNode *listNode = &listTotal[root];
    if (strcmp(name, listNode->totalName) == 0) {
        isPrint = TRUE;
    }
    int res = isPrint;
    if (isPrint) {
        char output[2048];
        memset(output, 0, 2048);
        fseek(fat12, listNode->startPos, 0);
        fread(output, 1, listNode->size, fat12);
        printf("%s", output);
    } else {
        int pos = listNode->subPos;
        while (pos > 0 && listTotal[pos].type != -1) {
            if(cat_print(name, isPrint, pos)) res = TRUE;
            pos++;
        }
    }
    return res;
}

void instruction_cat(const char * inputString) {
    int pos = 0;
    char name[60];
    memset(name, 0, 60);
    int cnt = 0;

    while (inputString[pos] != '\n') {
        pos++;
    }
    pos--;
    while (inputString[pos] == ' ') {
        pos--;
    }
    while (inputString[pos] != ' ') {
        pos--;
    }

    int twoFiles = FALSE;
    int tempPos = pos;
    while(tempPos > 0){
        if(inputString[tempPos] != ' '){
            twoFiles = TRUE;
            break;
        }
        tempPos--;
    }
    if(twoFiles){
        printf("Please input one file name.\n");
        return;
    }

    pos++;
    if(inputString[pos] != '/'){
        name[cnt++] = '/';
    }
    while (inputString[pos] != ' ' && inputString[pos] != '\n') {
        name[cnt++] = inputString[pos++];
    }

    if(strcmp(".TXT", &name[cnt - 4]) != 0){
        printf("Please input file name.\n");
        return;
    }
//    printf("%s", name);
    name[cnt] = '/';
    int res = cat_print(name, FALSE, 0);
    if(res == 0){
        printf("Can't find file, please input right file name.\n");
    }
}

int readInput() { // 0 exit, 1 ls, 2 ls -l, 3 cat, 4 incorrect param

    // cat 报错非.txt， ls 文件名（非文件夹）， 不支持参数报错

    printf("> ");
    fgets(input, 500, stdin);
    int type = -1; // 0 exit, 1 ls, 2 ls -l, 3 cat
    int inputPos = 0;
    while(input[inputPos] == ' '){
        inputPos++;
    }

    if (input[inputPos] == 'l' && input[inputPos + 1] == 's' &&
    (input[inputPos + 2] == ' ' || input[inputPos + 2] == '\n')) {
        type = LIST_FILE;
        int pos = 0;
        while (input[pos] != '\0') {
            if (input[pos] == '-') {
                if(input[pos + 1] == 'l'){
                    if((input[pos + 2] == ' ' || input[pos + 2] == '\n') ||
                    (input[pos + 2] == 'l' && input[pos + 3] == ' ') ||
                    (input[pos + 2] == 'l' && input[pos + 3] == '\n')){
                        type = LIST_FILE_L;
                    } else{
                        type = MIS_PARAM;
                    }
                }else{
                    type = MIS_PARAM;
                }
                break;
            }
            pos++;
        }
    } else if (input[inputPos] == 'c' && input[inputPos + 1] == 'a' && input[inputPos + 2] == 't'
    && (input[inputPos + 3] == ' ' || input[inputPos + 3] == '\n')) {
        type = CAT;
    } else if (input[inputPos] == 'e' && input[inputPos + 1] == 'x' && input[inputPos + 2] == 'i' && input[inputPos + 3] == 't'
    && (input[inputPos + 4] == ' ' || input[inputPos + 4] == '\n')) {
        type = EXIT;
    }

    switch (type) {
        case 0:
            return 0;
        case 1:
            instruction_ls(input + inputPos + 2, FALSE);
            break;
        case 2:
            instruction_ls(input + inputPos + 2, TRUE);
            break;
        case 3:
            instruction_cat(input + inputPos + 3);
            break;
        case 4:
            printf("Please input right parameter.\n");
            break;
        default:
            printf("Please input right instruction.\n");
            break;
    }
    memset(input, 0, 512);
    return 1;
}

int main() {
    loadFile();
    while (1) {
        if (!readInput()) break;
    }
    return 0;
}
