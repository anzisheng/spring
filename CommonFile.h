#ifndef COMMONFILE_H
#define COMMONFILE_H
#pragma execution_character_set("utf-8")
enum TreeType{OUTPUT_DEVICE = 0,ELECTRIC_DEVICE,SHOWTEAMS_DEVICE,
              ORBIT_DEVICE,COMP_DEVICE,QUEUE_DEVICE,OUTLINE_DEVICE};

enum drawType{DRAW_LINE,DRAW_RECT,DRAW_TEXT};

enum runType{ALWAYS_ON,DOMINO_RUN,IMAGE_SHOW,DMX512_SHOW,CLOCK_SHOW};

enum DMX512_TYPE{DMX512_ALWAYSON,DMX512_DOMINO,DMX512_TRACK,DMX512_FROMTO,DMX512_RAINBOW};

enum MUSIC_TYPE{NEW_PROCONTROL,NEW_MUSIC};

enum REPEAT_TYPE{SHOW_REPEAK,SHOW_MIRRORIMAGE,SHOW_OPPOSEMIRROR};//重复方式、镜像方式、反向镜像

enum ROW_TYPE{LINEAR_TYPE,CUSTOMDEFINE_TYPE}; //线性、自定义

enum OUTPUT_TYPE{DMX_TYPE,PROGRAMCONTROL_TYPE};  //DMX输出、程序控制输出

//编曲内表演编队和表演动作类型，用于右键增加表演动作时判断是在表演编队里增加还是表演动作里增加
enum MUSIC_ITEM_TYPE{SHOW_TEAM_ITEM,SHOW_ACTION_ITEM};


//////////////////////////////编曲文件操作/////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//电气设备名字
#define HARD_LEN 20			//最保险的长度DO_XXX:XXXX+4个回车=15个字节

/////////////////////////////////////////////////////////////////////////////
//预设信息

#define INIT_LEN		34		//定义预设置信息的空间大小
#define INIT_IP_HEAD		0	//定义IP最后一位的首地址	IP:XXX+2=8
#define INIT_MODE_HEAD	8	//定义通讯格式的首地址		通信格式：X+2=12
#define INIT_FORMAT_HEAD  20		//定义数据类型的首地址		数据类型：X=10


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//电气设备
#define EQ_LEN		88		//定义电气设备存储的空间大小

#define EQ_NUM_HEAD		0	//定义电气设备编号的首地址
#define EQ_NAME_HEAD	20	//定义电气设备名称的首地址 名称：20个汉字+1个0+2=48
#define EQ_PROPERTY_HEAD  68	//定义电气设备属性的首地址	属性：XX+1=8
#define EQ_OWNER_HEAD	76		//定义电气设备归属的首地址  归属：XXX=8

/////////////////////////////////////////////////////////////////////////////
//编队
#define TEAM_NUM		99	//最多1到99个编队，0是全系统

#define TEAM_NUM_HEAD	0	//定义编队编号的首地址，编号:XX+2＝9,读字符方式
#define TEAM_NUM_END	9	//定义编队编号的尾地址，－2是回车的地址
#define TEAM_NUM_CR		2	//定义编队编号的回车字符数量＝2

#define TEAM_USE_HEAD	9	//定义编队有效性的首地址，有效:X＋2＝8,读字符方式
#define TEAM_USE_END	17	//定义编队有效性的尾地址，－2是回车的地址
#define TEAM_USE_CR		2	//定义编队有效性的回车字符数量＝2

#define TEAM_NAME_HEAD	17	//定义编队名字的首地址，名称:20个汉字＋1个0＋2＝48
#define TEAM_NAME_END	65	//定义编队名字的尾地址，－2是回车的地址,读字符串方式
#define TEAM_NAME_CR	2	//定义编队名字的回车字符数量＝2

#define TEAM_MODE_HEAD	65	//定义编队模式的首地址，模式:X＋2＝8,读字符方式
#define TEAM_MODE_END	73	//定义编队模式的尾地址，－2是回车的地址
#define TEAM_MODE_CR	2	//定义编队模式的回车字符数量＝2

#define TEAM_MIN_HEAD	73	//编队最小编号的首地址，最小:XXX＋2＝10,读字符方式
#define TEAM_MIN_END	83	//编队最小编号的尾地址，－2是回车的地址
#define TEAM_MIN_CR		2	//编队最小编号的回车字符数量＝2

#define TEAM_MAX_HEAD	83	//编队最大编号的首地址，最大:XXX＋2＝10,读字符方式
#define TEAM_MAX_END	93	//编队最大编号的尾地址，－2是回车的地址
#define TEAM_MAX_CR		2	//编队最大编号的回车字符数量＝2

#define TEAM_STY_HEAD	93	//编队方式的首地址，方式:X＋2＝8,读字符方式
#define TEAM_STY_END	101	//编队方式的尾地址，－2是回车的地址
#define TEAM_STY_CR		2	//编队方式的回车字符数量＝2

#define TEAM_GNUM_HEAD	101	//编队分组数的首地址，组数:XXX＋2＝10,读字符方式
#define TEAM_GNUM_END	111	//编队分组数的尾地址，－2是回车的地址
#define TEAM_GNUM_CR	2	//编队分组数的回车字符数量＝2

#define TEAM_CONT_HEAD	111	//编队连续数的首地址，连续:XXX＋2＝10,读字符方式
#define TEAM_CONT_END	121	//编队连续数的尾地址，－2是回车的地址
#define TEAM_CONT_CR	2	//编队连续数的回车字符数量＝2

#define TEAM_INTE_HEAD	121	//编队间隔数的首地址，间隔:XXX＋2＝10,读字符方式
#define TEAM_INTE_END	131	//编队间隔数的尾地址，－2是回车的地址
#define TEAM_INTE_CR	2	//编队间隔数的回车字符数量＝2

#define TEAM_SPED_HEAD	131	//编队倒序的首地址，倒序:X＋2＝8,读字符方式
#define TEAM_SPED_END	139	//编队倒序的尾地址，－2是回车的地址
#define TEAM_SPED_CR	2	//编队倒序的回车字符数量＝2

#define TEAM_SPEW_HEAD	139	//编队往返的首地址，往返:X＋2＝8,读字符方式
#define TEAM_SPEW_END	147	//编队往返的尾地址，－2是回车的地址
#define TEAM_SPEW_CR	2	//编队往返的回车字符数量＝2

#define TEAM_DATA_HEAD	147	//编队数据的首地址，采用级联方式存储数据
//最多350个队列，最前面是个数XXX，然后后是XXX的数据，最后是回车换行
//如果个数为0，后面直接就是回车换行

/////////////////////////////////////////////////////////////////////////////
//轨迹
#define GUIJI_NUM		99	//最多0到99轨迹

#define GUIJI_NUM_HEAD	0	//轨迹编号的首地址，轨迹:XX+2＝9,读字符方式
#define GUIJI_NUM_END	9	//轨迹编号的尾地址，－2是回车的地址
#define GUIJI_NUM_CR	2	//轨迹编号的回车字符数量＝2

#define GUIJI_USE_HEAD	9	//轨迹有效性的首地址，有效:X＋2＝8,读字符方式
#define GUIJI_USE_END	17	//轨迹有效性的尾地址，－2是回车的地址
#define GUIJI_USE_CR	2	//轨迹有效性的回车字符数量＝2

#define GUIJI_NAME_HEAD	17	//轨迹名字的首地址，名称:20个汉字＋1个0＋2＝48
#define GUIJI_NAME_END	65	//轨迹名字的尾地址，－2是回车的地址,读字符串方式
#define GUIJI_NAME_CR	2	//轨迹名字的回车字符数量＝2

#define GUIJI_GNUM_HEAD	65	//轨迹节点数的首地址，组数:XX＋2＝9,读字符方式
#define GUIJI_GNUM_END	74	//轨迹节点数的尾地址，－2是回车的地址
#define GUIJI_GNUM_CR	2	//轨迹节点数的回车字符数量＝2

#define GUIJI_DATA_HEAD	74	//轨迹数据的首地址，采用级联方式存储数据
//最多100个节点，xx:xxxx,xxx
//最前面是节点序号XX,表示00到99,容易阅读
//中间有一个分号
//其次是时间，XXXX的HEX表示的无符号整数，表示毫秒数
//中间有一个逗号
//再次是XXX的高度数据，表示000到100
//最后是回车换行

#define COMP_LEN	300		//最大的编曲曲条条数

// CompLen编曲的类别：0程控，1：带音乐。类别：(5字节)X(1字节)＋2字节回车＝8字节
#define COMP_CLASS_HEAD	0	//类别：
#define COMP_CLASS_END	8	//
#define COMP_CLASS_CR	2	//两个字节的回车，0x0D-0x0A

// CompLen 曲条的条数，INT型，长度：(5字节)XXX(3字节)＋4字节回车＝12字节，CompLen至少有一条
#define COMP_LEN_HEAD	8	//曲条最多999条
#define COMP_LEN_END	20	//
#define COMP_LEN_CR		4	//四个字节的回车，0x0D-0x0A,0x0D-0x0A

// 歌曲的名字和路径，全名：100个汉字＋2回车＝207字节
#define COMP_NAME_HEAD	20
#define COMP_NAME_END	227
#define COMP_NAME_CR	2

// 不包含路径的歌曲的名字，歌曲：100个汉字＋4回车＝209字节
#define COMP_SHORT_HEAD	227
#define COMP_SHORT_END	436
#define COMP_SHORT_CR	4

//数据区
#define COMP_DATA_HEAD	436
#define COMP_DATA_LEN	104	//

// 曲条的序号，序号：xxx＋2回车＝10
#define COMP_SN_HEAD	0
#define COMP_SN_END		10
#define COMP_SN_CR		2

// time[COMP_LEN]	曲条的起始时间 时间：XXXXXX（16进制）＋2字节回车＝13字节
#define COMP_TIME_HEAD	10
#define COMP_TIME_END	23
#define COMP_TIME_CR	2

// kind[COMP_LEN]	输出种类，0:设备，1:编队；种类：X＋2字节回车＝8字节
#define COMP_KIND_HEAD	23
#define COMP_KIND_END	31
#define COMP_KIND_CR	2

// OutNum[COMP_LEN]	输出的设备号或者编队号，编号：XXX+2字节回车＝10字节
#define COMP_OUT_HEAD	31
#define COMP_OUT_END	41
#define COMP_OUT_CR		2

// 动作种类，0:常开，1:常闭，2:轨迹，3:多米诺开关，4:多米诺轨迹
// oper[COMP_LEN] 动作：X＋2字节回车＝8字节
#define COMP_OPER_HEAD	41
#define COMP_OPER_END	49
#define COMP_OPER_CR	2

// GJNum[COMP_LEN] 轨迹序号 轨迹：XX＋2字节回车＝9字节
#define COMP_GJ_HEAD	49
#define COMP_GJ_END		58
#define COMP_GJ_CR		2

// KeepTime[COMP_LEN] 保持时间 保持：XXXX（16进制）＋2字节回车＝11字节
#define COMP_KEEP_HEAD	58
#define COMP_KEEP_END	69
#define COMP_KEEP_CR	2

// DelayTime[COMP_LEN] 延迟时间 延迟：XXXX（16进制）＋2字节回车＝11字节
#define COMP_DELAY_HEAD	69
#define COMP_DELAY_END	80
#define COMP_DELAY_CR	2

// Periodic[COMP_LEN] 周期 周期：XXXX（16进制）＋2字节回车＝11字节
#define COMP_PERI_HEAD	80
#define COMP_PERI_END	91
#define COMP_PERI_CR	2

// Loop[COMP_LEN] 循环次数 循环：XXXX（16进制）＋2字节回车＝11字节
#define COMP_LOOP_HEAD	91
#define COMP_LOOP_END	102
#define COMP_LOOP_CR	2

#define ETH_LEN		600		//发送缓冲区长度

//add by eagle for 全局定时器扫描
#define TIME_RES	5		//定时器的分辨率＝10毫秒  

#define DMX_INF_HEAD	0		//DMX数据文件信息头的起始
#define DMX_INF_END		256		//DMX数据文件信息头的终止
#define DMX_INF_CR		2		//1个回车符号

#define QUEUE_INF_HEAD	0		//播放队列INF部分的起始位置
#define QUEUE_INF_LEN	128		//播放队列INF部分的长度
#define QUEUE_INF_CR	2		//播放队列INF部分的回车字节数，两个字节为一个回车

#define QUEUE_DATA_HEAD	0		//播放队列DMX文件名称部分的起始位置
#define QUEUE_DATA_LEN	512		//播放队列DMX文件名称部分的长度，空间较大，为将来预留
#define QUEUE_DATA_CR	2		//播放队列DMX文件名称部分的回车字节数，两个字节为一个回车

//该工程中每种板子的数量	(每次修改这个地方的数量即可)
#define DO64_N		2		//使用E-DO64的个数
#define FA8_N		0		//使用E-DO64的个数
#define AO8_N		0		//使用E-AO16的个数
#define AX8_N		0		//使用E-AX8的个数
#define LED_N		1		//使用E-LED_CON，每路可以串很多灯，一般一个就够了

#define STX	0x02
#define ETX	0x03

#endif // COMMONFILE_H
