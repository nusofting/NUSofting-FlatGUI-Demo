/*-----------------------------------------------------------------------------

Copyright (c) 2006 AraldFx
"used on limited license" 2007 NUSofting

Some XML-compliance corrections made in 2015 by Bernie Maier.

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#ifndef MAC
#include <windows.h>
#endif
#include "minixml.h"

#if 0
void dprintf (char* fmt, ...)
{
	static char temp[2048];
	va_list va;
	va_start(va, fmt);
	vsprintf (temp, fmt, va);
	va_end(va);
	strcat(temp, "\n");	
	OutputDebugString (temp);
	fflush(stdout);
}
#else
void dprintf (char* fmt, ...){}
#define dprintf if(0)dprintf
#endif

int skip_spaces (int index, char* data)
{
	if (data[index])
	{
		while(1)
		{
			char ch = data[index];
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
				++index;
			else
				break;
		}
	}
	else 
		index = -1;

	return index;
}

int skip_comment (int index, char* data)
{
	int updatedIndex = -1;
	const char startComment[] = "!--";
	if (!strncmp(&data[index], startComment, strlen(startComment)))
	{
		const char endComment[] = "-->";
		char* end = strstr(&data[index], endComment);
		if (end)
		{
			updatedIndex = end - data + strlen(endComment);
		}
	}

	return updatedIndex;
}

void freeTag (xmltag* ptag)
{
	if (ptag->child)
		freeTag(ptag->child);
	if (ptag->next)
		freeTag(ptag->next);
	
	while (ptag->attributes)
	{
		xmlattribute* next_attr = ptag->attributes->next_attr;
		free(ptag->attributes->name);
		if (ptag->attributes->value)
			free(ptag->attributes->value);
		free(ptag->attributes);		
		ptag->attributes = next_attr;
	}

	if (ptag->value)
		free(ptag->value);
	if (ptag->name)
		free(ptag->name);
}

int parseTag (char* data, int index, xmltag** pptag)
{
	int chstart, chend, chlen;
	xmltag* tag = (xmltag *)malloc(sizeof(xmltag)); //convert from 'void *' to 'xmltag *'
	if (!tag) return -1;
	dprintf("ParseTag (\"...\", int index: %d, xmltag** pptag: %08X", index, pptag);
	*pptag = tag;

	tag->attributes = NULL;
	tag->child = NULL;
	tag->next = NULL;
	tag->value = NULL;

	index = skip_spaces(index, data);
	if (data[index] != '<') { free(tag); return -1; }

	/* nome tag */
	++index;
	if (!isalpha(data[index])) { free(tag); return -1; }
	chstart = index;
	while (isalnum(data[index]) || data[index] == '_') ++index;
	chend = index;
	chlen = chend-chstart;
	tag->name = (char *) malloc(sizeof(char)*(chlen+1));
	memcpy(tag->name, data+chstart, chlen*sizeof(char));
	tag->name[chlen] = '\0';
	dprintf ("Found tag name: %s\n", tag->name);

	index = skip_spaces(index, data);	

	/* attributi */
check_attribute:
	if (isalpha(index[data]) || data[index] == '_') /* attributo */
	{
		xmlattribute* attr = (xmlattribute *) malloc(sizeof(xmlattribute));
		attr->name = NULL;
		attr->next_attr = NULL;
		attr->value = NULL;

		/* nome attributo */
		chstart = index;
		while (isalnum(data[index]) || data[index] == '_') ++index;
		chend = index;
		chlen = chend-chstart;		
		attr->name = (char *)malloc(sizeof(char)*(chlen+1));
		memcpy(attr->name, data+chstart, chlen*sizeof(char));
		attr->name[chlen] = '\0';
		index = skip_spaces(index, data);
		
		/* = */
		if (data[index] == '=')
		{
			char delim;
			xmlattribute* walk;
			++index;
			index = skip_spaces(index, data);
			
			/* cerca " */
			if (!(data[index] == '\"' || data[index] == '\'')) return -1;
			delim = data[index];
			++index;

			chstart = index;
			while (data[index] != delim) ++index;
			chend = index;
			chlen = chend-chstart;
			if (chlen)
			{
				attr->value = (char *)malloc(sizeof(char)*chlen+1);
				memcpy(attr->value, data+chstart, chlen);
				attr->value[chlen] = '\0';
			}
			++index;
			index = skip_spaces(index, data);

			if (!tag->attributes)			
				tag->attributes = attr;
			else
			{
				walk = tag->attributes;
				while (walk->next_attr) walk = walk->next_attr;
				walk->next_attr = attr;
			}
		}
		dprintf ("Found tag attribute %s=\"%s\"\n", attr->name, attr->value ? attr->value : "NULL");
		goto check_attribute;
	}

	if (data[index] == '/') /* fine tag */
	{
		*pptag = tag;
		++index;
		/* attende > */
		index = skip_spaces(index, data);
		if (data[index] != '>')
			return -1;
		++index;
		dprintf("End of tag %s (inline)\n", tag->name);
		return index;
	}

	if (data[index] == '>') /* tag chiuso, no fine */
	{	
		++index;
check_tags:	
		/* <tag>extract this value</tag> */
		index = skip_spaces(index, data);
		chstart = index;
		while (data[index] && data[index] != '<')
			++index;

		chend = index;
		if (chend != chstart)
		{
			chlen = chend - chstart;
			tag->value = (char *)malloc(chlen+1);
			memcpy(tag->value, data+chstart, sizeof(char)*chlen);
			tag->value[chlen] = '\0';
			dprintf ("Tag value: %s", tag->value);
		}
		else
			dprintf("Tag has no value\n");

		if (data[index] == '<')
		{
			++index;
			if (data[index] == '!')
			{
				index = skip_comment(index, data);
				if (index == -1)
				{
					free(tag);
					return -1;
				}
			}
			else if (data[index] != '/')
			{
				xmltag *child, *walk;
				dprintf("<%s> Found nested tag... parsing\n", tag->name);
				index = parseTag(data,index-1,&child);
				if (-1 == index) 
					return -1;
				if (tag->child)
				{
					walk = tag->child;
					while (walk->next) walk = walk->next;
					walk->next = child;
				}
				else
					tag->child = child;
			}
			else
			{				
				int chwrite = 0;
				char temp[256];
				++index; /* skip / */
				index = skip_spaces(index, data);
				while (isalnum(data[index]) || data[index] == '_')
					temp[chwrite++] = data[index++];
				temp[chwrite] = '\0';
				index = skip_spaces(index, data);
				if (data[index] != '>') return -1;
				++index;
				dprintf("Found end of tag %s: %s\n", tag->name, temp);
				if (strcmp(tag->name, temp)) return -1; /* </MISMATCH> */
				return index;
			}
		}
		goto check_tags;
	}
	return index;
}

extern void* hInstance;

char* readxml (const char* filename, int unpack)
{
	/* legge file xml */
	char* xmlfile = NULL;	
	int ccsize;		
	FILE* fin = fopen(filename, "r");
	if (fin)
	{
		fseek(fin, 0, SEEK_END);
		ccsize = ftell(fin) / sizeof(char);
		xmlfile = (char *)malloc((ccsize+1)*sizeof(char));
		fseek(fin, 0, SEEK_SET);
		fread(xmlfile, ccsize, sizeof(char), fin);
		xmlfile[ccsize] = '\0';

		fclose(fin);
	}
	return xmlfile;
}

char* unescape (char* str)
{
	int chscan, chout;
	char* quote = "&quot;";
	char* temp = (char *)malloc(strlen(str)+1);
	chscan = 0;
	chout = 0;

	for (;;)
	{
		if (!strncmp(str+chscan, quote, 6))
		{
			chscan += 6;
			temp[chout++] = '\"';			
		}
		else
			temp[chout++] = str[chscan++];

		if (!str[chscan])
		{
			temp[chout] = '\0';
			break;
		}
	}
	return temp;
}

xmltag* readXML (const char* fname, int unpack)
{
	xmltag* root = NULL;
	char* xml = readxml(fname, unpack);
	if (!xml) return NULL;
	if (-1 == parseTag(xml, 0, &root))
	{
		if (root)
			freeTag(root);
		root = NULL;
	}
	free(xml);
	return root;
}

