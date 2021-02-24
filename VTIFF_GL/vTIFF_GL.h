#pragma once
#include"glew.h"
#include<GL/GL.h>
#include<GL/GLU.h>
#include<vTIFF.h>
class vTIFF_GL:private vTIFF
{
private:
	GLint   inFormat;//�Ƽ�ʹ�õ��ڲ���ʽ
	GLenum  exFormat;
	GLenum  exType;
	void* pData;
	void SetFormat();
public:
	GLint InFormat() { return inFormat; }
	GLsizei Wid() { return Width(); }
	GLsizei Hig() { return Height(); }
	GLenum ExFormat() { return exFormat; }
	GLenum ExType() { return exType; }
	void* GetData() { return GetLayer(0); }
	vTIFF_GL();
	vTIFF_GL(std::string path);
	~vTIFF_GL();
	bool LoadFile(std::string path);
	
};

