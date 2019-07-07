#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lyric.h"

#pragma warning(disable:4996)
#define LRC_LEN_MAX 256
typedef struct LineLyric LineLyric;
struct LineLyric
{
	int startTime; // second millisecond
	char szLrc[LRC_LEN_MAX];
	LineLyric *next;
};

LineLyric *g_lineLyricHead = NULL;
LineLyric *g_lineLyricTail = NULL;
int g_lineLyricCount = 0;

int LyricInit(char *lrcpath)
{
	FILE *fp = NULL;
	char buf[LRC_LEN_MAX] = {0};
	WCHAR wbuf[LRC_LEN_MAX] = {0};
	char *p;

	fp = fopen(lrcpath, "r");
	if (fp == NULL)
	{
		printf("lyric init open %s error(%d)\n", lrcpath, GetLastError());
		return -1;
	}

	while ((p = fgets(buf, LRC_LEN_MAX, fp)) != NULL)
	{
		char *tok = "[";
		char *tmpval = NULL;
		if (*p == '\n')
		{
			continue;
		}
		tmpval = strchr(p, '[');
		if (tmpval == NULL)
		{
			continue;
		}
		tmpval++;
		if (strnicmp(tmpval, "ar", 2) == 0) //!< Artist
		{
		}
		else if (strnicmp(tmpval, "ti", 2) == 0) //!< title
		{
		}
		else if (strnicmp(tmpval, "al", 2) == 0) //!< album 
		{
		}
		else if (strnicmp(tmpval, "by", 2) == 0) //!< editor
		{
		}
		else if (strnicmp(tmpval, "offset", 6) == 0)//!< time compensated value
		{
		}
		else
		{
			char *lrc;
			LineLyric *llnode = NULL;
			float mi= 0.0;
			float ss = 0.0;
			int startTime= 0;
			char *left = p;
			char *right;
			lrc = strrchr(buf, ']');
			lrc++;

			do 
			{
				char tmp[128] = {0};
				left = strchr(left, '[');
				if (left == NULL)
				{
					break;
				}
				left++;
				right = strchr(left, ':');
				memcpy(tmp, left, right - left);
				mi = (float)atof(tmp);
				left = right+1;
				right = strchr(left, ']');
				memcpy(tmp, left, right - left);
				ss = (float)atof(tmp);
				startTime = (int)(mi*60+ss);
				
				llnode = malloc(sizeof(LineLyric));
				memset(llnode, 0, sizeof(LineLyric));
				llnode->startTime = startTime;
				strcpy(llnode->szLrc, lrc);
				llnode->next = NULL;
				if (g_lineLyricHead == NULL)
				{
					g_lineLyricHead = llnode;
					g_lineLyricTail = llnode;
				}
				else
				{
					LineLyric *llprev = g_lineLyricHead;
					LineLyric *llnext = llprev->next;
					if (llnode->startTime < llprev->startTime)
					{
						llnode->next = llprev;
						g_lineLyricHead = llnode;
					}
					else
					{
						while (llnext && llnext->startTime <= llnode->startTime)
						{
							llprev = llnext;
							llnext = llnext->next;
						}
						llnode->next = llnext;
						llprev->next = llnode;
						if (llnext == NULL)
						{
							g_lineLyricTail = llnode;
						}
					}
				}
				g_lineLyricCount++;
				left = right+1;

			} while (1);
		}
	}

	return 0;
}

int LyricDestroy()
{
	LineLyric *llnode;
	if (g_lineLyricHead == NULL)
	{
		return 0;
	}
	llnode = g_lineLyricHead;
	while (llnode)
	{
		LineLyric *llnext = llnode->next;
		free(llnode);
		llnode = llnext;
	}
	g_lineLyricHead = NULL;
	g_lineLyricTail = NULL;
	g_lineLyricCount = 0;
	return 0;
}

char *GetLyricByStartTime(int startTime)
{
	LineLyric *llnode;
	char *szLrc = NULL;
	if (g_lineLyricHead == NULL)
	{
		return NULL;
	}
	llnode = g_lineLyricHead;
	while (llnode)
	{
		if (llnode->startTime == startTime)
		{
			if (szLrc == NULL)
			{
				szLrc = malloc(strlen(llnode->szLrc)+1);
				memset(szLrc, 0, strlen(llnode->szLrc)+1);
				strcat(szLrc, llnode->szLrc);
			}
			else
			{
				szLrc = realloc(szLrc, strlen(szLrc)+strlen(llnode->szLrc)+2);
				strcat(szLrc, " ");
				strcat(szLrc, llnode->szLrc);
			}
		}
		llnode = llnode->next;
	}

	return szLrc;
}

char *GetLyricByPos(int pos)
{
	LineLyric *llnode;
	int i = 0;
	if (pos > g_lineLyricCount || g_lineLyricHead == NULL)
	{
		return NULL;
	}

	llnode = g_lineLyricHead;
	while (llnode)
	{
		if (i == pos)
		{
			break;
		}
		llnode = llnode->next;
		i++;
	}
	if (llnode == NULL)
	{
		return NULL;
	}
	return llnode->szLrc;
}

int GetLyricTotalLine()
{
	return g_lineLyricCount;
}
