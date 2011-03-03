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
@synthesize noteStr;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
	[self retrieveData];
}

-(void)awakeFromNib{
	timerFlag=FALSE;
	stoppedDayEntry=0;
	hideLoginFlag=FALSE;
	activeDayEntry=NULL;
	
	statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
	//[statusItem setMenu:statusMenu];
	[statusItem setTitle:@"Connecting..."];
	[statusItem setHighlightMode:YES];
	
	[statusItem setAction:@selector(popMenu:)];
	[statusItem setTarget:self];
	NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(applicationDidActivate:) name:NSApplicationDidBecomeActiveNotification object:app];
}

-(IBAction)showMenu:(id)sender
{
	NSLog(@"showMenu");
	[statusItem popUpStatusItemMenu:statusMenu];
}

-(IBAction)applicationDidActivate:(id)sender
{
	[self showMenu:self];	
}

-(IBAction)popMenu:(id)sender
{
	NSLog(@"popMenu");
	if ([app isActive]) {
		[self showMenu:self];
	} else {
		[app activateIgnoringOtherApps:YES];
	}
}


- (IBAction)menuProfile:(id)sender 
{
	NSString *urlStr = [NSString stringWithFormat:@"https://%@.harvestapp.com",accountStr];
	NSURL *someURL = [NSURL URLWithString:urlStr];
	if ([[NSWorkspace sharedWorkspace] openURL:someURL])
		printf("Opened successfully.\n");
	else
		printf("Failed to open URL.\n");
}

- (IBAction)menuPreferences:(id)sender
{
	printf("menuPreferences\n");
	[window makeKeyAndOrderFront:sender];
	[window center];
	
	NSArray *accountAry = [[NSUserDefaults standardUserDefaults] objectForKey:@"com.havagent.simon"];
	if ((accountAry!=NULL)||([accountAry count]==3)) {
		[accountText setStringValue:[accountAry objectAtIndex:0]];
		[mailText setStringValue:[accountAry objectAtIndex:1]];
		[passText setStringValue:[accountAry objectAtIndex:2]];
	}
	hideLoginFlag = [[NSUserDefaults standardUserDefaults] boolForKey:@"com.havagent.simonshow"];
	if (!hideLoginFlag) {
		[showLoginBtn setState:NSOnState];
	}
	else {
		[showLoginBtn setState:NSOffState];
	}

	
}

- (IBAction)menuHelp:(id)sender
{
	printf("menuHelp\n");
	NSString *urlStr = [NSString stringWithFormat:@"https://www.getharvest.com/help"];
	NSURL *someURL = [NSURL URLWithString:urlStr];
	if ([[NSWorkspace sharedWorkspace] openURL:someURL])
		printf("Opened successfully.\n");
	else
		printf("Failed to open URL.\n");
	
}
- (IBAction)menuQuit:(id)sender
{
	printf("menuQuit\n");
	harvest_logout(&myConnect);
	[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

- (IBAction)menuDayEntry:(id)sender
{
	printf("menuDayEntry:%d\n",[sender tag]);
	
	Timer retTimer={0};
	
	harvest_toggletimer(&myConnect,[sender tag],&retTimer);	
	[self clearMenu];
	[self retrieveData];
}

- (IBAction)menuOldDayEntry:(id)sender
{
	printf("menuOldDayEntry:%d\n",[sender tag]);
	
}

- (IBAction)menuProjectTask:(id)sender
{
	printf("menuProjectTask:%d\n",[sender tag]);
	[self addDayEntry:curMenuTag];
	
}

- (IBAction)btnConnect:(NSButton *)sender
{
	accountStr = [accountText stringValue];
	NSString *mailStr=[mailText stringValue];
	NSString *passStr=[passText stringValue];
	
	accountStr=[accountStr stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	if ([accountStr length]==0) {
		NSAlert* alert = [NSAlert alertWithMessageText: @"Invalid Input"
										 defaultButton: @"OK"
									   alternateButton: nil
										   otherButton: nil
							 informativeTextWithFormat: @"Please input correct account information."];
		[alert runModal];
		return;
	}
	
	mailStr=[mailStr stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	if ([mailStr length]==0) {
		NSAlert* alert = [NSAlert alertWithMessageText: @"Invalid Input"
										 defaultButton: @"OK"
									   alternateButton: nil
										   otherButton: nil
							 informativeTextWithFormat: @"Please input correct mail information."];
		[alert runModal];
		return;
	}
	
	passStr=[passStr stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
	if ([passStr length]==0) {
		NSAlert* alert = [NSAlert alertWithMessageText: @"Invalid Input"
										 defaultButton: @"OK"
									   alternateButton: nil
										   otherButton: nil
							 informativeTextWithFormat: @"Please input correct password information."];
		[alert runModal];
		return;
	}
	
	[progressInd startAnimation:sender];
	
	NSArray *accountAry = [NSArray arrayWithObjects:accountStr,mailStr,passStr,nil];
	[[NSUserDefaults standardUserDefaults] setObject:accountAry forKey:@"com.havagent.simon"];
	[self clearMenu];
	[self retrieveData];

	[progressInd stopAnimation:sender];
	[window orderOut:self];
}

- (IBAction)btnShowLogin:(NSButton *)sender
{
	if ([sender state]==NSOnState) {
		hideLoginFlag=FALSE;
	}
	else {
		hideLoginFlag=TRUE;
	}
	
	[[NSUserDefaults standardUserDefaults] setBool:hideLoginFlag forKey:@"com.havagent.simonshow"];
	
}

- (IBAction)btnStartStopTimer:(NSButton *)sender
{
	Timer retTimer={0};
	
	harvest_toggletimer(&myConnect,activeDayEntry->idV,&retTimer);	
	
	if (timerFlag) {
		//stoppedDayEntry=activeDayEntry->idV;
		[timerBtn setTitle:@"Start Timer"];
	}
	else {
		[timerBtn setTitle:@"Stop Timer"];
	}
	timerFlag=!timerFlag;

}

- (NSRect)confinementRectForMenu:(NSMenu *)menu onScreen:(NSScreen *)screen
{
	printf("confinementRectForMenu\n");
	
	/*
	NSRect frame;
	frame.origin.x=164.000000;
	frame.origin.y=217.000000;
	frame.size.width=555.000000;
	frame.size.height=479.000000;
	*/
	return NSZeroRect;
}
- (void)menuWillOpen:(NSMenu *)menu
{
	printf("menuWillOpen\n");
	//[entryViewController.window setStyleMask:NSBorderlessWindowMask];
	//[entryViewController.window setFrame:[entryView frame] display:YES];
	
}
- (void)menu:(NSMenu *)menu willHighlightItem:(NSMenuItem *)item
{
	printf("willHighlightItem:%d\n",[item tag]);
	curMenuTag=[item tag];
	if ([item tag]==11) {

		//NSView *tView = [statusItem view];
		/*
		NSWindow *tWnd = [entryView window];
		[tWnd makeFirstResponder:noteLabel];
		NSRect tFrame = [tWnd frame];
		printf("%f,%f,%f,%f\n",tFrame.origin.x,tFrame.origin.y,tFrame.size.width,tFrame.size.height);
		 */
		
	}
	if ([item tag]==2000) {
		//[[menu Window] makeFirstResponder:hourLabel];
		
	}
}

/*
- (BOOL)menuHasKeyEquivalent:(NSMenu *)menu forEvent:(NSEvent *)event target:(id *)target action:(SEL *)action
{
	printf("menuHasKeyEquivalent\n");
	return NO;
}
*/
int getDayOffYear(NSDate *inDate)
{
	NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	NSUInteger dayOfYear = [gregorian ordinalityOfUnit:NSDayCalendarUnit inUnit:NSYearCalendarUnit forDate:inDate];
	[gregorian release];
	return dayOfYear;
}


- (void)dispActiveDayEntry:(DayEntry *)dayEntry
{
	NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:@" " action:NULL keyEquivalent:@""];
	[newItem setTag:0];
	[statusMenu insertItem:newItem atIndex:0];
	[newItem setView:entryView];
	
	NSMenuItem *spItem = [NSMenuItem separatorItem];
	[spItem setTag:0];
	[statusMenu insertItem:spItem atIndex:1];
	
	int hours = dayEntry->hours;
	int minutes=60.0*(dayEntry->hours-hours*1.0);
	[taskLabel setStringValue:[NSString stringWithUTF8String:dayEntry->task]];
	[projectLabel setStringValue:[NSString stringWithFormat:@"%s (%s)",dayEntry->project,dayEntry->client]];
	[hourLabel setStringValue:[NSString stringWithFormat:@"%02d",hours]];
	[minuLabel setStringValue:[NSString stringWithFormat:@"%02d",minutes]];
	[timeLabel setStringValue:[NSString stringWithUTF8String:dayEntry->timer_started_at]];
	NSString *tmpNoteStr = [NSString stringWithUTF8String:dayEntry->notes];
	if ([tmpNoteStr isEqualToString:@"(null)"]) {
		//[tmpNoteStr release];
		tmpNoteStr=[NSString stringWithString:@"No Notes"];
	}
	[noteLabel setStringValue:tmpNoteStr];
	noteStr = [[NSString stringWithString:tmpNoteStr]retain];
	
	hourValue = hours;
	minuValue = minutes;
	NSString *statusTitle = [NSString stringWithFormat:@"%02d:%02d %s",hourValue,minuValue,dayEntry->task];
	[statusItem setTitle:statusTitle];
	timerFlag=TRUE;
}

- (void)clearMenu
{
	[entryView removeFromSuperview];
	while ([statusMenu numberOfItems]>5) {
		[statusMenu removeItemAtIndex:0];
	}
	harvest_cleandaily(&todayDaily);
	harvest_cleandaily(&yesterdayDaily);
	harvest_cleandaily(&beforeDaily);
	activeDayEntry=NULL;
	harvest_logout(&myConnect);
}

- (int) retrieveData
{
	UserDomain myDomain={0};

	memset(&myConnect,0,sizeof(myConnect));
	memset(&todayDaily,0,sizeof(todayDaily));
	memset(&yesterdayDaily,0,sizeof(yesterdayDaily));
	memset(&beforeDaily,0,sizeof(beforeDaily));
	
	NSArray *accountAry = [[NSUserDefaults standardUserDefaults] objectForKey:@"com.havagent.simon"];
	if ((accountAry==NULL)||([accountAry count]<3)) {
		[window makeKeyAndOrderFront:nil];
		[window center];
		return 0;
	}
	
	hideLoginFlag = [[NSUserDefaults standardUserDefaults] boolForKey:@"com.havagent.simonshow"];
	if (!hideLoginFlag) {
		[self menuPreferences:self];
	}
	NSString *acntStr=[accountAry objectAtIndex:0];
	NSString *mailStr=[accountAry objectAtIndex:1];
	NSString *passStr=[accountAry objectAtIndex:2];
	accountStr=[NSString stringWithString:acntStr];
	
	//myConnect.accountStr=accountStr;
	snprintf(myConnect.accountStr,sizeof(myConnect.accountStr),"%s",[acntStr UTF8String]);
	snprintf(myDomain.userStr,sizeof(myDomain.userStr),"%s",[mailStr UTF8String]);
	snprintf(myDomain.passStr,sizeof(myDomain.passStr),"%s",[passStr UTF8String]);
	UserInfo myUserInfo={0};
	
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
	
	NSMenuItem *acntMenu = [statusMenu itemAtIndex:0];
	NSString *acntMenuTitle = [NSString stringWithFormat:@"%@.harvestapp.com...",accountStr];
	[acntMenu setTitle:acntMenuTitle];
	
	int menuCnt=0;

	//for today
	int todayCnt = todayDaily.dayentryCount;
	if (todayCnt>0) {
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:@"Today" action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = todayDaily.dayentryArray;
		while (curDayEntry) {
			if (curDayEntry->timer_started_at[0]!=0) {
				activeDayEntry=curDayEntry;
				[self dispActiveDayEntry:curDayEntry];
				menuCnt+=2;
			}
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuDayEntry:) keyEquivalent:@""];
			//[newItem setTag:TODAY_MENU_BASE+curCnt];
			[newItem setTag:curDayEntry->idV];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}

		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
	}

	
	//for yesterday
	harvest_getdayofyear(&myConnect,dayYes,yearYes,&yesterdayDaily);

	int yestodayCnt = yesterdayDaily.dayentryCount;
	if (yestodayCnt>0) {
		
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:weekDayYes action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = yesterdayDaily.dayentryArray;
		while (curDayEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuOldDayEntry:) keyEquivalent:@""];
			//[newItem setTag:YESTERDAY_MENU_BASE+curCnt];
			[newItem setTag:curDayEntry->idV];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}

		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
	}
	
	//for the day before yesterday
	harvest_getdayofyear(&myConnect,dayBef,yearBef,&beforeDaily);
	
	int beforeCnt = beforeDaily.dayentryCount;
	if (beforeCnt>0) {
		NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:weekDayBef action:NULL keyEquivalent:@""];
		[newItem setTag:0];
		[statusMenu insertItem:newItem atIndex:menuCnt];
		menuCnt++;
		
		int curCnt=0;
		DayEntry *curDayEntry = beforeDaily.dayentryArray;
		while (curDayEntry) {
			NSString *itemTitle =[NSString stringWithFormat:@"%05.2f %s > %s > %s",curDayEntry->hours, curDayEntry->client,curDayEntry->project,curDayEntry->task];
			NSMenuItem *newItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuOldDayEntry:) keyEquivalent:@""];
			//[newItem setTag:BEFORE_MENU_BASE+curCnt];
			[newItem setTag:curDayEntry->idV];
			[newItem setIndentationLevel:1];
			[statusMenu insertItem:newItem atIndex:menuCnt];
			curDayEntry=curDayEntry->next;
			menuCnt++;
			curCnt++;
		}
		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
	}
	
	//for projects
	int projectCnt = todayDaily.projectCount;
	if (projectCnt>0) {
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
					NSMenuItem *newTaskItem= [[NSMenuItem alloc] initWithTitle:itemTitle action:@selector(menuProjectTask:) keyEquivalent:@""];
					[newTaskItem setTag:curCnt*1000+tkMenuCnt];
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

		NSMenuItem *spItem = [NSMenuItem separatorItem];
		[spItem setTag:0];
		[statusMenu insertItem:spItem atIndex:menuCnt];
		menuCnt++;
		
	}
	
	if (NULL==activeDayEntry) {
		[statusItem setTitle:@"Idle"];
	}
	
	/*
	
	
	harvest_cleandaily(&todayDaily);
	harvest_cleandaily(&yesterdayDaily);
	harvest_cleandaily(&beforeDaily);
	 */
	//harvest_getdaily(&myConnect,&myDaily);
	//harvest_cleandaily(&myDaily);
	//harvest_logout(&myConnect);
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


- (void)addDayEntry:(int)menuIndex
{
	int curProj=menuIndex/1000;
	int curTask=menuIndex-1000*curProj;
	int indProj=0;
	int curProjectId=0;
	int curTaskId=0;
	ProjectEntry *curProjEntry = todayDaily.projectArray;
	while (curProjEntry) {
		if (indProj==curProj) {
			curProjectId=curProjEntry->idV;
			//int taskCnt = curProjEntry->taskCount;
			int indTask=0;
			TaskEntry *curTaskEntry = curProjEntry->taskArray;
			while (curTaskEntry) {
				if (indTask==curTask) {
					curTaskId=curTaskEntry->idV;
					break;
				}
				else {
					curTaskEntry=curTaskEntry->next;
					indTask++;
				}
			}
			break;
		}
		else
		{
			indProj++;
			curProjEntry=curProjEntry->next;
		}
	}
	
	
	EntryRequest reqEntry={0};
	Timer retTimer={0};
	reqEntry.project_id = curProjectId;
	reqEntry.task_id = curTaskId;
	reqEntry.entry_id = 0;
	reqEntry.hours=0;
	snprintf(reqEntry.notes,MAX_FIELD_LEN,"%s","No Notes");
	NSCalendarDate *now = [NSCalendarDate calendarDate];
	snprintf(reqEntry.spent_at,MAX_FIELD_LEN,"%s",[[now descriptionWithCalendarFormat:@"%a, %d %b %Y"] UTF8String]);	
	
	harvest_addentry(&myConnect,&reqEntry,&retTimer);
	[self clearMenu];
	[self retrieveData];
}

- (void)updateDayEntry
{
	EntryRequest reqEntry={0};
	Timer retTimer={0};
	reqEntry.project_id = activeDayEntry->project_id;
	reqEntry.task_id = activeDayEntry->task_id;
	reqEntry.entry_id = activeDayEntry->idV;
	reqEntry.hours=hourValue*1.0+(minuValue*1.0)/60.0;
	snprintf(reqEntry.notes,MAX_FIELD_LEN,"%s",[noteStr UTF8String]);
	NSCalendarDate *now = [NSCalendarDate calendarDate];
	snprintf(reqEntry.spent_at,MAX_FIELD_LEN,"%s",[[now descriptionWithCalendarFormat:@"%a, %d %b %Y"] UTF8String]);	
	
	harvest_updateentry(&myConnect,&reqEntry,&retTimer);
	
}


- (BOOL)control:(NSControl *)control textShouldEndEditing:(NSText *)fieldEditor
{
	printf("control:textShouldEndEditing \n");
	if ((NSTextField *)control==hourLabel) 
	{
		int newHourValue = [[hourLabel stringValue] integerValue];
		if (newHourValue!=hourValue) {
			hourValue=newHourValue;
			[self updateDayEntry];
		}
	}
	else if ((NSTextField *)control==minuLabel) 
	{
		int newMinuValue = [[minuLabel stringValue] integerValue];
		if (newMinuValue!=minuValue) {
			minuValue=newMinuValue;
			[self updateDayEntry];
		}
	}
	else if ((NSTextField *)control==noteLabel)
	{
		NSString *newNoteStr = [noteLabel stringValue];
		if (![newNoteStr isEqualToString:noteStr]) {
			[noteStr release];
			noteStr = [NSString stringWithString:newNoteStr];
			[self updateDayEntry];
		}
	}

	return YES;
}
- (BOOL)textShouldBeginEditing:(NSText *)textObject
{
	printf("textShouldBeginEditing \n");
	return YES;
}
- (void)textDidBeginEditing:(NSNotification *)aNotification
{
	printf("textDidBeginEditing \n");
}
- (BOOL)textShouldEndEditing:(NSText *)textObject
{
	printf("textShouldEndEditing \n");
	return YES;
}
- (void)textDidEndEditing:(NSNotification *)aNotification
{
	printf("textDidEndEditing \n");
}
- (void)textDidChange:(NSNotification *)aNotification
{
	printf("textDidChange \n");
}

- (void)someAction:(id)sender
{
	printf("someAction\n");
	// do something interesting when the user hits <enter> in the text field
}

@end
