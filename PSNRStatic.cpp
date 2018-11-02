#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/stat.h>
#endif
#include <cmath>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 9)
	{
		printf("usage \n psnr.exe width height bitdepth fps framenumber orig test_yuv test_bin\n");
		return -1;
	}
	int width = stol(argv[1]);
	int height = stol(argv[2]);
	int bitdepth = stoi(argv[3]);
	int fps = stol(argv[4]);
	int frames = stol(argv[5]);
	string file_orig = string(argv[6]);
	string file_test_yuv = string(argv[7]);
	string file_test_bin = string(argv[8]);
	
	long long size = width * height;
	FILE * fp_orig = fopen(file_orig.c_str(), "rb");
	if (!fp_orig)
	{
		printf("open orig yuv file error!\n");
		return -1;
	}
	FILE * fp = fopen(file_test_yuv.c_str(), "rb");
	if (!fp)
	{
		printf("open rec yuv file error!\n");
		return -1;
	}
	FILE * fp_bin = fopen(file_test_bin.c_str(), "rb");
	if (!fp_bin)
	{
		printf("open bin file error!\n");
		return -1;
	}
	void *y_orig, *u_orig, *v_orig;
	void *y, *u, *v;
	if (bitdepth == 8)
	{
		y_orig = static_cast<void*>(new unsigned char[size]);
		u_orig = static_cast<void*>(new unsigned char[size / 4]);
		v_orig = static_cast<void*>(new unsigned char[size / 4]);
		y = static_cast<void*>(new unsigned char[size]);
		u = static_cast<void*>(new unsigned char[size / 4]);
		v = static_cast<void*>(new unsigned char[size / 4]);
	}
	else if (bitdepth > 8 && bitdepth <= 16)
	{
		y_orig = static_cast<void*>(new unsigned short[size]);
		u_orig = static_cast<void*>(new unsigned short[size / 4]);
		v_orig = static_cast<void*>(new unsigned short[size / 4]);
		y = static_cast<void*>(new unsigned short[size]);
		u = static_cast<void*>(new unsigned short[size / 4]);
		v = static_cast<void*>(new unsigned short[size / 4]);
	}
	else
	{
		printf("invalid bitdepth!\n");
		return -1;
	}
	
	double psnr_y = 0.0f;
	double psnr_u = 0.0f;
	double psnr_v = 0.0f;
	
	for (auto j = 0; j < frames; j++)
	{
		long long count_y = 0;
		long long count_u = 0;
		long long count_v = 0;
		
		if (bitdepth == 8)
		{
			fread(y_orig, 1, size, fp_orig);
			fread(y, 1, size, fp);
			fread(u_orig, 1, size / 4, fp_orig);
			fread(u, 1, size / 4, fp);
			fread(v_orig, 1, size / 4, fp_orig);
			fread(v, 1, size / 4, fp);
		}
		else
		{
			fread(y_orig, 1, size * sizeof(unsigned short), fp_orig);
			fread(y, 1, size * sizeof(unsigned short), fp);
			fread(u_orig, 1, size / 4 * sizeof(unsigned short), fp_orig);
			fread(u, 1, size / 4 * sizeof(unsigned short), fp);
			fread(v_orig, 1, size / 4 * sizeof(unsigned short), fp_orig);
			fread(v, 1, size / 4 * sizeof(unsigned short), fp);
		}
		
		if (bitdepth == 8)
		{
			unsigned char * _y = static_cast<unsigned char *>(y);
			unsigned char * _u = static_cast<unsigned char *>(u);
			unsigned char * _v = static_cast<unsigned char *>(v);
			unsigned char * _y_orig = static_cast<unsigned char *>(y_orig);
			unsigned char * _u_orig = static_cast<unsigned char *>(u_orig);
			unsigned char * _v_orig = static_cast<unsigned char *>(v_orig);
			for (auto k = 0; k < size; k++)
			{
				count_y += ((int)(_y[k]) - (int)(_y_orig[k])) * ((int)(_y[k]) - (int)(_y_orig[k]));
			}
			for (auto k = 0; k < size / 4; k++)
			{
				count_u += ((int)(_u[k]) - (int)(_u_orig[k])) * ((int)(_u[k]) - (int)(_u_orig[k]));
				count_v += ((int)(_v[k]) - (int)(_v_orig[k])) * ((int)(_v[k]) - (int)(_v_orig[k]));
			}
		}
		else
		{
			unsigned short * _y = static_cast<unsigned short *>(y);
			unsigned short * _u = static_cast<unsigned short *>(u);
			unsigned short * _v = static_cast<unsigned short *>(v);
			unsigned short * _y_orig = static_cast<unsigned short *>(y_orig);
			unsigned short * _u_orig = static_cast<unsigned short *>(u_orig);
			unsigned short * _v_orig = static_cast<unsigned short *>(v_orig);
			for (auto k = 0; k < size; k++)
			{
				count_y += ((int)(_y[k]) - (int)(_y_orig[k])) * ((int)(_y[k]) - (int)(_y_orig[k]));
			}
			for (auto k = 0; k < size / 4; k++)
			{
				count_u += ((int)(_u[k]) - (int)(_u_orig[k])) * ((int)(_u[k]) - (int)(_u_orig[k]));
				count_v += ((int)(_v[k]) - (int)(_v_orig[k])) * ((int)(_v[k]) - (int)(_v_orig[k]));
			}
		}
		
		long long psnr_rate = ((1 << bitdepth) - 1) * ((1 << bitdepth) - 1);
		psnr_y += 10.0f * log10((double)size * psnr_rate / count_y);
		psnr_u += 10.0f * log10((double)size / 4 * psnr_rate / count_u);
		psnr_v += 10.0f * log10((double)size / 4 * psnr_rate / count_v);
	}
	psnr_y /= frames;
	psnr_u /= frames;
	psnr_v /= frames;
#ifdef WIN32
	long bin_length = _filelength(_fileno(fp_bin));
#else
    long bin_length = -1;
    struct stat statbuff;
    if(stat(file_test_bin.c_str(), &statbuff) < 0){
        bin_length = 0;
    }else{
        bin_length = statbuff.st_size;
    }
#endif
	double bitrate = 1.0f * bin_length * fps * 8 / frames / 1000;
	printf("%.4f\t%.4f\t%.4f\t%.4f\t", bitrate, psnr_y, psnr_u, psnr_v);
	return 0;
}
