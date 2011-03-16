//
//  CustomBGView.m
//  harvagent
//
//  Created by boyd yang on 2/25/11.
//  Copyright 2011 home. All rights reserved.
//

#import "CustomBGView.h"


@implementation CustomBGView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

/*
#define menuItem ([self enclosingMenuItem])

- (void) drawRect: (NSRect) rect {
    BOOL isHighlighted = [menuItem isHighlighted];
    if (isHighlighted) {
        [[NSColor selectedMenuItemColor] set];
        [NSBezierPath fillRect:rect];
    } else {
        [super drawRect: rect];
    }
}
*/
- (void)drawRect:(NSRect)rect {
 [[NSColor selectedMenuItemColor] set];
 [NSBezierPath fillRect:rect];
}

/*
- (void)mouseMoved:(NSEvent *)theEvent
{
	printf("mouseMoved in CustomBGView\n");
}
*/
/*
- (BOOL)acceptsFirstResponder
{
    return YES;
}
*/
- (void)viewDidMoveToWindow
{
	//[[self window] makeFirstResponder:noteLabel];
}

@end
