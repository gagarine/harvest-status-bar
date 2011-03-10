//
//  NumberTextFieldFormatter.h
//  harvagent
//
//  Created by boyd yang on 2/28/11.
//  Copyright 2011 home. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface NumberTextFieldFormatter : NSFormatter {
	int maxLength;
	int maxValue;
}
- (void)setMaximumLength:(int)len;
- (int)maximumLength;

@end
