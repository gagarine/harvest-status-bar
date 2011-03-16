/*
 *  harvest.h
 *  harvest
 *
 *  Created by boyd yang on 2/21/11.
 *  Copyright 2011 home. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define MAX_FIELD_LEN 256

typedef struct HarvCon {
	char accountStr[MAX_FIELD_LEN];
    CURL *curl;
	struct curl_slist *cookies;
} HarvCon, *HarvConPtr;

typedef struct UserDomain {
	char userStr[MAX_FIELD_LEN];
	char passStr[MAX_FIELD_LEN];
} UserDomain;

/*
<?xml version="1.0" encoding="UTF-8"?>
<hash>
<user>
<email>searockcliff@163.com</email>
<admin type="boolean">true</admin>
<timestamp-timers type="boolean">false</timestamp-timers>
<id type="integer">193640</id>
<timezone>Beijing</timezone>
</user>
</hash>
*/


typedef struct UserInfo {
    char email[MAX_FIELD_LEN];
	bool admin;
	bool timestamp_timers;
	int idV;
	char timezoneV[MAX_FIELD_LEN];
} UserInfo;

/*
 <day_entry>
 <id type="integer">40187062</id>
 <spent_at type="date">2011-02-20</spent_at>
 <user_id type="integer">193640</user_id>
 <client>home</client>
 <project_id>1086562</project_id>
 <project>Internal</project>
 <task_id>769557</task_id>
 <task>Admin</task>
 <hours type="float">10.01</hours>
 <notes>TEST1</notes>
 <created_at type="datetime">Sun, 20 Feb 2011 05:46:30 +0000</created_at>
 <updated_at type="datetime">Sun, 20 Feb 2011 05:47:04 +0000</updated_at>
 </day_entry> 
 */

typedef struct DayEntry {
	int idV;
	char spend_at[MAX_FIELD_LEN];
	int user_id;
	char client[MAX_FIELD_LEN];
	int project_id;
	char project[MAX_FIELD_LEN];
	int task_id;
	char task[MAX_FIELD_LEN];
	float hours;
	char notes[MAX_FIELD_LEN];
	char created_at[MAX_FIELD_LEN];
	char updated_at[MAX_FIELD_LEN];
	char started_at[MAX_FIELD_LEN];
	char ended_at[MAX_FIELD_LEN];
	char timer_started_at[MAX_FIELD_LEN];
	struct DayEntry *next;
} DayEntry;

/*
 <task>
 <name>Admin</name>
 <id type="integer">769557</id>
 <billable type="boolean">false</billable>
 </task>
*/
typedef struct TaskEntry {
	char name[MAX_FIELD_LEN];
	int idV;
	bool billabe;
	struct TaskEntry *next;
} TaskEntry;

/*
 <project>
 <name>Internal</name>
 <code></code>
 <id type="integer">1086562</id>
 <client>home</client>
 <tasks>
 <task>
 <name>Admin</name>
 <id type="integer">769557</id>
 <billable type="boolean">false</billable>
 </task>
...
 */
typedef struct ProjectEntry {
	char name[MAX_FIELD_LEN];
	int code;
	int idV;
	bool billabe;
	char client[MAX_FIELD_LEN];
	int taskCount;
	TaskEntry *taskArray;
	struct ProjectEntry *next;
} ProjectEntry;

typedef struct Daily {
	char for_day[MAX_FIELD_LEN];
	int dayentryCount;
	DayEntry *dayentryArray;
	int projectCount;
	ProjectEntry *projectArray;
} Daily;

/*
 <request>
 <notes>Test api support</notes>
 <hours>3</hours>
 <project_id type="integer">3</project_id>
 <task_id type="integer">14</task_id>
 <spent_at type="date">Tue, 17 Oct 2006</spent_at>
 </request> 
 */
typedef struct EntryRequest {
	int entry_id;
	char notes[MAX_FIELD_LEN];
	float hours;
	int project_id;
	int task_id;
	char spent_at[MAX_FIELD_LEN];
} EntryRequest;

/*
 <timer>
 <!-- new entry -->
 <day_entry>
 <id type="integer">195168</id>
 <client>Iridesco</client>
 <project>Harvest</project>
 <task>Backend Programming</task>
 <hours>0.00</hours>
 <notes>Test api support</notes>
 
 <!-- OPTIONAL returned only if a timer was started -->
 <timer_started_at type="datetime">
 Wed, 17 Oct 2006 10:45:07 +0000
 </timer_started_at>
 </day_entry>
 
 <hours_for_previously_running_timer type="float">
 0.87
 </hours_for_previously_running_timer>
 </timer>
 */
typedef struct Timer {
	DayEntry newEntry;
	float hours_for_previously_running_timer;
} Timer;


int harvest_login(HarvCon *hConnect,UserDomain *pDomain, UserInfo *pUserInfo);
int harvest_getdaily(HarvCon *hConnect, Daily *pDaily);
int harvest_getdayofyear(HarvCon *hConnect, int day, int year, Daily *pDaily);
int harvest_cleandaily(Daily *pDaily);
int harvest_toggletimer(HarvCon *hConnect,int entryId,Timer *pTimer);
int harvest_addentry(HarvCon *hConnect,EntryRequest *pRequest,Timer *pTimer);
int harvest_updateentry(HarvCon *hConnect,EntryRequest *pRequest,Timer *pTimer);
int harvest_deleteentry(HarvCon *hConnect,int entryId);
int harvest_logout(HarvCon *hConnect);
