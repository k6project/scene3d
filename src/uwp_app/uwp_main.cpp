#include <napp/napp_main.h>

/*#include "uwp_appMain.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

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
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &NAppView::OnWindowClosed);
		DisplayInformation^ displayInfo = DisplayInformation::GetForCurrentView();
		displayInfo->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnDpiChanged);
		displayInfo->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnOrientationChanged);
		DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &NAppView::OnDisplayContentsInvalidated);
	}
	virtual void Load(Platform::String^ entryPoint)
	{
		if (m_main == nullptr)
		{
			m_main = new uwp_app::uwp_appMain();
			GetDeviceResources(); // Otherwise, no resources created
		}
	}
	virtual void Run()
	{
		while (!m_windowClosed)
		{
			if (m_windowVisible)
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				PIXBeginEvent(0, L"Update");
				{
					m_main->Update();
				}
				PIXEndEvent();
				PIXBeginEvent(0, L"Render");
				{
					if (m_main->Render())
					{
						GetDeviceResources()->Present();
					}
				}
				PIXEndEvent();
			}
			else
			{
				CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
			}
		}
	}
	virtual void Uninitialize() {}
	virtual ~NAppView()
	{
		if (m_deviceResources != nullptr)
		{
			delete m_deviceResources;
		}
		if (m_main != nullptr)
		{
			delete m_main;
		}
	}
protected:
	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
	void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
		deferral->Complete();
		//Save state in other thread
		//create_task([this, deferral]()
		//{
		//	m_main->OnSuspending();
		//	deferral->Complete();
		//});
	}
	void OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		m_main->OnResuming();
	}
	void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
	{
		GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
		m_main->OnWindowSizeChanged();
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
		GetDeviceResources()->SetDpi(sender->LogicalDpi);
		m_main->OnWindowSizeChanged();
	}
	void OnOrientationChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
		GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
		m_main->OnWindowSizeChanged();
	}
	void OnDisplayContentsInvalidated(DisplayInformation^ sender, Platform::Object^ args)
	{
		GetDeviceResources()->ValidateDevice();
	}
private:
	DX::DeviceResources* GetDeviceResources()
	{
		if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
		{
			delete m_deviceResources;
			m_deviceResources = nullptr;
			m_main->OnDeviceRemoved();
			DX::ReportLiveObjects();
		}
		if (m_deviceResources == nullptr)
		{
			m_deviceResources = new DX::DeviceResources();
			m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
			m_main->CreateRenderers(m_deviceResources);
		}
		return m_deviceResources;
	}

	DX::DeviceResources* m_deviceResources = nullptr;
	uwp_app::uwp_appMain* m_main = nullptr;
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
};*/

static void napp_main()
{
	if (napp_initialize())
	{
		//NAppSetFullscreen(false);
		//NAppSetViewSize(1280, 800);
		napp_run();
	}
}
