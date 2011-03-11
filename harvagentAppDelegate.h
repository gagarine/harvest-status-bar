//
//  harvagentAppDelegate.h
//  harvagent
//
//  Created by boyd yang on 2/23/11.
//  Copyright 2011 home. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "harvest.h"
#import "CustomBGView.h"
#import "CustomView.h"

#define TODAY_MENU_BASE 1000
#define YESTERDAY_MENU_BASE 2000
#define BEFORE_MENU_BASE 3000
#define PROJECT_MENU_BASE 4000

@interface harvagentAppDelegate : NSObject <NSApplicationDelegate,NSMenuDelegate,NSTextFieldDelegate> {
	IBOutlet NSApplication *app;
    NSWindow *window;
	IBOutlet NSMenu *statusMenu;
    NSStatusItem * statusItem;
	DayEntry *activeDayEntry;
	
	IBOutlet CustomBGView *entryView;
	
	IBOutlet NSTextField *taskLabel;
	IBOutlet NSTextField *projectLabel;
	IBOutlet NSTextField *hourLabel;
	IBOutlet NSTextField *minuLabel;
	IBOutlet NSTextField *timeLabel;
	IBOutlet NSTextField *noteLabel;
	
	IBOutlet NSButton *timerBtn;
	
	int hourValue;
	int minuValue;
	NSString *noteStr;
	
	HarvCon myConnect;
	Daily todayDaily;
	Daily yesterdayDaily;
	Daily beforeDaily;
	
	int curMenuTag;
	BOOL timerFlag;
	int stoppedDayEntry;
	
	IBOutlet NSTextField *accountText;
	IBOutlet NSTextField *mailText;
	IBOutlet NSTextField *passText;
	IBOutlet NSProgressIndicator *progressInd;
	NSString *accountStr;
	
	BOOL hideLoginFlag;
	IBOutlet NSButton *showLoginBtn;

	CustomView *stateView;
	
	NSDate *startDate;
	NSTimer *standTimer;
	
	int yearYes;
	int dayYes;
	int yearBef;
	int dayBef;
	
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) NSString *noteStr;
@property (assign) NSDate *startDate;

DayEntry *activeDayEntry;
HarvCon myConnect;
Daily todayDaily;
Daily yesterdayDaily;
Daily beforeDaily;

- (IBAction)menuProfile:(id)sender;
- (IBAction)menuPreferences:(id)sender;
- (IBAction)menuHelp:(id)sender;
- (IBAction)menuQuit:(id)sender;

- (IBAction)menuDayEntry:(id)sender;
- (IBAction)menuProjectTask:(id)sender;

- (IBAction)btnConnect:(NSButton *)sender;
- (IBAction)btnStartStopTimer:(NSButton *)sender;
- (IBAction)btnShowLogin:(NSButton *)sender;

- (IBAction)onEnter:(id)sender;

- (void)someAction:(id)sender;
- (int) retrieveData;

- (void)addDayEntry;
- (void)clearMenu;

- (void)showhideMenu;
- (void)restartTimer;

@end
