#include"ECways.h"
#pragma comment(lib,"ECways.lib")

char* Shift_Jis_to_GBK(const char* str)
{
	WCHAR* wide;
	int i = MultiByteToWideChar(932, 0, str, -1, NULL, 0);//��ȡstr����Ҫ���ֽڿռ�Ĵ�С
	wide = new WCHAR[i + 1];//����ռ�
	MultiByteToWideChar(932, 0, str, -1, wide, i);//��str�е��ַ�װ����ֽ�����wide
	char* result;
	i = WideCharToMultiByte(936, 0, wide, -1, NULL, 0, NULL, NULL);//��ȡstr����Ҫ�ֽڿռ�Ĵ�С
	result = new char[i + 1];//����ռ�
	WideCharToMultiByte(936, 0, wide, -1, result, i, NULL, NULL);//��str�е��ַ�װ���ֽ�����wide
	return result;//����
}

int main()
{
	//����Ѱ���ļ��ṹ
	WIN32_FIND_DATAA FindData;

	//�ж��ļ��Ƿ����
	if ( GetFileAttributesA("Scene")== INVALID_FILE_ATTRIBUTES)
	{
		MessageBoxA(nullptr, "���󣺵�ǰĿ¼��û��Scene�ļ���", nullptr, 0);
		return 0;
	}

	//��ָ��Ŀ¼�µ������ļ�
	HANDLE hFind = FindFirstFileA("Scene//*.cst", &FindData);

	//�ж��Ƿ��о籾�ļ�
	if (hFind == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(nullptr, "����SceneĿ¼��û��cst�籾�ļ�", nullptr, 0);
		return 0;
	}

	//�����ļ���
	CreateDirectoryA("CstUnpackTxT", nullptr);
	CreateDirectoryA("CstUnpackInI", nullptr);

	//�����ļ���·��
	CHAR TxtPath[] = "CstUnpackTxT//";
	CHAR IniPath[] = "CstUnpackInI//";
	CHAR ScenePath[] = "Scene//";

	//ѭ����������
	do
	{
		// �����ļ���
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//�����ȷcst�ļ�·��,txt·��,ini·��
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

			//��cst�ļ�
			HANDLE hCst = CreateFileA(cPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			//����ini�ļ�
			HANDLE hIni = CreateFileA(iPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			//����txt�ļ�
			HANDLE hTxt = CreateFileA(tPath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			//�����ļ�flag
			SetFilePointer(hCst, 8, nullptr, FILE_CURRENT);

			//��ȡ�ļ���ѹǰ������ݴ�С
			DWORD UncompressedSize;		DWORD CompressedSize;
			ReadFile(hCst, &CompressedSize, 4, nullptr, nullptr);
			ReadFile(hCst, &UncompressedSize, 4, nullptr, nullptr);

			//��ȡzlibѹ������
			BYTE* cData = new BYTE[CompressedSize];
			ReadFile(hCst, cData, CompressedSize, nullptr, nullptr);
			CloseHandle(hCst);

			//��ѹ����
			REdata* Data = ECways::dUncompress_Zlib(cData, CompressedSize, UncompressedSize);
			delete[] cData;


			//��ȡ�ļ�����λ��
			DWORD Dst;
			memcpy(&Dst, Data->data + 12, 4);

			//��¼���������ݣ������ļ�ͷ�������ƫ�Ʊ�
			WriteFile(hIni, Data->data, Dst + 16, nullptr, nullptr);

			//��λ����λ��
			BYTE* Ptr = Data->data + Dst + 16;

			//�Ѷ�����
			DWORD HashRead = Dst + 16;

		   //�س�����
			BYTE Enter =10;

			//��ȡ���о籾
			while (HashRead < Data->size)
			{
				//�ж���ָ������ ��Ϊ�籾����������������ת���ж�
				DWORD Code = *(Ptr + 1);Ptr += 2;
				bool IfScene = (Code==0x21||Code==0x20);

				if (Code==0x30)
				{
					//����ѡ��֧��ָ�����ͺ�ĵ�һ���ֽ�һ����0-9��Χ�ڵ������Ҿ�����֤����ѡ��ָ֧�����ݵ�һ���ֽڵķ�ΧΪ0-9�����ִ���ڼ���ѡ��֧
					//���Ǳ��⣬��ָ������Ϊ��scene ���⡱�������и�ָ������ݵ�4���ֽ�Ϊn��ǰ��3�ֽ���ÿ���ֽھ������ڱ���ָ������������
					//*(Ptr +3 ) == 'n':�����ַ����Ƚ��Ƿ�Ϊ"scene"���õ�4���ֽ����Ƿ�Ϊ'n'��Ϊ�����Ч��
					if ((*Ptr>= '0' && *Ptr <= '9')|| *(Ptr +3 ) == 'n')
					{
						IfScene = true;
						Code = 0xff;//����ָ�����ͣ���0xff����Ϊѡ��֧���߱��⣩
					}
				}

				//�����ʶ���ͣ�2�ֽڡ�����0x30Ҳ�п��ܻ�����Ҫ���������ݣ�
				//��˽���Ҫ������0x30��ʶ����ΪCode,�ڻ�ԭΪcst�ļ�ʱֻ��Ҫ��0xff��ʶ�ָ�Ϊ0x30���ɣ�
				CHAR Type[2];
				Type[0] = 1; Type[1] = Code;
				WriteFile(hIni, Type, 2, nullptr, nullptr);


				//��¼�籾��������ָ��ȣ�����ָ������������ֻ�����ʾ������飩
				DWORD Count = 0;

				while (true)
				{
					Count++;
					if (!(*Ptr))//�ﵽ����ʱ����0��ʶ������
					{
						Ptr++;


						if (IfScene)//д����Ҫ���������ݵ�txt
						{
							//�����ı���ת��Ϊ���ı���
							char* TransCode= Shift_Jis_to_GBK((char*)(Ptr - Count));
							WriteFile(hTxt, TransCode, strlen(TransCode), nullptr, nullptr);
						
							//д��س�����
							WriteFile(hTxt, &Enter, 1, nullptr, nullptr);

							delete[] TransCode;
						}
						else//д�벻��Ҫ���������ݵ�ini
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
	} while (FindNextFileA(hFind, &FindData));//������һ��cst�ļ�

	return 0;
}