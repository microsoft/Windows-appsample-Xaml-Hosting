// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#ifndef PCH_H
#define PCH_H
#include <unknwn.h>
// WinBase.h
// Windows::UI::Xaml::Media::Animation::IStoryboard::GetCurrentTime
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#include <winrt/Windows.system.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.Inking.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>

#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <tchar.h>

#include <windows.h>
#include <windowsx.h>
#endif
