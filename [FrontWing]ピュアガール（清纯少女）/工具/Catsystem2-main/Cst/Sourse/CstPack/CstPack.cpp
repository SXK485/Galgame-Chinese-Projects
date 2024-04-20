#include"ECways.h"
#pragma comment(lib,"ECways.lib")

int main()
{
	//储存寻找文件结构
	WIN32_FIND_DATAA FindData;

	//判断文件夹是否存在
    BOOL Result = GetFileAttributesA("CstUnpackTxT")!= INVALID_FILE_ATTRIBUTES && GetFileAttributesA("CstUnpackInI")!= INVALID_FILE_ATTRIBUTES;
	if (!Result)
	{
		MessageBoxA(nullptr,"错误：当前目录下没有CstUnpackTxt或者CstUnpackInI文件夹",nullptr,0);
		return 0;
	}

	//打开指定目录下的所有文件
	HANDLE hFind = FindFirstFileA("CstUnpackTxT//*.txt", &FindData);

	//判断是否有解压后的剧本文件
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(nullptr, "错误：CstUnpackTxt目录下没有txt剧本文件", nullptr, 0);
		return 0;
	}

	//创建文件夹
	CreateDirectoryA("Scene", nullptr);

	//填入文件夹路径
	CHAR TxtPath[] = "CstUnpackTxT//";
	CHAR IniPath[] = "CstUnpackInI//";
	CHAR CstPath[] = "Scene//";

	//循环导出配置
	do
	{
		// 忽略文件夹
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//获得正确cst文件路径,txt路径,ini路径
			CHAR cPath[128]{};
			lstrcatA(cPath, CstPath);
			lstrcatA(cPath, FindData.cFileName);

			BYTE cPathLength = strlen(cPath);
			cPath[cPathLength-3] = 'c'; cPath[cPathLength-2] = 's'; cPath[cPathLength-1] = 't';

			CHAR iPath[128]{};
			lstrcatA(iPath, IniPath);
			lstrcatA(iPath, FindData.cFileName);
			iPath[cPathLength + 4] = 'i'; iPath[cPathLength + 5] = 'n'; iPath[cPathLength + 6] = 'i';

			CHAR tPath[128]{};
			lstrcatA(tPath, TxtPath);
			lstrcatA(tPath, FindData.cFileName);
			
			
			//打开ini文件
			HANDLE hIni = CreateFileA(iPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//打开txt文件
			HANDLE hTxt = CreateFileA(tPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			//错误判断
			if (hIni == INVALID_HANDLE_VALUE || hTxt == INVALID_HANDLE_VALUE)
			{
				MessageBoxA(nullptr, "错误：CstUnpackTxt文件夹的txt剧本与CstUnpackInI文件夹的配置非一一对应，缺少配置ini文件或者txt文件", nullptr, 0);
				if (hIni != INVALID_HANDLE_VALUE)
					CloseHandle(hIni);
				if(hTxt!= INVALID_HANDLE_VALUE)
					CloseHandle(hTxt);;
				break;
			}

			//获取压缩前文件的总大小
			DWORD IniSize = GetFileSize(hIni, nullptr);
			DWORD TxtSize= GetFileSize(hTxt, nullptr);
			DWORD PreSize = IniSize + TxtSize;
			
			//用于接受恢复的数据
			BYTE* Data = new BYTE[PreSize];
			ReadFile(hIni, Data, 16, nullptr, nullptr);

			//获取指令正文偏移以及偏移表的偏移位置
			DWORD Offset; DWORD TableOffset;
			memcpy(&Offset,Data+12,4);
			memcpy(&TableOffset, Data +8, 4);

			//修正文件大小
			DWORD DataSize = PreSize -16;
			memcpy(Data,&DataSize, 4);

			//读取非正文部分
			ReadFile(hIni, Data+16, Offset, nullptr, nullptr);

			//记录已读数据大小
			DWORD HashRead = Offset + 16;

			//dPtr:指向恢复的数据数组：用于一个个数据的填入
			BYTE* dPtr = Data + HashRead;

			//tPtr:指向指令偏移表：用于修正每个剧本或者指令的长度
			BYTE* tPtr = Data + 16 + TableOffset;
			DWORD tOffset = 0;//指令对于长度表的偏移值：初始为0

			//用于储存指令集
			BYTE* dIni = new BYTE[IniSize-HashRead];
			ReadFile(hIni, dIni, IniSize - HashRead, nullptr, nullptr);
			CloseHandle(hIni);
			BYTE* Command = dIni;//将用作指针使用

			//用于储存txt的内容
			BYTE* dTxt = new BYTE[TxtSize];

			//读入txt文件内容
			ReadFile(hTxt, dTxt, TxtSize, nullptr, nullptr);
			CloseHandle(hTxt);
			BYTE* Scene = dTxt;//将用作指针使用

			//循环恢复数据
			while (HashRead < IniSize)
			{
				//获取指令类型
				*dPtr = *Command; Command++;
				
				//只有0x30指令中实际为标题或者选择支时才会在导出时修正为0xff，因此0xff需要汉化且内容是已经装入到txt内
				//判断是否为需要汉化的内容
				bool IfScene = (*Command ==0xff|| *Command==0x20|| *Command==0x21);

				//还原0xff指令为0x30
				*(dPtr+1) =(*Command==0xff?0x30: *Command);	Command++;

				HashRead += 2;	dPtr += 2;

				//注意：需要汉化的内容保存在txt内容。而文件头，语句表，偏移表，指令类型（2字节），非剧本指令内容均保存在ini文件中
				if (IfScene)//是需要汉化的内容，具体指令内容从txt内容对应数组读取
				{
					//剧本长度
					DWORD SceneSize = 0;
					while (true)
					{
						SceneSize++;
						if (*Scene == '\n')//达到结束时（以0标识结束）
						{
							Scene++;

							//记录非剧本指令
							memcpy(dPtr, Scene - SceneSize, SceneSize);
							dPtr += SceneSize;

							//将\n替换为0
							*(dPtr - 1) = 0;

							//修正偏移表
							memcpy(tPtr, &tOffset, 4);
							tOffset += SceneSize + 2;

							//移动指针
							tPtr += 4;

							break;
						}
						Scene++;
					}
				}
				else 	//非剧本指令，具体指令内容从ini文件内读取
				{
					//指令长度
					DWORD CommandSize = 0;
					while (true)
					{
						CommandSize++;
						if (!(*Command))//达到结束时（以0标识结束）
						{
							Command++;
							//记录非剧本指令
							memcpy(dPtr, Command - CommandSize, CommandSize);

							//修正长偏移表的偏移位置
							memcpy(tPtr, &tOffset, 4);
							tOffset += CommandSize + 2;

							//移动指针
							tPtr += 4;
							dPtr += CommandSize;
							HashRead += CommandSize;

							break;
						}
						Command++;
					}
				}
			}
			//将指针移动到开始位置
			delete[] dIni;
			delete[] dTxt;
			//压缩
			REdata* CompressedData=ECways::dCompress_Zlib(Data, PreSize,9);
			delete[] Data;

			//创建cst文件
			HANDLE hCst = CreateFileA(cPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			//写入标识
			WriteFile(hCst, "CatScene", 8, nullptr, nullptr);

			//写入压缩后的数据大小
			WriteFile(hCst, &CompressedData->size, 4, nullptr, nullptr);

			//写入压缩前的数据大小
			WriteFile(hCst, &PreSize,4, nullptr, nullptr);

			//写入数据
			WriteFile(hCst, CompressedData->data, CompressedData->size,nullptr, nullptr);

			delete CompressedData;
			CloseHandle(hCst);		
		}
	} while (FindNextFileA(hFind, &FindData));//搜索下一个txt文件

	return 0;
}
