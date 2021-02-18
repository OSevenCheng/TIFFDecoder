#include "TIFF.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
using namespace std;


int GetIntII(byte* pd, int startPos, int Length)
{
    int value = 0;
    for (int i = 0; i < Length; i++)
        value |= pd[startPos + i] << i * 8;
    return value;
}

int GetIntMM(byte* pd, int startPos, int Length)
{
    int value = 0;
    int st = startPos + Length - 1;
    for (int i = 0; i < Length; i++)
        value |= pd[st - i] << i * 8;
    return value;
}
void TIFF::Init(string path)
{
    pGetInt[1] = GetIntII;
    pGetInt[0] = GetIntMM;

	ReadFile(path);
    int pIFD = DecodeIFH();
    while (pIFD != 0)
    {
		IFD* p_ifd = new IFD(p_data,ByteOrder, pGetInt[ByteOrder]);
        pIFD = p_ifd->Decode(pIFD);
        p_ifd->PrintInfo();
        p_IFDs.push_back(p_ifd);
    }

    delete p_data;
}
int TIFF::DecodeIFH() 
{
    //string byteOrder = p_data[0];// +"" + (char)p_data[1];
    if (p_data[0] == 'I')
        ByteOrder = true;
    else if (p_data[0] == 'M')
        ByteOrder = false;
    else
        cout<<"The order value is not II or MM."<<endl;
    int Version = pGetInt[ByteOrder](p_data,2,2);

    if (Version != 42)
        cout << "Not TIFF."<<endl;

    return pGetInt[ByteOrder](p_data, 4, 4);
}

void TIFF::ReadFile(std::string path)
{
	ifstream f;   // ��ȡͼ��
	f.open(path, ios::in | ios::binary);

	f.seekg(0, ios::end);   // ���ļ�ָ���ƶ����ļ�ĩβ
	int length = f.tellg();   // �����ļ�ָ���λ��
	cout << "ͼ���������ֽ�����" << length << endl;
	f.seekg(0, ios::beg);
	//����ͼ�����ݳ��ȷ����ڴ�buffer
    p_data = new byte[length];
	f.read((char*)p_data, length);

	f.close();
}

byte** TIFF::GetLayer(int i=0)
{
    return p_IFDs[i]->GetImageData();
}
float* TIFF::GetPixel(int x, int y, int layer)
{
    return p_IFDs[layer]->GetPixel(x, y);
}