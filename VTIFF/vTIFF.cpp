#include "vTIFF.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
using namespace std;

void vTIFF::Load(string path)
{
    pGetInt[1] = GetIntII;
    pGetInt[0] = GetIntMM;

    if (!ReadFile(path))
    {
        return;
    }
    int pIFD = DecodeIFH();
    while (pIFD != 0)
    {
		vIFD* p_ifd = new vIFD(p_data,ByteOrder, pGetInt[ByteOrder]);
        pIFD = p_ifd->Decode(pIFD);
        //p_ifd->PrintInfo();
        p_IFDs.push_back(p_ifd);
    }
}
void vTIFF::Unload()//ֻ�ǰ�ԭʼ�����ͷ�//������Ҫ�������ݣ�GetLayer���󼴿��ͷŵ���
{
    if (p_data != nullptr)
    {
        delete p_data;
        p_data = nullptr;
    }
        
}
int vTIFF::DecodeIFH()
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
        cout << "Not vTIFF."<<endl;

    return pGetInt[ByteOrder](p_data, 4, 4);
}

bool vTIFF::ReadFile(std::string path)
{
	ifstream f;   // ��ȡͼ��
	f.open(path, ios::in | ios::binary);

	f.seekg(0, ios::end);   // ���ļ�ָ���ƶ����ļ�ĩβ
	int length = f.tellg();   // �����ļ�ָ���λ��
    if (length == -1)
    {
        cout << "�ļ�·������" << length << endl;
        return false;
    }
	
	f.seekg(0, ios::beg);
	//����ͼ�����ݳ��ȷ����ڴ�buffer
    p_data = new byte[length];
	f.read((char*)p_data, length);

	f.close();
    return true;
}

byte* vTIFF::GetLayer(int i=0)
{
    p_IFDs[i]->DecodeStrips();
    return p_IFDs[i]->GetImageData();
}
float* vTIFF::GetPixel(int x, int y, int layer)
{
    return p_IFDs[layer]->GetPixel(x, y);
}