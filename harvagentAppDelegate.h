//
//  harvagentAppDelegate.h
//  harvagent
//
//  Created by boyd yang on 2/23/11.
//  Copyright 2011 home. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "harvest.h"

#define TODAY_MENU_BASE 1000
#define YESTERDAY_MENU_BASE 2000
#define BEFORE_MENU_BASE 3000
#define PROJECT_MENU_BASE 4000

@interface harvagentAppDelegate : NSObject <NSApplicationDelegate,NSMenuDelegate> {
    NSWindow *window;
	IBOutlet NSMenu *statusMenu;
    NSStatusItem * statusItem;
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)menuWindow:(id)sender;
- (IBAction)menuPreferences:(id)sender;
- (IBAction)menuHelp:(id)sender;
- (IBAction)menuQuit:(id)sender;

- (IBAction)menuDayEntry:(id)sender;

- (IBAction)btnDone:(NSButton *)sender;
- (IBAction)btnCancel:(NSButton *)sender;
@end
