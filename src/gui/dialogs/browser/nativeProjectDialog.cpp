#include "src/gui/dialogs/browser/nativeProjectDialog.h"

#ifdef _WIN32
#include <juce_core/juce_core.h>
#include <shobjidl.h>
#include <wrl/client.h>

namespace giada::v
{
NativeProjectSelection showNativeProjectDialog(bool save, void* owner,
    const std::string& title, const std::string& buttonLabel,
    const std::string& directory, const std::string& name,
    const std::string& nameLabel, const std::string& folderLabel)
{
	using Microsoft::WRL::ComPtr;
	NativeProjectSelection result;
	const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	struct ComScope
	{
		bool initialized;
		~ComScope() { if (initialized) CoUninitialize(); }
	} comScope{SUCCEEDED(initialized)};
	if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE)
		return result;

	ComPtr<IFileOpenDialog> dialog;
	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
	        IID_PPV_ARGS(dialog.GetAddressOf()))))
		return result;

	FILEOPENDIALOGOPTIONS options;
	if (FAILED(dialog->GetOptions(&options)) ||
	    FAILED(dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR)))
		return result;
	dialog->SetTitle(juce::String::fromUTF8(title.c_str()).toWideCharPointer());
	dialog->SetOkButtonLabel(juce::String::fromUTF8(buttonLabel.c_str()).toWideCharPointer());
	dialog->SetFileNameLabel(juce::String::fromUTF8(folderLabel.c_str()).toWideCharPointer());

	if (!directory.empty())
	{
		ComPtr<IShellItem> initial;
		if (SUCCEEDED(SHCreateItemFromParsingName(
		        juce::String::fromUTF8(directory.c_str()).toWideCharPointer(), nullptr,
		        IID_PPV_ARGS(initial.GetAddressOf()))))
			dialog->SetFolder(initial.Get());
	}

	constexpr DWORD NAME_LABEL = 100;
	constexpr DWORD NAME_EDIT = 101;
	ComPtr<IFileDialogCustomize> customization;
	if (save)
	{
		if (FAILED(dialog.As(&customization)) ||
		    FAILED(customization->AddText(NAME_LABEL, juce::String::fromUTF8(nameLabel.c_str()).toWideCharPointer())) ||
		    FAILED(customization->AddEditBox(NAME_EDIT, juce::String::fromUTF8(name.c_str()).toWideCharPointer())))
			return result;
	}

	const HRESULT shown = dialog->Show(static_cast<HWND>(owner));
	if (shown == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		result.status = NativeProjectSelection::Status::CANCELLED;
		return result;
	}
	if (FAILED(shown))
		return result;

	ComPtr<IShellItem> selected;
	PWSTR path = nullptr;
	if (FAILED(dialog->GetResult(&selected)) || FAILED(selected->GetDisplayName(SIGDN_FILESYSPATH, &path)))
		return result;
	result.directory = juce::String(path).toStdString();
	CoTaskMemFree(path);
	if (save)
	{
		PWSTR enteredName = nullptr;
		if (FAILED(customization->GetEditBoxText(NAME_EDIT, &enteredName)))
			return result;
		result.name = juce::String(enteredName).toStdString();
		CoTaskMemFree(enteredName);
	}
	result.status = NativeProjectSelection::Status::SELECTED;
	return result;
}
} // namespace giada::v
#endif
