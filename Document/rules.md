wke 项目规范
===========================

目的
----

wke 对于研究 WebKit 是一个很好的项目，同时也可以作为网页的渲染引擎用于实际工程。目前原作者已经停止维护此项目，我个人 fork 出来自行维护，但希望有更多的人来参与研究、维护。为了此项目的长期良好地发展，有必要编写一份规范文档，以后的所有代码、文档、提交都应该按照此规范进行。

目录
----

- [文档规范](#document-rules)
- [代码规范](#coding-rules)
    - [命名规范](#code-naming)
        - [总体规范](#code-naming-summary)
        - [类型（类、结构体等）](#code-naming-type)
        - [变量（变量、函数等）](#code-naming-variable)
        - [常量（常量、宏等）](#code-naming-constant)
    - [排版规范](#code-formating)
        - [缩进](#code-formating-indent)
        - [换行](#code-formating-newline)
        - [空格](#code-formating-whitespace)

----

### <a name="document-rules"></a>文档规范

中文文档中的描述部分涉及的标点，应该使用中文全角。

### <a name="coding-rules"></a>代码规范：

#### <a name="code-naming"></a>命名规范

##### <a name="code-naming-summary"></a>总体规范

代码命名中应该尽量包含被命名实体的的功能性说明，不必包含它的类型信息，因为前者更能帮助阅读者理解整体代码。禁止使用匈牙利命名法。举例：
正确的命名：

```
class Student;
void study(Student* student);
void doHomeWork(Student* student);
```
错误的命名：

```
class ClassStudent; //不需要包含类型信息。
void functionStudy(ClassStudent* pClassStudent); //不需要包含类型信息。。
void functionDoWork(ClassStudent* pClassStudent); //不需要包含类型信息。
```

代码命名中涉及的单词应该避免缩写，除非名字包含的单词太多、某个单词特别长，或者该缩写是行业或团队通用的、常用的缩写，举例：
正确的命名：

```
class Student;
void study(Student* student);
void doHomeWork(Student* student);
double cpuUseage();
```

错误的命名：

```
class Stu;    //不必要的缩写。
void study(Stu* s);	//参数命名太简单，没有任何说明意义。
void work(Stu* student); //函数命名意义不明确。
double centralProcessingUnitUsage(); //应该使用公用的缩写。
```


##### <a name="code-naming-type"></a>类型（类、结构体、枚举、函数指针类型等）命名

类型（类类型、结构体类型、枚举类型、函数指针类型等）的命名，使用大驼峰命名法（little camel-case），即：命名中的所有单词（或单词缩写，后面统称单词）第一个字母大写，单词之间紧靠排列（不用间隔符，如下划线）例如：

```
class SomeClass;
class SomeLongNameClass;
enum SomeEnum;
typedef void (*SomeFuncPointer)();
```

##### <a name="code-naming-variable"></a>变量、函数命名

变量、函数的命名，使用小驼峰命名法（little camel-case），即：命名中的第一个单词全部小写，后续单词第一字母大写开始单词中的其余字母小写，单词之间紧靠排列（不用间隔符，如下划线）例如：


```
void someFunction();
void someLongNameFunction();
int someVariable;
int someLongNameVariable;
```

根据函数的作用，函数命名应该进一步规范：

- 若函数（包括成员函数和非成员函数）是执行操作，则直接以动作、功能的单词命名，如`doHomeWork`、`reportGrade`。
- 若函数（包括成员函数和非成员函数）的功能是设置数据，则以`set`开头，并紧根要设置数据的名称，如`setNickName`。
- 若成员函数的功能是获取成员数据，则直接以要获取的实体名称命名，如`Student::nickName`、`Student::grade`。
- 若非成员函数的功能是获取数据，则使用`get`开头，并紧跟要获取数据的名称，如`getNickName`。
- 若非成员函数的功能是获取 是/否 数据，则使用`is`开始，并紧跟要获取数据的名称，如`isEnglishPassed`


##### <a name="code-naming-constant"></a>宏、常量、枚等命名

宏、常量 、枚举的命名，使用全大写单词加下划线相连，如：

```
#define SOME_CONST_VALUE 123
const int SOME_CONST_VALUE = 123;
enum SomeEnum
{
	SOME_VALUE_1,
	SOME_VALUE_2
};
```

#### <a name="code-formating"></a>排版规范

##### <a name="code-formating-indent"></a>缩进规范

缩进应该使用4个空格，以避免使用 TAB 时在不同编辑器中缩进混乱。

##### <a name="code-formating-newline"></a>换行规范

起始大括号应该换行再写。

##### <a name="code-formating-whitespace"></a>空格规范

- `if`、`while`、`for`之类的关键字之后空格再跟括号，如`if (...)`、`for (...)`、`while (...)`。
- 标点之后加一个空格分隔实体，如`a, b, c`、`for (...; ...; ...)`、`a || b && c`。
- 括号内第一个字符之前，最后一个字符之后不需要空格，如`(a)`，`(a, b, c)`。


**待补充**