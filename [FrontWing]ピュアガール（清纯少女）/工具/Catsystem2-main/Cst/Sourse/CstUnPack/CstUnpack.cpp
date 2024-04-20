#include"ECways.h"
#pragma comment(lib,"ECways.lib")

char* Shift_Jis_to_GBK(const char* str)
{
	WCHAR* wide;
	int i = MultiByteToWideChar(932, 0, str, -1, NULL, 0);//获取str所需要宽字节空间的大小
	wide = new WCHAR[i + 1];//申请空间
	MultiByteToWideChar(932, 0, str, -1, wide, i);//将str中的字符装入宽字节数组wide
	char* result;
	i = WideCharToMultiByte(936, 0, wide, -1, NULL, 0, NULL, NULL);//获取str所需要字节空间的大小
	result = new char[i + 1];//申请空间
	WideCharToMultiByte(936, 0, wide, -1, result, i, NULL, NULL);//将str中的字符装入字节数组wide
	return result;//返回
}

int main()
{
	//储存寻找文件结构
	WIN32_FIND_DATAA FindData;

	//判断文件是否存在
	if ( GetFileAttributesA("Scene")== INVALID_FILE_ATTRIBUTES)
	{
		MessageBoxA(nullptr, "错误：当前目录下没有Scene文件夹", nullptr, 0);
		return 0;
	}

	//打开指定目录下的所有文件
	HANDLE hFind = FindFirstFileA("Scene//*.cst", &FindData);

	//判断是否有剧本文件
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(nullptr, "错误：Scene目录下没有cst剧本文件", nullptr, 0);
		return 0;
	}

	//创建文件夹
	CreateDirectoryA("CstUnpackTxT", nullptr);
	CreateDirectoryA("CstUnpackInI", nullptr);

	//填入文件夹路径
	CHAR TxtPath[] = "CstUnpackTxT//";
	CHAR IniPath[] = "CstUnpackInI//";
	CHAR ScenePath[] = "Scene//";

	//循环导出配置
	do
	{
		// 忽略文件夹
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//获得正确cst文件路径,txt路径,ini路径
			CHAR cPath[128]{};
			lstrcatA(cPath, ScenePath);
			lstrcatA(cPath, FindData.cFileName);

			BYTE cPathLength = strlen(cPath);

			CHAR iPath[128]{};
			lstrcatA(iPath, IniPath);
			lstrcatA(iPath, FindData.cFileName);
			iPath[cPathLength + 4] = 'i'; iPath[cPathLength + 5] = 'n'; iPath[cPathLength + 6] = 'i';

			CHAR tPath[128]{};
			lstrcatA(tPath, TxtPath);
			lstrcatA(tPath, FindData.cFileName);
			tPath[cPathLength + 4] = 't'; tPath[cPathLength + 5] = 'x'; tPath[cPathLength + 6] = 't';

			//打开cst文件
			HANDLE hCst = CreateFileA(cPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//创建ini文件
			HANDLE hIni = CreateFileA(iPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			//创建txt文件
			HANDLE hTxt = CreateFileA(tPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			//跳过文件flag
			SetFilePointer(hCst, 8, nullptr, FILE_CURRENT);

			//读取文件解压前后的数据大小
			DWORD UncompressedSize;		DWORD CompressedSize;
			ReadFile(hCst, &CompressedSize, 4, nullptr, nullptr);
			ReadFile(hCst, &UncompressedSize, 4, nullptr, nullptr);

			//读取zlib压缩数据
			BYTE* cData = new BYTE[CompressedSize];
			ReadFile(hCst, cData, CompressedSize, nullptr, nullptr);
			CloseHandle(hCst);

			//解压数据
			REdata* Data = ECways::dUncompress_Zlib(cData, CompressedSize, UncompressedSize);
			delete[] cData;


			//获取文件正文位置
			DWORD Dst;
			memcpy(&Dst, Data->data + 12, 4);

			//记录非正文数据（包含文件头和语句表和偏移表）
			WriteFile(hIni, Data->data, Dst + 16, nullptr, nullptr);

			//定位正文位置
			BYTE* Ptr = Data->data + Dst + 16;

			//已读长度
			DWORD HashRead = Dst + 16;

		   //回车符号
			BYTE Enter =10;

			//读取所有剧本
			while (HashRead < Data->size)
			{
				//判断是指令类型 否为剧本或者人名，将用于转码判断
				DWORD Code = *(Ptr + 1);Ptr += 2;
				bool IfScene = (Code==0x21||Code==0x20);

				if (Code==0x30)
				{
					//若是选择支其指令类型后的第一个字节一定是0-9范围内的数，且经过验证仅有选择支指令内容第一个字节的范围为0-9：数字代表第几个选择支
					//若是标题，其指令内容为“scene 标题”，而仅有该指令的内容第4个字节为n。前面3字节中每个字节均不是在标题指令中特征出现
					//*(Ptr +3 ) == 'n':不用字符串比较是否为"scene"而用第4个字节身是否为'n'是为了提高效率
					if ((*Ptr>= '0' && *Ptr <= '9')|| *(Ptr +3 ) == 'n')
					{
						IfScene = true;
						Code = 0xff;//更改指令类型（将0xff定义为选择支或者标题）
					}
				}

				//保存标识类型（2字节。由于0x30也有可能会有需要汉化的内容，
				//因此将需要汉化的0x30标识更改为Code,在还原为cst文件时只需要将0xff标识恢复为0x30即可）
				CHAR Type[2];
				Type[0] = 1; Type[1] = Code;
				WriteFile(hIni, Type, 2, nullptr, nullptr);


				//记录剧本或者其它指令长度（其它指令包括播放音乐或者显示人物表情）
				DWORD Count = 0;

				while (true)
				{
					Count++;
					if (!(*Ptr))//达到结束时（以0标识结束）
					{
						Ptr++;


						if (IfScene)//写入需要汉化的内容到txt
						{
							//将日文编码转化为中文编码
							char* TransCode= Shift_Jis_to_GBK((char*)(Ptr - Count));
							WriteFile(hTxt, TransCode, strlen(TransCode), nullptr, nullptr);
						
							//写入回车符号
							WriteFile(hTxt, &Enter, 1, nullptr, nullptr);

							delete[] TransCode;
						}
						else//写入不需要汉化的内容到ini
						{
							WriteFile(hIni, Ptr - Count, Count, nullptr, nullptr);
						}
						break;
					}
					Ptr++;
				}
				HashRead += Count + 2;
			}
			delete Data;
			CloseHandle(hIni);
			CloseHandle(hTxt);
		}
	} while (FindNextFileA(hFind, &FindData));//搜索下一个cst文件

	return 0;
}