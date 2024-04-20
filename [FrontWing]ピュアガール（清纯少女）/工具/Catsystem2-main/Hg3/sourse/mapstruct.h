#pragma once
#include<Windows.h>
#include<opencv2/opencv.hpp>

#define COLOR_24 3;//λ���ʶ
#define COLOR_32 4;//λ���ʶ

////////////////////////////////////////////////////////////////////////////////������/////////////////////////////////////////////////////////////////////////////////////////////////
void flipImageX(BYTE*& imgData, int length, int height,int COLOR_DEPTH) {//x�ᷭתͼƬ
	int width = length * COLOR_DEPTH; //һ��������4���ֽڱ�ʾ
	BYTE* tmp = new BYTE[width];
	for (int i = 0; i < height / 2; ++i) {
		// ��ȡҪ���������е�����
		BYTE* row1 = imgData + i * width;
		BYTE* row2 = imgData + (height - i - 1) * width;
		memcpy(tmp, row1, width);// ���ݵ�һ��
		memcpy(row1, row2, width);// �õڶ��е����ݸ��ǵ�һ��
		memcpy(row2, tmp, width);// �ñ��ݵ����ݸ��ǵڶ���
	}
	delete[]tmp;
}

void flipImageY(BYTE*& imgData, int length, int height, int COLOR_DEPTH) {//y�ᷭתͼƬ
	int pixelByte = COLOR_DEPTH;  //һ��������4���ֽڱ�ʾ
	int width = length * pixelByte; //ÿ�е���������
	BYTE* tmp = new BYTE[pixelByte]; //�ݴ�һ�����ص������
	for (int i = 0; i < height; ++i) {
		BYTE* row = imgData + i * width;
		for (int j = 0; j < length / 2; ++j) {
			// ��ȡҪ�������������ص������
			BYTE* pixel1 = row + j * pixelByte;
			BYTE* pixel2 = row + (length - j - 1) * pixelByte;
			memcpy(tmp, pixel1, pixelByte); // ���ݵ�һ�����ص�
			memcpy(pixel1, pixel2, pixelByte); //�õڶ������ص�����ݸ��ǵ�һ�����ص�
			memcpy(pixel2, tmp, pixelByte); //�ñ��ݵ����ݸ��ǵڶ������ص�
		}
	}
	delete[]tmp;
}
//////////////////////////////////////////////////////////////////////////////////////����ͼ///////////////////////////////////////////////////////////////////////////////////////////
struct BitMapData//����ͼ�ṹ
{
	BITMAPFILEHEADER* FileHeader;//�ļ�ͷ14�ֽ�
	tagBITMAPINFOHEADER* InfoHeader;//�ļ���Ϣͷ40�ֽ�
	tagRGBQUAD* colorboard;//��ɫ��
	BYTE* ImageData;//ͼ������
};

tagBITMAPINFOHEADER* get_bitmapinfoheader(DWORD height, DWORD length, DWORD Xsize, DWORD Ysize)//���������γɱ���ͼ���ļ�ͷ
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

void create_bitmap(LPCWSTR filename, BYTE* data, DWORD height, DWORD length,DWORD Xsize,DWORD Ysize)//������ͼƬΪ��alphͨ����32λɫ�����ͼ
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
//////////////////////////////////////////////////////////////////////////////////////hg3ͼƬ//////////////////////////////////////////////////////////////////////////////////////
struct Hg3Head//�ļ�ͷ
{
	BYTE FileType[4];//�ļ�������HG-3
	DWORD HeadSize;//�ļ�ͷ������Ϊ12�ֽ�
	DWORD Unknown;//δֵ֪����Ϊ0x3000
};

struct BlockHead//���ݿ�ͷ
{
	DWORD BlockOffset;//��һ��������ڵ�ǰ���ƫ��
	DWORD BlockFlag;//��ı�ʶ(�൱�ڿ��id),������ݿ��id����������һ��ͼ
};

struct SegmentHhead //���ݶ�ͷ
{
	char Lable[8]{};//���ݶα�ǩ��������
	DWORD SegmentOffset;//�¸����ݶ�ƫ��
	DWORD SegmentLength;//���ݶγ�
};

struct SegmentStdinfo//std����ͷ
{
	DWORD pLength;//ͼ��
	DWORD pHeight;//ͼ��
	DWORD pDepth;//ͼ���
	DWORD CanvasOffsetX;//�뻭��ƫ��xֵ
	DWORD CanvasOffsetY;//�뻭��ƫ��yֵ
	DWORD CanvasLength;//��������
	DWORD CanvasHeight;//�������
	DWORD pFlag;//ͼƬ��ʶ���Ƿ�Ϊ������0�����ǣ�1�����ǣ�
	DWORD BaseX;//��׼x����(ͼƬλ�òο�ֵ)
	DWORD BaseY;//��׼y���꣨ͼƬλ�òο�ֵ)
};
struct  SegmentImage_jpg
{
	BYTE* JpgData=nullptr;//jpgͼƬ�����ݣ������ļ�ͷ��
};

struct  SegmentImage_al
{
	DWORD CompressLength;//��ѹǰ��Alphaͨ�����ݳ�
	DWORD UnCompressLength;//��ѹ���Alphaͨ�����ݳ�����С����������Ŀ��
	BYTE* Alpha;//Alphaͨ������,zlibѹ��
};

struct  SegmentImage0000
{
	DWORD Unkonwn;//δ֪��Ϊ0
	DWORD pHeight;//ͼƬ���
	DWORD dCompressLength;//�������ѹǰ��С
	DWORD dUnCompressLength;//�������ѹ���С
	DWORD mCompressLength;//���ݱ��ѹǰ��С
	DWORD mUnCompressLength;//���ݱ��ѹ���С
	BYTE* Data=nullptr;//������ѹ������
	BYTE* Map=nullptr;//���ݱ�ѹ������
};

struct  SegmentAst00xx//δ֪�ֶ�
{
	DWORD u1;
	DWORD u2;
	DWORD u3;
	DWORD u4;
};

struct  SegmentCptype//Cptype�ֶ�
{
	DWORD Unkonwn;//��Ϊ0
};

struct  SegmentImagemode//SegmentImagemode�ֶ�
{
	DWORD Unkonwn;//��Ϊ0
};

////////////////////////////////////////////////////////////////////////////////////jpgͼƬ//////////////////////////////////////////////////////////////////////////////////////
BYTE* jpg_to_colordata(BYTE* jpgdata, BYTE* alphadata, DWORD jpgdatasize, DWORD height, DWORD length)
{
	//jpgͼ��ת��ɫ���ݣ�alphadataͨ��ֵΪnullptr����תΪ4�ֽڣ�ֻ�ǽ�͸��ͨ����ɫ��Ϊ���ֵ������͸��
	// �Ӷ��������ݴ���Mat����
	//cv::Mat(jpg_data)��Ҫ֪�����ݳ��ȣ����������ֽ��������޷���֪���ȵġ�������Ҫ�����ݷ��������н���
	std::vector<uchar> jpg_data;
	for (int i = 0; i < jpgdatasize; i++)
		jpg_data.push_back(jpgdata[i]);
	cv::Mat image = cv::imdecode(cv::Mat(jpg_data), cv::IMREAD_COLOR);//ת��������ɫ
	BYTE* data = image.data;
	if (!image.data)
	{
		printf_s("����jpgͼ�����ʧ��\\n");
		return nullptr;
	}
	BYTE* result = new BYTE[height * length * 4]{};
	for (int i = 0; i < height * length; i++)//����͸��ͨ��
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
////////////////////////////////////////////////////////////////////////////////////////////////pngͼƬ///////////////////////////////////////////////////////////////////////
BYTE* png_to_colordata(LPWSTR filename,DWORD&rheight ,DWORD&rlength)//pngͼƬתΪrgb��ɫ����
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
	if (image.channels() == 3)//�ж�ͨ���������Ƿ���͸��ɫ
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
	cv::Mat img = cv::Mat(height, length, CV_8UC4, data);	// ����һ��Mat����������ͼ�����ݣ�Ϊ�˴����͸���ȵ�PNGʹ��CV_8UC4
	cv::flip(img, img, 0);// ��תͼƬ��OpenCV��y��������������µģ���λͼͼ��������y�����������ϣ����෴��
	char* w = Wide_To_Char(filename);// ���ͼƬ��һ��.png�ļ���
	cv::imwrite(w, img);
}
