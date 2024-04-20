#pragma once
#include <Windows.h>
#include<stdio.h>
#include"Functions.h"
#include"mapstruct.h"
int mode = 0;//模式

struct InI//用于储存配置文件结构体
{
	DWORD BlockOffset;//块偏移
	DWORD BlockFlag;//块标识
	SegmentStdinfo stdinfo;//stdinfo段
	SegmentCptype cptype;//cptype段
	SegmentImagemode imgmode;//imgmode段
	SegmentAst00xx ast00xx;//ast00xx段
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

BYTE* GetMixTable(BYTE* tabledata, DWORD tablesize, BYTE* data, DWORD datasize, DWORD confirm_size,DWORD &mixtablesize)//返回混洗表
{
	CompleteReversee(tabledata, tablesize);//按比特逆置
	BYTE* bit = Data_to_Bit(tabledata, tablesize);//数据转比特
	delete[]tabledata; tabledata = nullptr;
	BYTE first = bit[0];
	tablesize = tablesize - 1;
	DWORD* table = uEliasGammaCode(bit + 1, tablesize);//EliasGamma编码
	BYTE* MixTable = new BYTE[table[0]]();//将值初始化为0
	DWORD ptr1 = 0;//指向指向data的位置
	DWORD ptr2 = 0;//指向mixtable的位置
	for (int i = 1; i < tablesize; i++)//将data的数据填入指定位置，其他位置为0
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
	mixtablesize = table[0];//值得注意的是，混洗表的大小不一定等于原数据的长*宽*4（只需要后续转为差分表后，从差分表前面截取其长为长*宽*4的一段用于前缀还原即可）
	delete[]table; 
	return MixTable;
}
BYTE* operate(char* data, DWORD height, DWORD length)//差分表还原为原数据（差分表的数据记录的是与它相邻像素的差值）
{
	char** array = new char* [height]();
	for (int i = 0; i < height; i++)//截取差分表中长为length*height*4的一段
	{
		array[i] = new char[length * 4]();
		memcpy(&array[i][0], data + i * length * 4, length * 4);
	}
	for (int i = 1; i < length; i++)//第一行按像素求前缀和还原（每个像素的大小是4字节，还原时加上的是像素的对应颜色值，并非是直接与数组前一个元素相加）
	{
		for (int k = 0; k < 4; k++)
		{
			array[0][i * 4 + k] += array[0][(i - 1) * 4 + k];
		}
	}
	for (int i = 0; i < length * 4; i++)//第二行直接求前缀和，反正都是不同的像素，值可以直接相加
	{
		for (int k = 1; k < height; k++)
			array[k][i] += array[k - 1][i];
	}
	BYTE* result = new BYTE[length * height * 4]();
	for (int i = 0; i < height; i++)//拷贝还原的值
	{
		memcpy(result + i * length * 4, &array[i][0], length * 4);
		delete[]array[i];
	}
	delete[]array;
	return result;
}

char* reoperate(char* data, DWORD height, DWORD length)//将原像素值还原为差分表，这是像素的差分记值法。过程与上面差分表转像素相反（由于像素差值可能为负，将其转为有符号类型的char*）
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

BYTE* back_to_mixtable(char* data, DWORD height, DWORD length)//将差分表还原混洗表
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
			if (data[i * 4 + k] >= 0) // 改成无符号数
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

void back_to_MDdata(BYTE* data, DWORD height, DWORD length, BYTE*& remap, BYTE*& redata, DWORD& mUncompresslength, DWORD& dUncompresslength)//将混洗表分离为数据表和数据体
{
	DWORD countnum = 0;
	DWORD dataptr = 0; DWORD numPTR = 1;//分别用于记录数据体的长度，数据表（目前为无符号整型数组）长度
	BYTE first = (data[0] != 0);
	DWORD datamapsize = height * length * 4>0x400? height * length * 4:0x400;
	DWORD* num_arr = new DWORD[datamapsize]{};
	num_arr[0] = datamapsize;
	BYTE* data_arr = new BYTE[datamapsize]{};
	if (height * length * 4 < 0x400)//修正大小，大小不足0x400默认修正为0x400
	{
		BYTE* resize = new BYTE[datamapsize]();
		memcpy(resize, data, datamapsize);
		delete[]data;
		data = resize;
	}
	for (int i = 0; i < datamapsize; i++)//分离数据表与数据体（还不是最终结果）
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
	//数据体可以直接返回了
	BYTE* data_result = new BYTE[dataptr];
	memcpy(data_result, data_arr, dataptr);
	delete[]data_arr;
	redata = data_result; dUncompresslength = dataptr;
	//数据表现在需要将DWORD数组重新回到编码前的比特数组
	BYTE* codedata = cEliasGammaCode(num_arr, numPTR, TRANS_BIT);
	delete[]num_arr;
	BYTE* reverse_map = new BYTE[numPTR + 1];
	reverse_map[0] = first;
	memcpy(reverse_map + 1, codedata, numPTR);
	DWORD size = numPTR + 1;
	remap = Bit_to_Data(reverse_map, size);
	//回到编码前的比特数组后，按比特位逆置
	CompleteReversee(remap, size);
	//返回
	delete[]reverse_map;
	mUncompresslength = size;
}

BYTE* GetTrueData(BYTE* table, BYTE* data, DWORD tablesize, DWORD datasize, DWORD height, DWORD length)
{
	DWORD mixtablesize = 0;
	BYTE* mixtable = GetMixTable(table, tablesize, data, datasize, height * length * 4, mixtablesize);//获得混洗表
	if (mixtable == nullptr)
		return nullptr;
	BitReverse(mixtable, mixtablesize);
	//分别获得混洗表4部分开头指针
	BYTE* BIT1 = mixtable;
	BYTE* BIT2 = BIT1 + mixtablesize / 4;
	BYTE* BIT3 = BIT2 + mixtablesize/4;
	BYTE* BIT4 = BIT3 + mixtablesize / 4;
	char* map = new char[mixtablesize]();//用于接收转化后的混洗表
	DWORD ptr = 0;
	for (int i = 0; i < height*length; i++, ptr++)//获取差分表（叫做差分表的原因并不是因为每次从每部分抽取2比特。真正原因是其转化后是一个像素的差分纪值表）
	{
		for (int k = 0; k < 4; k++)
		{//提取每部分的前两位组成新的一个数
			BYTE temp = (((BIT1[i] >> (6 - k * 2)) % 4) << 6) + (((BIT2[i] >> (6 - k * 2)) % 4) << 4) + (((BIT3[i] >> (6 - k * 2)) % 4) << 2) + (BIT4[i] >> (6 - k * 2)) % 4;
			if (temp % 2 == 0)
			{
				map[ptr * 4 + k] = temp >> 1;//改成有符号数
			}
			else
			{
				map[ptr * 4 + k] = -((temp + 1) >> 1);//改成有符号数
			}
		}
	}
	BYTE* result = operate(map, height, length);//获得原始颜色数据数组
	delete[]map; delete[]mixtable;
	return result;
}

void tans_picture(LPWSTR filename, BYTE* data, DWORD height, DWORD length, DWORD Xsize, DWORD Ysize)//根据不同模式转化为不同的图像输出
{
	if (mode == 2)//转位图
	{
		BYTE ptr = wcslen(filename);
		pathremove(filename, L".ini", L".bmp");
		create_bitmap(filename, data, height, length, Xsize, Ysize);
	}
	else if (mode == 3)//转png
	{
		pathremove(filename, L".ini", L".png");
		colordata_to_png(filename, data, height, length);
	}
}
void reform(BYTE* data, DWORD length, DWORD height, BYTE*& datatable, BYTE*& maptable, DWORD& mUncompresslength, DWORD& dUncompresslength, DWORD& mCompresslength, DWORD& dCompresslength)
{
	char* chiptable = reoperate((char*)data, height, length);//重新回到差分表表
	BYTE* mixtable = back_to_mixtable(chiptable, height, length);//回到混洗表
	delete[]chiptable;
	DWORD rightsize = length * height*4>0x400? length * height * 4:0x400;
	BitReverse(mixtable,rightsize);//混洗表数据比特位反向
	BYTE* uncompressedmap = nullptr; BYTE* uncompresseddata = nullptr;//用于接收未解压数据
	back_to_MDdata(mixtable, height, length, uncompressedmap, uncompresseddata, mUncompresslength, dUncompresslength);//接收数据表数据体数据，以及数据表和数据体未解压前的长度
	dCompresslength = dUncompresslength; mCompresslength = mUncompresslength;//由于 ZlibCompress会将传入的未压缩大小更新为压缩后的大小，所以进行赋值
	datatable = ZlibCompress(uncompresseddata, dCompresslength,9);//压缩数据
	maptable = ZlibCompress(uncompressedmap, mCompresslength,9);//压缩数据
}
void trans_hg3()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////reform(?--->hg3)
	WIN32_FIND_DATAW FindData;
	if (mode == 4)
	{
		HANDLE hfind = FindFirstFileW(L"*", &FindData);//寻找所有文件
		do
		{
			if (FindData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && hfind != INVALID_HANDLE_VALUE)//过滤非文件夹
			{
				if (FindData.cFileName[0] != '.')//忽略隐藏文件
				{
					WCHAR* filenameini = new WCHAR[64]{};//用于储存ini文件名
					//将要创建文件的地址写入filenameini
					lstrcatW(filenameini, FindData.cFileName);
					lstrcatW(filenameini, L"\\*.ini");
					//寻找所有ini文件
					WIN32_FIND_DATAW Findini{};
					HANDLE hfindini = FindFirstFileW(filenameini, &Findini);//寻找所有后缀为.ini的文件
					HANDLE  hfile = INVALID_HANDLE_VALUE;
					if ((INVALID_HANDLE_VALUE != hfindini))
					{
						WCHAR* hg3name = new WCHAR[128]{};//用于记录生成的hg3图片名
						//填入文件名
						lstrcatW(hg3name, FindData.cFileName);
						lstrcatW(hg3name, L".hg3");
						//创建要写入数据的文件
						hfile = CreateFileW(hg3name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						//写入标准hg3文件头
						Hg3Head HEAD{}; HEAD.FileType[0] = 0X48; HEAD.FileType[1] = 0X47; HEAD.FileType[2] = 0X2D; HEAD.FileType[3] = 0X33;
						HEAD.HeadSize = 0X0C; HEAD.Unknown = 0X300;
						WriteFile(hfile, &HEAD, 12, nullptr, nullptr);
						delete[]hg3name;
					}
					do
					{
						if (Findini.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY && hfindini != INVALID_HANDLE_VALUE)//过滤搜寻ini文件时找到的文件夹（某些文件夹可能命名为.ini）
						{
							WCHAR* pngname = new WCHAR[128]{};//用于记录每个ini文件对应的png图片名（现在临时记录为ini的文件名）
							//填入图片名
							lstrcatW(pngname, FindData.cFileName);
							lstrcatW(pngname, L"\\");
							lstrcatW(pngname, Findini.cFileName);
							//打开配置文件
							HANDLE hini = CreateFileW(pngname, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
							//获得图片名
							pathremove(pngname, L".ini", L".png");
							//用于接收图片的长宽
							DWORD height = 0; DWORD length = 0;
							//获得图片像素数据
							BYTE* data = png_to_colordata(pngname, height, length);//png图片数据转rgb颜色数据
							if (data != nullptr)
							{
								//用于储存数据的结构
								InI infomation{}; SegmentHhead segmenthead{};	SegmentImage0000 img0000{};
								//读取块标识
								ReadFile(hini, &infomation.BlockFlag, 4, nullptr, nullptr);
								while (1)
								{
									ReadFile(hini, &segmenthead, 16, nullptr, nullptr);
									if (!lstrcmpA(segmenthead.Lable, "stdinfo"))
									{
										ReadFile(hini, &infomation.stdinfo, 40, nullptr, nullptr);
										//遇到stdinfo段后立即写入图像数据
										img0000.pHeight = infomation.stdinfo.pHeight;
										//数据还原为压缩数据
										reform(data, length, height, img0000.Data, img0000.Map, img0000.mUnCompressLength, img0000.dUnCompressLength, img0000.mCompressLength, img0000.dCompressLength);
										//记录块偏移
										infomation.BlockOffset = GetFileSize(hini, nullptr) + img0000.dCompressLength + img0000.mCompressLength + 44;;
										WriteFile(hfile, &infomation, 8, nullptr, nullptr);
										WriteFile(hfile, &segmenthead, 16, nullptr, nullptr);
										infomation.stdinfo.pHeight = height; infomation.stdinfo.pLength = length;
										WriteFile(hfile, &infomation.stdinfo, 40, nullptr, nullptr);
										//写入段偏移
										char img0000flag[9] = "img0000"; DWORD offset = img0000.dCompressLength + img0000.mCompressLength + 40;
										WriteFile(hfile, &img0000flag, 8, nullptr, nullptr);
										WriteFile(hfile, &offset, 4, nullptr, nullptr);
										offset = offset - 16;
										WriteFile(hfile, &offset, 4, nullptr, nullptr);
										//写入img0000图像数据（不管之前图像是以img0000写入的，还是jpg形式。这里统一还原为img0000,比较jpg的图像算法对图像有损）
										WriteFile(hfile, &img0000, 24, nullptr, nullptr);
										WriteFile(hfile, img0000.Data, img0000.dCompressLength, nullptr, nullptr);
										WriteFile(hfile, img0000.Map, img0000.mCompressLength, nullptr, nullptr);
									}
									else
									{
										//用于记录其他段的数据
										BYTE  message[128]{};
										//写入其他非图像数据段
										ReadFile(hini, message, segmenthead.SegmentLength, nullptr, nullptr);
										WriteFile(hfile, &segmenthead, 16, nullptr, nullptr);
										WriteFile(hfile, &message, segmenthead.SegmentLength, nullptr, nullptr);//写入其他段ast00xx,cptype,imgmode等等
									}
									if (segmenthead.SegmentOffset == 0)//段退出标志
										break;
								}
							}
							delete[]data;
							CloseHandle(hini);
						}
					} while (FindNextFileW(hfindini, &Findini));//搜索下一个后缀为.ini的文件
					delete[]filenameini;
					FindClose(hfindini);
				}
			}
		} while (FindNextFileW(hfind, &FindData));//搜索下一个后缀为.hg3的文件
		FindClose(hfind);
		printf("\n已合成所有hg3图片\n");
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////transpicture(hg3-->?)
	HANDLE hfind = FindFirstFileW(L"*.hg3", &FindData);//打开当前目录下的所有文件
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) // 忽略某些名字的文件夹
			{
				HANDLE hfile = CreateFileW(FindData.cFileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				PathRemoveExtensionW(FindData.cFileName);
				CreateDirectoryW(FindData.cFileName, nullptr);
				BYTE sign = 0;
				SetFilePointer(hfile, 12, nullptr, FILE_CURRENT);
				while (1)//读取每个块的数据
				{
					DWORD has_remain = 1;
					WCHAR* fileaddress = new WCHAR[64]();
					lstrcatW(fileaddress, FindData.cFileName);
					lstrcatW(fileaddress, L"\\");
					lstrcatW(fileaddress, FindData.cFileName);
					WCHAR num[10] = L"#0000.ini";
					reNumber(num, sign);//序号储存在数组中
					lstrcatW(fileaddress, num);
					InI info{};	DWORD jpgdatasize = 0;
					ReadFile(hfile, &info, 8, nullptr, nullptr);
					HANDLE hini = CreateFileW(fileaddress, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//创建配置文件
					WriteFile(hini, &info.BlockFlag, 4, nullptr, nullptr);//写入块标识
					//各种零时用于接收从文件中读取数据的结构体
					SegmentHhead segmenthead{};
					SegmentImage0000 img0000{};
					SegmentImage_jpg img_jpg{};
					SegmentImage_al img_al{};
					BYTE* result = nullptr;
					while (1)
					{
						ReadFile(hfile, &segmenthead, 16, nullptr, nullptr);//读取数据头
						if (!lstrcmpA(segmenthead.Lable, "stdinfo"))//stdinfo段
						{
							ReadFile(hfile, &info.stdinfo, 40, nullptr, nullptr);
							WriteFile(hini, &segmenthead, 16, nullptr, nullptr);
							WriteFile(hini, &info.stdinfo, 40, nullptr, nullptr);//写入stdinfo段
						}
						else if (!lstrcmpA(segmenthead.Lable, "img0000"))//内置的一种图片格式
						{
							ReadFile(hfile, &img0000, 24, nullptr, nullptr);
							img0000.Data = new BYTE[img0000.dCompressLength];
							img0000.Map = new BYTE[img0000.mCompressLength];
							ReadFile(hfile, img0000.Data, img0000.dCompressLength, nullptr, nullptr);//数据体
							ReadFile(hfile, img0000.Map, img0000.mCompressLength, nullptr, nullptr);//数据表
							img0000.Data = ZlibUncompress(img0000.Data, img0000.dCompressLength, img0000.dUnCompressLength);//解压后的数据体
							img0000.Map = ZlibUncompress(img0000.Map, img0000.mCompressLength, img0000.mUnCompressLength);//解压后的数据表
							if (img0000.Data && img0000.Map)
							{//获得rgb像素数据
								result = GetTrueData(img0000.Map, img0000.Data, img0000.mCompressLength, img0000.dCompressLength, info.stdinfo.pHeight, info.stdinfo.pLength);
								//转化图片
								if (result != nullptr)
								tans_picture(fileaddress, result, info.stdinfo.pHeight, info.stdinfo.pLength, 0x1625, 0x1625);
								 delete[]result;
							}
						}
						else if (!lstrcmpA(segmenthead.Lable, "img_jpg"))//记录jpg数据
						{
							img_jpg.JpgData = new BYTE[segmenthead.SegmentLength];
							ReadFile(hfile, img_jpg.JpgData, segmenthead.SegmentLength, nullptr, nullptr);
							jpgdatasize = segmenthead.SegmentLength;
						}
						else if (!lstrcmpA(segmenthead.Lable, "img_al"))//记录透明通道，如果有jpg数据，即可进行图片转化
						{
							img_al.Alpha = new BYTE[segmenthead.SegmentLength - 8];
							ReadFile(hfile, &img_al, 8, nullptr, nullptr);
							ReadFile(hfile, img_al.Alpha, segmenthead.SegmentLength - 8, nullptr, nullptr);
							BYTE* alpha = nullptr;
							if (img_jpg.JpgData)//如果不是空值
							alpha = ZlibUncompress(img_al.Alpha, img_al.CompressLength,img_al.UnCompressLength);//解压透明通道
							if (alpha && (img_al.CompressLength == img_al.UnCompressLength))
							{
								result = jpg_to_colordata(img_jpg.JpgData, alpha, jpgdatasize, info.stdinfo.pHeight, info.stdinfo.pLength);//jpg数据还原为rgb颜色数据（已经将透明通道作为数据传入函数）
								if (result != nullptr)
								{
									tans_picture(fileaddress, result, info.stdinfo.pHeight, info.stdinfo.pLength, 0x1625, 0x1625);//调用转换图像函数
									delete[]result;  delete[]img_jpg.JpgData;
								}
								delete[]img_al.Alpha;
							}
							else
							{	printf("jpg图像解压出错\n");						}
						}
						else
						{
							//写入其他段ast00xx等等
							BYTE  message[128]{};
							ReadFile(hfile, message, segmenthead.SegmentLength, nullptr, nullptr);
							WriteFile(hini, &segmenthead, 16, nullptr, nullptr);
							WriteFile(hini, &message, segmenthead.SegmentLength, nullptr, nullptr);
						}
						if (segmenthead.SegmentOffset==0)//无下一个段时
							break;
					}
					CloseHandle(hini);
					if (strlen(segmenthead.Lable) == 0)//这是某一种情况文件结束标志（这个数据块的偏移标识为0，紧接着的数据段头也全是0，之后文件结束）
                         DeleteFileW(fileaddress);//用于删除多余的配置文件，这种情况下的配置文件无任何意义
					delete[]fileaddress;
					sign++;//用于给文件编号
				if (info.BlockOffset==0)//无下一个块时
						break;
				};
			}
		} while (FindNextFileW(hfind, &FindData));//搜索下一个后缀为.hg3的文件
		FindClose(hfind);
	}
	printf("\n所有hg3图片已经转化完成\n");
}