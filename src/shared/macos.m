#import <Cocoa/Cocoa.h>

#include <dlfcn.h>

#include "main.h"
#include "args.inl"

static void* gState = NULL;
static AppCallbacks* gCallbacks = NULL;
#define INVOKE(c) do if((c)) c (gState); while(0)

@interface AppWindowDelegate : NSObject<NSWindowDelegate>
@end
@implementation AppWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.f];
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation AppDelegate
{
    NSWindow* window;
    AppWindowDelegate* windowDelegate;
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateNow;
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSRect sSize = [[NSScreen mainScreen] visibleFrame];
    float width = gOptions->windowWidth, height = gOptions->windowHeight;
    float xOrg = (sSize.size.width - width) * 0.5f;
    float yOrg = (sSize.size.height - height) * 0.5f;
    NSRect wSize = NSMakeRect(xOrg, yOrg, width, height);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    window = [[NSWindow alloc] initWithContentRect:wSize styleMask:style backing:NSBackingStoreBuffered defer:NO];
    windowDelegate = [[AppWindowDelegate alloc] init];
    [window setDelegate:windowDelegate];
    [window makeKeyAndOrderFront:NSApp];
    INVOKE(gCallbacks->beforeStart);
}
@end

void appInitialize(AppCallbacks* callbacks, void* state)
{
    gState = state;
    gCallbacks = callbacks;
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
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    [NSApp finishLaunching];
}

bool appShouldKeepRunning(void)
{
    return true;
}

void appPollEvents(void)
{
    NSEvent* ev = nil;
    do {
        ev = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                   inMode: NSDefaultRunLoopMode
                                  dequeue: YES];
        if (ev)
            [NSApp sendEvent: ev];
    } while (ev);
}

//void* vkDll = dlopen("@rpath/vulkan.framework/vulkan", RTLD_LOCAL | RTLD_NOW);
//dlclose(vkDll);
