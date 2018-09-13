#if 0
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#define NAPP_PRIVATE
#include "napp_private.h"

@interface NAppWindowCocoa : NSWindow
{
@public NAppWindow* Window;
}
@end

@implementation NAppWindowCocoa
@end

@interface NAppWindowDelegate : NSObject<NSWindowDelegate>
{
@public NAppWindow* Window;
}
@end

@implementation NAppWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
    if (Window)
    {
        NAppWindow* window = Window;
        Window = NULL;
        NAppWindowClosed(window);
    }
}
@end

bool NAppCreateWindowImpl(NAppWindow* window)
{
    NAppWindowCocoa* wnd = nil;
    const NAppWindowInfo* info = &window->Info;
    NSRect sSize = [[NSScreen mainScreen] visibleFrame];
    NSRect wSize = NSMakeRect(info->XOrg, sSize.size.height - info->Height, info->Width, info->Height);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    wnd = [[NAppWindowCocoa alloc] initWithContentRect:wSize styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [wnd setTitle:[NSString stringWithUTF8String:info->Title]];
    NAppWindowDelegate* delegate = [[NAppWindowDelegate alloc] init];
    delegate->Window = window;
    wnd->Window = window;
    //wSize.size.width *= wnd.screen.backingScaleFactor;
    //wSize.size.height *= wnd.screen.backingScaleFactor;
    //MetalView = [[NSMetalView alloc]initWithFrame:wSize];
    //[Window setContentView:MetalView];
    [wnd setDelegate:delegate];
    [wnd makeKeyAndOrderFront:NSApp];
    window->WindowImpl = (__bridge void*)wnd;
    window->IsVisible = true;
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
    GState.IsRunning = true;
    //Pre-init graphics API:
    // Windows: D3D12 or Vulkan
    // MacOS, Linux, Android: Vulkan
    // PS4: GNM(X)
    // XBO: D3D12
    // Switch: NVN
    for (NAppWindow* window = GState.FirstWindow; window; window = window->Next)
    {
        NAppCreateWindowImpl(window);
        //create rendering environment in every window
    }
}
@end

int NAppRun(void)
{
    NAppDelegate* nappDelegate = [[NAppDelegate alloc] init];
    [NSApp setDelegate:nappDelegate];
    [NSApp run];
    return 0;
}

void NAppShutdownImpl(void)
{
    [NSApp terminate: [NSApp delegate]];
}
#endif
