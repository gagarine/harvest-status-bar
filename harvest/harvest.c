/*
 *  harvest.c
 *  harvest
 *
 *  Created by boyd yang on 2/21/11.
 *  Copyright 2011 home. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "harvest.h"


typedef struct recvBufStruct {
    char buff[40960];
    int recvSize;
} recvBufStruct, *recvBufStructPtr;


static size_t write_data(void *ptr, size_t size, size_t nmemb, recvBufStruct *stream)
{
	int nLen = size*nmemb;
	memcpy(stream->buff+stream->recvSize,ptr,nLen);
	stream->recvSize+=nLen;
	return nLen;
}

static void
print_cookies(CURL *curl)
{
	CURLcode res;
	struct curl_slist *cookies;
	struct curl_slist *nc;
	int i;
	
	printf("Cookies, curl knows:\n");
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		exit(1);
	}
	nc = cookies, i = 1;
	while (nc) {
		printf("[%d]: %s\n", i, nc->data);
		nc = nc->next;
		i++;
	}
	if (i == 1) {
		printf("(none)\n");
	}
	curl_slist_free_all(cookies);
}

int harvest_login(HarvCon *hConnect,UserDomain *pDomain, UserInfo *pUserInfo)
{
	CURLcode res;
	CURL *curl;
	//struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;

	curl = curl_easy_init();
	if(!curl) {
		return 1;
	}
	struct curl_slist *chunk = NULL;
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	char userpass[256]={0};
	
	chunk = curl_slist_append(chunk, "Accept: application/xml");
	chunk = curl_slist_append(chunk, "Content-Type: application/xml");
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/account/who_am_i",accountStr);
	snprintf(userpass, sizeof(userpass), "%s:%s",pDomain->userStr,pDomain->passStr);
	
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
	
	/* redo request with our own custom Accept: */
	res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
	
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);		
	/* set user name and password for the authentication */
	curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);
	
	/* no progress meter please */
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""); /* just to start the cookie engine */
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &hConnect->cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	hConnect->curl=curl;
	
	//print_cookies(curl);
	
	/* always cleanup */
	//curl_easy_cleanup(curl);
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		//login fail, body:Authentication failed for API request.
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	xmlNodePtr cur2 = cur1->xmlChildrenNode;
	while (cur2 != NULL) {
		if (!strcmp((char *)cur2->name, "user"))// && (cur->ns == ns))
		{
			xmlNodePtr cur = cur2->xmlChildrenNode;
			while (cur != NULL) {
				//printf("name:%s\n",cur->name);
				
				if (!strcmp((char *)cur->name, "id"))// && (cur->ns == ns))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					//printf("id:%s\n",tValue);
					pUserInfo->idV = atoi((char *)tValue);;
					xmlFree(tValue); 
					
					xmlChar *tpCh = xmlGetProp(cur, (xmlChar *)"type"); 
					//printf("type:%s\n", tpCh); 
					xmlFree(tpCh); 
					
				}
				else if (!strcmp((char *)cur->name, "email"))// && (cur->ns == ns))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					//printf("email:%s\n",tValue);
					snprintf(pUserInfo->email, sizeof(pUserInfo->email), "%s",tValue);

					xmlFree(tValue); 
				}
				else if (!strcmp((char *)cur->name, "admin"))// && (cur->ns == ns))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					//printf("admin:%s\n",tValue);
					pUserInfo->admin=!strcmp((char *)tValue, "true")?true:false;
					xmlFree(tValue); 
				}
				else if (!strcmp((char *)cur->name, "timezone"))// && (cur->ns == ns))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					//printf("timezone:%s\n",tValue);
					snprintf(pUserInfo->timezoneV, sizeof(pUserInfo->timezoneV), "%s",tValue);
					xmlFree(tValue); 
				}
				else if (!strcmp((char *)cur->name, "timestamp-timers"))// && (cur->ns == ns))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					//printf("timestamp-timers:%s\n",tValue);
					pUserInfo->timestamp_timers=!strcmp((char *)tValue, "true")?true:false;
					xmlFree(tValue); 
				}
				cur = cur->next;
			}
		}
		cur2 = cur2->next;
	}
	
	
	
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
}

int harvest_getdaily(HarvCon *hConnect, Daily *pDaily)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily",accountStr);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);	
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	while (cur1!=NULL) 
	{
		if (!strcmp((char *)cur1->name, "daily"))
		{
			xmlNodePtr cur2 = cur1->xmlChildrenNode;
			while (cur2 != NULL) 
			{
				if (!strcmp((char *)cur2->name, "for_day"))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					snprintf(pDaily->for_day, sizeof(pDaily->for_day), "%s",tValue);
					xmlFree(tValue); 
				}
				if (!strcmp((char *)cur2->name, "day_entries"))
				{
					xmlNodePtr cur3 = cur2->xmlChildrenNode;
					DayEntry *previousEntry=pDaily->dayentryArray;
					while (cur3 != NULL) 
					{
						if (!strcmp((char *)cur3->name, "day_entry"))
						{
							xmlNodePtr cur4 = cur3->xmlChildrenNode;
							pDaily->dayentryCount++;
							DayEntry *curDayEntry = malloc(sizeof(DayEntry));
							memset(curDayEntry,0,sizeof(DayEntry));
							while (cur4 != NULL) 
							{
								if (!strcmp((char *)cur4->name, "id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->idV = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "spent_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->spend_at, sizeof(curDayEntry->spend_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "user_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->user_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "client"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->client, sizeof(curDayEntry->client), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "project_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->project_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "project"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->project, sizeof(curDayEntry->project), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "task_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->task_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "task"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->task, sizeof(curDayEntry->task), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "hours"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->hours = atof((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "notes"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->notes, sizeof(curDayEntry->notes), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "created_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->created_at, sizeof(curDayEntry->created_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "updated_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->updated_at, sizeof(curDayEntry->notes), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "started_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->started_at, sizeof(curDayEntry->started_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "ended_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->ended_at, sizeof(curDayEntry->ended_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "timer_started_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->timer_started_at, sizeof(curDayEntry->timer_started_at), "%s",tValue);
									xmlFree(tValue); 
								}
								cur4=cur4->next;
							}
							//add to link
							curDayEntry->next=NULL;
							if (pDaily->dayentryArray==NULL) {
								pDaily->dayentryArray=curDayEntry;
								previousEntry = curDayEntry;
							}
							else {
								previousEntry->next=curDayEntry;
								previousEntry=curDayEntry;
							}

						}
						cur3=cur3->next;
					}

				}
				else if (!strcmp((char *)cur2->name, "projects"))
				{
					xmlNodePtr cur3 = cur2->xmlChildrenNode;
					ProjectEntry *previousProject = pDaily->projectArray;
					while (cur3 != NULL) 
					{
						if (!strcmp((char *)cur3->name, "project"))
						{
							pDaily->projectCount++;
							ProjectEntry *curProject = malloc(sizeof(ProjectEntry));
							memset(curProject,0,sizeof(ProjectEntry));
							xmlNodePtr cur4 = cur3->xmlChildrenNode;
							while (cur4 != NULL) 
							{
								if (!strcmp((char *)cur4->name, "id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curProject->idV = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "code"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									if (tValue) {
										curProject->code = atoi((char *)tValue);
										xmlFree(tValue); 
									}

								}
								else if (!strcmp((char *)cur4->name, "name"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curProject->name, sizeof(curProject->name), "%s",tValue);
									xmlFree(tValue); 
								}								
								else if (!strcmp((char *)cur4->name, "client"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curProject->client, sizeof(curProject->client), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "tasks"))
								{
									xmlNodePtr cur5 = cur4->xmlChildrenNode;
									TaskEntry *previousTask = curProject->taskArray;
									while (cur5 != NULL) 
									{
										if (!strcmp((char *)cur5->name, "task"))
										{
											curProject->taskCount++;
											TaskEntry *curTask = malloc(sizeof(TaskEntry));
											memset(curTask,0,sizeof(TaskEntry));
											xmlNodePtr cur6 = cur5->xmlChildrenNode;
											while (cur6 != NULL) 
											{
												if (!strcmp((char *)cur6->name, "id"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc, cur6->xmlChildrenNode, 1);
													curTask->idV = atoi((char *)tValue);
													xmlFree(tValue); 
												}
												else if (!strcmp((char *)cur6->name, "name"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc,cur6->xmlChildrenNode, 1);
													snprintf(curTask->name, sizeof(curTask->name), "%s",tValue);
													xmlFree(tValue); 
												}
												else if (!strcmp((char *)cur6->name, "billable"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc, cur6->xmlChildrenNode, 1);
													curTask->billabe=!strcmp((char *)tValue, "true")?true:false;
													xmlFree(tValue); 
												}
												cur6=cur6->next;
											}
											//add to link
											curTask->next=NULL;
											if (curProject->taskArray==NULL) {
												curProject->taskArray=curTask;
												previousTask = curTask;
											}
											else {
												previousTask->next=curTask;
												previousTask=curTask;
											}
										}
										cur5=cur5->next;
									}
								}
								cur4=cur4->next;
							}
							//add to link
							curProject->next=NULL;
							if (pDaily->projectArray==NULL) {
								pDaily->projectArray=curProject;
								previousProject = curProject;
							}
							else {
								previousProject->next=curProject;
								previousProject=curProject;
							}
						}
						cur3=cur3->next;
					}
					
				}

				cur2=cur2->next;
			}
		}
		cur1=cur1->next;		
	}
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
	
}

int harvest_getdayofyear(HarvCon *hConnect, int day, int year, Daily *pDaily)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily/%d/%d",accountStr,day,year);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);	
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	while (cur1!=NULL) 
	{
		if (!strcmp((char *)cur1->name, "daily"))
		{
			xmlNodePtr cur2 = cur1->xmlChildrenNode;
			while (cur2 != NULL) 
			{
				if (!strcmp((char *)cur2->name, "for_day"))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					snprintf(pDaily->for_day, sizeof(pDaily->for_day), "%s",tValue);
					xmlFree(tValue); 
				}
				if (!strcmp((char *)cur2->name, "day_entries"))
				{
					xmlNodePtr cur3 = cur2->xmlChildrenNode;
					DayEntry *previousEntry=pDaily->dayentryArray;
					while (cur3 != NULL) 
					{
						if (!strcmp((char *)cur3->name, "day_entry"))
						{
							xmlNodePtr cur4 = cur3->xmlChildrenNode;
							pDaily->dayentryCount++;
							DayEntry *curDayEntry = malloc(sizeof(DayEntry));
							memset(curDayEntry,0,sizeof(DayEntry));
							while (cur4 != NULL) 
							{
								if (!strcmp((char *)cur4->name, "id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->idV = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "spent_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->spend_at, sizeof(curDayEntry->spend_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "user_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->user_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "client"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->client, sizeof(curDayEntry->client), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "project_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->project_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "project"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->project, sizeof(curDayEntry->project), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "task_id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->task_id = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "task"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->task, sizeof(curDayEntry->task), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "hours"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curDayEntry->hours = atof((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "notes"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->notes, sizeof(curDayEntry->notes), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "created_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->created_at, sizeof(curDayEntry->created_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "updated_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->updated_at, sizeof(curDayEntry->notes), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "started_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->started_at, sizeof(curDayEntry->started_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "ended_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->ended_at, sizeof(curDayEntry->ended_at), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "timer_started_at"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curDayEntry->timer_started_at, sizeof(curDayEntry->timer_started_at), "%s",tValue);
									xmlFree(tValue); 
								}
								cur4=cur4->next;
							}
							//add to link
							curDayEntry->next=NULL;
							if (pDaily->dayentryArray==NULL) {
								pDaily->dayentryArray=curDayEntry;
								previousEntry = curDayEntry;
							}
							else {
								previousEntry->next=curDayEntry;
								previousEntry=curDayEntry;
							}
							
						}
						cur3=cur3->next;
					}
					
				}
				else if (!strcmp((char *)cur2->name, "projects"))
				{
					xmlNodePtr cur3 = cur2->xmlChildrenNode;
					ProjectEntry *previousProject = pDaily->projectArray;
					while (cur3 != NULL) 
					{
						if (!strcmp((char *)cur3->name, "project"))
						{
							pDaily->projectCount++;
							ProjectEntry *curProject = malloc(sizeof(ProjectEntry));
							memset(curProject,0,sizeof(ProjectEntry));
							xmlNodePtr cur4 = cur3->xmlChildrenNode;
							while (cur4 != NULL) 
							{
								if (!strcmp((char *)cur4->name, "id"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									curProject->idV = atoi((char *)tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "code"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
									if (tValue) {
										curProject->code = atoi((char *)tValue);
										xmlFree(tValue); 
									}
									
								}
								else if (!strcmp((char *)cur4->name, "name"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curProject->name, sizeof(curProject->name), "%s",tValue);
									xmlFree(tValue); 
								}								
								else if (!strcmp((char *)cur4->name, "client"))
								{
									xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
									snprintf(curProject->client, sizeof(curProject->client), "%s",tValue);
									xmlFree(tValue); 
								}
								else if (!strcmp((char *)cur4->name, "tasks"))
								{
									xmlNodePtr cur5 = cur4->xmlChildrenNode;
									TaskEntry *previousTask = curProject->taskArray;
									while (cur5 != NULL) 
									{
										if (!strcmp((char *)cur5->name, "task"))
										{
											curProject->taskCount++;
											TaskEntry *curTask = malloc(sizeof(TaskEntry));
											memset(curTask,0,sizeof(TaskEntry));
											xmlNodePtr cur6 = cur5->xmlChildrenNode;
											while (cur6 != NULL) 
											{
												if (!strcmp((char *)cur6->name, "id"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc, cur6->xmlChildrenNode, 1);
													curTask->idV = atoi((char *)tValue);
													xmlFree(tValue); 
												}
												else if (!strcmp((char *)cur6->name, "name"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc,cur6->xmlChildrenNode, 1);
													snprintf(curTask->name, sizeof(curTask->name), "%s",tValue);
													xmlFree(tValue); 
												}
												else if (!strcmp((char *)cur6->name, "billable"))
												{
													xmlChar *tValue = xmlNodeListGetString(doc, cur6->xmlChildrenNode, 1);
													curTask->billabe=!strcmp((char *)tValue, "true")?true:false;
													xmlFree(tValue); 
												}
												cur6=cur6->next;
											}
											//add to link
											curTask->next=NULL;
											if (curProject->taskArray==NULL) {
												curProject->taskArray=curTask;
												previousTask = curTask;
											}
											else {
												previousTask->next=curTask;
												previousTask=curTask;
											}
										}
										cur5=cur5->next;
									}
								}
								cur4=cur4->next;
							}
							//add to link
							curProject->next=NULL;
							if (pDaily->projectArray==NULL) {
								pDaily->projectArray=curProject;
								previousProject = curProject;
							}
							else {
								previousProject->next=curProject;
								previousProject=curProject;
							}
						}
						cur3=cur3->next;
					}
					
				}
				
				cur2=cur2->next;
			}
		}
		cur1=cur1->next;		
	}
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
	
}

int harvest_cleandaily(Daily *pDaily)
{
	if (pDaily==NULL) {
		return 0;
	}
	DayEntry *curDayEntry = pDaily->dayentryArray;
	while (curDayEntry!=NULL) {
		DayEntry *nextDayEntry=curDayEntry->next;
		free(curDayEntry);
		curDayEntry=nextDayEntry;
	}
	ProjectEntry *curProjectEntry = pDaily->projectArray;
	while (curProjectEntry!=NULL) {
		ProjectEntry *nextProjectEntry=curProjectEntry->next;
		
		TaskEntry *curTaskEntry=curProjectEntry->taskArray;
		while (curTaskEntry!=NULL) {
			TaskEntry *nextTaskEntry=curTaskEntry->next;
			free(curTaskEntry);
			curTaskEntry=nextTaskEntry;
		}
		
		free(curProjectEntry);
		curProjectEntry=nextProjectEntry;
	}
	memset(pDaily,0,sizeof(Daily));
	return 0;
}

int harvest_toggletimer(HarvCon *hConnect,int entryId,Timer *pTimer)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily/timer/%d",accountStr,entryId);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);
	
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	while (cur1!=NULL) 
	{
		if (!strcmp((char *)cur1->name, "timer"))
		{
			xmlNodePtr cur2 = cur1->xmlChildrenNode;
			while (cur2 != NULL) 
			{
				
				if (!strcmp((char *)cur2->name, "day_entry"))
				{
					xmlNodePtr cur4 = cur2->xmlChildrenNode;
					DayEntry *curDayEntry = &(pTimer->newEntry);
					memset(curDayEntry,0,sizeof(DayEntry));
					while (cur4 != NULL) 
					{
						if (!strcmp((char *)cur4->name, "id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->idV = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "spent_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->spend_at, sizeof(curDayEntry->spend_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "user_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->user_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "client"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->client, sizeof(curDayEntry->client), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->project_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->project, sizeof(curDayEntry->project), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->task_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->task, sizeof(curDayEntry->task), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "hours"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->hours = atof((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "notes"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->notes, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "created_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->created_at, sizeof(curDayEntry->created_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "updated_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->updated_at, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->started_at, sizeof(curDayEntry->started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "ended_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->ended_at, sizeof(curDayEntry->ended_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "timer_started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->timer_started_at, sizeof(curDayEntry->timer_started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						cur4=cur4->next;
					}							
				}
				else if (!strcmp((char *)cur2->name, "hours_for_previously_running_timer"))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					pTimer->hours_for_previously_running_timer = atof((char *)tValue);
					xmlFree(tValue); 					
				}				
				cur2=cur2->next;
			}
		}
		cur1=cur1->next;		
	}
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	printf("response:%ld\n",respcode);
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
}

int harvest_addentry(HarvCon *hConnect,EntryRequest *pRequest,Timer *pTimer)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily/add",accountStr);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);
	
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	/* Now specify the POST data */
	char postData[1024]={0};
	snprintf(postData,sizeof(postData),"<request><notes>%s</notes><hours>%f</hours><project_id type=\"integer\">%d</project_id><task_id type=\"integer\">%d</task_id><spent_at type=\"date\">%s</spent_at></request>",
			 pRequest->notes,
			 pRequest->hours,
			 pRequest->project_id,
			 pRequest->task_id,
			 pRequest->spent_at);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	while (cur1!=NULL) 
	{
		if (!strcmp((char *)cur1->name, "add"))
		{
			xmlNodePtr cur2 = cur1->xmlChildrenNode;
			while (cur2 != NULL) 
			{

				if (!strcmp((char *)cur2->name, "day_entry"))
				{
					xmlNodePtr cur4 = cur2->xmlChildrenNode;
					DayEntry *curDayEntry = &(pTimer->newEntry);
					memset(curDayEntry,0,sizeof(DayEntry));
					while (cur4 != NULL) 
					{
						if (!strcmp((char *)cur4->name, "id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->idV = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "spent_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->spend_at, sizeof(curDayEntry->spend_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "user_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->user_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "client"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->client, sizeof(curDayEntry->client), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->project_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->project, sizeof(curDayEntry->project), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->task_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->task, sizeof(curDayEntry->task), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "hours"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->hours = atof((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "notes"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->notes, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "created_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->created_at, sizeof(curDayEntry->created_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "updated_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->updated_at, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->started_at, sizeof(curDayEntry->started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "ended_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->ended_at, sizeof(curDayEntry->ended_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "timer_started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->timer_started_at, sizeof(curDayEntry->timer_started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						cur4=cur4->next;
					}							
				}
				else if (!strcmp((char *)cur2->name, "hours_for_previously_running_timer"))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					pTimer->hours_for_previously_running_timer = atof((char *)tValue);
					xmlFree(tValue); 					
				}				
				cur2=cur2->next;
			}
		}
		cur1=cur1->next;		
	}
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
	
}


int harvest_updateentry(HarvCon *hConnect,EntryRequest *pRequest,Timer *pTimer)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily/update/%d",accountStr,pRequest->entry_id);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);
		
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	/* Now specify the POST data */
	//char postData[]="<request><notes>New notes post</notes><hours>1.07</hours><spent_at type=\"date\">Sun, 20 Feb 2011</spent_at><project_id>1090781</project_id><task_id>769559</task_id></request>";
    char postData[1024]={0};
	snprintf(postData,sizeof(postData),"<request><notes>%s</notes><hours>%f</hours><project_id type=\"integer\">%d</project_id><task_id type=\"integer\">%d</task_id><spent_at type=\"date\">%s</spent_at></request>",
			 pRequest->notes,
			 pRequest->hours,
			 pRequest->project_id,
			 pRequest->task_id,
			 pRequest->spent_at);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	xmlDocPtr doc; /* the resulting document tree */
	
	/*
	 * The document being in memory, it have no base per RFC 2396,
	 * and the "noname.xml" argument will serve as its base.
	 */
	doc = xmlReadMemory(httpBuff.buff, httpBuff.recvSize, "noname.xml", NULL, 0);
	if (doc == NULL) {
		fprintf(stderr, "Failed to parse document\n");
		return 1;
	}
	
	xmlNodePtr cur1 = doc->xmlChildrenNode;
	while (cur1!=NULL) 
	{
		if (!strcmp((char *)cur1->name, "add"))
		{
			xmlNodePtr cur2 = cur1->xmlChildrenNode;
			while (cur2 != NULL) 
			{
				
				if (!strcmp((char *)cur2->name, "day_entry"))
				{
					xmlNodePtr cur4 = cur2->xmlChildrenNode;
					DayEntry *curDayEntry = &(pTimer->newEntry);
					memset(curDayEntry,0,sizeof(DayEntry));
					while (cur4 != NULL) 
					{
						if (!strcmp((char *)cur4->name, "id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->idV = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "spent_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->spend_at, sizeof(curDayEntry->spend_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "user_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->user_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "client"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->client, sizeof(curDayEntry->client), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->project_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "project"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->project, sizeof(curDayEntry->project), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task_id"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->task_id = atoi((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "task"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->task, sizeof(curDayEntry->task), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "hours"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc, cur4->xmlChildrenNode, 1);
							curDayEntry->hours = atof((char *)tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "notes"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->notes, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "created_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->created_at, sizeof(curDayEntry->created_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "updated_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->updated_at, sizeof(curDayEntry->notes), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->started_at, sizeof(curDayEntry->started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "ended_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->ended_at, sizeof(curDayEntry->ended_at), "%s",tValue);
							xmlFree(tValue); 
						}
						else if (!strcmp((char *)cur4->name, "timer_started_at"))
						{
							xmlChar *tValue = xmlNodeListGetString(doc,cur4->xmlChildrenNode, 1);
							snprintf(curDayEntry->timer_started_at, sizeof(curDayEntry->timer_started_at), "%s",tValue);
							xmlFree(tValue); 
						}
						cur4=cur4->next;
					}							
				}
				else if (!strcmp((char *)cur2->name, "hours_for_previously_running_timer"))
				{
					xmlChar *tValue = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					pTimer->hours_for_previously_running_timer = atof((char *)tValue);
					xmlFree(tValue); 					
				}				
				cur2=cur2->next;
			}
		}
		cur1=cur1->next;		
	}
	
	xmlFreeDoc(doc);
	
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
	/*
	 * this is to debug memory for regression tests
	 */
	xmlMemoryDump();
	
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
	
}

int harvest_deleteentry(HarvCon *hConnect,int entryId)
{
	CURLcode res;
	CURL *curl;
	struct curl_slist *cookies;
	char *accountStr=hConnect->accountStr;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	//get daily
	if(!curl) {
		return 1;
	}
	recvBufStruct httpBuff={0};
	recvBufStruct headBuff={0};
	char urlStr[256]={0};
	
	snprintf(urlStr, sizeof(urlStr), "https://%s.harvestapp.com/daily/delete/%d",accountStr,entryId);
	
	/* request with the built-in Accept: */
	curl_easy_setopt(curl, CURLOPT_URL, urlStr);
	
	
	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	
	curl_easy_setopt(curl,   CURLOPT_WRITEDATA, &httpBuff);
	
	/* we want the headers to this file handle */
	curl_easy_setopt(curl,   CURLOPT_WRITEHEADER, &headBuff);
	
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, cookies);
	
	curl_easy_setopt(curl,   CURLOPT_CUSTOMREQUEST, "DELETE");
	
	res = curl_easy_perform(curl);
	
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
		return 1;
	}
	long respcode;
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &respcode);
	printf("response:%ld\n",respcode);
	
	printf("head:%s\n",headBuff.buff);
	printf("body:%s\n",httpBuff.buff);
	
	if (respcode != 200) {
		return 1;
	}
	return 0;
}

int harvest_logout(HarvCon *hConnect)
{
	CURL *curl;
	struct curl_slist *cookies;
	
	curl = hConnect->curl;
	cookies = hConnect->cookies;	
	
	/* always cleanup */
	curl_easy_cleanup(curl);
	curl_slist_free_all(cookies);
	return 0;
}


