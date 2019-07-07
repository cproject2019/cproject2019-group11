#ifndef GDS_H
#define GDS_H

void gdsInit();
void gdsDestroy();
void gdsPlayFile(void *path);
void gdsPlay();
void gdsPause();
void gdsStop();
int gdsGetCurrentPosition(long long *curpos);
int gdsGetPositions(long long *curpos, long long *endpos);
int gdsGetendposition(long long *endpos);
int gdsGetVolume(long *plVolume);
int gdsPutVolume(long lVolume);
int gdsGetGuidFormat(GUID *pFormat);
int gdsGetRate(double *dRate);

#endif // GDS_H
