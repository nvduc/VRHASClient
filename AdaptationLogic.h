#ifndef AL_H
#define AL_H
#include "Metadata.h"
#include "Viewport.h"
class AdaptationLogic
{
public:
	Metadata *metadata;
	double margin; // safety margin
	int METHOD;
	double alpha_est_error;
	//
	double* seg_play_time;
	int* seg_stall_time;
	int* decide_time;
	//
	double* seg_end_time;
	double* seg_start_time;
	double* seg_thrp;
	double* est_thrp;
	int* cur_vp;
	int* est_vp;
	double* seg_bitrate;
	int* real_vp_first;
	int* real_vp_last;
	double** tileSize;
	int** tileVer;
	double* RTT;
	double* dataThrp;
	double* est_vp_psnr_first;
	double* est_vp_psnr_last;
	int* est_vp_last;
	int* est_error;
	AdaptationLogic(Metadata *metadata);
	~AdaptationLogic();
	double* getNextSegment(int index);
	int* equalTile(int index);
	// int* directRoIBG(int index)
	// int* getVisibleTile(int cur_vp)
	double getSegBitrate(int index, int* tileVer);
	void estimator(int index);
	int calcViewport(int** vp_trace, double t_now);
	double* getTileSize(int index, int* tileVer);
	// void setSegmentThrp(int index, double segmentThrp);
	// void setSegStartTime(int index, long startRequestTime);
	// void setSegFinishTime(int index, long finishReceiveTime);
	// void updateSegPlayTime(int d, int index);
	// void writeLogData();
};
#endif