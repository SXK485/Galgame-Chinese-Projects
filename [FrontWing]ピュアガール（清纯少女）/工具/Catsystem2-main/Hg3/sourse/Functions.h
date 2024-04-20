#pragma once
#include<windows.h>
#include"zlib.h"
#pragma comment(lib,"zlib.lib")
#define TRANS_BYTE 1
#define TRANS_BIT 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////zlib压缩
//zlib压缩
//Data:需要编码的数据
//Size:数据数组长度（不计算具体大小）
 BYTE * ZlibCompress(BYTE* Data, DWORD& Size, int Level)//请包含zlib.h并导入静态库，函数仅64位下可运行。
{
	if (Level < -1 || Level>9)
		return nullptr;
	if (Data == nullptr)
		return nullptr;
	DWORD MaxSize = compressBound(Size);
	BYTE* rZlib = new BYTE[MaxSize];

	if ((Level == -1 ? compress(rZlib, &MaxSize, Data, Size) : compress2(rZlib, &MaxSize, Data, Size, Level)) != Z_OK)
	{
		delete[]rZlib;
		return nullptr;
	}
	BYTE* data = new BYTE[MaxSize];
	memcpy(data, rZlib, MaxSize);
	delete[]rZlib;
	Size = MaxSize;
	return data;
}

//zlib解压
//Data:需要解码的数据
//Size:数据数组长度（不计算具体大小）
//MaxSize:数据最大解压大小（需要用户给出）
//请包含zlib.h并导入静态库，函数仅64位下可运行。
 BYTE* ZlibUncompress(BYTE* Data, DWORD& Size, DWORD MaxSize)
{
	if (Data == nullptr)
		return nullptr;

	BYTE* rZlib = new BYTE[MaxSize];
	if (uncompress(rZlib, &MaxSize, Data, Size) != Z_OK)
	{
		delete[]rZlib;
		return nullptr;
	}
	BYTE* data = new BYTE[MaxSize];
	memcpy(data, rZlib, MaxSize);
	delete[]rZlib;
	Size = MaxSize;
	return data;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////字节数组转比特位数组
BYTE* Data_to_Bit(BYTE* data, DWORD& size)
{
	BYTE* bData = new BYTE[size * 8];
	for (int i = 0; i < size; i++)
	{
		for (int k = 0; k < 8; k++)
		{
			bData[i * 8 + k] = ((data[i] >> (7 - k))) % 2;
		}
	}
	size = size * 8;
	return bData;
}

BYTE* Bit_to_Data(BYTE* bit, DWORD& size)//比特数组转字节数组(没有单位元素1比特的数组，所以用字节数组代替)
{
	DWORD presize = size;
	size % 8 == 0 ? size = size / 8 : size=size/8+1;
	BYTE* result = new BYTE[size]();
	for (int i = 0; i < size; i++)
	{
		for (int k = 7; k >= 0; k--)
		{
			if(i * 8 + 7 - k<presize)
			result[i] += (bit[i * 8 +7-k] << k);
		}
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////EliasGammaCode正整数编码
DWORD* uEliasGammaCode(BYTE* data, DWORD& size)//EliasGammaCode解码，size更新为返回数组的大小解码。无符号整型数组
{
	DWORD count = 0;
	DWORD* table = new DWORD[99999999]{};
	DWORD ptr = 0;
	for (int i = 0; i < size; i++)
	{
		if (data[i] == 1 && i + count < size)
		{
			table[ptr] = 1;
			for (int k = 1; k <= count; k++)
			{
				table[ptr] = table[ptr] * 2 + data[i + k];
			}
			i = i + count;
			ptr = ptr + 1;
			count = 0;
			continue;
		}
		count++;
	}
	size = ptr;
	DWORD* result = new DWORD[size];
	memcpy(result, table, size * 4);
	delete[]table; table = nullptr;
	return result;
}
/////////////////////////////////////////////////////////////////////用EliasGammaCode编码数字数组，并修改size大小
BYTE* cEliasGammaCode(DWORD* data, DWORD& size,DWORD TRANS_KIND)//TRANS_KIND的不同影响是否最后将编码的比特位重新转化为字节数组节省空间。
{
	BYTE* table = new BYTE[size *32]();
	DWORD ptr = 0;
	for (int k = 0; k < size; k++)
	{
		if (data[k] == 0)
			return nullptr;
		for (int i = 31; i >= 0; i--)
		{
			if ((data[k] >> i) == 1)
			{
				ptr = ptr + i;
				for (int w = i; w >= 0; w--, ptr++)
				{
					table[ptr] = (data[k] >> w) % 2;
				}
			}
		}
	}
	if (TRANS_KIND)
	{
		if (ptr % 8 != 0)
			ptr = ptr + 8 - (ptr % 8);
		size = ptr / 8;
		BYTE* result = new BYTE[ptr / 8]();
		for (int i = 0; i < ptr / 8; i++)
		{
			for (int k = 0; k < 8; k++)
			{
				result[i] = result[i] + (table[i * 8 + k] << (7 - k));
			}
		}
		delete[]table;
		return result;
	}
	BYTE* rebit = new BYTE[ptr](); size = ptr;
	memcpy(rebit, table, ptr);
	delete[]table;
	return rebit;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////Reverse
void  CompleteReversee(BYTE* data, DWORD size)//将字节数组的比特位完全逆转，如10110100->00101101
{
	for (int i = 0; i < size; i++)
	{
		BYTE sample = 0;
		for (int k = 0; k < 8; k++)
		sample = sample + (((data[i] >> k) % 2) << (7 - k));
		data[i] = sample;
	}
	return;
}

void BitReverse(BYTE* data, DWORD size)//将元素按每比特逆转，每字节中的元素顺序不变
{
	for (int i = 0; i < size; i++)
		data[i] = ((data[i] % 4) << 6) + (((data[i] >> 2) % 4) << 4) + (((data[i] >> 4) % 4) << 2) + ((data[i] >> 6) % 4);
}

WCHAR* Utf8_To_Wide(const char* str)//utf8数据转宽字节
{
	WCHAR* wide;
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);//获取str所需要宽字节空间的大小
	wide = new WCHAR[i + 1];//申请空间
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wide, i);//将str中的字符装入宽字节数组wide
	return wide;//返回
}

WCHAR* Char_To_Wide(const char* str)//char转宽字节
{
	WCHAR* wide;
	int i = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);//获取str所需要宽字节空间的大小
	wide = new WCHAR[i + 1];//申请空间
	MultiByteToWideChar(CP_ACP, 0, str, -1, wide, i);//将str中的字符装入宽字节数组wide
	return wide;//返回
}

LPSTR Wide_To_Char(WCHAR* str)//宽字节转char
{
	char* wide;
	int i = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//获取str所需要字节空间的大小
	wide = new char[i + 1];//申请空间
	WideCharToMultiByte(CP_ACP, 0, str, -1, wide, i, NULL, NULL);//将str中的字符装入字节数组wide
	return wide;//返回
}

//去掉扩展名，即.xxx
void PathRemoveExtensionW(LPWSTR path)//去掉路径上中的文件扩展名
{
	for (int i = wcslen(path) - 1; i >= 0; i--)
	{
		if (path[i] == '.')
		{
			path[i] = '\0';
			break;
		}
		path[i] = '\0';
	}
}
//去掉路径上的指定字符
void pathremove(LPWSTR& str, LPCWSTR replace, LPCWSTR to)
{
	int length = lstrlenW(replace);
	WCHAR* result = new WCHAR[lstrlenW(str) + 1]();
	for (int i = 0; i < lstrlenW(str); i++)
	{
		if (str[i] == replace[0])
		{
			int count = 1;
			for (int k = 0; k < length; k++)
			{
				if (str[i + k] != replace[k])
					count = 0;	break;
			}
			if (count)
			{
				lstrcatW(result, to);
				i += length - 1;
				continue;
			}
		}
		else
		{
			result[lstrlenW(result)] = str[i];
		}
	}
	WCHAR* TrueReturn = new WCHAR[lstrlenW(result) + 1];
	lstrcpynW(TrueReturn, result, lstrlenW(result) + 1);
	delete[]result;
	str = TrueReturn;
}