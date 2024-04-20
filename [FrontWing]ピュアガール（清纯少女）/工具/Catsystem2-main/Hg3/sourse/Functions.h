#pragma once
#include<windows.h>
#include"zlib.h"
#pragma comment(lib,"zlib.lib")
#define TRANS_BYTE 1
#define TRANS_BIT 0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////zlibѹ��
//zlibѹ��
//Data:��Ҫ���������
//Size:�������鳤�ȣ�����������С��
 BYTE * ZlibCompress(BYTE* Data, DWORD& Size, int Level)//�����zlib.h�����뾲̬�⣬������64λ�¿����С�
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

//zlib��ѹ
//Data:��Ҫ���������
//Size:�������鳤�ȣ�����������С��
//MaxSize:��������ѹ��С����Ҫ�û�������
//�����zlib.h�����뾲̬�⣬������64λ�¿����С�
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////�ֽ�����ת����λ����
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

BYTE* Bit_to_Data(BYTE* bit, DWORD& size)//��������ת�ֽ�����(û�е�λԪ��1���ص����飬�������ֽ��������)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////EliasGammaCode����������
DWORD* uEliasGammaCode(BYTE* data, DWORD& size)//EliasGammaCode���룬size����Ϊ��������Ĵ�С���롣�޷�����������
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
/////////////////////////////////////////////////////////////////////��EliasGammaCode�����������飬���޸�size��С
BYTE* cEliasGammaCode(DWORD* data, DWORD& size,DWORD TRANS_KIND)//TRANS_KIND�Ĳ�ͬӰ���Ƿ���󽫱���ı���λ����ת��Ϊ�ֽ������ʡ�ռ䡣
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
void  CompleteReversee(BYTE* data, DWORD size)//���ֽ�����ı���λ��ȫ��ת����10110100->00101101
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

void BitReverse(BYTE* data, DWORD size)//��Ԫ�ذ�ÿ������ת��ÿ�ֽ��е�Ԫ��˳�򲻱�
{
	for (int i = 0; i < size; i++)
		data[i] = ((data[i] % 4) << 6) + (((data[i] >> 2) % 4) << 4) + (((data[i] >> 4) % 4) << 2) + ((data[i] >> 6) % 4);
}

WCHAR* Utf8_To_Wide(const char* str)//utf8����ת���ֽ�
{
	WCHAR* wide;
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);//��ȡstr����Ҫ���ֽڿռ�Ĵ�С
	wide = new WCHAR[i + 1];//����ռ�
	MultiByteToWideChar(CP_UTF8, 0, str, -1, wide, i);//��str�е��ַ�װ����ֽ�����wide
	return wide;//����
}

WCHAR* Char_To_Wide(const char* str)//charת���ֽ�
{
	WCHAR* wide;
	int i = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);//��ȡstr����Ҫ���ֽڿռ�Ĵ�С
	wide = new WCHAR[i + 1];//����ռ�
	MultiByteToWideChar(CP_ACP, 0, str, -1, wide, i);//��str�е��ַ�װ����ֽ�����wide
	return wide;//����
}

LPSTR Wide_To_Char(WCHAR* str)//���ֽ�תchar
{
	char* wide;
	int i = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//��ȡstr����Ҫ�ֽڿռ�Ĵ�С
	wide = new char[i + 1];//����ռ�
	WideCharToMultiByte(CP_ACP, 0, str, -1, wide, i, NULL, NULL);//��str�е��ַ�װ���ֽ�����wide
	return wide;//����
}

//ȥ����չ������.xxx
void PathRemoveExtensionW(LPWSTR path)//ȥ��·�����е��ļ���չ��
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
//ȥ��·���ϵ�ָ���ַ�
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