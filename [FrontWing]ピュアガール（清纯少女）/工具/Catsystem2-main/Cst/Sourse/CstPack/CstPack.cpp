#include"ECways.h"
#pragma comment(lib,"ECways.lib")

int main()
{
	//����Ѱ���ļ��ṹ
	WIN32_FIND_DATAA FindData;

	//�ж��ļ����Ƿ����
    BOOL Result = GetFileAttributesA("CstUnpackTxT")!= INVALID_FILE_ATTRIBUTES && GetFileAttributesA("CstUnpackInI")!= INVALID_FILE_ATTRIBUTES;
	if (!Result)
	{
		MessageBoxA(nullptr,"���󣺵�ǰĿ¼��û��CstUnpackTxt����CstUnpackInI�ļ���",nullptr,0);
		return 0;
	}

	//��ָ��Ŀ¼�µ������ļ�
	HANDLE hFind = FindFirstFileA("CstUnpackTxT//*.txt", &FindData);

	//�ж��Ƿ��н�ѹ��ľ籾�ļ�
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(nullptr, "����CstUnpackTxtĿ¼��û��txt�籾�ļ�", nullptr, 0);
		return 0;
	}

	//�����ļ���
	CreateDirectoryA("Scene", nullptr);

	//�����ļ���·��
	CHAR TxtPath[] = "CstUnpackTxT//";
	CHAR IniPath[] = "CstUnpackInI//";
	CHAR CstPath[] = "Scene//";

	//ѭ����������
	do
	{
		// �����ļ���
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//�����ȷcst�ļ�·��,txt·��,ini·��
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
			
			
			//��ini�ļ�
			HANDLE hIni = CreateFileA(iPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//��txt�ļ�
			HANDLE hTxt = CreateFileA(tPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			//�����ж�
			if (hIni == INVALID_HANDLE_VALUE || hTxt == INVALID_HANDLE_VALUE)
			{
				MessageBoxA(nullptr, "����CstUnpackTxt�ļ��е�txt�籾��CstUnpackInI�ļ��е����÷�һһ��Ӧ��ȱ������ini�ļ�����txt�ļ�", nullptr, 0);
				if (hIni != INVALID_HANDLE_VALUE)
					CloseHandle(hIni);
				if(hTxt!= INVALID_HANDLE_VALUE)
					CloseHandle(hTxt);;
				break;
			}

			//��ȡѹ��ǰ�ļ����ܴ�С
			DWORD IniSize = GetFileSize(hIni, nullptr);
			DWORD TxtSize= GetFileSize(hTxt, nullptr);
			DWORD PreSize = IniSize + TxtSize;
			
			//���ڽ��ָܻ�������
			BYTE* Data = new BYTE[PreSize];
			ReadFile(hIni, Data, 16, nullptr, nullptr);

			//��ȡָ������ƫ���Լ�ƫ�Ʊ��ƫ��λ��
			DWORD Offset; DWORD TableOffset;
			memcpy(&Offset,Data+12,4);
			memcpy(&TableOffset, Data +8, 4);

			//�����ļ���С
			DWORD DataSize = PreSize -16;
			memcpy(Data,&DataSize, 4);

			//��ȡ�����Ĳ���
			ReadFile(hIni, Data+16, Offset, nullptr, nullptr);

			//��¼�Ѷ����ݴ�С
			DWORD HashRead = Offset + 16;

			//dPtr:ָ��ָ����������飺����һ�������ݵ�����
			BYTE* dPtr = Data + HashRead;

			//tPtr:ָ��ָ��ƫ�Ʊ���������ÿ���籾����ָ��ĳ���
			BYTE* tPtr = Data + 16 + TableOffset;
			DWORD tOffset = 0;//ָ����ڳ��ȱ��ƫ��ֵ����ʼΪ0

			//���ڴ���ָ�
			BYTE* dIni = new BYTE[IniSize-HashRead];
			ReadFile(hIni, dIni, IniSize - HashRead, nullptr, nullptr);
			CloseHandle(hIni);
			BYTE* Command = dIni;//������ָ��ʹ��

			//���ڴ���txt������
			BYTE* dTxt = new BYTE[TxtSize];

			//����txt�ļ�����
			ReadFile(hTxt, dTxt, TxtSize, nullptr, nullptr);
			CloseHandle(hTxt);
			BYTE* Scene = dTxt;//������ָ��ʹ��

			//ѭ���ָ�����
			while (HashRead < IniSize)
			{
				//��ȡָ������
				*dPtr = *Command; Command++;
				
				//ֻ��0x30ָ����ʵ��Ϊ�������ѡ��֧ʱ�Ż��ڵ���ʱ����Ϊ0xff�����0xff��Ҫ�������������Ѿ�װ�뵽txt��
				//�ж��Ƿ�Ϊ��Ҫ����������
				bool IfScene = (*Command ==0xff|| *Command==0x20|| *Command==0x21);

				//��ԭ0xffָ��Ϊ0x30
				*(dPtr+1) =(*Command==0xff?0x30: *Command);	Command++;

				HashRead += 2;	dPtr += 2;

				//ע�⣺��Ҫ���������ݱ�����txt���ݡ����ļ�ͷ������ƫ�Ʊ�ָ�����ͣ�2�ֽڣ����Ǿ籾ָ�����ݾ�������ini�ļ���
				if (IfScene)//����Ҫ���������ݣ�����ָ�����ݴ�txt���ݶ�Ӧ�����ȡ
				{
					//�籾����
					DWORD SceneSize = 0;
					while (true)
					{
						SceneSize++;
						if (*Scene == '\n')//�ﵽ����ʱ����0��ʶ������
						{
							Scene++;

							//��¼�Ǿ籾ָ��
							memcpy(dPtr, Scene - SceneSize, SceneSize);
							dPtr += SceneSize;

							//��\n�滻Ϊ0
							*(dPtr - 1) = 0;

							//����ƫ�Ʊ�
							memcpy(tPtr, &tOffset, 4);
							tOffset += SceneSize + 2;

							//�ƶ�ָ��
							tPtr += 4;

							break;
						}
						Scene++;
					}
				}
				else 	//�Ǿ籾ָ�����ָ�����ݴ�ini�ļ��ڶ�ȡ
				{
					//ָ���
					DWORD CommandSize = 0;
					while (true)
					{
						CommandSize++;
						if (!(*Command))//�ﵽ����ʱ����0��ʶ������
						{
							Command++;
							//��¼�Ǿ籾ָ��
							memcpy(dPtr, Command - CommandSize, CommandSize);

							//������ƫ�Ʊ��ƫ��λ��
							memcpy(tPtr, &tOffset, 4);
							tOffset += CommandSize + 2;

							//�ƶ�ָ��
							tPtr += 4;
							dPtr += CommandSize;
							HashRead += CommandSize;

							break;
						}
						Command++;
					}
				}
			}
			//��ָ���ƶ�����ʼλ��
			delete[] dIni;
			delete[] dTxt;
			//ѹ��
			REdata* CompressedData=ECways::dCompress_Zlib(Data, PreSize,9);
			delete[] Data;

			//����cst�ļ�
			HANDLE hCst = CreateFileA(cPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			//д���ʶ
			WriteFile(hCst, "CatScene", 8, nullptr, nullptr);

			//д��ѹ��������ݴ�С
			WriteFile(hCst, &CompressedData->size, 4, nullptr, nullptr);

			//д��ѹ��ǰ�����ݴ�С
			WriteFile(hCst, &PreSize,4, nullptr, nullptr);

			//д������
			WriteFile(hCst, CompressedData->data, CompressedData->size,nullptr, nullptr);

			delete CompressedData;
			CloseHandle(hCst);		
		}
	} while (FindNextFileA(hFind, &FindData));//������һ��txt�ļ�

	return 0;
}
