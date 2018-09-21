// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "pch.h"
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Media::Playback;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Core;
using namespace Windows::UI::Text;
using namespace Windows::UI::Input::Inking;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Media::Core;

#pragma region - Defined identifiers
// Detail pane region identifier
#define DPR_LEFT 0
#define DPR_RIGHT 1
#define DPR_TOP 2
#define DPR_BOTTOM 3
#pragma endregion

#pragma region - Structs
struct SCENARIO
{
	Grid xamlUI = NULL;
	std::vector<HWND> winControlsLeft{};  //0
	std::vector<HWND> winControlsRight{}; //1
	std::vector<HWND> winControlsTop{};   //2
	std::vector<HWND> winControlsBottom{};//3
} Scenario1, Scenario2;
#pragma endregion

#pragma region - Global variables
// Global variables
DesktopWindowXamlSource detailWindowXamlSource, selectionWindowXamlSource;
HWND hwndParent, hwndDetail, hwndSelection, hwndLeft, hwndRight, hwndTop, hwndBottom;
HWND hwndDetailXamlIsland = nullptr;
HWND hwndSelectionXamlIsland = nullptr;
HWND hwndComboBox = nullptr;
HWND hwndButton = nullptr;
Grid SelectionMenu = nullptr;
int selectedScenario = 0;

// Define the heights and widths of the detail window's children
float topHeightPercent = 0.2f;
float bottomHeightPercent = 0.1f;
float rightHeightPercent = 1 - topHeightPercent - bottomHeightPercent;
float leftHeightPercent = 1 - topHeightPercent - bottomHeightPercent;
float rightWidthPercent = 0.25f;
float leftWidthPercent = 0.25f;

int selectionWidth;
int selectionHeight;
int detailWidth;
int detailHeight;
int topWndWidth;
int topWndHeight;
int bottomWndWidth;
int bottomWndHeight;
int leftWndWidth;
int leftWndHeight;
int rightWndWidth;
int rightWndHeight;

#pragma endregion

#pragma region - Function declarations
// Forward declarations of functions

// Declaration of WNDPROC functions
LRESULT CALLBACK DetailWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DetailTopWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DetailBottomWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DetailRightWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DetailLeftWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ParentWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SelectionWndProc(HWND, UINT, WPARAM, LPARAM);
// Declaration of XAML related functions
Grid CreateInkCanvas();
Grid CreateMediaPlayer();
void CleanupMediaPlayer();
Grid CreateSelectionMenu();
void LoadModule(int);
// Declaration of window initialization function (class declartion and window creation)
int WindowInit(HINSTANCE, int);
#pragma endregion

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	MSG msg;

	winrt::init_apartment(apartment_type::single_threaded);
	WindowInit(hInstance, nCmdShow);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

#pragma region - WndProc callbacks
void ParentWnd_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	//On parent resize, align nav HWND and underlying xaml island
	MoveWindow(hwndSelection, 0, 0, cx, cy, TRUE);
	MoveWindow(hwndSelectionXamlIsland, 0, 0, cx, cy, TRUE);
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	return TRUE;
}

void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	//PaintContent(hwnd, &ps);
	EndPaint(hwnd, &ps);
}

void TopDetail_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	HFONT hFont;

	hdc = BeginPaint(hwnd, &ps);
	GetClientRect(hwnd, &rect);
	hFont = CreateFont(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
	SelectObject(hdc, hFont);

	if (selectedScenario == 0) {
		DrawText(hdc,
			TEXT("Ink Canvas: The UWP XAML hosting API provides the low-level infrastructure for hosting UWP controls in desktop applications. In this scenario the InkCanvas control and tool bar are hosted allowing you to perform ink operations using mouse, touch, and pen within the win32 app."),
			-1, &rect, DT_TOP | DT_WORDBREAK | DT_LEFT);
	}
	else if (selectedScenario == 1) {
		DrawText(hdc,
			TEXT("Media Player Element: The UWP XAML hosting API provides the low-level infrastructure for hosting UWP controls in desktop applications. In this scenario the Media Player Element control is hosted within the Win 32 app. Select a podcast from the Win32 combobox and play the clip."),
			-1, &rect, DT_TOP | DT_WORDBREAK | DT_LEFT);
	}
	else {
		DrawText(hdc, TEXT("Please select a scenario."), -1, &rect, DT_TOP | DT_WORDBREAK | DT_LEFT);
	}
	EndPaint(hwnd, &ps);
}
# pragma endregion 

#pragma region - Window initialization

// Defines and registers WNDCLASS structs for windows, creates the windows including the window for hosting the XAML Navigation View control,
// and displays the navigation UI. 
int WindowInit(HINSTANCE hInstance, int nCmdShow) {
	static TCHAR szParentWndClass[] = TEXT("ParentWndClass");
	static TCHAR szDetailWndClass[] = TEXT("DetailWndClass");
	static TCHAR szDetailTopWndClass[] = TEXT("DetailTopWndClass");
	static TCHAR szDetailBottomWndClass[] = TEXT("DetailBottomWndClass");
	static TCHAR szDetailLeftWndClass[] = TEXT("DetailLeftWndClass");
	static TCHAR szDetailRightWndClass[] = TEXT("DetailRightWndClass");
	static TCHAR szSelectionWndClass[] = TEXT("SelectionWndClass");

	WNDCLASS wndParentClass, wndDetailClass, wndDetailLeftClass, wndDetailRightClass,
		wndDetailTopClass, wndDetailBottomClass, wndSelectionClass;

	//Define WNDCLASS struct for parent window
	wndParentClass.style = CS_HREDRAW | CS_VREDRAW;
	wndParentClass.lpfnWndProc = ParentWndProc;
	wndParentClass.cbClsExtra = 0;
	wndParentClass.cbWndExtra = sizeof(long);
	wndParentClass.hInstance = hInstance;
	wndParentClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndParentClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndParentClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndParentClass.lpszMenuName = NULL;
	wndParentClass.lpszClassName = szParentWndClass;

	//Define WNDCLASS struct for the child window that will be used for holding a Xaml island child window
	wndDetailClass.style = CS_HREDRAW | CS_VREDRAW;
	wndDetailClass.lpfnWndProc = DetailWndProc;
	wndDetailClass.cbClsExtra = 0;
	wndDetailClass.cbWndExtra = sizeof(long);
	wndDetailClass.hInstance = hInstance;
	wndDetailClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndDetailClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndDetailClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDetailClass.lpszMenuName = NULL;
	wndDetailClass.lpszClassName = szDetailWndClass;

	wndDetailLeftClass.style = CS_HREDRAW | CS_VREDRAW;
	wndDetailLeftClass.lpfnWndProc = DetailLeftWndProc;
	wndDetailLeftClass.cbClsExtra = 0;
	wndDetailLeftClass.cbWndExtra = sizeof(long);
	wndDetailLeftClass.hInstance = hInstance;
	wndDetailLeftClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndDetailLeftClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndDetailLeftClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDetailLeftClass.lpszMenuName = NULL;
	wndDetailLeftClass.lpszClassName = szDetailLeftWndClass;

	wndDetailRightClass.style = CS_HREDRAW | CS_VREDRAW;
	wndDetailRightClass.lpfnWndProc = DetailRightWndProc;
	wndDetailRightClass.cbClsExtra = 0;
	wndDetailRightClass.cbWndExtra = sizeof(long);
	wndDetailRightClass.hInstance = hInstance;
	wndDetailRightClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndDetailRightClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndDetailRightClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDetailRightClass.lpszMenuName = NULL;
	wndDetailRightClass.lpszClassName = szDetailRightWndClass;

	wndDetailTopClass.style = CS_HREDRAW | CS_VREDRAW;
	wndDetailTopClass.lpfnWndProc = DetailTopWndProc;
	wndDetailTopClass.cbClsExtra = 0;
	wndDetailTopClass.cbWndExtra = sizeof(long);
	wndDetailTopClass.hInstance = hInstance;
	wndDetailTopClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndDetailTopClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndDetailTopClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDetailTopClass.lpszMenuName = NULL;
	wndDetailTopClass.lpszClassName = szDetailTopWndClass;

	wndDetailBottomClass.style = CS_HREDRAW | CS_VREDRAW;
	wndDetailBottomClass.lpfnWndProc = DetailBottomWndProc;
	wndDetailBottomClass.cbClsExtra = 0;
	wndDetailBottomClass.cbWndExtra = sizeof(long);
	wndDetailBottomClass.hInstance = hInstance;
	wndDetailBottomClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndDetailBottomClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndDetailBottomClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndDetailBottomClass.lpszMenuName = NULL;
	wndDetailBottomClass.lpszClassName = szDetailBottomWndClass;

	//Define WNDCLASS struct for the child window that will be used for selection UI
	wndSelectionClass.style = CS_HREDRAW | CS_VREDRAW;
	wndSelectionClass.lpfnWndProc = SelectionWndProc;
	wndSelectionClass.cbClsExtra = 0;
	wndSelectionClass.cbWndExtra = sizeof(long);
	wndSelectionClass.hInstance = hInstance;
	wndSelectionClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndSelectionClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndSelectionClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndSelectionClass.lpszMenuName = NULL;
	wndSelectionClass.lpszClassName = szSelectionWndClass;


	//Register the WNDCLASS structures
	if (!RegisterClass(&wndParentClass))
	{
		MessageBox(NULL, TEXT("Error registering window class."), szParentWndClass, MB_ICONERROR);
		return 0;
	}

	if (!RegisterClass(&wndDetailClass))
	{
		MessageBox(NULL, TEXT("Error registering window class."), szDetailWndClass, MB_ICONERROR);
		return 0;
	}
	if (!RegisterClass(&wndDetailLeftClass))
	{
		MessageBox(NULL, TEXT("Error registering left detail window class."), szDetailLeftWndClass, MB_ICONERROR);
		return 0;
	}
	if (!RegisterClass(&wndDetailRightClass))
	{
		MessageBox(NULL, TEXT("Error registering right detail window class."), szDetailRightWndClass, MB_ICONERROR);
		return 0;
	}
	if (!RegisterClass(&wndDetailTopClass))
	{
		MessageBox(NULL, TEXT("Error registering top detail window class."), szDetailTopWndClass, MB_ICONERROR);
		return 0;
	}
	if (!RegisterClass(&wndDetailBottomClass))
	{
		MessageBox(NULL, TEXT("Error registering bottom detail window class."), szDetailBottomWndClass, MB_ICONERROR);
		return 0;
	}
	if (!RegisterClass(&wndSelectionClass))
	{
		MessageBox(NULL, TEXT("Error registering window class."), szSelectionWndClass, MB_ICONERROR);
		return 0;
	}

	//Create windows
	hwndParent = CreateWindow(szParentWndClass,
		TEXT("Xaml Hosting Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		wndParentClass.hInstance,
		NULL);

	RECT appRect;
	GetClientRect(hwndParent, &appRect);

	hwndSelection = CreateWindow(szSelectionWndClass,
		TEXT("Win32 App child selection window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, appRect.right, appRect.bottom,
		hwndParent,
		NULL,
		wndSelectionClass.hInstance,
		NULL);

	hwndDetail = CreateWindow(szDetailWndClass,
		TEXT("Win32 App child window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		hwndParent,
		NULL,
		wndDetailClass.hInstance,
		NULL);

	hwndLeft = CreateWindow(szDetailLeftWndClass,
		TEXT("Win32 detail left window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		hwndDetail,
		NULL,
		wndDetailLeftClass.hInstance,
		NULL);

	hwndRight = CreateWindow(szDetailRightWndClass,
		TEXT("Win32 detail right window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		hwndDetail,
		NULL,
		wndDetailRightClass.hInstance,
		NULL);

	hwndTop = CreateWindow(szDetailTopWndClass,
		TEXT("Win32 detail top window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		hwndDetail,
		NULL,
		wndDetailTopClass.hInstance,
		NULL);

	hwndBottom = CreateWindow(szDetailBottomWndClass,
		TEXT("Win32 detail bottom window"),
		WS_CHILD | WS_CLIPSIBLINGS,
		0, 0, 0, 0,
		hwndDetail,
		NULL,
		wndDetailBottomClass.hInstance,
		NULL);

	//Set windows show state and send WM_PAINT message
	ShowWindow(hwndParent, nCmdShow);
	UpdateWindow(hwndParent);

	ShowWindow(hwndSelection, nCmdShow);
	UpdateWindow(hwndSelection);

	//Setup XAML island and associate with the child window
	WindowsXamlManager windowsXamlManager = WindowsXamlManager::InitializeForCurrentThread();

	auto interopDetail = detailWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
	auto interopSelection = selectionWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
	check_hresult(interopDetail->AttachToWindow(hwndDetail));
	check_hresult(interopSelection->AttachToWindow(hwndSelection));

	//Stash the interop handle so that we can resize it when the main hwnd is resized
	interopDetail->get_WindowHandle(&hwndDetailXamlIsland);
	interopSelection->get_WindowHandle(&hwndSelectionXamlIsland);

	SelectionMenu = CreateSelectionMenu();
	selectionWindowXamlSource.Content(SelectionMenu);
	SetWindowPos(hwndSelectionXamlIsland, 0, 0, 0, appRect.right, appRect.bottom, SWP_SHOWWINDOW);
	LoadModule(selectedScenario);

	//End XAML Island
	return 0;
}

# pragma endregion 

#pragma region - WndProc

LRESULT CALLBACK ParentWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_SIZE, ParentWnd_OnSize);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK SelectionWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void CreateControls(HWND hwnd, LPARAM lParam, int region)
{
	if (region == DPR_LEFT) //Left region
	{
		// Scenario 1 - Ink Canvas controls
		//  none

		// Scenario 2 - Media Player Element -  controls
		hwndComboBox = CreateWindow(TEXT("ComboBox"), TEXT("MediaCombo"), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED,
			0, 0, 200, 100, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		ComboBox_AddString(hwndComboBox, TEXT("Podcast URL:1"));
		ComboBox_AddString(hwndComboBox, TEXT("Podcast URL:2"));
		ComboBox_AddString(hwndComboBox, TEXT("Podcast URL:3"));
		ComboBox_AddString(hwndComboBox, TEXT("Podcast URL:4"));
		ComboBox_AddString(hwndComboBox, TEXT("Podcast URL:5"));
		Scenario2.winControlsLeft.push_back(hwndComboBox);
	}
	else //Other regions not defined yet
	{
	}
}

void HandleControlMessage(WPARAM wParam, LPARAM lParam, int region)
{
	if (region == DPR_LEFT) //Left region
	{
		// Scenario 1 controls
		//  none

		//Scenario 2 controls
		if (HIWORD(wParam) == CBN_SELCHANGE) {

			auto index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, 0, 0);
			auto mediaPL = Scenario2.xamlUI.FindName(L"mediaPlayer").as<MediaPlayerElement>();
			Uri path1{ L"https://sec.ch9.ms/ch9/f6b9/3d904698-21b0-4f5c-a175-97962344f6b9/20180827-VSToolbox.mp4" };
			Uri path2{ L"https://sec.ch9.ms/ch9/1e13/b8bc79bb-3a51-4df4-b8c9-281ebbfb1e13/Azure-IoT-Hub-and-Event-GridsV2.mp4" };
			Uri path3{ L"https://sec.ch9.ms/ch9/ac4b/18057df9-af43-4b79-8bbf-7199ebceac4b/IoTInActionUpcomingInPersonEvents.mp4" };
			Uri path4{ L"https://sec.ch9.ms/ch9/968a/38f40667-7ed9-4131-ba09-e6fbbda7968a/MonitoringAndDiagnosticsForIoTHub.mp4" };
			Uri path5{ L"https://sec.ch9.ms/ch9/d514/7cd1d9ec-d5c9-40c6-a54d-7a89b5cbd514/goonazureeventsmessaging.mp4" };

			switch (index) {
			case 0:
				mediaPL.Source(MediaSource::CreateFromUri(path1));
				break;

			case 1:
				mediaPL.Source(MediaSource::CreateFromUri(path2));
				break;

			case 2:
				mediaPL.Source(MediaSource::CreateFromUri(path3));
				break;

			case 3:
				mediaPL.Source(MediaSource::CreateFromUri(path4));
				break;

			case 4:
				mediaPL.Source(MediaSource::CreateFromUri(path5));
				break;

			}
		}
	}
}

LRESULT CALLBACK DetailWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DetailLeftWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

	case WM_CREATE:
		CreateControls(hwnd, lParam, DPR_LEFT);
		return 0;

	case WM_COMMAND:
		HandleControlMessage(wParam, lParam, DPR_LEFT);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DetailRightWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	case WM_CREATE:
		CreateControls(hwnd, lParam, DPR_RIGHT);
		return 0;

	case WM_COMMAND:
		HandleControlMessage(wParam, lParam, DPR_RIGHT);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DetailTopWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, TopDetail_OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

	case WM_CREATE:
		CreateControls(hwnd, lParam, DPR_TOP);
		return 0;

	case WM_COMMAND:
		HandleControlMessage(wParam, lParam, DPR_TOP);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DetailBottomWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	case WM_CREATE:
		CreateControls(hwnd, lParam, DPR_BOTTOM);
		return 0;

	case WM_COMMAND:
		HandleControlMessage(wParam, lParam, DPR_BOTTOM);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

#pragma endregion

#pragma region - Xaml creation

// This function returns a XAML grid UI element that contains the navigation view control. This XAML is hosted in hwndSelectionXamlIsland.
Grid CreateSelectionMenu()
{
	Grid xamlContainer;


	//Create rows in grid for UI layout
	GridLength columnWidth{ 1, GridUnitType::Star };
	ColumnDefinition column0{};
	column0.Width(columnWidth);
	xamlContainer.ColumnDefinitions().Append(column0);

	NavigationView nav;
	NavigationViewItem i1;
	NavigationViewItem i2;
	i1.Icon(SymbolIcon{ Symbol::Edit });
	i1.Content(box_value(L"Inking"));
	i1.Tag(box_value(L"Inking"));
	i1.Visibility(Visibility::Visible);
	i2.Icon(SymbolIcon{ Symbol::SlideShow });
	i2.Content(box_value(L"Media player"));
	i2.Tag(box_value(L"Media"));
	i2.Visibility(Visibility::Visible);
	nav.MenuItems().Append(i1);
	nav.MenuItems().Append(i2);
	nav.IsSettingsVisible(false);
	nav.IsTabStop(false);
	nav.PaneTitle(L"Scenario selection");
	nav.IsBackButtonVisible(NavigationViewBackButtonVisible::Collapsed);
	nav.IsBackEnabled(false);
	nav.Name(L"SelectionMenu");
	Frame contentFrame;
	contentFrame.Name(L"ContentFrame");
	contentFrame.Opacity(0);
	nav.Content(contentFrame);


	// This handler sets the default Navigation View item and lays out the child windows of hwndDetail that overlay the HWND Detail Pane region.
	auto loaded_handler = [](IInspectable const& sender, RoutedEventArgs const& args)
	{
		auto nav = sender.as<NavigationView>();
		auto height = nav.ActualHeight();
		auto width = nav.ActualWidth();
		auto frame = nav.Content().as<Frame>();

		for (auto const& item : nav.MenuItems())
		{
			if (unbox_value<hstring>(item.as<NavigationViewItem>().Tag()) == L"Inking")
			{
				nav.SelectedItem(item);
			}
		}

		selectionWidth = (int)(width - frame.ActualWidth());
		selectionHeight = (int)height;

		detailWidth = (int)frame.ActualWidth();
		detailHeight = (int)frame.ActualHeight();

		topWndWidth = detailWidth;
		topWndHeight = (int)(detailHeight * topHeightPercent);
		bottomWndWidth = detailWidth;
		bottomWndHeight = (int)(detailHeight * bottomHeightPercent);

		leftWndWidth = (int)(detailWidth * leftWidthPercent);
		leftWndHeight = (int)(detailHeight * leftHeightPercent);
		rightWndWidth = (int)(detailWidth * rightWidthPercent);
		rightWndHeight = (int)(detailHeight * rightHeightPercent);

		SetWindowPos(hwndDetail, 0, selectionWidth, 0, detailWidth, detailHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndLeft, 0, 0, topWndHeight, leftWndWidth, leftWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndRight, 0, detailWidth - rightWndWidth, topWndHeight, rightWndWidth, rightWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndTop, 0, 0, 0, topWndWidth, topWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndBottom, 0, 0, detailHeight - bottomWndHeight, bottomWndWidth, bottomWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndDetailXamlIsland, 0, leftWndWidth, topWndHeight, detailWidth - leftWndWidth - rightWndWidth, detailHeight - topWndHeight - bottomWndHeight, SWP_SHOWWINDOW);
		UpdateWindow(hwndDetail);
	};

	// This handler determines the items selected in the Navigation View and loads the appropriate Win32 and XAML UI module in the detail window.
	auto selection_handler = [](IInspectable const& sender, NavigationViewSelectionChangedEventArgs const& args)
	{
		auto selected = args.SelectedItem().as<NavigationViewItem>();
		hstring tag = unbox_value<hstring>(selected.Tag());
		
		switch (selectedScenario)
		{
		case 1:
			CleanupMediaPlayer();
			break;
		default:
			break;
		}

		if (tag == L"Inking")
		{
			selectedScenario = 0;
			InvalidateRect(hwndTop, NULL, true);
			UpdateWindow(hwndTop);
		}
		else if (tag == L"Media")
		{
			selectedScenario = 1;
			InvalidateRect(hwndTop, NULL, true);
			UpdateWindow(hwndTop);
		}
		else
		{
			selectedScenario = -1;
		}

		LoadModule(selectedScenario);
		UpdateWindow(hwndLeft);

		if (selectedScenario == 1)
		{
			ShowWindow(hwndButton, SW_HIDE);
			ShowWindow(hwndComboBox, SW_SHOW);
			SetWindowText(hwndComboBox, TEXT("Choose a podcast..."));
			UpdateWindow(hwndLeft);

		}
		else if (selectedScenario == 2)
		{
			ShowWindow(hwndComboBox, SW_HIDE);
			ShowWindow(hwndButton, SW_SHOW);
			UpdateWindow(hwndLeft);
		}
		else
		{
			ShowWindow(hwndComboBox, SW_HIDE);
			ShowWindow(hwndButton, SW_HIDE);
			UpdateWindow(hwndLeft);

		}
	};

	// This handler responds to hwndParent size changes and updates the detail windows appropriately.
	auto frameSize_handler = [](IInspectable const& sender, SizeChangedEventArgs const& args)
	{
		auto newHeight = args.NewSize().Height;
		auto newWidth = args.NewSize().Width;

		selectionWidth = (int)(SelectionMenu.ActualWidth() - SelectionMenu.FindName(L"ContentFrame").as<Frame>().ActualWidth());
		selectionHeight = (int)newHeight;
		detailWidth = (int)newWidth;
		detailHeight = (int)newHeight;

		topWndWidth = detailWidth;
		topWndHeight = (int)(detailHeight * topHeightPercent);
		bottomWndWidth = detailWidth;
		bottomWndHeight = (int)(detailHeight * bottomHeightPercent);

		leftWndWidth = (int)(detailWidth * leftWidthPercent);
		leftWndHeight = (int)(detailHeight * leftHeightPercent);
		rightWndWidth = (int)(detailWidth * rightWidthPercent);
		rightWndHeight = (int)(detailHeight * rightHeightPercent);

		SetWindowPos(hwndDetail, 0, selectionWidth, 0, detailWidth, detailHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndLeft, 0, 0, topWndHeight, leftWndWidth, leftWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndRight, 0, detailWidth - rightWndWidth, topWndHeight, rightWndWidth, rightWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndTop, 0, 0, 0, topWndWidth, topWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndBottom, 0, 0, detailHeight - bottomWndHeight, bottomWndWidth, bottomWndHeight, SWP_SHOWWINDOW);
		SetWindowPos(hwndDetailXamlIsland, 0, leftWndWidth, topWndHeight, detailWidth - leftWndWidth - rightWndWidth, detailHeight - topWndHeight - bottomWndHeight, SWP_SHOWWINDOW);
		UpdateWindow(hwndDetail);
	};

	contentFrame.SizeChanged(frameSize_handler);
	nav.SelectionChanged(selection_handler);
	nav.Loaded(loaded_handler);


	//Assign Xaml controls position in Grid
	xamlContainer.SetColumn(nav, 0);

	xamlContainer.Children().Append(nav);

	xamlContainer.UpdateLayout();

	return xamlContainer;
}


// Creates the InkCanvas and InkToolbar controls. Creates and returns a XAML UI element that it hosted in hwndDetailXamlIsland.
Grid CreateInkCanvas()
{
	//Define controls for Xaml island
	Grid xamlContainer;
	InkCanvas inkCanvas;
	InkToolbar inkToolbar;
	TextBlock textBlock;

	//Create rows in grid for UI layout
	GridLength rowHeight{ 0, GridUnitType::Auto };
	RowDefinition row0{};
	RowDefinition row1{};
	RowDefinition row2{};

	row0.Height(rowHeight);
	row1.Height(rowHeight);

	xamlContainer.RowDefinitions().Append(row0);
	xamlContainer.RowDefinitions().Append(row1);
	xamlContainer.RowDefinitions().Append(row2);

	//Create border around grid to emphasize Xaml island's window
	xamlContainer.BorderBrush(SolidColorBrush{ Windows::UI::Colors::Black() });
	xamlContainer.BorderThickness(Thickness{ 1,1,1,1 });
	xamlContainer.Padding(Thickness{ 3,3,3,3 });

	//Assign Xaml controls position in Grid
	xamlContainer.SetRow(textBlock, 0);
	xamlContainer.SetRow(inkToolbar, 1);
	xamlContainer.SetRow(inkCanvas, 2);

	//Set control properties
	hstring text(L"Xaml InkCanvas and InkToolbar");
	Binding titleBinding{};
	titleBinding.Mode(BindingMode::OneTime);
	titleBinding.Source(box_value(text));
	textBlock.SetBinding(TextBlock().TextProperty(), titleBinding);
	textBlock.Width(300);
	textBlock.HorizontalAlignment(HorizontalAlignment::Left);

	inkToolbar.VerticalAlignment(VerticalAlignment::Top);
	inkToolbar.HorizontalAlignment(HorizontalAlignment::Center);
	inkToolbar.TargetInkCanvas(inkCanvas);

	inkCanvas.InkPresenter().InputDeviceTypes(
		CoreInputDeviceTypes::Mouse |
		CoreInputDeviceTypes::Touch |
		CoreInputDeviceTypes::Pen);

	//Add controls to UI grid
	xamlContainer.Children().Append(inkCanvas);
	xamlContainer.Children().Append(inkToolbar);
	xamlContainer.Children().Append(textBlock);
	xamlContainer.UpdateLayout();
	return xamlContainer;
}

// Creates the UWP MediaPlayerElement control. Creates and returns a XAML UI element that it hosted in hwndDetailXamlIsland.
Grid CreateMediaPlayer()
{
	//Define controls for Xaml island
	Grid xamlContainer;
	TextBlock textBlock;
	MediaPlayerElement media;
	media.Name(L"mediaPlayer");

	//Create rows in grid for UI layout
	GridLength rowHeightAuto{ 0, GridUnitType::Auto };
	GridLength rowHeightStar{ 1, GridUnitType::Star };

	RowDefinition row0{};
	RowDefinition row1{};

	row0.Height(rowHeightAuto);
	row1.Height(rowHeightStar);

	xamlContainer.RowDefinitions().Append(row0);
	xamlContainer.RowDefinitions().Append(row1);

	//Create border around grid to emphasize Xaml island's window
	xamlContainer.BorderBrush(SolidColorBrush{ Windows::UI::Colors::Black() });
	xamlContainer.BorderThickness(Thickness{ 1,1,1,1 });
	xamlContainer.Padding(Thickness{ 3,3,3,3 });

	//Assign Xaml controls position in Grid
	xamlContainer.SetRow(textBlock, 0);
	xamlContainer.SetRow(media, 1);

	//Set control properties
	hstring text(L"Xaml MediaPlayerElement");
	Binding titleBinding{};
	titleBinding.Mode(BindingMode::OneTime);
	titleBinding.Source(box_value(text));
	textBlock.SetBinding(TextBlock().TextProperty(), titleBinding);
	textBlock.Width(300);
	textBlock.HorizontalAlignment(HorizontalAlignment::Left);

	//Set Media Player Element stretch and show media bar

	media.Stretch(Stretch::UniformToFill);
	media.AreTransportControlsEnabled(true);

	//Add controls to UI grid

	xamlContainer.Children().Append(textBlock);
	xamlContainer.Children().Append(media);
	xamlContainer.UpdateLayout();
	return xamlContainer;

}

void CleanupMediaPlayer() 
{
	auto mediaPL = Scenario2.xamlUI.FindName(L"mediaPlayer").as<MediaPlayerElement>();
	if (!mediaPL.Source()== NULL) {
		mediaPL.MediaPlayer().Pause();
	}
}

// Sets the appropriate XAML for hosting based on Navigation View selection.
void LoadModule(int index)
{
	switch (index)
	{
	case 0:
		if (!Scenario1.xamlUI)
		{
			Scenario1.xamlUI = CreateInkCanvas();
			detailWindowXamlSource.Content(Scenario1.xamlUI);
		}
		else
			detailWindowXamlSource.Content(Scenario1.xamlUI);
		break;
	case 1:
		if (!Scenario2.xamlUI)
		{
			Scenario2.xamlUI = CreateMediaPlayer();
			detailWindowXamlSource.Content(Scenario2.xamlUI);
		}
		else
			detailWindowXamlSource.Content(Scenario2.xamlUI);
		break;
	default:
		break;
	}
}
#pragma endregion

