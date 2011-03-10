//
//  CustomView.h
//  
//


#import <Cocoa/Cocoa.h>
#define MAX_TEXT_NUM 13

@class harvagentAppDelegate;
@interface CustomView : NSView {
    __weak harvagentAppDelegate *controller;
    BOOL highFlag;
	NSString *indText;
}

@property (assign) NSString *indText;
- (id)initWithFrame:(NSRect)frame controller:(harvagentAppDelegate *)ctrlr;
- (void)updateIndText:(NSString *)inText;
- (void)setHighFlag:(BOOL)inFlag;
@end
