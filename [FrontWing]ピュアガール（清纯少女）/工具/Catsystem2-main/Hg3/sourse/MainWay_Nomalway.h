#pragma once
#include <Windows.h>
#include<stdio.h>
#include"Functions.h"
#include"mapstruct.h"
int mode = 0;//ģʽ

struct InI//���ڴ��������ļ��ṹ��
{
	DWORD BlockOffset;//��ƫ��
	DWORD BlockFlag;//���ʶ
	SegmentStdinfo stdinfo;//stdinfo��
	SegmentCptype cptype;//cptype��
	SegmentImagemode imgmode;//imgmode��
	SegmentAst00xx ast00xx;//ast00xx��
};

void  reNumber(LPWSTR name, BYTE num)//name="#0000"
{
	int ptr = 4;
	while (num != 0)
	{
		int num2 = num % 10;
		num = num / 10;
		name[ptr--] = num2 + 48;
	}
}

BYTE* GetMixTable(BYTE* tabledata, DWORD tablesize, BYTE* data, DWORD datasize, DWORD confirm_size,DWORD &mixtablesize)//���ػ�ϴ��
{
	CompleteReversee(tabledata, tablesize);//����������
	BYTE* bit = Data_to_Bit(tabledata, tablesize);//����ת����
	delete[]tabledata; tabledata = nullptr;
	BYTE first = bit[0];
	tablesize = tablesize - 1;
	DWORD* table = uEliasGammaCode(bit + 1, tablesize);//EliasGamma����
	BYTE* MixTable = new BYTE[table[0]]();//��ֵ��ʼ��Ϊ0
	DWORD ptr1 = 0;//ָ��ָ��data��λ��
	DWORD ptr2 = 0;//ָ��mixtable��λ��
	for (int i = 1; i < tablesize; i++)//��data����������ָ��λ�ã�����λ��Ϊ0
	{
		if ((first + i) % 2 == 0)
		{
			for (int k = 0; k < table[i]; k++)
			{
				MixTable[ptr2 + k] = data[ptr1 + k];
			}
			ptr1 = ptr1 + table[i];
		}
		ptr2 += table[i];
	}
	mixtablesize = table[0];//ֵ��ע����ǣ���ϴ��Ĵ�С��һ������ԭ���ݵĳ�*��*4��ֻ��Ҫ����תΪ��ֱ�󣬴Ӳ�ֱ�ǰ���ȡ�䳤Ϊ��*��*4��һ������ǰ׺��ԭ���ɣ�
	delete[]table; 
	return MixTable;
}
BYTE* operate(char* data, DWORD height, DWORD length)//��ֱ�ԭΪԭ���ݣ���ֱ�����ݼ�¼���������������صĲ�ֵ��
{
	char** array = new char* [height]();
	for (int i = 0; i < height; i++)//��ȡ��ֱ��г�Ϊlength*height*4��һ��
	{
		array[i] = new char[length * 4]();
		memcpy(&array[i][0], data + i * length * 4, length * 4);
	}
	for (int i = 1; i < length; i++)//��һ�а�������ǰ׺�ͻ�ԭ��ÿ�����صĴ�С��4�ֽڣ���ԭʱ���ϵ������صĶ�Ӧ��ɫֵ��������ֱ��������ǰһ��Ԫ����ӣ�
	{
		for (int k = 0; k < 4; k++)
		{
			array[0][i * 4 + k] += array[0][(i - 1) * 4 + k];
		}
	}
	for (int i = 0; i < length * 4; i++)//�ڶ���ֱ����ǰ׺�ͣ��������ǲ�ͬ�����أ�ֵ����ֱ�����
	{
		for (int k = 1; k < height; k++)
			array[k][i] += array[k - 1][i];
	}
	BYTE* result = new BYTE[length * height * 4]();
	for (int i = 0; i < height; i++)//������ԭ��ֵ
	{
		memcpy(result + i * length * 4, &array[i][0], length * 4);
		delete[]array[i];
	}
	delete[]array;
	return result;
}

char* reoperate(char* data, DWORD height, DWORD length)//��ԭ����ֵ��ԭΪ��ֱ��������صĲ�ּ�ֵ���������������ֱ�ת�����෴���������ز�ֵ����Ϊ��������תΪ�з������͵�char*��
{
	char** array = new char* [height]();
	for (int i = 0; i < height; i++)
	{
		array[i] = new char[length * 4]();
		memcpy(&array[i][0], data + i * length * 4, length * 4);
	}
	for (int i = 0; i < length * 4; i++)
	{
		for (int k = height - 1; k >= 1; k--)
			array[k][i] -= array[k - 1][i];
	}
	for (int i = length - 1; i >= 1; i--)
	{
		for (int k = 0; k < 4; k++)
		{
			array[0][i * 4 + k] -= array[0][(i - 1) * 4 + k];
		}
	}
	char* result = new char[length * height * 4>0x400? length * height * 4:0x400]();
	for (int i = 0; i < height; i++)
	{
		memcpy(result + i * length * 4, &array[i][0], length * 4);
		delete[]array[i];
	}
	delete[]array;
	return result;
}

BYTE* back_to_mixtable(char* data, DWORD height, DWORD length)//����ֱ�ԭ��ϴ��
{
	DWORD rightsize = height * length * 4>0x400?height*length*4:0x400;
	BYTE* BIT1 = new BYTE[rightsize]();
	BYTE* BIT2 = BIT1 + rightsize/4;
	BYTE* BIT3 = BIT2 + rightsize/4;
	BYTE* BIT4 = BIT3 + rightsize/4;
	for (int i = 0; i < height*length; i++)
	{
		for (int k = 0; k < 4; k++)
		{
			BYTE temp = 0;
			if (data[i * 4 + k] >= 0) // �ĳ��޷�����
			{
				temp = data[i * 4 + k] * 2;
			}
			else
			{
				temp = -2 * data[i * 4 + k] - 1;
			}
			BIT1[i] += ((temp >> 6) % 4) << (6 - k * 2);
			BIT2[i] += ((temp >> 4) % 4) << (6 - k * 2);
			BIT3[i] += ((temp >> 2) % 4) << (6 - k * 2);
			BIT4[i] += (temp % 4) << (6 - k * 2);
		}
	}
	return BIT1;
}

void back_to_MDdata(BYTE* data, DWORD height, DWORD length, BYTE*& remap, BYTE*& redata, DWORD& mUncompresslength, DWORD& dUncompresslength)//����ϴ�����Ϊ���ݱ��������
{
	DWORD countnum = 0;
	DWORD dataptr = 0; DWORD numPTR = 1;//�ֱ����ڼ�¼������ĳ��ȣ����ݱ�ĿǰΪ�޷����������飩����
	BYTE first = (data[0] != 0);
	DWORD datamapsize = height * length * 4>0x400? height * length * 4:0x400;
	DWORD* num_arr = new DWORD[datamapsize]{};
	num_arr[0] = datamapsize;
	BYTE* data_arr = new BYTE[datamapsize]{};
	if (height * length * 4 < 0x400)//������С����С����0x400Ĭ������Ϊ0x400
	{
		BYTE* resize = new BYTE[datamapsize]();
		memcpy(resize, data, datamapsize);
		delete[]data;
		data = resize;
	}
	for (int i = 0; i < datamapsize; i++)//�������ݱ��������壨���������ս����
	{
		if (data[i])
		{
			if (i != 0)
			{
				num_arr[numPTR++] = countnum;
				countnum = 0;
			}
			while (data[i] && i < datamapsize)
			{
				data_arr[dataptr++] = data[i];
				i++; countnum++;
			}
			num_arr[numPTR++] = countnum;
			countnum = 0;
		}
		if (i == datamapsize - 1)
		{
			num_arr[numPTR++] = countnum + 1;
		}
		countnum++;
	}
	//���������ֱ�ӷ�����
	BYTE* data_result = new BYTE[dataptr];
	memcpy(data_result, data_arr, dataptr);
	delete[]data_arr;
	redata = data_result; dUncompresslength = dataptr;
	//���ݱ�������Ҫ��DWORD�������»ص�����ǰ�ı�������
	BYTE* codedata = cEliasGammaCode(num_arr, numPTR, TRANS_BIT);
	delete[]num_arr;
	BYTE* reverse_map = new BYTE[numPTR + 1];
	reverse_map[0] = first;
	memcpy(reverse_map + 1, codedata, numPTR);
	DWORD size = numPTR + 1;
	remap = Bit_to_Data(reverse_map, size);
	//�ص�����ǰ�ı�������󣬰�����λ����
	CompleteReversee(remap, size);
	//����
	delete[]reverse_map;
	mUncompresslength = size;
}

BYTE* GetTrueData(BYTE* table, BYTE* data, DWORD tablesize, DWORD datasize, DWORD height, DWORD length)
{
	DWORD mixtablesize = 0;
	BYTE* mixtable = GetMixTable(table, tablesize, data, datasize, height * length * 4, mixtablesize);//��û�ϴ��
	if (mixtable == nullptr)
		return nullptr;
	BitReverse(mixtable, mixtablesize);
	//�ֱ��û�ϴ��4���ֿ�ͷָ��
	BYTE* BIT1 = mixtable;
	BYTE* BIT2 = BIT1 + mixtablesize / 4;
	BYTE* BIT3 = BIT2 + mixtablesize/4;
	BYTE* BIT4 = BIT3 + mixtablesize / 4;
	char* map = new char[mixtablesize]();//���ڽ���ת����Ļ�ϴ��
	DWORD ptr = 0;
	for (int i = 0; i < height*length; i++, ptr++)//��ȡ��ֱ�������ֱ��ԭ�򲢲�����Ϊÿ�δ�ÿ���ֳ�ȡ2���ء�����ԭ������ת������һ�����صĲ�ּ�ֵ��
	{
		for (int k = 0; k < 4; k++)
		{//��ȡÿ���ֵ�ǰ��λ����µ�һ����
			BYTE temp = (((BIT1[i] >> (6 - k * 2)) % 4) << 6) + (((BIT2[i] >> (6 - k * 2)) % 4) << 4) + (((BIT3[i] >> (6 - k * 2)) % 4) << 2) + (BIT4[i] >> (6 - k * 2)) % 4;
			if (temp % 2 == 0)
			{
				map[ptr * 4 + k] = temp >> 1;//�ĳ��з�����
			}
			else
			{
				map[ptr * 4 + k] = -((temp + 1) >> 1);//�ĳ��з�����
			}
		}
	}
	BYTE* result = operate(map, height, length);//���ԭʼ��ɫ��������
	delete[]map; delete[]mixtable;
	return result;
}

void tans_picture(LPWSTR filename, BYTE* data, DWORD height, DWORD length, DWORD Xsize, DWORD Ysize)//���ݲ�ͬģʽת��Ϊ��ͬ��ͼ�����
{
	if (mode == 2)//תλͼ
	{
		BYTE ptr = wcslen(filename);
		pathremove(filename, L".ini", L".bmp");
		create_bitmap(filename, data, height, length, Xsize, Ysize);
	}
	else if (mode == 3)//תpng
	{
		pathremove(filename, L".ini", L".png");
		colordata_to_png(filename, data, height, length);
	}
}
void reform(BYTE* data, DWORD length, DWORD height, BYTE*& datatable, BYTE*& maptable, DWORD& mUncompresslength, DWORD& dUncompresslength, DWORD& mCompresslength, DWORD& dCompresslength)
{
	char* chiptable = reoperate((char*)data, height, length);//���»ص���ֱ��
	BYTE* mixtable = back_to_mixtable(chiptable, height, length);//�ص���ϴ��
	delete[]chiptable;
	DWORD rightsize = length * height*4>0x400? length * height * 4:0x400;
	BitReverse(mixtable,rightsize);//��ϴ�����ݱ���λ����
	BYTE* uncompressedmap = nullptr; BYTE* uncompresseddata = nullptr;//���ڽ���δ��ѹ����
	back_to_MDdata(mixtable, height, length, uncompressedmap, uncompresseddata, mUncompresslength, dUncompresslength);//�������ݱ����������ݣ��Լ����ݱ��������δ��ѹǰ�ĳ���
	dCompresslength = dUncompresslength; mCompresslength = mUncompresslength;//���� ZlibCompress�Ὣ�����δѹ����С����Ϊѹ����Ĵ�С�����Խ��и�ֵ
	datatable = ZlibCompress(uncompresseddata, dCompresslength,9);//ѹ������
	maptable = ZlibCompress(uncompressedmap, mCompresslength,9);//ѹ������
}
void trans_hg3()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////reform(?--->hg3)
	WIN32_FIND_DATAW FindData;
	if (mode == 4)
	{
		HANDLE hfind = FindFirstFileW(L"*", &FindData);//Ѱ�������ļ�
		do
		{
			if (FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && hfind != INVALID_HANDLE_VALUE)//���˷��ļ���
			{
				if (FindData.cFileName[0] != '.')//���������ļ�
				{
					WCHAR* filenameini = new WCHAR[64]{};//���ڴ���ini�ļ���
					//��Ҫ�����ļ��ĵ�ַд��filenameini
					lstrcatW(filenameini, FindData.cFileName);
					lstrcatW(filenameini, L"\\*.ini");
					//Ѱ������ini�ļ�
					WIN32_FIND_DATAW Findini{};
					HANDLE hfindini = FindFirstFileW(filenameini, &Findini);//Ѱ�����к�׺Ϊ.ini���ļ�
					HANDLE  hfile = INVALID_HANDLE_VALUE;
					if ((INVALID_HANDLE_VALUE != hfindini))
					{
						WCHAR* hg3name = new WCHAR[128]{};//���ڼ�¼���ɵ�hg3ͼƬ��
						//�����ļ���
						lstrcatW(hg3name, FindData.cFileName);
						lstrcatW(hg3name, L".hg3");
						//����Ҫд�����ݵ��ļ�
						hfile = CreateFileW(hg3name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						//д���׼hg3�ļ�ͷ
						Hg3Head HEAD{}; HEAD.FileType[0] = 0X48; HEAD.FileType[1] = 0X47; HEAD.FileType[2] = 0X2D; HEAD.FileType[3] = 0X33;
						HEAD.HeadSize = 0X0C; HEAD.Unknown = 0X300;
						WriteFile(hfile, &HEAD, 12, nullptr, nullptr);
						delete[]hg3name;
					}
					do
					{
						if (Findini.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY && hfindini != INVALID_HANDLE_VALUE)//������Ѱini�ļ�ʱ�ҵ����ļ��У�ĳЩ�ļ��п�������Ϊ.ini��
						{
							WCHAR* pngname = new WCHAR[128]{};//���ڼ�¼ÿ��ini�ļ���Ӧ��pngͼƬ����������ʱ��¼Ϊini���ļ�����
							//����ͼƬ��
							lstrcatW(pngname, FindData.cFileName);
							lstrcatW(pngname, L"\\");
							lstrcatW(pngname, Findini.cFileName);
							//�������ļ�
							HANDLE hini = CreateFileW(pngname, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
							//���ͼƬ��
							pathremove(pngname, L".ini", L".png");
							//���ڽ���ͼƬ�ĳ���
							DWORD height = 0; DWORD length = 0;
							//���ͼƬ��������
							BYTE* data = png_to_colordata(pngname, height, length);//pngͼƬ����תrgb��ɫ����
							if (data != nullptr)
							{
								//���ڴ������ݵĽṹ
								InI infomation{}; SegmentHhead segmenthead{};	SegmentImage0000 img0000{};
								//��ȡ���ʶ
								ReadFile(hini, &infomation.BlockFlag, 4, nullptr, nullptr);
								while (1)
								{
									ReadFile(hini, &segmenthead, 16, nullptr, nullptr);
									if (!lstrcmpA(segmenthead.Lable, "stdinfo"))
									{
										ReadFile(hini, &infomation.stdinfo, 40, nullptr, nullptr);
										//����stdinfo�κ�����д��ͼ������
										img0000.pHeight = infomation.stdinfo.pHeight;
										//���ݻ�ԭΪѹ������
										reform(data, length, height, img0000.Data, img0000.Map, img0000.mUnCompressLength, img0000.dUnCompressLength, img0000.mCompressLength, img0000.dCompressLength);
										//��¼��ƫ��
										infomation.BlockOffset = GetFileSize(hini, nullptr) + img0000.dCompressLength + img0000.mCompressLength + 44;;
										WriteFile(hfile, &infomation, 8, nullptr, nullptr);
										WriteFile(hfile, &segmenthead, 16, nullptr, nullptr);
										infomation.stdinfo.pHeight = height; infomation.stdinfo.pLength = length;
										WriteFile(hfile, &infomation.stdinfo, 40, nullptr, nullptr);
										//д���ƫ��
										char img0000flag[9] = "img0000"; DWORD offset = img0000.dCompressLength + img0000.mCompressLength + 40;
										WriteFile(hfile, &img0000flag, 8, nullptr, nullptr);
										WriteFile(hfile, &offset, 4, nullptr, nullptr);
										offset = offset - 16;
										WriteFile(hfile, &offset, 4, nullptr, nullptr);
										//д��img0000ͼ�����ݣ�����֮ǰͼ������img0000д��ģ�����jpg��ʽ������ͳһ��ԭΪimg0000,�Ƚ�jpg��ͼ���㷨��ͼ������
										WriteFile(hfile, &img0000, 24, nullptr, nullptr);
										WriteFile(hfile, img0000.Data, img0000.dCompressLength, nullptr, nullptr);
										WriteFile(hfile, img0000.Map, img0000.mCompressLength, nullptr, nullptr);
									}
									else
									{
										//���ڼ�¼�����ε�����
										BYTE  message[128]{};
										//д��������ͼ�����ݶ�
										ReadFile(hini, message, segmenthead.SegmentLength, nullptr, nullptr);
										WriteFile(hfile, &segmenthead, 16, nullptr, nullptr);
										WriteFile(hfile, &message, segmenthead.SegmentLength, nullptr, nullptr);//д��������ast00xx,cptype,imgmode�ȵ�
									}
									if (segmenthead.SegmentOffset == 0)//���˳���־
										break;
								}
							}
							delete[]data;
							CloseHandle(hini);
						}
					} while (FindNextFileW(hfindini, &Findini));//������һ����׺Ϊ.ini���ļ�
					delete[]filenameini;
					FindClose(hfindini);
				}
			}
		} while (FindNextFileW(hfind, &FindData));//������һ����׺Ϊ.hg3���ļ�
		FindClose(hfind);
		printf("\n�Ѻϳ�����hg3ͼƬ\n");
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////transpicture(hg3-->?)
	HANDLE hfind = FindFirstFileW(L"*.hg3", &FindData);//�򿪵�ǰĿ¼�µ������ļ�
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) // ����ĳЩ���ֵ��ļ���
			{
				HANDLE hfile = CreateFileW(FindData.cFileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				PathRemoveExtensionW(FindData.cFileName);
				CreateDirectoryW(FindData.cFileName, nullptr);
				BYTE sign = 0;
				SetFilePointer(hfile, 12, nullptr, FILE_CURRENT);
				while (1)//��ȡÿ���������
				{
					DWORD has_remain = 1;
					WCHAR* fileaddress = new WCHAR[64]();
					lstrcatW(fileaddress, FindData.cFileName);
					lstrcatW(fileaddress, L"\\");
					lstrcatW(fileaddress, FindData.cFileName);
					WCHAR num[10] = L"#0000.ini";
					reNumber(num, sign);//��Ŵ�����������
					lstrcatW(fileaddress, num);
					InI info{};	DWORD jpgdatasize = 0;
					ReadFile(hfile, &info, 8, nullptr, nullptr);
					HANDLE hini = CreateFileW(fileaddress, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//���������ļ�
					WriteFile(hini, &info.BlockFlag, 4, nullptr, nullptr);//д����ʶ
					//������ʱ���ڽ��մ��ļ��ж�ȡ���ݵĽṹ��
					SegmentHhead segmenthead{};
					SegmentImage0000 img0000{};
					SegmentImage_jpg img_jpg{};
					SegmentImage_al img_al{};
					BYTE* result = nullptr;
					while (1)
					{
						ReadFile(hfile, &segmenthead, 16, nullptr, nullptr);//��ȡ����ͷ
						if (!lstrcmpA(segmenthead.Lable, "stdinfo"))//stdinfo��
						{
							ReadFile(hfile, &info.stdinfo, 40, nullptr, nullptr);
							WriteFile(hini, &segmenthead, 16, nullptr, nullptr);
							WriteFile(hini, &info.stdinfo, 40, nullptr, nullptr);//д��stdinfo��
						}
						else if (!lstrcmpA(segmenthead.Lable, "img0000"))//���õ�һ��ͼƬ��ʽ
						{
							ReadFile(hfile, &img0000, 24, nullptr, nullptr);
							img0000.Data = new BYTE[img0000.dCompressLength];
							img0000.Map = new BYTE[img0000.mCompressLength];
							ReadFile(hfile, img0000.Data, img0000.dCompressLength, nullptr, nullptr);//������
							ReadFile(hfile, img0000.Map, img0000.mCompressLength, nullptr, nullptr);//���ݱ�
							img0000.Data = ZlibUncompress(img0000.Data, img0000.dCompressLength, img0000.dUnCompressLength);//��ѹ���������
							img0000.Map = ZlibUncompress(img0000.Map, img0000.mCompressLength, img0000.mUnCompressLength);//��ѹ������ݱ�
							if (img0000.Data && img0000.Map)
							{//���rgb��������
								result = GetTrueData(img0000.Map, img0000.Data, img0000.mCompressLength, img0000.dCompressLength, info.stdinfo.pHeight, info.stdinfo.pLength);
								//ת��ͼƬ
								if (result != nullptr)
								tans_picture(fileaddress, result, info.stdinfo.pHeight, info.stdinfo.pLength, 0x1625, 0x1625);
								 delete[]result;
							}
						}
						else if (!lstrcmpA(segmenthead.Lable, "img_jpg"))//��¼jpg����
						{
							img_jpg.JpgData = new BYTE[segmenthead.SegmentLength];
							ReadFile(hfile, img_jpg.JpgData, segmenthead.SegmentLength, nullptr, nullptr);
							jpgdatasize = segmenthead.SegmentLength;
						}
						else if (!lstrcmpA(segmenthead.Lable, "img_al"))//��¼͸��ͨ���������jpg���ݣ����ɽ���ͼƬת��
						{
							img_al.Alpha = new BYTE[segmenthead.SegmentLength - 8];
							ReadFile(hfile, &img_al, 8, nullptr, nullptr);
							ReadFile(hfile, img_al.Alpha, segmenthead.SegmentLength - 8, nullptr, nullptr);
							BYTE* alpha = nullptr;
							if (img_jpg.JpgData)//������ǿ�ֵ
							alpha = ZlibUncompress(img_al.Alpha, img_al.CompressLength,img_al.UnCompressLength);//��ѹ͸��ͨ��
							if (alpha && (img_al.CompressLength == img_al.UnCompressLength))
							{
								result = jpg_to_colordata(img_jpg.JpgData, alpha, jpgdatasize, info.stdinfo.pHeight, info.stdinfo.pLength);//jpg���ݻ�ԭΪrgb��ɫ���ݣ��Ѿ���͸��ͨ����Ϊ���ݴ��뺯����
								if (result != nullptr)
								{
									tans_picture(fileaddress, result, info.stdinfo.pHeight, info.stdinfo.pLength, 0x1625, 0x1625);//����ת��ͼ����
									delete[]result;  delete[]img_jpg.JpgData;
								}
								delete[]img_al.Alpha;
							}
							else
							{	printf("jpgͼ���ѹ����\n");						}
						}
						else
						{
							//д��������ast00xx�ȵ�
							BYTE  message[128]{};
							ReadFile(hfile, message, segmenthead.SegmentLength, nullptr, nullptr);
							WriteFile(hini, &segmenthead, 16, nullptr, nullptr);
							WriteFile(hini, &message, segmenthead.SegmentLength, nullptr, nullptr);
						}
						if (segmenthead.SegmentOffset==0)//����һ����ʱ
							break;
					}
					CloseHandle(hini);
					if (strlen(segmenthead.Lable) == 0)//����ĳһ������ļ�������־��������ݿ��ƫ�Ʊ�ʶΪ0�������ŵ����ݶ�ͷҲȫ��0��֮���ļ�������
                         DeleteFileW(fileaddress);//����ɾ������������ļ�����������µ������ļ����κ�����
					delete[]fileaddress;
					sign++;//���ڸ��ļ����
				if (info.BlockOffset==0)//����һ����ʱ
						break;
				};
			}
		} while (FindNextFileW(hfind, &FindData));//������һ����׺Ϊ.hg3���ļ�
		FindClose(hfind);
	}
	printf("\n����hg3ͼƬ�Ѿ�ת�����\n");
}