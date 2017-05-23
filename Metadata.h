#ifndef METADATA_H
#define METADATA_H
#include "Viewport.h"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;
class Metadata
{
public:
	Viewport* vp;
	double SD; 
	int SESS_DUR; // session duration in secs
	int NO_VER;
	int FPS;
	int NO_SEG; // about 5sec
	double BUFFSIZE; // buffer size as number of segments
	int NO_SEG_FULL;
	int NO_HEAD_SAMPLE;
	int NO_VMASK_SAMPLE;
	double*** TILE_BR;
	double*** TILE_SIZE;
	double*** TILE_MSE;
	double*** TILE_PSNR;
	double** VER_BR;
	int **head_pos;
	// double** VER_MSE;
	// double** VER_PSNR;
	// char* url_base;
	// int** vp_trace;
	// int* view_angle;
	int** vmask;
	int** pixel;
	Metadata(Viewport* vp_, double SD_, int SESS_DUR_, int NO_VER_, int FPS_,int NO_SEG_, double BUFFSIZE_, int NO_SEG_DUL_);
	~Metadata();
	void loadTileInfo();
	void loadHeadTrace(char *filename);
	void loadVisibleMask(char *filename);
	void import_matrix_from_txt_file(const char* filename_X, vector <double>& v, int& rows, int& cols);
	int ReadNumbers( const string & s, vector <double> & v );
};
#endif