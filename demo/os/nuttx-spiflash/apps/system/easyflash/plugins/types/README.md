# EasyFlash Types 插件（plugin）

---

## 1、介绍

目前 EasyFlash 会将环境变量以字符串形式存储于 Flash 中，在这种模式下，对于非字符串类型的环境变量在使用时，就必须得增加额外的字符串转换代码。设计 Types 插件就是为了方便用户在使用 EasyFlash 时，以更加简单的方式去操作各种类型的环境变量。

主要支持的类型包括：C 的 **基本类型** 、 **数组类型** 以及 **结构体类型** 。对于结构体类型， Types 插件内部采用 [struct2json](https://github.com/armink/struct2json) 库进行转换，所以项目中需要依赖 [struct2json](https://github.com/armink/struct2json) 库。

## 2、使用

### 2.1 源码导入

导入之前需要确认自己的项目中已包含 EasyFlash 核心源码，即包括 "\easyflash\inc"、"\easyflash\port" 及 "\easyflash\src" 下相关文件（导入方法可以参考这个移植文档：[点击打开](https://github.com/armink/EasyFlash/blob/master/docs/zh/port.md)）。再将 Types 插件源码导入到项目中，最好连同 "plugins\types" 文件夹一起拷贝至项目中已有的 easyflash 文件夹下。然后需要添加 `easyflash\plugins\types\struct2json\inc` 及 `easyflash\plugins\types` 两个文件夹路径到项目头文件路径中即可。

### 2.2 初始化

```C
void ef_types_init(S2jHook *hook) 
```

这个方法为 Types 插件的初始化方法，主要初始化 struct2json 库所需的内存管理方法。默认使用的 malloc 及 free 作为内存管理方法，如果使用默认内存管理方式，则无需初始化。例如如果使用 RT-Thread 操作系统自带的内存管理方法，则可以参考下面的初始化代码：

```C
S2jHook s2jHook = {
        .free_fn = rt_free,
        .malloc_fn = (void *(*)(size_t))rt_malloc,
};
ef_types_init(&s2jHook);
```

### 2.3 操作环境变量

#### 2.3.1 基本类型

对于基本类型的环境变量操作方法与 EasyFlash 原有的 API 一致，只是修改了入参及出参的类型，所有可用的 API 如下：

```C
bool ef_get_bool(const char *key);
char ef_get_char(const char *key);
short ef_get_short(const char *key);
int ef_get_int(const char *key);
long ef_get_long(const char *key);
float ef_get_float(const char *key);
double ef_get_double(const char *key);
EfErrCode ef_set_bool(const char *key, bool value);
EfErrCode ef_set_char(const char *key, char value);
EfErrCode ef_set_short(const char *key, short value);
EfErrCode ef_set_int(const char *key, int value);
EfErrCode ef_set_long(const char *key, long value);
EfErrCode ef_set_float(const char *key, float value);
EfErrCode ef_set_double(const char *key, double value);
```

#### 2.3.2 数组类型

与基本类型的操作方法大体一致，不同点在于：获取到的环境变量需通过指针类型的入参进行返回。所有可用的 API 如下：

```C
void ef_get_bool_array(const char *key, bool *value);
void ef_get_char_array(const char *key, char *value);
void ef_get_short_array(const char *key, short *value);
void ef_get_int_array(const char *key, int *value);
void ef_get_long_array(const char *key, long *value);
void ef_get_float_array(const char *key, float *value);
void ef_get_double_array(const char *key, double *value);
void ef_get_string_array(const char *key, char **value);
EfErrCode ef_set_bool_array(const char *key, bool *value, size_t len);
EfErrCode ef_set_char_array(const char *key, char *value, size_t len);
EfErrCode ef_set_short_array(const char *key, short *value, size_t len);
EfErrCode ef_set_int_array(const char *key, int *value, size_t len);
EfErrCode ef_set_long_array(const char *key, long *value, size_t len);
EfErrCode ef_set_float_array(const char *key, float *value, size_t len);
EfErrCode ef_set_double_array(const char *key, double *value, size_t len);
EfErrCode ef_set_string_array(const char *key, char **value, size_t len);
```
#### 2.3.3 结构体类型

对于结构体类型，这里首先需要使用 struct2json 库来编写与该结构体对应的 JSON 互转方法，再将编写好的互转方法作为入参进行使用。结构体类型环境变量操作的 API 如下：

```C
void *ef_get_struct(const char *key, ef_types_get_cb get_cb);
EfErrCode ef_set_struct(const char *key, void *value, ef_types_set_cb set_cb);
```

具体使用流程及结构体与 JSON 之间的互转方法可以参考下面的 Demo：

```C
/* 定义结构体 */
typedef struct {
    char name[16];
} Hometown;
typedef struct {
    uint8_t id;
    double weight;
    uint8_t score[8];
    char name[16];
    Hometown hometown;
} Student;

/* 定义结构体转 JSON 的方法 */
static cJSON *stu_set_cb(void* struct_obj) {
    Student *struct_student = (Student *)struct_obj;
    /* 创建 Student JSON 对象 */
    s2j_create_json_obj(json_student);
    /* 序列化数据到 Student JSON 对象 */
    s2j_json_set_basic_element(json_student, struct_student, int, id);
    s2j_json_set_basic_element(json_student, struct_student, double, weight);
    s2j_json_set_array_element(json_student, struct_student, int, score, 8);
    s2j_json_set_basic_element(json_student, struct_student, string, name);
    /* 序列化数据到 Student.Hometown JSON 对象 */
    s2j_json_set_struct_element(json_hometown, json_student, struct_hometown, struct_student, Hometown, hometown);
    s2j_json_set_basic_element(json_hometown, struct_hometown, string, name);
    return json_student;
}

/* 定义 JSON 转结构体的方法 */
static void *stu_get_cb(cJSON* json_obj) {
    /* 创建 Student 结构体对象（提示： s2j_ 开头的方法是 struct2json 库提供的） */
    s2j_create_struct_obj(struct_student, Student);
    /* 反序列化数据到 Student 结构体对象 */
    s2j_struct_get_basic_element(struct_student, json_obj, int, id);
    s2j_struct_get_array_element(struct_student, json_obj, int, score);
    s2j_struct_get_basic_element(struct_student, json_obj, string, name);
    s2j_struct_get_basic_element(struct_student, json_obj, double, weight);
    /* 反序列化数据到 Student.Hometown 结构体对象 */
    s2j_struct_get_struct_element(struct_hometown, struct_student, json_hometown, json_obj, Hometown, hometown);
    s2j_struct_get_basic_element(struct_hometown, json_hometown, string, name);
    return struct_student;
}

/* 设置结构体类型环境变量 */
Student orignal_student = {
        .id = 24,
        .weight = 71.2,
        .score = {1, 2, 3, 4, 5, 6, 7, 8},
        .name = "张三",
        .hometown.name = "北京",
};
ef_set_struct("张三学生", &orignal_student, stu_set_cb);

/* 获取结构体类型环境变量 */
Student *student;
ef_get_struct("张三学生", student, stu_get_cb);

/* 打印获取到的结构体内容 */
printf("姓名：%s 籍贯：%s \n", student->name, student->hometown.name);

/* 释放获取结构体类型环境变量过程中开辟的动态内存 */
s2jHook.free_fn(student);
```