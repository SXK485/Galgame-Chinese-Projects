#ifndef HS_PACK
#define HS_PACK
#include<Windows.h>

//��̬��ECways������������Դ��
//compressBound��compress2,compress,uncompress��Ϊzlib��Դ���ڵĺ���

/*
REdata::~REdata()
{
 if(data)
 delete data;
}
*/

//zlibѹ��
//Data:��Ҫ���������
//Size:�������鳤�ȣ�����������С��
/*
REdata* ECways::dCompress_Zlib(BYTE* Data, DWORD Size, int Level)//�����zlib.h�����뾲̬�⣬������64λ�¿����С�
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

//zlib��ѹ
//Data:��Ҫ���������
//Size:�������鳤�ȣ�����������С��
//MaxSize:��������ѹ��С����Ҫ�û�������
//�����zlib.h�����뾲̬�⣬������64λ�¿����С�
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


//���ܻ�������ݷ���(����Ϊ�ֽ�����)
//Data:����
//Size:�������鳤�ȣ�����������С��
class REdata
{
public:
	BYTE* data;
	DWORD size;
	~REdata();
};


// dCompress����ѹ��
// dUncompress���ݽ�ѹ
class ECways
{
public:

	//zlibѹ��
	//Data:��Ҫ���������
	//Size:�������鳤�ȣ�����������С��
	//Levelѹ�����𣺿�����-1��9��    
	//-1 Ĭ��ѹ��
	//0 ��ѹ��
	//1--9ѹ��������ߵ��ٶ��½�
	static REdata* dCompress_Zlib(BYTE* Data, DWORD Size, int Level);

	//zlib��ѹ
	//Data:��Ҫ���������
	//Size:�������鳤�ȣ�����������С��
	//MaxSize:��������ѹ��С����Ҫ�û�������
	//�����zlib.h�����뾲̬�⣬������64λ�¿����С�
	static REdata* dUncompress_Zlib(BYTE* Data, DWORD Size, DWORD MaxSize);
};

#endif HS_PACK