#include "napp.h"

#include <wrl.h>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

static void OnGlobalVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
}

ref class NAppView sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	NAppView() {}
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
	{
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &NAppView::OnActivated);
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &NAppView::OnSuspending);
		CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &NAppView::OnResuming);
	}
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window)
	{
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &NAppView::OnWindowSizeChanged);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &NAppView::OnVisibilityChanged);
		//window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(&OnGlobalVisibilityChanged);
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &NAppView::OnWindowClosed);
		DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
		displayInfo->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnDpiChanged);
		displayInfo->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnOrientationChanged);
		DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnDisplayContentsInvalidated);
	}
	virtual void Load(Platform::String^ entryPoint) {}
	virtual void Run()
	{
		while (!m_windowClosed)
		{
			if (m_windowVisible)
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			}
			else
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}
	virtual void Uninitialize() {}
	virtual ~NAppView() {}
protected:
	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
	void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
		deferral->Complete();
	}
	void OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
	}
	void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
	{
	}
	void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
	{
		m_windowVisible = args->Visible;
	}
	void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
	{
		m_windowClosed = true;
	}
	void OnDpiChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
	}
	void OnOrientationChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
	}
	void OnDisplayContentsInvalidated(DisplayInformation^ sender, Platform::Object^ args)
	{
	}
private:
	bool m_windowClosed = false;
	bool m_windowVisible = true;
};

ref class NAppViewSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
	{
		return ref new NAppView();
	}
};

void NAppArgv(Platform::Array<Platform::String^>^)
{
}

bool NAppInitialize()
{
	return true;
}

void NAppRun()
{
	auto viewSource = ref new NAppViewSource();
	Windows::ApplicationModel::Core::CoreApplication::Run(viewSource);
}
