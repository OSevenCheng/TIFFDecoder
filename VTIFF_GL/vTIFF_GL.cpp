#include "vTIFF_GL.h"
vTIFF_GL::vTIFF_GL()
{

}
vTIFF_GL::vTIFF_GL(std::string path)
{
	//LoadFile(path);
}
vTIFF_GL::~vTIFF_GL() 
{

}
bool vTIFF_GL::LoadFile(std::string path)
{
	if (!Load(path))
	{
		return false;//�ļ�������
	}
	SetFormat();
	return true;
}
void vTIFF_GL::SetFormat()
{
	int channelNum = Pass();
	vFormat f = Format();
	if (channelNum == 1)
	{

	}
	else if (channelNum == 3)
	{
		switch (f)
		{
		case vFormat::VT_UNSIGNED_BYTE:
			inFormat = GL_RGB8;//�޷��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_UNSIGNED_BYTE;
			break;
		case vFormat::VT_UNSIGNED_SHORT:
			inFormat = GL_RGB16;//�޷��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_UNSIGNED_SHORT;
			break;
		case vFormat::VT_UNSIGNED_INT:
			inFormat = GL_RGB32UI;//�޷��Ų���һ��
			exFormat = GL_RGB_INTEGER;//����һ��
			exType = GL_UNSIGNED_INT;
			break;
		case vFormat::VT_BYTE:
			inFormat = GL_RGB8_SNORM;//�з��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_UNSIGNED_BYTE;
			break;
		case vFormat::VT_SHORT:
			inFormat = GL_RGB16_SNORM;//�з��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_UNSIGNED_SHORT;
			break;
		case vFormat::VT_INTE:
			inFormat = GL_RGB32I;//�з��Ų���һ��
			exFormat = GL_RGB_INTEGER;//����һ��
			exType = GL_UNSIGNED_INT;
			break;
		case vFormat::VT_HALF_FLOAT:
			inFormat = GL_RGB16F;//�з��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_HALF_FLOAT;
			break;
		case vFormat::VT_FLOAT:
			inFormat = GL_RGB32F;//�з��Ź�һ��
			exFormat = GL_RGB;//��һ��
			exType = GL_FLOAT;
			break;
		case vFormat::VT_DOUBLE:
			inFormat = GL_RGB32F;//���32
			exFormat = GL_RGB;//��һ��
			exType = GL_DOUBLE;
			break;
		}
	}
	else if (channelNum == 4)
	{
		switch (f)
		{
		case vFormat::VT_UNSIGNED_BYTE:
			inFormat = GL_RGBA8;//�޷��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_UNSIGNED_BYTE;
			break;
		case vFormat::VT_UNSIGNED_SHORT:
			inFormat = GL_RGBA16;//�޷��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_UNSIGNED_SHORT;
			break;
		case vFormat::VT_UNSIGNED_INT:
			inFormat = GL_RGBA32UI;//�޷��Ų���һ��
			exFormat = GL_RGBA_INTEGER;//����һ��
			exType = GL_UNSIGNED_INT;
			break;
		case vFormat::VT_BYTE:
			inFormat = GL_RGBA8_SNORM;//�з��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_UNSIGNED_BYTE;
			break;
		case vFormat::VT_SHORT:
			inFormat = GL_RGBA16_SNORM;//�з��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_UNSIGNED_SHORT;
			break;
		case vFormat::VT_INTE:
			inFormat = GL_RGBA32I;//�з��Ų���һ��
			exFormat = GL_RGBA_INTEGER;//����һ��
			exType = GL_UNSIGNED_INT;
			break;
		case vFormat::VT_HALF_FLOAT:
			inFormat = GL_RGBA16F;//�з��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_HALF_FLOAT;
			break;
		case vFormat::VT_FLOAT:
			inFormat = GL_RGBA32F;//�з��Ź�һ��
			exFormat = GL_RGBA;//��һ��
			exType = GL_FLOAT;
			break;
		case vFormat::VT_DOUBLE:
			inFormat = GL_RGBA32F;//���32
			exFormat = GL_RGBA;//��һ��
			exType = GL_DOUBLE;
			break;
		}
	}
}