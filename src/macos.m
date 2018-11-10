#include "global.h"

#include "args.h"
#include "vk_api.h"

#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

static void* gState = NULL;
static const Options* gOpts = NULL;
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
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
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
    appPrintf("====  LOG END  ====\n\n");
    return NSTerminateNow;
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    NSRect wSize, sSize = [[NSScreen mainScreen] visibleFrame];
    if (gOpts->isFullscreen)
        wSize = sSize;
    else
    {
        float width = gOpts->windowWidth, height = gOpts->windowHeight;
        float xOrg = (sSize.size.width - width) * 0.5f;
        float yOrg = (sSize.size.height - height) * 0.5f;
        wSize = NSMakeRect(xOrg, yOrg, width, height);
    }
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    window = [[NSWindow alloc] initWithContentRect:wSize styleMask:style backing:NSBackingStoreBuffered defer:NO];
    windowDelegate = [[AppWindowDelegate alloc] init];
    if (gOpts->isFullscreen)
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
    appPrintf("\n==== LOG BEGIN ====\n");
    INVOKE(gCallbacks->beforeStart);
}
@end

void appGetName(char* buff, size_t max)
{
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    id appName = [info objectForKey:@"CFBundleDisplayName"];
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
        appName = [info objectForKey:@"CFBundleName"];
    if (!appName || ![appName isKindOfClass:[NSString class]] || [appName isEqualToString:@""])
        appName = [info objectForKey:@"CFBundleExecutable"];
    assert(appName && [appName isKindOfClass:[NSString class]] && ![appName isEqualToString:@""]);
    memcpy(buff, [appName UTF8String], max);
}

void appInitialize(HMemAlloc mem, const Options* opts, AppCallbacks* callbacks, void* state)
{
    gOpts = opts;
    gState = state;
    gCallbacks = callbacks;
    NSApplication* nsApp = [NSApplication sharedApplication];
    NSMenu* menuBar = [[NSMenu alloc] init];
    [nsApp setMainMenu:menuBar];
    NSString* appName = [NSString stringWithUTF8String:gOpts->appName];
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
    NSEvent* ev = nil;
    do {
        ev = [NSApp nextEventMatchingMask: NSEventMaskAny
                                untilDate: nil
                                   inMode: NSDefaultRunLoopMode
                                  dequeue: YES];
        if (ev)
            [NSApp sendEvent: ev];
    } while (ev);
    return true;
}

void appPollEvents(void)
{
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

void appPrintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

bool appCreateVkSurface(VkInstance inst, const VkAllocationCallbacks* alloc, VkSurfaceKHR* surface)
{
    VkMacOSSurfaceCreateInfoMVK createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK, .pNext = NULL,
        .flags = 0, .pView = (__bridge void*)[[NSApp keyWindow] contentView]
    };
    if (vkCreateMacOSSurfaceMVK(inst, &createInfo, alloc, surface) != VK_SUCCESS)
    {
        appPrintf("ERROR: Failed to create surface");
        return false;
    }
    appPrintf("Created MacOS view-based surface\n");
    return true;
}

extern int appMain(int argc, const char** argv);

int main(int argc, const char** argv)
{
    return appMain(argc, argv);
}
