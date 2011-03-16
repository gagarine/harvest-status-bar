//
//  NumberTextFieldFormatter.m
//  harvagent
//
//  Created by boyd yang on 2/28/11.
//  Copyright 2011 home. All rights reserved.
//

#import "NumberTextFieldFormatter.h"


@implementation NumberTextFieldFormatter

- (id)init {
	[super init];
	maxLength = 2;
	maxValue = 60;
	return self;
}

- (void)setMaximumLength:(int)len {
	maxLength = len;
}

- (int)maximumLength {
	return maxLength;
}

- (NSString *)stringForObjectValue:(id)object {
	return (NSString *)object;
}

- (BOOL)getObjectValue:(id *)object forString:(NSString *)string errorDescription:(NSString **)error {
	*object = string;
	return YES;
}

- (BOOL)isPartialStringValid:(NSString **)partialStringPtr
	   proposedSelectedRange:(NSRangePointer)proposedSelRangePtr
			  originalString:(NSString *)origString
	   originalSelectedRange:(NSRange)origSelRange
			errorDescription:(NSString **)error {
    if ([*partialStringPtr length] > maxLength) {
        return NO;
    }
	
	NSRange curRange = [*partialStringPtr rangeOfCharacterFromSet:[NSCharacterSet letterCharacterSet]];
	
	if (curRange.location!=NSNotFound) {
		return NO;
	}
    if (![*partialStringPtr isEqual:[*partialStringPtr uppercaseString]]) {
		*partialStringPtr = [*partialStringPtr uppercaseString];
		return NO;
    }
	
	if ([*partialStringPtr intValue]>=maxValue) {
		return NO;
	}
	return YES;
}

- (NSAttributedString *)attributedStringForObjectValue:(id)anObject withDefaultAttributes:(NSDictionary *)attributes {
	return nil;
}

@end
