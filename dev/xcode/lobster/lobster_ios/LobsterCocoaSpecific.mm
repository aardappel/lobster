//
//  LobsterCocoaSpecific.m
//  lobster
//
//  Created by Wouter van Oortmerssen on 6/6/13.
//
//

#import "LobsterCocoaSpecific.h"

@implementation LobsterCocoaSpecific

@end

void IOSLog(const char *msg)
{
    NSLog(@"Lobster: %s", msg);
}
