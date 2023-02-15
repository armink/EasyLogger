# C结构体与 JSON 快速互转库

---

## struct2json

[struct2json](https://github.com/armink/struct2json) 是一个开源的C结构体与 JSON 快速互转库，它可以快速实现 **结构体对象** 与 **JSON 对象** 之间序列化及反序列化要求。快速、简洁的 API 设计，大大降低直接使用 JSON 解析库来实现此类功能的代码复杂度。

## 起源

把面向对象设计应用到C语言中，是当下很流行的设计思想。由于C语言中没有类，所以一般使用结构体 `struct` 充当类，那么结构体变量就是对象。有了对象之后，很多时候需要考虑对象的序列化及反序列化问题。C语言不像很多高级语言拥有反射等机制，使得对象序列化及反序列化被原生的支持。

对于C语言来说，序列化为 JSON 字符串是个不错的选择，所以就得使用 [cJSON](https://github.com/kbranigan/cJSON) 这类 JSON 解析库，但是使用后的代码冗余且逻辑性差，所以萌生对cJSON库进行二次封装，实现一个 struct 与 JSON 之间快速互转的库。 struct2json 就诞生于此。下面是 struct2json 主要使用场景：

- **持久化** ：结构体对象序列化为 JSON 对象后，可直接保存至文件、Flash，实现对结构体对象的掉电存储；
- **通信** ：高级语言对JSON支持的很友好，例如： Javascript、Groovy 就对 JSON 具有原生的支持，所以 JSON 也可作为C语言与其他语言软件之间的通信协议格式及对象传递格式；
- **可视化** ：序列化为 JSON 后的对象，可以更加直观的展示到控制台或者 UI 上，可用于产品调试、产品二次开发等场景；

## 如何使用

### 声明结构体

如下声明了两个结构体，结构体 `Hometown` 是结构体 `Student` 的子结构体

```C
/* 籍贯 */
typedef struct {
    char name[16];
} Hometown;

/* 学生 */
typedef struct {
    uint8_t id;
    uint8_t score[8];
    char name[10];
    double weight;
    Hometown hometown;
} Student;
```

### 将结构体对象序列化为 JSON 对象

|使用前（[源文件](https://github.com/armink/struct2json/blob/master/docs/zh/assets/not_use_struct2json.c)）|使用后（[源文件](https://github.com/armink/struct2json/blob/master/docs/zh/assets/used_struct2json.c)）|
|:-----:|:-----:|
|![结构体转JSON-使用前](https://git.oschina.net/Armink/struct2json/raw/master/docs/zh/images/not_use_struct2json.png)| ![结构体转JSON-使用后](https://git.oschina.net/Armink/struct2json/raw/master/docs/zh/images/used_struct2json.png)|

### 将 JSON 对象反序列化为结构体对象

|使用前（[源文件](https://github.com/armink/struct2json/blob/master/docs/zh/assets/not_use_struct2json_for_json.c)）|使用后（[源文件](https://github.com/armink/struct2json/blob/master/docs/zh/assets/used_struct2json_for_json.c)）|
|:-----:|:-----:|
|![JSON转结构体-使用前](https://git.oschina.net/Armink/struct2json/raw/master/docs/zh/images/not_use_struct2json_for_json.png)| ![JSON转结构体-使用后](https://git.oschina.net/Armink/struct2json/raw/master/docs/zh/images/used_struct2json_for_json.png)|

欢迎大家 **fork and pull request**([Github](https://github.com/armink/struct2json)|[OSChina](http://git.oschina.net/armink/struct2json)|[Coding](https://coding.net/u/armink/p/struct2json/git)) 。如果觉得这个开源项目很赞，可以点击[项目主页](https://github.com/armink/struct2json) 右上角的**Star**，同时把它推荐给更多有需要的朋友。

## 文档

具体内容参考[`\docs\zh\`](https://github.com/armink/struct2json/tree/master/docs/zh)下的文件。务必保证在 **阅读文档** 后再使用。

## 许可

MIT Copyright (c) armink.ztl@gmail.com
