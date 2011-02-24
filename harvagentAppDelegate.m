//
//  harvagentAppDelegate.m
//  harvagent
//
//  Created by boyd yang on 2/23/11.
//  Copyright 2011 home. All rights reserved.
//

#import "harvagentAppDelegate.h"

@implementation harvagentAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
	[self retrieveData];
}

-(void)awakeFromNib{
	
	statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
	[statusItem setMenu:statusMenu];
	[statusItem setTitle:@"Photoshop"];
	[statusItem setHighlightMode:YES];
	
}

- (IBAction)menuWindow:(id)sender 
{
	printf("openWindow\n");
}

- (IBAction)menuPreferences:(id)sender
{
	printf("menuPreferences\n");
	[window makeKeyAndOrderFront:sender];

}
- (IBAction)menuHelp:(id)sender
{
	printf("menuHelp\n");
}
- (IBAction)menuQuit:(id)sender
{
	printf("menuQuit\n");
	[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

- (IBAction)menuDayEntry:(id)sender
{
	printf("menuDayEntry:%d\n",[sender tag]);
}
- (IBAction)btnDone:(NSButton *)sender
{
	[window orderOut:sender];
}

- (IBAction)btnCancel:(NSButton *)sender
{
	[window orderOut:sender];
}

- (void)menu:(NSMenu *)menu willHighlightItem:(NSMenuItem *)item
{
	printf("willHighlightItem:%d\n",[item tag]);
}
int getDayOffYear(NSDate *inDate)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	NSUInteger dayOfYear = [gregorian ordinalityOfUnit:NSDayCalendarUnit inUnit:NSYearCalendarUnit forDate:inDate];
	[gregorian release];
	return dayOfYear;
}

- (int) retrieveData
{
	char accountStr[]="searockcliff";
	char userStr[]="searockcliff@163.com";
	char passStr[]="harvestapp";
	HarvCon myConnect;
	UserDomain myDomain;
	myConnect.accountStr=accountStr;
	myDomain.userStr=userStr;
	myDomain.passStr=passStr;
	UserInfo myUserInfo={0};
	Daily todayDaily={0};
	Daily yesterdayDaily={0};
	Daily beforeDaily={0};
	
	//caculate days
	NSCalendar *cal = [NSCalendar currentCalendar];
	
	NSDate *date = [NSDate date];
	NSDateComponents *comps = [cal components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit) 
									 fromDate:date];
	NSDate *today = [cal dateFromComponents:comps];
	
	NSDateComponents *components1 = [[NSDateComponents alloc] init];
	[components1 setDay:-1];
	NSDate *yesterday = [cal dateByAddingComponents:components1 toDate:today options:0];

	NSDateComponents *compsYes = [cal components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit) 
									 fromDate:yesterday];
	int yearYes = [compsYes year];
	int dayYes = getDayOffYear(yesterday);
	
	NSDateFormatter* theDateFormatter = [[[NSDateFormatter alloc] init] autorelease];
	[theDateFormatter setFormatterBehavior:NSDateFormatterBehavior10_4];
	[theDateFormatter setDateFormat:@"EEEE"];
	NSString *weekDayYes =  [theDateFormatter stringFromDate:yesterday];
	
	
	NSDateComponents *components2 = [[NSDateComponents alloc] init];
	[components2 setDay:-2];
	NSDate *thedaybefore = [cal dateByAddingComponents:components2 toDate:today options:0];
	
	NSDateComponents *compsBef = [cal components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit) 
										fromDate:thedaybefore];
	int yearBef = [compsBef year];
	int dayBef = getDayOffYear(thedaybefore);
	NSString *weekDayBef =  [theDateFormatter stringFromDate:thedaybefore];
	
	//get data	
	harvest_login(&myConnect,&myDomain,&myUserInfo);
	
	harvest_getdaily(&myConnect,&todayDaily);
	
	int menuCnt=1;

	//for today
	int todayCnt = todayDaily.dayentryCount;
	if (todayCnt>0) {
		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:@"Today" action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = todayDaily.dayentryArray;
		while (curDayEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
			[newItem setTag:TODAY_MENU_BASE+curCnt];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}
	}

	
	//for yesterday
	harvest_getdayofyear(&myConnect,dayYes,yearYes,&yesterdayDaily);

	int yestodayCnt = yesterdayDaily.dayentryCount;
	if (yestodayCnt>0) {
		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:weekDayYes action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = yesterdayDaily.dayentryArray;
		while (curDayEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
			[newItem setTag:YESTERDAY_MENU_BASE+curCnt];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}
	}
	
	//for the day before yesterday
	harvest_getdayofyear(&myConnect,dayBef,yearBef,&beforeDaily);
	
	int beforeCnt = beforeDaily.dayentryCount;
	if (beforeCnt>0) {
		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:weekDayBef action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = beforeDaily.dayentryArray;
		while (curDayEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
			[newItem setTag:BEFORE_MENU_BASE+curCnt];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}
	}
	
	//for projects
	int projectCnt = todayDaily.projectCount;
	if (projectCnt>0) {
		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:@"Active Projects" action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		ProjectEntry *curProjEntry = todayDaily.projectArray;
		while (curProjEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%s (%s)",curProjEntry->name,curProjEntry->client];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
			//[newItem setTag:PROJECT_MENU_BASE+curCnt];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			
			int tkMenuCnt=0;
			int taskCnt = curProjEntry->taskCount;
			if (taskCnt>0) {
				NSMenu *curProjMenu = [[NSMenu alloc] initWithTitle:itemTitle];
				[curProjMenu setDelegate:self];
				[newItem setSubmenu:curProjMenu];
				TaskEntry *curTaskEntry = curProjEntry->taskArray;
				while (curTaskEntry) {
					NSString *itemTitle =[NSString stringWithFormat:@"%s",curTaskEntry->name];
					NSMenuItem *newTaskItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
					[newTaskItem setTag:PROJECT_MENU_BASE+curCnt*100+tkMenuCnt];
					//[newTaskItem setIndentationLevel:1];
					[curProjMenu insertItem:newTaskItem atIndex:tkMenuCnt];
					curTaskEntry=curTaskEntry->next;
					tkMenuCnt++;
				}
				
			}
			
			curProjEntry=curProjEntry->next;
			menuCnt++;
			curCnt++;
		}
	}
	
	/*
	
	
	harvest_cleandaily(&todayDaily);
	harvest_cleandaily(&yesterdayDaily);
	harvest_cleandaily(&beforeDaily);
	 */
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	harvest_logout(&myConnect);
	/*
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
	*/
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

@end
