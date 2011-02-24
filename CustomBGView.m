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

- (void)drawRect:(NSRect)dirtyRect {
    // Drawing code here.
	// set any NSColor for filling, say white:
    [[NSColor blueColor] setFill];
    NSRectFill(dirtyRect);
}

@end
