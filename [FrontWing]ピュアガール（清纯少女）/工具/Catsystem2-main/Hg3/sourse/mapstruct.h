#pragma once
#include<Windows.h>
#include<opencv2/opencv.hpp>

#define COLOR_24 3;//位深标识
#define COLOR_32 4;//位深标识

////////////////////////////////////////////////////////////////////////////////函数区/////////////////////////////////////////////////////////////////////////////////////////////////
void flipImageX(BYTE*& imgData, int length, int height,int COLOR_DEPTH) {//x轴翻转图片
	int width = length * COLOR_DEPTH; //一个像素用4个字节表示
	BYTE* tmp = new BYTE[width];
	for (int i = 0; i < height / 2; ++i) {
		// 获取要交换的两行的数据
		BYTE* row1 = imgData + i * width;
		BYTE* row2 = imgData + (height - i - 1) * width;
		memcpy(tmp, row1, width);// 备份第一行
		memcpy(row1, row2, width);// 用第二行的数据覆盖第一行
		memcpy(row2, tmp, width);// 用备份的数据覆盖第二行
	}
	delete[]tmp;
}

void flipImageY(BYTE*& imgData, int length, int height, int COLOR_DEPTH) {//y轴翻转图片
	int pixelByte = COLOR_DEPTH;  //一个像素用4个字节表示
	int width = length * pixelByte; //每行的总数据量
	BYTE* tmp = new BYTE[pixelByte]; //暂存一个像素点的数据
	for (int i = 0; i < height; ++i) {
		BYTE* row = imgData + i * width;
		for (int j = 0; j < length / 2; ++j) {
			// 获取要交换的两个像素点的数据
			BYTE* pixel1 = row + j * pixelByte;
			BYTE* pixel2 = row + (length - j - 1) * pixelByte;
			memcpy(tmp, pixel1, pixelByte); // 备份第一个像素点
			memcpy(pixel1, pixel2, pixelByte); //用第二个像素点的数据覆盖第一个像素点
			memcpy(pixel2, tmp, pixelByte); //用备份的数据覆盖第二个像素点
		}
	}
	delete[]tmp;
}
//////////////////////////////////////////////////////////////////////////////////////比特图///////////////////////////////////////////////////////////////////////////////////////////
struct BitMapData//比特图结构
{
	BITMAPFILEHEADER* FileHeader;//文件头14字节
	tagBITMAPINFOHEADER* InfoHeader;//文件信息头40字节
	tagRGBQUAD* colorboard;//调色板
	BYTE* ImageData;//图像数据
};

tagBITMAPINFOHEADER* get_bitmapinfoheader(DWORD height, DWORD length, DWORD Xsize, DWORD Ysize)//传入数据形成比特图的文件头
{
	tagBITMAPINFOHEADER* infoheader =new tagBITMAPINFOHEADER();
	infoheader->biSize = 40;
	infoheader->biWidth = length;
	infoheader->biHeight = height;
	infoheader->biBitCount = 32;
	infoheader->biSizeImage = height * 4 * length;
	infoheader->biXPelsPerMeter = Xsize;
	infoheader->biYPelsPerMeter = Ysize;
	return infoheader;
}

void create_bitmap(LPCWSTR filename, BYTE* data, DWORD height, DWORD length,DWORD Xsize,DWORD Ysize)//创建的图片为带alph通道的32位色深比特图
{
	BitMapData bitmap{};
	bitmap.FileHeader = new BITMAPFILEHEADER();
	bitmap.InfoHeader = get_bitmapinfoheader(height,  length, Xsize, Ysize);
	bitmap.FileHeader->bfType = 0x4d42;
	bitmap.FileHeader->bfSize = height * length * 4 + 54;
	bitmap.FileHeader->bfOffBits = 54;
	HANDLE hfile = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hfile, bitmap.FileHeader, 14, nullptr, nullptr);
	WriteFile(hfile, bitmap.InfoHeader, 40, nullptr, nullptr);
	WriteFile(hfile, data, height * length * 4, nullptr, nullptr);
	CloseHandle(hfile);
};
//////////////////////////////////////////////////////////////////////////////////////hg3图片//////////////////////////////////////////////////////////////////////////////////////
struct Hg3Head//文件头
{
	BYTE FileType[4];//文件名类型HG-3
	DWORD HeadSize;//文件头长，恒为12字节
	DWORD Unknown;//未知值，恒为0x3000
};

struct BlockHead//数据块头
{
	DWORD BlockOffset;//下一个块相对于当前块的偏移
	DWORD BlockFlag;//块的标识(相当于块的id),程序根据块的id来鉴定是哪一幅图
};

struct SegmentHhead //数据段头
{
	char Lable[8]{};//数据段标签，即名字
	DWORD SegmentOffset;//下个数据段偏移
	DWORD SegmentLength;//数据段长
};

struct SegmentStdinfo//std数据头
{
	DWORD pLength;//图长
	DWORD pHeight;//图宽
	DWORD pDepth;//图深度
	DWORD CanvasOffsetX;//与画布偏移x值
	DWORD CanvasOffsetY;//与画布偏移y值
	DWORD CanvasLength;//画布长度
	DWORD CanvasHeight;//画布宽度
	DWORD pFlag;//图片标识，是否为画布（0代表是，1代表不是）
	DWORD BaseX;//基准x坐标(图片位置参考值)
	DWORD BaseY;//基准y坐标（图片位置参考值)
};
struct  SegmentImage_jpg
{
	BYTE* JpgData=nullptr;//jpg图片的数据（不含文件头）
};

struct  SegmentImage_al
{
	DWORD CompressLength;//解压前的Alpha通道数据长
	DWORD UnCompressLength;//解压后的Alpha通道数据长（大小等于像素数目）
	BYTE* Alpha;//Alpha通道数据,zlib压缩
};

struct  SegmentImage0000
{
	DWORD Unkonwn;//未知恒为0
	DWORD pHeight;//图片宽度
	DWORD dCompressLength;//数据体解压前大小
	DWORD dUnCompressLength;//数据体解压后大小
	DWORD mCompressLength;//数据表解压前大小
	DWORD mUnCompressLength;//数据表解压后大小
	BYTE* Data=nullptr;//数据体压缩数据
	BYTE* Map=nullptr;//数据表压缩数据
};

struct  SegmentAst00xx//未知字段
{
	DWORD u1;
	DWORD u2;
	DWORD u3;
	DWORD u4;
};

struct  SegmentCptype//Cptype字段
{
	DWORD Unkonwn;//恒为0
};

struct  SegmentImagemode//SegmentImagemode字段
{
	DWORD Unkonwn;//恒为0
};

////////////////////////////////////////////////////////////////////////////////////jpg图片//////////////////////////////////////////////////////////////////////////////////////
BYTE* jpg_to_colordata(BYTE* jpgdata, BYTE* alphadata, DWORD jpgdatasize, DWORD height, DWORD length)
{
	//jpg图像转颜色数据，alphadata通道值为nullptr依旧转为4字节，只是将透明通道颜色设为最大值，即不透明
	// 从二进制数据创建Mat对象
	//cv::Mat(jpg_data)需要知道数据长度，而单纯的字节数组是无法得知长度的。所以需要将数据放于容器中接收
	std::vector<uchar> jpg_data;
	for (int i = 0; i < jpgdatasize; i++)
		jpg_data.push_back(jpgdata[i]);
	cv::Mat image = cv::imdecode(cv::Mat(jpg_data), cv::IMREAD_COLOR);//转化所有颜色
	BYTE* data = image.data;
	if (!image.data)
	{
		printf_s("创建jpg图像对象失败\\n");
		return nullptr;
	}
	BYTE* result = new BYTE[height * length * 4]{};
	for (int i = 0; i < height * length; i++)//填入透明通道
	{
		int row = i / length;
		int col = i % length;
		for (int k = 0; k < 3; k++)
		{
			result[(height - 1 - row) * length * 4 + col * 4 + k] = data[i * 3 + k];
		}
		if (alphadata)
		{
			result[(height - 1 - row) * length * 4 + col * 4 + 3] = alphadata[i];
		}
		else
		{
			result[(height - 1 - row) * length * 4 + col * 4 + 3] = 255;
		}
	}
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////png图片///////////////////////////////////////////////////////////////////////
BYTE* png_to_colordata(LPWSTR filename,DWORD&rheight ,DWORD&rlength)//png图片转为rgb颜色数据
{
	char* name = Wide_To_Char(filename);
	cv::Mat image = cv::imread(name, cv::IMREAD_UNCHANGED);
	BYTE* data = image.data; 
	cv::Size msize(image.size());
	rheight = msize.height;
	rlength = msize.width;
	BYTE* result = new BYTE[rheight*rlength*4];
	int a = image.channels();
	flipImageX(data, rlength, rheight, image.channels());
	if (image.channels() == 3)//判断通道数量，是否有透明色
	{
		for (int i = 0; i < rheight * rlength; i++)
		{
			for (int k = 0; k < 3; k++)
			result[i * 4 + k] = data[i * 3 + k];
			result[i * 4 + 3] = 255;
	    }
		return result;
	}
	memcpy(result, data,rheight * rlength * 4);
	return result;
}

void colordata_to_png(LPWSTR filename, BYTE* data, DWORD height, DWORD length)
{
	cv::Mat img = cv::Mat(height, length, CV_8UC4, data);	// 创建一个Mat对象来保存图像数据，为了处理带透明度的PNG使用CV_8UC4
	cv::flip(img, img, 0);// 翻转图片，OpenCV中y轴的正方向是向下的，和位图图像数据中y轴正方向（向上）是相反的
	char* w = Wide_To_Char(filename);// 输出图片到一个.png文件中
	cv::imwrite(w, img);
}
