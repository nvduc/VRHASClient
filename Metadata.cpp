#include "Metadata.h"
#include "Viewport.h"
#include <stdio.h>
#include <cmath>
#define _USE_MATH_DEFINES
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

Metadata::Metadata(Viewport* vp_, double SD_, int SESS_DUR_, int NO_VER_, int FPS_,int NO_SEG_, double BUFFSIZE_, int NO_SEG_FULL_){
	int i,j;
	char filename[100];
	vp = vp_;
	SD = SD_;
	SESS_DUR = SESS_DUR_;
	NO_VER = NO_VER_;
	FPS = FPS_;
	NO_SEG = NO_SEG_;
	BUFFSIZE = BUFFSIZE_;
	NO_SEG_FULL = NO_SEG_FULL_;
	// load data from file
	cout << "$$$$ Load tile info" << endl;
	loadTileInfo();
	//
	cout << "$$$$ Load head position trace" << endl;
	sprintf(filename, "data/head_pos_variable_trace_3.txt");
	loadHeadTrace(filename);
	//
	cout << "$$$$ Load visible mask" << endl;
	sprintf(filename, "data/visible_mask_data_all_tile.txt");
	loadVisibleMask(filename);
	//
	cout << "$$$$ Load inputs finished !!!" << endl;
}

Metadata::~Metadata(){

}
void Metadata::import_matrix_from_txt_file(const char* filename_X, vector <double>& v, int& rows, int& cols){
	ifstream file_X;
    string line;
    // erase all current elements
    v.erase(v.begin(), v.end());
    file_X.open(filename_X);
    if (file_X.is_open())
    {
        int i=0;
        getline(file_X, line);
        
        
        cols =ReadNumbers( line, v );
        // cout << "cols:" << cols << endl;
        
        
        for ( i=1;i<32767;i++){
            if ( getline(file_X, line) == 0 ) break;
            ReadNumbers( line, v );
            
        }
        
        rows=i;
        // cout << "rows :" << rows << endl;
        if(rows >32766) cout<< "N must be smaller than MAX_INT";
        
        file_X.close();
    }
    else{
        cout << "file open failed";
    }
    
    // cout << "v:" << endl;
    // for (int i=0;i<rows;i++){
        // for (int j=0;j<cols;j++){
            // cout << v[i*cols+j] << "\t" ;
        // }
        // cout << endl;
    // }
}
int Metadata::ReadNumbers( const string & s, vector <double> & v ){
	istringstream is( s );
    double n;
    while( is >> n ) {
        v.push_back( n );
    }
    return v.size();
}
void Metadata::loadTileInfo(){
	int tid = 0;
	int i,j,k;
	int seg_id;
	cout << NO_SEG << " "<<vp->No_tile<<" "<<NO_VER<<endl;
	// initiate variables
	TILE_BR = new double**[NO_SEG_FULL];
	TILE_MSE = new double**[NO_SEG_FULL];
	TILE_PSNR = new double**[NO_SEG_FULL];
	TILE_SIZE = new double**[NO_SEG_FULL];
	for(i=0; i < NO_SEG_FULL; i++){
		TILE_BR[i] = new double*[vp->No_tile];
		TILE_MSE[i] = new double*[vp->No_tile];
		TILE_PSNR[i] = new double*[vp->No_tile];
		TILE_SIZE[i] = new double*[vp->No_tile];
		for(j=0; j < vp->No_tile; j++){
			TILE_BR[i][j] = new double[NO_VER];
			TILE_MSE[i][j] = new double[NO_VER];
			TILE_PSNR[i][j] = new double[NO_VER];
			TILE_SIZE[i][j] = new double[NO_VER];
		}
	}
	VER_BR = new double*[NO_SEG_FULL];
	for(j=0; j < NO_SEG_FULL; j++){
		VER_BR[j] = new double[NO_VER];
	}
	//
	char tile_info_path[] = "data/tile_info/";
	string s;
	char buff[50];
	vector <double> v;
	int rows;
	int cols;
	// for each tile
	for(tid=0; tid < vp->No_tile; tid++){
	// for(tid=0; tid < 1; tid++){
		sprintf(buff, "%stile_%d.txt", tile_info_path, tid);
		s = buff;
		// cout << s << endl;
		import_matrix_from_txt_file(buff, v, rows, cols);
		for(i=0; i < NO_SEG; i++){
			for(j=0; j < NO_VER; j++){
				// cout << i << " " << j << " " << tid <<endl;
				TILE_BR[i][tid][j] = v[i * cols + 3*j];
				TILE_PSNR[i][tid][j] = v[i * cols + 3*j + 1];
				TILE_MSE[i][tid][j] = v[i * cols + 3*j + 2];
				TILE_SIZE[i][tid][j] = TILE_BR[i][tid][j] * SD;
				VER_BR[i][j] += TILE_SIZE[i][tid][j];
			}
		}
		// repeat video sequence
		cout << "NO_SEG_FULL: " << NO_SEG_FULL << " NO_SEG:" << NO_SEG <<endl;
		if(NO_SEG < NO_SEG_FULL){
			for(i=NO_SEG; i < NO_SEG_FULL; i++){
				// cout << "i:" << i << endl;
				for(k=1; k <= NO_VER; k++){
					//System.out.printf("k=%d\n", k);
					TILE_BR[i][tid][k-1] = TILE_BR[i%NO_SEG][tid][k-1];
					TILE_PSNR[i][tid][k-1] = TILE_PSNR[i%NO_SEG][tid][k-1];
					TILE_MSE[i][tid][k-1] = TILE_MSE[i%NO_SEG][tid][k-1];
					TILE_SIZE[i][tid][k-1] = TILE_SIZE[i%NO_SEG][tid][k-1];
					// cout << TILE_BR[i][tid][k-1] << " " << TILE_PSNR[i][tid][k-1] << " " << TILE_MSE[i][tid][k-1] << endl; 
				}
			}
		}
	}
}
void Metadata::loadHeadTrace(char* filename){
	int i,j,k;
	vector <double> v;
	int rows;
	int cols;
	import_matrix_from_txt_file(filename, v, rows, cols);
	NO_HEAD_SAMPLE = rows;
	head_pos = new int*[rows];
	for(i=0; i < rows; i++)
		head_pos[i] = new int[cols];
	for(i=0; i < rows; i++){
		for(j=0; j < cols; j++){
			head_pos[i][j] = v[i * cols + j];
		}
	}
}
void Metadata::loadVisibleMask(char* filename){
	int i,j,k;
	vector <double> v;
	int rows;
	int cols;
	import_matrix_from_txt_file(filename, v, rows, cols);
	NO_VMASK_SAMPLE = rows;
	vmask = new int*[NO_VMASK_SAMPLE];
	pixel = new int*[NO_VMASK_SAMPLE];
	for(i=0; i < rows; i++){
		vmask[i] = new int[vp->No_tile];
		pixel[i] = new int[vp->No_tile];
	}
	for(i=0; i < NO_VMASK_SAMPLE; i++){
		for(j=0; j < vp->No_tile; j++){
			pixel[i][j] = v[i * cols + 2*j + 2];
			if(pixel[i][j] > 0)
				vmask[i][j] = 1;
			else
				vmask[i][j] = 0;
		}
	}
}