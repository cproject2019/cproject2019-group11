#include <windows.h>
#include <stdio.h>
#include <string.h>

#define Wrongitem "default.m3u"

typedef struct item item;
struct item
{
	char Item[256];
	item *next;
};

item *g_itemhead = NULL;
item *g_itemtail = NULL;
int number = 0;
int g_itemnumber = 0;
char g_wrongitem[256] = {0};

int CreateItem()
{
	FILE *fp;
	char wrongitem[256] = {0};
	char *way;
	char buf[256] = {0};

	GetModuleFileName(NULL, wrongitem, 256);
	way = strrchr(wrongitem, '\\');
	way[1] = '\0';
	strcpy(g_wrongitem, wrongitem);
	sprintf(wrongitem, "%s%s", wrongitem, Wrongitem);
	fp = fopen(wrongitem, "r");
	if (fp == NULL)
	{
		printf("Couldn't open\n",Wrongitem);
		return -1;
	}

	do 
	{
		char *p;
		item *line;
		p = fgets(buf, 256, fp);
		if (p == NULL)
		{
			break;
		}
		g_itemnumber++;
		line = malloc(sizeof(item));
		memset(line->Item, 0, MAX_PATH);
		strncpy(line->Item, buf, strlen(buf) - 1); //not read \n
		line->next = NULL;
		if (g_itemhead ==  NULL)
		{
			g_itemhead = line;
			g_itemtail = g_itemhead;
		}
		else
		{
			g_itemtail->next =line;
			g_itemtail = line;
		}
		
	} while (1);
	return 0;
}

int DestroyWrongItem()
{
	item *line;
	item *linenext;
	if (g_itemhead == NULL)
	{
		return 0;
	}

	line = g_itemhead;
	while (line)
	{
		linenext = line->next;
		free(line);
		line = linenext;
	}
	return 0;
}

int ItemIn()
{
	return CreateItem();
}

int ItemRelease()
{
	return DestroyWrongItem();
}

char *InItem(int pos)
{
	int i = 0;
	item *line;
	if (g_itemhead == NULL)
	{
		return NULL;
	}

	if (pos >= g_itemnumber)
	{
		return NULL;
	}

	line = g_itemhead;
	
	while (i < pos && line != NULL)
	{
		i++;
		line = line->next;
	}

	if (line == NULL)
	{
		return NULL;
	}

	return line->Item;
}

int InTotal()
{
	return g_itemnumber;
}

// 绝对路径
int AddItem(char *itempath)
{
	WIN32_FIND_DATA finddata;
	HANDLE handle;

	handle = FindFirstFile(itempath, &finddata);
	if (INVALID_HANDLE_VALUE == handle)
	{
		return -1;
	}

	if (finddata.cFileName)
	{
		item *line;
		line = malloc(sizeof(item));
		memset(line->Item, 0, 256);
		strcpy(line->Item, itempath);
		line->next = NULL;
		if (g_itemhead ==  NULL)
		{
			g_itemhead = line;
			g_itemtail = g_itemhead;
		}
		else
		{
			g_itemtail->next = line;
			g_itemtail = line;
		}
		g_itemnumber++;
		number++;
	}

	return 0;
}

// delete one,pos从0开始
int DeleteItem(int pos)
{
	item *line;
	item *lineprev;
	int i = 0;
	if (pos >= g_itemnumber || g_itemhead == NULL)
	{
		return 0;
	}

	number++;

	// only one in the list
	if (g_itemnumber == 1)
	{
		free(g_itemhead);
		g_itemhead = NULL;
		g_itemtail = NULL;
		g_itemnumber=g_itemnumber-1;
		return 0;
	}
	
	// delete the first one
	if (pos == 0)
	{
		line = g_itemhead;
		g_itemhead = line->next;
		free(line);
		line = NULL;
		return 0;
	}

	// delete the last one
	if (pos == g_itemnumber -1)
	{
		line = g_itemhead;
		while (line->next->next)
		{
			line = line->next;
		}

		g_itemtail = line;
		line = line->next;
		g_itemtail->next = NULL;
		free(line);
		line = NULL;
		g_itemnumber=g_itemnumber-1;
		return 0;
	}

	// delete middle one
	lineprev = g_itemhead;
	while (i < pos - 1)
	{
		lineprev =lineprev->next;
		i++;
	}

	line = lineprev->next;

	free(line);
	line = NULL;
	lineprev->next = NULL;
	g_itemnumber--;
	return 0;
}

int Save()
{
	FILE *fp;
	item *line;
	char List[256] = {0};

	if (number == 0 || g_itemnumber == 0 || g_itemhead == NULL)
	{
		return 0;
	}

	sprintf(List, "%s%s", g_wrongitem, Wrongitem);
	fp = fopen(List, "w+");
	if (fp == NULL)
	{
		printf("open %s error\n", Wrongitem);
		return -1;
	}

	line = g_itemhead;
	while (line)
	{
		fputs(line->Item, fp);
		fputs("\n", fp);
		line = line->next;
		
	}
	fflush(fp);
	fclose(fp);
	return 0;
}
