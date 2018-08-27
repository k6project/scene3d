#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#define NAPP_PRIVATE
#include "napp_private.h"

@interface NAppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation NAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    //Request window to be closed, cancel for now
    return NSTerminateCancel;
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [NSApp stop:nil];
    //Notfy app state
}
@end

bool NAppInitializeImpl()
{
    if (!NSApp)
    {
        return true;
    }
    
    [NSApplication sharedApplication];
    
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    id appName = [info objectForKey:@"CFBundleDisplayName"];
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
    {
        appName = [info objectForKey:@"CFBundleName"];
    }
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
    {
        appName = [info objectForKey:@"CFBundleExecutable"];
    }
    assert(appName && [appName isKindOfClass:[NSString class]] && ![appName isEqualToString:@""]);

    NSMenu* menuBar = [[NSMenu alloc] init];
    [NSApp setMainMenu:menuBar];
    NSMenuItem* appItem = [menuBar addItemWithTitle:appName
                                             action:NULL
                                      keyEquivalent:@""];
    NSMenu* appSubMenu = [[NSMenu alloc] init];
    [appSubMenu addItemWithTitle:@"Quit"
                          action:@selector(terminate:)
                   keyEquivalent:@"q"];
    [appItem setSubmenu:appSubMenu];
    
    //create and set delegate
    [NSApp run];
    
    return true;
}
