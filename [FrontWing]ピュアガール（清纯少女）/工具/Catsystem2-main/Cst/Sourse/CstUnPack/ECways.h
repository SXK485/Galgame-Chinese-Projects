#ifndef HS_PACK
#define HS_PACK
#include<Windows.h>

//静态库ECways中三个函数的源码
//compressBound和compress2,compress,uncompress均为zlib开源库内的函数

/*
REdata::~REdata()
{
 if(data)
 delete data;
}
*/

//zlib压缩
//Data:需要编码的数据
//Size:数据数组长度（不计算具体大小）
/*
REdata* ECways::dCompress_Zlib(BYTE* Data, DWORD Size, int Level)//请包含zlib.h并导入静态库，函数仅64位下可运行。
{
	if (Level < -1 || Level>9)
		return nullptr;
	if (Data == nullptr)
		return nullptr;
	REdata* rStruct = nullptr;
	DWORD MaxSize = compressBound(Size);
	BYTE* rZlib = new BYTE[MaxSize];

	if ((Level==-1? compress(rZlib, &MaxSize, Data, Size):compress2(rZlib, &MaxSize, Data, Size,Level)) != Z_OK)
	{
		delete[]rZlib;
		return nullptr;
	}
	rStruct = new REdata;
	rStruct->size = MaxSize;
	rStruct->data = new BYTE[MaxSize];
	memcpy(rStruct->data, rZlib, MaxSize);
	delete[]rZlib;
	return rStruct;
}
*/

//zlib解压
//Data:需要解码的数据
//Size:数据数组长度（不计算具体大小）
//MaxSize:数据最大解压大小（需要用户给出）
//请包含zlib.h并导入静态库，函数仅64位下可运行。
/*
REdata* ECways::dUncompress_Zlib(BYTE* Data, DWORD Size, DWORD MaxSize)
{
	if (Data == nullptr)
		return nullptr;

	REdata* rStruct = nullptr;
	BYTE* rZlib = new BYTE[MaxSize];
	if (uncompress(rZlib, &MaxSize, Data, Size) != Z_OK)
	{
		delete[]rZlib;
		return nullptr;
	}
	rStruct = new REdata;
	rStruct->size = MaxSize;
	rStruct->data = new BYTE[MaxSize];
	memcpy(rStruct->data, rZlib, MaxSize);
	delete[]rZlib;
	return rStruct;
}
*/


//加密或解密数据返回(数据为字节类型)
//Data:数据
//Size:数据数组长度（不计算具体大小）
class REdata
{
public:
	BYTE* data;
	DWORD size;
	~REdata();
};


// dCompress数据压缩
// dUncompress数据解压
class ECways
{
public:

	//zlib压缩
	//Data:需要编码的数据
	//Size:数据数组长度（不计算具体大小）
	//Level压缩级别：可填入-1到9：    
	//-1 默认压缩
	//0 不压缩
	//1--9压缩比逐级提高但速度下降
	static REdata* dCompress_Zlib(BYTE* Data, DWORD Size, int Level);

	//zlib解压
	//Data:需要解码的数据
	//Size:数据数组长度（不计算具体大小）
	//MaxSize:数据最大解压大小（需要用户给出）
	//请包含zlib.h并导入静态库，函数仅64位下可运行。
	static REdata* dUncompress_Zlib(BYTE* Data, DWORD Size, DWORD MaxSize);
};

#endif HS_PACK