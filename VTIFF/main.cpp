#include<iostream>
#include"vTIFF.h"
#include "vTIFF.h"
using namespace std;
int main()
{
	//��ȡvTIFF�ļ�
	vTIFF* tiff = new vTIFF("D:\\Projects\\Unity\\Streamlines\\Assets\\Streamlines\\Data\\1.tif");
	
	//��ȡ��0ҳ�����ݡ����ض�ά�ֽ�����
	byte** layer0 = tiff->GetLayer(0);
	cout<< tiff->Height()<<endl;
	cout << tiff->Width() << endl;
	cout << tiff->Layer() << endl;
	cout << tiff->Pass() << endl;
	//��ȡĳһ���ĳ�����ص����ݡ�
	//for (int x = 0; x < 1440; x++)
	//{
		float* color = tiff->GetPixel(720, 360);
		cout << color[0] << "	" << color[1] << "	" << color[2] << "	" << color[3] << endl;
	//}
	
	delete tiff;
	return 0;
}