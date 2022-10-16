/*-----------------------------------------------------------------------------

Copyright (c) 2006 AraldFx
"used on limited license" 2007 NUSofting

-----------------------------------------------------------------------------*/

#pragma once

typedef struct xmlattribute_tag xmlattribute;
typedef struct xmltag_tag xmltag;

struct xmlattribute_tag
{
	char* name;
	char* value;
	struct xmlattribute_tag* next_attr; //pointer to another itself // <-self reference
};

struct xmltag_tag
{
	xmlattribute* attributes;
	xmltag* next;  //pointer to another itself // <-self reference
	xmltag* child; //pointer to another itself // <-self reference
	char* name;
	char* value;
};
#ifdef __cplusplus
extern "C"
{
#endif
xmltag* readXML (const char* fname, int unpack);
void freeTag (xmltag* ptag);
#ifdef __cplusplus
}
#endif
