#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <strsafe.h>//provide additional processing for proper buffer handling in your code to solve some security issues
#include <conio.h>//Defines functions for data input and data output through the console
#include <ShlObj.h>
#include "resource.h"

#define whWi 450
#define whHe 500


#include "gds.h"
#include "playlist.h"
#include "lyric.h"

#pragma comment (lib, "Comctl32.lib")//to register the window class for the specified control
#pragma comment (lib, "gds.lib")
#pragma comment (lib, "winmm.lib")

typedef enum
{
	C_BTN_PLAY = 1,
	C_BTN_PAUSE,
	C_BTN_PREV,
	C_BTN_NEXT,
	C_BTN_STOP,
	C_BTN_FROM_PL,
	C_LYRIC_STATIC,
	C_TIME_STATIC,
	C_BTN_ADD,
	C_BTN_DIR,
	C_BTN_DEL,
	C_BTN_SAVE,
	C_LAST
}COMPONENT_ID;

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
int playIndex = 0;
static int status = -1; // C_BTN_PLAY:play;C_BTN_PAUSE:pause;C_BTN_STOP:stop
UINT g_timeId = 0;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void createBtn(HWND hWnd, LPARAM lParam);
void createListBox(HWND hWnd);
void createLyricStatic(HWND hWnd);
void createTimeStatic(HWND hWnd);
int addFile(HWND hWnd);
int addDir(HWND hWnd);
int delFile(HWND hWnd);
char *getLyricPath(char *songPath, char *lyricPath);
void timeProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2);
void endTime();
void beginTime();

HWND btnPrev = NULL;
HWND btnPlay = NULL;
HWND btnStop = NULL;
HWND btnNext = NULL;
HWND playlistLB = NULL;
HWND lyricStatic = NULL;
HWND timeStatic = NULL;
HWND btnAdd = NULL;
HWND btnDir = NULL;
HWND btnDel = NULL;
HWND btnSave = NULL;

int APIENTRY WinMain(HINSTANCE hInstance, 
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, //命令行command line
	int nCmdShow) //display mode
{
	MSG msg;//信息
	HWND hWnd;//画布
	char szTitle[] = "GDSPLAYER"; // The title bar text
	WNDCLASSEX wcex = { 0 };

	g_hInst = hInstance;
	wcex.cbSize = sizeof(WNDCLASSEX); //size of WNDCLASSEX
	wcex.style = CS_HREDRAW | CS_VREDRAW; //redraw when position changes
	wcex.lpfnWndProc = (WNDPROC)WndProc; //message-handler function, to call CALLBACK WndProc（LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)）
	wcex.hInstance = 0; //handle to the current instance
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //background color
	wcex.lpszClassName = "gdsclass"; //参窗口类名Pprameter window class name
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)105); //icon
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); //cursor 
	wcex.lpszMenuName = 0; //MenuName
	wcex.hIconSm = 0; 
	

	RegisterClassEx(&wcex); //regester 

	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, "gdsclass", szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_VISIBLE, //创建窗口
		CW_USEDEFAULT, CW_USEDEFAULT, whWi, whHe, NULL, NULL, 0, NULL);
	
	if (!hWnd)
	{
		return FALSE;
	}

	g_hWnd = hWnd;


	while (GetMessage(&msg, NULL, 0, 0)) // message loop, windows keep showing
	{
		TranslateMessage(&msg); //converts virtual keys to character messages
		DispatchMessage(&msg); //the dispatch message calls the callback function分派消息调用回调函数
	}
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	HDC hdc;
	RECT rect;


	static HDC    s_hdcMem;
	static RECT oldRect;
	switch (message) 
	{
	case WM_PAINT: //Redraw message
		{
			hdc = BeginPaint(hWnd, &ps);
			GetWindowRect(hWnd, &rect);
			if (rect.right - rect.left != oldRect.right - oldRect.left
				|| rect.bottom - rect.top != oldRect.bottom - oldRect.top)
			{
				oldRect = rect;

				SetWindowPos(playlistLB, NULL, 1, 90, whWi - 18, oldRect.bottom - oldRect.top - 170, SWP_SHOWWINDOW);
				SetWindowPos(lyricStatic, NULL, 1, 35, whWi - 18, oldRect.bottom - oldRect.top - 450, SWP_SHOWWINDOW);
			}
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_CREATE:
		{
			GetWindowRect(hWnd, &oldRect);
			createBtn(hWnd, lParam);
			createLyricStatic(hWnd);
			createListBox(hWnd);
			createTimeStatic(hWnd);
		}
		break;

	case WM_COMMAND:
	{
		int id = LOWORD(wParam);//get buttons' ID

		switch (id)
		{
		case C_BTN_PREV:
		{
			char *p = NULL;
			static WCHAR songpath[MAX_PATH] = { 0 };
			char lyricpath[MAX_PATH] = { 0 };

			memset(songpath, 0, MAX_PATH);

			playIndex--;
			if (playIndex < 0)
			{
				playIndex = GetDefaultPlaylistTotalItem();
			}

			p = GetItemFromDefaultPlaylist(playIndex);
			if (p == NULL)
			{
				break;
			}
			getLyricPath(p, lyricpath);
			LyricDestroy();
			LyricInit(lyricpath);

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
			endTime();
			gdsPlayFile(songpath);
			beginTime();
			status = C_BTN_PLAY;
			SetWindowText(btnPlay, "pause");
			SetWindowText(hWnd, p);
		}

				break;
			case C_BTN_PLAY:
				{
					
					if (status != C_BTN_PLAY )
					{
						gdsPlay();
						status = C_BTN_PLAY;
						SetWindowText(btnPlay, "pause");
					}
					else if (status == C_BTN_PLAY)
					{
						gdsPause();
						status = C_BTN_PAUSE;
						SetWindowText(btnPlay, "play");
					}
				}
				break;
			case C_BTN_STOP:
				{
					if (status == C_BTN_PLAY || status == C_BTN_PAUSE)
					{
						gdsStop();
						status = C_BTN_STOP;
						SetWindowText(btnPlay, "play");
					}
				}
				break;
			case C_BTN_NEXT:
				{
					char *p = NULL;
					static WCHAR songpath[MAX_PATH] = {0};
					char lyricpath[MAX_PATH] = {0};

					memset(songpath, 0, MAX_PATH);

					playIndex++;
					if (playIndex > GetDefaultPlaylistTotalItem())
					{
						playIndex = 0;
					}

					p = GetItemFromDefaultPlaylist(playIndex);
					if (p == NULL)
					{
						break;
					}
					getLyricPath(p, lyricpath);
					LyricDestroy();
					LyricInit(lyricpath);

					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
					endTime();
					gdsPlayFile(songpath);
					beginTime();
					status = C_BTN_PLAY;
					SetWindowText(btnPlay, "pause");
					SetWindowText(hWnd, p);
				}
				break;
			case C_BTN_FROM_PL:
				{
					int selid = HIWORD(wParam);//message type
					switch (selid) 
					{ 
					case LBN_DBLCLK://double click
						{
							// Get selected index.
							int lbItem = (int)SendMessage(playlistLB, LB_GETCURSEL, 0, 0); //Gets the double-clicked entry index
							char *p = NULL;
							static WCHAR songpath[MAX_PATH] = {0};
							char lyricpath[MAX_PATH] = {0};

							memset(songpath, 0, MAX_PATH);

							playIndex = lbItem;
							p = GetItemFromDefaultPlaylist(playIndex);//Gets the path of the song to play
							if (p == NULL)
							{
								break;
							}
							getLyricPath(p, lyricpath); //Convert the song name to the lyrics name
							LyricDestroy();
							LyricInit(lyricpath);// initialize the lyrics

							MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
							endTime();
							gdsPlayFile(songpath); //Begin playing the song 
							beginTime(); // create a timer beginTime() to check the play progress
							status = C_BTN_PLAY;//Set the play button state
							SetWindowText(btnPlay, "pause");
							SetWindowText(hWnd, p);
						}
						break;
					default:
						{
						}
						break;
					}
				}
				break;
			case C_BTN_ADD:
				{
					addFile(hWnd);
				}
				break;
			case C_BTN_DIR:
				{
					addDir(hWnd);
				}
				break;
			case C_BTN_DEL:
				{
					delFile(hWnd);
				}
				break;
			case C_BTN_SAVE:
				{
					DefaultPlaylistSave();
				}
				break;
			default:
				break;
			}
		}
		break;
	case WM_DESTROY: //WINDOWS destroy massage
		PostQuitMessage(0);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//create buttons
void createBtn(HWND hWnd, LPARAM lParam)
{
	int xPos, yPos;
	xPos = 0;
	yPos = 425;
	btnPrev = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "prev", WS_VISIBLE | WS_CHILD, xPos + 5, yPos + 0, 45, 30, hWnd, C_BTN_PREV, g_hInst, NULL);
	btnPlay = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "play", WS_VISIBLE | WS_CHILD, xPos + 55, yPos + 0, 45, 30, hWnd, C_BTN_PLAY, g_hInst, NULL);
	btnNext = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "next", WS_VISIBLE | WS_CHILD, xPos + 105, yPos + 0, 45, 30, hWnd, C_BTN_NEXT, g_hInst, NULL);
	btnStop = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "stop", WS_VISIBLE | WS_CHILD, xPos + 155, yPos + 0, 45, 30, hWnd, C_BTN_STOP, g_hInst, NULL);
	xPos = whWi + 20;
	btnAdd = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "add", WS_VISIBLE | WS_CHILD, xPos - 235, yPos, 45, 30, hWnd, C_BTN_ADD, g_hInst, NULL);
	btnDir = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "dir", WS_VISIBLE | WS_CHILD, xPos - 185, yPos, 45, 30, hWnd, C_BTN_DIR, g_hInst, NULL);
	btnDel = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "del", WS_VISIBLE | WS_CHILD, xPos - 135, yPos, 45, 30, hWnd, C_BTN_DEL, g_hInst, NULL);
	btnSave = CreateWindowEx(WS_EX_ACCEPTFILES, WC_BUTTON, "save", WS_VISIBLE | WS_CHILD, xPos - 85, yPos, 45, 30, hWnd, C_BTN_SAVE, g_hInst, NULL);
}

//file
int addFile(HWND hWnd)
{
	OPENFILENAME fn;
	char    filefilter[] =
		"All Supported files\0*.mp1;*.mp2;*.mp3;*.m3u;*.ogg;*.pls;*.wav\0MPEG audio files (*.mp1;*.mp2;*.mp3)\0*.mp1;*.mp2;*.mp3\0Vorbis files (*.ogg)\0Playlist files (*.m3u;*.pls)\0*.m3u;*.pls\0WAV files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	BOOL    retVal = FALSE;
	char    initialfilename[MAX_PATH * 100] = "";
	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = hWnd;
	fn.hInstance = NULL;
	fn.lpstrFilter = filefilter;
	fn.lpstrCustomFilter = NULL;
	fn.nMaxCustFilter = 0;
	fn.nFilterIndex = 0;
	fn.lpstrFile = initialfilename;
	fn.nMaxFile = MAX_PATH * 200;
	fn.lpstrFileTitle = NULL;
	fn.nMaxFileTitle = 0;
	fn.lpstrInitialDir = "./";
	fn.lpstrTitle = NULL;
	fn.Flags =
		OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_EXPLORER |
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	fn.nFileOffset = 0;
	fn.nFileExtension = 0;
	fn.lpstrDefExt = NULL;
	fn.lCustData = 0;
	fn.lpfnHook = NULL;
	fn.lpTemplateName = NULL;

	retVal = GetOpenFileName(&fn);//Get the added file path

	if (retVal != FALSE)
	{
		char path_buffer[MAX_PATH];
		char *pval = NULL;
		strcpy(path_buffer, fn.lpstrFile);

		pval = strrchr(path_buffer, '\\');
		if (pval == NULL)
		{
			pval = path_buffer;
		}
		else
		{
			pval++;
		}
		SendMessage(playlistLB, LB_INSERTSTRING, (-1), pval);//Send the song name to the playlist playlistBL component
		DefaultPlaylistAddItem(path_buffer);//Add the song path to the default playlist
	}
	return 0;
}

int addDir(HWND hWnd)
{
	BROWSEINFO browseinfo;
	LPITEMIDLIST itemlist;
	int     image = 0;
	char    directorychoice[MAX_PATH];
	char    fullpath[MAX_PATH];
	HANDLE  found;
	WIN32_FIND_DATA finddata;
	char    pathbuf2[MAX_PATH];
	char	dirBuf[MAX_PATH];

	browseinfo.hwndOwner = hWnd;
	browseinfo.pidlRoot = NULL;
	browseinfo.pszDisplayName = directorychoice;
	browseinfo.lpszTitle = "Choose a directory to add";
	browseinfo.ulFlags = BIF_EDITBOX;
	browseinfo.lpfn = NULL;
	browseinfo.lParam = 0;
	browseinfo.iImage = image;

	itemlist = SHBrowseForFolder(&browseinfo);
	if (itemlist == NULL)
	{
		return 1;
	}

	SHGetPathFromIDList(itemlist, dirBuf);

	if (dirBuf[strlen(dirBuf) - 1] == '\\'
		&& strcmp(dirBuf, "\\") != 0) dirBuf[strlen(dirBuf) - 1] =
		'\0';

	strcpy(fullpath, dirBuf);

	if (strcmp(fullpath, "\\") == 0)
		strcat(fullpath, ".\\*.mp3");
	else
		strcat(fullpath, "\\*.mp3");

	found = FindFirstFile(fullpath, &finddata);
	do {
		char    somepath[MAX_PATH];
		strcpy(somepath, dirBuf);
		if (strcmp(somepath, "\\") == 0)
			strcpy(somepath, "\\.");
		sprintf(pathbuf2, "%s\\%s", somepath, finddata.cFileName);

		if ((finddata.cFileName[0] != '.'
			&& finddata.cFileName[1] != 0)
			&& (finddata.cFileName[0] != '.'
				&& finddata.cFileName[1] != '.'
				&& finddata.cFileName[2] != 0))
		{
			char *pval = NULL;
			pval = strrchr(pathbuf2, '\\');
			if (pval == NULL)
			{
				pval = pathbuf2;
			}
			else
			{
				pval++;
			}
			SendMessage(playlistLB, LB_INSERTSTRING, (-1), pval);
			DefaultPlaylistAddItem(pathbuf2);
		}
	} while (FindNextFile(found, &finddata));
	FindClose(found);

	return 0;
}

int delFile(HWND hWnd)
{

	int lbItem = -1;
	lbItem = (int)SendMessage(playlistLB, LB_GETCURSEL, 0, 0);//Gets the index of the currently selected song
	if (lbItem != -1)
	{
		DefaultPlaylistDeleteItem(lbItem); // Deletes songs from the playlist and items from the playlist component
		SendMessage(playlistLB, LB_DELETESTRING, lbItem, 0);
	}
	return 0;
}

//create playlist 
void createListBox(HWND hWnd)
{
	int i = 0;
	POINT p = {10, 70};
	int ident = 0;
	int plItemCount = 0;
	int colWidth = 0;
	int colMaxWidth = 300;
	RECT rect;

	GetWindowRect(hWnd, &rect);

	PlayListInit();//Initializes the default playlist
	plItemCount = GetDefaultPlaylistTotalItem();
	playlistLB = CreateWindowEx(WS_EX_ACCEPTFILES, WC_LISTBOX, "default", 
		WS_VISIBLE | LBS_COMBOBOX | WS_CHILD|WS_VSCROLL |LBN_DBLCLK|LBS_NOTIFY,
		1, 90, whWi-18, rect.bottom - rect.top - 170, hWnd, C_BTN_FROM_PL, g_hInst, NULL);//create

	for (i = 0; i < plItemCount; i++)
	{
		char *val = NULL;
		char *pval = NULL;
		char plItem[MAX_PATH] = {0};
		val = GetItemFromDefaultPlaylist(i);//Gets all the song paths and inserts them into the playlist component获取所有歌曲路径并将其插入到播放列表组件中
		pval = strrchr(val, '\\');
		if (pval == NULL)
		{
			pval = val;
		}
		else
		{
			pval++;
		}
		sprintf(plItem, "%03d.%s", i+1, pval);
		SendMessage(playlistLB, LB_INSERTSTRING, (-1), plItem);
	}
	
}

//create lyric
char *getLyricPath(char *songPath, char *lyricPath)
{
	char buf[MAX_PATH] = { 0 };
	char *p = buf;
	strcpy(buf, songPath);
	p = strrchr(p, '.');
	p++;
	memcpy(p, "lrc", 3);
	*(p + 3) = '\0';
	strcpy(lyricPath, buf);
	return lyricPath;
}

void createLyricStatic(HWND hWnd)
{
	RECT rect;

	GetWindowRect(hWnd, &rect);

	lyricStatic = CreateWindowEx(WS_EX_ACCEPTFILES, WC_STATIC, "lyric",
		WS_VISIBLE | WS_CHILD | LBS_NOTIFY | SS_SUNKEN | SS_CENTER, 1, 35, whWi - 18, rect.bottom - rect.top - 450, hWnd, C_LYRIC_STATIC, g_hInst, NULL);
}



//time
void timeProc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2)
{
	long long curpos = 0;
	long long endpos = 0;
	int curtime = 0;
	int endtime = 0;
	char *p = NULL;
	char *szLryic = NULL;
	char szTime[1024] = {0};
	double rate = 0.0;

	gdsGetPositions(&curpos, &endpos);

	if (curpos == endpos)
	{
		// play the next
		WCHAR songpath[MAX_PATH] = {0};
		char lyricpath[MAX_PATH] = {0};
		char *p;

		playIndex++;
		if (playIndex > GetDefaultPlaylistTotalItem())
		{
			playIndex = 0;
		}

		p = GetItemFromDefaultPlaylist(playIndex);
		if (p == NULL)
		{
			return;
		}
		getLyricPath(p, lyricpath);
		LyricDestroy();
		LyricInit(lyricpath);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, p, strlen(p), songpath, MAX_PATH);
		gdsPlayFile(songpath);
		status = C_BTN_PLAY;
		SetWindowText(btnPlay, "pause");
		SetWindowText(g_hWnd, p);
	}

	curtime = (int)(curpos / (double)10000000);
	endtime = (int)(endpos / (double)10000000);
	sprintf(szTime, "TIME: \n%02d:%02d / %02d:%02d", curtime/60, curtime%60, endtime/60, endtime%60);
	SendMessage(timeStatic, WM_SETTEXT, 0, szTime);

	szLryic = GetLyricByStartTime(curtime);

	if (szLryic != NULL)
	{
		SendMessage(lyricStatic, WM_SETTEXT, 0, szLryic);
	}
}

void beginTime()
{

	if((g_timeId = timeSetEvent(500, 1,(LPTIMECALLBACK)timeProc,(DWORD_PTR)0,TIME_PERIODIC)) == 0)
	{
		printf("time set event error!\n");
	}
}

void endTime()
{
	if (g_timeId != 0)
	{
		timeKillEvent(g_timeId);
		g_timeId = 0;
	}
}

void createTimeStatic(HWND hWnd)
{
	timeStatic = CreateWindowEx(WS_EX_ACCEPTFILES, WC_STATIC, "TIME: \n00:00 / 00:00", 
		WS_VISIBLE | WS_CHILD|LBS_NOTIFY|SS_CENTER, whWi-150, 0, 100, 30, hWnd, C_TIME_STATIC, g_hInst, NULL);

}
