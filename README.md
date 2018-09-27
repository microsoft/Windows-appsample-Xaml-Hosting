# Introduction 
The XAML Hosting API sample is a win32 desktop app that demonstrates assorted scenarios using the UWP XAML hosting API (also called XAML islands).  

![XAML Hosted API sample showing the layout architecture](Images/XamlInking.JPG)
 
Highlights: 
 
* Windows desktop app that uses the XAML hosting API and c++ /WinRT to demonstrate usage of XAML controls within a win32 context. The scenarios demonstrated are: 
    * Using the Windows Ink UWP feature within a desktop app
    * Playing select video files with the UWP Media Player Element control
    * Handling XAML and native windows events/messages
    * Demonstrates basic XAML data binding

>Note: The UWP XAML hosting API is currently available as a developer preview. Although we encourage you to try out this API in your own prototype code now, we do not recommend that you use it in production code at this time. This API, and the sample, will continue to mature and stabilize in future Windows releases. 
 

# Prerequisites

1.	Visual Studio 2017
2.  Windows SDK Insider Preview Build 17763
3.	Windows 10 Insider Preview Build 17763

# App architecture
This sample is a "gallery style" project that incorporates Universal Windows Platform (UWP) controls into a Win32 Desktop app.  Currently the following controls are used:

- Windows Ink Canvas and Ink Toolbar 
- Media Player Element
- Navigation View

The presentation is module-based, and you can click through each of the XAML UI supported scenarios via the Navigation View control. The latter is a hosted XAML UI element that wraps all HWND/XAML content to provide a UWP functional, and themed, navigation experience. The below diagram illustrates the parent and child HWND layout, and the hosted XAML UI.

![XAML Hosted API sample showing the layout architecture](Images/XAMLLayout.JPG)


1. The HWND parent/child hierarchy in the sample.
2. The parent **HWND (hwndParent)** contains two child windows. The first, **hwndSelection**, holds the child **hwndSelectionXamlIsland**, which hosts the XAML Navigation View control. 
3. The second child of **hwndParent**, **hwndDetail**, is not bound to the Navigation View content property and the window is overlayed onto the **HWND Detail Pane** region depicted in the second callout. This HWND contains windows for Win32 controls as well as a window for containing XAML UI controls.

# Code at a glance

If you're just interested in code snippets for certain areas, and don't want to browse or run the full sample, check out the following functions. The code for this project is located in [Main.cpp](XamlDemoGallery/XamlDemoGallery/Main.cpp#L10).

- **WindowInit** - Defines and registers WNDCLASS structs for windows, creates the windows including the window for hosting the XAML Navigation View control, and displays the navigation UI. 
- **CreateSelectionMenu** - This function returns a XAML grid UI element that contains the navigation view control. This XAML is hosted in **hwndSelectionXamlIsland**. The function also registers handlers for three events:
    - **NavigationView.Loaded** - This handler sets the default Navigation View item and lays out the child windows of hwndDetail that overlay the HWND Detail Pane region.
    - **NavigationView.SelectionChanged** - This handler determines the items selected in the Navigation View and loads the appropriate Win32 and XAML UI module in the detail window.
    - **Frame.SizeChanged** - This handler responds to **hwndParent** size changes and updates the detail windows appropriately.
- **CreateInkCanvas** - Creates the InkCanvas and InkToolbar controls. Creates and returns a XAML UI element that it hosted in **hwndDetailXamlIsland.**
- **CreateMediaPlayer** - Creates the UWP MediaPlayerElement control. Creates and returns a XAML UI element that it hosted in **hwndDetailXamlIsland.**
- **LoadModule** - Sets the appropriate XAML for hosting based on Navigation View selection.


# Known Issues

- The Media Player Element control cannot be maximized to full screen in the **Media Player** module at this time. The control will disappear if this is done and you'll need to click out of the module and back to restore the view.


# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# See also

* [C++/WinRT Documentation](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)
* [Using the UWP XAML hosting API](https://docs.microsoft.com/windows/uwp/xaml-platform/using-the-xaml-hosting-api)
* [DesktopWindowXamlSource](https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.hosting.desktopwindowxamlsource)
