#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <dlfcn.h>

#include "main.h"
#include "args.inl"

static void* gState = NULL;
static AppCallbacks* gCallbacks = NULL;
#define INVOKE(c) do if((c)) c (gState); while(0)

@interface NSMetalView : NSView
@end
@implementation NSMetalView
-(CALayer *)makeBackingLayer
{
    NSRect bounds = self.bounds;
    CAMetalLayer* layer = [[CAMetalLayer alloc] init];
    layer.device = MTLCreateSystemDefaultDevice();
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.drawableSize = bounds.size;
    layer.framebufferOnly = YES;
    layer.frame = bounds;
    return layer;
}
-(BOOL)acceptsFirstResponder
{
    return YES;
}
- (instancetype)initWithFrame:(NSRect)frameRect
{
    if ((self = [super initWithFrame:frameRect]))
    {
        self.wantsLayer = YES;
    }
    return self;
}
@end

@interface AppWindowDelegate : NSObject<NSWindowDelegate>
@end
@implementation AppWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
    INVOKE(gCallbacks->beforeStop);
    [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.f];
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate>
@end
@implementation AppDelegate
{
    NSWindow* window;
    NSMetalView* view;
    AppWindowDelegate* windowDelegate;
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateNow;
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSRect wSize, sSize = [[NSScreen mainScreen] visibleFrame];
    if (gOptions->isFullscreen)
        wSize = sSize;
    else
    {
        float width = gOptions->windowWidth, height = gOptions->windowHeight;
        float xOrg = (sSize.size.width - width) * 0.5f;
        float yOrg = (sSize.size.height - height) * 0.5f;
        wSize = NSMakeRect(xOrg, yOrg, width, height);
    }
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    window = [[NSWindow alloc] initWithContentRect:wSize styleMask:style backing:NSBackingStoreBuffered defer:NO];
    windowDelegate = [[AppWindowDelegate alloc] init];
    if (gOptions->isFullscreen)
    {
        [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [window toggleFullScreen:self];
    }
    wSize.size.width *= window.screen.backingScaleFactor;
    wSize.size.height *= window.screen.backingScaleFactor;
    view = [[NSMetalView alloc]initWithFrame:wSize];
    [window setContentView:view];
    [window setDelegate:windowDelegate];
    [window makeKeyAndOrderFront:NSApp];
    INVOKE(gCallbacks->beforeStart);
}
@end

void appInitialize(AppCallbacks* callbacks, void* state)
{
    gState = state;
    gCallbacks = callbacks;
    NSApplication* nsApp = [NSApplication sharedApplication];
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    id appName = [info objectForKey:@"CFBundleDisplayName"];
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
        appName = [info objectForKey:@"CFBundleName"];
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
        appName = [info objectForKey:@"CFBundleExecutable"];
    assert(appName && [appName isKindOfClass:[NSString class]] && ![appName isEqualToString:@""]);
    NSMenu* menuBar = [[NSMenu alloc] init];
    [nsApp setMainMenu:menuBar];
    NSMenuItem* appItem = [menuBar addItemWithTitle:appName
                                             action:NULL
                                      keyEquivalent:@""];
    NSMenu* appSubMenu = [[NSMenu alloc] init];
    [appSubMenu addItemWithTitle:@"Quit"
                          action:@selector(terminate:)
                   keyEquivalent:@"q"];
    [appItem setSubmenu:appSubMenu];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    [nsApp setDelegate:appDelegate];
    [nsApp finishLaunching];
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

bool appLoadLibrary(const char* name, void** handle)
{
    void* ptr = dlopen(name, RTLD_LOCAL | RTLD_NOW);
    *handle = (ptr) ? ptr : NULL;
    return (ptr) ? true : false;
}

void* appGetLibraryProc(void* handle, const char* name)
{
    if (handle)
        return dlsym(handle, name);
    return NULL;
}

void appUnloadLibrary(void* handle)
{
    if (handle)
        dlclose(handle);
}
