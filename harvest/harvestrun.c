/*
 *  harvestrun.c
 *  harvest
 *
 *  Created by boyd yang on 2/25/11.
 *  Copyright 2011 home. All rights reserved.
 *
 */

#include "harvest.h"

int main(void)
{
	char accountStr[]="searockcliff";
	char userStr[]="searockcliff@163.com";
	char passStr[]="harvestapp";
	HarvCon myConnect;
	UserDomain myDomain;
	//myConnect.accountStr=accountStr;
	snprintf(myConnect.accountStr,sizeof(myConnect.accountStr),"%s",accountStr);
	snprintf(myDomain.userStr,sizeof(myDomain.userStr),"%s",userStr);
	snprintf(myDomain.passStr,sizeof(myDomain.passStr),"%s",passStr);

	UserInfo myUserInfo={0};
	Daily myDaily={0};
	harvest_login(&myConnect,&myDomain,&myUserInfo);
	harvest_getdaily(&myConnect,&myDaily);
	harvest_cleandaily(&myDaily);
	
	EntryRequest myRequest={0};
	myRequest.entry_id=0;
	snprintf(myRequest.notes,sizeof(myRequest.notes),"%s","new notes add");
	myRequest.hours=3.3;
	myRequest.project_id=1090781;
	myRequest.task_id=769557;
	snprintf(myRequest.spent_at,sizeof(myRequest.spent_at),"%s","Sun, 20 Feb 2011");
	
	Timer myTimer={0};
	harvest_addentry(&myConnect, &myRequest, &myTimer);
	
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	
	harvest_toggletimer(&myConnect, myTimer.newEntry.idV,&myTimer);
	
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	
	myRequest.entry_id=myTimer.newEntry.idV;
	snprintf(myRequest.notes,sizeof(myRequest.notes),"%s","new notes update");
	myRequest.hours=4.4;
	myRequest.project_id=1090781;
	myRequest.task_id=769557;
	harvest_updateentry(&myConnect, &myRequest,&myTimer);
	
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	
	harvest_deleteentry(&myConnect,myTimer.newEntry.idV);
	
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	
	harvest_logout(&myConnect);
	
	/*
	 harvest_login();
	 //harvest_toggletimer("40187181");
	 //harvest_addentry();
	 //harvest_deleteentry("40187181");
	 harvest_updateentry("40187462");
	 harvest_getdaily();
	 harvest_logout();
	 */
	return 0;
}
