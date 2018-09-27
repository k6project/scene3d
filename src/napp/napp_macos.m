#include <napp/macros.h>

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

static struct
{
    const char* ViewTitle;
    int ViewWidth, ViewHeight;
    bool IsFullscreen;
} NApp;

NAPP_API void NAppArgv(int argc, char** argv)
{
}

void NAppSetAppName(const char* value)
{
    NApp.ViewTitle = value;
}

const char* NAppGetAppName(void)
{
    return NApp.ViewTitle;
}

void NAppSetFullscreen(bool value)
{
    NApp.IsFullscreen = value;
}

void NAppSetViewSize(int width, int height)
{
    NApp.ViewWidth = width;
    NApp.ViewHeight = height;
}

@interface NAppWindowCocoa : NSWindow
@end

@implementation NAppWindowCocoa
@end

@interface NAppWindowDelegate : NSObject<NSWindowDelegate>
{
}
@end

@implementation NAppWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.f];
}
@end

static void NAppCreateWindowCocoa(void)
{
    NAppWindowCocoa* wnd = nil;
    NSRect sSize = [[NSScreen mainScreen] visibleFrame];
    float width = NApp.ViewWidth, height = NApp.ViewHeight;
    float xOrg = (sSize.size.width - width) * 0.5f;
    float yOrg = (sSize.size.height - height) * 0.5f;
    NSRect wSize = NSMakeRect(xOrg, yOrg, width, height);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    wnd = [[NAppWindowCocoa alloc] initWithContentRect:wSize styleMask:style backing:NSBackingStoreBuffered defer:NO];
    if (NApp.ViewTitle)
    {
        [wnd setTitle:[NSString stringWithUTF8String:NApp.ViewTitle]];
    }
    NAppWindowDelegate* delegate = [[NAppWindowDelegate alloc] init];
    [wnd setDelegate:delegate];
    [wnd makeKeyAndOrderFront:NSApp];
}

bool NAppInitialize(void)
{
    if (NSApp)
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
    return true;
}

@interface NAppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation NAppDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateNow;
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NAppCreateWindowCocoa();
}
@end

NAPP_API void napp_run(void)
{
    NAppDelegate* nappDelegate = [[NAppDelegate alloc] init];
    [NSApp setDelegate:nappDelegate];
#if 0
    [NSApp run];
#else
    [NSApp finishLaunching];
    NSEvent* ev;
    while (true)
    do {
        ev = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                   inMode: NSDefaultRunLoopMode
                                  dequeue: YES];
        if (ev) [NSApp sendEvent: ev];
    } while (ev);
#endif
}
