//
//  CustomView.m
//  NSStatusItemTest
//
//  Created by Matt Gemmell on 04/03/2008.
//  Copyright 2008 Magic Aubergine. All rights reserved.
//

#import "CustomView.h"
#import "harvagentAppDelegate.h"


@implementation CustomView

@synthesize indText;

- (id)initWithFrame:(NSRect)frame controller:(harvagentAppDelegate *)ctrlr
{
    if (self = [super initWithFrame:frame]) {
        controller = ctrlr; // deliberately weak reference.
    }
    
	indText=[NSString stringWithString:@"Connecting..."];
	
	highFlag=FALSE;
	
    return self;
}


- (void)dealloc
{
    controller = nil;
    [super dealloc];
}


- (void)drawRect:(NSRect)rect {
    // Draw background if appropriate.
    if (highFlag) {
        [[NSColor selectedMenuItemColor] set];
        NSRectFill(rect);
    }
    else {
		NSImage *hImage = [NSImage imageNamed:@"hidle.png"];
		[hImage drawInRect: [self frame] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
		[hImage release];
	}

	
    // Draw some text, just to show how it's done.
    //indText = @"Connecting..."; // whatever you want
    //printf("indText:%s\n",[indText UTF8String]);
    NSColor *textColor = [NSColor controlTextColor];
    if (highFlag) {
        textColor = [NSColor selectedMenuItemTextColor];
    }
    
    NSFont *msgFont = [NSFont menuBarFontOfSize:15.0];
    NSMutableParagraphStyle *paraStyle = [[NSMutableParagraphStyle alloc] init];
    [paraStyle setParagraphStyle:[NSParagraphStyle defaultParagraphStyle]];
    [paraStyle setAlignment:NSCenterTextAlignment];
    [paraStyle setLineBreakMode:NSLineBreakByTruncatingTail];
    NSMutableDictionary *msgAttrs = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                     msgFont, NSFontAttributeName,
                                     textColor, NSForegroundColorAttributeName,
                                     paraStyle, NSParagraphStyleAttributeName,
                                     nil];
    [paraStyle release];
    
    NSSize msgSize = [indText sizeWithAttributes:msgAttrs];
    NSRect msgRect = NSMakeRect(0, 0, msgSize.width, msgSize.height);
    msgRect.origin.x = ([self frame].size.width - msgSize.width) / 2.0;
    msgRect.origin.y = ([self frame].size.height - msgSize.height) / 2.0;
    
    [indText drawInRect:msgRect withAttributes:msgAttrs];
}

- (void)updateIndText:(NSString *)inText
{
	NSString *tmpStr=inText;
	
	if ([inText length]>MAX_TEXT_NUM) {
		tmpStr=[inText substringToIndex:MAX_TEXT_NUM];
		tmpStr=[tmpStr stringByAppendingString:@"..."];
	}
	
	[indText release];
	indText = [[NSString stringWithString:tmpStr]retain];
	
	//[self drawRect:[self frame]];

	[self setNeedsDisplay:YES];

}

- (void)setHighFlag:(BOOL)inFlag
{
	highFlag=inFlag;
	[self setNeedsDisplay:YES];
}

- (void)mouseDown:(NSEvent *)event
{
    //NSRect frame = [[self window] frame];
    //NSPoint pt = NSMakePoint(NSMidX(frame), NSMinY(frame));
	printf("mouseDown:%d\n",highFlag?1:0);
	[controller showhideMenu];
//    [self setNeedsDisplay:YES];
}


- (void)mouseUp:(NSEvent *)event
{
    //NSRect frame = [[self window] frame];
    //NSPoint pt = NSMakePoint(NSMidX(frame), NSMinY(frame));
    //highFlag = !highFlag;
	printf("mouseUp:%d\n",highFlag?1:0);
    //[self setNeedsDisplay:YES];
}

- (BOOL)acceptsFirstResponder
{
    return TRUE;
}

@end
