#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <io.h>
#include <string>

using namespace std;

void main(int argc, char *argv[])
{
	if (argc != 8)
	{
		printf("usage \n psnr.exe width height fps framenumber orig test_yuv test_bin\n");
		return;
	}
	int width = stol(argv[1]);
	int height = stol(argv[2]);
	int fps = stol(argv[3]);
	int frames = stol(argv[4]);
	string file_orig = string(argv[5]);
	string file_test_yuv = string(argv[6]);
	string file_test_bin = string(argv[7]);
	
	long long size = width * height;
	FILE * fp_orig = fopen(file_orig.c_str(), "rb");
	if (!fp_orig)
	{
		printf("open orig yuv file error!\n");
		return;
	}
	FILE * fp = fopen(file_test_yuv.c_str(), "rb");
	if (!fp)
	{
		printf("open rec yuv file error!\n");
		return;
	}
	FILE * fp_bin = fopen(file_test_bin.c_str(), "rb");
	if (!fp_bin)
	{
		printf("open bin file error!\n");
		return;
	}
	unsigned char *y_orig = new unsigned char[size];
	unsigned char *u_orig = new unsigned char[size / 4];
	unsigned char *v_orig = new unsigned char[size / 4];
	unsigned char *y = new unsigned char[size];
	unsigned char *u = new unsigned char[size / 4];
	unsigned char *v = new unsigned char[size / 4];
	
	double psnr_y = 0.0f;
	double psnr_u = 0.0f;
	double psnr_v = 0.0f;
	
	for (auto j = 0; j < frames; j++)
	{
		long long count_y = 0;
		long long count_u = 0;
		long long count_v = 0;
		
		fread(y_orig, 1, size, fp_orig);
		fread(y, 1, size, fp);
		fread(u_orig, 1, size / 4, fp_orig);
		fread(u, 1, size / 4, fp);
		fread(v_orig, 1, size / 4, fp_orig);
		fread(v, 1, size / 4, fp);
		
		for (auto k = 0; k < size; k++)
		{
			count_y += (y[k] - y_orig[k]) * (y[k] - y_orig[k]);
		}
		for (auto k = 0; k < size / 4; k++)
		{
			count_u += (u[k] - u_orig[k]) * (u[k] - u_orig[k]);
			count_v += (v[k] - v_orig[k]) * (v[k] - v_orig[k]);
		}
		
		psnr_y += 10.0f * log10(size * 255 * 255 / count_y);
		psnr_u += 10.0f * log10(size / 4 * 255 * 255 / count_u);
		psnr_v += 10.0f * log10(size / 4 * 255 * 255 / count_v);
	}
	psnr_y /= frames;
	psnr_u /= frames;
	psnr_v /= frames;
	long bin_length = _filelength(_fileno(fp_bin));
	double bitrate = 1.0f * bin_length * fps * 8 / frames / 1000;
	printf("%.4f\t%.4f\t%.4f\t%.4f\n", bitrate, psnr_y, psnr_u, psnr_v);
	return;
}