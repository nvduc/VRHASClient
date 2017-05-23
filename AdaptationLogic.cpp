#include "AdaptationLogic.h"
#include "Metadata.h"
#include "Viewport.h"
#include <math.h>
#include <cmath>
AdaptationLogic::AdaptationLogic(Metadata * meta){
	int i,j;
	margin = 0.1;
	METHOD = 1;
	alpha_est_error = 0.2;
	metadata = meta;
	// cout << "No_segment" << metadata->NO_SEG << endl;
	seg_play_time = new double[metadata->NO_SEG];
	for(i=0; i < metadata->NO_SEG; i++){
		if(metadata->BUFFSIZE == 1)
			seg_play_time[i]  = i * metadata->SD * 1000;
		else
			seg_play_time[i] = i * metadata->SD * 1000 + metadata->BUFFSIZE * metadata->SD * 1000;
	}
	// cout << "No_segment #2" << metadata->NO_SEG << endl;
	seg_stall_time = new int[metadata->NO_SEG];
	decide_time = new int[metadata->NO_SEG];
	
	seg_end_time = new double[metadata->NO_SEG];
	seg_start_time = new double[metadata->NO_SEG];
	seg_thrp = new double[metadata->NO_SEG];
	dataThrp = new double[metadata->NO_SEG];
	est_thrp = new double[metadata->NO_SEG];
	cur_vp = new int[metadata->NO_SEG];
	est_vp = new int[metadata->NO_SEG];
	est_vp_last = new int[metadata->NO_SEG];
	seg_bitrate = new double[metadata->NO_SEG];
	real_vp_first = new int[metadata->NO_SEG];
	real_vp_last = new int[metadata->NO_SEG];
	RTT = new double[metadata->NO_SEG];
	est_vp_psnr_first = new double[metadata->NO_SEG];
	est_vp_psnr_last = new double[metadata->NO_SEG];
	est_error = new int[metadata->NO_SEG];
	tileSize = new double*[metadata->NO_SEG];
	tileVer = new int*[metadata->NO_SEG];
	for(i=0; i < metadata->NO_SEG; i++){
		tileSize[i] = new double[metadata->vp->No_tile];
		tileVer[i] = new int[metadata->vp->No_tile];
	}  
}
AdaptationLogic::~AdaptationLogic(){

}
double* AdaptationLogic::getNextSegment(int index){
	estimator(index);
	switch(METHOD){
	case 1:
		tileVer[index]= equalTile(index);//
		break;
	}
	// calculate tiles' size and segments' bitrate
	tileSize[index] = getTileSize(index, tileVer[index]);
	seg_bitrate[index] = getSegBitrate(index, tileVer[index]);
	// estimate viewport-psnr
	// est_vp_psnr_first[index] = estimateViewportPSNR(index, tileVer[index], real_vp_first[index]);
	// est_vp_psnr_last[index] = estimateViewportPSNR(index, tileVer[index], real_vp_last[index]);
	cout << "Tile size:" << endl;
	for (int i = 0; i < metadata->vp->No_tile_v; i++)
    {
    	for(int j=0; j < metadata->vp->No_tile_h; j++){
    		cout << tileSize[index][i * metadata->vp->No_tile_h + j] << " ";
    	}
    	cout << endl;
    }
    cout << "bitrate: " << seg_bitrate[index] << endl;
	return tileSize[index];
}
int* AdaptationLogic::equalTile(int index){
	int i;
	int select_ver;
	int* tile = new int[metadata->vp->No_tile];
	// search for the version having highest bitrate lower than estimated bandwidth
	i = 0;
	while(i < metadata->NO_VER && metadata->VER_BR[index][i] < est_thrp[index]) i++;
	select_ver = (i > 0)?(i-1):i;
	//
	cout << "select_ver: " << select_ver << endl;
	for(i=0; i < metadata->vp->No_tile; i++)
		tile[i]  = select_ver;
	return tile;
}
void AdaptationLogic::estimator(int index) {
		if(index == 0){
			est_thrp[index] = 0;
			cur_vp[index] = 0;
			est_vp[index] = 0;
			est_vp_last[index] = 0;
			real_vp_first[index] = 0;
			est_error[index] = 0;
		}else{
			// estimate throughput
//			est_thrp[index] = dataThrp[index-1]; // last-data-throughput based
			est_thrp[index] = (1-margin) * seg_thrp[index-1]; // last-throughput based
//			est_thrp[index] = (1-margin) * 5700;
			// estimate future viewport
			if(index == 1){
				cur_vp[index] = metadata->head_pos[index-1][0];
				est_vp[index] = cur_vp[index];
				est_vp_last[index] = cur_vp[index];
				est_error[index] = 0; // at the beginning, assume that estimation error is zero.
			}else{
				// calculate viewport at the moment
				cur_vp[index] = calcViewport(metadata->head_pos, decide_time[index]);
				// predict viewport at the segment's play time
				est_vp[index] = (int) (cur_vp[index] + (seg_play_time[index] - decide_time[index]) * (cur_vp[index]-cur_vp[index-1])/(decide_time[index] - decide_time[index-1]));
				if(est_vp[index] >= 360)
					est_vp[index] %= 360;
				if(est_vp[index] < 0) est_vp[index] += 360;
				est_vp_last[index] = (int) (cur_vp[index] + (seg_play_time[index] - decide_time[index] + metadata->SD * 1000) * (cur_vp[index]-cur_vp[index-1])/(decide_time[index] - decide_time[index-1]));
				if(est_vp_last[index] >= 360)
					est_vp_last[index] %= 360;
				if(est_vp_last[index] < 0) est_vp_last[index] += 360;
			}
			// estimate prediction error: model: weighted sum
			est_error[index] = (int) (alpha_est_error * est_error[index-1] + (1-alpha_est_error) * abs(cur_vp[index] - est_vp[index-1]));
			// calculate real viewport
			real_vp_first[index] = calcViewport(metadata->head_pos, seg_play_time[index]);
			real_vp_last[index] = calcViewport(metadata->head_pos, seg_play_time[index+1]);
		}
}
int AdaptationLogic::calcViewport(int** vp_trace, double t_now) {
		int cur_vp = 0;
		int i=0;
		while(i < metadata->NO_VMASK_SAMPLE && vp_trace[i][0] <= t_now) i++;
		if(i==metadata->NO_VMASK_SAMPLE)
			cur_vp = vp_trace[metadata->NO_VMASK_SAMPLE-1][1];
		else{
			// intepolate from two nearest positions [i-1] and [i]
			cur_vp = (int) (vp_trace[i-1][1] + (vp_trace[i][1]*1.0 - vp_trace[i-1][1])/(vp_trace[i][0] - vp_trace[i-1][0])*(t_now - vp_trace[i-1][0]));
		}
		return cur_vp;
}
double* AdaptationLogic::getTileSize(int index, int *tileVer){
	double* tileSize = new double[metadata->vp->No_tile];
	for(int i=0; i < metadata->vp->No_tile; i++){
		tileSize[i] = metadata->TILE_SIZE[index][i][tileVer[i]];
	}
	return tileSize;
}
double AdaptationLogic::getSegBitrate(int index, int *tileVer){
		double seg_bitrate = 0;
		for(int i=0; i < metadata->vp->No_tile; i++)
			seg_bitrate += metadata->TILE_BR[index][i][tileVer[i]];
		return seg_bitrate;	
}