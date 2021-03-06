#include "vIFD.h"
#include "Utils.h"
#include "CompressionLZW.h"
#include <iostream>



vIFD::vIFD(byte* p_data, bool byteorder, int(*fun)(byte* ,int, int) ) :
	p_Data(p_data),
	imageData(nullptr),
	ByteOrder(byteorder),
	/*BitsPerSample(nullptr),
	StripOffsets(nullptr),
	StripByteCounts(nullptr),
	SampleFormat(nullptr),*/
	pGetInt(fun)
{

}
vIFD::~vIFD()
{
	if (imageData != nullptr)
	{
		/*for (int i = 1; i < StripCount; i++)
		{
			delete[] imageData[i];
		}*/
		delete[] imageData;
	}
}
int vIFD::Decode(int Pos)
{
	int n = Pos;
	int DECount = pGetInt(p_Data, n, 2);
	n += 2;
	for (int i = 0; i < DECount; i++)
	{
		DecodeDE(n);
		n += 12;
	}
	ChannelCount = BitsPerSample.size();
	if (ChannelCount == 0)
	{
		//错误
		return 0;
	}
	BytesPerSample = BitsPerSample[0]/8;
	int mo = BitsPerSample[0] % 8;
	if (BytesPerSample == 0 || mo != 0)
	{
		//不支持 通道宽度不为8bits的整数倍
		return 0;
	}
	bool isSame = true;
	if (ChannelCount > 1)
	{
		for (int i = 1; i < ChannelCount; i++)
			isSame = isSame && (BitsPerSample[i] == BitsPerSample[i - 1]);
	}
	if (!isSame)
	{
		//不支持 各通道不一致的情况
		return 0;
	}
	//if(BytesPerSample)
	if (SampleFormat.size() == 0)
	{
		SampleFormat.push_back(1);//默认为1
	}
	
	/*也可以之后获取指定层的时候解码
	//已获得每条扫描线位置，大小，压缩方式和数据类型，接下来进行解码
	DecodeStrips();
	*/
	int pNext = pGetInt(p_Data, n, 4);
	return pNext;
}
void vIFD::DecodeDE(int Pos)
{
	int TagIndex = pGetInt(p_Data, Pos, 2);
	int TypeIndex = pGetInt(p_Data, Pos + 2, 2);
	int Count = pGetInt(p_Data, Pos + 4, 4);
	//Debug.Log("Tag: " + Tag(TagIndex) + " DataType: " + TypeArray[TypeIndex].name + " Count: " + Count);

	//先把找到数据的位置
	int pData = Pos + 8;
	int totalSize = TypeArray[TypeIndex] * Count;
	if (totalSize > 4)
		pData = GetInt(pData, 4);

	//再根据Tag把值读出并存起来
	GetDEValue(TagIndex, TypeIndex, Count, pData);
}
void vIFD::GetDEValue(int TagIndex, int TypeIndex, int Count, int pdata)
{
	int typesize = TypeArray[TypeIndex];
	switch (TagIndex)
	{
	case 254: break;//NewSubfileType
	case 255: break;//SubfileType
	case 256://ImageWidth
		ImageWidth = GetInt(pdata, typesize); break;
	case 257://ImageLength
		ImageLength = GetInt(pdata, typesize); break;
	case 258://BitsPerSample
		GetIntArray(pdata, typesize, Count, BitsPerSample); break;
	case 259: //Compression
		Compression = GetInt(pdata, typesize); break;
	case 262: //PhotometricInterpretation
		PhotometricInterpretation = GetInt(pdata, typesize); break;
	case 273://StripOffsets
		GetIntArray(pdata, typesize, Count, StripOffsets); StripCount = Count; break;
	case 274: //Orientation
		Orientation = GetInt(pdata, typesize);break;
	case 277: break;//SamplesPerPixel
	case 278://RowsPerStrip
		RowsPerStrip = GetInt(pdata, typesize); break;
	case 279://StripByteCounts
		GetIntArray(pdata, typesize, Count, StripByteCounts); break;
	case 282: //XResolution
		XResolution = GetRational(pdata); break;
	case 283://YResolution
		YResolution = GetRational(pdata); break;
	case 284: break;//PlanarConfig
	case 296://ResolutionUnit
		ResolutionUnit = GetInt(pdata, typesize); break;
	case 305://Software
		Software = GetString(pdata, typesize); break;
	case 306://DateTime
		DateTime = GetString(pdata, typesize); break;
	case 315: break;//Artist
	case 317: //Differencing Predictor
		Predictor = GetInt(pdata, typesize); break;
	case 320: break;//ColorDistributionTable
	case 338: break;//ExtraSamples
	case 339: //SampleFormat
		GetIntArray(pdata,typesize,Count, SampleFormat); break;//默认为无符号整型
	default: break;
	}
}
void vIFD::DecodeImage()
{
	int BitsCount = BitsPerSample[0];
	switch (BitsCount)//不管有无符号，都按有符号处理
	{
	case 8://byte
		DecodeStrips<char>();
		break;
	case 16://short
		DecodeStrips<short>();
		break;
	case 32:
		if (SampleFormat[0] == 3)//float
		{
			DecodeStrips<float>();
		}
		else//int
		{
			DecodeStrips<int>();
		}
		break;
	case 64:
		DecodeStrips<double>();
		break;
	default:
		break;
	}
	hasDecode = true;
}
template<class T>
void vIFD::DealPredictor()//RGB
{
	T* sData = (T*)(imageData);
	for (int j = 0; j < ImageLength; j++)
	{
		int before = j * ImageWidth * ChannelCount;
		for (int ch = 0; ch < ChannelCount; ch++)
		{
			char last = 0;
			for (int i = before + ch; i < before + ch + ImageWidth * ChannelCount; i += ChannelCount)
			{
				sData[i] += last;
				last = sData[i];
			}
		}
	}
}
#define REPEAT4(n, op)		\
    switch (n) {		\
    default: { \
        size_t i; for (i = n-4; i > 0; i--) { op; } }  /*-fallthrough*/  \
    case 4:  op; /*-fallthrough*/ \
    case 3:  op; /*-fallthrough*/ \
    case 2:  op; /*-fallthrough*/ \
    case 1:  op; /*-fallthrough*/ \
    case 0:  ;			\
    }
void vIFD::DealPredictor(uint8_t* cp0, size_t cc)//Float
{
	size_t stride = 1;
	uint32_t bps = BytesPerSample;
	size_t wc = cc / bps;
	size_t count = cc;//一条扫描线的byte数
	uint8_t* cp = (uint8_t*)cp0;
	uint8_t* tmp;

	tmp = (uint8_t*)malloc(cc);
	if (!tmp)
		return;
	uint8_t t1 = cp0[0];
	uint8_t t2 = cp0[ImageWidth];
	uint8_t t3 = cp0[ImageWidth*2];
	uint8_t t4 = cp0[ImageWidth *3];
	while (count > stride) 
	{
		//REPEAT4(stride, cp[stride] =(unsigned char)((cp[stride] + cp[0]) & 0xff); cp++)
			
		//REPEAT4(n, op)		
		switch (stride) {

		default: 
		{
			size_t i; 
			for (i = stride - 4; i > 0; i--)
			{ 
				unsigned char x = (unsigned char)((cp[stride] + cp[0]) & 0xff);
				cp[stride] = x;
				cp++; 
			} 
		}  /*-fallthrough*/
		case 4:  cp[stride] = (unsigned char)((cp[stride] + cp[0]) & 0xff); cp++; /*-fallthrough*/
		case 3:  cp[stride] = (unsigned char)((cp[stride] + cp[0]) & 0xff); cp++; /*-fallthrough*/
		case 2:  cp[stride] = (unsigned char)((cp[stride] + cp[0]) & 0xff); cp++; /*-fallthrough*/
		case 1:  cp[stride] = (unsigned char)((cp[stride] + cp[0]) & 0xff); cp++; /*-fallthrough*/
		case 0:;
		}

		count -= stride;
	}

	memcpy(tmp, cp0, cc);
	cp = (uint8_t*)cp0;
	for (count = 0; count < wc; count++) {
		uint32_t byteNum;
		for (byteNum = 0; byteNum < bps; byteNum++) 
		{
			if (ByteOrder)// "II"
			{
				int di = bps * count + byteNum;
				int si = (bps - byteNum - 1) * wc + count;
				uint8_t t = tmp[si];
				cp[di] = t;
			}
			else// "MM"  WORDS_BIGENDIAN
				cp[bps * count + byteNum] = tmp[byteNum * wc + count];
		}
		float* fTemp = reinterpret_cast<float*>(cp0);
		float zz = *fTemp;
	}
	free(tmp);
	return;


	//uint8_t * tmp = (uint8_t*)malloc(cc);
	//uint8_t R[] =  { 0,0,0,0 };
	//uint8_t LastR[] = { cp0[ 0 ], cp0[1 * ImageWidth], cp0[2 * ImageWidth], cp0[3 * ImageWidth] };
	//for (size_t x = 1; x < ImageWidth * RowsPerStrip; x++)
	//{
	//    for (int c = 0; c < 4; c++)
	//    {
	//        R[c] = (uint8_t)( (cp0[x + c * ImageWidth] + LastR[c] ) & 0xff);
	//        LastR[c] = R[c];
	//        //R[c] = Dval[x + c * ImageWidth];

	//    }
	//	uint8_t byteTemp[4] = { R[3], R[2], R[1], R[0] };
	//	tmp[x * 4 + 0] = R[3];
	//	tmp[x * 4 + 1] = R[2];
	//	tmp[x * 4 + 2] = R[1];
	//	tmp[x * 4 + 3] = R[0];
	//		float* fTemp = reinterpret_cast<float*>(byteTemp);
	//		float zz = *fTemp;
	//}
	//memcpy(cp0, tmp, cc);
	//free(tmp);
}

template<class T>
void vIFD::DecodeStrips()
{
	int pStrip = 0;
	int size = 0;
	byteCountPerStripe = ImageWidth * RowsPerStrip * ChannelCount * BitsPerSample[0]/8;//解码后的宽度
	imageData = new byte[StripCount* byteCountPerStripe];
	int endi = StripCount - 1;
	auto orientation1= [](int i, int endi) { return endi - i; };
	auto orientation4 = [](int i, int endi) { return i; };
	int (*outputIndex)(int i, int endi);
	
	if (Orientation <= 2)//目前只考虑1和4两种情况
	{
		outputIndex = orientation1;
	}
	else
	{
		outputIndex = orientation4;
	}
	if (Compression == 1)//No compression
	{
		
		for (int i = 0; i < StripCount; i++)
		{
			pStrip = StripOffsets[i];
			size = StripByteCounts[i];
			//imageData[i * byteCountPerStripe] = p_Data[pStrip];
			memcpy(&imageData[outputIndex(i, endi) * byteCountPerStripe], &p_Data[pStrip], size);
		}
	}
	else if (Compression == 5)//LZW compression
	{
		//Timer time("LZW Decode Time");
		CompressionLZW* lzw = new CompressionLZW();
		for (int i = 0; i < StripCount; i++)
		{
			pStrip = StripOffsets[i];
			size = StripByteCounts[i];
			byte* outputpos = &imageData[outputIndex(i, endi) * byteCountPerStripe];
			lzw->Decode(p_Data,pStrip,size, outputpos);
			DealPredictor(outputpos, byteCountPerStripe);
			float* x = reinterpret_cast<float*>(outputpos);
			float* y = x+1;
			float* z = x + 2;
		}
		delete lzw;
		/*if (Predictor == 2)
		{
			DealPredictor<T>();
		}
		else if (Predictor == 3)
		{
			DealPredictor();
		}*/
	}
		
}

void* vIFD::GetPixel(int x, int y)
{
	//int f = SampleFormat[0];
	int b = BitsPerSample[0] / 8;
	int c = BitsPerSample.size();
	void* color = nullptr;
	auto func_Char = [](byte* data,int x,int y, int c,int w) {
		char* sData = (char*)(data);
		return (void*)&sData[x + y * w * c];
	};
	auto func_Short = [](byte* data, int x, int y, int c, int w) {
		short* sData = (short*)(data);
		return (void*)&sData[x + y * w * c];
	};
	auto func_Int = [](byte* data, int x, int y, int c, int w) {
		int* sData = (int*)(data);
		return (void*)&sData[x + y * w * c];
	};
	void* (*func_ptr[4])(byte * data, int x, int y ,int c, int w)
		= { func_Char, func_Short, 0 , func_Int };
	return func_ptr[b - 1](imageData, x, y, c, ImageWidth);
}
//float* vIFD::GetPixel(int x, int y)
//{
//	float* color = nullptr;
//	if (GetFormat() == vFormat::VT_FLOAT && GetPass() == 4)
//	{
//		byte* pStripY = &imageData[y * byteCountPerStripe];
//		float R = GetFloat(pStripY, x * PixelBytes);
//		float G = GetFloat(pStripY, x * PixelBytes + 4);
//		float B = GetFloat(pStripY, x * PixelBytes + 8);
//		float A = GetFloat(pStripY, x * PixelBytes + 12);
//		color = new float[4]{ R,G,B,A };
//	}
//	else if (GetFormat() == vFormat::VT_UNSIGNED_BYTE && GetPass() == 3)
//	{
//		byte* pStripY = &imageData[y * byteCountPerStripe];
//		float R = (float)pGetInt(pStripY, x * PixelBytes, 1) / 255.0;
//		float G = (float)pGetInt(pStripY, x * PixelBytes + 1, 1) / 255.0;
//		float B = (float)pGetInt(pStripY, x * PixelBytes + 2, 1) / 255.0;
//		color = new float[3]{ R,G,B };
//	}
//	return color;
//}
void vIFD::PrintInfo()
{
	/*std::cout << "ImageWidth: " << ImageWidth << std::endl;
	std::cout << "ImageLength: " << ImageLength << std::endl;*/
}
int vIFD::GetInt(int startPos, int Length)
{
	return pGetInt(p_Data, startPos, Length);
}
float vIFD::GetRational(int startPos)
{
	int A = GetInt(startPos, 4);
	int B = GetInt(startPos + 4, 4);
	return A / B;
}
std::string vIFD::GetString(int startPos, int Length)//II和MM对String没有影响
{
	std::string tmp = "";
	for (int i = 0; i < Length; i++)
		tmp += (char)p_Data[startPos];
	return tmp;
}
void vIFD::GetIntArray(int startPos,int typeSize,int count, std::vector<int>& arr)
{
	arr = std::vector<int>(count);
	//arr.resize(count);
	for (int i = 0; i < count; i++)
	{
		int v = GetInt(startPos + i * typeSize, typeSize);
		arr[i] = v;
	}
}
float vIFD::GetFloat(byte* b, int startPos)
{
	byte* byteTemp;
	if (ByteOrder)// "II")
	    byteTemp = new byte[4]{ b[startPos],b[startPos + 1],b[startPos + 2],b[startPos + 3] };
	else
	    byteTemp = new byte[4]{ b[startPos + 3],b[startPos + 2],b[startPos + 1],b[startPos] };
	
	return *(float*)(byteTemp);
}

byte* vIFD::GetPixelByte(int x, int y)
{
	byte* pStripY = &imageData[y* byteCountPerStripe];
	byte R = pStripY[x];
	byte G = pStripY[x + 1];
	byte B = pStripY[x + 2];
	byte A = pStripY[x + 3];
	byte* color = new byte[4]{ R,G,B,A };
	return color;
}

vFormat vIFD::GetFormat()
{
	/*int f = SampleFormat.size() == 0 ? VT_UNSIGNED_BYTE : (SampleFormat[0]*3-2);
	int f2 = BitsPerSample[0] == 8
	if(BitsPerSample[0] == 8)
		return vFormat::*/
	int f = SampleFormat.size() == 0 ? 1 : SampleFormat[0];
	int b = BitsPerSample[0];
	vFormat format = VT_UNDEFINED;
	if (f == 1)
	{
		if (b == 8)//无符号整型
		{
			format = VT_UNSIGNED_BYTE;
		}
		else if (b == 16)
		{
			format = VT_UNSIGNED_SHORT;//16-bits
		}
		else if (b == 32)
		{
			format = VT_UNSIGNED_INT;//32-bits
		}
	}
	else if (f == 2)//有符号整型
	{
		if (b == 8)
		{
			format = VT_BYTE;
		}
		else if (b == 16)
		{
			format = VT_SHORT;
		}
		else if (b == 32)
		{
			format = VT_INTE;
		}
	}
	else if (f == 3)
	{
		if (b == 16)
		{
			format = VT_HALF_FLOAT;
		}
		else if (b == 32)
		{
			format = VT_FLOAT; //32-bits
		}
		else if (b == 64)
		{
			format = VT_DOUBLE;//64-bits
		}
	}

	return format;
}